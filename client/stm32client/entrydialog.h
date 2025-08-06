#ifndef ENTRYDIALOG_H
#define ENTRYDIALOG_H

#include <QDialog>

class QLineEdit;
class QPushButton;

class EntryDialog : public QDialog
{
    Q_OBJECT
public:
    explicit EntryDialog(QWidget *parent = nullptr);

    QString get_username() const;
    QString get_ip() const;
    QString get_password() const;

private:
    QLineEdit *username_edit;
    QLineEdit *ip_edit;
    QLineEdit *password_edit;
    QPushButton *ok_button;
    QPushButton *cancel_button;
};

#endif // ENTRYDIALOG_H
