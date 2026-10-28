#include "helperclient.h"
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDebug>
#include <QVariant>

namespace
{
const char kService[] = "com.debmanager.Helper";
const char kPath[] = "/com/debmanager/Helper";
const char kInterface[] = "com.debmanager.Helper1";

// 助手等待 polkit 最长 120 秒，客户端需更宽限
const int kCallTimeoutMs = 150000;

QString friendlyError(const QString &name, const QString &message)
{
    QString detail = message.trimmed();
    if (detail.isEmpty()) {
        detail = QStringLiteral("未知错误");
    }

    if (name == QLatin1String("com.debmanager.Helper1.Error.NotAuthorized")) {
        return QStringLiteral("未获得授权：%1").arg(detail);
    }
    if (name == QLatin1String("com.debmanager.Helper1.Error.Busy")) {
        return QStringLiteral("已有操作正在进行，请稍后再试");
    }
    if (name == QLatin1String("com.debmanager.Helper1.Error.InvalidArgument")) {
        return QStringLiteral("参数无效：%1").arg(detail);
    }
    if (name == QLatin1String("org.freedesktop.DBus.Error.ServiceUnknown")
        || name == QLatin1String("org.freedesktop.DBus.Error.NameHasNoOwner")) {
        return QStringLiteral("包管理助手服务未运行，请检查 apt-helperd 是否已启动");
    }
    if (name == QLatin1String("org.freedesktop.DBus.Error.NoReply")) {
        return QStringLiteral("等待助手响应超时：%1").arg(detail);
    }
    if (name == QLatin1String("org.freedesktop.DBus.Error.Disconnected")) {
        return QStringLiteral("与系统总线的连接已断开");
    }
    return detail;
}
}

HelperClient::HelperClient(QObject *parent)
    : QObject(parent)
{
    QDBusConnection bus = QDBusConnection::systemBus();
    if (!bus.connect(QLatin1String(kService), QLatin1String(kPath), QLatin1String(kInterface),
                     QStringLiteral("Output"), this, SLOT(onRemoteOutput(QString,QString)))) {
        qWarning() << "订阅助手 Output 信号失败:" << bus.lastError().message();
    }
    if (!bus.connect(QLatin1String(kService), QLatin1String(kPath), QLatin1String(kInterface),
                     QStringLiteral("Finished"), this, SLOT(onRemoteFinished(QString,int,QString)))) {
        qWarning() << "订阅助手 Finished 信号失败:" << bus.lastError().message();
    }
}

void HelperClient::installLocalPackages(const QStringList &paths)
{
    call(QStringLiteral("InstallLocalPackages"), QVariantList{QVariant::fromValue(paths)});
}

void HelperClient::installPackages(const QStringList &names)
{
    call(QStringLiteral("InstallPackages"), QVariantList{QVariant::fromValue(names)});
}

void HelperClient::removePackages(const QStringList &names, bool purge)
{
    call(QStringLiteral("RemovePackages"), QVariantList{QVariant::fromValue(names), purge});
}

void HelperClient::updateCache()
{
    call(QStringLiteral("UpdateCache"), QVariantList());
}

void HelperClient::upgrade(bool distUpgrade)
{
    call(QStringLiteral("Upgrade"), QVariantList{distUpgrade});
}

void HelperClient::autoRemove()
{
    call(QStringLiteral("AutoRemove"), QVariantList());
}

void HelperClient::call(const QString &method, const QVariantList &arguments)
{
    m_transaction.clear();

    QDBusMessage request = QDBusMessage::createMethodCall(QLatin1String(kService),
                                                          QLatin1String(kPath),
                                                          QLatin1String(kInterface),
                                                          method);
    request.setArguments(arguments);

    QDBusPendingCall pending = QDBusConnection::systemBus().asyncCall(request, kCallTimeoutMs);
    auto *watcher = new QDBusPendingCallWatcher(pending, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this](QDBusPendingCallWatcher *self) {
        self->deleteLater();

        const QDBusMessage reply = self->reply();
        if (reply.type() == QDBusMessage::ErrorMessage) {
            qWarning() << "助手请求失败:" << reply.errorName() << reply.errorMessage();
            emit failed(friendlyError(reply.errorName(), reply.errorMessage()));
            return;
        }

        const QString transaction = reply.arguments().value(0).toString();
        if (transaction.isEmpty()) {
            emit failed(QStringLiteral("助手返回了空的事务标识"));
            return;
        }

        m_transaction = transaction;
        emit accepted(transaction);
    });
}

void HelperClient::onRemoteOutput(const QString &transaction, const QString &text)
{
    // 回复到达前可能先收到信号，此时 m_transaction 为空，按当前唯一事务处理
    if (m_transaction.isEmpty() || transaction == m_transaction) {
        emit output(transaction, text);
    }
}

void HelperClient::onRemoteFinished(const QString &transaction, int exitCode, const QString &message)
{
    if (!m_transaction.isEmpty() && transaction != m_transaction) {
        return;
    }

    m_transaction.clear();
    emit finished(transaction, exitCode, message);
}
