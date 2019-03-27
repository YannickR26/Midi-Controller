#include "configpadwindow.h"

#include <QMessageBox>
#include <QPushButton>
#include <QLayout>
#include <QFileDialog>


configPadWindow::configPadWindow(tPad *pad_, tMidi *midi_, QList<tPlaylist *> *playlist_, QDialog *parent)
    : QDialog(parent)
{
    this->setWindowTitle(tr("Configuration"));

    pad = pad_;
    midi = midi_;
    playlist = playlist_;

    // Creation des lineEdit
    lineName = new QLineEdit();
    linePathIcon = new QLineEdit();
    linePathSample = new QLineEdit();

    // Creation des comboBox
    comboBoxCmd = new QComboBox();
    QStringList listOfCommands;
    listOfCommands << tr("Recule Rapide") << tr("Précédent") << tr("Play/Pause") << tr("Stop") << tr("Suivant") << tr("Avance Rapide") << tr("Muet");
    comboBoxCmd->addItems(listOfCommands);
    comboBoxM3U = new QComboBox();
    for (int i=0 ; i<playlist->size() ; ++i) {
        comboBoxM3U->addItem(playlist->at(i)->getNameOfPlaylist());
    }
    comboBoxMidiIn = new QComboBox();
    comboBoxMidiIn->setMinimumWidth(170);
    comboBoxMidiIn->addItems(midi->getDeviceInOpen());
    comboBoxMidiOut = new QComboBox();
    comboBoxMidiOut->addItems(midi->getDeviceOutOpen());
    comboBoxMidiOutType = new QComboBox();
    QStringList listOfType;
    listOfType << tr("NOTE_ON") << tr("NOTE_OFF") << tr("POLY_PRESSURE") << tr("CONTROL_CHANGE") << tr("PROGRAM_CHANGE") << tr("CHANNEL_PRESSURE") << tr("PITCH_BEND");
    comboBoxMidiOutType->addItems(listOfType);

    // Creation des spinBox
    spinBoxMidiOutChannel = new QSpinBox();
    spinBoxMidiOutChannel->setMinimum(0);
    spinBoxMidiOutChannel->setMaximum(15);
    spinBoxMidiOutNumber = new QSpinBox();
    spinBoxMidiOutNumber->setMinimum(0);
    spinBoxMidiOutNumber->setMaximum(127);
    spinBoxMidiOutValMin = new QSpinBox();
    spinBoxMidiOutValMin->setMinimum(0);
    spinBoxMidiOutValMin->setMaximum(127);
    spinBoxMidiOutValMax = new QSpinBox();
    spinBoxMidiOutValMax->setMinimum(0);
    spinBoxMidiOutValMax->setMaximum(127);

    // Creation des CheckBox
    checkBoxSampleInfinite = new QCheckBox(tr("En boucle"));
    checkBoxSampleInfinite->setCheckable(true);
    checkBoxSampleInfinite->setChecked(false);
    checkBoxSampleMute = new QCheckBox(tr("Silencieux"));
    checkBoxSampleMute->setCheckable(true);
    checkBoxSampleMute->setChecked(false);

    // Creation des label
    labelMidiInType = new QLabel(tr("Type:"));
    labelMidiInChannel = new QLabel(tr("Channel:"));
    labelMidiInNumber = new QLabel(tr("Number:"));
    labelMidiInValue = new QLabel(tr("Value:"));

    // Creation des pushButton
    QPushButton *buttonPathIcon = new QPushButton(tr("..."));
    buttonPathIcon->setMaximumWidth(40);
    QPushButton *buttonPathMP3 = new QPushButton(tr("..."));
    buttonPathMP3->setMaximumWidth(40);
    QPushButton *buttonOK = new QPushButton(tr("OK"));
    QPushButton *buttonQuit = new QPushButton(tr("Annuler"));

    // Connection des slot
    connect(buttonPathIcon, SIGNAL(clicked()), this, SLOT(buttonIconClicked()));
    connect(buttonPathMP3, SIGNAL(clicked()), this, SLOT(buttonSampleClicked()));
    connect(buttonOK, SIGNAL(clicked()), this, SLOT(buttonOKClicked()));
    connect(buttonQuit, SIGNAL(clicked()), this, SLOT(reject()));

    // Mise en forme en ligne pour le nom
    QHBoxLayout *hBoxName = new QHBoxLayout();
    hBoxName->addWidget(new QLabel(tr("Nom :")));
    hBoxName->addWidget(lineName);

    // Mise en forme dans des groupBox
    // Icon
    QHBoxLayout *hBoxIcon = new QHBoxLayout();
    hBoxIcon->addWidget(new QLabel(tr("Icone :")));
    hBoxIcon->addWidget(linePathIcon);
    hBoxIcon->addWidget(buttonPathIcon);
    // Command
    QHBoxLayout *hBoxCmd = new QHBoxLayout();
    hBoxCmd->addWidget(comboBoxCmd);
    groupBoxCmd = new QGroupBox(tr("Commande"));
    groupBoxCmd->setCheckable(true);
    groupBoxCmd->setChecked(false);
    groupBoxCmd->setLayout(hBoxCmd);
    connect(groupBoxCmd, SIGNAL(clicked(bool)), this, SLOT(checkBoxCmdClicked(bool)));
    // M3U
    QHBoxLayout *hBoxM3U = new QHBoxLayout();
    hBoxM3U->addWidget(comboBoxM3U);
    groupBoxM3U = new QGroupBox(tr("Playlist"));
    groupBoxM3U->setCheckable(true);
    groupBoxM3U->setChecked(false);
    groupBoxM3U->setLayout(hBoxM3U);
    connect(groupBoxM3U, SIGNAL(clicked(bool)), this, SLOT(checkBoxM3UClicked(bool)));
    // Sample
    QHBoxLayout *hBoxSample = new QHBoxLayout();
    hBoxSample->addWidget(linePathSample);
    hBoxSample->addWidget(buttonPathMP3);
    QHBoxLayout *hBoxSampleCheckbox = new QHBoxLayout();
    hBoxSampleCheckbox->addWidget(checkBoxSampleInfinite);
    hBoxSampleCheckbox->addWidget(checkBoxSampleMute);
    QVBoxLayout *vboxSample = new QVBoxLayout();
    vboxSample->addLayout(hBoxSampleCheckbox);
    vboxSample->addLayout(hBoxSample);
    groupBoxSample = new QGroupBox(tr("Sample"));
    groupBoxSample->setCheckable(true);
    groupBoxSample->setChecked(false);
    groupBoxSample->setLayout(vboxSample);
    connect(groupBoxSample, SIGNAL(clicked(bool)), this, SLOT(checkBoxSampleClicked(bool)));
    // Midi In
    QVBoxLayout *vBoxMidiIn = new QVBoxLayout();
    vBoxMidiIn->addWidget(comboBoxMidiIn);
    vBoxMidiIn->addWidget(labelMidiInType);
    vBoxMidiIn->addWidget(labelMidiInChannel);
    vBoxMidiIn->addWidget(labelMidiInNumber);
    vBoxMidiIn->addWidget(labelMidiInValue);
    groupBoxMidiIn = new QGroupBox(tr("Midi Input"));
    groupBoxMidiIn->setCheckable(true);
    groupBoxMidiIn->setChecked(false);
    groupBoxMidiIn->setLayout(vBoxMidiIn);
    // Midi Out
    // Mise en forme Horizontale pour toute les valeur du midi Out
    QHBoxLayout *hBoxMidiOutType = new QHBoxLayout();
    hBoxMidiOutType->addWidget(new QLabel(tr("Type: ")));
    hBoxMidiOutType->addWidget(comboBoxMidiOutType);
    QHBoxLayout *hBoxMidiOutChannel = new QHBoxLayout();
    hBoxMidiOutChannel->addWidget(new QLabel(tr("Channel: ")));
    hBoxMidiOutChannel->addWidget(spinBoxMidiOutChannel);
    QHBoxLayout *hBoxMidiOutNumber = new QHBoxLayout();
    hBoxMidiOutNumber->addWidget(new QLabel(tr("Number: ")));
    hBoxMidiOutNumber->addWidget(spinBoxMidiOutNumber);
    QHBoxLayout *hBoxMidiOutVal = new QHBoxLayout();
    hBoxMidiOutVal->addWidget(new QLabel(tr("Value: ")));
    hBoxMidiOutVal->addWidget(spinBoxMidiOutValMin);
    hBoxMidiOutVal->addWidget(new QLabel(tr(" to")));
    hBoxMidiOutVal->addWidget(spinBoxMidiOutValMax);
    QVBoxLayout *vBoxMidiOut = new QVBoxLayout();
    vBoxMidiOut->addWidget(comboBoxMidiOut);
    vBoxMidiOut->addLayout(hBoxMidiOutType);
    vBoxMidiOut->addLayout(hBoxMidiOutChannel);
    vBoxMidiOut->addLayout(hBoxMidiOutNumber);
    vBoxMidiOut->addLayout(hBoxMidiOutVal);
    groupBoxMidiOut = new QGroupBox(tr("Midi Output"));
    groupBoxMidiOut->setCheckable(true);
    groupBoxMidiOut->setChecked(false);
    groupBoxMidiOut->setLayout(vBoxMidiOut);

    // Mise en forme Horizontale pour la config Midi
    QHBoxLayout *hBoxMidi = new QHBoxLayout();
    hBoxMidi->addWidget(groupBoxMidiIn);
    hBoxMidi->addWidget(groupBoxMidiOut);

    // Annuler et OK
    QHBoxLayout *hBox = new QHBoxLayout();
    hBox->addWidget(buttonQuit);
    hBox->addWidget(buttonOK);

    // Mise en forme Verticale
    QVBoxLayout *vBoxMain = new QVBoxLayout();
    if (pad->slider != NULL) {
        vBoxMain->addLayout(hBoxName);
    }
    else if (pad->button != NULL) {
        vBoxMain->addLayout(hBoxIcon);
        vBoxMain->addWidget(groupBoxCmd);
        vBoxMain->addWidget(groupBoxM3U);
        vBoxMain->addWidget(groupBoxSample);
    }
    vBoxMain->addLayout(hBoxMidi);
    vBoxMain->addLayout(hBox);

    setLayout(vBoxMain);

    updateWinFromPad();

    // Connect le signal du midi pour mise a jour de la valeur
    connect(midi, SIGNAL(onNewMessageReceive(tShortMessage*)), this, SLOT(midiMsgReceived(tShortMessage*)));
}

configPadWindow::~configPadWindow()
{
    disconnect(midi, SIGNAL(onNewMessageReceive(tShortMessage*)), this, SLOT(midiMsgReceived(tShortMessage*)));
}

void configPadWindow::updateWinFromPad()
{
    if (pad->slider != NULL) {
        groupBoxCmd->setEnabled(false);
        groupBoxM3U->setEnabled(false);
        groupBoxSample->setEnabled(false);
    }

    if (!pad->urlIcon.isEmpty()) {
        linePathIcon->setText(pad->urlIcon);
    }

    switch (pad->mode) {
    case tPad::MODE_COMMAND:
        groupBoxCmd->setChecked(true);
        comboBoxCmd->setCurrentIndex(pad->command-1);
        break;

    case tPad::MODE_SAMPLE:
        groupBoxSample->setChecked(true);
        linePathSample->setText(pad->urlSample);
        checkBoxSampleInfinite->setChecked(pad->isInfinited);
        checkBoxSampleMute->setChecked(pad->sampleIsMuted);
        break;

    case tPad::MODE_PLAYLIST:
        groupBoxM3U->setChecked(true);
        comboBoxM3U->setCurrentText(pad->playlist);
        break;

    default:
        break;
    }

    if (pad->label != NULL) {
        lineName->setText(pad->label->text());
    }

    // Si nous avons une configuration du midi In, nous l'appliquons
    if (!pad->deviceMidiIn.isEmpty()) {
        groupBoxMidiIn->setChecked(true);
        comboBoxMidiIn->setCurrentText(pad->deviceMidiIn);
        labelMidiInType->setText(QString("Type: %1").arg(tShortMessage::getStringCommand(pad->msgMidiIn.command)));
        labelMidiInChannel->setText(QString("Channel: %1").arg(pad->msgMidiIn.channel));
        labelMidiInNumber->setText(QString("Number: %1").arg(pad->msgMidiIn.number));
        labelMidiInValue->setText(QString("Value: %1").arg(pad->msgMidiIn.value));
        msg = pad->msgMidiIn;
    }

    // Si nous avons une configuration du midi Out, nous l'appliquons
    if (!pad->deviceMidiOut.isEmpty()) {
        groupBoxMidiOut->setChecked(true);
        comboBoxMidiOut->setCurrentText(pad->deviceMidiOut);
        comboBoxMidiOutType->setCurrentText(tShortMessage::getStringCommand(pad->msgMidiOut.command));
        spinBoxMidiOutChannel->setValue(pad->msgMidiOut.channel);
        spinBoxMidiOutNumber->setValue(pad->msgMidiOut.number);
        spinBoxMidiOutValMin->setValue(pad->msgMidiOut.valueMin);
        spinBoxMidiOutValMax->setValue(pad->msgMidiOut.valueMax);
    }
}

void configPadWindow::buttonIconClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Ouvrir une icone"), "", tr("Image Files (*.bmp *.gif *.jpg *.png)"));

    if (fileName.isEmpty()) return;

    linePathIcon->setText(fileName);
}

void configPadWindow::buttonSampleClicked()
{
    QUrl fileName = QFileDialog::getOpenFileUrl(this, tr("Ouvrir une musique"), QDir::homePath(), tr("Musique Files (*.mp3 *.wma *.m4a *.wav *.mp4 *.avi *.mov *.mkv)"));

    if (fileName.isEmpty()) return;

    linePathSample->setText(fileName.toString());
}

void configPadWindow::buttonOKClicked()
{
    // update value of pad
    pad->urlIcon = linePathIcon->text();

    pad->mode = tPad::MODE_DISABLE;
    pad->playlist.clear();
    pad->urlSample.clear();
    pad->command = tPad::CMD_UNKNOWN;
    pad->isInfinited = false;

    if (groupBoxM3U->isChecked()) {
        pad->mode = tPad::MODE_PLAYLIST;
        pad->playlist = comboBoxM3U->currentText();
    }
    else if (groupBoxSample->isChecked()) {
        pad->mode = tPad::MODE_SAMPLE;
        pad->urlSample = linePathSample->text();
        pad->isInfinited = checkBoxSampleInfinite->isChecked();
        pad->sampleIsMuted = checkBoxSampleMute->isChecked();
    }
    else if (groupBoxCmd->isChecked()) {
        pad->mode = tPad::MODE_COMMAND;
        pad->command = (tPad::tCommand)(comboBoxCmd->currentIndex()+1);
    }
    else if (groupBoxMidiIn->isChecked() && groupBoxMidiOut->isChecked()) {
        pad->mode = tPad::MODE_TRANSLATION;
    }

    if (pad->label != NULL) {
        pad->label->setText(lineName->text());
    }

    // update midi message and device
    pad->deviceMidiIn.clear();
    pad->msgMidiIn.clear();
    if (groupBoxMidiIn->isChecked()) {
        pad->deviceMidiIn = comboBoxMidiIn->currentText();
        pad->msgMidiIn = msg;
    }

    pad->deviceMidiOut.clear();
    pad->msgMidiOut.clear();
    if (groupBoxMidiOut->isChecked()) {
        pad->deviceMidiOut = comboBoxMidiOut->currentText();
        pad->msgMidiOut.channel = spinBoxMidiOutChannel->value();
        pad->msgMidiOut.command = tShortMessage::getCommandFromString(comboBoxMidiOutType->currentText());
        pad->msgMidiOut.number = spinBoxMidiOutNumber->value();
        pad->msgMidiOut.valueMin = spinBoxMidiOutValMin->value();
        pad->msgMidiOut.valueMax = spinBoxMidiOutValMax->value();
    }

    // update pad
    pad->update();

    // close this windows
    accept();
}

void configPadWindow::midiMsgReceived(tShortMessage *msg_)
{
    // Suppress NOTE_OFF Command
    if (msg_->command == tShortMessage::Command::NOTE_OFF) return;

    if (!groupBoxMidiIn->isChecked()) {
        groupBoxMidiIn->setChecked(true);
    }

    msg = *msg_;
    labelMidiInType->setText(QString("Type: %1").arg(tShortMessage::getStringCommand(msg_->command)));
    labelMidiInChannel->setText(QString("Channel: %1").arg(msg_->channel));
    labelMidiInNumber->setText(QString("Number: %1").arg(msg_->number));
    labelMidiInValue->setText(QString("Value: %1").arg(msg_->value));

//    if (groupBoxMidiOut->isChecked()) {
//        comboBoxMidiOutType->setCurrentText(tShortMessage::getStringCommand(msg_->command));
//        spinBoxMidiOutChannel->setValue(msg_->channel);
//        spinBoxMidiOutNumber->setValue(msg_->number);
//        spinBoxMidiOutValMax->setValue(msg_->value);
//    }
}

void configPadWindow::checkBoxCmdClicked(bool clicked_)
{
    if (clicked_) {
        // dechoche les autres groupbox
        if (groupBoxM3U->isChecked()) {
            groupBoxM3U->setChecked(false);
        }
        if (groupBoxSample->isChecked()) {
            groupBoxSample->setChecked(false);
        }
    }
}

void configPadWindow::checkBoxM3UClicked(bool clicked_)
{
    if (clicked_) {
        // dechoche les autres groupbox
        if (groupBoxCmd->isChecked()) {
            groupBoxCmd->setChecked(false);
        }
        if (groupBoxSample->isChecked()) {
            groupBoxSample->setChecked(false);
        }
    }
}

void configPadWindow::checkBoxSampleClicked(bool clicked_)
{
    if (clicked_) {
        // dechoche les autres groupbox
        if (groupBoxCmd->isChecked()) {
            groupBoxCmd->setChecked(false);
        }
        if (groupBoxM3U->isChecked()) {
            groupBoxM3U->setChecked(false);
        }
    }
}
