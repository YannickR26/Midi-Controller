#include "configplaylistwindow.h"
#include "common.h"

#include <QFileDialog>
#include <QLayout>
#include <QLabel>
#include <QDebug>

#include <QStandardPaths>
#include <QMessageBox>

configPlaylistWindow::configPlaylistWindow(QList<tPlaylist *> *playlist_, QDialog *parent)
    : QDialog(parent)
{

    this->setWindowTitle(tr("Playlist"));

    playlist = playlist_;

    mediaPlaylist = new QMediaPlaylist();
    player = new QMediaPlayer();
    player->setPlaylist(mediaPlaylist);
    //connect(player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(mediaStatusChanged(QMediaPlayer::MediaStatus)));
    connect(player, SIGNAL(metaDataChanged()), this, SLOT(metaDataChanged()));

    // Creation des listWidget
    listPlaylist = new QListWidget();
    listPlaylist->setMaximumWidth(300);
    connect(listPlaylist, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(listPlaylistItemChanged(QListWidgetItem*)));
    connect(listPlaylist, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(listPlaylistItemClicked(QListWidgetItem*)));
    listSound = new QListWidget();
//    listSound->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
    connect(listSound, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(listMP3ItemClicked(QListWidgetItem*)));

    // Creation des boutons pour la playlist
    QPushButton *buttonAddPlaylist = new QPushButton();
    buttonAddPlaylist->setIcon(QIcon(ICON_AJOUTER));
    buttonAddPlaylist->setMinimumSize(30, 30);
    buttonAddPlaylist->setMaximumSize(30, 30);
    buttonAddPlaylist->setFlat(true);
    connect(buttonAddPlaylist, SIGNAL(clicked()), this, SLOT(buttonAddPlaylistClicked()));

    buttonRemovePlaylist = new QPushButton();
    buttonRemovePlaylist->setIcon(QIcon(ICON_SUPPRIMER));
    buttonRemovePlaylist->setMinimumSize(30, 30);
    buttonRemovePlaylist->setMaximumSize(30, 30);
    buttonRemovePlaylist->setFlat(true);
    buttonRemovePlaylist->setEnabled(false);
    connect(buttonRemovePlaylist, SIGNAL(clicked()), this, SLOT(buttonRemovePlaylistClicked()));

    // Creation du checkBox single pour la playlist
    checkBoxSingle = new QCheckBox(tr("Single"));
    checkBoxSingle->setFont(QFont(FONT));
    checkBoxSingle->setEnabled(false);
    connect(checkBoxSingle, SIGNAL(clicked(bool)), this, SLOT(checkBoxClicked(bool)));

    // Creation du checkBox fullPath
    checkBoxFullPath = new QCheckBox(tr("Full Path"));
    checkBoxFullPath->setFont(QFont(FONT));
    checkBoxFullPath->setEnabled(false);
    connect(checkBoxFullPath, SIGNAL(clicked(bool)), this, SLOT(checkBoxFullPathClicked(bool)));

    // Creation du spinBox TimeTransition
    spinBoxTimeTransition = new QSpinBox();
    spinBoxTimeTransition->setSuffix(" ms");
    spinBoxTimeTransition->setMaximum(10000);
    spinBoxTimeTransition->setSingleStep(50);
    spinBoxTimeTransition->setEnabled(false);
    connect(spinBoxTimeTransition, SIGNAL(valueChanged(int)), this, SLOT(spinBoxValueChanged(int)));

    // Creation des boutons pour les MP3
    buttonAddMP3 = new QPushButton();
    buttonAddMP3->setIcon(QIcon(ICON_AJOUTER));
    buttonAddMP3->setMinimumSize(30, 30);
    buttonAddMP3->setMaximumSize(30, 30);
    buttonAddMP3->setFlat(true);
    buttonAddMP3->setEnabled(false);
    connect(buttonAddMP3, SIGNAL(clicked()), this, SLOT(buttonAddMP3Clicked()));

    buttonRemoveMP3 = new QPushButton();
    buttonRemoveMP3->setIcon(QIcon(ICON_SUPPRIMER));
    buttonRemoveMP3->setMinimumSize(30, 30);
    buttonRemoveMP3->setMaximumSize(30, 30);
    buttonRemoveMP3->setFlat(true);
    buttonRemoveMP3->setEnabled(false);
    connect(buttonRemoveMP3, SIGNAL(clicked()), this, SLOT(buttonRemoveMP3Clicked()));

    buttonInc = new QPushButton();
    buttonInc->setIcon(QIcon(ICON_FLECHE_HAUT));
    buttonInc->setMinimumSize(30, 30);
    buttonInc->setMaximumSize(30, 30);
    buttonInc->setFlat(true);
    buttonInc->setEnabled(false);
    connect(buttonInc, SIGNAL(clicked()), this, SLOT(buttonIncClicked()));

    buttonDec = new QPushButton();
    buttonDec->setIcon(QIcon(ICON_FLECHE_BAS));
    buttonDec->setMinimumSize(30, 30);
    buttonDec->setMaximumSize(30, 30);
    buttonDec->setFlat(true);
    buttonDec->setEnabled(false);
    connect(buttonDec, SIGNAL(clicked()), this, SLOT(buttonDecClicked()));

    linePathCover = new QLineEdit();
    connect(linePathCover, SIGNAL(returnPressed()), this, SLOT(linePathCoverReturnPressed()));
    buttonPathCover = new QPushButton(tr("..."));
    buttonPathCover->setFixedSize(30, 30);
    buttonPathCover->setEnabled(false);
    connect(buttonPathCover, SIGNAL(clicked()), this, SLOT(buttonPathCoverClicked()));

    // Mise en forme Vertical des 2 listes
    QLabel *labelPlaylist = new QLabel(tr("Playlist"));
    labelPlaylist->setFont(QFont(FONT, 12));
    QHBoxLayout *hBoxButtonPlaylist = new QHBoxLayout();
    hBoxButtonPlaylist->addWidget(labelPlaylist);
    hBoxButtonPlaylist->addWidget(buttonAddPlaylist);
    hBoxButtonPlaylist->addWidget(buttonRemovePlaylist);
    QVBoxLayout *vBoxListPlaylist = new QVBoxLayout();
    vBoxListPlaylist->addLayout(hBoxButtonPlaylist);
    vBoxListPlaylist->addWidget(listPlaylist);

    QLabel *labelSound = new QLabel(tr("Musiques"));
    labelSound->setFont(QFont(FONT, 12));

    QHBoxLayout *hBoxCheckBox = new QHBoxLayout();
    hBoxCheckBox->addWidget(labelSound);
    hBoxCheckBox->addSpacing(10);
    hBoxCheckBox->addWidget(checkBoxSingle);
    hBoxCheckBox->addSpacing(10);
    hBoxCheckBox->addWidget(checkBoxFullPath);
    hBoxCheckBox->addSpacing(10);
    QLabel *transition = new QLabel(tr("Durée de transition:"));
    transition->setFont(QFont(FONT));
    hBoxCheckBox->addWidget(transition);
    hBoxCheckBox->addWidget(spinBoxTimeTransition);
    hBoxCheckBox->addSpacing(10);
    QLabel *cover = new QLabel(tr("Cover:"));
    cover->setFont(QFont(FONT));
    hBoxCheckBox->addWidget(cover);
    hBoxCheckBox->addWidget(linePathCover);
    hBoxCheckBox->addWidget(buttonPathCover);

    QVBoxLayout *vBoxButtonMP3 = new QVBoxLayout();
//    vBoxButtonMP3->addSpacing(30);
    vBoxButtonMP3->addWidget(buttonAddMP3);
    vBoxButtonMP3->addWidget(buttonRemoveMP3);
    vBoxButtonMP3->addStretch(100);
    vBoxButtonMP3->addWidget(buttonInc);
    vBoxButtonMP3->addWidget(buttonDec);
//    vBoxButtonMP3->addSpacing(50);

    QHBoxLayout *hBoxListSound = new QHBoxLayout();
    hBoxListSound->addWidget(listSound);
    hBoxListSound->addLayout(vBoxButtonMP3);

    QVBoxLayout *vBoxListSound = new QVBoxLayout();
    vBoxListSound->addLayout(hBoxCheckBox);
    vBoxListSound->addLayout(hBoxListSound);

    QHBoxLayout *hBoxList = new QHBoxLayout();
    hBoxList->addLayout(vBoxListPlaylist);
    hBoxList->addLayout(vBoxListSound);
//    hBoxList->addLayout(vBoxButtonMP3);

    // Creation des boutons de sortie
    buttonOK = new QPushButton(tr("OK"));
    connect(buttonOK, SIGNAL(clicked()), this, SLOT(buttonOKClicked()));
    buttonQuit = new QPushButton(tr("Annuler"));
    connect(buttonQuit, SIGNAL(clicked()), this, SLOT(reject()));

    // Annuler et OK
    QHBoxLayout *hBox = new QHBoxLayout();
    hBox->addWidget(buttonQuit);
    hBox->addStretch(100);
    hBox->addWidget(buttonOK);

    // Mise en forme Verticale
    QVBoxLayout *vBoxMain = new QVBoxLayout();
    vBoxMain->addLayout(hBoxList);
    vBoxMain->addLayout(hBox);

    setLayout(vBoxMain);

    this->setMinimumSize(700, 500);

    // Update les lists
    updateListPlaylist();
}

void configPlaylistWindow::updateListPlaylist()
{
    // Efface tout le contenu
    listPlaylist->clear();

    // Ajoute une a une toute les playlist
    tPlaylist *list;
    foreach (list, *playlist) {
        listPlaylist->addItem(list->getNameOfPlaylist());
        listPlaylist->item(listPlaylist->count()-1)->setFlags(listPlaylist->item(listPlaylist->count()-1)->flags() | Qt::ItemIsEditable);
    }

    buttonPathCover->setEnabled(false);
    buttonRemovePlaylist->setEnabled(false);
    buttonAddMP3->setEnabled(false);
    checkBoxSingle->setEnabled(false);
    checkBoxFullPath->setEnabled(false);
    spinBoxTimeTransition->setEnabled(false);
}

void configPlaylistWindow::updateListMP3()
{
    // Efface tout le contenu
    listSound->clear();

    // Ajoute toute les musiques et affiche seulement le nom du fichier
    QList<tPlaylist::playlist_typedef> list = currentPlaylist->getPlaylist();
    tPlaylist::playlist_typedef mp3;
    foreach (mp3, list) {
        if (this->checkBoxFullPath->isChecked()) {
            listSound->addItem(mp3.url);
        }
        else {
            QFileInfo fileInfo(mp3.url);
            listSound->addItem(fileInfo.baseName());
        }
    }

    // Update du CheckBox
    checkBoxSingle->setChecked(currentPlaylist->getSingle());

    // Update du SpinBox
    spinBoxTimeTransition->setValue(currentPlaylist->getTimeTransition());

    // Update du Cover
    this->linePathCover->setText(currentPlaylist->getUrlCover());

    // Desactive quelques bouton
    buttonRemoveMP3->setEnabled(false);
    buttonInc->setEnabled(false);
    buttonDec->setEnabled(false);
}

void configPlaylistWindow::buttonAddPlaylistClicked()
{
    playlist->append(new tPlaylist(QString("Playlist %1").arg(rand())));
    this->updateListPlaylist();
}

void configPlaylistWindow::buttonRemovePlaylistClicked()
{
    playlist->removeAt(listPlaylist->currentRow());

    this->updateListPlaylist();
    // Efface tout le contenu
    listSound->clear();
}

void configPlaylistWindow::buttonAddMP3Clicked()
{
    // Ouvre la boite de dialog pour aller selectionner les musiques a ajouter
    QList<QUrl> fileName = QFileDialog::getOpenFileUrls(this, tr("Séléctionner les musiques à ajouter"),
                                                        QDir::homePath(),
                                                        tr("Musique Files (*.mp3 *.wma *.m4a *.wav)"));

    // Si aucune musiques selectionner, on quit
    if (fileName.isEmpty()) return;

    mediaPlaylist->clear();
    for (int i=0 ; i<fileName.size() ; ++i) {
        mediaPlaylist->addMedia(fileName.at(i));
//        currentPlaylist->addToPlaylist(fileName.at(i).toString());
    }
    mediaPlaylist->setCurrentIndex(0);

    buttonAddMP3->setEnabled(false);
    buttonOK->setEnabled(false);
    buttonQuit->setEnabled(false);
}

void configPlaylistWindow::buttonRemoveMP3Clicked()
{
    QList<tPlaylist::playlist_typedef> list = currentPlaylist->getPlaylist();
    list.removeAt(listSound->currentRow());
    currentPlaylist->setPlaylist(list);

    this->updateListMP3();
}

void configPlaylistWindow::buttonIncClicked()
{
    // Si nous sommes a la premiere place, nous ne pouvons pas remonter plus
    if (listSound->currentRow() == 0) return;

    QList<tPlaylist::playlist_typedef> list = currentPlaylist->getPlaylist();
    list.move(listSound->currentRow(), listSound->currentRow()-1);
    currentPlaylist->setPlaylist(list);

    this->updateListMP3();
}

void configPlaylistWindow::buttonDecClicked()
{
    // Si nous sommes a la derniere place, nous ne pouvons pas remonter plus
    if (listSound->currentRow() == listSound->count()-1) return;

    QList<tPlaylist::playlist_typedef> list = currentPlaylist->getPlaylist();
    list.move(listSound->currentRow(), listSound->currentRow()+1);
    currentPlaylist->setPlaylist(list);

    this->updateListMP3();
}

void configPlaylistWindow::buttonOKClicked()
{
    // Quit la fenetre
    this->accept();
}

void configPlaylistWindow::linePathCoverReturnPressed()
{
    this->currentPlaylist->setUrlCover(linePathCover->text());
}

void configPlaylistWindow::buttonPathCoverClicked()
{
    // Ouvre la boite de dialog pour aller selectionner les musiques a ajouter
    QUrl fileName = QFileDialog::getOpenFileUrl(this, tr("Séléctionner le cover de la playlist"),
                                                        QUrl(""),
                                                        tr("Images Files (*.png *.jpeg *.jpg *.bmp)"));

    if (fileName.isEmpty()) return;

    QFileInfo info(fileName.toString());
    this->linePathCover->setText(info.fileName());

    this->currentPlaylist->setUrlCover(fileName.toString());
}

void configPlaylistWindow::listPlaylistItemChanged(QListWidgetItem *item_)
{
    playlist->at(listPlaylist->row(item_))->setNameOfPlaylist(item_->text());
}

void configPlaylistWindow::listPlaylistItemClicked(QListWidgetItem *item_)
{
    buttonPathCover->setEnabled(true);
    buttonRemovePlaylist->setEnabled(true);
    buttonAddMP3->setEnabled(true);
    checkBoxSingle->setEnabled(true);
    checkBoxFullPath->setEnabled(true);
    spinBoxTimeTransition->setEnabled(true);

    QString itemStr = item_->text();
    tPlaylist *pl;
    foreach (pl, *playlist) {
        if (pl->getNameOfPlaylist() == itemStr) {
            currentPlaylist = pl;
            updateListMP3();
        }
    }
}

void configPlaylistWindow::listMP3ItemClicked(QListWidgetItem *item_)
{
    Q_UNUSED(item_)
    buttonRemoveMP3->setEnabled(true);
    buttonDec->setEnabled(true);
    buttonInc->setEnabled(true);
}

void configPlaylistWindow::checkBoxClicked(bool clicked_)
{
    currentPlaylist->setSingle(clicked_);
}

void configPlaylistWindow::checkBoxFullPathClicked(bool clicked_)
{
    Q_UNUSED(clicked_)
    this->updateListMP3();
}

void configPlaylistWindow::spinBoxValueChanged(int value_)
{
    currentPlaylist->setTimeTransition(value_);
}

void configPlaylistWindow::mediaStatusChanged(QMediaPlayer::MediaStatus status_)
{
    if (status_ == QMediaPlayer::LoadedMedia) {

        qDebug() << "mediaStatusChanged";
        // Get Information
        if (this->player->isMetaDataAvailable()) {
            tPlaylist::playlist_typedef *media = new tPlaylist::playlist_typedef();
            media->url = player->currentMedia().canonicalUrl().toString();
            media->title = player->metaData(QMediaMetaData::Title).toString();
            media->artist = player->metaData(QMediaMetaData::ContributingArtist).toString();
            media->duration = player->metaData(QMediaMetaData::Duration).toInt();

            // Add media to playlist
            currentPlaylist->addToPlaylist(*media);

            qDebug() << "New Media, URL: " << media->url << ", Title: " << media->title << ", Artist: "<< media->artist << ", Duration: " << media->duration;


            int index = mediaPlaylist->nextIndex();
            if (index < 0) {
                buttonAddMP3->setEnabled(true);
                buttonOK->setEnabled(true);
                buttonQuit->setEnabled(true);
                //isExtractMode = false;
                //            mediaPlaylist->setCurrentIndex(0);
            }
            else {
                mediaPlaylist->setCurrentIndex(index);
            }
        }
    }
    else if (status_ == QMediaPlayer::InvalidMedia) {
        qDebug() << "Impossible d'ajouter la musique: " << player->currentMedia().canonicalUrl().toString();
        QFileInfo info(player->currentMedia().canonicalUrl().toString());
        QMessageBox::warning(this, tr("Erreur"), QString(tr("Impossible d'ouvrir la musique %1")).arg(info.fileName()));
    }
    this->updateListMP3();
}

void configPlaylistWindow::metaDataChanged()
{
    qDebug() << "metaDataChanged: " << this->player->mediaStatus() << ", meta available: " << this->player->isMetaDataAvailable();
    if (this->player->mediaStatus() == QMediaPlayer::LoadedMedia) {
        // Get Information
        if (this->player->isMetaDataAvailable()) {
            tPlaylist::playlist_typedef media;
            media.url = player->currentMedia().canonicalUrl().toString();
            media.title = player->metaData(QMediaMetaData::Title).toString();
            media.artist = player->metaData(QMediaMetaData::ContributingArtist).toString();
            media.duration = player->duration();

            // Add media to playlist
            currentPlaylist->addToPlaylist(media);

            qDebug() << "New Media, URL: " << media.url << ", Title: " << media.title << ", Artist: "<< media.artist << ", Duration: " << media.duration;

            int index = mediaPlaylist->nextIndex();
            if (index < 0) {
                buttonAddMP3->setEnabled(true);
                buttonOK->setEnabled(true);
                buttonQuit->setEnabled(true);
            }
            else {
                mediaPlaylist->setCurrentIndex(index);
            }
        }
    }
    else if (this->player->mediaStatus() == QMediaPlayer::InvalidMedia) {
        qDebug() << "Impossible d'ajouter la musique: " << player->currentMedia().canonicalUrl().toString();
        QFileInfo info(player->currentMedia().canonicalUrl().toString());
        QMessageBox::warning(this, tr("Erreur"), QString(tr("Impossible d'ouvrir la musique %1")).arg(info.fileName()));
    }
    this->updateListMP3();
}
