#include "packagemanager.h"
#include "helperclient.h"
#include <QDateTime>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QTimer>

PackageManager::PackageManager(QObject *parent)
    : QObject(parent)
    , m_lastError("")
    , m_helper(new HelperClient(this))
{
    connect(m_helper, &HelperClient::output, this,
            [this](const QString &, const QString &text) {
        emit operationOutput(text);
    });
    connect(m_helper, &HelperClient::finished, this,
            [this](const QString &, int exitCode, const QString &message) {
        const QString successMessage = m_pendingSuccessMessage;
        m_pendingSuccessMessage.clear();

        if (exitCode == 0) {
            emit operationFinished(true, successMessage.isEmpty()
                                   ? QStringLiteral("操作完成") : successMessage);
        } else {
            m_lastError = message;
            emit operationFinished(false, message);
        }
    });
    connect(m_helper, &HelperClient::failed, this,
            [this](const QString &message) {
        m_pendingSuccessMessage.clear();
        m_lastError = message;
        emit operationFinished(false, message);
    });

    if (!checkSystemCommands()) {
        m_lastError = "系统命令不可用，请确保 apt 和 dpkg 已安装";
    }
}

PackageManager::~PackageManager()
{
}

void PackageManager::loadInstalledPackages()
{
    if (m_listProcess) {
        return;
    }

    emit operationStarted(QStringLiteral("正在加载已安装包列表..."));

    QProcess *process = new QProcess(this);
    m_listProcess = process;

    QTimer *timeout = new QTimer(process);
    timeout->setSingleShot(true);
    connect(timeout, &QTimer::timeout, process, [process]() {
        if (process->state() != QProcess::NotRunning) {
            process->setProperty("timedOut", true);
            process->kill();
        }
    });
    timeout->start(10000);

    connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
        if (m_listProcess != process) {
            process->deleteLater();
            return;
        }
        m_listProcess = nullptr;

        const QByteArray errorOutput = process->readAllStandardError();
        const QByteArray output = process->readAllStandardOutput();
        const bool timedOut = process->property("timedOut").toBool();
        process->deleteLater();

        if (timedOut) {
            m_lastError = QStringLiteral("加载包列表超时");
            emit operationFinished(false, m_lastError);
            return;
        }
        if (exitCode != 0 || exitStatus != QProcess::NormalExit) {
            m_lastError = QString::fromLocal8Bit(errorOutput).trimmed();
            if (m_lastError.isEmpty()) {
                m_lastError = QStringLiteral("加载包列表失败");
            }
            emit operationFinished(false, m_lastError);
            return;
        }

        parseInstalledPackages(QString::fromUtf8(output));
    });

    connect(process, &QProcess::errorOccurred, this,
            [this, process](QProcess::ProcessError error) {
        if (error != QProcess::FailedToStart || m_listProcess != process) {
            return;
        }
        m_listProcess = nullptr;
        m_lastError = process->errorString();
        process->deleteLater();
        emit operationFinished(false, m_lastError);
    });

    process->start(QStringLiteral("dpkg"), QStringList{QStringLiteral("-l")});
}

void PackageManager::parseInstalledPackages(const QString &output)
{
    currentPackages.clear();

    int count = 0;
    const QStringList lines = output.split(QLatin1Char('\n'));
    for (const QString &line : lines) {
        if (!line.startsWith(QLatin1String("ii"))) { // 只处理已安装的包
            continue;
        }

        const DebPackage pkg = parseDpkgLine(line);
        if (pkg.name.isEmpty()) {
            continue;
        }

        currentPackages.append(pkg);
        ++count;

        // 每处理100个包发送一次进度更新
        if (count % 100 == 0) {
            emit operationProgress(QStringLiteral("已加载 %1 个包").arg(count));
        }
    }

    emit operationProgress(QStringLiteral("共加载 %1 个已安装包").arg(currentPackages.size()));
    emit operationFinished(true, QStringLiteral("成功加载 %1 个已安装包").arg(currentPackages.size()));
}

bool PackageManager::installPackage(const QString &packagePath)
{
    return installMultiplePackages(QStringList{packagePath});
}

bool PackageManager::removePackage(const QString &packageName, bool purge)
{
    if (packageName.isEmpty()) {
        m_lastError = "没有选择任何包";
        emit operationFinished(false, m_lastError);
        return false;
    }

    emit operationStarted(purge
                          ? QStringLiteral("正在完全卸载包并删除配置: %1").arg(packageName)
                          : QStringLiteral("正在卸载包: %1").arg(packageName));
    m_pendingSuccessMessage = purge
            ? QStringLiteral("包完全卸载并删除配置成功: %1").arg(packageName)
            : QStringLiteral("包卸载成功: %1").arg(packageName);

    m_helper->removePackages(QStringList{packageName}, purge);
    return true;
}

bool PackageManager::removeMultiplePackages(const QStringList &packageNames, bool purge)
{
    if (packageNames.isEmpty()) {
        m_lastError = "没有选择任何包";
        emit operationFinished(false, m_lastError);
        return false;
    }

    emit operationStarted(purge
                          ? QStringLiteral("正在批量完全卸载 %1 个包并删除配置").arg(packageNames.size())
                          : QStringLiteral("正在批量卸载 %1 个包").arg(packageNames.size()));
    m_pendingSuccessMessage = purge
            ? QStringLiteral("所有 %1 个包完全卸载并删除配置成功").arg(packageNames.size())
            : QStringLiteral("所有 %1 个包卸载成功").arg(packageNames.size());

    m_helper->removePackages(packageNames, purge);
    return true;
}

bool PackageManager::installMultiplePackages(const QStringList &packagePaths)
{
    if (packagePaths.isEmpty()) {
        m_lastError = "没有选择任何包文件";
        emit operationFinished(false, m_lastError);
        return false;
    }

    emit operationStarted(QStringLiteral("正在批量安装 %1 个包").arg(packagePaths.size()));
    m_pendingSuccessMessage = QStringLiteral("所有 %1 个包安装成功").arg(packagePaths.size());

    m_helper->installLocalPackages(packagePaths);
    return true;
}

bool PackageManager::autoRemove()
{
    emit operationStarted(QStringLiteral("正在执行自动清理..."));
    m_pendingSuccessMessage = QStringLiteral("自动清理完成");

    m_helper->autoRemove();
    return true;
}

bool PackageManager::checkAptCache()
{
    // 检查apt缓存是否存在，通过查看更新时间
    const QFileInfo cache(QStringLiteral("/var/cache/apt/pkgcache.bin"));
    if (!cache.exists()) {
        return false;
    }

    const qint64 cacheTime = cache.lastModified().toSecsSinceEpoch();
    if (cacheTime <= 0) {
        return false;
    }

    return (QDateTime::currentSecsSinceEpoch() - cacheTime) < 86400; // 24小时
}

bool PackageManager::updateAptCache()
{
    emit operationStarted(QStringLiteral("正在更新apt缓存..."));
    m_pendingSuccessMessage = QStringLiteral("apt缓存更新完成");

    m_helper->updateCache();
    return true;
}

bool PackageManager::searchOnlinePackages(const QString &searchTerm)
{
    if (searchTerm.isEmpty()) {
        m_lastError = "请输入搜索关键词";
        emit operationFinished(false, m_lastError);
        return false;
    }
    if (m_searchProcess) {
        m_lastError = "已有搜索正在进行，请稍候";
        emit operationFinished(false, m_lastError);
        return false;
    }

    emit operationStarted(QStringLiteral("正在搜索在线包: %1").arg(searchTerm));

    QProcess *process = new QProcess(this);
    m_searchProcess = process;

    connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, [this, process, searchTerm](int exitCode, QProcess::ExitStatus exitStatus) {
        if (m_searchProcess != process) {
            process->deleteLater();
            return;
        }
        m_searchProcess = nullptr;

        const QByteArray errorOutput = process->readAllStandardError();
        const QByteArray output = process->readAllStandardOutput();
        process->deleteLater();

        if (exitCode != 0 || exitStatus != QProcess::NormalExit) {
            m_lastError = QString::fromLocal8Bit(errorOutput).trimmed();
            if (m_lastError.isEmpty()) {
                m_lastError = QStringLiteral("搜索包失败");
            }
            emit operationFinished(false, m_lastError);
            return;
        }

        parseAptSearchOutput(QString::fromUtf8(output), searchTerm);
    });

    connect(process, &QProcess::errorOccurred, this,
            [this, process](QProcess::ProcessError error) {
        if (error != QProcess::FailedToStart || m_searchProcess != process) {
            return;
        }
        m_searchProcess = nullptr;
        m_lastError = process->errorString();
        process->deleteLater();
        emit operationFinished(false, m_lastError);
    });

    process->start(QStringLiteral("apt"), QStringList{QStringLiteral("search"), searchTerm});
    return true;
}

void PackageManager::parseAptSearchOutput(const QString &output, const QString &searchTerm)
{
    onlineSearchResults.clear();

    const QStringList lines = output.split(QLatin1Char('\n'));

    // 跳过警告信息和头信息，直到找到包信息行
    bool foundPackages = false;
    DebPackage currentPkg;
    bool hasCurrentPackage = false;

    for (int i = 0; i < lines.size(); ++i) {
        const QString &line = lines[i];
        const QString trimmedLine = line.trimmed();
        if (trimmedLine.isEmpty()) {
            // 空行表示一个包信息的结束，如果有未添加的包，现在添加
            if (hasCurrentPackage && !currentPkg.name.isEmpty()) {
                if (currentPkg.name.contains(searchTerm, Qt::CaseInsensitive)
                    || currentPkg.description.contains(searchTerm, Qt::CaseInsensitive)) {
                    onlineSearchResults.append(currentPkg);
                }
                hasCurrentPackage = false;
            }
            continue;
        }

        // 跳过警告行
        if (trimmedLine.contains("WARNING") || trimmedLine.contains("apt does not have a stable CLI interface")) {
            continue;
        }

        // 跳过排序和搜索提示
        if (trimmedLine.contains("正在排序") || trimmedLine.contains("全文搜索")) {
            foundPackages = true;
            continue;
        }

        if (!foundPackages) {
            continue;
        }

        // 检查是否是描述行（以空格开头）
        if (line.startsWith(QLatin1String("    "))) {
            if (hasCurrentPackage) {
                if (currentPkg.description.isEmpty()) {
                    currentPkg.description = trimmedLine;
                } else {
                    currentPkg.description += QLatin1Char(' ') + trimmedLine;
                }
            }
            continue;
        }

        // 包信息行：先处理上一个包，再开始解析新包
        if (hasCurrentPackage && !currentPkg.name.isEmpty()) {
            if (currentPkg.name.contains(searchTerm, Qt::CaseInsensitive)
                || currentPkg.description.contains(searchTerm, Qt::CaseInsensitive)) {
                onlineSearchResults.append(currentPkg);
            }
        }
        currentPkg = parseAptSearchLine(line);
        hasCurrentPackage = !currentPkg.name.isEmpty();
    }

    // 处理最后一个包
    if (hasCurrentPackage && !currentPkg.name.isEmpty()) {
        if (currentPkg.name.contains(searchTerm, Qt::CaseInsensitive)
            || currentPkg.description.contains(searchTerm, Qt::CaseInsensitive)) {
            onlineSearchResults.append(currentPkg);
        }
    }

    updateInstalledStatus();

    emit operationFinished(true, QStringLiteral("找到 %1 个匹配的包").arg(onlineSearchResults.size()));
}

void PackageManager::updateInstalledStatus()
{
    // 获取所有已安装包的名称
    QStringList installedPackageNames;
    for (const DebPackage &pkg : currentPackages) {
        installedPackageNames.append(pkg.name);
    }

    // 更新搜索结果中每个包的已安装状态
    for (int i = 0; i < onlineSearchResults.size(); ++i) {
        DebPackage &pkg = onlineSearchResults[i];
        pkg.installed = installedPackageNames.contains(pkg.name);
    }
}

bool PackageManager::installOnlinePackages(const QStringList &packageNames)
{
    if (packageNames.isEmpty()) {
        m_lastError = "没有选择任何包";
        emit operationFinished(false, m_lastError);
        return false;
    }

    emit operationStarted(QStringLiteral("正在安装 %1 个包").arg(packageNames.size()));
    m_pendingSuccessMessage = QStringLiteral("所有 %1 个包安装成功").arg(packageNames.size());

    m_helper->installPackages(packageNames);
    return true;
}

bool PackageManager::upgradeSystem(bool distUpgrade)
{
    emit operationStarted(distUpgrade ? QStringLiteral("正在执行系统完全更新...")
                                      : QStringLiteral("正在执行系统更新..."));
    m_pendingSuccessMessage = distUpgrade ? QStringLiteral("系统完全更新完成")
                                          : QStringLiteral("系统更新完成");

    m_helper->upgrade(distUpgrade);
    return true;
}

DebPackage PackageManager::parseAptSearchLine(const QString &line)
{
    DebPackage pkg;
    pkg.installed = false;
    pkg.version = "";

    // apt search 实际输出格式示例:
    // amule-gnome-support/未知,未知 1:2.3.2-5 all
    //   ed2k links handling support for GNOME web browsers

    const QString trimmedLine = line.trimmed();
    if (trimmedLine.isEmpty()) {
        return pkg;
    }

    // 检查是否是描述行（以空格开头）
    if (line.startsWith(QLatin1String("    "))) {
        return pkg;
    }

    // 解析包信息行
    const QStringList parts = trimmedLine.split(QRegExp("\\s+"), QString::SkipEmptyParts);

    if (parts.size() >= 2) {
        // 包名包含/，需要提取
        const QString pkgFullName = parts[0];
        const int slashIndex = pkgFullName.indexOf(QLatin1Char('/'));
        if (slashIndex != -1) {
            pkg.name = pkgFullName.left(slashIndex);
        } else {
            pkg.name = pkgFullName;
        }

        // 版本信息在第二部分
        pkg.version = parts[1];

        // 描述暂时设为空，后续可以改进为读取下一行
        pkg.description = "";
    }

    return pkg;
}

QString PackageManager::getLastError()
{
    return m_lastError;
}

DebPackage PackageManager::parseDpkgLine(const QString &line)
{
    DebPackage pkg;
    pkg.installed = true;

    // dpkg -l 输出格式: ii  package-name  version  description
    const QStringList parts = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);

    if (parts.size() >= 4) {
        pkg.name = parts[1];
        pkg.version = parts[2];

        // 描述可能包含空格，所以需要合并剩余部分
        QStringList descParts;
        for (int i = 3; i < parts.size(); ++i) {
            descParts.append(parts[i]);
        }
        pkg.description = descParts.join(QLatin1Char(' '));
    }

    return pkg;
}

bool PackageManager::checkSystemCommands()
{
    if (QStandardPaths::findExecutable(QStringLiteral("apt")).isEmpty()) {
        m_lastError = "apt命令不可用";
        return false;
    }

    if (QStandardPaths::findExecutable(QStringLiteral("dpkg")).isEmpty()) {
        m_lastError = "dpkg命令不可用";
        return false;
    }

    return true;
}
