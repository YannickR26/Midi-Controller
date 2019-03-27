#include "mainwindow.h"
#include "configpadwindow.h"
#include "configmidiwindow.h"
#include "configappwindow.h"
#include "configplaylistwindow.h"
#include "dynamicfontsizepushbutton.h"
#include "common.h"

#include <QItemSelectionModel>
#include <QMediaMetaData>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->createMenu();
    this->createFrame();

    // Ajoute une bare d'etat
    this->barreEtat = this->statusBar();
    this->barreEtat->showMessage(tr("Prêt"));

    this->modeConfig = false;
    this->mediaPlayer = new tMediaPlayer(2);
    connect(this->mediaPlayer, SIGNAL(positionChanged(qint64)), this, SLOT(mediaPlayerPositionChanged(qint64)));
    connect(this->mediaPlayer, SIGNAL(durationChanged(qint64)), this, SLOT(mediaPlayerDurationChanged(qint64)));
    connect(this->mediaPlayer, SIGNAL(metaDataChanged()), this, SLOT(mediaPlayerMetaDataChanged()));
    connect(this->mediaPlayer, SIGNAL(currentIndexChanged(int)), this, SLOT(mediaPlayerPlaylistPositionChanged(int)));

    this->videoWidget = new tVideoWidget();

    videoWindow = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(this->videoWidget);
    videoWindow->setLayout(layout);
    videoWindow->setAttribute(Qt::WA_DeleteOnClose, false);
    videoWindow->setAttribute(Qt::WA_QuitOnClose, false);
    Qt::WindowFlags flags = 0;
    flags |= Qt::WindowMinimizeButtonHint;
    videoWindow->setWindowFlags(flags);

    this->midi = new tMidi();
    connect(midi, SIGNAL(onNewMessageReceive(tShortMessage*)), this, SLOT(newMidiMsgReceived(tShortMessage*)));

    this->durationCurrent = 0;
    this->durationTotal = 0;
    this->positionCurrent = 0;
    this->positionTotal = 0;

    this->update();

    // Creation du thread pour la reception des données
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerOverflow()));
    timer->setSingleShot(true);
    timer->start(10);

    // Creation du timer pour mise a jour de l'heure
    QTimer *timerClock = new QTimer(this);
    connect(timerClock, SIGNAL(timeout()), this, SLOT(timerClockOverflow()));
    timerClock->start(1000);

    // Force display clock
    timerClockOverflow();
}

MainWindow::~MainWindow()
{
    qInfo() << "Arret du programme";
    qInfo() << "";

    this->mediaPlayer->stop();
    tPad *pad;
    foreach (pad, listPad) {
        if (!pad->deviceMidiIn.isNull()) {
            midi->setColorToAPCMini(pad->msgMidiIn, tShortMessage::BLACK);
        }
    }
    midi->closeAllDevice();
    if (!this->fileNameXml.isEmpty()) {
        mySettings->setValue("FileNameXml", this->fileNameXml);
    }

    mySettings->setValue("videowindow_isHidden", this->videoWindow->isHidden());
    mySettings->setValue("videowindow_size", this->videoWindow->size());
    mySettings->setValue("videowindow_pos", this->videoWindow->pos());
    mySettings->setValue("videowindow_fullScreen", this->videoWidget->isFullScreen());

    mySettings->setValue("mainwindow_size", this->size());
    mySettings->setValue("mainwindow_pos", this->pos());
    mySettings->setValue("mainwindow_fullScreen", this->isFullScreen());

    mySettings->setValue("mainwindow_splitterBtSlider", this->splitterBtSlider->saveState());
    mySettings->setValue("mainwindow_splitterBtPlaylist", this->splitterBtPlaylist->saveState());
    mySettings->setValue("mainwindow_splitterCover", this->splitterCover->saveState());
}

void MainWindow::readSetting()
{
    this->resize(mySettings->value("mainwindow_size",  QSize(800, 500)).toSize());
    this->move(mySettings->value("mainwindow_pos", QPoint(200, 200)).toPoint());

    if (mySettings->value("mainwindow_fullScreen").toBool() == true) {
        this->showFullScreen();
    }

    if (mySettings->value("AutoOpen", 0).toBool() == true) {
        this->fileNameXml = mySettings->value("FileNameXml").toString();
        readXmlFile();
    }

    this->splitterBtSlider->restoreState(mySettings->value("mainwindow_splitterBtSlider").toByteArray());
    this->splitterBtPlaylist->restoreState(mySettings->value("mainwindow_splitterBtPlaylist").toByteArray());
    this->splitterCover->restoreState(mySettings->value("mainwindow_splitterCover").toByteArray());

    this->timeFastForward = mySettings->value("TimeFast", 15).toInt();
}

void MainWindow::readPlaylist(QString &playlist_)
{
    tPlaylist *pl;
    // Cherche la playlist dans la liste
    foreach (pl, listPlaylist) {
        if (pl->getNameOfPlaylist() == playlist_) {
            // Efface l'ancienne playlist
            this->mediaPlayer->clear();
            this->mediaPlayer->setIsSingle(pl->getSingle());
            this->mediaPlayer->setTimeTransition(pl->getTimeTransition());
            // Ajoute tout les MP3
            for (int i=0 ; i<pl->getPlaylist().size() ; ++i) {
                tPlaylist::playlist_typedef media = pl->getPlaylist().at(i);
                this->mediaPlayer->addMedia(QMediaContent(media.url));
                this->listMP3->insertRow(this->listMP3->rowCount());
                this->listMP3->setItem(this->listMP3->rowCount()-1, 0, new QTableWidgetItem(media.title));
                this->listMP3->setItem(this->listMP3->rowCount()-1, 1, new QTableWidgetItem(media.artist));
                // Calcul de la duree de la musique
                int duration = media.duration / 1000;
                QTime currentTime((duration/3600)%60, (duration/60)%60, duration%60, (duration*1000)%1000);
                QString format = "mm:ss";
                if (duration > 3600)
                    format = "hh:mm:ss";
                this->listMP3->setItem(this->listMP3->rowCount()-1, 2, new QTableWidgetItem(currentTime.toString(format)));
            }
//            this->listMP3->resizeColumnsToContents();
            this->listMP3->resizeRowsToContents();
            // Ajoute le signal "Single"
            if (pl->getSingle()) {
                this->labelSingle->setText("Single");
            }
            // Affiche la cover
            if (!pl->getUrlCover().isEmpty()) {
                this->setCover(QUrl(pl->getUrlCover()).toLocalFile());
            }
            else {
                this->setCoverText(pl->getNameOfPlaylist());
            }
            // Positionne le curseur sur la premiere musique
            this->mediaPlayer->setCurrentIndex(0);
            // Recupere la durée total de lecture
            this->durationTotal = pl->getTotalPlaylistDuration() / 1000;
            this->sliderPlaylistPosition->setMaximum(this->durationTotal);
            // Memorise la playlist en cours
            this->playlist = pl;
            // Active le slider de playlist
            this->sliderPlaylistPosition->setEnabled(true);
            qInfo() << "Ouverture de la playlist: " << this->playlist->getNameOfPlaylist();
        }
    }
}

/* Creation du menu principale */
void MainWindow::createMenu()
{
    // Creation des boutons dans Fichier
    QAction *newAct = new QAction(tr("&Nouveau"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Creer un nouvelle scene"));
    connect(newAct, &QAction::triggered, this, &MainWindow::newScene);

    QAction *saveAct = new QAction(tr("&Enregistrer"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Sauvegarde la scene"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::saveScene);

    QAction *saveAsAct = new QAction(tr("&Enregistrer sous ..."), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Sauvegarde la scene"));
    connect(saveAsAct, &QAction::triggered, this, &MainWindow::saveAsScene);

    QAction *openAct = new QAction(tr("&Ouvrir"), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("ouvre une scene existante"));
    connect(openAct, &QAction::triggered, this, &MainWindow::openScene);

    QAction *exitAct = new QAction(tr("&Quitter"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Quitte le logiciel"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    // Creation du menu Fichier
    QMenu *fileMenu = menuBar()->addMenu(tr("&Fichier"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    // Creation des boutons dans Outils
    QAction *editAct = new QAction(tr("&Mode Editeur"), this);
    editAct->setStatusTip(tr("Permet de passer en mode éditeur"));
    editAct->setCheckable(true);
    connect(editAct, SIGNAL(triggered(bool)), this, SLOT(setEditable(bool)));

    QAction *confMidiAct = new QAction(tr("&Midi"), this);
    confMidiAct->setStatusTip(tr("Permet de configurer les interfaces MIDI"));
    connect(confMidiAct, SIGNAL(triggered()), this, SLOT(enterMidiConfig()));

    QAction *confAct = new QAction(tr("&Préférences"), this);
    confAct->setStatusTip(tr("Permet de paramètrer le logiciel"));
    connect(confAct, SIGNAL(triggered()), this, SLOT(enterConfig()));

    QAction *playlistAct = new QAction(tr("&Playlist"), this);
    playlistAct->setStatusTip(tr("Permet de creer des playlist"));
    connect(playlistAct, SIGNAL(triggered()), this, SLOT(enterPlaylist()));

    // Creation du menu Configuration
    QMenu *confMenu = menuBar()->addMenu(tr("&Configuration"));
    confMenu->addAction(editAct);
    confMenu->addAction(confMidiAct);
    confMenu->addAction(confAct);
    confMenu->addAction(playlistAct);

    QAction *fullScreenAct = new QAction(tr("&Plein écran"), this);
    fullScreenAct->setStatusTip(tr("Permet d'afficher le logiciel en mode pleine écran"));
    fullScreenAct->setCheckable(true);
    connect(fullScreenAct, SIGNAL(triggered()), this, SLOT(enterFullScreen()));

    videoWinAct = new QAction(tr("&Vidéo Player"), this);
    videoWinAct->setStatusTip(tr("Permet d'afficher la fenetre de lecture vidéo"));
    videoWinAct->setCheckable(true);
    connect(videoWinAct, SIGNAL(triggered(bool)), this, SLOT(showVideoWindow(bool)));

    // Creation du menu Affichage
    QMenu *fenMenu = menuBar()->addMenu(tr("Fe&nêtre"));
    fenMenu->addAction(fullScreenAct);
    fenMenu->addAction(videoWinAct);

    QAction *aProposAct = new QAction(tr("A propos"), this);
    aProposAct->setStatusTip(tr("A propos du logiciel"));
    connect(aProposAct, SIGNAL(triggered()), this, SLOT(enterAPropos()));

    // Creation du menu Aide
    QMenu *aideMenu = menuBar()->addMenu(tr("?"));
    aideMenu->addAction(aProposAct);
}

/* Creation de la fenetre principale */
void MainWindow::createFrame()
{
    int nbBtCol = mySettings->value("NbColBt", 4).toInt();
    int nbBtRow = mySettings->value("NbRowBt", 5).toInt();
    int nbSlider = mySettings->value("NbSlider", 6).toInt();

    // Creation du label pour l'heure
    labelClock = new DynamicFontSizeLabel();
    labelClock->setFont(QFont(FONT, 40, QFont::Bold, false));
    labelClock->setAlignment(Qt::AlignCenter);

    // Creation du Cover
    labelCover = new DynamicFontSizeLabel();
    labelCover->setFont(QFont(FONT));
    labelCover->setAlignment(Qt::AlignCenter);
    //labelCover->setMaximumSize(600, 600);

    // Creation des label
    labelCurrentAlbum = new ScrollText();
    labelCurrentAlbum->setText(tr("Album"));
    labelCurrentAlbum->setFont(QFont(FONT, 14, QFont::Bold, false));
    labelCurrentAlbum->setMinimumHeight(25);
    labelCurrentTitle = new ScrollText();
    labelCurrentTitle->setText(tr("Titre"));
    labelCurrentTitle->setFont(QFont(FONT, 12, QFont::Bold, true));
    labelCurrentTitle->setMinimumHeight(20);
    labelCurrentArtiste = new ScrollText();
    labelCurrentArtiste->setText(tr("Artiste"));
    labelCurrentArtiste->setFont(QFont(FONT, 10, QFont::Normal, true));
    labelCurrentArtiste->setMinimumHeight(18);
    labelSingle = new QLabel();
    labelSingle->setFont(QFont(FONT, 12, QFont::Bold));
    labelNumberTotal = new QLabel(tr("0 / 0"));
    labelNumberTotal->setFont(QFont(FONT, 18, QFont::Bold));

    // Creation de la position de la musique
    labelSoundStartDuration = new QLabel(tr("0:00"));
    labelSoundStartDuration->setFont(QFont(FONT, 12));
    labelSoundStartDuration->setFixedWidth(60);
    sliderSoundPosition = new QSlider();
    sliderSoundPosition->setOrientation(Qt::Horizontal);
    sliderSoundPosition->setFixedHeight(25);
    connect(sliderSoundPosition, SIGNAL(sliderMoved(int)), this, SLOT(mediaPlayerMovePosition(int)));
    labelSoundEndDuration = new QLabel(tr("0:00"));
    labelSoundEndDuration->setFont(QFont(FONT, 12));
    labelSoundEndDuration->setFixedWidth(60);
    labelSoundEndDuration->setAlignment(Qt::AlignRight);
    // Mise en layout horizontal
    QHBoxLayout *layoutSoundDuration = new QHBoxLayout();
    layoutSoundDuration->addWidget(labelSoundStartDuration);
    layoutSoundDuration->addWidget(sliderSoundPosition, 100);
    layoutSoundDuration->addWidget(labelSoundEndDuration);

    // Creation de la position de la playlist
    labelPlaylistStartDuration = new QLabel(tr("00:00"));
    labelPlaylistStartDuration->setFont(QFont(FONT, 10));
    labelPlaylistStartDuration->setFixedWidth(60);
    sliderPlaylistPosition = new QSlider();
    sliderPlaylistPosition->setOrientation(Qt::Horizontal);
    sliderPlaylistPosition->setFixedHeight(15);
    labelPlaylistEndDuration = new QLabel(tr("00:00"));
    labelPlaylistEndDuration->setFont(QFont(FONT, 10));
    labelPlaylistEndDuration->setFixedWidth(60);
    labelPlaylistEndDuration->setAlignment(Qt::AlignRight);
    // Mise en layout horizontal
    QHBoxLayout *layoutPlaylistDuration = new QHBoxLayout();
    layoutPlaylistDuration->addWidget(labelPlaylistStartDuration);
    layoutPlaylistDuration->addWidget(sliderPlaylistPosition, 100);
    layoutPlaylistDuration->addWidget(labelPlaylistEndDuration);

    // Creation du ListWidget
    listMP3 = new QTableWidget();
    listMP3->setColumnCount(3);
    QStringList header;
    header << "Titre" << "Artiste" << "Durée";
    listMP3->setHorizontalHeaderLabels(header);
    listMP3->setSelectionBehavior(QAbstractItemView::SelectRows);
    listMP3->setEditTriggers(QAbstractItemView::NoEditTriggers);
    listMP3->horizontalHeader()->setStretchLastSection(true);
//    listMP3->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    listMP3->setColumnWidth(0, 250);
    listMP3->setColumnWidth(1, 120);
    listMP3->setColumnWidth(2, 50);
    connect(this->listMP3, SIGNAL(activated(QModelIndex)), this, SLOT(mediaPlayerJumpTo(QModelIndex)));

    // Mise en forme Vertical
    QVBoxLayout *vBoxLabel = new QVBoxLayout();
    vBoxLabel->addWidget(labelCurrentAlbum);
    vBoxLabel->addWidget(labelCurrentTitle);
    vBoxLabel->addWidget(labelCurrentArtiste);

    QVBoxLayout * vboxSingle = new QVBoxLayout();
    vboxSingle->addWidget(labelSingle, 0, Qt::AlignCenter);
    vboxSingle->addWidget(labelNumberTotal, 0, Qt::AlignCenter);

    // Mise en forme Horizontale
    QHBoxLayout *hboxLabel = new QHBoxLayout();
    hboxLabel->addLayout(vBoxLabel, 100);
    hboxLabel->addLayout(vboxSingle);

    // Mise en forme Vertical
    QVBoxLayout *vBoxLayout = new QVBoxLayout();
    vBoxLayout->addLayout(hboxLabel);
    vBoxLayout->addLayout(layoutSoundDuration);
    vBoxLayout->addLayout(layoutPlaylistDuration);
    vBoxLayout->addWidget(listMP3);

    QHBoxLayout *hboxcover = new QHBoxLayout();
    hboxcover->addWidget(labelCover, 100, Qt::AlignCenter);
    QVBoxLayout *vboxcover = new QVBoxLayout();
    vboxcover->addLayout(hboxcover, 100);

    QWidget *widgetCover = new QWidget();
    widgetCover->setLayout(vboxcover);

    QWidget *widgetPlaylist = new QWidget();
    widgetPlaylist->setLayout(vBoxLayout);

    splitterCover = new QSplitter(Qt::Vertical);
    splitterCover->addWidget(labelClock);
    splitterCover->addWidget(labelCover);
    splitterCover->addWidget(widgetPlaylist);
    connect(splitterCover, SIGNAL(splitterMoved(int,int)), this, SLOT(splitterBtPlaylsitMoved(int,int)));

    // Creation de la matrice de bouton
    // Mise en forme en grille
    QGridLayout *gridLayout = new QGridLayout;
    for (int r=0 ; r<nbBtRow ; ++r) {
        gridLayout->setRowStretch(r, 1);
        for (int c=0 ; c<nbBtCol ; ++c) {
            gridLayout->setColumnStretch(c, 1);
            DynamicFontSizePushButton *button = new DynamicFontSizePushButton();
            button->setText(tr("Vide"));
            button->setFont(QFont(FONT, 10));
            button->setMinimumSize(50, 30);
            button->setMaximumSize(1000, 1000);
            button->setCheckable(true);
            connect(button, SIGNAL(clicked()), this, SLOT(buttonPadClicked()));
            listPad.append(new tPad(button));
            gridLayout->addWidget(button, r, c);
        }
    }

    // Mise en forme Horizontal des Bouton Mute
    QHBoxLayout *hBoxLayoutBtMute = new QHBoxLayout();
    // Ajout des Sliders
    for (int c=0 ; c<nbSlider ; ++c) {
        QPushButton *buttonMute = new QPushButton();
        buttonMute->setFont(QFont(FONT, 10));
        buttonMute->setFixedSize(30, 40);
        connect(buttonMute, SIGNAL(clicked()), this, SLOT(buttonPadClicked()));
        listPad.append(new tPad(buttonMute));
        listPad.last()->mode = tPad::MODE_COMMAND;
        listPad.last()->command = tPad::CMD_MUTE;
        hBoxLayoutBtMute->addWidget(buttonMute);
        hBoxLayoutBtMute->setAlignment(buttonMute, Qt::AlignCenter);
    }

    // Mise en forme Horizontal des Sliders
    QHBoxLayout *hBoxLayoutSlider = new QHBoxLayout();
    QHBoxLayout *hBoxLayoutLabel = new QHBoxLayout();
    // Ajout des Sliders
    for (int c=0 ; c<nbSlider ; ++c) {
        QSlider *slider = new QSlider();
        slider->setOrientation(Qt::Orientation::Vertical);
        slider->setMinimumHeight(100);
        slider->setMinimum(0);
        slider->setMaximum(127);
        slider->setTickPosition(QSlider::TicksBothSides);
        connect(slider, SIGNAL(sliderReleased()), this, SLOT(sliderPadClicked()));
        hBoxLayoutSlider->addWidget(slider);
        hBoxLayoutSlider->setAlignment(slider, Qt::AlignHCenter);

        QLabel *label = new QLabel(tr(""));
        label->setFont(QFont(FONT, 14));
        hBoxLayoutLabel->addWidget(label);
        hBoxLayoutLabel->setAlignment(label, Qt::AlignHCenter);

        listPad.append(new tPad(slider, label));
    }

    QWidget *widgetBt = new QWidget();
    widgetBt->setLayout(gridLayout);

    QVBoxLayout *vBoxSlider = new QVBoxLayout();
    vBoxSlider->addLayout(hBoxLayoutBtMute);
    vBoxSlider->addLayout(hBoxLayoutSlider, 100);
    vBoxSlider->addLayout(hBoxLayoutLabel);

    QWidget *widgetSlider = new QWidget();
    widgetSlider->setLayout(vBoxSlider);

    // Mise en forme Vertical pour les boutons et les sliders
    splitterBtSlider = new QSplitter(Qt::Vertical);
    splitterBtSlider->addWidget(widgetBt);
    splitterBtSlider->addWidget(widgetSlider);
    connect(splitterBtSlider, SIGNAL(splitterMoved(int,int)), this, SLOT(splitterBtSliderMoved(int,int)));

    QHBoxLayout *hBoxLayoutSlitterBtSlider = new QHBoxLayout();
    hBoxLayoutSlitterBtSlider->addWidget(splitterBtSlider);
    QWidget *widgetBtSlider = new QWidget();
    widgetBtSlider->setLayout(hBoxLayoutSlitterBtSlider);

    splitterBtPlaylist = new QSplitter(Qt::Horizontal);
    splitterBtPlaylist->addWidget(splitterCover);
    splitterBtPlaylist->addWidget(widgetBtSlider);
    connect(splitterBtPlaylist, SIGNAL(splitterMoved(int,int)), this, SLOT(splitterBtPlaylsitMoved(int,int)));

    // Mise en forme Horizontal
    QHBoxLayout *hBoxLayout = new QHBoxLayout();
    hBoxLayout->addWidget(splitterBtPlaylist);

    // Set layout in QWidget
    QWidget *window = new QWidget();
    window->setLayout(hBoxLayout);

    // Apply frame to central widget
    setCentralWidget(window);
}

void MainWindow::newMidiMsgReceived(tShortMessage *msg_)
{
    qDebug() << "Reception donnée Midi, " << msg_->command << ", " << msg_->channel << ", " << msg_->number << ", " << msg_->value;

    foreach (tPad *pad, listPad) {
        // Si nous avons un device midi, nous cherchons une corelations
        if (!pad->deviceMidiIn.isEmpty()) {
            if (pad->msgMidiIn.isEqual(msg_)) {
                // Si c'est un bouton
                if (pad->button != NULL) {
                    actionFromPad(pad);
                }

                // Si c'est un slider
                else if (pad->slider != NULL) {
                    pad->slider->setValue(msg_->value);
                }

                // Si nous avons une translation à faire
                if (!pad->deviceMidiOut.isEmpty()) {
                    if (msg_->command == tShortMessage::CONTROL_CHANGE) {
                        pad->msgMidiOut.value = msg_->value * (pad->msgMidiOut.valueMax - pad->msgMidiOut.valueMin)/127 + pad->msgMidiOut.valueMin;
                    }
                    else if (msg_->command == tShortMessage::NOTE_ON) {
                        if (pad->msgMidiOut.value == pad->msgMidiOut.valueMin) {
                            pad->msgMidiOut.value = pad->msgMidiOut.valueMax;
                        }
                        else {
                            pad->msgMidiOut.value = pad->msgMidiOut.valueMin;
                        }
                    }
                    qDebug() << "Envoie donnée Midi, " << pad->msgMidiOut.command << ", " << pad->msgMidiOut.channel << ", " << pad->msgMidiOut.number << ", " << pad->msgMidiOut.value;
                    midi->sendMessage(pad->deviceMidiOut, pad->msgMidiOut);
                }
            }
        }
    }
}

void MainWindow::newScene()
{
    // Supprime le fichier deja ouvert
    this->fileNameXml.clear();

    // Eteint tout les pad
    tPad *pad;
    foreach (pad, listPad) {
        if (!pad->deviceMidiIn.isNull()) {
            midi->setColorToAPCMini(pad->msgMidiIn, tShortMessage::BLACK);
        }
    }

    // Supprime les configurations des pad
    foreach (pad, listPad) {
        pad->clear();
        pad->update();
    }

    // Supprime les playlist
    this->listPlaylist.clear();

    // Supprime la playlist du widget
    this->listMP3->clear();

    // Supprime les textes de la musique en cours
    this->labelCurrentAlbum->setText(tr("Album"));
    this->labelCurrentArtiste->setText(tr("Artiste"));
    this->labelCurrentTitle->setText(tr("Titre"));

    // Remet à 0 les slider
    this->durationCurrent = 0;
    this->durationTotal = 0;
    this->positionCurrent = 0;
    this->positionTotal = 0;

    // Supprime la playlist du mediaPlayer
    this->mediaPlayer->clear();
}

void MainWindow::openScene()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Ouvre une configuration"), "", tr("XML Files (*.xml)"));

    if (filename.isEmpty()) return;

    this->newScene();

    this->fileNameXml = filename;

    this->readXmlFile();
}

void MainWindow::saveScene()
{
    if (this->fileNameXml.isEmpty()) {
        this->fileNameXml = QFileDialog::getSaveFileName(this, tr("Enregistre la configuration"), "", tr("XML Files (*.xml)"));
    }

    // Si nous n'avons pas choisi de fichier, on quit
    if (this->fileNameXml.isEmpty()) return;

    // Ecrit le fichier de configuration
    writeXmlFile();
}

void MainWindow::saveAsScene()
{
    this->fileNameXml = QFileDialog::getSaveFileName(this, tr("Enregistre la configuration"), "", tr("XML Files (*.xml)"));

    // Si nous n'avons pas choisi de fichier, on quit
    if (this->fileNameXml.isEmpty()) return;

    // Ecrit le fichier de configuration
    writeXmlFile();
}

void MainWindow::writeXmlFile()
{
    // Ouvre le fichier Xml
    QDomDocument dom("XML");
    QFile xml_doc(this->fileNameXml);
    if (!xml_doc.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Erreur à l'ouverture du document XML", "Impossible d'ouvrir le fichier !");
        return;
    }

    // Crée la 1ere Balise Xml
    QDomElement child = dom.createElement("Gestion_Sono");
    dom.appendChild(child);

    // Crée la Balise Pad_List
    QDomElement pad_list = dom.createElement("Pad_List");
    child.appendChild(pad_list);

    // Ajoute la configuration des Pad
    tPad *pad;
    foreach (pad, listPad) {
        pad->toXml(dom, pad_list);
    }

    // Crée la Balise Playlist_List
    QDomElement playlist_list = dom.createElement("Playlist_List");
    child.appendChild(playlist_list);

    // Ajoute la configuration des Pad
    tPlaylist *playlist;
    foreach (playlist, listPlaylist) {
        playlist->toXml(dom, playlist_list);
    }

    // Crée la Balise Midi_Device
    QDomElement midi_list = dom.createElement("Midi_Device");
    child.appendChild(midi_list);

    // Ajoute la configuration du midi
    midi->toXml(dom, midi_list);

    // Crée le string Xml
    QString xml = dom.toString();

    // Ecrit le fichier
    QTextStream ts(&xml_doc);
    ts << xml.toUtf8();

    xml_doc.close();

    barreEtat->showMessage(tr("Fichier enregistrer"));
}

void MainWindow::readXmlFile()
{
    if (this->fileNameXml.isEmpty()) return;

    qInfo() << "Ouverture du fichier: " << this->fileNameXml;

    // Ouvre le fichier Xml
    QDomDocument dom("XML");
    QFile xml_doc(this->fileNameXml);
    if(!xml_doc.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, "Erreur à l'ouverture du document XML", "Impossible d'ouvrir le fichier !");
        return;
    }
    if (!dom.setContent(&xml_doc))
    {
        xml_doc.close();
        QMessageBox::warning(this, "Erreur à l'ouverture du document XML", "Le document XML n'a pas pu être attribué à l'objet QDomDocument.");
        return;
    }
    xml_doc.close();
    QDomElement dom_element = dom.documentElement();

    // Verifie la premiere Balise
    if (dom_element.tagName() == "Gestion_Sono") {
        QDomElement child = dom_element.firstChildElement();
        while(!child.isNull()) {
            // Si c'est un pad_list
            if (child.tagName() == "Pad_List") {
                QDomElement smallChild = child.firstChildElement();
                // Ajoute à tous les Pad déja existant, la configuration
                tPad *pad;
                foreach (pad, listPad) {
                    pad->fromXml(smallChild);
                    smallChild = smallChild.nextSiblingElement();
                }
            }
            else if (child.tagName() == "Playlist_List") {
                QDomElement smallChild = child.firstChildElement();
                while(!smallChild.isNull()) {
                    // On crée les playlist
                    this->listPlaylist.append(new tPlaylist(smallChild));
                    smallChild = smallChild.nextSiblingElement();
                }
            }
            else if (child.tagName() == "Midi_Device") {
                QDomElement smallChild = child.firstChildElement();
                midi->fromXml(smallChild);
            }
            child = child.nextSiblingElement();
        }
    }

    // Mise a jour des boutons du APC Mini
    SendConfigToAPC();

    barreEtat->showMessage(tr("Lecture du fichier terminer"));
}

void MainWindow::SendConfigToAPC()
{
    tPad *pad;
    foreach (pad, listPad) {
        // Si nous avons configurer un device Midi
        if (!pad->deviceMidiIn.isEmpty()) {
            switch (pad->mode) {
            case tPad::MODE_PLAYLIST:
                midi->setColorToAPCMini(pad->msgMidiIn, tShortMessage::ORANGE);
                break;

            case tPad::MODE_SAMPLE:
                midi->setColorToAPCMini(pad->msgMidiIn, tShortMessage::ORANGE);
                break;

            case tPad::MODE_COMMAND:
                midi->setColorToAPCMini(pad->msgMidiIn, tShortMessage::RED);
                break;

            default:
                break;
            }
            // Si nous avons une translation et que c'est un bouton
            if ((pad->mode == tPad::MODE_TRANSLATION) && (pad->button != NULL)) {
                midi->setColorToAPCMini(pad->msgMidiIn, tShortMessage::GREEN);
            }
        }
    }
}

void MainWindow::setEditable(bool checked_)
{
    if (checked_ == true) {
        foreach (tPad *pad, listPad) {
            pad->setEnable(true);
        }
    }
    else {
        foreach (tPad *pad, listPad) {
            pad->update();
        }
    }
    modeConfig = checked_;
}

void MainWindow::enterMidiConfig()
{
    configMidiWindow configMidi(midi);
    configMidi.exec();
}

void MainWindow::enterConfig()
{
    configAppWindow config;
    config.exec();
}

void MainWindow::enterPlaylist()
{
    configPlaylistWindow playlist(&listPlaylist);
    playlist.exec();
}

void MainWindow::enterFullScreen()
{
    if (this->isFullScreen()) {
        this->showNormal();
    }
    else {
        this->showFullScreen();
    }
}

void MainWindow::showVideoWindow(bool checked_)
{
    if (checked_ == true) {
        this->videoWindow->resize(mySettings->value("videowindow_size",  QSize(800, 500)).toSize());
        this->videoWindow->move(mySettings->value("videowindow_pos", QPoint(200, 200)).toPoint());
        if (mySettings->value("videowindow_fullScreen").toBool() == true) {
            this->videoWidget->setFullScreen(true);
        }
        this->videoWindow->show();
    }
    else {
        mySettings->setValue("videowindow_size", this->videoWindow->size());
        mySettings->setValue("videowindow_pos", this->videoWindow->pos());
        mySettings->setValue("videowindow_fullScreen", this->videoWidget->isFullScreen());
        videoWindow->hide();
    }
}

void MainWindow::enterAPropos()
{
   QMessageBox::information(this, tr("A propos"), QString(tr("Logiciel Open Source de gestion de playlist audio/vidéo par MIDI\n V1.4 créer par \"BG26\"")));
}

void MainWindow::updatePad()
{
    foreach (tPad *pad, listPad) {
        pad->update();
    }
}

void MainWindow::timerOverflow()
{
    if (mySettings->value("videowindow_isHidden").toBool() == false) {
        this->videoWinAct->setChecked(true);
        this->showVideoWindow(true);
    }
    this->updatePad();

    this->readSetting();

    this->update();

    setCover(PIXMAP_NO_COVER);
}

void MainWindow::timerClockOverflow()
{
    this->labelClock->setText(QTime::currentTime().toString("hh:mm"));
}

void MainWindow::actionFromPad(tPad *pad_)
{
    tPad *pad;
    // Si c'est un bouton
    if (pad_->button != NULL) {
        switch (pad_->mode) {
        case tPad::MODE_PLAYLIST:
            // Si le bouton est appuyer
            if (!pad_->button->isChecked()) {
                if (mediaPlayer->getCurrentPlaylist()->isEmpty()) {
                    pad_->button->setChecked(true);
                    labelCurrentAlbum->setText(QFileInfo(pad_->playlist).baseName());
                    readPlaylist(pad_->playlist);
                    // Allume en orange les commandes et en rouge les autres (MP3 et Playlist)
                    foreach (pad, listPad) {
                        if (pad->mode == tPad::MODE_COMMAND) {
                            midi->setColorToAPCMini(pad->msgMidiIn, tShortMessage::ORANGE);
                        }
                        if (pad->mode == tPad::MODE_PLAYLIST) {
                            midi->setColorToAPCMini(pad->msgMidiIn, tShortMessage::RED);
                        }
                    }
                    // Alume en Vert le pad selectionné
                    midi->setColorToAPCMini(pad_->msgMidiIn, tShortMessage::GREEN);
                }
            }
            else {
                qInfo() << "Fermeture de la playlist";
                pad_->button->setChecked(false);
                labelCurrentAlbum->setText(tr("Album"));
                labelCurrentTitle->setText(tr("Titre"));
                labelCurrentArtiste->setText(tr("Artiste"));
                this->labelSingle->setText("");
                this->labelNumberTotal->setText("0 / 0");
                this->positionCurrent = 0;
                this->positionTotal = 0;
                this->durationCurrent = 0;
                this->durationTotal = 0;
                this->mediaPlayer->clear();
                this->setCover(PIXMAP_NO_COVER);
                while (this->listMP3->rowCount()) {
                    listMP3->removeRow(0);
                }
                midi->setColorToAPCMini(pad_->msgMidiIn, tShortMessage::ORANGE);
                // Allume en rouge les commandes et en orange les autres
                foreach (pad, listPad) {
                    if (pad->mode == tPad::MODE_COMMAND) {
                        midi->setColorToAPCMini(pad->msgMidiIn, tShortMessage::RED);
                        if (pad->command == tPad::CMD_PLAY_PAUSE) {
                            pad->setIcon(ICON_PLAY);
                        }
                    }
                    if (pad->mode == tPad::MODE_PLAYLIST) {
                        midi->setColorToAPCMini(pad->msgMidiIn, tShortMessage::ORANGE);
                    }
                }
                this->mediaPlayerupdateDurationInfo();
            }
            break;

        case tPad::MODE_SAMPLE:
            if (!pad_->button->isChecked()) {
                qInfo() << "Lecture du sample: " << pad_->urlSample;
                pad_->button->setChecked(true);
                pad_->playSample();
                if (pad_->urlSample.contains(".avi") || pad_->urlSample.contains(".mp4") || pad_->urlSample.contains(".mov") || pad_->urlSample.contains(".mkv")) {
                    pad_->getMediaPlayer()->setVideoOutput(videoWidget);
                }
                connect(pad_->getMediaPlayer(), SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(sampleStateChanged(QMediaPlayer::State)));
                // Alume en Vert le pad selectionné
                midi->setColorToAPCMini(pad_->msgMidiIn, tShortMessage::GREEN);
            }
            else {
                qInfo() << "Arret du sample: " << pad_->urlSample;
                pad_->button->setChecked(false);
                pad_->stopSample();
                disconnect(pad_->getMediaPlayer(), SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(sampleStateChanged(QMediaPlayer::State)));
                // Alume en Vert le pad selectionné
                midi->setColorToAPCMini(pad_->msgMidiIn, tShortMessage::ORANGE);
            }
            break;

        case tPad::MODE_COMMAND:
            switch (pad_->command) {
            case tPad::CMD_RECULE_RAPIDE:
                qInfo() << "Commande: Recule rapide";
                this->mediaPlayer->setPosition(this->mediaPlayer->position() - this->timeFastForward * 1000);
                break;

            case tPad::CMD_PRECEDENT:
                qInfo() << "Commande: Precedent";
                if (this->mediaPlayer->state() == tMediaPlayer::PlayingState) {
                    this->mediaPlayer->previous();
                    this->mediaPlayer->play();
                }
                else {
                    this->mediaPlayer->previous();
                }
                break;

            case tPad::CMD_PLAY_PAUSE:
                if ((mediaPlayer->mediaStatus() != tMediaPlayer::NoMedia) && !this->mediaPlayer->getCurrentPlaylist()->isEmpty()) {
                    if (mediaPlayer->state() == tMediaPlayer::PlayingState) {
                        qInfo() << "Commande: Pause";
                        mediaPlayer->pause();
                        midi->setColorToAPCMini(pad_->msgMidiIn, tShortMessage::GREEN_BLINK);
                        pad_->setIcon(ICON_PLAY);
                    }
                    else {
                        qInfo() << "Commande: Play";
                        mediaPlayer->play();
                        midi->setColorToAPCMini(pad_->msgMidiIn, tShortMessage::GREEN);
                        pad_->setIcon(ICON_PAUSE);
                    }
                }
                break;

            case tPad::CMD_STOP:
                if (mediaPlayer->mediaStatus() != tMediaPlayer::NoMedia) {
                    qInfo() << "Commande: Stop";
                    mediaPlayer->stop();
                    // Allume en orange les commandes
                    foreach (pad, listPad) {
                        if ((pad->mode == tPad::MODE_COMMAND) && (pad->command == tPad::CMD_PLAY_PAUSE)) {
                            midi->setColorToAPCMini(pad->msgMidiIn, tShortMessage::ORANGE);
                            pad->setIcon(ICON_PLAY);
                        }
                    }
                }
                break;

            case tPad::CMD_SUIVANT:
                qInfo() << "Commande: Suivant";
                if (this->mediaPlayer->state() == tMediaPlayer::PlayingState) {
                    this->mediaPlayer->next();
                    this->mediaPlayer->play();
                }
                else {
                    this->mediaPlayer->next();
                }
                break;

            case tPad::CMD_AVANCE_RAPIDE:
                qInfo() << "Commande: Avance Rapide";
                this->mediaPlayer->setPosition(this->mediaPlayer->position() + this->timeFastForward * 1000);
                break;

            default:

            case tPad::CMD_MUTE:
                if (pad_->isMuted) {
                    midi->setColorToAPCMini(pad_->msgMidiIn, tShortMessage::GREEN);
                    pad_->setIcon(ICON_VOLUME);
                    pad_->isMuted = false;
                }
                else {
                    midi->setColorToAPCMini(pad_->msgMidiIn, tShortMessage::GREEN_BLINK);
                    pad_->setIcon(ICON_MUTE);
                    pad_->isMuted = true;
                }
                break;

            case tPad::CMD_UNKNOWN:
                pad_->button->setChecked(false);
                break;
            }

            break;

        default:
        case tPad::MODE_DISABLE:
//            pad_->button->setChecked(false);
            break;
        }

    }
    // Si c'est un slider
    else if (pad_->slider != NULL) {

    }
}

void MainWindow::buttonPadClicked()
{
    // Get sender
    QPushButton* button = qobject_cast<QPushButton*>(sender());

    // Mode configuration, on ouvre la fenetre de config
    if (modeConfig) {
        // Cherche le pad qu'on a appuyer
        tPad *pad;
        foreach (pad, listPad) {
            if (button == pad->button) {
                configPadWindow win(pad, midi, &listPlaylist);
                disconnect(midi, SIGNAL(onNewMessageReceive(tShortMessage*)), this, SLOT(newMidiMsgReceived(tShortMessage*)));
                win.exec();
                SendConfigToAPC();
                connect(midi, SIGNAL(onNewMessageReceive(tShortMessage*)), this, SLOT(newMidiMsgReceived(tShortMessage*)));
            }
        }
        button->setChecked(false);
    }
    // Mode normal
    else {
        // Cherche le pad qu'on a appuyer
        tPad *pad;
        foreach (pad, listPad) {
            if (button == pad->button) {
                button->setChecked(!button->isChecked());
                actionFromPad(pad);
            }
        }
    }
}

void MainWindow::sliderPadClicked()
{
    // Get sender
    QSlider* slider = qobject_cast<QSlider*>(sender());

    // Mode configuration, on ouvre la fenetre de config
    if (modeConfig) {
        // Cherche le pad qu'on a appuyer
        tPad *pad;
        foreach (pad, listPad) {
            if (slider == pad->slider) {
                configPadWindow win(pad, midi, &listPlaylist);
                disconnect(midi, SIGNAL(onNewMessageReceive(tShortMessage*)), this, SLOT(newMidiMsgReceived(tShortMessage*)));
                if (win.exec() == QDialog::Accepted) {
                    // Si nous avons selectionner un device midi, nous l'ouvrons
//                    if (!pad->deviceMidiIn.isEmpty()) {
//                        midi->openDeviceIn(midi->getIdxFromDeviceIn(pad->deviceMidiIn));
//                    }
                }
                connect(midi, SIGNAL(onNewMessageReceive(tShortMessage*)), this, SLOT(newMidiMsgReceived(tShortMessage*)));
            }
        }
    }
    // Mode normal
    else {
        // Cherche le slider qu'on a bouger
        tPad *pad;
        foreach (pad, listPad) {
            if (slider == pad->slider) {
                actionFromPad(pad);
            }
        }
    }
}

void MainWindow::mediaPlayerupdateDurationInfo()
{
    QString tStr;

    // Calcul de la position de la musique en cours
    QTime currentTime((this->positionCurrent/3600)%60, (this->positionCurrent/60)%60, this->positionCurrent%60, (this->positionCurrent*1000)%1000);
    QString format = "mm:ss";
    if (this->positionCurrent > 3600)
        format = "hh:mm:ss";
    tStr = currentTime.toString(format);
    this->labelSoundStartDuration->setText(tStr);

    // Calcul de la durée réstante de la musique en cours
    qint64 duration;
    if (this->positionCurrent <= this->durationCurrent) {
        duration = this->durationCurrent - this->positionCurrent;
    }
    else {
        duration = 0;
    }
    QTime currentDuration((duration/3600)%60, (duration/60)%60, duration%60, (duration*1000)%1000);
    format = "mm:ss";
    if (duration > 3600)
        format = "hh:mm:ss";
    tStr = currentDuration.toString(format);
    this->labelSoundEndDuration->setText(tStr);

    // Calcul de la position de toute la playlist
    if (this->sliderPlaylistPosition->isEnabled()) {
        qint64 posPlaylist = this->positionTotal + this->positionCurrent;
        QTime totalTime((posPlaylist/3600)%60, (posPlaylist/60)%60, posPlaylist%60, (posPlaylist*1000)%1000);
        format = "mm:ss";
        if (posPlaylist > 3600)
            format = "hh:mm:ss";
        tStr = totalTime.toString(format);
        this->labelPlaylistStartDuration->setText(tStr);

        // Calcul de la durée restante de toute la playlist
        qint64 durationPlaylist = 0;
        if (this->durationTotal >= posPlaylist)
            durationPlaylist = this->durationTotal - posPlaylist;
        QTime totalDuration((durationPlaylist/3600)%60, (durationPlaylist/60)%60, durationPlaylist%60, (durationPlaylist*1000)%1000);
        format = "mm:ss";
        if (durationPlaylist > 3600)
            format = "hh:mm:ss";
        tStr = totalDuration.toString(format);
        this->labelPlaylistEndDuration->setText(tStr);

        if (!sliderPlaylistPosition->isSliderDown()) {
            sliderPlaylistPosition->setValue(posPlaylist);
        }
    }

    if (!sliderSoundPosition->isSliderDown()) {
        sliderSoundPosition->setValue(this->positionCurrent);
    }

    tStr = QString("%1 / %2").arg(this->mediaPlayer->currentIndex()+1).arg(this->mediaPlayer->getCurrentPlaylist()->mediaCount());
    this->labelNumberTotal->setText(tStr);
}

void MainWindow::setCover(QString urlCover_)
{
    this->pixmapCover = QPixmap(urlCover_);
    this->labelCover->setPixmap(pixmapCover.scaled(this->labelCover->size(), Qt::KeepAspectRatio));
}

void MainWindow::setCoverText(QString text_)
{
    this->labelCover->setPixmap(QPixmap());
    this->labelCover->setText(text_);
}

void MainWindow::mediaPlayerMovePosition(int newPosition_)
{
    this->mediaPlayer->setPosition(newPosition_ * 1000);
}

void MainWindow::mediaPlayerPositionChanged(qint64 position_)
{
    this->positionCurrent = position_ / 1000;

    this->mediaPlayerupdateDurationInfo();
}

void MainWindow::mediaPlayerDurationChanged(qint64 duration_)
{
    this->sliderSoundPosition->setMaximum(duration_ / 1000);

    if (this->sliderPlaylistPosition->isEnabled()) {
        this->positionTotal = this->playlist->getTotalDuration(this->mediaPlayer->currentIndex()) / 1000;
    }

    this->durationCurrent = duration_ / 1000;

    if (this->mediaPlayer->state() != tMediaPlayer::PlayingState) {
        // Change le logo du bouton
        foreach (tPad *pad, listPad) {
            if ((pad->mode == tPad::MODE_COMMAND) && (pad->command == tPad::CMD_PLAY_PAUSE)) {
                midi->setColorToAPCMini(pad->msgMidiIn, tShortMessage::ORANGE);
                pad->setIcon(ICON_PLAY);
            }
        }
    }

    this->mediaPlayerupdateDurationInfo();
}

void MainWindow::mediaPlayerPlaylistPositionChanged(int newIndex_)
{
    this->listMP3->selectRow(newIndex_);
}

void MainWindow::mediaPlayerJumpTo(QModelIndex newIndex_)
{
    if (newIndex_.isValid()) {
        this->mediaPlayer->setCurrentIndex(newIndex_.row());
        this->mediaPlayer->play();

        // Allume en vert le bouton play/pause
        foreach (tPad *pad, listPad) {
            if ((pad->mode == tPad::MODE_COMMAND) && (pad->command == tPad::CMD_PLAY_PAUSE)) {
                midi->setColorToAPCMini(pad->msgMidiIn, tShortMessage::GREEN);
                pad->setIcon(ICON_PAUSE);
            }
        }
    }
}

void MainWindow::mediaPlayerMetaDataChanged()
{
    if (this->mediaPlayer->isMetaDataAvailable()) {
        this->labelCurrentTitle->setText(this->mediaPlayer->metaData(QMediaMetaData::Title).toString());
        this->labelCurrentArtiste->setText(this->mediaPlayer->metaData(QMediaMetaData::ContributingArtist).toString());
    }
}

void MainWindow::sampleStateChanged(QMediaPlayer::State state_)
{
    // Get sender
    QMediaPlayer* player = qobject_cast<QMediaPlayer*>(sender());

    // Cherche le sample
    foreach (tPad *pad, listPad) {
        if (player == pad->getMediaPlayer()) {
            if (state_ == QMediaPlayer::StoppedState) {
                pad->button->setChecked(false);
                midi->setColorToAPCMini(pad->msgMidiIn, tShortMessage::ORANGE);
            }
        }
    }

}
