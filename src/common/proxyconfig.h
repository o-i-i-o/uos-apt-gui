#ifndef PROXYCONFIG_H
#define PROXYCONFIG_H

#include <QString>

class QProcessEnvironment;

namespace ProxyConfig
{
// 从受信任的配置文件读取代理变量并写入 environment。
// 仅接受固定的变量白名单；文件不存在、属主不符或组/其他用户可写时静默跳过。
void applyFromFile(const QString &path, QProcessEnvironment *environment);
}

#endif // PROXYCONFIG_H
