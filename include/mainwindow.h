#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QProgressBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QTabWidget>
#include <QCheckBox>
#include <QGroupBox>
#include "packagemanager.h"
#include "operationprogressdialog.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void refreshPackageList();
    void searchLocalPackages();
    void removeSelectedPackages();
    void installLocalPackages();
    void autoRemovePackages();
    void onOperationStarted(const QString &message);
    void onOperationProgress(const QString &message);
    void onOperationFinished(bool success, const QString &message);
    void onPackageSelectionChanged();
    void onItemChanged(QTableWidgetItem *item);
    QStringList getSelectedPackages();
    
    // 在线包管理槽函数
    void searchOnlinePackages();
    void installSelectedOnlinePackages();
    void updateAptCache();
    void upgradeSystem();
    void onOnlinePackageSelectionChanged();
    void onOnlineItemChanged(QTableWidgetItem *item);
    QStringList getSelectedOnlinePackages();
    void onTabChanged(int index);

private:
    void setupUI();
    void setupConnections();
    void populatePackageTable(const QList<DebPackage> &packages);
    void showStatusMessage(const QString &message, int timeout = 5000);
    
    QTabWidget *tabWidget;
    QTableWidget *packageTable;
    QLineEdit *searchEdit;
    QPushButton *searchButton;
    QPushButton *refreshButton;
    QPushButton *removeButton;
    QPushButton *batchInstallButton;
    QPushButton *autoRemoveButton;
    QCheckBox *purgeConfigCheckbox;
    QLabel *statusLabel;
    QProgressBar *progressBar;
    
    // 在线包管理组件
    QLineEdit *onlineSearchEdit;
    QPushButton *onlineSearchButton;
    QPushButton *refreshAptCacheButton;
    QPushButton *installOnlineButton;
    QPushButton *upgradeSystemButton;
    QCheckBox *distUpgradeCheckbox;
    QTableWidget *onlinePackageTable;
    QList<DebPackage> currentOnlinePackages;
    
    // 操作进度对话框
    OperationProgressDialog *progressDialog;
    
    PackageManager *packageManager;
    QList<DebPackage> currentPackages;
};

#endif