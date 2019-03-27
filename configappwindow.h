#ifndef CONFIGAPPWINDOW_H
#define CONFIGAPPWINDOW_H

#include <QListWidget>
#include <QCheckBox>
#include <QSpinBox>
#include <QDialog>

class configAppWindow : public QDialog
{
    Q_OBJECT

public:
    explicit configAppWindow(QDialog *parent = nullptr);

signals:

public slots:
    void buttonOKClicked();

protected:
    void updateFromSettings();

private:
    QSpinBox        *spinBoxNbRowBt, *spinBoxNbColBt;
    QSpinBox        *spinBoxNbSlider;
    QSpinBox        *spinBoxTime;
    QSpinBox        *spinBoxVolumeMain;
    QSpinBox        *spinBoxVolumeSample;
    QCheckBox       *checkBoxAutoOpen;
};

#endif // CONFIGAPPWINDOW_H
