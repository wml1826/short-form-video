#include "movielable.h"
#include "ui_movielable.h"
#include<QDebug>

movielable::movielable(QWidget *parent) :
    QWidget(parent),m_movie(NULL),
    ui(new Ui::movielable)
{
    ui->setupUi(this);

    //安装事件监听器，让lb_movie识别事件
    ui->lb_movie->installEventFilter(this);
}

movielable::~movielable()
{
    delete ui;
}

QString movielable::rtmpUrl() const
{
    return m_rtmpUrl;
}

QMovie *movielable::movie() const
{
    return m_movie;
}
//设置动画
void movielable::setMovie(QMovie *movie)
{
    m_movie=movie;
    ui->lb_movie->setMovie(movie);
    movie->start();
    movie->stop();
}

void movielable::clearContent()
{
    if(m_movie)
    {
        m_movie->stop();
        ui->lb_movie->setMovie(NULL);
        delete m_movie;
        m_movie=NULL;
    }
    ui->lb_movie->clear();
    m_rtmpUrl.clear();
}
//鼠标移入
void movielable::enterEvent(QEvent *event)
{
    if(m_movie){
        m_movie->start();
    }
}
//鼠标移出
void movielable::leaveEvent(QEvent *event)
{
    if(m_movie){
        m_movie->stop();
    }
}
//设置控件播放url
void movielable::setRtmpUrl(QString url)
{
    m_rtmpUrl=url;
}
//事件过滤处理
bool movielable::eventFilter(QObject *watched, QEvent *event)
{
    //动画点击
    if(watched==ui->lb_movie&&event->type()==QEvent::MouseButtonPress)
    {
        qDebug()<<"mouse Press";
        Q_EMIT SIG_labelClicked();
        return true;
    }
    return QWidget::eventFilter(watched,event);
}
