#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QMessageBox>
#include <QCloseEvent>

#include <libssh/libssh.h>
#include "sshcontrol.h"


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


    // Слоты для обработки сигналов от воркера
    void handleSshConnected();
    void handleSshDisconnected();
    void handleSshError(const QString &errorMessage);
    void handleSshData(const QString &data);

private:
    void closeEvent(QCloseEvent *event) override;

    QComboBox *boardComboBox;
    QPushButton *addBoardButton;
    QPushButton *configureBoardButton;
    QPushButton *updateFirmwareButton;
    QPlainTextEdit *logOutput;
    QPushButton *deleteBoardButton;

    ssh_session my_ssh_session;
    QThread *m_sshThread; // Поток
    SshControl *m_sshControl; // Воркер
};
#endif // MAINWINDOW_H
