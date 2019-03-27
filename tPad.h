#ifndef PAD_H
#define PAD_H

#include "tmidi.h"
#include "common.h"

#include <QDomDocument>
#include <QDomElement>
#include <QPushButton>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QMessageBox>
#include <QFileInfo>
#include <QSlider>
#include <QLabel>
#include <QString>
#include <QDebug>

class tPad
{

public:
    typedef enum {
        MODE_DISABLE    = 0,
        MODE_COMMAND,
        MODE_PLAYLIST,
        MODE_SAMPLE,
        MODE_TRANSLATION,
    } tMode;

    typedef enum {
        CMD_UNKNOWN         = 0,
        CMD_RECULE_RAPIDE,
        CMD_PRECEDENT,
        CMD_PLAY_PAUSE,
        CMD_STOP,
        CMD_SUIVANT,
        CMD_AVANCE_RAPIDE,
        CMD_MUTE,
    } tCommand;

    tPad(QPushButton *bp_) {
        button = bp_;
        slider = NULL;
        label = NULL;
        clear();
    }

    tPad(QSlider *slider_, QLabel *label_) {
        button = NULL;
        slider = slider_;
        label = label_;
        clear();
    }

    void clear() {
        mode = MODE_DISABLE;
        command = CMD_UNKNOWN;
        player = NULL;
        mediaPlaylist = NULL;
        deviceMidiIn.clear();
        deviceMidiOut.clear();
        urlIcon.clear();
        playlist.clear();
        urlSample.clear();
        isMuted = false;
        msgMidiIn.clear();
        msgMidiOut.clear();
    }

    void setEnable(bool enable_) {
        if (button != NULL) {
            button->setEnabled(enable_);
            button->setFlat(false);
        }
        if (slider != NULL)
            slider->setEnabled(enable_);
    }

    void setIcon(QString icon_) {
        if (button != NULL) {
            button->setText("");
            button->setIconSize(button->size());
            button->setCheckable(false);
            button->setFlat(true);
            button->setIcon(QIcon(icon_));
        }
    }

    void playSample() {
        if (player != NULL) {
            player->play();
            player->setMuted(sampleIsMuted);
        }
    }

    void stopSample() {
        if (player != NULL) {
            player->stop();
        }
    }

    QMediaPlayer* getMediaPlayer() {
        return player;
    }

    void update() {
        if (button != NULL) {
            if (mode == MODE_PLAYLIST) {
                button->setText(QFileInfo(playlist).baseName());
                button->setFlat(false);
                button->setIcon(QIcon());
                button->setEnabled(true);
            }
            else if (mode == MODE_SAMPLE) {
                button->setText(QFileInfo(urlSample).baseName());
                button->setEnabled(true);
                button->setFlat(false);
                if (player == NULL) {
                    player = new QMediaPlayer;
                    mediaPlaylist = new QMediaPlaylist();
                    mediaPlaylist->addMedia(QMediaContent(this->urlSample));
                    if (isInfinited) {
                        mediaPlaylist->setPlaybackMode(QMediaPlaylist::Loop);
                    }
                    player->setPlaylist(mediaPlaylist);
                    player->setVolume(mySettings->value("VolumeSample", 90).toInt());
                    player->setPlaybackRate(1.0);
                }
                else {
                    if (mediaPlaylist->media(0).canonicalUrl().toString() != this->urlSample) {
                        mediaPlaylist->clear();
                        mediaPlaylist->addMedia(QMediaContent(this->urlSample));
                    }
                    if (isInfinited) {
                        mediaPlaylist->setPlaybackMode(QMediaPlaylist::Loop);
                    }
                    else {
                        mediaPlaylist->setPlaybackMode(QMediaPlaylist::CurrentItemOnce);
                    }
                }
            }
            else if (mode == MODE_COMMAND) {
                switch (command) {
                case CMD_RECULE_RAPIDE:     this->setIcon(ICON_RECULE_RAPIDE);  break;
                case CMD_PRECEDENT:         this->setIcon(ICON_PRECEDENT);      break;
                case CMD_PLAY_PAUSE:        this->setIcon(ICON_PLAY);           break;
                case CMD_STOP:              this->setIcon(ICON_STOP);           break;
                case CMD_SUIVANT:           this->setIcon(ICON_SUIVANT);        break;
                case CMD_AVANCE_RAPIDE:     this->setIcon(ICON_AVANCE_RAPIDE);  break;
                case CMD_MUTE:              this->setIcon(ICON_VOLUME);         break;
                default: break;
                }
                button->setEnabled(true);
            }
            else {
                button->setText("");
                button->setIcon(QIcon());
                button->setFlat(true);
                button->setEnabled(false);
            }

            if (!urlIcon.isEmpty()) {
                button->setText("");
                button->setIcon(QIcon(urlIcon));
                button->setIconSize(QSize(button->size().width()-10, button->size().height()-10));
                button->setFlat(true);
                button->setEnabled(true);
            }
        }

        if (slider != NULL) {
            if (mode == MODE_TRANSLATION/*!deviceMidiIn.isEmpty() || !deviceMidiOut.isEmpty()*/) {
                slider->setEnabled(true);
            }
            else {
                slider->setEnabled(false);
            }
        }
    }

    // Convert Data to Xml
    void toXml(QDomDocument &parent, QDomElement &child) {
        QDomElement padChild = parent.createElement("Pad");
        if (this->label != NULL) {
            padChild.setAttribute("Name", this->label->text());
        }
        else {
            padChild.setAttribute("Name", "");
        }
        padChild.setAttribute("mode", this->mode);
        padChild.setAttribute("command", this->command);
        padChild.setAttribute("urlIcon", this->urlIcon);
        padChild.setAttribute("playlist", this->playlist);
        padChild.setAttribute("urlSample", this->urlSample);
        padChild.setAttribute("isInfinited", this->isInfinited);
        padChild.setAttribute("sampleIsMuted", this->sampleIsMuted);
        padChild.setAttribute("deviceMidiIn", this->deviceMidiIn);
        if (!deviceMidiIn.isEmpty()) {
            QDomElement midiInChild = parent.createElement("Midi_In");
            this->msgMidiIn.toXml(midiInChild);
            padChild.appendChild(midiInChild);
        }
        padChild.setAttribute("deviceMidiOut", this->deviceMidiOut);
        if (!deviceMidiOut.isEmpty()) {
            QDomElement midiOutChild = parent.createElement("Midi_Out");
            this->msgMidiOut.toXml(midiOutChild);
            padChild.appendChild(midiOutChild);
        }
        child.appendChild(padChild);
    }

    // Convert Xml to Data
    void fromXml(QDomElement &node) {
        if (node.tagName() == "Pad") {
            if (this->label != NULL) {
                this->label->setText(node.attribute("Name"));
            }
            this->mode = (tMode)node.attribute("mode").toInt();
            this->command = (tCommand)node.attribute("command").toInt();
            this->urlIcon = node.attribute("urlIcon");
            this->playlist = node.attribute("playlist");
            this->urlSample = node.attribute("urlSample");
            this->isInfinited = node.attribute("isInfinited").toInt() > 0 ? true : false;
            this->sampleIsMuted = node.attribute("sampleIsMuted").toInt() > 0 ? true : false;
            this->deviceMidiIn = node.attribute("deviceMidiIn");
            this->deviceMidiOut = node.attribute("deviceMidiOut");
            QDomElement child = node.firstChildElement();
            while (!child.isNull()) {
                if (child.tagName() == "Midi_In") {
                    msgMidiIn.fromXml(child);
                }
                else if (child.tagName() == "Midi_Out") {
                    msgMidiOut.fromXml(child);
                }
                child = child.nextSiblingElement();
            }
            this->update();
        }
    }

    QPushButton             *button;
    QSlider                 *slider;
    QLabel                  *label;
    tMode                   mode;
    tCommand                command;
    QString                 urlIcon, playlist, urlSample;
    bool                    isMuted;

    // for midi message
    QString                 deviceMidiIn;
    tShortMessage           msgMidiIn;
    QString                 deviceMidiOut;
    tShortMessage           msgMidiOut;

    // for Sample audio
    bool                    isInfinited;
    bool                    sampleIsMuted;
    QMediaPlayer            *player;
    QMediaPlaylist          *mediaPlaylist;
};

#endif // PAD_H
