#ifndef SSHCONTROL_H
#define SSHCONTROL_H

#include <QObject>
#include <QDebug>
#include <QString>
#include <QFileInfo>
#include <QFile>
#include <QTimer>

#include <libssh/libssh.h>
#include <libssh/sftp.h>

#ifdef _WIN32
#include <fcntl.h> // Для O_WRONLY, O_CREAT, O_TRUNC в Windows (MSVC, MinGW)
#else
#include <sys/types.h> // Для систем POSIX (Linux, macOS)
#include <sys/stat.h>
#include <fcntl.h>
#endif

class SshControl : public QObject
{
    Q_OBJECT

public:
    explicit SshControl(QObject *parent = nullptr);
    ~SshControl();

    bool isConnected() const;

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString &errorMessage);
    void newData(const QString &data);
    void logMessage(const QString &message);
    void flashingFinished(bool success);

    void monitoringStopped();

public slots:

    void doConnect(const QString &host, const QString &user, const QString &password);
    void doDisconnect();
    void startMonitoring(QString mulpin,bool with_usb);
    void stopMonitoring();
    void performFirmwareUpdate(const QString &localFilePath,bool with_usb);

private slots:
    void _private_uploadAndFlashFirmware(bool with_usb);

private:
    const QString m_destinationPath = "/home/avopadla/ota_upg/files";
    const QString m_remotePythonScript = "/home/avopadla/ota_upg/flash_firmware.py";

    QString m_firmwareUpdatePath;

    ssh_session m_session;
    ssh_channel m_channel;
    bool m_stop;
    bool m_isAuthenticated;

    QTimer *m_timer = nullptr;
};

#endif // SSHCONTROL_H



