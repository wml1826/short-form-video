#ifndef UPLOADDRAFTMANAGER_H
#define UPLOADDRAFTMANAGER_H

#include <QString>
#include <QDateTime>
#include <QMutex>
#include <QList>
#include <QByteArray>

struct UploadDraft
{
    QString localPath;
    QString fileName;
    int64_t fileSize = 0;
    int64_t uploadedBytes = 0;
    QByteArray hobby;
    QString gifName;
    QString fileType;
    QDateTime createTime;
    QString status = "paused";
};

class UploadDraftManager
{
public:
    static UploadDraftManager* instance();

    QList<UploadDraft> loadDrafts(int userId);
    void saveDrafts(int userId, const QList<UploadDraft>& drafts);
    void addDraft(int userId, const UploadDraft& draft);
    void removeDraft(int userId, const QString& localPath);
    void clearAll(int userId);

private:
    UploadDraftManager() {}
    QMutex m_mutex;
    QList<UploadDraft> loadDrafts_unlocked(int userId);
    void saveDrafts_unlocked(int userId, const QList<UploadDraft>& drafts);
};

#endif // UPLOADDRAFTMANAGER_H
