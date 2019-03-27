#include "tmediaplayer.h"
#include "common.h"

tMediaPlayer::tMediaPlayer(uint8_t numberMediaPlayer, QObject *parent)
    : QObject(parent)
{
    // Create Playlist
    this->currentPlaylist = new QMediaPlaylist();

    // Connect signal to slot
    connect(this->currentPlaylist, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentIndexChanged(int)));

    for (int i=0 ; i<numberMediaPlayer ; ++i) {
        // Create mediaPlayer
        this->listMediaPlayer.append(new QMediaPlayer(parent, QMediaPlayer::LowLatency));

        // Set Volume to maximum
        this->listMediaPlayer.last()->setVolume(mySettings->value("VolumeMain", 60).toInt());

        // Set speed of read
        this->listMediaPlayer.last()->setPlaybackRate(1.0);


        // Connect signal to slot
        connect(this->listMediaPlayer.last(), SIGNAL(positionChanged(qint64)), this, SLOT(onPositionChanged(qint64)));
        connect(this->listMediaPlayer.last(), SIGNAL(durationChanged(qint64)), this, SLOT(onDurationChanged(qint64)));
        connect(this->listMediaPlayer.last(), SIGNAL(metaDataChanged()), this, SLOT(onMetaDataChanged()));
        connect(this->listMediaPlayer.last(), SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(onMediaStatusChanged(QMediaPlayer::MediaStatus)));
    }

    this->currentPlayerIndex = 0;
    this->currentDuration = 0;
    this->isSingle = false;
}

bool tMediaPlayer::clear()
{
    foreach (QMediaPlayer *player, listMediaPlayer) {
        player->stop();
    }
    return this->currentPlaylist->clear();
}

bool tMediaPlayer::addMedia(const QMediaContent &content)
{
    return this->currentPlaylist->addMedia(content);
}

bool tMediaPlayer::addMedia(const QList<QMediaContent> &items)
{
    bool ret = true;
    foreach (QMediaContent media, items) {
        ret &= this->addMedia(media);
    }
    return ret;
}

int tMediaPlayer::currentIndex()
{
    return this->currentPlaylist->currentIndex();
}

tMediaPlayer::State tMediaPlayer::state()
{
    return (tMediaPlayer::State)this->listMediaPlayer.at(this->currentPlayerIndex)->state();
}

tMediaPlayer::MediaStatus tMediaPlayer::mediaStatus()
{
    return (tMediaPlayer::MediaStatus)this->listMediaPlayer.at(this->currentPlayerIndex)->mediaStatus();
}

bool tMediaPlayer::isMetaDataAvailable()
{
    return this->listMediaPlayer.at(this->currentPlayerIndex)->isMetaDataAvailable();
}

QVariant tMediaPlayer::metaData(const QString &key)
{
    return this->listMediaPlayer.at(this->currentPlayerIndex)->metaData(key);
}

qint64 tMediaPlayer::duration()
{
    return this->listMediaPlayer.at(this->currentPlayerIndex)->duration();
}

qint64 tMediaPlayer::position()
{
    return this->listMediaPlayer.at(this->currentPlayerIndex)->position();
}

void tMediaPlayer::onPositionChanged(qint64 position_)
{
    // Get sender
    QMediaPlayer* player = qobject_cast<QMediaPlayer*>(sender());
    if (player == this->listMediaPlayer.at(this->currentPlayerIndex)) {
        emit this->positionChanged(position_);
        if (!this->isSingle) {
            // Lorsque nous dépassons la moitier de la musique, nous chargons la prochaine dans le 2eme player
            if (position_ > this->currentDuration/2) {
                if (this->currentPlaylist->currentIndex() < this->currentPlaylist->mediaCount()-1) {
                    if (this->listMediaPlayer.at((this->currentPlayerIndex+1)%this->listMediaPlayer.size())->media() != this->currentPlaylist->media(this->currentPlaylist->currentIndex()+1)) {
                        this->listMediaPlayer.at((this->currentPlayerIndex+1)%this->listMediaPlayer.size())->setMedia(this->currentPlaylist->media(this->currentPlaylist->currentIndex()+1));
                        qDebug() << "Chargement de la musique " << this->currentPlaylist->currentIndex()+1 << ", dans le player " << (this->currentPlayerIndex+1)%this->listMediaPlayer.size();
                    }
                }
            }
            // Lorsque nous arrivons à la fin de la musique, nous lançons le 2eme player
            if (this->currentDuration && (position_ >= (this->currentDuration - this->timeFading))) {
                if (this->currentPlaylist->currentIndex() < this->currentPlaylist->mediaCount()-1) {
                    this->currentPlayerIndex = (this->currentPlayerIndex + 1) % this->listMediaPlayer.size();
                    this->listMediaPlayer.at(this->currentPlayerIndex)->play();
                    qDebug() << "Lecture sur le player " << this->currentPlayerIndex;
                    this->currentPlaylist->next();
                    this->currentDuration = this->listMediaPlayer.at(this->currentPlayerIndex)->duration();
                    emit this->metaDataChanged();
                    emit this->durationChanged(this->currentDuration);
                }
                else {
                    if (position_ >= this->currentDuration) {
                        this->listMediaPlayer.at(this->currentPlayerIndex)->stop();
                        emit this->durationChanged(this->currentDuration);
                    }
                }
            }
        }
        else {
            // Lorsque nous arrivons à la fin de la musique, nous pré-chargons la prochaine musique
            if (this->currentDuration && (position_ >= this->currentDuration)) {
                if (this->currentPlaylist->currentIndex() < this->currentPlaylist->mediaCount()-1) {
                    this->currentDuration = 0;
                    this->currentPlaylist->next();
                    this->listMediaPlayer.at(this->currentPlayerIndex)->setMedia(this->currentPlaylist->currentMedia());
                    qDebug() << "Chargement de la musique "<< this->currentIndex() << ", sur le player " << this->currentPlayerIndex;
                }
                else {
                    this->listMediaPlayer.at(this->currentPlayerIndex)->stop();
                    emit this->durationChanged(this->currentDuration);
                }
            }
        }
    }
}

void tMediaPlayer::onDurationChanged(qint64 duration_)
{
    // Get sender
    QMediaPlayer* player = qobject_cast<QMediaPlayer*>(sender());
    if (player == this->listMediaPlayer.at(this->currentPlayerIndex)) {
        if (duration_) {
            this->currentDuration = duration_;
            emit this->durationChanged(duration_);
        }
    }
}

void tMediaPlayer::onMetaDataChanged()
{
    // Get sender
    QMediaPlayer* player = qobject_cast<QMediaPlayer*>(sender());
    if (player == this->listMediaPlayer.at(this->currentPlayerIndex))
        emit this->metaDataChanged();
}

void tMediaPlayer::onCurrentIndexChanged(int newIndex_)
{
    emit this->currentIndexChanged(newIndex_);
}

void tMediaPlayer::onMediaStatusChanged(QMediaPlayer::MediaStatus status_)
{
    QMediaPlayer* player = qobject_cast<QMediaPlayer*>(sender());
    if (status_ == QMediaPlayer::EndOfMedia) {
        player->stop();
    }
}

void tMediaPlayer::play()
{
    if (this->currentPlaylist->isEmpty()) return;
    this->listMediaPlayer.at(this->currentPlayerIndex)->play();
    qDebug() << "Lecture de la musique " << this->currentPlaylist->currentMedia().canonicalUrl().toString() << "sur le player " << this->currentPlayerIndex;
}

void tMediaPlayer::pause()
{
    if (this->currentPlaylist->isEmpty()) return;
    this->listMediaPlayer.at(this->currentPlayerIndex)->pause();
}

void tMediaPlayer::stop()
{
    if (this->currentPlaylist->isEmpty()) return;
    this->listMediaPlayer.at(this->currentPlayerIndex)->stop();
}

void tMediaPlayer::setPosition(qint64 position_)
{
    if (this->currentPlaylist->isEmpty()) return;
    this->listMediaPlayer.at(this->currentPlayerIndex)->setPosition(position_);
}

void tMediaPlayer::next()
{
    if (this->currentPlaylist->isEmpty()) return;
    if (this->currentPlaylist->currentIndex() == this->currentPlaylist->mediaCount()-1) return;

    this->currentPlaylist->next();
    QMediaContent media = this->currentPlaylist->currentMedia();
    this->listMediaPlayer.at(this->currentPlayerIndex)->setMedia(media);
//    this->listMediaPlayer.at(this->currentPlayerIndex)->play();
}

void tMediaPlayer::previous()
{
    if (this->currentPlaylist->isEmpty()) return;
    if (this->currentPlaylist->currentIndex() == 0) return;

    this->currentPlaylist->previous();
    QMediaContent media = this->currentPlaylist->currentMedia();
    this->listMediaPlayer.at(this->currentPlayerIndex)->setMedia(media);
//    this->listMediaPlayer.at(this->currentPlayerIndex)->play();
}

void tMediaPlayer::setCurrentIndex(int index)
{
    if (this->currentPlaylist->isEmpty()) return;
    this->currentPlaylist->setCurrentIndex(index);
    QMediaContent media = this->currentPlaylist->currentMedia();
    this->listMediaPlayer.at(this->currentPlayerIndex)->setMedia(media);
}
