#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>
#include "aptbackend.h"
#include "helperservice.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("apt-helperd"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1.0.1"));

    const QStringList arguments = app.arguments();
    const bool dryRun = arguments.contains(QStringLiteral("--dry-run"));

    QString stagingDir;
    const int stagingIndex = arguments.indexOf(QStringLiteral("--staging-dir"));
    if (stagingIndex >= 0) {
        if (stagingIndex + 1 >= arguments.size()
            || arguments.at(stagingIndex + 1).startsWith(QLatin1String("--"))) {
            qCritical() << "--staging-dir 需要一个目录参数";
            return 1;
        }
        stagingDir = arguments.at(stagingIndex + 1);
        if (!stagingDir.startsWith(QLatin1Char('/'))) {
            qCritical() << "--staging-dir 必须是绝对路径";
            return 1;
        }
    }

    AptBackend backend;
    backend.setDryRun(dryRun);
    backend.setStagingDir(stagingDir);

    HelperService service(&backend);

    QDBusConnection bus = QDBusConnection::systemBus();
    if (!bus.isConnected()) {
        qCritical() << "无法连接系统总线:" << bus.lastError().message();
        return 1;
    }
    if (!bus.registerObject(QStringLiteral("/com/debmanager/Helper"), &service,
                            QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
        qCritical() << "注册 D-Bus 对象失败:" << bus.lastError().message();
        return 1;
    }
    if (!bus.registerService(QStringLiteral("com.debmanager.Helper"))) {
        qCritical() << "注册 D-Bus 服务名失败:" << bus.lastError().message();
        return 1;
    }

    if (dryRun) {
        qInfo() << "apt-helperd 以 dry-run 模式运行，不执行真实命令";
    }

    return app.exec();
}
