#ifndef ONLINEDIALOG_H
#define ONLINEDIALOG_H

#include <QDialog>
#include"logindialog.h"
#include"TcpClientMediator.h"
#include"packdef.h"
#include"uploaddialog.h"
#include"QObject"
#include<QThread>
#include<QMap>

namespace Ui {
class OnlineDialog;
}

class UploadWork:public QObject
{
    Q_OBJECT
public slots:
    void slot_UploadFile(QString filePath,QString imgPath,Hobby hy);
};

class OnlineDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OnlineDialog(QWidget *parent = nullptr);
    ~OnlineDialog();

    static OnlineDialog* m_online;

    TcpClientMediator* getTcpMediator() const { return m_tcp; }

signals:
    void SIG_updateProcess(qint64 cur,qint64 max);
    void SIG_PlayUrl(QString url);
    void SIG_LoginSuccess(int userId, QString userName);
    // 把收到的直播包转发给 LiveDialog（buf 所有权仍归 OnlineDialog，
    // 由 slot_ReadyData 末尾统一 delete[]，LiveDialog 只读不释放）
    void SIG_LivePacket(char* buf, int nlen);

public slots:
    void on_pb_login_clicked();

    void slot_loginCommit(QString name,QString password);
    void slot_registerCommit(QString name,QString password,Hobby hy);
    void slot_ReadyData(unsigned int lSendIP , char* buf , int nlen);
    void slot_loginRs(unsigned int lSendIP, char *buf, int nlen);
    void slot_registerRs(unsigned int lSendIP, char *buf, int nlen);
    void slot_uploadRs(unsigned int lSendIP, char *buf, int nlen);
    void slot_downloadRs(unsigned int lSendIP, char *buf, int nlen);
    void slot_fileblockrq(unsigned int lSendIP, char *buf, int nlen);
    void slot_uploadHistoryRs(unsigned int lSendIP, char *buf, int nlen);

    void slot_UploadFile(QString filePath,QString imgPath,Hobby hy);
    void UploadFile(QString filePath,Hobby hy,QString gifName="");
    void on_pb_upload_clicked();
    void slot_PlayClicked();//点播项被点击
    void slot_MyPlayClicked(); // 上传历史被点击
private slots:
    void on_pb_fresh_clicked();

    void on_pb_video_clicked();

    void on_pb_uploadHistory_clicked();

private:
    void clearVideoLabels(const QString &prefix);

    Ui::OnlineDialog *ui;

    LoginDialog *m_login;
    TcpClientMediator *m_tcp;

    QString m_user;
    int m_id;

    UploadDialog* m_uploadDialog;
    QThread *m_uploadthread;
    UploadWork* m_uploadWorker;
    QMap<int,FileInfo*>m_mapVideoIDToFileInfo;

};

#endif // ONLINEDIALOG_H
