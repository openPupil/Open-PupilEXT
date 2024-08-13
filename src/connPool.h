#pragma once

/**
    @author Gabor Benyei
*/

#include <QtCore/QObject>
#include <QByteArray>
#include <QUdpSocket>

/**
    
    This enum contains flags that can be used to manage sockets/ports used for multiple purposes, by different classes.

*/
enum class ConnPoolPurposeFlag
{
    CAMERA_TRIGGER = 1 << 0, // 1
    STREAMING = 1 << 1, // 2
    REMOTE_CONTROL = 1 << 2 //, // 4
};

// TODO: make virtual class for connPool classes

// TODO: make connPool classes singleton, and first instantiated in mainWindow

// TODO: it should not be allowed to use a pool instance, whose purpose is already assigned for microcontroller connection, to be reassigned for anything else
