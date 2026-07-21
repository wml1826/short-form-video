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
#include <QMutex>
#include <QWaitCondition>
#include <atomic>

namespace Ui {
class OnlineDialog;
}

class UploadWork:public QObject
{
    Q_OBJECT
public slots:
    void slot_UploadFile(QString filePath,QString imgPath,Hobby hy);
    void slot_ContinueUpload(QString localPath);  // ★ 新增：草稿续传
};

class OnlineDialog : public QDialog
{
    Q_OBJECT

    friend class UploadWork;

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
    void SIG_UploadMessage(QString title, QString text);  // ★ worker → GUI 弹窗
    void SIG_ContinueUpload(QString localPath);           // ★ GUI → worker 续传

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

    // ===== 断点续传新增槽 =====
    void on_pb_draft_clicked();
    void slot_ResumeRs(unsigned int lSendIP, char *buf, int nlen);
    void slot_UploadRs2(unsigned int lSendIP, char *buf, int nlen);
    void slot_ContinueDraft();
    void slot_DeleteDraft();
    void slot_ShowDraftMenu(const QPoint &pos);
    void slot_ShowUploadMessage(QString title, QString text);  // ★ GUI 线程弹窗

private:
    void clearVideoLabels(const QString &prefix);

    // ===== 断点续传新增辅助 =====
    void refreshDraftList();
    void ContinueUpload(const QString& localPath);
    void saveDraftOnFailure(const QString& filePath, int64_t fileSize,
                            const Hobby& hy, const QString& gifName,
                            const QString& fileType, int64_t uploadedBytes);

    Ui::OnlineDialog *ui;

    LoginDialog *m_login;
    TcpClientMediator *m_tcp;

    QString m_user;
    int m_id;

    UploadDialog* m_uploadDialog;
    QThread *m_uploadthread;
    UploadWork* m_uploadWorker;
    QMap<int,FileInfo*>m_mapVideoIDToFileInfo;

    // ===== 断点续传成员 =====
    QMutex          m_resumeMutex;
    QWaitCondition  m_resumeCond;
    bool            m_resumePending = false;
    STRU_UPLOAD_RESUME_RS m_resumeRs;

    QMutex          m_uploadRsMutex;
    QWaitCondition  m_uploadRsCond;
    bool            m_uploadRsPending = false;
    STRU_UPLOAD_RS  m_uploadRs2;

    QString m_uploadingLocalPath;           // 当前上传的本地路径
    int64_t m_uploadingFileSize = 0;         // 当前上传的文件大小
    std::atomic<int64_t> m_uploadingPos{0};  // ★ atomic：析构函数从 GUI 线程读
    std::atomic<bool> m_quitFlag{false};     // ★ 通知 worker 线程退出

};

#endif // ONLINEDIALOG_H
