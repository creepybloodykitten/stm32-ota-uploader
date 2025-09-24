#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QMessageBox>
#include <QCloseEvent>
#include <QFileDialog>
#include <QRadioButton>

#include <libssh/libssh.h>
#include "sshcontrol.h"

struct BoardInfo {
    QString name;
    QString pin;
};

class QComboBox;
class QPushButton;
class QPlainTextEdit;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void set_username(const QString &name);
    void set_ip(const QString &ip);
    void set_password(const QString &password);


private slots:
    void onAddButtonClicked();
    void onDeleteButtonClicked();
    void onConfigureButtonClicked();
    void onUpdFirmwareButtonClicked();
    void onUsbUsageClicked();

    void handleSshConnected();
    void handleSshDisconnected();
    void handleSshError(const QString &errorMessage);
    void handleSshData(const QString &data);
    void handleLogMessage(const QString &message);
    void handleFlashingFinished(bool success);

    void onBoardSelectionChanged(int index);

private:
    void closeEvent(QCloseEvent *event) override;

    QComboBox *boardComboBox;
    QPushButton *addBoardButton;
    QPushButton *configureBoardButton;
    QPushButton *updateFirmwareButton;
    QPlainTextEdit *logOutput;
    QPushButton *deleteBoardButton;
    QRadioButton *usb_usage;

    ssh_session my_ssh_session;
    QThread *m_sshThread;
    SshControl *m_sshControl;

    std::vector<BoardInfo> m_boards;
};
#endif // MAINWINDOW_H
