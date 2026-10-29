#ifndef PROGRAMPATHS_H
#define PROGRAMPATHS_H

#include <QString>

namespace ProgramPaths
{
// 仅在固定系统目录中查找可执行文件，不继承调用方的 PATH，避免被同名
// 程序劫持。返回绝对路径；找不到时返回空字符串。
QString findSystemExecutable(const QString &name);
}

#endif // PROGRAMPATHS_H
