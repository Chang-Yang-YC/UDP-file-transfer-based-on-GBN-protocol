#include "filetransfer.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    fileTransfer w;
    w.show();
    return a.exec();
}
