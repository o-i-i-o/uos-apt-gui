#include "packageargs.h"
#include <QFileDevice>
#include <QFileInfo>
#include <QRegularExpression>

namespace
{
const int kMaxItems = 200;

QString abbreviate(const QString &value)
{
    return value.left(96);
}
}

QString PackageArgs::validatePackageNames(const QStringList &names)
{
    if (names.isEmpty()) {
        return QStringLiteral("未选择任何软件包");
    }
    if (names.size() > kMaxItems) {
        return QStringLiteral("单次最多处理 %1 个软件包").arg(kMaxItems);
    }

    static const QRegularExpression pattern(QStringLiteral("^[a-z0-9][a-z0-9+.-]*$"));
    for (const QString &name : names) {
        if (name.size() > 200 || !pattern.match(name).hasMatch()) {
            return QStringLiteral("非法包名: %1").arg(abbreviate(name));
        }
    }
    return QString();
}

QString PackageArgs::validateLocalDebPaths(const QStringList &paths, QStringList *canonicalPaths)
{
    if (paths.isEmpty()) {
        return QStringLiteral("未选择任何 DEB 文件");
    }
    if (paths.size() > kMaxItems) {
        return QStringLiteral("单次最多处理 %1 个 DEB 文件").arg(kMaxItems);
    }

    QStringList resolved;
    for (const QString &path : paths) {
        if (path.isEmpty() || path.contains(QLatin1Char('\0')) || !path.startsWith(QLatin1Char('/'))) {
            return QStringLiteral("必须是绝对路径: %1").arg(abbreviate(path));
        }

        const QString canonical = QFileInfo(path).canonicalFilePath();
        if (canonical.isEmpty()) {
            return QStringLiteral("文件不存在: %1").arg(abbreviate(path));
        }

        const QFileInfo target(canonical);
        if (!target.isFile()) {
            return QStringLiteral("不是普通文件: %1").arg(abbreviate(path));
        }
        if (target.suffix().compare(QLatin1String("deb"), Qt::CaseSensitive) != 0) {
            return QStringLiteral("不是 .deb 文件: %1").arg(abbreviate(path));
        }
        if (target.permissions() & (QFileDevice::WriteGroup | QFileDevice::WriteOther)) {
            return QStringLiteral("文件权限不安全（组或其他用户可写）: %1").arg(abbreviate(path));
        }

        resolved.append(canonical);
    }

    if (canonicalPaths) {
        *canonicalPaths = resolved;
    }
    return QString();
}
