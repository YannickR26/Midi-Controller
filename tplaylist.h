#ifndef TPLAYLIST_H
#define TPLAYLIST_H

#include <QStringList>
#include <QDomDocument>
#include <QDomElement>
#include <QMediaPlayer>
#include <QDebug>
#include <QMediaMetaData>
#include <QMediaPlaylist>
#include <QObject>

class tPlaylist : public QObject
{
    Q_OBJECT

public:

    typedef struct {
        QString     url;
        QString     title;
        QString     artist;
        int         duration;
    } playlist_typedef;

    tPlaylist(QObject *parent = 0) :
        QObject(parent) {
       this->clear();
    }

    tPlaylist(QString name_) :
        QObject(0) {
        this->clear();
        nameOfPlaylist = name_;
    }

    tPlaylist(QDomElement &node):
        QObject(0) {
        this->fromXml(node);
    }


    void clear() {
        playlist.clear();
        nameOfPlaylist.clear();
        single = false;
    }

    QList<playlist_typedef> getPlaylist() {
        return this->playlist;
    }
    void setPlaylist(QList<playlist_typedef> playlist_) {
        this->playlist = playlist_;
    }

    int getTotalPlaylistDuration() {
        int length=0;
        for (int i=0 ; i<this->playlist.size() ; ++i) {
            length += this->playlist.at(i).duration;
        }
        return length;
    }

    int getTotalDuration(int toIndex_) {
        int length=0;
        if (toIndex_ < playlist.size()) {
            for (int i=0 ; i<toIndex_ ; ++i) {
                length += this->playlist.at(i).duration;
            }
        }
        return length;
    }

    void addToPlaylist(QString url_, QString title_, QString artist_, int duration_) {
        playlist_typedef playlist;
        playlist.url = url_;
        playlist.title = title_;
        playlist.artist = artist_;
        playlist.duration = duration_;
        this->playlist.append(playlist);
    }
    void addToPlaylist(QString url_, QString artist_, int duration_) {
        this->addToPlaylist(url_, "", artist_, duration_);
    }
    void addToPlaylist(QString url_, int duration_) {
        this->addToPlaylist(url_, "", "", duration_);
    }
    void addToPlaylist(QString url_) {
        this->addToPlaylist(url_, "", "", 0);
    }
    void addToPlaylist(playlist_typedef media_) {
        this->playlist.append(media_);
    }

    void removePlaylist(QString url_) {
        for (int i=0 ; i<this->playlist.size() ; ++i) {
            if (this->playlist.at(i).url == url_) {
                this->playlist.removeAt(i);
                return;
            }
        }
    }

    QString getNameOfPlaylist() const { return nameOfPlaylist; }
    void setNameOfPlaylist(const QString &value_) { nameOfPlaylist = value_; }

    bool getSingle() const { return single; }
    void setSingle(bool value_) { single = value_; }

    int getTimeTransition() const { return timeTransition; }
    void setTimeTransition(int time_) { timeTransition = time_; }

    QString getUrlCover()  { return urlCover; }
    void setUrlCover(const QString &value) { urlCover = value; }

    // Convert Data to Xml
    void toXml(QDomDocument &parent, QDomElement &child) {
        QDomElement childPlaylist = parent.createElement("Playlist");
        childPlaylist.setAttribute("nameOfPlaylist", this->nameOfPlaylist);
        childPlaylist.setAttribute("single", this->single);
        childPlaylist.setAttribute("timeTransition", this->timeTransition);
        childPlaylist.setAttribute("urlCover", this->urlCover);
        child.appendChild(childPlaylist);

        for (int i=0 ; i<playlist.size() ; ++i) {
            QDomElement childMP3 = parent.createElement("MP3");
            childMP3.setAttribute("URL", this->playlist.at(i).url);
            childMP3.setAttribute("Title", this->playlist.at(i).title);
            childMP3.setAttribute("Artist", this->playlist.at(i).artist);
            childMP3.setAttribute("Duration", this->playlist.at(i).duration);
            childPlaylist.appendChild(childMP3);
        }
    }

    // Convert Xml to Data
    void fromXml(QDomElement &node) {
        if (node.tagName() == "Playlist") {
            this->nameOfPlaylist = node.attribute("nameOfPlaylist");
            this->single = (node.attribute("single") == "0") ? false : true;
            this->timeTransition = node.attribute("timeTransition").toInt();
            this->urlCover = node.attribute("urlCover");
            this->playlist.clear();
            QDomElement child = node.firstChildElement();
            playlist_typedef list;
            while (!child.isNull()) {
                if (child.tagName() == "MP3") {
                    list.url = child.attribute("URL");
                    list.title = child.attribute("Title");
                    list.artist = child.attribute("Artist");
                    list.duration = child.attribute("Duration").toInt();
                    this->playlist.append(list);
                }
                child = child.nextSiblingElement();
            }
        }
    }

private:
    QList<playlist_typedef>     playlist;
    QString                     nameOfPlaylist;
    bool                        single;
    int                         timeTransition;
    QString                     urlCover;
};

#endif // TPLAYLIST_H
