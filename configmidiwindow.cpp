#include "configmidiwindow.h"

#include <QListWidget>
#include <QPushButton>
#include <QGroupBox>
#include <QLayout>
#include <QLabel>

configMidiWindow::configMidiWindow(tMidi *midi_, QDialog *parent)
    : QDialog(parent)
{
    this->setWindowTitle(tr("Interfaces MIDI"));

    this->midi = midi_;

    // Creation des pushButton
    QPushButton *buttonOK = new QPushButton(tr("OK"));
    QPushButton *buttonQuit = new QPushButton(tr("Annuler"));

    // Connection des slot
    connect(buttonOK, SIGNAL(clicked()), this, SLOT(buttonOKClicked()));
    connect(buttonQuit, SIGNAL(clicked()), this, SLOT(reject()));

    // Creation des ListWidget
    listMidiIn = new QListWidget();
    listMidiIn->addItems(midi->getDeviceIn());
    QStringList midiInOpen = midi->getDeviceInOpen();
    // Permet de cocher les items
    for (int i=0 ; i<listMidiIn->count() ; ++i) {
        QListWidgetItem* item = listMidiIn->item(i);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
        item->setCheckState(Qt::Unchecked); // AND initialize check state
        for (int i=0 ; i<midiInOpen.size() ; ++i) {
            if (midiInOpen.contains(item->text())) {
                item->setCheckState(Qt::Checked);
            }
        }
    }
    listMidiOut = new QListWidget();
    listMidiOut->addItems(midi->getDeviceOut());
    QStringList midiOutOpen = midi->getDeviceOutOpen();
    // Permet de cocher les items
    for (int i=0 ; i<listMidiOut->count() ; ++i) {
        QListWidgetItem* item = listMidiOut->item(i);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
        item->setCheckState(Qt::Unchecked); // AND initialize check state
        for (int i=0 ; i<midiOutOpen.size() ; ++i) {
            if (midiOutOpen.contains(item->text())) {
                item->setCheckState(Qt::Checked);
            }
        }
    }

    // Bouton Annuler et OK
    QHBoxLayout *hBox = new QHBoxLayout();
    hBox->addWidget(buttonQuit);
    hBox->addWidget(buttonOK);

    // Vertical Layout principale
    QVBoxLayout *vBoxMidi = new QVBoxLayout();
    vBoxMidi->addWidget(new QLabel(tr("Entrée :")));
    vBoxMidi->addWidget(listMidiIn, 100);
    vBoxMidi->addWidget(new QLabel(tr("Sortie :")));
    vBoxMidi->addWidget(listMidiOut, 100);
    vBoxMidi->addLayout(hBox);

    setLayout(vBoxMidi);
}

void configMidiWindow::buttonOKClicked()
{
    // Verifie les device qui sont cocher, et les ouvrent
    for (int i=0 ; i<listMidiIn->count() ; ++i) {
        QListWidgetItem* item = listMidiIn->item(i);
        if (item->checkState() == Qt::Checked) {
            midi->openDeviceIn(midi->getIdxFromDeviceIn(item->text()));
        }
        else {
            midi->closeDeviceIn(item->text());
        }
    }
    for (int i=0 ; i<listMidiOut->count() ; ++i) {
        QListWidgetItem* item = listMidiOut->item(i);
        if (item->checkState() == Qt::Checked) {
            midi->openDeviceOut(midi->getIdxFromDeviceOut(item->text()));
        }
        else {
            midi->closeDeviceOut(item->text());
        }
    }

    accept();
}
