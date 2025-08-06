#include "mainwindow.h"


#include <QComboBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QSpacerItem>
#include <QPlainTextEdit>
#include <QMessageBox>

#include "adddialog.h"
#include "entrydialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),my_ssh_session(nullptr)

{
    this->setWindowTitle("stm32client");
    this->resize(800,600);


    boardComboBox = new QComboBox(this);
    boardComboBox->setMinimumWidth(200);

    addBoardButton = new QPushButton("Add board", this);
    deleteBoardButton=new QPushButton("Delete this board",this);
    configureBoardButton = new QPushButton("Config connection", this);
    updateFirmwareButton=new QPushButton("Update firmware to this board",this);

    QHBoxLayout *controlPanelLayout = new QHBoxLayout();
    controlPanelLayout->addWidget(configureBoardButton);
    controlPanelLayout->addWidget(boardComboBox);
    controlPanelLayout->addWidget(addBoardButton);
    controlPanelLayout->addWidget(deleteBoardButton);



    QSpacerItem *horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    controlPanelLayout->addSpacerItem(horizontalSpacer);

    controlPanelLayout->addWidget(updateFirmwareButton);

    logOutput = new QPlainTextEdit(this);
    logOutput->setMaximumBlockCount(1000);
    logOutput->setReadOnly(true);

    logOutput->setFont(QFont("Consolas", 10));

    logOutput->setStyleSheet(
        "QPlainTextEdit {"
        "    background-color: #000000;"
        "    color: #FFFFFF;"
        "    border: 1px solid #4A4A4A;"
        "    font-weight: bold;"
        "}"
        );
    //logOutput->appendPlainText("--- START ---");


    QVBoxLayout *mainLayout = new QVBoxLayout();

    mainLayout->addLayout(controlPanelLayout);
    mainLayout->addWidget(logOutput);

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    this->setCentralWidget(centralWidget);


    connect(addBoardButton, &QPushButton::clicked, this, &MainWindow::onAddButtonClicked);
    connect(deleteBoardButton, &QPushButton::clicked, this, &MainWindow::onDeleteButtonClicked);
    connect(configureBoardButton,&QPushButton::clicked, this,&MainWindow::onConfigureButtonClicked);


    m_sshThread = new QThread(this);
    m_sshControl = new SshControl();
    m_sshControl->moveToThread(m_sshThread);


    connect(m_sshControl, &SshControl::connected, this, &MainWindow::handleSshConnected);
    connect(m_sshControl, &SshControl::disconnected, this, &MainWindow::handleSshDisconnected);
    connect(m_sshControl, &SshControl::errorOccurred, this, &MainWindow::handleSshError);
    connect(m_sshControl, &SshControl::newData, this, &MainWindow::handleSshData);


    connect(m_sshThread, &QThread::finished, m_sshControl, &SshControl::deleteLater);
    connect(m_sshThread, &QThread::finished, m_sshThread, &QThread::deleteLater);

    m_sshThread->start();
}

MainWindow::~MainWindow()
{
    if (my_ssh_session)
    {
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
    }
}




void MainWindow::onAddButtonClicked()
{
    AddDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted)
    {
        QString name = dialog.getName();
        QString pin = dialog.getPin();

        if (!name.isEmpty() && !pin.isEmpty())
        {
            boardComboBox->addItem(name + " (pin: " + pin + ")");
        }
    }
}

void MainWindow::onDeleteButtonClicked()
{
    int index = boardComboBox->currentIndex();
    if (index != -1)
    {
        boardComboBox->removeItem(index);
    }
}

void MainWindow::onConfigureButtonClicked()
{
    EntryDialog ent_dialog;
    if (ent_dialog.exec() == QDialog::Accepted)
    {
        QString username = ent_dialog.get_username();
        QString ip = ent_dialog.get_ip(); //host==ip
        QString password=ent_dialog.get_password();

        if (ip.isEmpty() || username.isEmpty() || password.isEmpty())
        {
            QMessageBox::warning(this, "Ошибка", "Заполните все поля.");
            return;
        }


        if (m_sshControl->isConnected())
        { // Простая проверка, можно сделать лучше
            // Если да, то кнопка работает как "Отключиться"
            logOutput->appendPlainText("Отключаемся...");
            //QMetaObject::invokeMethod(m_sshControl, "doDisconnect", Qt::QueuedConnection);
            //connectButton->setEnabled(false); // Блокируем кнопку на время отключения
        }

        logOutput->clear();
        logOutput->appendPlainText(QString("Подключение к %1...").arg(ip));
        //connectButton->setEnabled(false); // Блокируем кнопку на время подключения

        // Вызываем слот воркера в его потоке
        QMetaObject::invokeMethod(m_sshControl, "doConnect", Qt::QueuedConnection,
                                  Q_ARG(QString, ip),
                                  Q_ARG(QString, username),
                                  Q_ARG(QString, password));

    }
}


void MainWindow::handleSshConnected()
{
    logOutput->appendPlainText("Подключение успешно. Запускаю мониторинг данных...");
    QMetaObject::invokeMethod(m_sshControl, "startMonitoring", Qt::QueuedConnection); //entry point to logging
}

void MainWindow::handleSshDisconnected()
{
    logOutput->appendPlainText("Соединение разорвано.");
}

void MainWindow::handleSshError(const QString &errorMessage)
{
    logOutput->appendPlainText("ОШИБКА: " + errorMessage);
}

void MainWindow::handleSshData(const QString &data)
{
    logOutput->appendPlainText(QString("Получены данные: %1").arg(data));
}

// Этот метод вызывается автоматически, когда пользователь пытается закрыть окно
void MainWindow::closeEvent(QCloseEvent *event)
{
    logOutput->appendPlainText("Приложение закрывается, завершаем фоновый поток...");

    m_sshThread->quit();

    if (!m_sshThread->wait(1)) {
        logOutput->appendPlainText("Поток не отвечает, принудительное завершение.");
        m_sshThread->terminate();
        m_sshThread->wait(); // Ждем, пока система его убьет
    } else {
        logOutput->appendPlainText("Поток успешно завершен.");
    }
    event->accept();
}
