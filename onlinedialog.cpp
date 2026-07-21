#include "onlinedialog.h"
#include "ui_onlinedialog.h"
#include "UploadDraftManager.h"
#include<QCryptographicHash>
#include<QMessageBox>
#include<QFileInfo>
#include<QDir>
#include"movielable.h"
#include <QListWidgetItem>
#include <QMenu>

#define MD5_KEY 12345

static QByteArray GetMD5(QString val){
    QCryptographicHash hash(QCryptographicHash::Md5);
    QString tmp=QString("%1_%2").arg(val).arg(MD5_KEY);
    hash.addData(tmp.toStdString().c_str());
    QByteArray bt=hash.result();
    return bt.toHex();
}

OnlineDialog* OnlineDialog::m_online=NULL;

OnlineDialog::OnlineDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OnlineDialog),m_id(0)
{
    ui->setupUi(this);
    m_online=this;
    qsrand(QTime(0,0,0).msecsTo(QTime::currentTime()));
    //注册信号中使用的结构
    qRegisterMetaType<Hobby>("Hobby");

    m_login=new LoginDialog();
    m_login->hide();

    connect(ui->pb_play1,SIGNAL(SIG_labelClicked()),this,SLOT(slot_PlayClicked()));
    connect(ui->pb_play2,SIGNAL(SIG_labelClicked()),this,SLOT(slot_PlayClicked()));
    connect(ui->pb_play3,SIGNAL(SIG_labelClicked()),this,SLOT(slot_PlayClicked()));
    connect(ui->pb_play4,SIGNAL(SIG_labelClicked()),this,SLOT(slot_PlayClicked()));
    connect(ui->pb_play5,SIGNAL(SIG_labelClicked()),this,SLOT(slot_PlayClicked()));
    connect(ui->pb_play6,SIGNAL(SIG_labelClicked()),this,SLOT(slot_PlayClicked()));
    connect(ui->pb_play7,SIGNAL(SIG_labelClicked()),this,SLOT(slot_PlayClicked()));
    connect(ui->pb_play8,SIGNAL(SIG_labelClicked()),this,SLOT(slot_PlayClicked()));
    connect(ui->pb_play9,SIGNAL(SIG_labelClicked()),this,SLOT(slot_PlayClicked()));
    connect(ui->pb_play10,SIGNAL(SIG_labelClicked()),this,SLOT(slot_PlayClicked()));
    connect(ui->pb_play11,SIGNAL(SIG_labelClicked()),this,SLOT(slot_PlayClicked()));
    connect(ui->pb_play12,SIGNAL(SIG_labelClicked()),this,SLOT(slot_PlayClicked()));
    connect(ui->pb_play13,SIGNAL(SIG_labelClicked()),this,SLOT(slot_PlayClicked()));
    connect(ui->pb_play14,SIGNAL(SIG_labelClicked()),this,SLOT(slot_PlayClicked()));
    connect(ui->pb_play15,SIGNAL(SIG_labelClicked()),this,SLOT(slot_PlayClicked()));

    connect(ui->pb_myplay1,SIGNAL(SIG_labelClicked()),this,SLOT(slot_MyPlayClicked()));
    connect(ui->pb_myplay2,SIGNAL(SIG_labelClicked()),this,SLOT(slot_MyPlayClicked()));
    connect(ui->pb_myplay3,SIGNAL(SIG_labelClicked()),this,SLOT(slot_MyPlayClicked()));
    connect(ui->pb_myplay4,SIGNAL(SIG_labelClicked()),this,SLOT(slot_MyPlayClicked()));
    connect(ui->pb_myplay5,SIGNAL(SIG_labelClicked()),this,SLOT(slot_MyPlayClicked()));
    connect(ui->pb_myplay6,SIGNAL(SIG_labelClicked()),this,SLOT(slot_MyPlayClicked()));
    connect(ui->pb_myplay7,SIGNAL(SIG_labelClicked()),this,SLOT(slot_MyPlayClicked()));
    connect(ui->pb_myplay8,SIGNAL(SIG_labelClicked()),this,SLOT(slot_MyPlayClicked()));
    connect(ui->pb_myplay9,SIGNAL(SIG_labelClicked()),this,SLOT(slot_MyPlayClicked()));
    connect(ui->pb_myplay10,SIGNAL(SIG_labelClicked()),this,SLOT(slot_MyPlayClicked()));
    connect(ui->pb_myplay11,SIGNAL(SIG_labelClicked()),this,SLOT(slot_MyPlayClicked()));
    connect(ui->pb_myplay12,SIGNAL(SIG_labelClicked()),this,SLOT(slot_MyPlayClicked()));
    connect(ui->pb_myplay13,SIGNAL(SIG_labelClicked()),this,SLOT(slot_MyPlayClicked()));
    connect(ui->pb_myplay14,SIGNAL(SIG_labelClicked()),this,SLOT(slot_MyPlayClicked()));
    connect(ui->pb_myplay15,SIGNAL(SIG_labelClicked()),this,SLOT(slot_MyPlayClicked()));


    connect(m_login,SIGNAL(SIG_loginCommit(QString,QString)),this,SLOT(slot_loginCommit(QString,QString)));
    connect(m_login,SIGNAL(SIG_registerCommit(QString,QString,Hobby)),this,SLOT(slot_registerCommit(QString,QString,Hobby)));

    m_tcp=new TcpClientMediator;
    connect(m_tcp,SIGNAL(SIG_ReadyData(unsigned int, char*, int)),this,SLOT(slot_ReadyData(unsigned int, char*, int)));

    bool ok = m_tcp->OpenNet("172.20.10.5", 8000);
    if(!ok){
        QMessageBox::about(this,"提示","连接服务器失败");
    }

    m_uploadDialog=new UploadDialog;
   // connect(m_uploadDialog,SIGNAL(SIG_UploadFile(QString,QString,Hobby)),this,SLOT(slot_UploadFile(QString,QString,Hobby)));
    connect(this,SIGNAL(SIG_updateProcess(qint64,qint64)),m_uploadDialog,SLOT(slot_updateProcess(qint64,qint64)));
    m_uploadDialog->hide();

    m_uploadWorker=new UploadWork;
    m_uploadthread=new QThread;
    connect(m_uploadDialog,SIGNAL(SIG_UploadFile(QString,QString,Hobby)),m_uploadWorker,SLOT(slot_UploadFile(QString,QString,Hobby)));
    m_uploadWorker->moveToThread(m_uploadthread);
    m_uploadthread->start();

    // ===== 断点续传：草稿箱按钮连接 =====
      connect(ui->pb_draft, SIGNAL(clicked()), this, SLOT(on_pb_draft_clicked()));
      connect(ui->lw_drafts, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
              this, SLOT(slot_ContinueDraft()));
      ui->lw_drafts->setContextMenuPolicy(Qt::CustomContextMenu);
      connect(ui->lw_drafts, SIGNAL(customContextMenuRequested(QPoint)),
              this, SLOT(slot_ShowDraftMenu(QPoint)));

      // ★ worker 线程弹窗信号连接（worker → GUI）
      connect(this, SIGNAL(SIG_UploadMessage(QString, QString)),
              this, SLOT(slot_ShowUploadMessage(QString, QString)));
      // ★ 草稿续传信号连接（GUI → worker）
      connect(this, SIGNAL(SIG_ContinueUpload(QString)),
              m_uploadWorker, SLOT(slot_ContinueUpload(QString)));
}

OnlineDialog::~OnlineDialog()
{
    // ★ 通知 worker 线程退出
        m_quitFlag = true;

        // ★ 如果还在上传，写草稿（m_uploadingPos 是 atomic，可以安全读）
        if (!m_uploadingLocalPath.isEmpty() && m_uploadingFileSize > 0) {
            UploadDraft d;
            d.localPath = m_uploadingLocalPath;
            d.fileName = QFileInfo(m_uploadingLocalPath).fileName();
            d.fileSize = m_uploadingFileSize;
            d.uploadedBytes = m_uploadingPos.load();
            d.createTime = QDateTime::currentDateTime();
            d.status = "paused";
            UploadDraftManager::instance()->addDraft(m_id, d);
        }

    if(m_login){
        delete m_login;
        m_login=NULL;
    }
    if(m_tcp){
        delete m_tcp;
        m_tcp=NULL;
    }
    if(m_uploadWorker){
       delete m_uploadWorker;
        m_uploadWorker=NULL;
    }
    if(m_uploadthread){
        m_uploadthread->quit();
        m_uploadthread->wait(100);
        if(m_uploadthread->isRunning()){
            m_uploadthread->terminate();
            m_uploadthread->wait(100);
        }
        delete m_uploadthread;
        m_uploadthread=NULL;
    }
    delete ui;
}

void OnlineDialog::clearVideoLabels(const QString &prefix)
{
    for(int i=1;i<=15;++i)
    {
        movielable *label=findChild<movielable *>(QString("%1%2").arg(prefix).arg(i));
        if(label)
            label->clearContent();
    }
}
//登录模块
void OnlineDialog::on_pb_login_clicked()
{
    m_login->show();
}

//登录提交
void OnlineDialog::slot_loginCommit(QString name, QString password)
{
    m_user=name;

    std::string strname=name.toStdString();
    char* bufName=(char*)strname.c_str();

    QByteArray bt=GetMD5(password);
    STRU_LOGIN_RQ rq;
    strcpy_s(rq.user,_MAX_SIZE,bufName);
    memcpy(rq.password,bt.data(),bt.length());

    if(m_tcp->SendData(0,(char*)&rq,sizeof(rq))<0){
        QMessageBox::about(this,"提示","网络故障");
    }
}

//注册提交
void OnlineDialog::slot_registerCommit(QString name, QString password, Hobby hy)
{
    std::string strname=name.toStdString();
    char* bufName=(char*)strname.c_str();

    QByteArray bt=GetMD5(password);
    STRU_REGISTER_RQ rq;
    strcpy_s(rq.user,_MAX_SIZE,bufName);
    memcpy(rq.password,bt.data(),bt.length());
    rq.dance  =hy.dance  ;
    rq.edu    =hy.edu    ;
    rq.ennegy =hy.ennegy ;
    rq.food   =hy.food   ;
    rq.funny  =hy.funny  ;
    rq.music  =hy.music  ;
    rq.outside=hy.outside;
    rq.video  =hy.video  ;
    if(m_tcp->SendData(0,(char*)&rq,sizeof(rq))<0){
        QMessageBox::about(this,"提示","网络故障");
    }
}
//Tcp网络接收
void OnlineDialog::slot_ReadyData(unsigned int lSendIP, char *buf, int nlen)
{
    int nType=*(int*)buf;
    switch(nType)
    {
    case _DEF_PACK_LOGIN_RS:
        slot_loginRs(lSendIP,buf,nlen);
        break;
    case _DEF_PACK_REGISTER_RS:
        slot_registerRs(lSendIP,buf,nlen);
        break;
    case _DEF_PACK_UPLOAD_RS:
        slot_UploadRs2(lSendIP, buf, nlen);  // ★ 改用新函数
        break;
    case _DEF_PACK_UPLOAD_RESUME_RS:         // ★ 新增
        slot_ResumeRs(lSendIP, buf, nlen);
        break;
    case DEF_PACK_DOWNLOAD_RS:
        slot_downloadRs(lSendIP,buf,nlen);
        break;
    case _DEF_PACK_FILEBLOCK_RQ:
        slot_fileblockrq(lSendIP,buf,nlen);
        break;
    case _DEF_PACK_UPLOADHISTORY_RS:
        slot_uploadHistoryRs(lSendIP,buf,nlen);
        break;
    case _DEF_PACK_LIVE_START_RS:
    case _DEF_PACK_LIVE_STOP_RS:
    case _DEF_PACK_LIVE_LIST_RS:
    case _DEF_PACK_LIVE_LIST_END:
        emit SIG_LivePacket(buf, nlen);   // LiveDialog 只读 buf，不 delete
        break;
    }

    delete[] buf;
}
//用户登录回复
void OnlineDialog::slot_loginRs(unsigned int lSendIP, char *buf, int nlen)
{
    STRU_LOGIN_RS*rs=(STRU_LOGIN_RS*)buf;
    switch(rs->result){
        case user_not_exist:
            QMessageBox::about(m_login,"提示","用户不存在，登录失败");
        break;
        case password_error:
            QMessageBox::about(m_login,"提示","密码错误，登录失败");
        break;
        case login_success:
            QMessageBox::about(m_login,"提示","登录成功");
            clearVideoLabels("pb_play");
            clearVideoLabels("pb_myplay");
            ui->sw_page->setCurrentIndex(0);
            ui->lb_name->setText(QString("您好,%1").arg(m_user));
            m_login->hide();
            m_id=rs->userid;
            emit SIG_LoginSuccess(m_id, m_user);

            //下载列表文件
            STRU_DOWNLOAD_RQ rq;
            rq.m_nUserId=m_id;
            m_tcp->SendData(0,(char*)&rq,sizeof(rq));

        break;
    }
}
//用户注册回复
void OnlineDialog::slot_registerRs(unsigned int lSendIP, char *buf, int nlen)
{
    STRU_REGISTER_RS*rs=(STRU_REGISTER_RS*)buf;
    switch(rs->result){
        case user_is_exist:
            QMessageBox::about(m_login,"提示","用户已存在，注册失败");
        break;
        case register_success:
            QMessageBox::about(m_login,"提示","注册成功");
        break;

    }
}
//上传的回复
void OnlineDialog::slot_uploadRs(unsigned int lSendIP, char *buf, int nlen)
{
    STRU_UPLOAD_RS *rs=(STRU_UPLOAD_RS*)buf;
    switch(rs->m_nResult)
    {
       case 1:
           QMessageBox::about(m_login,"提示","上传成功");
       break;
    }
}
//下载回复
void OnlineDialog::slot_downloadRs(unsigned int lSendIP, char *buf, int nlen)
{
    STRU_DOWNLOAD_RS *rs=(STRU_DOWNLOAD_RS*)buf;

    //文件头 给FileInfo赋值
    FileInfo *info =new FileInfo;
    info->videoId=rs->m_nVideoId;
    info->fileId=rs->m_nFileId;
    info->fileName=rs->m_szFileName;

    QDir dir;
    if(!dir.exists(QDir::currentPath()+"/temp/"))
    {
        dir.mkpath(QDir::currentPath()+"/temp/");
    }
    info->filePath=QString("./temp/%1").arg(rs->m_szFileName);
    qDebug() << "准备写入文件:" << info->filePath;

    info->filePos=0;
    info->fileSize=rs->m_nFileSize;
    info->rtmpUrl=QString("rtmp://%1/vod%2").arg(DEF_SSERVER_IP).arg(rs->m_rtmp);
    qDebug()<<"rtmp--"<<info->rtmpUrl;
    info->pFile=new QFile(info->filePath);

    if(info->pFile->open(QIODevice::WriteOnly))
    {
        m_mapVideoIDToFileInfo[info->videoId]=info;
    }else{
        qDebug() << "文件打开失败:" << info->pFile->errorString();
        delete info;
    }
}
//下载文件块
void OnlineDialog::slot_fileblockrq(unsigned int lSendIP, char *buf, int nlen)
{
    STRU_FILEBLOCK_RQ *rq=(STRU_FILEBLOCK_RQ*)buf;

    auto ite=m_mapVideoIDToFileInfo.find(rq->m_nFileId);
    if(ite==m_mapVideoIDToFileInfo.end())
    {
        qDebug() << "找不到对应的文件信息，key:" << rq->m_nFileId;
        return;
    }
    FileInfo* info=m_mapVideoIDToFileInfo[rq->m_nFileId];

    int64_t res=info->pFile->write(rq->m_szFileContent,rq->m_nBlockLen);
    if(res < 0)
        {
            qDebug() << "写入文件失败:" << info->pFile->errorString();
            return;
        }
    info->filePos+=res;
    if(info->filePos>=info->fileSize)
    {
        //关闭文件
        info->pFile->close();
        //删除节点
        m_mapVideoIDToFileInfo.erase(ite);
        //设置到控件
        //QString pbNum=QString("pb_play%1").arg(info->fileId+1);
        QString pbNum;
        if(ui->sw_page->currentIndex() == 0) {
            // 推荐影视页
            pbNum = QString("pb_play%1").arg(info->fileId + 1);
        } else {
            // 上传历史页
            pbNum = QString("pb_myplay%1").arg(info->fileId + 1);
        }
         movielable* pb_play=ui->sw_page->findChild<movielable *>(pbNum);
        QMovie *LastMovie=pb_play->movie();
        if(LastMovie &&LastMovie->isValid())
        {
            delete LastMovie;
        }
         QMovie *movie=new QMovie(info->filePath);
         if (!pb_play) {
             qDebug() << "找不到控件：" << pbNum;
             delete info;
             return;
         }
         pb_play->setMovie(movie);
         pb_play->setRtmpUrl(info->rtmpUrl);
        //回收info
         delete info;
         info=NULL;
    }
}
//上传历史
void OnlineDialog::slot_uploadHistoryRs(unsigned int lSendIP, char *buf, int nlen)
{
    STRU_DOWNLOAD_RS *rs = (STRU_DOWNLOAD_RS*)buf;

    FileInfo *info = new FileInfo;
    info->videoId = rs->m_nVideoId;
    info->fileId = rs->m_nFileId;
    info->fileName = rs->m_szFileName;

    QDir dir;
    if(!dir.exists(QDir::currentPath() + "/temp/"))
        dir.mkpath(QDir::currentPath() + "/temp/");

    info->filePath = QString("./temp/%1").arg(rs->m_szFileName);
    info->filePos = 0;
    info->fileSize = rs->m_nFileSize;
    info->rtmpUrl = QString("rtmp://%1/vod%2").arg(DEF_SSERVER_IP).arg(rs->m_rtmp);
    info->pFile = new QFile(info->filePath);

    if(info->pFile->open(QIODevice::WriteOnly)) {
        m_mapVideoIDToFileInfo[info->videoId] = info;
    } else {
        delete info;
    }
}
//上传文件响应
void OnlineDialog::slot_UploadFile(QString filePath, QString imgPath, Hobby hy)
{
    //上传
    qDebug()<<"开始上传";

    UploadFile(imgPath,hy);
    UploadFile(filePath,hy,imgPath);
}
//上传文件（支持断点续传）
void OnlineDialog::UploadFile(QString filePath, Hobby hy, QString gifName)
{
    // ★ 空文件检查
    if (filePath.isEmpty()) {
        return;
    }
    QFileInfo info(filePath);
    int64_t fileSize = info.size();
    if (fileSize == 0) {
        emit SIG_UploadMessage("提示", "文件大小为 0，跳过上传");
        return;
    }

    QString fileTypeStr;
    int dotIndex = filePath.lastIndexOf('.');
    if (dotIndex >= 0) {
        fileTypeStr = filePath.right(filePath.length() - dotIndex - 1);
    }

    // 记录当前上传信息（中断时写草稿用）
    m_uploadingLocalPath = filePath;
    m_uploadingFileSize = fileSize;
    m_uploadingPos = 0;

    // 1. 发 RESUME_RQ
    STRU_UPLOAD_RESUME_RQ rq;
    rq.m_nType = _DEF_PACK_UPLOAD_RESUME_RQ;
    rq.m_UserId = m_id;
    rq.m_nFileId = qrand() % 10000;
    rq.m_nFileSize = fileSize;
    rq.m_nClientUploaded = 0;
    QByteArray fn = info.fileName().toUtf8();
    strncpy(rq.m_szFileName, fn.constData(), _MAX_PATH - 1);
    QByteArray bt = fileTypeStr.toUtf8();
    memcpy(rq.m_szFileType, bt.data(), bt.length());
    if (!gifName.isEmpty()) {
        QFileInfo gi(gifName);
        QByteArray gn = gi.fileName().toLocal8Bit();
        strncpy(rq.m_szGifName, gn.constData(), _MAX_PATH - 1);
    }
    memcpy(rq.m_szHobby, &hy, sizeof(hy));

    // 2. 等待 RESUME_RS
    STRU_UPLOAD_RESUME_RS rs;
    {
        QMutexLocker lock(&m_resumeMutex);
        m_resumePending = true;                       // ★ 先置位
        m_tcp->SendData(0, (char*)&rq, sizeof(rq));   // 再发送
        if (!m_resumeCond.wait(&m_resumeMutex, 15000)) {
            m_resumePending = false;
            saveDraftOnFailure(filePath, fileSize, hy, gifName, fileTypeStr, 0);
            emit SIG_UploadMessage("提示", "上传查询超时，已存为草稿");
            return;
        }
        rs = m_resumeRs;
    }

    int64_t offset = rs.m_nResumeFrom;
    m_uploadingPos = offset;

    // 3. 打开本地文件，从 offset 开始读
    QFile src(filePath);
    if (!src.open(QIODevice::ReadOnly)) {
        emit SIG_UploadMessage("提示", "无法打开本地文件");
        return;
    }
    src.seek(offset);
    if (offset > 0) {
        emit SIG_updateProcess(offset, fileSize);
    }

    // ★ 提前设置 UPLOAD_RS pending（防"回复先到"竞态）
    {
        QMutexLocker lock(&m_uploadRsMutex);
        m_uploadRsPending = true;
    }

    // 4. 循环发块
    int64_t pos = offset;
    int fileId = rs.m_nFileId;
    STRU_FILEBLOCK_RQ blockrq;
    bool netOk = true;
    while (pos < fileSize) {
        // ★ 检查退出标志（用户关闭程序）
        if (m_quitFlag) {
            src.close();
            saveDraftOnFailure(filePath, fileSize, hy, gifName, fileTypeStr, pos);
            return;
        }
        int64_t r = src.read(blockrq.m_szFileContent, _DEF_CONTENT_SIZE);
        if (r <= 0) break;
        blockrq.m_nType = _DEF_PACK_FILEBLOCK_RQ;
        blockrq.m_nUserId = m_id;
        blockrq.m_nFileId = fileId;
        blockrq.m_nBlockLen = (int)r;
        blockrq.m_nOffset = pos;  // ★ 续传核心
        if (m_tcp->SendData(0, (char*)&blockrq, sizeof(blockrq)) <= 0) {
            netOk = false;
            break;
        }
        pos += r;
        m_uploadingPos = pos;
        emit SIG_updateProcess(pos, fileSize);
    }
    src.close();

    // 5. 失败 → 写草稿
    if (!netOk) {
        saveDraftOnFailure(filePath, fileSize, hy, gifName, fileTypeStr, pos);
        emit SIG_UploadMessage("提示",
            QString("网络中断，上传已暂停（已传 %1 / %2 字节），已存入草稿箱")
            .arg(pos).arg(fileSize));
        return;
    }

    // 6. 成功 → 等 UPLOAD_RS（pending 已在循环前设置）
    STRU_UPLOAD_RS uploadRs;
    {
        QMutexLocker lock(&m_uploadRsMutex);
        if (!m_uploadRsCond.wait(&m_uploadRsMutex, 60000)) {
            m_uploadRsPending = false;
            saveDraftOnFailure(filePath, fileSize, hy, gifName, fileTypeStr, pos);
            emit SIG_UploadMessage("提示", "等待完成确认超时，已存为草稿");
            return;
        }
        uploadRs = m_uploadRs2;
    }
    if (uploadRs.m_nResult == 1) {
        UploadDraftManager::instance()->removeDraft(m_id, filePath);
        emit SIG_UploadMessage("提示", "上传成功");
    } else {
        saveDraftOnFailure(filePath, fileSize, hy, gifName, fileTypeStr, pos);
        emit SIG_UploadMessage("提示", "上传失败，已存为草稿");
    }
    m_uploadingLocalPath.clear();
}

// 辅助：存草稿
void OnlineDialog::saveDraftOnFailure(const QString& filePath, int64_t fileSize,
                                     const Hobby& hy, const QString& gifName,
                                     const QString& fileType, int64_t uploadedBytes)
{
    UploadDraft d;
    d.localPath = filePath;
    d.fileName = QFileInfo(filePath).fileName();
    d.fileSize = fileSize;
    d.uploadedBytes = uploadedBytes;
    d.hobby.resize(DEF_HOBBY_COUNT);
    memcpy(d.hobby.data(), &hy, sizeof(hy));
    d.gifName = gifName.isEmpty() ? QString() : QFileInfo(gifName).fileName();
    d.fileType = fileType;
    d.createTime = QDateTime::currentDateTime();
    d.status = "paused";
    UploadDraftManager::instance()->addDraft(m_id, d);
}

//点击上传视频
void OnlineDialog::on_pb_upload_clicked()
{
   if(m_id==0){
       QMessageBox::about(this,"提示","先登录");
       return;
   }
    m_uploadDialog->clear();
    m_uploadDialog->show();
}

void OnlineDialog::slot_PlayClicked()
{
    movielable *pb_play=(movielable*)QObject::sender();
    if(pb_play && !pb_play->rtmpUrl().isEmpty())
        Q_EMIT SIG_PlayUrl(pb_play->rtmpUrl());
}

void OnlineDialog::slot_MyPlayClicked()
{
    movielable *pb_play = (movielable*)QObject::sender();
    if(pb_play && !pb_play->rtmpUrl().isEmpty())
        Q_EMIT SIG_PlayUrl(pb_play->rtmpUrl());  // 复用已有的播放信号
}

//工作者上传流程
void UploadWork::slot_UploadFile(QString filePath, QString imgPath, Hobby hy)
{
    OnlineDialog::m_online->slot_UploadFile(filePath,imgPath,hy);
}

void OnlineDialog::on_pb_fresh_clicked()
{
    if(!m_id)
    {
        QMessageBox::about(this,"提示","先登录");
        return;
    }
    clearVideoLabels("pb_play");
    STRU_DOWNLOAD_RQ rq;
    rq.m_nUserId=m_id;
    m_tcp->SendData(0,(char*)&rq,sizeof(rq));
}


void OnlineDialog::on_pb_video_clicked()
{
    ui->sw_page->setCurrentIndex(0);  // 切回推荐影视页
}


void OnlineDialog::on_pb_uploadHistory_clicked()
{
    if(m_id == 0) {
        QMessageBox::about(this, "提示", "先登录");
        return;
    }
    ui->sw_page->setCurrentIndex(1);  // 切到 page_2（上传历史页）
    clearVideoLabels("pb_myplay");

    // 发送上传历史请求
    STRU_UPLOADHISTORY_RQ rq;
    rq.m_nUserId = m_id;
    m_tcp->SendData(0, (char*)&rq, sizeof(rq));
}

// ===== 断点续传新增函数 =====

// GUI 线程弹窗（接收 worker 线程的信号）
void OnlineDialog::slot_ShowUploadMessage(QString title, QString text)
{
    QMessageBox::information(this, title, text);
}

// 点击"草稿箱"按钮
void OnlineDialog::on_pb_draft_clicked()
{
    if (m_id == 0) {
        QMessageBox::about(this, "提示", "先登录");
        return;
    }
    ui->sw_page->setCurrentIndex(2);
    refreshDraftList();
}

// 刷新草稿列表
void OnlineDialog::refreshDraftList()
{
    ui->lw_drafts->clear();
    QList<UploadDraft> drafts = UploadDraftManager::instance()->loadDrafts(m_id);
    for (int i = 0; i < drafts.size(); ++i) {
        const UploadDraft& d = drafts[i];
        QFileInfo fi(d.localPath);
        QString fileState;
        if (!fi.exists()) {
            fileState = " [本地文件已删除]";
        } else if (fi.size() != d.fileSize) {
            fileState = QString(" [文件大小已变: %1 -> %2]")
                          .arg(d.fileSize).arg(fi.size());
        }
        double percent = d.fileSize > 0
            ? (double)d.uploadedBytes * 100.0 / (double)d.fileSize
            : 0.0;
        QString text = QString("%1\n  已传 %2 / %3 字节  (%4%%)%5\n  暂停于 %6")
            .arg(d.fileName)
            .arg(d.uploadedBytes)
            .arg(d.fileSize)
            .arg(percent, 0, 'f', 1)
            .arg(fileState)
            .arg(d.createTime.toString("yyyy-MM-dd hh:mm:ss"));
        QListWidgetItem* item = new QListWidgetItem(text, ui->lw_drafts);
        item->setData(Qt::UserRole, d.localPath);
        item->setToolTip(d.localPath);
    }
    if (drafts.isEmpty()) {
        QListWidgetItem* item = new QListWidgetItem("（暂无草稿）", ui->lw_drafts);
        item->setFlags(Qt::NoItemFlags);
    }
}

// 右键菜单
void OnlineDialog::slot_ShowDraftMenu(const QPoint &pos)
{
    QListWidgetItem* item = ui->lw_drafts->itemAt(pos);
    if (!item) return;
    if (item->flags() == Qt::NoItemFlags) return;
    QMenu menu(this);
    QAction* actContinue = menu.addAction("继续上传");
    QAction* actDelete = menu.addAction("删除草稿");
    QAction* selected = menu.exec(ui->lw_drafts->mapToGlobal(pos));
    if (selected == actContinue) slot_ContinueDraft();
    else if (selected == actDelete) slot_DeleteDraft();
}

// 双击 → 继续上传（★ 发信号给 worker 线程，不直接调）
void OnlineDialog::slot_ContinueDraft()
{
    QListWidgetItem* item = ui->lw_drafts->currentItem();
    if (!item) return;
    QString localPath = item->data(Qt::UserRole).toString();
    if (localPath.isEmpty()) return;
    // ★ 发信号触发 worker 线程，不能直接调 ContinueUpload（会阻塞 GUI 线程）
    emit SIG_ContinueUpload(localPath);
}

// 右键 → 删除
void OnlineDialog::slot_DeleteDraft()
{
    QListWidgetItem* item = ui->lw_drafts->currentItem();
    if (!item) return;
    QString localPath = item->data(Qt::UserRole).toString();
    if (localPath.isEmpty()) return;
    if (QMessageBox::question(this, "确认",
        QString("确定删除草稿\n%1 ？").arg(localPath))
        != QMessageBox::Yes) return;
    UploadDraftManager::instance()->removeDraft(m_id, localPath);
    refreshDraftList();
}

// 启动续传（★ 在 worker 线程执行，由 UploadWork::slot_ContinueUpload 调用）
void OnlineDialog::ContinueUpload(const QString& localPath)
{
    QFileInfo fi(localPath);
    if (!fi.exists()) {
        emit SIG_UploadMessage("提示", "本地文件已不存在，无法续传。");
        return;
    }

    QList<UploadDraft> drafts = UploadDraftManager::instance()->loadDrafts(m_id);
    int idx = -1;
    for (int i = 0; i < drafts.size(); ++i) {
        if (drafts[i].localPath == localPath) { idx = i; break; }
    }
    if (idx < 0) {
        emit SIG_UploadMessage("提示", "找不到对应的草稿记录");
        return;
    }
    const UploadDraft& d = drafts[idx];

    // 文件大小校验
    if (fi.size() != d.fileSize) {
        emit SIG_UploadMessage("提示",
            QString("文件大小已变化（原 %1 字节，现 %2 字节），无法续传。")
            .arg(d.fileSize).arg(fi.size()));
        return;
    }

    m_uploadingLocalPath = localPath;
    m_uploadingFileSize = fi.size();
    m_uploadingPos = 0;

    // 1. 发 RESUME_RQ
    STRU_UPLOAD_RESUME_RQ rq;
    rq.m_nType = _DEF_PACK_UPLOAD_RESUME_RQ;
    rq.m_UserId = m_id;
    rq.m_nFileId = qrand() % 10000;
    rq.m_nFileSize = fi.size();
    rq.m_nClientUploaded = d.uploadedBytes;
    QByteArray fn = d.fileName.toUtf8();
    strncpy(rq.m_szFileName, fn.constData(), _MAX_PATH - 1);
    QByteArray gn = d.gifName.toUtf8();
    strncpy(rq.m_szGifName, gn.constData(), _MAX_PATH - 1);
    QByteArray ft = d.fileType.toUtf8();
    strncpy(rq.m_szFileType, ft.constData(), _MAX_SIZE - 1);
    if (d.hobby.size() >= DEF_HOBBY_COUNT) {
        memcpy(rq.m_szHobby, d.hobby.data(), DEF_HOBBY_COUNT);
    }

    // 2. 等 RESUME_RS
    STRU_UPLOAD_RESUME_RS rs;
    {
        QMutexLocker lock(&m_resumeMutex);
        m_resumePending = true;
        m_tcp->SendData(0, (char*)&rq, sizeof(rq));
        if (!m_resumeCond.wait(&m_resumeMutex, 15000)) {
            m_resumePending = false;
            emit SIG_UploadMessage("提示", "续传查询超时，请检查网络");
            return;
        }
        rs = m_resumeRs;
    }

    if (rs.m_nResult == upload_resume_mismatch) {
        emit SIG_UploadMessage("提示", "服务端记录与本地不一致，无法续传");
        return;
    }
    if (rs.m_nResult == upload_resume_fail) {
        emit SIG_UploadMessage("提示", "续传查询失败");
        return;
    }

    int64_t resumeFrom = rs.m_nResumeFrom;
    m_uploadingPos = resumeFrom;

    // 3. 打开本地文件
    QFile src(localPath);
    if (!src.open(QIODevice::ReadOnly)) {
        emit SIG_UploadMessage("提示", "无法打开本地文件");
        return;
    }
    src.seek(resumeFrom);

    // ★ 提前设置 UPLOAD_RS pending
    {
        QMutexLocker lock(&m_uploadRsMutex);
        m_uploadRsPending = true;
    }

    // 4. 循环发块
    int fileId = rs.m_nFileId;
    int64_t pos = resumeFrom;
    int64_t fileSize = fi.size();
    STRU_FILEBLOCK_RQ blockrq;
    bool netOk = true;
    while (pos < fileSize) {
        if (m_quitFlag) {
            src.close();
            UploadDraft nd = d;
            nd.uploadedBytes = pos;
            nd.createTime = QDateTime::currentDateTime();
            UploadDraftManager::instance()->addDraft(m_id, nd);
            return;
        }
        int64_t r = src.read(blockrq.m_szFileContent, _DEF_CONTENT_SIZE);
        if (r <= 0) break;
        blockrq.m_nType = _DEF_PACK_FILEBLOCK_RQ;
        blockrq.m_nUserId = m_id;
        blockrq.m_nFileId = fileId;
        blockrq.m_nBlockLen = (int)r;
        blockrq.m_nOffset = pos;
        if (m_tcp->SendData(0, (char*)&blockrq, sizeof(blockrq)) <= 0) {
            netOk = false;
            break;
        }
        pos += r;
        m_uploadingPos = pos;
        emit SIG_updateProcess(pos, fileSize);
    }
    src.close();

    // 5. 失败 → 更新草稿
    if (!netOk) {
        UploadDraft nd = d;
        nd.uploadedBytes = pos;
        nd.createTime = QDateTime::currentDateTime();
        UploadDraftManager::instance()->addDraft(m_id, nd);
        emit SIG_UploadMessage("提示", "网络中断，上传已暂停，已存入草稿箱");
        return;
    }

    // 6. 成功 → 等 UPLOAD_RS
    STRU_UPLOAD_RS uploadRs;
    {
        QMutexLocker lock(&m_uploadRsMutex);
        if (!m_uploadRsCond.wait(&m_uploadRsMutex, 60000)) {
            m_uploadRsPending = false;
            emit SIG_UploadMessage("提示", "等待完成确认超时");
            return;
        }
        uploadRs = m_uploadRs2;
    }
    if (uploadRs.m_nResult == 1) {
        UploadDraftManager::instance()->removeDraft(m_id, localPath);
        emit SIG_UploadMessage("提示", "上传成功");
    } else {
        emit SIG_UploadMessage("提示", "上传失败");
    }
    m_uploadingLocalPath.clear();
}

// 收到 RESUME_RS（GUI 线程）
void OnlineDialog::slot_ResumeRs(unsigned int lSendIP, char *buf, int nlen)
{
    QMutexLocker lock(&m_resumeMutex);
    if (m_resumePending && nlen >= (int)sizeof(STRU_UPLOAD_RESUME_RS)) {
        m_resumeRs = *(STRU_UPLOAD_RESUME_RS*)buf;
        m_resumePending = false;
        m_resumeCond.wakeOne();
    }
}

// 收到 UPLOAD_RS（GUI 线程，配合续传同步原语）
void OnlineDialog::slot_UploadRs2(unsigned int lSendIP, char *buf, int nlen)
{
    QMutexLocker lock(&m_uploadRsMutex);
    if (m_uploadRsPending && nlen >= (int)sizeof(STRU_UPLOAD_RS)) {
        m_uploadRs2 = *(STRU_UPLOAD_RS*)buf;
        m_uploadRsPending = false;
        m_uploadRsCond.wakeOne();
    }
}

// ★ worker 线程接收草稿续传信号
void UploadWork::slot_ContinueUpload(QString localPath)
{
    OnlineDialog::m_online->ContinueUpload(localPath);
}
