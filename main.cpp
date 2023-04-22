#include "filetransfer.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    fileTransfer w;
    w.show();

    fileTransfer w2;
    w2.show();
    return a.exec();
}
