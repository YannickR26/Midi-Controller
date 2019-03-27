#ifndef TMIDI_H
#define TMIDI_H

#include "RtMidi.h"

#include <QObject>
#include <QStringList>
#include <QDomDocument>
#include <QDomElement>

class tCommand
{
public:
    tCommand() {}
    tCommand(uint8_t cmd_, QString str_) {
        cmd = cmd_;
        cmdStr = str_;
    }

    uint8_t cmd;
    QString cmdStr;
};

class tShortMessage
{
public:
    typedef enum e_Command {
        UNKNOWN                                     = 0x00,
        NOTE_OFF                                    = 0x80,     // 128
        NOTE_ON                                     = 0x90,     // 144
        POLY_PRESSURE                               = 0xA0,     // 160
        CONTROL_CHANGE                              = 0xB0,     // 176
        PROGRAM_CHANGE                              = 0xC0,     // 192
        CHANNEL_PRESSURE                            = 0xD0,     // 208
        PITCH_BEND                                  = 0xE0      // 224
    } Command;

    typedef enum e_ColorAPCMini {
        BLACK           = 0,
        GREEN           = 1,
        GREEN_BLINK     = 2,
        RED             = 3,
        RED_BLINK       = 4,
        ORANGE          = 5,
        ORANGE_BLINK    = 6
    } ColorAPCMini;

    tShortMessage() { command=UNKNOWN; channel=0; number=0; value=0; valueMin=0, valueMax=0; }
    tShortMessage(uint8_t command_, uint8_t channel_, uint8_t data1_, uint8_t data2_) { command=command_; channel=channel_; number=data1_; value=data2_; valueMin=0; valueMax=0; }

    // Return Command in string mode
    static QString getStringCommand(uint8_t command_) {
        switch (command_) {
        case Command::NOTE_ON : return kListCommand.at(0).cmdStr; break;
        case Command::NOTE_OFF : return kListCommand.at(1).cmdStr; break;
        case Command::POLY_PRESSURE : return kListCommand.at(2).cmdStr; break;
        case Command::CONTROL_CHANGE : return kListCommand.at(3).cmdStr; break;
        case Command::PROGRAM_CHANGE : return kListCommand.at(4).cmdStr; break;
        case Command::CHANNEL_PRESSURE : return kListCommand.at(5).cmdStr; break;
        case Command::PITCH_BEND : return kListCommand.at(6).cmdStr; break;
        default: return QString("UNKNOWN_COMMAND"); break;
        }
    }

    // Return Command from String
    static uint8_t getCommandFromString(QString cmd_) {
        tCommand cmd;
        foreach (cmd, kListCommand) {
            if (cmd.cmdStr == cmd_) {
                return cmd.cmd;
            }
        }
        return 0;
    }

    bool isEqual(tShortMessage *msg) {
        if (command != msg->command) return false;
        if (channel != msg->channel) return false;
        if (number != msg->number) return false;
//        if (value != msg->value) return false;
        return true;
    }

    void clear() {
        command=UNKNOWN;
        channel=0;
        number=0;
        value=0;
        valueMin=0;
        valueMax=0;
        deltaTime=0;
    }

    // Convert Data to Xml
    void toXml(QDomElement &child) {
        child.setAttribute("command", this->command);
        child.setAttribute("channel", this->channel);
        child.setAttribute("number", this->number);
        child.setAttribute("value", this->value);
        child.setAttribute("valueMin", this->valueMin);
        child.setAttribute("valueMax", this->valueMax);
    }

    // Convert Xml to Data
    void fromXml(QDomElement &node) {
        this->command = node.attribute("command").toInt();
        this->channel = node.attribute("channel").toInt();
        this->number = node.attribute("number").toInt();
        this->value = node.attribute("value").toInt();
        this->valueMin = node.attribute("valueMin").toInt();
        this->valueMax = node.attribute("valueMax").toInt();
    }

    uint8_t command, channel, number, value;
    uint8_t valueMin, valueMax;
    uint32_t deltaTime;
    static const QList<tCommand> kListCommand;
};

class tMidi : public QObject
{
    Q_OBJECT

public:

    tMidi(QObject *parent = 0);

    unsigned int getNbDeviceIn() { return RtMidiIn().getPortCount(); }
    unsigned int getNbDeviceOut() { return RtMidiOut().getPortCount(); }
    QStringList getDeviceIn();
    QStringList getDeviceOut();
    QStringList getDeviceInOpen() { return listStringMidiIn; }
    QStringList getDeviceOutOpen() { return listStringMidiOut; }
    int getIdxFromDeviceIn(QString device_);
    int getIdxFromDeviceOut(QString device_);
    int openDeviceIn(int idxDevice_);
    int openDeviceOut(int idxDevice_);
    bool isDeviceInIsOpen(int idxList_);
    bool isDeviceOutIsOpen(int idxList_);
    void closeDeviceIn(int idxList_);
    void closeDeviceIn(QString device_);
    void closeDeviceOut(int idxList_);
    void closeDeviceOut(QString device_);
    void closeAllDevice();

    int sendMessage(int idxList_, tShortMessage &msg_);
    int sendMessage(QString device_, tShortMessage &msg_);
    int getMessage(int idxList_, tShortMessage *msg_);

    void toXml(QDomDocument &parent, QDomElement &child);
    void fromXml(QDomElement &node);

    int setColorToAPCMini(const tShortMessage &msg_, tShortMessage::ColorAPCMini color_);

signals:
    void onNewMessageReceive(tShortMessage *msg);

private:
    QList<RtMidiIn *>       listMidiIn;
    QStringList             listStringMidiIn;
    QList<RtMidiOut *>      listMidiOut;
    QStringList             listStringMidiOut;
};

#endif // TMIDI_H
