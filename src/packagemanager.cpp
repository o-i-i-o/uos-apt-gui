#include "packagemanager.h"
#include <QProcess>
#include <QDir>
#include <QTimer>
#include <QCoreApplication>
#include <QDateTime>

PackageManager::PackageManager(QObject *parent)
    : QObject(parent)
    , m_lastError("")
{
    if (!checkSystemCommands()) {
        m_lastError = "系统命令不可用，请确保apt和pkexec已安装";
    }
}

PackageManager::~PackageManager()
{
}

void PackageManager::loadInstalledPackages()
{
    emit operationStarted("正在加载已安装包列表...");
    
    QProcess process;
    QString command = "dpkg -l";
    
    process.start(command);
    if (!process.waitForFinished(10000)) { // 10秒超时
        m_lastError = "加载包列表超时";
        emit operationFinished(false, m_lastError);
        return;
    }
    
    if (process.exitCode() != 0) {
        m_lastError = process.readAllStandardError();
        emit operationFinished(false, m_lastError);
        return;
    }
    
    QString output = process.readAllStandardOutput();
    QStringList lines = output.split("\n");
    
    currentPackages.clear();
    int count = 0;
    
    for (const QString &line : lines) {
        if (line.startsWith("ii")) { // 只处理已安装的包
            DebPackage pkg = parseDpkgLine(line);
            if (!pkg.name.isEmpty()) {
                currentPackages.append(pkg);
                count++;
                
                // 每处理100个包发送一次进度更新
                if (count % 100 == 0) {
                    emit operationProgress(QString("已加载 %1 个包").arg(count));
                }
            }
        }
    }
    
    emit operationProgress(QString("共加载 %1 个已安装包").arg(currentPackages.size()));
    emit operationFinished(true, QString("成功加载 %1 个已安装包").arg(currentPackages.size()));
}

bool PackageManager::installPackage(const QString &packagePath)
{
    if (!QFile::exists(packagePath)) {
        m_lastError = "文件不存在: " + packagePath;
        emit operationFinished(false, m_lastError);
        return false;
    }
    
    emit operationStarted(QString("正在安装包: %1").arg(QFileInfo(packagePath).fileName()));
    
    // 创建并配置进程
    QProcess *process = new QProcess(this);
    
    // 连接信号
    connect(process, &QProcess::readyReadStandardOutput, this, [this, process]() {
        QString output = process->readAllStandardOutput();
        emit operationOutput(output);
    });
    
    connect(process, &QProcess::readyReadStandardError, this, [this, process]() {
        QString output = process->readAllStandardError();
        emit operationOutput(output);
    });
    
    // 明确指定finished信号的重载版本
    connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), 
            this, [this, process, packagePath](int exitCode, QProcess::ExitStatus exitStatus) {
        QString fileName = QFileInfo(packagePath).fileName();
        if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
            emit operationFinished(true, QString("包安装成功: %1").arg(fileName));
        } else {
            QString error = process->readAllStandardError();
            m_lastError = error.isEmpty() ? "安装失败" : error;
            emit operationFinished(false, m_lastError);
        }
        process->deleteLater();
    });
    
    // 明确指定errorOccurred信号的重载版本
    connect(process, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::errorOccurred), 
            this, [this, process, packagePath](QProcess::ProcessError error) {
        QString fileName = QFileInfo(packagePath).fileName();
        m_lastError = QString("安装过程出错: %1").arg(process->errorString());
        emit operationFinished(false, m_lastError);
        process->deleteLater();
    });
    
    // 使用参数化执行，防止命令注入
    QStringList arguments = {"apt", "install", "-y", packagePath};
    process->start("pkexec", arguments);
    return true;
}

bool PackageManager::removePackage(const QString &packageName, bool purge)
{
    QString operation = purge ? "正在完全卸载包并删除配置: %1" : "正在卸载包: %1";
    emit operationStarted(QString(operation).arg(packageName));
    
    // 创建并配置进程
    QProcess *process = new QProcess(this);
    
    // 连接信号
    connect(process, &QProcess::readyReadStandardOutput, this, [this, process]() {
        QString output = process->readAllStandardOutput();
        emit operationOutput(output);
    });
    
    connect(process, &QProcess::readyReadStandardError, this, [this, process]() {
        QString output = process->readAllStandardError();
        emit operationOutput(output);
    });
    
    // 明确指定finished信号的重载版本
    connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), 
            this, [this, process, packageName, purge](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
            QString successMsg = purge ? "包完全卸载并删除配置成功: %1" : "包卸载成功: %1";
            emit operationFinished(true, QString(successMsg).arg(packageName));
        } else {
            QString error = process->readAllStandardError();
            m_lastError = error.isEmpty() ? "卸载失败" : error;
            emit operationFinished(false, m_lastError);
        }
        process->deleteLater();
    });
    
    // 明确指定errorOccurred信号的重载版本
    connect(process, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::errorOccurred), 
            this, [this, process, packageName](QProcess::ProcessError error) {
        m_lastError = QString("卸载过程出错: %1").arg(process->errorString());
        emit operationFinished(false, m_lastError);
        process->deleteLater();
    });
    
    // 使用参数化执行，防止命令注入
    QStringList arguments;
    arguments << "apt";
    arguments << (purge ? "purge" : "remove");
    arguments << "-y" << packageName;
    process->start("pkexec", arguments);
    return true;
}

bool PackageManager::removeMultiplePackages(const QStringList &packageNames, bool purge)
{
    if (packageNames.isEmpty()) {
        m_lastError = "没有选择任何包";
        emit operationFinished(false, m_lastError);
        return false;
    }
    
    QString operation = purge ? "正在批量完全卸载 %1 个包并删除配置" : "正在批量卸载 %1 个包";
    emit operationStarted(QString(operation).arg(packageNames.size()));
    
    // 创建并配置进程
    QProcess *process = new QProcess(this);
    
    // 连接信号
    connect(process, &QProcess::readyReadStandardOutput, this, [this, process]() {
        QString output = process->readAllStandardOutput();
        emit operationOutput(output);
    });
    
    connect(process, &QProcess::readyReadStandardError, this, [this, process]() {
        QString output = process->readAllStandardError();
        emit operationOutput(output);
    });
    
    // 明确指定finished信号的重载版本
    connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), 
            this, [this, process, packageNames, purge](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
            QString successMsg = purge ? "所有 %1 个包完全卸载并删除配置成功" : "所有 %1 个包卸载成功";
            emit operationFinished(true, QString(successMsg).arg(packageNames.size()));
        } else {
            QString error = process->readAllStandardError();
            m_lastError = error.isEmpty() ? "部分或全部包卸载失败" : error;
            emit operationFinished(false, m_lastError);
        }
        process->deleteLater();
    });
    
    // 明确指定errorOccurred信号的重载版本
    connect(process, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::errorOccurred), 
            this, [this, process, packageNames](QProcess::ProcessError error) {
        m_lastError = QString("卸载过程出错: %1").arg(process->errorString());
        emit operationFinished(false, m_lastError);
        process->deleteLater();
    });
    
    // 使用参数化执行，防止命令注入
    QStringList arguments;
    arguments << "apt";
    arguments << (purge ? "purge" : "remove");
    arguments << "-y";
    arguments << packageNames;
    process->start("pkexec", arguments);
    return true;
}

bool PackageManager::installMultiplePackages(const QStringList &packagePaths)
{
    if (packagePaths.isEmpty()) {
        m_lastError = "没有选择任何包文件";
        emit operationFinished(false, m_lastError);
        return false;
    }
    
    emit operationStarted(QString("正在批量安装 %1 个包").arg(packagePaths.size()));
    
    // 创建并配置进程
    QProcess *process = new QProcess(this);
    
    // 连接信号
    connect(process, &QProcess::readyReadStandardOutput, this, [this, process]() {
        QString output = process->readAllStandardOutput();
        emit operationOutput(output);
    });
    
    connect(process, &QProcess::readyReadStandardError, this, [this, process]() {
        QString output = process->readAllStandardError();
        emit operationOutput(output);
    });
    
    // 明确指定finished信号的重载版本
    connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), 
            this, [this, process, packagePaths](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
            emit operationFinished(true, QString("所有 %1 个包安装成功").arg(packagePaths.size()));
        } else {
            QString error = process->readAllStandardError();
            m_lastError = error.isEmpty() ? "部分或全部包安装失败" : error;
            emit operationFinished(false, m_lastError);
        }
        process->deleteLater();
    });
    
    // 明确指定errorOccurred信号的重载版本
    connect(process, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::errorOccurred), 
            this, [this, process, packagePaths](QProcess::ProcessError error) {
        m_lastError = QString("安装过程出错: %1").arg(process->errorString());
        emit operationFinished(false, m_lastError);
        process->deleteLater();
    });
    
    // 使用参数化执行，防止命令注入
    QStringList arguments;
    arguments << "apt" << "install" << "-y";
    arguments << packagePaths;
    process->start("pkexec", arguments);
    return true;
}

bool PackageManager::autoRemove()
{
    emit operationStarted("正在执行自动清理...");
    
    QString command = "pkexec apt autoremove -y";
    
    // 创建并配置进程
    QProcess *process = new QProcess(this);
    
    // 连接信号
    connect(process, &QProcess::readyReadStandardOutput, this, [this, process]() {
        QString output = process->readAllStandardOutput();
        emit operationOutput(output);
    });
    
    connect(process, &QProcess::readyReadStandardError, this, [this, process]() {
        QString output = process->readAllStandardError();
        emit operationOutput(output);
    });
    
    // 明确指定finished信号的重载版本
    connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), 
            this, [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
            emit operationFinished(true, "自动清理完成");
        } else {
            QString error = process->readAllStandardError();
            m_lastError = error.isEmpty() ? "自动清理失败" : error;
            emit operationFinished(false, m_lastError);
        }
        process->deleteLater();
    });
    
    // 明确指定errorOccurred信号的重载版本
    connect(process, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::errorOccurred), 
            this, [this, process](QProcess::ProcessError error) {
        m_lastError = QString("自动清理过程出错: %1").arg(process->errorString());
        emit operationFinished(false, m_lastError);
        process->deleteLater();
    });
    
    // 启动进程
    process->start(command);
    return true;
}

bool PackageManager::checkAptCache()
{
    // 检查apt缓存是否存在，通过查看更新时间
    QProcess process;
    QString command = "stat -c %Y /var/cache/apt/pkgcache.bin";
    
    process.start(command);
    if (!process.waitForFinished(5000)) {
        m_lastError = "检查apt缓存超时";
        return false;
    }
    
    QString output = process.readAllStandardOutput().trimmed();
    if (output.isEmpty()) {
        // 缓存文件不存在
        return false;
    }
    
    // 检查缓存是否超过24小时
    qint64 cacheTime = output.toLongLong();
    qint64 currentTime = QDateTime::currentSecsSinceEpoch();
    
    return (currentTime - cacheTime) < 86400; // 24小时
}

bool PackageManager::updateAptCache()
{
    emit operationStarted("正在更新apt缓存...");
    
    QString command = "pkexec apt update";
    
    // 创建并配置进程
    QProcess *process = new QProcess(this);
    
    // 连接信号
    connect(process, &QProcess::readyReadStandardOutput, this, [this, process]() {
        QString output = process->readAllStandardOutput();
        emit operationOutput(output);
    });
    
    connect(process, &QProcess::readyReadStandardError, this, [this, process]() {
        QString output = process->readAllStandardError();
        emit operationOutput(output);
    });
    
    // 明确指定finished信号的重载版本
    connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), 
            this, [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
            emit operationFinished(true, "apt缓存更新完成");
        } else {
            QString error = process->readAllStandardError();
            m_lastError = error.isEmpty() ? "更新apt缓存失败" : error;
            emit operationFinished(false, m_lastError);
        }
        process->deleteLater();
    });
    
    // 明确指定errorOccurred信号的重载版本
    connect(process, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::errorOccurred), 
            this, [this, process](QProcess::ProcessError error) {
        m_lastError = QString("更新apt缓存过程出错: %1").arg(process->errorString());
        emit operationFinished(false, m_lastError);
        process->deleteLater();
    });
    
    // 启动进程
    process->start(command);
    return true;
}

bool PackageManager::searchOnlinePackages(const QString &searchTerm)
{
    emit operationStarted(QString("正在搜索在线包: %1").arg(searchTerm));
    
    // 搜索操作不需要root权限，也不需要实时输出，保持同步执行
    QProcess process;
    
    // 使用参数化执行，防止命令注入
    QStringList arguments;
    arguments << "search" << searchTerm;
    process.start("apt", arguments);
    
    if (!process.waitForFinished(-1)) {
        m_lastError = "搜索包超时";
        emit operationFinished(false, m_lastError);
        return false;
    }
    
    QString output = process.readAllStandardOutput();
    QString error = process.readAllStandardError();
    
    if (process.exitCode() != 0) {
        m_lastError = error.isEmpty() ? "搜索包失败" : error;
        emit operationFinished(false, m_lastError);
        return false;
    }
    
    QStringList lines = output.split("\n");
    onlineSearchResults.clear();
    
    // 跳过警告信息和头信息，直到找到包信息行
    bool foundPackages = false;
    DebPackage currentPkg;
    bool hasCurrentPackage = false;
    
    for (int i = 0; i < lines.size(); ++i) {
        const QString &line = lines[i];
        QString trimmedLine = line.trimmed();
        if (trimmedLine.isEmpty()) {
            // 空行表示一个包信息的结束，如果有未添加的包，现在添加
            if (hasCurrentPackage && !currentPkg.name.isEmpty()) {
                // 检查包名或描述是否包含搜索关键词，确保只显示相关包
                if (currentPkg.name.contains(searchTerm, Qt::CaseInsensitive) || 
                    currentPkg.description.contains(searchTerm, Qt::CaseInsensitive)) {
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
        
        // 开始解析包信息行
        if (foundPackages) {
            // 检查是否是描述行（以空格开头）
            if (line.startsWith("    ")) {
                // 这是描述行，添加到当前包
                if (hasCurrentPackage) {
                    if (currentPkg.description.isEmpty()) {
                        currentPkg.description = trimmedLine;
                    } else {
                        currentPkg.description += " " + trimmedLine;
                    }
                }
            } else {
                // 这是包信息行，先处理之前的包
                if (hasCurrentPackage && !currentPkg.name.isEmpty()) {
                    // 检查包名或描述是否包含搜索关键词，确保只显示相关包
                    if (currentPkg.name.contains(searchTerm, Qt::CaseInsensitive) || 
                        currentPkg.description.contains(searchTerm, Qt::CaseInsensitive)) {
                        onlineSearchResults.append(currentPkg);
                    }
                }
                // 开始解析新包
                currentPkg = parseAptSearchLine(line);
                hasCurrentPackage = !currentPkg.name.isEmpty();
            }
        }
    }
    
    // 处理最后一个包
    if (hasCurrentPackage && !currentPkg.name.isEmpty()) {
        // 检查包名或描述是否包含搜索关键词，确保只显示相关包
        if (currentPkg.name.contains(searchTerm, Qt::CaseInsensitive) || 
            currentPkg.description.contains(searchTerm, Qt::CaseInsensitive)) {
            onlineSearchResults.append(currentPkg);
        }
    }
    
    // 更新已安装状态
    updateInstalledStatus();
    
    emit operationFinished(true, QString("找到 %1 个匹配的包").arg(onlineSearchResults.size()));
    return true;
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
    
    emit operationStarted(QString("正在安装 %1 个包").arg(packageNames.size()));
    
    // 创建并配置进程
    QProcess *process = new QProcess(this);
    
    // 连接信号
    connect(process, &QProcess::readyReadStandardOutput, this, [this, process]() {
        QString output = process->readAllStandardOutput();
        emit operationOutput(output);
    });
    
    connect(process, &QProcess::readyReadStandardError, this, [this, process]() {
        QString output = process->readAllStandardError();
        emit operationOutput(output);
    });
    
    // 明确指定finished信号的重载版本
    connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), 
            this, [this, process, packageNames](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
            emit operationFinished(true, QString("所有 %1 个包安装成功").arg(packageNames.size()));
        } else {
            QString error = process->readAllStandardError();
            m_lastError = error.isEmpty() ? "部分或全部包安装失败" : error;
            emit operationFinished(false, m_lastError);
        }
        process->deleteLater();
    });
    
    // 明确指定errorOccurred信号的重载版本
    connect(process, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::errorOccurred), 
            this, [this, process, packageNames](QProcess::ProcessError error) {
        m_lastError = QString("安装过程出错: %1").arg(process->errorString());
        emit operationFinished(false, m_lastError);
        process->deleteLater();
    });
    
    // 使用参数化执行，防止命令注入
    QStringList arguments;
    arguments << "apt" << "install" << "-y";
    arguments << packageNames;
    process->start("pkexec", arguments);
    return true;
}

bool PackageManager::upgradeSystem(bool distUpgrade)
{
    emit operationStarted(distUpgrade ? "正在执行系统完全更新..." : "正在执行系统更新...");
    
    QString command;
    if (distUpgrade) {
        command = "pkexec apt dist-upgrade -y";
    } else {
        command = "pkexec apt upgrade -y";
    }
    
    // 创建并配置进程
    QProcess *process = new QProcess(this);
    
    // 连接信号
    connect(process, &QProcess::readyReadStandardOutput, this, [this, process]() {
        QString output = process->readAllStandardOutput();
        emit operationOutput(output);
    });
    
    connect(process, &QProcess::readyReadStandardError, this, [this, process]() {
        QString output = process->readAllStandardError();
        emit operationOutput(output);
    });
    
    // 明确指定finished信号的重载版本
    connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), 
            this, [this, process, distUpgrade](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
            emit operationFinished(true, distUpgrade ? "系统完全更新完成" : "系统更新完成");
        } else {
            QString error = process->readAllStandardError();
            m_lastError = error.isEmpty() ? "系统更新失败" : error;
            emit operationFinished(false, m_lastError);
        }
        process->deleteLater();
    });
    
    // 明确指定errorOccurred信号的重载版本
    connect(process, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::errorOccurred), 
            this, [this, process](QProcess::ProcessError error) {
        m_lastError = QString("系统更新过程出错: %1").arg(process->errorString());
        emit operationFinished(false, m_lastError);
        process->deleteLater();
    });
    
    // 启动进程
    process->start(command);
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
    
    QString trimmedLine = line.trimmed();
    if (trimmedLine.isEmpty()) {
        return pkg;
    }
    
    // 检查是否是描述行（以空格开头）
    if (line.startsWith("    ")) {
        // 这是描述行，暂时不处理
        return pkg;
    }
    
    // 解析包信息行
    QStringList parts = trimmedLine.split(QRegExp("\\s+"), QString::SkipEmptyParts);
    
    if (parts.size() >= 2) {
        // 包名包含/，需要提取
        QString pkgFullName = parts[0];
        int slashIndex = pkgFullName.indexOf("/");
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
    QStringList parts = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
    
    if (parts.size() >= 4) {
        pkg.name = parts[1];
        pkg.version = parts[2];
        
        // 描述可能包含空格，所以需要合并剩余部分
        QStringList descParts;
        for (int i = 3; i < parts.size(); ++i) {
            descParts.append(parts[i]);
        }
        pkg.description = descParts.join(" ");
    }
    
    return pkg;
}

bool PackageManager::checkSystemCommands()
{
    // 检查apt命令是否可用
    QProcess process;
    process.start("which apt");
    process.waitForFinished();
    
    if (process.exitCode() != 0) {
        m_lastError = "apt命令不可用";
        return false;
    }
    
    // 检查pkexec命令是否可用
    process.start("which pkexec");
    process.waitForFinished();
    
    if (process.exitCode() != 0) {
        m_lastError = "pkexec命令不可用";
        return false;
    }
    
    return true;
}

