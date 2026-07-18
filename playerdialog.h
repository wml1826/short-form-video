#ifndef PLAYERDIALOG_H
#define PLAYERDIALOG_H

#include <QDialog>
#include "videoplayer.h"
#include <QTimer>
#include"onlinedialog.h"
#include "livedialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class PlayerDialog; }
QT_END_NAMESPACE

class PlayerDialog : public QDialog
{
    Q_OBJECT

public:
    PlayerDialog(QWidget *parent = nullptr);
    ~PlayerDialog();

private slots:

    void on_pb_start_clicked();

    void slot_setOneImage(QImage img);

    void on_pb_resume_clicked();

    void on_pb_pause_clicked();

    void on_pb_stop_clicked();

    void slot_PlayerStateChanged(int state);

    void slot_getTotalTime(qint64 uSec);

    void slot_TimerTimeOut();

    //事件过滤器
    bool eventFilter(QObject *obj,QEvent *event);
    void on_pb_online_clicked();
    void on_pb_live_clicked();
    void slot_PlayUrl(QString url);
private:
    Ui::PlayerDialog *ui;

    VideoPlayer* m_player;
    QTimer m_timer;
    OnlineDialog *m_onlineDialog;
    LiveDialog *m_liveDialog;

    //停止的状态
    bool isStop;

};
#endif // PLAYERDIALOG_H
