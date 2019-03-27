#ifndef CONFIGMIDIWINDOW_H
#define CONFIGMIDIWINDOW_H

#include "tmidi.h"

#include <QListWidget>
#include <QCheckBox>
#include <QDialog>

class configMidiWindow : public QDialog
{
    Q_OBJECT

public:
    configMidiWindow(tMidi *midi_, QDialog *parent = nullptr);

public slots:
    void buttonOKClicked();

private:
    tMidi           *midi;
    QListWidget     *listMidiIn;
    QListWidget     *listMidiOut;
};

#endif // CONFIGMIDIWINDOW_H
