#ifndef CONFIGPLAYLISTWINDOW_H
#define CONFIGPLAYLISTWINDOW_H

#include "tplaylist.h"

#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <QDialog>

class configPlaylistWindow : public QDialog
{
    Q_OBJECT

public:
    configPlaylistWindow(QList<tPlaylist *> *playlist_, QDialog *parent = nullptr);

public slots:
    void buttonAddPlaylistClicked();
    void buttonRemovePlaylistClicked();
    void buttonAddMP3Clicked();
    void buttonRemoveMP3Clicked();
    void buttonIncClicked();
    void buttonDecClicked();
    void buttonOKClicked();
    void linePathCoverReturnPressed();
    void buttonPathCoverClicked();
    void listPlaylistItemChanged(QListWidgetItem *item_);
    void listPlaylistItemClicked(QListWidgetItem *item_);
    void listMP3ItemClicked(QListWidgetItem *item_);
    void checkBoxClicked(bool clicked_);
    void checkBoxFullPathClicked(bool clicked_);
    void spinBoxValueChanged(int value_);
    void mediaStatusChanged(QMediaPlayer::MediaStatus status_);
    void metaDataChanged();

protected:
    void updateListPlaylist();
    void updateListMP3();

private:
    QListWidget             *listPlaylist, *listSound;
    QLineEdit               *linePathCover;
    QList<tPlaylist *>      *playlist;
    tPlaylist               *currentPlaylist;

    QPushButton             *buttonRemovePlaylist;
    QPushButton             *buttonAddMP3, *buttonRemoveMP3;
    QPushButton             *buttonInc, *buttonDec;
    QPushButton             *buttonPathCover;

    QPushButton             *buttonOK, *buttonQuit;

    QCheckBox               *checkBoxSingle;
    QCheckBox               *checkBoxFullPath;
    QSpinBox                *spinBoxTimeTransition;

    QMediaPlayer            *player;
    QMediaPlaylist          *mediaPlaylist;
};

#endif // CONFIGPLAYLISTWINDOW_H
