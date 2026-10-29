#ifndef LOCALPACKAGES_H
#define LOCALPACKAGES_H

#include <QString>
#include <QStringList>
#include <QVector>

// 授权前打开并持有的本地 .deb 描述符，使后续暂存绑定到已校验的 inode，
// 而不是授权结束后重新按路径打开。
class LocalPackageSet
{
public:
    struct File
    {
        QString path;
        int fd = -1;
        quint64 device = 0;
        quint64 inode = 0;
        quint64 size = 0;
    };

    LocalPackageSet();
    ~LocalPackageSet();
    LocalPackageSet(const LocalPackageSet &) = delete;
    LocalPackageSet &operator=(const LocalPackageSet &) = delete;

    // 路径需已通过 PackageArgs::validateLocalDebPaths 规范化；失败时 error 为中文原因
    bool open(const QStringList &paths, QString *error);
    const QVector<File> &files() const { return m_files; }

private:
    void closeAll();

    QVector<File> m_files;
};

#endif // LOCALPACKAGES_H
