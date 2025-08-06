#ifndef SSHCONTROL_H
#define SSHCONTROL_H

#include <QObject>
#include <QDebug>
#include <QString>
#include <libssh/libssh.h>

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

public slots:

    void doConnect(const QString &host, const QString &user, const QString &password);
    void doDisconnect();
    void startMonitoring();

private:
    ssh_session m_session;
    ssh_channel m_channel;
    bool m_stop;
    bool m_isAuthenticated;
};

#endif // SSHCONTROL_H


#ifndef SSHWORKER_H
#define SSHWORKER_H





#endif // SSHWORKER_H
