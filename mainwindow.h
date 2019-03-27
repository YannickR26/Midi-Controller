#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "tPad.h"
#include "tmidi.h"
#include "tplaylist.h"
#include "tmediaplayer.h"
#include "scrolltext.h"
#include "tVideoWidget.h"
#include "dynamicfontsizelabel.h"

#include <QMainWindow>
#include <QtWidgets>


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void newMidiMsgReceived(tShortMessage *msg_);
    // Action depuis le menu Fichier
    void newScene();
    void openScene();
    void saveScene();
    void saveAsScene();
    // Action depuis le menu configuration
    void setEditable(bool checked_);
    void enterMidiConfig();
    void enterConfig();
    void enterPlaylist();
    void enterFullScreen();
    void showVideoWindow(bool checked_);
    void enterAPropos();
    void updatePad();
    void timerOverflow();
    void timerClockOverflow();

    void splitterBtSliderMoved(int pos, int index) {
        Q_UNUSED(pos)
        Q_UNUSED(index)
        foreach (tPad *pad, listPad) {
            pad->update();
        }
    }

    void splitterBtPlaylsitMoved(int pos, int index) {
        Q_UNUSED(pos)
        Q_UNUSED(index)
        foreach (tPad *pad, listPad) {
            pad->update();
        }
        if (this->labelCover->pixmap() != NULL) {
            this->labelCover->setPixmap(this->pixmapCover.scaled(this->labelCover->size(), Qt::KeepAspectRatio));
        }
    }

    void buttonPadClicked();
    void sliderPadClicked();
    void mediaPlayerMovePosition(int newPosition_);
    void mediaPlayerPositionChanged(qint64 position_);
    void mediaPlayerDurationChanged(qint64 duration_);
    void mediaPlayerPlaylistPositionChanged(int newIndex_);
    void mediaPlayerJumpTo(QModelIndex newIndex_);
    void mediaPlayerMetaDataChanged();
    void sampleStateChanged(QMediaPlayer::State state_);

protected:
    void createMenu();
    void createFrame();
    void writeXmlFile();
    void readXmlFile();
    void actionFromPad(tPad *pad_);
    void readSetting();
    void readPlaylist(QString &playlist_);
    void readSample(QString &urlSample_);
    void SendConfigToAPC();
    void mediaPlayerupdateDurationInfo();
    void setCover(QString urlCover_);
    void setCoverText(QString text_);

private:
    QAction                 *videoWinAct;
    QString                 fileNameXml;
    QStatusBar              *barreEtat;
    DynamicFontSizeLabel    *labelClock;
    DynamicFontSizeLabel    *labelCover;
    QPixmap                 pixmapCover;
    ScrollText              *labelCurrentAlbum;
    ScrollText              *labelCurrentTitle;
    ScrollText              *labelCurrentArtiste;
    QLabel                  *labelSingle;
    QLabel                  *labelNumberTotal;
    QLabel                  *labelSoundStartDuration, *labelSoundEndDuration;
    QSlider                 *sliderSoundPosition;
    QLabel                  *labelPlaylistStartDuration, *labelPlaylistEndDuration;
    QSlider                 *sliderPlaylistPosition;
//    QListWidget             *listMP3;
    QTableWidget            *listMP3;
    QList<tPad *>           listPad;
    QList<tPlaylist *>      listPlaylist;
    tPlaylist               *playlist;
    tMediaPlayer            *mediaPlayer;
    tVideoWidget            *videoWidget;
    QWidget                 *videoWindow;
    tMidi                   *midi;
    bool                    modeConfig;
    QSplitter               *splitterCover, *splitterBtSlider, *splitterBtPlaylist;
    qint64                  positionCurrent, durationCurrent, positionTotal, durationTotal;
    int                     timeFastForward;
};

#endif // MAINWINDOW_H
