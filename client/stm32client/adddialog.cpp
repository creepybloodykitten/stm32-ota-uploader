#include "adddialog.h"

AddDialog::AddDialog(QWidget *parent) : QDialog(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *nameLayout = new QHBoxLayout;
    nameLayout->addWidget(new QLabel("boardname:"));
    nameEdit = new QLineEdit;
    nameLayout->addWidget(nameEdit);

    QHBoxLayout *pinLayout = new QHBoxLayout;
    pinLayout->addWidget(new QLabel("pin:"));
    pinEdit = new QLineEdit;
    pinLayout->addWidget(pinEdit);

    QPushButton *okButton = new QPushButton("add");

    mainLayout->addLayout(nameLayout);
    mainLayout->addLayout(pinLayout);
    mainLayout->addWidget(okButton);

    connect(okButton, &QPushButton::clicked, this, &AddDialog::accept);
}
