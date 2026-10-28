#ifndef HELPERCLIENT_H
#define HELPERCLIENT_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>

class HelperClient : public QObject
{
    Q_OBJECT

public:
    explicit HelperClient(QObject *parent = nullptr);

    void installLocalPackages(const QStringList &paths);
    void installPackages(const QStringList &names);
    void removePackages(const QStringList &names, bool purge);
    void updateCache();
    void upgrade(bool distUpgrade);
    void autoRemove();

signals:
    // 事务已被助手接受；此后 Output/Finished 才会携带该 ID
    void accepted(const QString &transaction);
    // 请求被拒绝（未授权、忙、参数非法、服务不可用等），message 为可直接展示的文本
    void failed(const QString &message);
    void output(const QString &transaction, const QString &text);
    void finished(const QString &transaction, int exitCode, const QString &message);

private slots:
    void onRemoteOutput(const QString &transaction, const QString &text);
    void onRemoteFinished(const QString &transaction, int exitCode, const QString &message);

private:
    void call(const QString &method, const QVariantList &arguments);

    QString m_transaction;
};

#endif // HELPERCLIENT_H
