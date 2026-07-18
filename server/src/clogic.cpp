#include "clogic.h"

void CLogic::setNetPackMap()
{
    NetPackMap(_DEF_PACK_REGISTER_RQ)    = &CLogic::RegisterRq;
    NetPackMap(_DEF_PACK_LOGIN_RQ)       = &CLogic::LoginRq;
    NetPackMap(_DEF_PACK_UPLOAD_RQ)      =&CLogic::UploadRq;
    NetPackMap(_DEF_PACK_FILEBLOCK_RQ)   =&CLogic::UploadBlockRq;
    NetPackMap(DEF_PACK_DOWNLOAD_RQ)     =&CLogic::DownloadRq;
    NetPackMap(_DEF_PACK_UPLOADHISTORY_RQ) = &CLogic::UploadHistoryRq;
    NetPackMap(_DEF_PACK_LIVE_START_RQ) = &CLogic::LiveStartRq;
    NetPackMap(_DEF_PACK_LIVE_STOP_RQ)  = &CLogic::LiveStopRq;
    NetPackMap(_DEF_PACK_LIVE_LIST_RQ)  = &CLogic::LiveListRq;
}

#define RootPath "/home/colin/video/"
#define _DEF_COUT_FUNC_    cout << "clientfd:"<< clientfd << __func__ << endl;

void CLogic::close()
{
    for(auto ite=m_mapFileIDToFileInfo.begin();ite!=m_mapFileIDToFileInfo.end();++ite)
    {
        delete ite->second;
    }
    m_mapFileIDToFileInfo.clear();
    m_mapIDToUserFD.clear();
}

//注册
void CLogic::RegisterRq(sock_fd clientfd,char* szbuf,int nlen)
{
    _DEF_COUT_FUNC_
   
    STRU_REGISTER_RQ*rq=(STRU_REGISTER_RQ*)szbuf;
    STRU_REGISTER_RS rs;

    char sqlBuf[ _DEF_SQLIEN]=" ";
    sprintf(sqlBuf,"select name from t_UserData where name='%s';",rq->user);
    list<string> resList;
    bool res=m_sql->SelectMysql(sqlBuf,1,resList);
    if(!res){
        cout<<"SelectMysql error:"<<sqlBuf<<endl;
        return;
    }

    if(resList.size()>0){
        rs.result=user_is_exist;
    }else{
        char sqlBuf[ _DEF_SQLIEN]=" ";
        sprintf(sqlBuf,"insert into t_UserData(name,password,food,funny,ennegy,dance,music,video,outside,edu) values('%s','%s',%d,%d,%d,%d,%d,%d,%d,%d);",
                rq->user,rq->password,rq->food,rq->funny,rq->ennegy,rq->dance,rq->music,rq->video,rq->outside,rq->edu);
        m_sql->UpdataMysql(sqlBuf);

        sprintf(sqlBuf,"select id from t_UserData where name='%s';",rq->user);
        list<string> resID;
        m_sql->SelectMysql(sqlBuf,1,resID);
        int id=0;
        if(resID.size()>0){
            id=atoi(resID.front().c_str());
        }
       // rs.userid=id;

        //新注册的用户创建一个路径
        char path[_MAX_PATH]="";
        sprintf(path,"%sflv/%s/",RootPath,rq->user); //home/colin/video/flv/uer

        umask(0);
        mkdir(path,S_IRWXU|S_IRWXG|S_IRWXO);

        rs.result=register_success;
    }
    m_tcp->SendData(clientfd,(char*)&rs,sizeof(rs));
}

//登录
void CLogic::LoginRq(sock_fd clientfd ,char* szbuf,int nlen)
{
    _DEF_COUT_FUNC_
    STRU_LOGIN_RQ*rq=(STRU_LOGIN_RQ*)szbuf;
    STRU_LOGIN_RS rs;
    char sqlBuf[ _DEF_SQLIEN]=" ";
    sprintf(sqlBuf,"select password,id from t_UserData where name='%s';",rq->user);
    list<string> resList;
    bool res=m_sql->SelectMysql(sqlBuf,2,resList);
    if(!res){
        cout<<"SelectMysql error:"<<sqlBuf<<endl;
        return;
    }
    if(resList.size()>0){
        if(strcmp(resList.front().c_str(),rq->password)==0){
            rs.result=login_success;
            resList.pop_front();
            rs.userid=atoi(resList.front().c_str());
            m_tcp->SendData(clientfd,(char*)&rs,sizeof(rs));

            //存储映射关系
            this->m_mapIDToUserFD[rs.userid]=clientfd;
        }else{
            rs.result=password_error;
        }
    }else{
        rs.result=user_not_exist;
    }

    m_tcp->SendData( clientfd , (char*)&rs , sizeof rs );
}
//上传请求
void CLogic::UploadRq(sock_fd clientfd, char *szbuf, int nlen)
{
    _DEF_COUT_FUNC_

    STRU_UPLOAD_RQ *rq=(STRU_UPLOAD_RQ*)szbuf;

    FileInfo *info=new FileInfo;
    info->m_nPos=0;
    memcpy(info->m_Hobby,rq->m_szHobby,DEF_HOBBY_COUNT);

    info->m_nUserId = rq->m_UserId;
    info->m_nFileID = rq->m_nFileId;
    info->m_VideoID = 0;

    info->m_nFileSize = rq->m_nFileSize;


    strcpy(info->m_szFileName, rq->m_szFileName);
    strcpy(info->m_szFileType, rq->m_szFileType);

    //找到用户名
    char sqlBuf[_DEF_SQLIEN] = "";
    sprintf(sqlBuf, "select name from t_UserData where id = %d;", info->m_nUserId);
    list<string> resList;
    if(!m_sql->SelectMysql(sqlBuf, 1, resList))
    {
        cout<<"SelectMysql error:"<<sqlBuf<<endl;
        delete info;
        return;
    }
    if(resList.size()<=0)
    {
        delete info;
        return;
    }

    strcpy(info->m_UserName,resList.front().c_str());
    sprintf(info->m_szFilePath,"%sflv/%s/%s",RootPath,info->m_UserName,info->m_szFileName);
    sprintf( info->m_szRtmp,"//%s/%s",info->m_UserName,info->m_szFileName);

    if(strcmp(rq->m_szFileType, "gif") != 0)
    {
        strcpy(info->m_szGifName, rq->m_szGifName);
        sprintf(info->m_szGifPath,"%sflv/%s/%s",RootPath,info->m_UserName,info->m_szGifName);
    }
   info->pFile=fopen(info->m_szFilePath,"w");

   m_mapFileIDToFileInfo[info->m_nFileID]=info;
}
//上传的文件块
void CLogic::UploadBlockRq(sock_fd clientfd, char *szbuf, int nlen)
{
     _DEF_COUT_FUNC_

     STRU_FILEBLOCK_RQ *rq = (STRU_FILEBLOCK_RQ *)szbuf;
     if (m_mapFileIDToFileInfo.find(rq->m_nFileId) == m_mapFileIDToFileInfo.end())
         return;

     FileInfo* info = m_mapFileIDToFileInfo[rq->m_nFileId];

     int64_t res = fwrite(rq->m_szFileContent, 1, rq->m_nBlockLen, info->pFile);
     info->m_nPos += res;

     if (rq->m_nBlockLen < _DEF_CONTENT_SIZE || info->m_nPos >= info->m_nFileSize)
     {
         // 写完了
         fclose(info->pFile);

         // 判断 不是gif 写表记录  返回信息
         if (strcmp(info->m_szFileType, "gif") != 0)
         {
             // 写表
             char sqlBuf[_DEF_SQLIEN] = "";
             sprintf(sqlBuf,
                     "INSERT INTO t_VideoInfo (userId, videoName, picName, videoPath, picPath, rtmp, food, funny, ennegy, dance, music, video, outside, edu, hotdegree) values (%d, '%s', '%s', '%s', '%s', '%s', %d, %d, %d, %d, %d, %d, %d, %d, %d);"
                     ,rq->m_nFileId,info->m_szFileName,info->m_szGifName,info->m_szFilePath,info->m_szGifPath,info->m_szRtmp,
                     info->m_Hobby[0],info->m_Hobby[1],info->m_Hobby[2],info->m_Hobby[3],info->m_Hobby[4],info->m_Hobby[5],info->m_Hobby[6],info->m_Hobby[7], 0);
             if(!m_sql->UpdataMysql(sqlBuf)){
                   cout<<"UpdataMysql error:"<<sqlBuf<<endl;
             }
             // 返回
             STRU_UPLOAD_RS rs;
             rs.m_nResult = 1;
             m_tcp->SendData(clientfd,(char*)&rs, sizeof(rs));
         }

         m_mapFileIDToFileInfo.erase(rq->m_nFileId);
         delete info;
         info = NULL;
     }
}

void CLogic::DownloadRq(sock_fd clientfd, char *szbuf, int nlen)
{
     _DEF_COUT_FUNC_
             // 1. 校验接收的长度是否足够装下结构体
                 if (nlen < sizeof(STRU_DOWNLOAD_RQ))
                 {
                     printf("非法包：长度不够，丢弃！nlen=%d\n", nlen);
                     return;
                 }
                 // 2. 校验指针不能为空
                 if (!szbuf)
                 {
                     printf("szbuf 为空！\n");
                     return;
                 }
     STRU_DOWNLOAD_RQ *rq=(STRU_DOWNLOAD_RQ*)szbuf;
     list<FileInfo*> fileList;

     GetFileList(fileList,rq->m_nUserId);

     while(fileList.size()>0)
     {
         FileInfo *info=fileList.front();
         fileList.pop_front();

         STRU_DOWNLOAD_RS rs;
         strcpy( rs.m_rtmp , info->m_szRtmp );
         rs.m_nFileId = info->m_nFileID;
         rs.m_nVideoId = info->m_VideoID;
         rs.m_nFileSize = info->m_nFileSize;
         strcpy( rs.m_szFileName , info->m_szFileName );

         m_tcp->SendData( clientfd , (char*)&rs , sizeof(rs) );
         cout<<"Send  STRU_DOWNLOAD_RS: "<<endl;

         info->pFile = fopen( info->m_szFilePath , "r");
         if( !info->pFile )
         {
             cout<<"Open file failed: "<<info->m_szFilePath<<endl;
             delete info;
             continue;
         }
         if( info->pFile )
         {
             while(1)
             {
                 STRU_FILEBLOCK_RQ blockrq;
                 cout<<"STRU_FILEBLOCK_RQ"<<endl;
                 int64_t res = fread( blockrq.m_szFileContent, 1 ,1024 , info->pFile );
                 if(res <= 0)
                 {
                     cout<<"res<=0"<<endl;
                     // 读取结束或出错
                     break;
                 }
                 blockrq.m_nBlockLen = res;
                 info->m_nPos += res;
                 blockrq.m_nFileId=info->m_VideoID;
                 blockrq.m_nUserId=rq->m_nUserId;

                 int ret = m_tcp->SendData(clientfd, (char*)&blockrq, sizeof(blockrq));
                 if (ret < 0) {
                     cout << "SendData failed, client disconnected" << endl;
                     break; // 退出循环，不再发送
                 }

                 if( info->m_nPos >= info->m_nFileSize )
                 {
                     fclose(info->pFile);
                     delete info;
                     info = NULL;
                     break;
                 }
             }
         }
     }
     cout<<"All video send done, close client: "<<clientfd<<endl;
}

void CLogic::UploadHistoryRq(sock_fd clientfd, char *szbuf, int nlen)
{
    STRU_UPLOADHISTORY_RQ *rq = (STRU_UPLOADHISTORY_RQ*)szbuf;

    char sqlBuf[1024] = "";
    list<string> resList;

    // 查询该用户上传的所有视频
    sprintf(sqlBuf,
        "select videoId, picName, picPath, rtmp from t_VideoInfo where userId = %d order by hotdegree desc;",
        rq->m_nUserId);

    if(!m_sql->SelectMysql(sqlBuf, 4, resList)) {
        cout << "SelectMysql error:" << sqlBuf << endl;
        return;
    }

    int nCount = 0;
    int nSize = resList.size() / 4;

    for(int i = 0; i < nSize; ++i)
    {
        FileInfo *info = new FileInfo;
        info->m_nPos = 0;

        info->m_VideoID = atoi(resList.front().c_str());
        resList.pop_front();

        strcpy(info->m_szFileName, resList.front().c_str());
        resList.pop_front();

        strcpy(info->m_szFilePath, resList.front().c_str());
        resList.pop_front();

        strcpy(info->m_szRtmp, resList.front().c_str());
        resList.pop_front();

        info->m_nFileID = nCount++;

        // 只发送 GIF 缩略图（picPath），不需要发送视频本体
        // 构造 GIF 的文件路径
        info->pFile = fopen(info->m_szFilePath, "r");  // picPath 是 GIF 路径
        if(!info->pFile) {
            delete info;
            continue;
        }
        fseek(info->pFile, 0, SEEK_END);
        info->m_nFileSize = ftell(info->pFile);
        fseek(info->pFile, 0, SEEK_SET);

        // 发送回复头（复用 STRU_DOWNLOAD_RS）
        STRU_DOWNLOAD_RS rs;
        strcpy(rs.m_szFileName, info->m_szFileName);
        rs.m_nFileId = info->m_nFileID;
        rs.m_nVideoId = info->m_VideoID;
        rs.m_nFileSize = info->m_nFileSize;
        strcpy(rs.m_rtmp, info->m_szRtmp);
        m_tcp->SendData(clientfd, (char*)&rs, sizeof(rs));

        // 发送 GIF 文件块
        while(1)
        {
            STRU_FILEBLOCK_RQ blockrq;
            int64_t res = fread(blockrq.m_szFileContent, 1, 1024, info->pFile);
            if(res <= 0) break;

            blockrq.m_nBlockLen = res;
            info->m_nPos += res;
            blockrq.m_nFileId = info->m_VideoID;
            blockrq.m_nUserId = rq->m_nUserId;

            m_tcp->SendData(clientfd, (char*)&blockrq, sizeof(blockrq));

            if(info->m_nPos >= info->m_nFileSize) break;
        }

        fclose(info->pFile);
        delete info;
    }
}

void CLogic::GetFileList(list<FileInfo*>&fileList,int userid)
{
    cout<<"CLogic::GetFileList"<<endl;

    char sqlBuf[1024] = "";
    list<string> resList;
    cout << "[GetFileList] Start, userid: " << userid << endl;
    sprintf(sqlBuf,"select count(videoId) from t_VideoInfo where t_VideoInfo.videoId not in ( select t_UserRecv.videoId from t_UserRecv where t_UserRecv.userId = %d);",userid);

    int nCount=0;

    if(!m_sql->SelectMysql(sqlBuf,1,resList))
    {
        cout<<"SelectMysql error:"<<sqlBuf<<endl;
        return;
    }
    if(resList.size()==0)
    {
        cout<<"[GetFileList] Count result empty!"<<endl;
        return;

        }
    nCount=atoi(resList.front().c_str());
    cout << "[GetFileList] Video count: " << nCount << endl;
    if(nCount==0)
    {
        cout<<"[GetFileList] No new video, delete t_UserRecv"<<endl;
        sprintf(sqlBuf,"delete from t_UserRecv where userId = %d",userid);
        cout << "[GetFileList] Select SQL: " << sqlBuf << endl;
        if(!m_sql->UpdataMysql(sqlBuf)){
              cout<<"UpdataMysql error:"<<sqlBuf<<endl;
              return;
        }
    }

    resList.clear();
    sprintf(sqlBuf,"select videoId ,picName , picPath , rtmp from t_VideoInfo where t_VideoInfo.videoId not in( select t_UserRecv.videoId from t_UserRecv where t_UserRecv.userId = %d) order by hotdegree desc limit 0,10;",userid);
    if(!m_sql->SelectMysql(sqlBuf,4,resList))
    {
        cout<<"SelectMysql error:"<<sqlBuf<<endl;
        return;
    }
    cout << "[GetFileList] Select result count: " << resList.size() << endl;
    nCount=0;
    int nSize=resList.size()/4;
    for(int i=0 ;i < nSize ; ++i)
    {
        cout << "[GetFileList] Parsing video " << i << endl;
        FileInfo *info=new FileInfo;

        info->m_nPos=0;
        info->m_VideoID=atoi(resList.front().c_str());
        resList.pop_front();
        cout << "  videoId: " << info->m_VideoID << endl;

        strcpy(info->m_szFileName,resList.front().c_str());
        resList.pop_front();
        cout << "  fileName: " << info->m_szFileName << endl;

        strcpy(info->m_szFilePath,resList.front().c_str());
        resList.pop_front();
        cout << "  filePath: " << info->m_szFilePath << endl;

        strcpy(info->m_szRtmp,resList.front().c_str());
        resList.pop_front();
        cout << "  rtmp: " << info->m_szRtmp << endl;

        info->m_nFileID=nCount++;

        info->pFile=fopen(info->m_szFilePath,"r");
        if(!info->pFile)
                {
                    cout<<"[GetFileList] Open file failed: "<<info->m_szFilePath<<endl;
                    delete info;
                    continue;
                }
        fseek(info->pFile,0,SEEK_END);
        info->m_nFileSize=ftell(info->pFile);
        fseek(info->pFile,0,SEEK_SET);
        fclose(info->pFile);
        info->pFile=NULL;
        cout << "  fileSize: " << info->m_nFileSize << endl;
        fileList.push_back(info);
        cout << "[GetFileList] Add to fileList, size now: " << fileList.size() << endl;
        sprintf(sqlBuf,"insert into t_UserRecv values(%d ,%d);",userid,info->m_VideoID);
        if(!m_sql->UpdataMysql(sqlBuf)){
              cout<<"UpdataMysql error:"<<sqlBuf<<endl;
              return;
        }
    }
    cout << "[GetFileList] Done, fileList size: " << fileList.size() << endl;
}

// ===== 开始直播 =====
void CLogic::LiveStartRq(sock_fd clientfd, char* szbuf, int nlen)
{
    STRU_LIVE_START_RQ* rq = (STRU_LIVE_START_RQ*)szbuf;
    STRU_LIVE_START_RS  rs;
    rs.m_nType = _DEF_PACK_LIVE_START_RS;

    // 1. 查询用户名
    char sqlBuf[_DEF_SQLIEN] = "";
    sprintf(sqlBuf, "SELECT name FROM t_UserData WHERE id = %d;", rq->m_nUserId);
    list<string> resList;
    if (!m_sql->SelectMysql(sqlBuf, 1, resList) || resList.size() == 0) {
        rs.m_nResult = 0;
        m_tcp->SendData(clientfd, (char*)&rs, sizeof(rs));
        return;
    }
    string userName = resList.front();

    // 2. 生成 streamKey（用 "user_用户ID" 作为唯一标识）
    char streamKey[64] = "";
    sprintf(streamKey, "user_%d", rq->m_nUserId);

    // 3. 若该用户已有直播记录，先清掉（重新开播）
    sprintf(sqlBuf, "DELETE FROM t_LiveStream WHERE userId = %d;", rq->m_nUserId);
    m_sql->UpdataMysql(sqlBuf);

    // 4. 插入新的直播记录
    sprintf(sqlBuf,
        "INSERT INTO t_LiveStream (userId, streamKey, title, startTime, status) "
        "VALUES (%d, '%s', '%s', NOW(), 1);",
        rq->m_nUserId, streamKey, rq->m_szTitle);

    if (!m_sql->UpdataMysql(sqlBuf)) {
        rs.m_nResult = 0;
        m_tcp->SendData(clientfd, (char*)&rs, sizeof(rs));
        return;
    }

    // 5. 返回成功和 streamKey
    rs.m_nResult = 1;
    strcpy(rs.m_szStreamKey, streamKey);
    m_tcp->SendData(clientfd, (char*)&rs, sizeof(rs));
}

// ===== 停止直播 =====
void CLogic::LiveStopRq(sock_fd clientfd, char* szbuf, int nlen)
{
    STRU_LIVE_STOP_RQ* rq = (STRU_LIVE_STOP_RQ*)szbuf;
    STRU_LIVE_STOP_RS  rs;
    rs.m_nType = _DEF_PACK_LIVE_STOP_RS;

    char sqlBuf[_DEF_SQLIEN] = "";
    sprintf(sqlBuf,
        "UPDATE t_LiveStream SET status = 0 WHERE userId = %d AND status = 1;",
        rq->m_nUserId);

    rs.m_nResult = m_sql->UpdataMysql(sqlBuf) ? 1 : 0;
    m_tcp->SendData(clientfd, (char*)&rs, sizeof(rs));
}

// ===== 获取直播列表 =====
void CLogic::LiveListRq(sock_fd clientfd, char* szbuf, int nlen)
{
    // 查询所有正在直播的条目，关联用户名
    char sqlBuf[_DEF_SQLIEN] = "";
    sprintf(sqlBuf,
        "SELECT ls.streamId, ls.userId, u.name, ls.title, ls.streamKey "
        "FROM t_LiveStream ls "
        "JOIN t_UserData u ON ls.userId = u.id "
        "WHERE ls.status = 1 "
        "ORDER BY ls.startTime DESC;");

    list<string> resList;
    if (!m_sql->SelectMysql(sqlBuf, 5, resList)) {
        STRU_LIVE_LIST_END endPkt;
        endPkt.m_nType  = _DEF_PACK_LIVE_LIST_END;
        endPkt.m_nCount = 0;
        m_tcp->SendData(clientfd, (char*)&endPkt, sizeof(endPkt));
        return;
    }

    int count = 0;
    int nSize = resList.size() / 5;

    for (int i = 0; i < nSize; ++i) {
        STRU_LIVE_LIST_RS rs;
        rs.m_nType     = _DEF_PACK_LIVE_LIST_RS;
        rs.m_nStreamId = atoi(resList.front().c_str()); resList.pop_front();
        rs.m_nUserId   = atoi(resList.front().c_str()); resList.pop_front();
        strncpy(rs.m_szAnchorName, resList.front().c_str(), 39); resList.pop_front();
        rs.m_szAnchorName[39] = '\0';
        strncpy(rs.m_szTitle,      resList.front().c_str(), 127); resList.pop_front();
        rs.m_szTitle[127] = '\0';
        strncpy(rs.m_szStreamKey,  resList.front().c_str(), 63);  resList.pop_front();
        rs.m_szStreamKey[63] = '\0';

        m_tcp->SendData(clientfd, (char*)&rs, sizeof(rs));
        ++count;
    }

    // 最后发一个结束包告诉客户端列表发完了
    STRU_LIVE_LIST_END endPkt;
    endPkt.m_nType  = _DEF_PACK_LIVE_LIST_END;
    endPkt.m_nCount = count;
    m_tcp->SendData(clientfd, (char*)&endPkt, sizeof(endPkt));
}
