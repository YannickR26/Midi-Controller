#ifndef TMEDIAPLAYER_H
#define TMEDIAPLAYER_H

#include <QObject>
#include <QMediaPlayer>
#include <QMediaPlaylist>

class tMediaPlayer : public QObject
{
    Q_OBJECT

public:
    enum State
    {
        StoppedState,
        PlayingState,
        PausedState
    };

    enum MediaStatus
    {
        UnknownMediaStatus,
        NoMedia,
        LoadingMedia,
        LoadedMedia,
        StalledMedia,
        BufferingMedia,
        BufferedMedia,
        EndOfMedia,
        InvalidMedia
    };

    tMediaPlayer(uint8_t numberMediaPlayer = 2, QObject *parent = 0);

    bool clear();
    bool addMedia(const QMediaContent &content);
    bool addMedia(const QList<QMediaContent> &items);
    int currentIndex();

    State state();
    MediaStatus mediaStatus();

    bool isMetaDataAvailable();
    QVariant metaData(const QString &key);

    qint64 duration();
    qint64 position();

    QMediaPlaylist *getCurrentPlaylist() const {
        return currentPlaylist;
    }

    void setIsSingle(bool value_) {
        this->isSingle = value_;
    }

    void setTimeTransition(int time_) {
        this->timeFading = time_;
    }

public Q_SLOTS:
    void onPositionChanged(qint64 position_);
    void onDurationChanged(qint64 duration_);
    void onMetaDataChanged();
    void onCurrentIndexChanged(int newIndex_);
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status_);

    void play();
    void pause();
    void stop();

    void setPosition(qint64 position_);

    void next();
    void previous();
    void setCurrentIndex(int index);

Q_SIGNALS:
    void positionChanged(qint64 position_);
    void durationChanged(qint64 duration_);
    void metaDataChanged();
    void currentIndexChanged(int newIndex_);

private:
    QList<QMediaPlayer *>       listMediaPlayer;
    QMediaPlaylist *            currentPlaylist;
    uint8_t                     currentPlayerIndex;
    qint64                      currentDuration;
    qint16                      timeFading;
    bool                        isSingle;
};

#endif // TMEDIAPLAYER_H
