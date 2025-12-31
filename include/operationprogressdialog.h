#ifndef OPERATIONPROGRESSDIALOG_H
#define OPERATIONPROGRESSDIALOG_H

#include <QDialog>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCloseEvent>

class OperationProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OperationProgressDialog(QWidget *parent = nullptr);
    ~OperationProgressDialog();

    void setOperationName(const QString &name);
    void appendOutput(const QString &output);
    void setFinished(bool success, const QString &message);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    QLabel *operationLabel;
    QTextEdit *outputTextEdit;
    QPushButton *closeButton;
    bool isFinished;
    bool isSuccess;
    QString resultMessage;
};

#endif // OPERATIONPROGRESSDIALOG_H