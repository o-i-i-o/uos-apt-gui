#ifndef APTBACKEND_H
#define APTBACKEND_H

#include <QObject>
#include <QProcess>
#include <QStringList>

class AptBackend : public QObject
{
    Q_OBJECT

public:
    explicit AptBackend(QObject *parent = nullptr);

    void setDryRun(bool dryRun);
    void setStagingDir(const QString &dir);
    bool isBusy() const;
    QString lastError() const { return m_lastError; }
    void clearLastError() { m_lastError.clear(); }

    QString installLocalPackages(const QStringList &paths);
    QString installPackages(const QStringList &names);
    QString removePackages(const QStringList &names, bool purge);
    QString updateCache();
    QString upgrade(bool distUpgrade);
    QString autoRemove();

signals:
    void output(const QString &transaction, const QString &text);
    void finished(const QString &transaction, int exitCode, const QString &message);

private:
    QString begin(const QString &program, const QStringList &arguments);
    bool ensureStagingDir();
    bool stageLocalPackages(const QStringList &paths, QStringList *staged);
    void clearStagingDirectory();
    void cleanupStagedFiles();
    void readStandardOutput();
    void readStandardError();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);
    void finish(const QString &transaction, int exitCode, const QString &message);

    QProcess *m_process = nullptr;
    QString m_transaction;
    QString m_stagingDir = QStringLiteral("/var/lib/apt-gui/staging");
    QStringList m_stagedFiles;
    QString m_lastError;
    int m_counter = 0;
    bool m_dryRun = false;
};

#endif // APTBACKEND_H
