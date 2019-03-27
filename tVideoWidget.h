#ifndef TVIDEOWIDGET_H
#define TVIDEOWIDGET_H

#include <QVideoWidget>

class tVideoWidget : public QVideoWidget
{
    Q_OBJECT

public:
    tVideoWidget(QWidget *parent = 0);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
};

#endif // TVIDEOWIDGET_H
