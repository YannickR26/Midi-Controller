#include "configappwindow.h"
#include "common.h"

#include <QPushButton>
#include <QMessageBox>
#include <QGroupBox>
#include <QSettings>
#include <QLayout>
#include <QLabel>

configAppWindow::configAppWindow(QDialog *parent)
    : QDialog(parent)
{
    this->setWindowTitle(tr("Préférences"));

    // Creation des pushButton
    QPushButton *buttonOK = new QPushButton(tr("OK"));
    QPushButton *buttonQuit = new QPushButton(tr("Annuler"));

    // Connection des slot
    connect(buttonOK, SIGNAL(clicked()), this, SLOT(buttonOKClicked()));
    connect(buttonQuit, SIGNAL(clicked()), this, SLOT(reject()));

    // Creation des SpinBox
    spinBoxNbColBt = new QSpinBox();
    spinBoxNbColBt->setMaximum(8);
    spinBoxNbRowBt = new QSpinBox();
    spinBoxNbRowBt->setMaximum(8);
    spinBoxNbSlider = new QSpinBox();
    spinBoxNbSlider->setMaximum(8);

    // Mise en layout
    QHBoxLayout *hBoxCol = new QHBoxLayout();
    hBoxCol->addWidget(new QLabel(tr("Nombre de colonne :")));
    hBoxCol->addWidget(spinBoxNbColBt);
    QHBoxLayout *hBoxRow = new QHBoxLayout();
    hBoxRow->addWidget(new QLabel(tr("Nombre de ligne :")));
    hBoxRow->addWidget(spinBoxNbRowBt);
    QHBoxLayout *hBoxSlider = new QHBoxLayout();
    hBoxSlider->addWidget(new QLabel(tr("Nombre de slider :")));
    hBoxSlider->addWidget(spinBoxNbSlider);
    QVBoxLayout *vBoxBt = new QVBoxLayout();
    vBoxBt->addLayout(hBoxCol);
    vBoxBt->addLayout(hBoxRow);
    vBoxBt->addLayout(hBoxSlider);

    // Mise en groupBox
    QGroupBox *groupBoxBt = new QGroupBox(tr("Boutons et Sliders"));
    groupBoxBt->setLayout(vBoxBt);

    // Creation du spinbox pour la durée d'avance et recule rapide
    spinBoxTime = new QSpinBox();
    spinBoxTime->setSuffix(" s");

    // Mise en layout
    QHBoxLayout *hBoxTime = new QHBoxLayout();
    hBoxTime->addWidget(new QLabel(tr("Durée d'avance/recule rapide")));
    hBoxTime->addWidget(spinBoxTime);

    // Creation des spinbox pour les volumes
    spinBoxVolumeMain = new QSpinBox();
    spinBoxVolumeMain->setSuffix(" %");
    spinBoxVolumeMain->setMaximum(100);
    spinBoxVolumeMain->setSingleStep(1);
    spinBoxVolumeSample = new QSpinBox();
    spinBoxVolumeSample->setSuffix(" %");
    spinBoxVolumeSample->setMaximum(100);
    spinBoxVolumeSample->setSingleStep(1);

    // Mise en layout
    QHBoxLayout *hBoxVolumeMain = new QHBoxLayout();
    hBoxVolumeMain->addWidget(new QLabel(tr("Volume principale")));
    hBoxVolumeMain->addWidget(spinBoxVolumeMain);
    QHBoxLayout *hBoxVolumeSample = new QHBoxLayout();
    hBoxVolumeSample->addWidget(new QLabel(tr("Volume sample")));
    hBoxVolumeSample->addWidget(spinBoxVolumeSample);

    // Creation du checkbox
    checkBoxAutoOpen = new QCheckBox(tr("Ouvrir automatiquement le dernier fichier"));

    // Bouton Annuler et OK
    QHBoxLayout *hBox = new QHBoxLayout();
    hBox->addWidget(buttonQuit);
    hBox->addWidget(buttonOK);

    // Vertical Layout principale
    QVBoxLayout *vBoxMain = new QVBoxLayout();
    vBoxMain->addWidget(groupBoxBt);
    vBoxMain->addLayout(hBoxTime);
    vBoxMain->addLayout(hBoxVolumeMain);
    vBoxMain->addLayout(hBoxVolumeSample);
    vBoxMain->addWidget(checkBoxAutoOpen);
    vBoxMain->addStretch(100);
    vBoxMain->addLayout(hBox);

    setLayout(vBoxMain);

    // Read data from setting
    updateFromSettings();
}

void configAppWindow::updateFromSettings()
{
  this->spinBoxNbColBt->setValue(mySettings->value("NbColBt", 4).toInt());
  this->spinBoxNbRowBt->setValue(mySettings->value("NbRowBt", 5).toInt());
  this->spinBoxNbSlider->setValue(mySettings->value("NbSlider", 6).toInt());
  this->spinBoxTime->setValue(mySettings->value("TimeFast", 15).toInt());
  this->spinBoxVolumeMain->setValue(mySettings->value("VolumeMain", 60).toInt());
  this->spinBoxVolumeSample->setValue(mySettings->value("VolumeSample", 90).toInt());
  this->checkBoxAutoOpen->setChecked(mySettings->value("AutoOpen", 0).toBool());
}


void configAppWindow::buttonOKClicked()
{
    // Sauvegarde la configuration
    mySettings->setValue("NbColBt", this->spinBoxNbColBt->value());
    mySettings->setValue("NbRowBt", this->spinBoxNbRowBt->value());
    mySettings->setValue("NbSlider", this->spinBoxNbSlider->value());
    mySettings->setValue("TimeFast", this->spinBoxTime->value());
    mySettings->setValue("VolumeMain", this->spinBoxVolumeMain->value());
    mySettings->setValue("VolumeSample", this->spinBoxVolumeSample->value());
    mySettings->setValue("AutoOpen", this->checkBoxAutoOpen->isChecked());

    /** Avertissement de redemarrage nécésaire */
    QMessageBox::information(this, tr("Information"), QString(tr("Vous devez redémarrer l'application pour prendre en compte les modifications !")));

    // close this windows
    accept();
}
