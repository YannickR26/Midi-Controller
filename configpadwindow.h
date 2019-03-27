#ifndef CONFIGBUTTONWINDOW_H
#define CONFIGBUTTONWINDOW_H

#include "tPad.h"
#include "tmidi.h"
#include "tplaylist.h"

#include <QLabel>
#include <QDialog>
#include <QSpinBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>

class configPadWindow : public QDialog
{
    Q_OBJECT

public:
    explicit configPadWindow(tPad *pad_, tMidi *midi_, QList<tPlaylist *> *playlist_, QDialog *parent = nullptr);
    ~configPadWindow();

    void updateWinFromPad();
signals:

public slots:
    void buttonIconClicked();
    void buttonSampleClicked();
    void buttonOKClicked();
    void midiMsgReceived(tShortMessage *msg_);
    void checkBoxCmdClicked(bool clicked_);
    void checkBoxM3UClicked(bool clicked_);
    void checkBoxSampleClicked(bool clicked_);

private:
    tPad                    *pad;
    tMidi                   *midi;
    QList<tPlaylist *>      *playlist;
    QLineEdit               *lineName;
    QComboBox               *comboBoxCmd;
    QComboBox               *comboBoxM3U;
    QLineEdit               *linePathSample;
    QCheckBox               *checkBoxSampleInfinite;
    QCheckBox               *checkBoxSampleMute;
    QLineEdit               *linePathIcon;
    QGroupBox               *groupBoxCmd;
    QGroupBox               *groupBoxM3U;
    QGroupBox               *groupBoxSample;
    QGroupBox               *groupBoxMidiIn;
    QGroupBox               *groupBoxMidiOut;
    QComboBox               *comboBoxMidiIn;
    QComboBox               *comboBoxMidiOut;
    QComboBox               *comboBoxMidiOutType;
    QSpinBox                *spinBoxMidiOutChannel;
    QSpinBox                *spinBoxMidiOutNumber;
    QSpinBox                *spinBoxMidiOutValMin, *spinBoxMidiOutValMax;
    QLabel                  *labelMidiInType;
    QLabel                  *labelMidiInChannel;
    QLabel                  *labelMidiInNumber;
    QLabel                  *labelMidiInValue;
    tShortMessage           msg;
};

#endif // CONFIGBUTTONWINDOW_H
