#ifndef MOVIELABLE_H
#define MOVIELABLE_H

#include <QWidget>
#include <QMovie>
#include <QEvent>

namespace Ui {
class movielable;
}

class movielable : public QWidget
{
    Q_OBJECT

public:
    explicit movielable(QWidget *parent = nullptr);
    ~movielable();

    QString rtmpUrl() const;
    QMovie *movie() const;

signals:
    void SIG_labelClicked();
public slots:
    void setMovie(QMovie *movie);
    void clearContent();
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void setRtmpUrl(QString url);
    virtual bool eventFilter(QObject *watched,QEvent *event);
private:
    Ui::movielable *ui;
    QMovie *m_movie;
    QString m_rtmpUrl;
};

#endif // MOVIELABLE_H
