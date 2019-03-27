#include "dynamicfontsizepushbutton.h"

#define FONT_PRECISION (0.5)

#include <QDebug>

DynamicFontSizePushButton::DynamicFontSizePushButton(QWidget* parent)
    :QPushButton(parent)
{

}

void DynamicFontSizePushButton::paintEvent(QPaintEvent *event)
{
    QFont newFont = font();
    float fontSize = getWidgetMaximumFontSize(this, this->text());
    newFont.setPointSizeF(fontSize);
    setFont(newFont);

    QPushButton::paintEvent(event);
}

float DynamicFontSizePushButton::getWidgetMaximumFontSize(QWidget *widget, QString text)
{
    QFont font = widget->font();
    const QRect widgetRect = widget->contentsRect();
    const float widgetWidth = widgetRect.width() * 0.9;
    const float widgetHeight = widgetRect.height() * 0.9;

    QRectF newFontSizeRect;
    float currentSize = font.pointSizeF();

    float step = currentSize/2.0;

    /* If too small, increase step */
    if (step<=FONT_PRECISION){
        step = FONT_PRECISION*4.0;
    }

    float lastTestedSize = currentSize;

    float currentHeight = 0;
    float currentWidth = 0;
    if (text==""){
        return currentSize;
    }

    /* Only stop when step is small enough and new size is smaller than QWidget */
    while(step>FONT_PRECISION || (currentHeight > widgetHeight) || (currentWidth > widgetWidth)){
        /* Keep last tested value */
        lastTestedSize = currentSize;

        /* Test pushButton with its font */
        font.setPointSizeF(currentSize);
        /* Use font metrics to test */
        QFontMetricsF fm(font);

        /* Check if widget is QpushButton */
        QPushButton *pushButton = qobject_cast<QPushButton*>(widget);
        if (pushButton) {
            newFontSizeRect = fm.boundingRect(widgetRect, 0, text);
        }
        else {
            return currentSize;
        }

        currentHeight = newFontSizeRect.height();
        currentWidth = newFontSizeRect.width();

        /* If new font size is too big, decrease it */
        if ((currentHeight > widgetHeight) || (currentWidth > widgetWidth)){
            currentSize -=step;
            /* if step is small enough, keep it constant, so it converge to biggest font size */
            if (step>FONT_PRECISION){
                step/=2.0;
            }
            /* Do not allow negative size */
            if (currentSize<=0){
                break;
            }
        }
        /* If new font size is smaller than maximum possible size, increase it */
        else{
            currentSize +=step;
        }
    }
    return lastTestedSize;
}

void DynamicFontSizePushButton::setTextColor(QColor color){
    if (color.isValid() && color!=textColor){
        textColor = color;
        setStyleSheet("color : "+color.name()+";");
    }
}

void DynamicFontSizePushButton::setTextAndColor(const QString &text, QColor color){
    setTextColor(color);
    setText(text);
}

QColor DynamicFontSizePushButton::getTextColor(){
    return textColor;
}

/* Do not give any size hint as it it changes during paintEvent */
QSize DynamicFontSizePushButton::minimumSizeHint() const
{
    return QWidget::minimumSizeHint();
}

/* Do not give any size hint as it it changes during paintEvent */
QSize DynamicFontSizePushButton::sizeHint() const
{
     return QWidget::sizeHint();
}
