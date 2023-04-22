#ifndef PREFERENCEDIALOG_H
#define PREFERENCEDIALOG_H

#include <QObject>
#include <QDialog>
#include <QDebug>

namespace Ui {
class preferenceDialog;
}

class preferenceDialog : public QDialog
{
    Q_OBJECT

public:
    QString thisip;             //本地ip地址
    QString thisUDPPort;       //本地端口号
    int dataSize = 1024;        //数据字段长度
    int errRate = 10;           //数据出错率：每errRate个包出错一个
    int lostRate = 10;          //数据丢失率：每lostRate个包丢失一个
    int swSize = 4;             //滑动窗口大小
    int initSeqNo = 1;          //起始PDU序号
    int timeOut = 1000;         //超时定时器时长
    explicit preferenceDialog(QWidget *parent = nullptr);
    ~preferenceDialog();
    void show_now_param(QString msg);
    void displayParam();
    void getParam();

signals:
    void send_Back_Param(QString);

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::preferenceDialog *ui;
};

#endif // PREFERENCEDIALOG_H
