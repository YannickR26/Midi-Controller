#include "tmidi.h"

#include <QDebug>
#include <QMessageBox>

#define MIDI_APC     "APC MINI"
#define MIDI_XR12    "USB MIDI "

void callback(double deltatime, std::vector<unsigned char> *message, void *userData);

const QList<tCommand> tShortMessage::kListCommand = {
    tCommand(NOTE_ON, "NOTE_ON"),
    tCommand(NOTE_OFF, "NOTE_OFF"),
    tCommand(POLY_PRESSURE, "POLY_PRESSURE"),
    tCommand(CONTROL_CHANGE, "CONTROL_CHANGE"),
    tCommand(PROGRAM_CHANGE, "PROGRAM_CHANGE"),
    tCommand(CHANNEL_PRESSURE, "CHANNEL_PRESSURE"),
    tCommand(PITCH_BEND, "PITCH_BEND")
};

tMidi::tMidi(QObject *parent)
    : QObject(parent)
{
}

QStringList tMidi::getDeviceIn()
{
    RtMidiIn *midiIn = new RtMidiIn();
    QStringList strList;
    int nbPort = midiIn->getPortCount();
    for (int i=0 ; i<nbPort ; ++i) {
       strList.append(QString::fromStdString(midiIn->getPortName(i)).remove(QRegExp("[0-9]")));
    }

    return strList;
}

QStringList tMidi::getDeviceOut()
{
    RtMidiOut *midiOut = new RtMidiOut();
    QStringList strList;
    int nbPort = midiOut->getPortCount();
    for (int i=0 ; i<nbPort ; ++i) {
       strList.append(QString::fromStdString(midiOut->getPortName(i)).remove(QRegExp("[0-9]")));
    }

    return strList;
}

int tMidi::getIdxFromDeviceIn(QString device_)
{
    QStringList devices = this->getDeviceIn();
    for (int i=0 ; i<devices.size() ; ++i) {
        if (devices.at(i) == device_) {
            return i;
        }
    }
    return -1;
}

int tMidi::getIdxFromDeviceOut(QString device_)
{
    QStringList devices = this->getDeviceOut();
    for (int i=0 ; i<devices.size() ; ++i) {
        if (devices.at(i) == device_) {
            return i;
        }
    }
    return -1;
}

int tMidi::openDeviceIn(int idxDevice_)
{
    RtMidiIn *midiIn = new RtMidiIn();
    if (idxDevice_ < 0) return 1;
    // Si le port existe deja, nous ne l'ajoutons pas
    if (listStringMidiIn.contains(QString::fromStdString(midiIn->getPortName(idxDevice_)).remove(QRegExp("[0-9]")))) return 2;
    if (midiIn->getPortCount() == 0) return 3;
    if (midiIn->isPortOpen()) return 4;
    midiIn->openPort(idxDevice_);
    midiIn->setCallback(&callback, this);
    listMidiIn.append(midiIn);
    listStringMidiIn.append(QString::fromStdString(midiIn->getPortName(idxDevice_)).remove(QRegExp("[0-9]")));
    return 0;
}

int tMidi::openDeviceOut(int idxDevice_)
{
    RtMidiOut *midiOut = new RtMidiOut();
    if (idxDevice_ < 0) return 1;
    // Si le port existe deja, nous ne l'ajoutons pas
    if (listStringMidiOut.contains(QString::fromStdString(midiOut->getPortName(idxDevice_)).remove(QRegExp("[0-9]")))) return 2;
    if (midiOut->getPortCount() == 0) return 3;
    if (midiOut->isPortOpen()) return 4;
    midiOut->openPort(idxDevice_);
    listMidiOut.append(midiOut);
    listStringMidiOut.append(QString::fromStdString(midiOut->getPortName(idxDevice_)).remove(QRegExp("[0-9]")));
    return 0;
}

bool tMidi::isDeviceInIsOpen(int idxList_)
{
    if (listMidiIn.length() <= idxList_) return false;
    return listMidiIn.at(idxList_)->isPortOpen();
}

bool tMidi::isDeviceOutIsOpen(int idxList_)
{
    if (listMidiOut.length() <= idxList_) return false;
    return listMidiOut.at(idxList_)->isPortOpen();
}

void tMidi::closeDeviceIn(int idxList_)
{
    if (this->isDeviceInIsOpen(idxList_)) {
        listMidiIn.at(idxList_)->closePort();
        listMidiIn.at(idxList_)->cancelCallback();
        listMidiIn.removeAt(idxList_);
        listStringMidiIn.removeAt(idxList_);
    }
}

void tMidi::closeDeviceIn(QString device_)
{
    // Si nous avons trouver le device dans la list de device ouvert, nous le fermons et le supprimons
    for (int i=0 ; i<listStringMidiIn.size() ; ++i) {
        if (listStringMidiIn.at(i) == device_) {
            this->closeDeviceIn(i);
        }
    }
}

void tMidi::closeDeviceOut(int idxList_)
{
    if (this->isDeviceOutIsOpen(idxList_)) {
        listMidiOut.at(idxList_)->closePort();
        listMidiOut.removeAt(idxList_);
        listStringMidiOut.removeAt(idxList_);
    }
}

void tMidi::closeDeviceOut(QString device_)
{
    // Si nous avons trouver le device dans la list de device ouvert, nous le fermons et le supprimons
    for (int i=0 ; i<listStringMidiOut.size() ; ++i) {
        if (listStringMidiOut.at(i) == device_) {
            this->closeDeviceOut(i);
        }
    }
}

void tMidi::closeAllDevice()
{
    for (int i=0 ; i<listMidiIn.length() ; ++i) {
        this->closeDeviceIn(i);
    }

    for (int i=0 ; i<listMidiOut.length() ; ++i) {
        this->closeDeviceOut(i);
    }
}

int tMidi::sendMessage(int idxList_, tShortMessage &msg_)
{
    if (!this->isDeviceOutIsOpen(idxList_)) return 1;

    // check for valid values
    if (msg_.command >= 0xF0 || msg_.command < 0x80) {
        return 1;
    }
    if ((msg_.channel & 0xFFFFFFF0) != 0) { // <=> (channel<0 || channel>15)
        return 1;
    }
    if (msg_.number > 127) {
        return 1;
    }
    if (msg_.value > 127) {
        return 1;
    }

    std::vector<unsigned char> message;
    message.push_back((msg_.command & 0xF0) | (msg_.channel & 0x0F));
    message.push_back(msg_.number);
    message.push_back(msg_.value);

    qDebug() << "sendMessage => cmd: " << msg_.command << ", channel: " << msg_.channel << ", number: " << msg_.number << ", value: " << msg_.value;
    listMidiOut.at(idxList_)->sendMessage(&message);

    return 0;
}

int tMidi::sendMessage(QString device_, tShortMessage &msg_)
{
    for (int i=0 ; i<listStringMidiOut.size() ; ++i) {
        if (listStringMidiOut.at(i).contains(device_)) {
            return this->sendMessage(i, msg_);
        }
    }
    return 1;
}

int tMidi::getMessage(int idxList_, tShortMessage *msg_)
{
    if (!this->isDeviceInIsOpen(idxList_)) return 1;

    // Don't ignore sysex, timing, or active sensing messages.
    listMidiIn.at(idxList_)->ignoreTypes(false, false, false);

    std::vector<unsigned char> message;
    double timeStamp = listMidiIn.at(idxList_)->getMessage(&message);

    if (timeStamp == 0) return 1;
    if (message.size() < 3) return 1;

    msg_->command = message.at(0) & 0xF0;
    msg_->channel = message.at(0) & 0x0F;
    msg_->number = message.at(1) & 0xF0;
    msg_->value = message.at(2) & 0xF0;

    return 0;
}

void callback(double deltatime, std::vector<unsigned char> *message, void *userData)
{
    // Read Nb Byte receive
    unsigned int nBytes = message->size();
    if (nBytes < 3) return;

    tShortMessage *msg = new tShortMessage(message->at(0) & 0xF0, message->at(0) & 0x0F, message->at(1), message->at(2));
    msg->deltaTime = deltatime;

    //qDebug() << "Reception donnée Midi bas niveau, " << msg->command << ", " << msg->channel << ", " << msg->number << ", " << msg->value;

    // emit signal
    tMidi *midi = reinterpret_cast<tMidi *>(userData);
    emit midi->onNewMessageReceive(msg);
}

// Convert Data to Xml
void tMidi::toXml(QDomDocument &parent, QDomElement &child) {
    for (int i=0 ; i<this->listStringMidiIn.size() ; ++i) {
        QDomElement midiInChild = parent.createElement("Device_In");
        midiInChild.setAttribute("name", this->listStringMidiIn.at(i));
        child.appendChild(midiInChild);
    }
    for (int i=0 ; i<this->listStringMidiOut.size() ; ++i) {
        QDomElement midiOutChild = parent.createElement("Device_Out");
        midiOutChild.setAttribute("name", this->listStringMidiOut.at(i));
        child.appendChild(midiOutChild);
    }
}

// Convert Xml to Data
void tMidi::fromXml(QDomElement &node) {
    while (!node.isNull()) {
        if (node.tagName() == "Device_In") {
            if (this->openDeviceIn(this->getIdxFromDeviceIn(node.attribute("name"))) != 0) {
                QMessageBox::warning(NULL, tr("Erreur"), (QString(tr("Le device midi \"%1\" n'a pas pus s'ouvrir correctement !")).arg(node.attribute("name"))));
            }
        }
        else if (node.tagName() == "Device_Out") {
            if (this->openDeviceOut(this->getIdxFromDeviceOut(node.attribute("name"))) != 0) {
                QMessageBox::warning(NULL, tr("Erreur"), (QString(tr("Le device midi \"%1\" n'a pas pus s'ouvrir correctement !")).arg(node.attribute("name"))));
            }
        }
        node = node.nextSiblingElement();
    }
}

//*****************************************/
// Commande particuliere
//*****************************************/
int tMidi::setColorToAPCMini(const tShortMessage &msg_, tShortMessage::ColorAPCMini color_)
{
    tShortMessage msg = msg_;
    msg.command = tShortMessage::Command::NOTE_ON;
    msg.value = color_;
    return this->sendMessage(MIDI_APC, msg);
}
