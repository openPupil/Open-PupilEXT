#pragma once

/**
    @author Gábor Bényei
*/

#include <QtWidgets/QWidget>
#include <QDebug>
#include <QSlider>


/**
    This class is a custom version of QSlider class.
    Differences: 
    1,  You can click wherever you want, and the value will be set exactly there, not just one pagestep before/after the current position.
    2,  Currently only supports Horizontal orientation.
    3,  This draws a color line to indicate the position of file reading

    NOTE: code partially inspired from https://stackoverflow.com/a/11133022/11414500 
*/
class PlaybackSlider : public QSlider
{
    // Empirically determined. Necessary
    const uint leftMargin = 3;
    const uint rightMargin = 3;

    // same
    const uint leftDrawMargin = 5; //3;
    const uint rightDrawMargin = 5; //3;

    float colorTickPos = 0.0f; // between 0 and 1

public:
    void setColorTickPos(float value) {
        colorTickPos = value;
//        this->update();
    }

    PlaybackSlider(QWidget * parent) : QSlider(parent){}

    PlaybackSlider() {};

protected:

    void paintEvent(QPaintEvent * event) override {
        QPainter p{this};
        
        p.setPen({0, 200, 0});
        float width = (rect().right()-rightDrawMargin)-(rect().left()+leftDrawMargin);
        int xPos = round(width*colorTickPos)+leftDrawMargin;
        p.drawLine(xPos, rect().top(), xPos, rect().bottom());

        QSlider::paintEvent(event);
    }

    void mousePressEvent ( QMouseEvent * event )
    {
        if(event->button() == Qt::LeftButton)
        {
            QPoint clickPos = event->pos(); 
            //qDebug() << clickPos.x();
            int desiredValue;

            //if (orientation() == Qt::Vertical) {
            //    desiredValue = minimum() + ((maximum()-minimum()) * (height()-clickPos.y())) / height();
            //    if(desiredValue != value())
            //        setValue(desiredValue) ;
            //} else {
                //desiredValue = minimum() + ((maximum()-minimum()) * (float)clickPos.x()) / (float)width();
                desiredValue = minimum() + ((maximum()-minimum()) * ((float)clickPos.x()-(leftMargin+rightMargin))) / ((float)width()-2*(leftMargin+rightMargin));
                
                if(desiredValue != value())
                    setValue(desiredValue);
            //}
            
            event->accept();
        }
        //QSlider::mousePressEvent(event); // buggy. like 3 out of 10 times, it hops one pagestep forward
    }

    void mouseMoveEvent ( QMouseEvent * event )
    {
        qDebug() << (event->buttons() & Qt::LeftButton);
        
        //if(event->button() == Qt::LeftButton) // doesn't work. See explanation here: https://stackoverflow.com/a/10780752/11414500
        if(event->buttons() & Qt::LeftButton)
        {
            QPoint clickPos = event->pos(); 
            //qDebug() << clickPos.x();
            int desiredValue;

            //if (orientation() == Qt::Vertical) {
            //    desiredValue = minimum() + ((maximum()-minimum()) * (height()-clickPos.y())) / height();
            //    if(desiredValue != value())
            //        setValue(desiredValue) ;
            //} else {
                //desiredValue = minimum() + ((maximum()-minimum()) * (float)clickPos.x()) / (float)width();
                desiredValue = minimum() + ((maximum()-minimum()) * ((float)clickPos.x()-(leftMargin+rightMargin))) / ((float)width()-2*(leftMargin+rightMargin));
                
                if(desiredValue != value())
                    setValue(desiredValue);
            //}
            
            event->accept();
        }
        
    }
};