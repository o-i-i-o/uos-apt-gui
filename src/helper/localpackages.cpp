#include "localpackages.h"
#include <QFile>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace
{
// 单个与单次事务的暂存上限，避免被用来耗尽磁盘
const quint64 kMaxSingleDebBytes = 4ull * 1024 * 1024 * 1024;
const quint64 kMaxTotalDebBytes = 8ull * 1024 * 1024 * 1024;

QString systemError()
{
    return QString::fromLocal8Bit(std::strerror(errno));
}
}

LocalPackageSet::LocalPackageSet() = default;

LocalPackageSet::~LocalPackageSet()
{
    closeAll();
}

void LocalPackageSet::closeAll()
{
    for (File &file : m_files) {
        if (file.fd >= 0) {
            ::close(file.fd);
            file.fd = -1;
        }
    }
    m_files.clear();
}

bool LocalPackageSet::open(const QStringList &paths, QString *error)
{
    closeAll();

    quint64 totalBytes = 0;
    for (const QString &path : paths) {
        // O_NONBLOCK 只影响非常规文件，避免被换成 FIFO 时阻塞事件循环
        const int fd = ::open(QFile::encodeName(path).constData(),
                              O_RDONLY | O_NOFOLLOW | O_CLOEXEC | O_NONBLOCK);
        if (fd < 0) {
            *error = QStringLiteral("无法打开 %1：%2").arg(path, systemError());
            closeAll();
            return false;
        }

        struct stat info;
        QString message;
        if (::fstat(fd, &info) != 0) {
            message = QStringLiteral("无法读取 %1 的信息：%2").arg(path, systemError());
        } else if (!S_ISREG(info.st_mode)) {
            message = QStringLiteral("%1 不是普通文件").arg(path);
        } else if ((info.st_mode & (S_IWGRP | S_IWOTH)) != 0) {
            message = QStringLiteral("%1 允许其他用户写入，已拒绝安装").arg(path);
        } else if (info.st_size <= 0) {
            message = QStringLiteral("%1 是空文件").arg(path);
        } else if (quint64(info.st_size) > kMaxSingleDebBytes) {
            message = QStringLiteral("%1 超过单个安装包大小上限").arg(path);
        } else if (totalBytes + quint64(info.st_size) > kMaxTotalDebBytes) {
            message = QStringLiteral("所选安装包总大小超过上限");
        }
        if (!message.isEmpty()) {
            ::close(fd);
            *error = message;
            closeAll();
            return false;
        }

        File file;
        file.path = path;
        file.fd = fd;
        file.device = quint64(info.st_dev);
        file.inode = quint64(info.st_ino);
        file.size = quint64(info.st_size);
        m_files.append(file);
        totalBytes += file.size;
    }
    return true;
}
