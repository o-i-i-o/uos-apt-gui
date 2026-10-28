#include "polkitauthorizer.h"
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDebug>
#include <QVariantMap>

namespace Polkit
{
struct Subject {
    QString kind;
    QVariantMap details;
};

QDBusArgument &operator<<(QDBusArgument &argument, const Subject &subject)
{
    argument.beginStructure();
    argument << subject.kind << subject.details;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, Subject &subject)
{
    argument.beginStructure();
    argument >> subject.kind >> subject.details;
    argument.endStructure();
    return argument;
}
}
Q_DECLARE_METATYPE(Polkit::Subject)

PolkitAuthorizer::PolkitAuthorizer(QObject *parent)
    : QObject(parent)
{
    qDBusRegisterMetaType<Polkit::Subject>();
}

void PolkitAuthorizer::check(const QString &actionId, quint32 pid, quint64 startTime,
                             const std::function<void(bool, const QString &)> &callback)
{
    Polkit::Subject subject;
    subject.kind = QStringLiteral("unix-process");
    subject.details.insert(QStringLiteral("pid"), QVariant::fromValue<quint32>(pid));
    subject.details.insert(QStringLiteral("start-time"), QVariant::fromValue<quint64>(startTime));

    QDBusMessage call = QDBusMessage::createMethodCall(
            QStringLiteral("org.freedesktop.PolicyKit1"),
            QStringLiteral("/org/freedesktop/PolicyKit1/Authority"),
            QStringLiteral("org.freedesktop.PolicyKit1.Authority"),
            QStringLiteral("CheckAuthorization"));
    call << QVariant::fromValue(subject)
         << actionId
         << QVariantMap()
         << QVariant::fromValue<quint32>(1)   // AllowUserInteraction
         << QString();

    QDBusPendingCall pending = QDBusConnection::systemBus().asyncCall(call, 120000);
    auto *watcher = new QDBusPendingCallWatcher(pending, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [callback](QDBusPendingCallWatcher *self) {
        self->deleteLater();

        const QDBusMessage reply = self->reply();
        if (reply.type() != QDBusMessage::ReplyMessage || reply.arguments().isEmpty()) {
            qWarning() << "polkit CheckAuthorization 失败:" << reply.errorName() << reply.errorMessage();
            callback(false, QStringLiteral("授权服务不可用"));
            return;
        }

        bool authorized = false;
        const QVariant first = reply.arguments().constFirst();
        if (first.userType() == qMetaTypeId<QDBusArgument>()) {
            // const 才能命中 beginStructure() 的只读重载
            const QDBusArgument argument = first.value<QDBusArgument>();
            bool challenge = false;
            QVariantMap details;
            argument.beginStructure();
            argument >> authorized >> challenge >> details;
            argument.endStructure();
            Q_UNUSED(challenge);
            Q_UNUSED(details);
        }

        callback(authorized, authorized ? QString() : QStringLiteral("未获得授权"));
    });
}
