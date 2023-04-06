#pragma once

/**
    @author Gábor Bényei
*/

#include <QtWidgets/QWidget>
#include <QDebug>
#include <QtMath>
#include <QWheelEvent>
#include <QKeyEvent>

/**

    This class is a custom version of QDial class.
    Differences: 
    1,  The original QDial widget sets the value whenever it is clicked or moved, to match the value/angle the cursor points to. 
        This version only takes the relative movement into consideration 
        (can be grabbed wnywhere, and will only move with respect to direction of rotation).
    2,  This widget now emits two signals to encode relative movement: increment, decrement. 
        This is its purpose, so its value() method should not be used externally. Inc/dec does not have an "amount" associated.
    3,  Right or Up keys emit incremented(), Left or Down keys emit decremented()
    
*/
class PlaybackDial : public QDial {
    Q_OBJECT

public:

    // NOTE: by QDial default, value will range from 0 to 99, the endpoints representing the bottom (southmost) direction on screen

    PlaybackDial(QWidget *parent = nullptr) 
    {
        this->setWrapping(true);
        lastAngle = this->value();
    };

private:
    const int mapDim = 100; // map to a 100-scale now (if we were to map to degrees, here we could use 360 instead)
    int lastAngle = 0; // again, this is not in degrees, but 0-99 numbers (= possible QDial values)

protected:

    void keyPressEvent(QKeyEvent * e) override {
        if(e->key() == Qt::Key_Right || e->key() == Qt::Key_Up) 
            emit incremented();
        else if(e->key() == Qt::Key_Left || e->key() == Qt::Key_Down)
            emit decremented();
        else
            e->ignore();
    }

    void mousePressEvent(QMouseEvent *event) override {
        QPoint dialPos = this->geometry().center(); // RELATIVE TO PARENT
        QPoint clickPos = event->pos() + this->geometry().topLeft(); 
        QPoint diff = clickPos - dialPos;
        //qDebug() << "dialPos = " << dialPos << Qt::endl;
        //qDebug() << "clickPos = " << clickPos << Qt::endl;
        //qDebug() << "diff = " << diff << Qt::endl;
        int angle = qFloor(qAtan2(diff.y(), diff.x()) *(mapDim/2) /M_PI); 
        //qDebug() << "angle = " << angle << Qt::endl;
        //
        // at this point, value 0 is the rightmost state of the dial. We convert the value to match what QDial represents as "value"
        angle = angle - mapDim/4;
        if(angle < 0)
            angle = angle + mapDim;

        lastAngle = angle;
    };

    void mouseMoveEvent(QMouseEvent *event) override {
        QPoint dialPos = this->geometry().center(); // RELATIVE TO PARENT
        QPoint clickPos = event->pos() + this->geometry().topLeft(); 
        QPoint diff = clickPos - dialPos;
        //qDebug() << "dialPos = " << dialPos << Qt::endl;
        //qDebug() << "clickPos = " << clickPos << Qt::endl;
        //qDebug() << "diff = " << diff << Qt::endl;
        int angle = qFloor(qAtan2(diff.y(), diff.x()) *(mapDim/2) /M_PI); 
        //qDebug() << "angle = " << angle << Qt::endl;
        //
        // at this point, value 0 is the rightmost state of the dial. We convert the value to match what QDial represents as "value"
        angle = angle - mapDim/4;
        if(angle < 0)
            angle = angle + mapDim;
        
        if(lastAngle < angle || (lastAngle>=75 && angle<=25)) {
            //qDebug() << "MOUSE INC" << Qt::endl;
            this->setValue(this->value()+qFabs(lastAngle-angle));
            //this->setValue(this->value()+1);
            emit incremented();
        }
        if(lastAngle > angle || (lastAngle<=25 && angle>=75)) {
            //qDebug() << "MOUSE DEC" << Qt::endl;
            this->setValue(this->value()-qFabs(lastAngle-angle));
            //this->setValue(this->value()-1);
            emit decremented();
        }

        lastAngle = angle;
    };

    void mouseReleaseEvent(QMouseEvent *event) override {
        //std::cout << this->value() <<std::endl;
    };

    void wheelEvent(QWheelEvent *event) override {
        if(event->angleDelta().ry() > 0) {
            //qDebug() << "WHEEL INC" << Qt::endl;
            this->setValue(this->value()+1);
            emit incremented();
        } 
        if(event->angleDelta().ry() < 0) {
            //qDebug() << "WHEEL DEC" << Qt::endl;
            this->setValue(this->value()-1);
            emit decremented();
        }
    };

signals:
    void incremented();
    void decremented();

};