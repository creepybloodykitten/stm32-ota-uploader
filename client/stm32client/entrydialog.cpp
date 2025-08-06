#include "entrydialog.h"

#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QHBoxLayout>

EntryDialog::EntryDialog(QWidget *parent)
    : QDialog(parent)
{
    username_edit = new QLineEdit(this);
    ip_edit= new QLineEdit(this);
    password_edit=new QLineEdit(this);

    username_edit->setText("avopadla");
    ip_edit->setText("192.168.196.173");
    password_edit->setText("123");

    ok_button = new QPushButton("ok", this);
    cancel_button = new QPushButton("cancel", this);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow("username:", username_edit);
    formLayout->addRow("IP-address:", ip_edit);
    formLayout->addRow("password:", password_edit);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(ok_button);
    buttonLayout->addWidget(cancel_button);


    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);

    connect(ok_button, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancel_button, &QPushButton::clicked, this, &QDialog::reject);

    setWindowTitle("auth");
}

QString EntryDialog::get_username() const
{
    return username_edit->text();
}

QString EntryDialog::get_ip() const
{
    return ip_edit->text();
}

QString EntryDialog::get_password() const
{
    return password_edit->text();
}

