#ifndef PACKAGEARGS_H
#define PACKAGEARGS_H

#include <QString>
#include <QStringList>

namespace PackageArgs
{
// 返回空字符串表示通过；否则返回可直接展示给用户的中文错误
QString validatePackageNames(const QStringList &names);
QString validateLocalDebPaths(const QStringList &paths, QStringList *canonicalPaths);
}

#endif // PACKAGEARGS_H
