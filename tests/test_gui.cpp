#include <QtTest>
#include <QAbstractButton>
#include <QApplication>
#include <QLabel>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QSignalSpy>
#include <QTextEdit>
#include <QTimer>

#include "mainwindow.h"
#include "operationprogressdialog.h"
#include "packagemanager.h"

namespace
{
void answerNextMessageBox(QMessageBox::StandardButton button)
{
    QTimer::singleShot(0, [button]() {
        QMessageBox *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
        if (!box) {
            for (QWidget *widget : QApplication::topLevelWidgets()) {
                QMessageBox *candidate = qobject_cast<QMessageBox *>(widget);
                if (candidate && candidate->isVisible()) {
                    box = candidate;
                    break;
                }
            }
        }
        if (!box) {
            return;
        }
        // 点击真实按钮，确保 QMessageBox::question 返回标准按钮值
        QAbstractButton *target = box->button(button);
        if (target) {
            target->click();
        } else {
            box->done(button);
        }
    });
}
}

class GuiTest : public QObject
{
    Q_OBJECT

private slots:
    void loadIsBackground();
    void installShowsDialogAndTypedProgress();
    void typedFinishOnlyClosesMatchingDialog();
    void stalledAndRejectedCancel();
    void windowCancelWithoutTransaction();
    void cancelConfirmationEmitsOnce();
    void closeDuringRunRequiresConfirmation();
    void finishedButtonClosesDialog();

private:
    static QString statusText(MainWindow &window)
    {
        QLabel *label = window.findChild<QLabel *>(QStringLiteral("statusLabel"));
        return label ? label->text() : QString();
    }

    static bool waitForInitialLoad(MainWindow &window)
    {
        return QTest::qWaitFor([&window]() {
            const QString text = statusText(window);
            return text.startsWith(QStringLiteral("就绪 - 共加载"))
                    || text.startsWith(QStringLiteral("加载包列表失败"));
        }, 15000);
    }

    static OperationProgressDialog *findDialog(MainWindow &window)
    {
        return window.findChild<OperationProgressDialog *>();
    }

    static QPushButton *dialogButton(OperationProgressDialog *dialog)
    {
        return dialog->findChild<QPushButton *>();
    }

    static QString dialogText(OperationProgressDialog *dialog)
    {
        QTextEdit *edit = dialog->findChild<QTextEdit *>();
        return edit ? edit->toPlainText() : QString();
    }

    static QLabel *dialogLabel(OperationProgressDialog *dialog)
    {
        return dialog->findChild<QLabel *>();
    }
};

void GuiTest::loadIsBackground()
{
    MainWindow window;
    QVERIFY(waitForInitialLoad(window));

    PackageManager *manager = window.findChild<PackageManager *>();
    QVERIFY(manager);
    QProgressBar *bar = window.findChild<QProgressBar *>();
    QVERIFY(bar);

    QVERIFY(!findDialog(window));
    QVERIFY(bar->isHidden());

    emit manager->operationStarted(PackageManager::OperationType::LoadInstalled,
                                   QStringLiteral("加载中"));
    emit manager->operationProgress(PackageManager::OperationType::LoadInstalled,
                                    QStringLiteral("解析中"));
    QVERIFY(!findDialog(window));
    QVERIFY(bar->isHidden());
    QVERIFY(window.isEnabled());

    emit manager->operationFinished(PackageManager::OperationType::LoadInstalled, false,
                                    QStringLiteral("dpkg 不可用"));
    QVERIFY(!findDialog(window));
    QVERIFY(statusText(window).contains(QStringLiteral("加载包列表失败")));
}

void GuiTest::installShowsDialogAndTypedProgress()
{
    MainWindow window;
    QVERIFY(waitForInitialLoad(window));

    PackageManager *manager = window.findChild<PackageManager *>();
    QVERIFY(manager);
    QProgressBar *bar = window.findChild<QProgressBar *>();
    QVERIFY(bar);

    const QString message = QStringLiteral("正在安装 1 个软件包");
    emit manager->operationStarted(PackageManager::OperationType::InstallLocal, message);

    OperationProgressDialog *dialog = findDialog(window);
    QVERIFY(dialog);
    QVERIFY(!dialog->isHidden());
    QVERIFY(!window.isEnabled());
    QVERIFY(!bar->isHidden());
    QVERIFY(dialogLabel(dialog));
    QCOMPARE(dialogLabel(dialog)->text(), message);
    QCOMPARE(dialogButton(dialog)->text(), QStringLiteral("取消操作"));

    emit manager->operationProgress(PackageManager::OperationType::InstallLocal,
                                    QStringLiteral("正在解包"));
    QCOMPARE(dialogLabel(dialog)->text(), QStringLiteral("正在解包"));

    // 后台加载进度不应改写当前安装对话框
    emit manager->operationProgress(PackageManager::OperationType::LoadInstalled,
                                    QStringLiteral("后台刷新"));
    QCOMPARE(dialogLabel(dialog)->text(), QStringLiteral("正在解包"));
}

void GuiTest::typedFinishOnlyClosesMatchingDialog()
{
    MainWindow window;
    QVERIFY(waitForInitialLoad(window));

    PackageManager *manager = window.findChild<PackageManager *>();
    QVERIFY(manager);
    QProgressBar *bar = window.findChild<QProgressBar *>();
    QVERIFY(bar);

    emit manager->operationStarted(PackageManager::OperationType::InstallLocal,
                                   QStringLiteral("正在安装"));
    OperationProgressDialog *dialog = findDialog(window);
    QVERIFY(dialog);

    // 加载操作的结束信号不能收尾安装对话框
    emit manager->operationFinished(PackageManager::OperationType::LoadInstalled, true,
                                    QStringLiteral("加载完成"));
    QCOMPARE(dialogButton(dialog)->text(), QStringLiteral("取消操作"));
    QVERIFY(!window.isEnabled());
    QVERIFY(!bar->isHidden());

    emit manager->operationFinished(PackageManager::OperationType::InstallLocal, false,
                                    QStringLiteral("安装失败原因"));
    QCOMPARE(dialogButton(dialog)->text(), QStringLiteral("关闭"));
    QVERIFY(window.isEnabled());
    QVERIFY(bar->isHidden());
    QVERIFY(dialogText(dialog).contains(QStringLiteral("安装失败原因")));
    QVERIFY(statusText(window).contains(QStringLiteral("操作失败: 安装失败原因")));
}

void GuiTest::stalledAndRejectedCancel()
{
    MainWindow window;
    QVERIFY(waitForInitialLoad(window));

    PackageManager *manager = window.findChild<PackageManager *>();
    QVERIFY(manager);

    emit manager->operationStarted(PackageManager::OperationType::InstallLocal,
                                   QStringLiteral("正在安装"));
    OperationProgressDialog *dialog = findDialog(window);
    QVERIFY(dialog);

    emit manager->operationStalled(PackageManager::OperationType::InstallLocal);
    QVERIFY(dialogText(dialog).contains(QStringLiteral("长时间没有输出")));

    emit manager->cancelRejected(QStringLiteral("没有正在进行的操作"));
    QVERIFY(dialogText(dialog).contains(QStringLiteral("取消失败: 没有正在进行的操作")));
    QCOMPARE(dialogButton(dialog)->text(), QStringLiteral("取消操作"));
    QVERIFY(dialogButton(dialog)->isEnabled());
    QVERIFY(statusText(window).contains(QStringLiteral("取消失败")));
}

void GuiTest::windowCancelWithoutTransaction()
{
    MainWindow window;
    QVERIFY(waitForInitialLoad(window));

    PackageManager *manager = window.findChild<PackageManager *>();
    QVERIFY(manager);

    emit manager->operationStarted(PackageManager::OperationType::InstallLocal,
                                   QStringLiteral("正在安装"));
    OperationProgressDialog *dialog = findDialog(window);
    QVERIFY(dialog);

    answerNextMessageBox(QMessageBox::Yes);
    dialogButton(dialog)->click();

    // 没有活动事务时助手会立即拒绝取消，对话框应恢复可再次取消
    QTRY_VERIFY(statusText(window).contains(QStringLiteral("取消失败")));
    QVERIFY(dialogText(dialog).contains(QStringLiteral("取消失败")));
    QCOMPARE(dialogButton(dialog)->text(), QStringLiteral("取消操作"));
    QVERIFY(dialogButton(dialog)->isEnabled());
}

void GuiTest::cancelConfirmationEmitsOnce()
{
    OperationProgressDialog dialog;
    dialog.show();
    QSignalSpy spy(&dialog, &OperationProgressDialog::cancelRequested);

    answerNextMessageBox(QMessageBox::Yes);
    dialog.requestCancel();
    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(dialogButton(&dialog)->text(), QStringLiteral("正在取消..."));
    QVERIFY(!dialogButton(&dialog)->isEnabled());

    // 取消进行中再次请求不应重复发信号
    dialog.requestCancel();
    QCOMPARE(spy.count(), 1);

    dialog.cancelFailed(QStringLiteral("被拒绝"));
    QCOMPARE(dialogButton(&dialog)->text(), QStringLiteral("取消操作"));
    QVERIFY(dialogButton(&dialog)->isEnabled());
    QVERIFY(dialogText(&dialog).contains(QStringLiteral("取消失败: 被拒绝")));
}

void GuiTest::closeDuringRunRequiresConfirmation()
{
    OperationProgressDialog dialog;
    dialog.show();
    QSignalSpy spy(&dialog, &OperationProgressDialog::cancelRequested);

    answerNextMessageBox(QMessageBox::No);
    dialog.close();
    QCOMPARE(spy.count(), 0);
    QVERIFY(!dialog.isHidden());

    answerNextMessageBox(QMessageBox::Yes);
    dialog.close();
    QTRY_COMPARE(spy.count(), 1);
    QVERIFY(!dialog.isHidden());

    dialog.setFinished(true, QStringLiteral("操作完成"));
    dialog.close();
    QVERIFY(dialog.isHidden());
}

void GuiTest::finishedButtonClosesDialog()
{
    OperationProgressDialog dialog;
    dialog.show();
    dialog.setFinished(true, QStringLiteral("操作完成"));

    QPushButton *button = dialogButton(&dialog);
    QCOMPARE(button->text(), QStringLiteral("关闭"));
    button->click();
    QVERIFY(dialog.isHidden());
}

int main(int argc, char *argv[])
{
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("offscreen"));
    }
    QApplication app(argc, argv);
    GuiTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_gui.moc"
