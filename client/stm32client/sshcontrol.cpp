#include "sshcontrol.h"


SshControl::SshControl(QObject *parent): QObject(parent),
    m_session(nullptr), m_channel(nullptr), m_stop(false),m_isAuthenticated(false){}

SshControl::~SshControl()
{
    doDisconnect();
}

void SshControl::doConnect(const QString &host, const QString &user, const QString &password)
{
    doDisconnect();

    m_stop = false;
    m_isAuthenticated = false;

    m_session = ssh_new();
    if (!m_session) {
        emit errorOccurred("Не удалось создать сессию SSH.");
        return;
    }

    ssh_options_set(m_session, SSH_OPTIONS_HOST, host.toUtf8().constData());
    ssh_options_set(m_session, SSH_OPTIONS_USER, user.toUtf8().constData());

    int rc = ssh_connect(m_session);
    if (rc != SSH_OK) {
        emit errorOccurred(QString("Ошибка подключения: %1").arg(ssh_get_error(m_session)));
        ssh_free(m_session);
        m_session = nullptr;
        return;
    }

    rc = ssh_userauth_password(m_session, nullptr, password.toUtf8().constData());
    if (rc != SSH_AUTH_SUCCESS) {
        emit errorOccurred(QString("Ошибка аутентификации: %1").arg(ssh_get_error(m_session)));
        doDisconnect();
        return;
    }

    m_isAuthenticated = true;

    emit connected();
}


void SshControl::doDisconnect()
{
    m_stop = true;


    if (m_channel)
    {
        if(ssh_channel_is_open(m_channel))
        {
            ssh_channel_send_eof(m_channel);
            ssh_channel_close(m_channel);
        }
        ssh_channel_free(m_channel);
        m_channel = nullptr;
    }
    if (m_session)
    {
        ssh_disconnect(m_session);
        ssh_free(m_session);
        m_session = nullptr;
    }

    if (m_isAuthenticated) {
        m_isAuthenticated = false;
        emit disconnected();
    }
}

bool SshControl::isConnected() const
{
    return m_isAuthenticated;
}

void SshControl::startMonitoring()
{
    if (!m_session) {
        emit errorOccurred("Нельзя начать мониторинг: сессия не установлена.");
        return;
    }

    m_channel = ssh_channel_new(m_session);
    if (!m_channel) {
        emit errorOccurred("Не удалось создать канал.");
        return;
    }

    int rc = ssh_channel_open_session(m_channel);
    if (rc != SSH_OK) {
        ssh_channel_free(m_channel);
        m_channel = nullptr;
        emit errorOccurred("Не удалось открыть сессию на канале.");
        return;
    }

    //варик если ниче не рабоатет
    // // Запрашиваем выделение псевдо-терминала (pty)
    // if (ssh_channel_request_pty(m_channel) != SSH_OK) {
    //     emit errorOccurred("Не удалось выделить PTY.");
    //     ssh_channel_close(m_channel);
    //     ssh_channel_free(m_channel);
    //     m_channel = nullptr;
    //     return;
    // }

    // Команда для запуска нашего бесконечного скрипта
    const char* command = "python3 /home/avopadla/dht22env/sensors2.py";
    rc = ssh_channel_request_exec(m_channel, command);
    if (rc != SSH_OK) {
        ssh_channel_close(m_channel);
        ssh_channel_free(m_channel);
        m_channel = nullptr;
        emit errorOccurred("Не удалось запустить скрипт мониторинга.");
        return;
    }

    //reading
    char buffer[256];
    int nbytes;

    int nbytes_stdout, nbytes_stderr;
    QString log_message;
    while (!m_stop)
    {

        nbytes_stdout = ssh_channel_read_nonblocking(m_channel, buffer, sizeof(buffer), 0); // readstdout
        nbytes_stderr = ssh_channel_read_nonblocking(m_channel, buffer, sizeof(buffer), 1); // read stderr

        if (nbytes_stdout > 0) {
            QString data = QString::fromLatin1(buffer, nbytes_stdout); // Используем fromLatin1 для сырых данных
            emit newData(data);
        }

        if (nbytes_stderr > 0) {
            QString error_data = QString::fromLatin1(buffer, nbytes_stderr);
            log_message = QString("ОШИБКА В СКРИПТЕ (stderr): %1").arg(error_data);
            emit errorOccurred(log_message);
            // Можно даже выйти из цикла, раз мы поймали ошибку
            // break;
        }


        if (ssh_channel_is_eof(m_channel)) {
            emit newData("Канал достиг EOF. Завершение.");
            break;
        }
        if (ssh_channel_is_closed(m_channel)) {
            emit errorOccurred("Канал закрыт. Завершение.");
            break;
        }


    }

    // Очистка после завершения цикла
    if(m_channel) {
        ssh_channel_close(m_channel);
        ssh_channel_free(m_channel);
        m_channel = nullptr;
    }
}


