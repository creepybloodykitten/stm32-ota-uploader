#ifndef ADDDIALOG_H
#define ADDDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

class AddDialog : public QDialog
{
    Q_OBJECT
public:
    AddDialog(QWidget *parent = nullptr);
    QString getName() const { return nameEdit->text(); }
    QString getPin() const { return pinEdit->text(); }

private:
    QLineEdit *nameEdit;
    QLineEdit *pinEdit;
};


#endif // ADDDIALOG_H
