#include "sshcontrol.h"


SshControl::SshControl(QObject *parent): QObject(parent),
    m_session(nullptr), m_channel(nullptr), m_stop(false),m_isAuthenticated(false),m_timer(nullptr){}

SshControl::~SshControl()
{
    doDisconnect();
}

void SshControl::stopMonitoring()
{
    m_stop = true;
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

    if (m_timer) {
        m_timer->stop();
        m_timer->deleteLater();
        m_timer = nullptr;
    }

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
    if (m_timer || m_channel) {
        doDisconnect(); // doDisconnect теперь безопасно все очищает
        // Заново устанавливаем флаги, т.к. doDisconnect их сбрасывает
        m_isAuthenticated = true;
        m_stop = false;
    }

    m_stop = false;
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
    if (rc != SSH_OK)
    {
        ssh_channel_close(m_channel);
        ssh_channel_free(m_channel);
        m_channel = nullptr;
        emit errorOccurred("Не удалось запустить скрипт мониторинга.");
        return;
    }

    //reading
    // char buffer[256];
    // int nbytes;

    // int nbytes_stdout, nbytes_stderr;
    // QString log_message;
    // while (!m_stop)
    // {

    //     nbytes_stdout = ssh_channel_read_nonblocking(m_channel, buffer, sizeof(buffer), 0); // readstdout
    //     nbytes_stderr = ssh_channel_read_nonblocking(m_channel, buffer, sizeof(buffer), 1); // read stderr

    //     if (nbytes_stdout > 0) {
    //         QString data = QString::fromLatin1(buffer, nbytes_stdout);
    //         emit newData(data);
    //     }

    //     if (nbytes_stderr > 0) {
    //         QString error_data = QString::fromLatin1(buffer, nbytes_stderr);
    //         log_message = QString("ОШИБКА В СКРИПТЕ (stderr): %1").arg(error_data);
    //         emit errorOccurred(log_message);
    //         // Можно даже выйти из цикла, раз мы поймали ошибку
    //         // break;
    //     }

    //     if (ssh_channel_is_eof(m_channel)) {
    //         emit newData("Канал достиг EOF. Завершение.");
    //         break;
    //     }

    //     if (ssh_channel_is_closed(m_channel)) {
    //         emit errorOccurred("Канал закрыт. Завершение.");
    //         break;
    //     }
    // }
    //
    // if(m_channel) {
    //     ssh_channel_close(m_channel);
    //     ssh_channel_free(m_channel);
    //     m_channel = nullptr;
    // }

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, [this]() {
        // Условие остановки: нет канала, или пришел флаг stop, или канал закрылся сам
        if (!m_channel || m_stop || ssh_channel_is_closed(m_channel) || ssh_channel_is_eof(m_channel)) {
            // --- НАЧАЛО БЛОКА ОЧИСТКИ ---
            if (m_timer) {
                m_timer->stop();
                m_timer->deleteLater();
                m_timer = nullptr;
            }
            if (m_channel) {
                // Это безопасно, т.к. мы больше не будем использовать этот канал
                ssh_channel_send_eof(m_channel);
                ssh_channel_close(m_channel);
                ssh_channel_free(m_channel);
                m_channel = nullptr;
                emit newData("Мониторинг завершён.");
            }
            emit monitoringStopped();
            // --- КОНЕЦ БЛОКА ОЧИСТКИ ---
            return; // Выходим из лямбды
        }

        char buffer[256];
        int nbytes_stdout = ssh_channel_read_nonblocking(m_channel, buffer, sizeof(buffer), 0);
        if (nbytes_stdout > 0) {
            emit newData(QString::fromLatin1(buffer, nbytes_stdout));
        }

        int nbytes_stderr = ssh_channel_read_nonblocking(m_channel, buffer, sizeof(buffer), 1);
        if (nbytes_stderr > 0) {
            emit errorOccurred(QString("stderr: %1").arg(QString::fromLatin1(buffer, nbytes_stderr)));
        }
    });
    m_timer->start(100);


}




void SshControl::performFirmwareUpdate(const QString &localFilePath)
{
    if (!m_isAuthenticated) {
        emit logMessage("Ошибка: нет активного SSH-соединения.\n");
        emit flashingFinished(false);
        return;
    }

    m_firmwareUpdatePath = localFilePath; // Сохраняем путь для следующего шага

    // Проверяем, запущен ли мониторинг
    if (m_timer && m_timer->isActive()) {
        emit logMessage("Приостановка мониторинга для обновления прошивки...\n");

        // --- ВОТ УПРОЩЕНИЕ ---
        // Мы подключаем сигнал к лямбда-функции.
        // Эта лямбда-функция "захватывает" this, чтобы иметь доступ к членам класса.
        connect(this, &SshControl::monitoringStopped, this, [this]() {
            // Лямбда вызывается, когда мониторинг остановлен.
            // Теперь она напрямую вызывает наш единственный приватный слот.
            _private_uploadAndFlashFirmware();
        }, Qt::SingleShotConnection);
        // ----------------------

        // Даем команду на остановку
        stopMonitoring();
    } else {
        // Мониторинг не был запущен, можно сразу начинать прошивку
        emit logMessage("Мониторинг не активен, начинаю прошивку...\n");
        _private_uploadAndFlashFirmware();
    }
}

void SshControl::_private_uploadAndFlashFirmware()
{
    const QString localFilePath = m_firmwareUpdatePath;
    m_stop = false;
    bool success = true;
    if (!m_isAuthenticated) {
        emit logMessage("Ошибка: нет активного SSH-соединения.\n");
        emit flashingFinished(false);
        return;
    }

    QFileInfo fileInfo(localFilePath);
    QString fileName = fileInfo.fileName();
    QString remoteFilePath = m_destinationPath + "/" + fileName;

    emit logMessage(QString("--- Начало процесса прошивки для файла %1 ---\n").arg(fileName));
    emit logMessage("[Шаг 1/2] Загрузка файла на Raspberry Pi (используя SFTP)...\n");

    // 1. Создаем сессию SFTP
    sftp_session sftp = sftp_new(m_session);
    if (sftp == nullptr) {
        emit logMessage(QString("Критическая ошибка: не удалось создать SFTP сессию: %1\n").arg(ssh_get_error(m_session)));
        emit flashingFinished(false);
        return;
    }

    // 2. Инициализируем сессию SFTP
    int rc = sftp_init(sftp);
    if (rc != SSH_OK) {
        emit logMessage(QString("Критическая ошибка: не удалось инициализировать SFTP сессию: %1\n").arg(sftp_get_error(sftp)));
        sftp_free(sftp);
        emit flashingFinished(false);
        return;
    }

    // 3. Открываем файл на удаленной машине для записи
    // Флаги: O_WRONLY (только запись), O_CREAT (создать, если нет), O_TRUNC (обрезать до нуля, если существует)
    // Права доступа: 0644 (чтение/запись для владельца, чтение для остальных)
    sftp_file file_handle = sftp_open(sftp, remoteFilePath.toUtf8().constData(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_handle == nullptr) {
        emit logMessage(QString("Ошибка SFTP: не удалось открыть удаленный файл для записи: %1\n").arg(sftp_get_error(sftp)));
        sftp_free(sftp);
        emit flashingFinished(false);
        return;
    }

    // 4. Читаем локальный файл и пишем в удаленный
    QFile localFile(localFilePath);
    if (!localFile.open(QIODevice::ReadOnly)) {
        emit logMessage(QString("Не удалось открыть локальный файл: %1\n").arg(localFilePath));
        sftp_close(file_handle);
        sftp_free(sftp);
        emit flashingFinished(false);
        return;
    }

    char buffer[16384]; // Буфер для чтения/записи
    qint64 bytesRead;
    while ((bytesRead = localFile.read(buffer, sizeof(buffer))) > 0) {
        qint64 bytesWritten = sftp_write(file_handle, buffer, bytesRead);
        if (bytesWritten != bytesRead) {
            emit logMessage(QString("Ошибка SFTP: не удалось записать данные в удаленный файл: %1\n").arg(sftp_get_error(sftp)));
            localFile.close();
            sftp_close(file_handle);
            sftp_free(sftp);
            emit flashingFinished(false);
            return;
        }
    }
    localFile.close();

    // 5. Закрываем удаленный файл и освобождаем ресурсы
    sftp_close(file_handle);
    sftp_free(sftp);
    emit logMessage("Файл успешно загружен!\n");


    // --- ШАГ 2: ЗАПУСК СКРИПТА ПРОШИВКИ (эта часть не меняется) ---
    emit logMessage("\n[Шаг 2/2] Запуск скрипта прошивки на Raspberry Pi...\n");

    ssh_channel exec_channel = ssh_channel_new(m_session);
    if (!exec_channel) {
        emit logMessage("Ошибка: не удалось создать канал для выполнения команды.\n");
        emit flashingFinished(false);
        return;
    }

    rc = ssh_channel_open_session(exec_channel);
    if (rc != SSH_OK) {
        ssh_channel_free(exec_channel);
        emit logMessage("Ошибка: не удалось открыть сессию на канале.\n");
        emit flashingFinished(false);
        return;
    }

    // Используем тот же remoteFilePath, что и при загрузке
    QString command = QString("sudo python3 %1 \"%2\"").arg(m_remotePythonScript, remoteFilePath);
    emit logMessage(QString("Выполняется команда: %1\n").arg(command));

    rc = ssh_channel_request_exec(exec_channel, command.toUtf8().constData());
    if (rc != SSH_OK) {
        ssh_channel_close(exec_channel);
        ssh_channel_free(exec_channel);
        emit logMessage("Ошибка: не удалось запустить удаленную команду.\n");
        emit flashingFinished(false);
        return;
    }

    // Читаем вывод от скрипта
    while ((bytesRead = ssh_channel_read(exec_channel, buffer, sizeof(buffer), 0)) > 0) {
        emit logMessage(QString::fromUtf8(buffer, bytesRead));
    }
    // Читаем вывод ошибок от скрипта
    while ((bytesRead = ssh_channel_read(exec_channel, buffer, sizeof(buffer), 1)) > 0) {
        emit logMessage(QString::fromUtf8(buffer, bytesRead));
    }


    int exit_status = ssh_channel_get_exit_status(exec_channel);
    ssh_channel_send_eof(exec_channel);
    ssh_channel_close(exec_channel);
    ssh_channel_free(exec_channel);

    if (exit_status != 0)
    {
        emit logMessage(QString("\nОшибка: удаленный скрипт завершился с кодом %1.\n").arg(exit_status));
        success = false;
    }

    emit flashingFinished(success);
}



// void SshControl::uploadAndFlashFirmware(const QString &localFilePath)
// {
//     m_stop = false;
//     bool success = true;
//     if (!m_isAuthenticated) {
//         emit logMessage("Ошибка: нет активного SSH-соединения.\n");
//         emit flashingFinished(false);
//         return;
//     }

//     QFileInfo fileInfo(localFilePath);
//     QString fileName = fileInfo.fileName();
//     QString remoteFilePath = m_destinationPath + "/" + fileName;

//     emit logMessage(QString("--- Начало процесса прошивки для файла %1 ---\n").arg(fileName));
//     emit logMessage("[Шаг 1/2] Загрузка файла на Raspberry Pi (используя SFTP)...\n");

//     // 1. Создаем сессию SFTP
//     sftp_session sftp = sftp_new(m_session);
//     if (sftp == nullptr) {
//         emit logMessage(QString("Критическая ошибка: не удалось создать SFTP сессию: %1\n").arg(ssh_get_error(m_session)));
//         emit flashingFinished(false);
//         return;
//     }

//     // 2. Инициализируем сессию SFTP
//     int rc = sftp_init(sftp);
//     if (rc != SSH_OK) {
//         emit logMessage(QString("Критическая ошибка: не удалось инициализировать SFTP сессию: %1\n").arg(sftp_get_error(sftp)));
//         sftp_free(sftp);
//         emit flashingFinished(false);
//         return;
//     }

//     // 3. Открываем файл на удаленной машине для записи
//     // Флаги: O_WRONLY (только запись), O_CREAT (создать, если нет), O_TRUNC (обрезать до нуля, если существует)
//     // Права доступа: 0644 (чтение/запись для владельца, чтение для остальных)
//     sftp_file file_handle = sftp_open(sftp, remoteFilePath.toUtf8().constData(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
//     if (file_handle == nullptr) {
//         emit logMessage(QString("Ошибка SFTP: не удалось открыть удаленный файл для записи: %1\n").arg(sftp_get_error(sftp)));
//         sftp_free(sftp);
//         emit flashingFinished(false);
//         return;
//     }

//     // 4. Читаем локальный файл и пишем в удаленный
//     QFile localFile(localFilePath);
//     if (!localFile.open(QIODevice::ReadOnly)) {
//         emit logMessage(QString("Не удалось открыть локальный файл: %1\n").arg(localFilePath));
//         sftp_close(file_handle);
//         sftp_free(sftp);
//         emit flashingFinished(false);
//         return;
//     }

//     char buffer[16384]; // Буфер для чтения/записи
//     qint64 bytesRead;
//     while ((bytesRead = localFile.read(buffer, sizeof(buffer))) > 0) {
//         qint64 bytesWritten = sftp_write(file_handle, buffer, bytesRead);
//         if (bytesWritten != bytesRead) {
//             emit logMessage(QString("Ошибка SFTP: не удалось записать данные в удаленный файл: %1\n").arg(sftp_get_error(sftp)));
//             localFile.close();
//             sftp_close(file_handle);
//             sftp_free(sftp);
//             emit flashingFinished(false);
//             return;
//         }
//     }
//     localFile.close();

//     // 5. Закрываем удаленный файл и освобождаем ресурсы
//     sftp_close(file_handle);
//     sftp_free(sftp);
//     emit logMessage("Файл успешно загружен!\n");


//     // --- ШАГ 2: ЗАПУСК СКРИПТА ПРОШИВКИ (эта часть не меняется) ---
//     emit logMessage("\n[Шаг 2/2] Запуск скрипта прошивки на Raspberry Pi...\n");

//     ssh_channel exec_channel = ssh_channel_new(m_session);
//     if (!exec_channel) {
//         emit logMessage("Ошибка: не удалось создать канал для выполнения команды.\n");
//         emit flashingFinished(false);
//         return;
//     }

//     rc = ssh_channel_open_session(exec_channel);
//     if (rc != SSH_OK) {
//         ssh_channel_free(exec_channel);
//         emit logMessage("Ошибка: не удалось открыть сессию на канале.\n");
//         emit flashingFinished(false);
//         return;
//     }

//     // Используем тот же remoteFilePath, что и при загрузке
//     QString command = QString("sudo python3 %1 \"%2\"").arg(m_remotePythonScript, remoteFilePath);
//     emit logMessage(QString("Выполняется команда: %1\n").arg(command));

//     rc = ssh_channel_request_exec(exec_channel, command.toUtf8().constData());
//     if (rc != SSH_OK) {
//         ssh_channel_close(exec_channel);
//         ssh_channel_free(exec_channel);
//         emit logMessage("Ошибка: не удалось запустить удаленную команду.\n");
//         emit flashingFinished(false);
//         return;
//     }

//     // Читаем вывод от скрипта
//     while ((bytesRead = ssh_channel_read(exec_channel, buffer, sizeof(buffer), 0)) > 0) {
//         emit logMessage(QString::fromUtf8(buffer, bytesRead));
//     }
//     // Читаем вывод ошибок от скрипта
//     while ((bytesRead = ssh_channel_read(exec_channel, buffer, sizeof(buffer), 1)) > 0) {
//         emit logMessage(QString::fromUtf8(buffer, bytesRead));
//     }


//     int exit_status = ssh_channel_get_exit_status(exec_channel);
//     ssh_channel_send_eof(exec_channel);
//     ssh_channel_close(exec_channel);
//     ssh_channel_free(exec_channel);

//     if (exit_status != 0)
//     {
//         emit logMessage(QString("\nОшибка: удаленный скрипт завершился с кодом %1.\n").arg(exit_status));
//         success = false;
//     }

//     emit flashingFinished(success);
// }
