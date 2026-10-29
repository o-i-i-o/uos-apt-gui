#include "proxyconfig.h"

#include <QFile>
#include <QProcessEnvironment>
#include <QStringList>
#include <sys/stat.h>
#include <unistd.h>

namespace
{
const qint64 kMaxConfigSize = 64 * 1024;
const int kMaxValueLength = 4096;

const QStringList &allowedVariables()
{
    static const QStringList variables{
        QStringLiteral("http_proxy"),
        QStringLiteral("https_proxy"),
        QStringLiteral("ftp_proxy"),
        QStringLiteral("no_proxy"),
        QStringLiteral("HTTP_PROXY"),
        QStringLiteral("HTTPS_PROXY"),
        QStringLiteral("FTP_PROXY"),
        QStringLiteral("NO_PROXY")
    };
    return variables;
}

bool isValidValue(const QString &value)
{
    if (value.isEmpty() || value.size() > kMaxValueLength) {
        return false;
    }
    for (const QChar ch : value) {
        if (ch.unicode() < 0x20 || ch.unicode() == 0x7f) {
            return false;
        }
    }
    return true;
}
}

void ProxyConfig::applyFromFile(const QString &path, QProcessEnvironment *environment)
{
    if (!environment || path.isEmpty()) {
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    // 打开后校验实际文件属性，避免符号链接或授权期间替换绕过检查
    struct stat info;
    if (::fstat(file.handle(), &info) != 0
            || !S_ISREG(info.st_mode)
            || info.st_uid != ::geteuid()
            || (info.st_mode & (S_IWGRP | S_IWOTH)) != 0
            || info.st_size <= 0
            || info.st_size > kMaxConfigSize) {
        return;
    }

    const QStringList lines = QString::fromUtf8(file.readAll()).split(QLatin1Char('\n'));
    for (const QString &rawLine : lines) {
        const QString line = rawLine.trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#'))) {
            continue;
        }

        const int equals = line.indexOf(QLatin1Char('='));
        if (equals <= 0) {
            continue;
        }

        const QString key = line.left(equals).trimmed();
        QString value = line.mid(equals + 1).trimmed();
        if (value.size() >= 2) {
            const QChar quote = value.at(0);
            if ((quote == QLatin1Char('"') || quote == QLatin1Char('\''))
                    && value.at(value.size() - 1) == quote) {
                value = value.mid(1, value.size() - 2);
            }
        }

        if (!allowedVariables().contains(key) || !isValidValue(value)) {
            continue;
        }
        environment->insert(key, value);
    }
}
