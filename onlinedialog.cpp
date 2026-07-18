#include "onlinedialog.h"
#include "ui_onlinedialog.h"
#include<QCryptographicHash>
#include<QMessageBox>
#include<QFileInfo>
#include<QDir>
#include"movielable.h"

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

    bool ok = m_tcp->OpenNet("192.168.43.239", 8000);
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
}

OnlineDialog::~OnlineDialog()
{
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
            slot_uploadRs(lSendIP,buf,nlen);
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
//上传文件
void OnlineDialog::UploadFile(QString filePath, Hobby hy,QString gifName)
{
    //兼容中文
    QFileInfo info(filePath);
    QString FileName=info.fileName();
    std::string strName=FileName.toStdString();
    const char* file_name=strName.c_str();


    STRU_UPLOAD_RQ rq;
    rq.m_nFileId=qrand()%10000;
    rq.m_nFileSize=info.size();
    strcpy_s(rq.m_szFileName,_MAX_PATH,file_name);

    int dotIndex = filePath.lastIndexOf('.');
    QString fileTypeStr = filePath.right(filePath.length() - dotIndex - 1);
    QByteArray bt = fileTypeStr.toUtf8();

   // QByteArray bt=filePath.right(filePath.length()-filePath.lastIndexOf('.')-1);
    memcpy(rq.m_szFileType,bt.data(),bt.length());
    if(!gifName.isEmpty()){
        QFileInfo info(gifName);
        strcpy_s(rq.m_szGifName,_MAX_PATH,info.fileName().toLocal8Bit().data());
    }
    memcpy(rq.m_szHobby,&hy,sizeof(hy));
    rq.m_UserId=m_id;

    m_tcp->SendData(0,(char*)&rq,sizeof(rq));

    FileInfo fi;
    fi.fileId=rq.m_nFileId;
    fi.fileName=rq.m_szFileName;
    fi.filePath=filePath;
    fi.filePos=0;
    fi.fileSize=rq.m_nFileSize;
    fi.pFile=new QFile(filePath);
    if(fi.pFile->open(QIODevice::ReadOnly)){
        while(1){
            STRU_FILEBLOCK_RQ blockrq;
            int64_t res=fi.pFile->read(blockrq.m_szFileContent,_DEF_CONTENT_SIZE);
            fi.filePos+=res;
            blockrq.m_nBlockLen=res;
            blockrq.m_nFileId=rq.m_nFileId;
            blockrq.m_nUserId=m_id;

            m_tcp->SendData(0,(char*)&blockrq,sizeof(blockrq));
            emit SIG_updateProcess(fi.filePos,fi.fileSize);
            if(fi.filePos>=fi.fileSize){
                fi.pFile->close();
                delete fi.pFile;
                fi.pFile=NULL;
                break;
            }
        }
    }
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

