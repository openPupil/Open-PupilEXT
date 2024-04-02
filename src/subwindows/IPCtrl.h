#pragma once

/**
    @authors Gábor Bényei
*/

#include <QFrame>
#include <QLineEdit>
#include <QIntValidator>
#include "stdint.h"
#include <QHBoxLayout>
#include <QFont>
#include <QLabel>
#include <QKeyEvent>


class IPCtrl : public QFrame
{
    Q_OBJECT

/**
   
   This a quite nice custom control/widget that can be used for IP address input.
   GB NOTE: It is mostly not my work. The code from a Stackoverflow answer, by user Tugo: 
   https://stackoverflow.com/a/11358560/11414500

*/

public:
    IPCtrl(QWidget *parent = 0);
    ~IPCtrl();

    // BG added
    void setValue(QString ip);
    QString getValue();
    // BG added end

    virtual bool eventFilter( QObject *obj, QEvent *event );

public slots:
    void slotTextChanged( QLineEdit* pEdit );

signals:
    void signalTextChanged( QLineEdit* pEdit );

private:
    enum
    {
        QTUTL_IP_SIZE   = 4,// число октетов IP адресе
        MAX_DIGITS      = 3 // число символов в LineEdit
    };

    QLineEdit *(m_pLineEdit[QTUTL_IP_SIZE]);
    void MoveNextLineEdit(int i);
    void MovePrevLineEdit(int i);
};

