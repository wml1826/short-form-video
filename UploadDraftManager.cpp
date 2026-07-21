#include "UploadDraftManager.h"
#include <QSettings>

UploadDraftManager* UploadDraftManager::instance()
{
    static UploadDraftManager inst;
    return &inst;
}

QList<UploadDraft> UploadDraftManager::loadDrafts_unlocked(int userId)
{
    QList<UploadDraft> list;
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       "MediaPlay", "drafts");
    settings.beginGroup(QString("user_%1").arg(userId));
    int n = settings.beginReadArray("drafts");
    for (int i = 0; i < n; ++i) {
        settings.setArrayIndex(i);
        UploadDraft d;
        d.localPath     = settings.value("localPath").toString();
        d.fileName      = settings.value("fileName").toString();
        d.fileSize      = settings.value("fileSize").toLongLong();
        d.uploadedBytes = settings.value("uploadedBytes").toLongLong();
        d.hobby         = settings.value("hobby").toByteArray();
        d.gifName       = settings.value("gifName").toString();
        d.fileType      = settings.value("fileType").toString();
        d.createTime    = settings.value("createTime").toDateTime();
        d.status        = settings.value("status", "paused").toString();
        if (!d.localPath.isEmpty()) list.append(d);
    }
    settings.endArray();
    settings.endGroup();
    return list;
}

void UploadDraftManager::saveDrafts_unlocked(int userId, const QList<UploadDraft>& drafts)
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       "MediaPlay", "drafts");
    settings.beginGroup(QString("user_%1").arg(userId));
    settings.remove("");
    settings.beginWriteArray("drafts");
    for (int i = 0; i < drafts.size(); ++i) {
        const UploadDraft& d = drafts[i];
        settings.setArrayIndex(i);
        settings.setValue("localPath",     d.localPath);
        settings.setValue("fileName",      d.fileName);
        settings.setValue("fileSize",      (qlonglong)d.fileSize);
        settings.setValue("uploadedBytes", (qlonglong)d.uploadedBytes);
        settings.setValue("hobby",         d.hobby);
        settings.setValue("gifName",       d.gifName);
        settings.setValue("fileType",      d.fileType);
        settings.setValue("createTime",    d.createTime);
        settings.setValue("status",        d.status);
    }
    settings.endArray();
    settings.endGroup();
    settings.sync();
}

QList<UploadDraft> UploadDraftManager::loadDrafts(int userId)
{
    QMutexLocker lock(&m_mutex);
    return loadDrafts_unlocked(userId);
}

void UploadDraftManager::saveDrafts(int userId, const QList<UploadDraft>& drafts)
{
    QMutexLocker lock(&m_mutex);
    saveDrafts_unlocked(userId, drafts);
}

void UploadDraftManager::addDraft(int userId, const UploadDraft& draft)
{
    QMutexLocker lock(&m_mutex);
    QList<UploadDraft> list = loadDrafts_unlocked(userId);
    for (int i = 0; i < list.size(); ++i) {
        if (list[i].localPath == draft.localPath) {
            list.removeAt(i);
            break;
        }
    }
    list.append(draft);
    saveDrafts_unlocked(userId, list);
}

void UploadDraftManager::removeDraft(int userId, const QString& localPath)
{
    QMutexLocker lock(&m_mutex);
    QList<UploadDraft> list = loadDrafts_unlocked(userId);
    for (int i = 0; i < list.size(); ++i) {
        if (list[i].localPath == localPath) {
            list.removeAt(i);
            break;
        }
    }
    saveDrafts_unlocked(userId, list);
}

void UploadDraftManager::clearAll(int userId)
{
    QMutexLocker lock(&m_mutex);
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       "MediaPlay", "drafts");
    settings.remove(QString("user_%1").arg(userId));
    settings.sync();
}
