#include "mainwindow.h"

#include <QApplication>




int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    // EntryDialog ent_dialog;
    // if (ent_dialog.exec() == QDialog::Accepted) {
    //     QString username = ent_dialog.get_username();
    //     QString ip = ent_dialog.get_ip();

    //     MainWindow w;
    //     w.set_username(username);
    //     w.set_ip(ip);

    //     w.show();

    //     return a.exec();
    // }

    // return 0;
    return a.exec();
}
