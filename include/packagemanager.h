#ifndef PACKAGEMANAGER_H
#define PACKAGEMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>

struct DebPackage {
    QString name;
    QString version;
    QString description;
    bool installed;
};

class PackageManager : public QObject
{
    Q_OBJECT

public:
    explicit PackageManager(QObject *parent = nullptr);
    ~PackageManager();

    void loadInstalledPackages();
    bool installPackage(const QString &packagePath);
    bool removePackage(const QString &packageName, bool purge = false);
    bool installMultiplePackages(const QStringList &packagePaths);
    bool removeMultiplePackages(const QStringList &packageNames, bool purge = false);
    bool autoRemove();
    
    // 在线包管理方法
    bool checkAptCache();
    bool updateAptCache();
    bool searchOnlinePackages(const QString &searchTerm);
    bool installOnlinePackages(const QStringList &packageNames);
    bool upgradeSystem(bool distUpgrade);
    
    QString getLastError();
    QList<DebPackage> getCurrentPackages() const { return currentPackages; }
    QList<DebPackage> getOnlineSearchResults() const { return onlineSearchResults; }

signals:
    void operationStarted(const QString &message);
    void operationProgress(const QString &message);
    void operationOutput(const QString &output);
    void operationFinished(bool success, const QString &message);

private:
    DebPackage parseDpkgLine(const QString &line);
    DebPackage parseAptSearchLine(const QString &line);
    void updateInstalledStatus();
    bool checkSystemCommands();
    
    QString m_lastError;
    QList<DebPackage> currentPackages;
    QList<DebPackage> onlineSearchResults;
};

#endif