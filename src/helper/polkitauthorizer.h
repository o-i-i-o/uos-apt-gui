#ifndef POLKITAUTHORIZER_H
#define POLKITAUTHORIZER_H

#include <QObject>
#include <QString>
#include <functional>

class PolkitAuthorizer : public QObject
{
public:
    explicit PolkitAuthorizer(QObject *parent = nullptr);

    // 异步校验；回调恰好调用一次，error 为中文原因
    void check(const QString &actionId, quint32 pid, quint64 startTime,
               const std::function<void(bool authorized, const QString &error)> &callback);
};

#endif // POLKITAUTHORIZER_H
