#include "operationprogressdialog.h"
#include <QMessageBox>
#include <QScrollBar>

OperationProgressDialog::OperationProgressDialog(QWidget *parent)
    : QDialog(parent)
    , isFinished(false)
    , isSuccess(false)
{
    setWindowTitle("操作进度");
    setMinimumSize(600, 400);
    setModal(true);

    // 创建布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 操作名称标签
    operationLabel = new QLabel("正在执行操作...");
    operationLabel->setStyleSheet("font-weight: bold;");
    mainLayout->addWidget(operationLabel);

    // 输出文本框
    outputTextEdit = new QTextEdit();
    outputTextEdit->setReadOnly(true);
    outputTextEdit->setFontFamily("Monospace");
    outputTextEdit->setLineWrapMode(QTextEdit::NoWrap);
    mainLayout->addWidget(outputTextEdit);

    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    // 关闭按钮
    closeButton = new QPushButton("关闭");
    closeButton->setEnabled(false);
    connect(closeButton, &QPushButton::clicked, this, &OperationProgressDialog::close);
    buttonLayout->addWidget(closeButton);

    mainLayout->addLayout(buttonLayout);
}

OperationProgressDialog::~OperationProgressDialog()
{
}

void OperationProgressDialog::setOperationName(const QString &name)
{
    operationLabel->setText(name);
}

void OperationProgressDialog::appendOutput(const QString &output)
{
    outputTextEdit->append(output);
    // 自动滚动到底部
    QScrollBar *scrollBar = outputTextEdit->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void OperationProgressDialog::setFinished(bool success, const QString &message)
{
    isFinished = true;
    isSuccess = success;
    resultMessage = message;

    appendOutput("\n" + message);
    closeButton->setEnabled(true);
    closeButton->setFocus();
}

void OperationProgressDialog::closeEvent(QCloseEvent *event)
{
    if (!isFinished) {
        int result = QMessageBox::question(this, "确认关闭", 
                                        "操作正在进行中，确定要关闭吗？",
                                        QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::No) {
            event->ignore();
            return;
        }
    }
    event->accept();
}