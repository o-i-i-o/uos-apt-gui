#include "programpaths.h"

#include <QStandardPaths>
#include <QStringList>

QString ProgramPaths::findSystemExecutable(const QString &name)
{
    // 只需要裸命令名，路径由本函数决定；带斜杠的输入一律拒绝
    if (name.isEmpty() || name.contains(QLatin1Char('/'))) {
        return QString();
    }

    static const QStringList directories{
        QStringLiteral("/usr/bin"),
        QStringLiteral("/bin"),
        QStringLiteral("/usr/sbin"),
        QStringLiteral("/sbin")
    };

    return QStandardPaths::findExecutable(name, directories);
}
