#pragma once

/**
    @author Gabor Benyei
*/

#include <QtCore/QObject>
#include <QByteArray>
#include "connPool.h"
#include <QUdpSocket>

/**
    This struct holds a bundle of settings that are necessary for opening a UDP port
*/
struct ConnPoolUDPInstanceSettings {
    QHostAddress ipAddress;
    quint16 portNumber;
};

/**
    This class is necessary for the UDP pool to work. Each of these instances can start a separate listener.
*/
class ConnPoolUDPInstance : public QObject {
    Q_OBJECT

    public:
        explicit ConnPoolUDPInstance(QUdpSocket *UDPsocket, ConnPoolPurposeFlag purposeFlag, QObject *parent) : QObject(parent), UDPSocket(UDPsocket), purposeFlags((uint8_t)purposeFlag) {};
        ~ConnPoolUDPInstance() override {};

        bool startListening() {
            bool isOpen = UDPSocket->isOpen();
            bool handleReadyReadWorking = connect(UDPSocket, SIGNAL(readyRead()), this, SLOT(handleReadyRead()), Qt::QueuedConnection);
            // NOTE: No errorOccured() or similar signal exists for the UDP socket instances, as it was the case for COM instances
            return isOpen && handleReadyReadWorking;
        };

    QUdpSocket *UDPSocket;
    uint8_t purposeFlags;
        
    private slots:
        void handleReadyRead() {

            QHostAddress sender;
            quint16 port;
            QByteArray datagram;
            while(UDPSocket->hasPendingDatagrams())
            {
                quint64 timestamp  = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                //qDebug() << "Got message at unix time: " << QString::number(timestamp);

                datagram.resize(UDPSocket->pendingDatagramSize());
                // NOTE: the (target/sender filter) IP address of the connection is also kept in UDPSocket->objectName()
                // but we dont read and convert it here, because it should be the same as ipAddress anyway. The objectName()
                // is only for the ConnPoolUDP to know what IP we are targeting/filtering, although writing to the port
                // is not (yet) handled by the ConnPoolUDPInstance
                UDPSocket->readDatagram(datagram.data(), datagram.size(), &sender, &port);
                emit messageReceived(QString::fromStdString(datagram.toStdString()), timestamp);
            }

            // NOTE: Somehow there is always one more "dummy" readDatagram call to make,
            // otherwise no new readyRead() will be emitted, and this function would never be called again.
            // Is this a Qt bug? Seen in Qt 5.15
            datagram.resize(UDPSocket->pendingDatagramSize());
            UDPSocket->readDatagram(datagram.data(), datagram.size(), &sender, &port);
        };

//        void handleError() {
//
//        }
    
    public slots:
        int getNumSubscribedListeners() {
            return this->receivers(SIGNAL(messageReceived(QString, quint64)));
        };
        
    signals:
        void messageReceived(QString msg, quint64 timestamp);
};

/**
    This class helps handling a UDP connection to another machine, in a way that different classes can use the same connection for a purpose they specify.
    
    Incoming data is read once, and any method that has subcribed beforehand to get the message, will receive it as a signal. 
    If a method is not interested in the connection anymore, it can unsubscribe from the messages listened, or even prompt the pool class to close the connection. 
    But a connection is only closed if the number of its subscribed "listener" methods is 0. 
    
    This class makes it very easy to manage when for example PupilEXT is running and listening to e.g. a UDP port, and gets a remote command
    from that port to start streaming (on that same port, or any other port). Also it can be used to manage listening to the same port (pool instance)
    from different classes, with no need to worry for failing to open an already opened port.

    For sending data, or accessing socket directly, the socket instance pointer can be gotten from outside this class, via getInstance().
    This should be used with caution, as the connection should not be closed directly, but only via closeConnection() method of the pool class.

    IMPORTANT:
        ! The PORTNUMBER field is used as a unique key for identifying a UDP port. (It could be the PORTNUMBER + IPADDRESS combo, but this is easier.)
        When this class is told to open a new connection (and assign an instance) to a UDP port,
            1, whose PORTNUMBER matches that of an already opened port /existing instance:
                - if the IPADDRESS (filter) setting is not matching with the desired one, it will:
                    - not open a new port and assign a pool UDP instance,
                    - and neither return with the identifier (vector index) of the matching instance.
                    => Connection will be seen as unsuccessful from the method that called setupAndOpenConnection().
                - if the IPADDRESS (filter) setting is also the same, it will follow a conservative strategy:
                    - not open a new port and assign a pool COM instance,
                    - but return with the identifier (vector index) of the matching instance.
                    => Connection will be seen as successful from the method that called setupAndOpenConnection().
            2, whose PORTNUMBER does NOT match with the desired one, then it will:
                    - open a new port and assign a pool UDP instance.

        One opened UDP port /pool UDP instance can be used for multiple purposes, marked by purposeFlags.
        Also - in this version - multiple ports can be used for the same purpose.
        Classes that use these pools must internally keep track of their pool instance IDs assigned, returned by 
        setupAndOpenConnection(), and reset it to e.g. -1 or other invalid value when the connection is closed.
*/
class ConnPoolUDP : public QObject {
    Q_OBJECT

public:
    explicit ConnPoolUDP(QObject *parent = 0) : QObject(parent) {};
    ~ConnPoolUDP() override {};

    std::vector<quint16> getOpenedNames() {
        std::vector<quint16> found;
        for(int c=0; c<instancePool.size(); c++)
            if(instancePool[c] && instancePool[c]->UDPSocket && instancePool[c]->UDPSocket->isOpen() )
                found.push_back(instancePool[c]->UDPSocket->localPort());
        return found;
    }

    int setupAndOpenConnection(ConnPoolUDPInstanceSettings p, ConnPoolPurposeFlag purposeFlag) {

        qDebug() << instancePool.size();

        for(int c=0; c<instancePool.size(); c++)
            if(instancePool[c] && instancePool[c]->UDPSocket && instancePool[c]->UDPSocket->isOpen() &&
               instancePool[c]->UDPSocket->localPort() == p.portNumber ) {
                // If there is already an open port here, return with its index

                if(instancePool[c]->UDPSocket->objectName() != p.ipAddress.toString() ) {
                    
                    qDebug() << "Attempted to open a new port with the same port name, but different (target/sender filter) IP settings. \nNo new port was opened, nor was an existing reassigned.";
                    return -1;
                }

                if(instancePool[c]->purposeFlags & (uint8_t)purposeFlag)
                    qDebug() << "This UDP pool instance is already in use for the same purpose.";
                else {
                    qDebug() << "Already used UDP pool instance assigned for another purpose too.";
                    instancePool[c]->purposeFlags = instancePool[c]->purposeFlags | (uint8_t)purposeFlag;
                }
                return c;
            }

        // NOTES: 
        //  1, Code below should be only executed if there is no instance with the p.name in the pool
        //  2, There should never be an instance in the pool vector that is closed, but not set to nullptr
        //  3, The index of any instance in the pool must be unique, even if ane element below it gets closed.
        //      so when removing an instance, we do not "erase()" that vector element, 
        //      but only free up the space it is pointing to, and set the vector element to nullptr

        QUdpSocket *newUDPSocket = new QUdpSocket();
        newUDPSocket->setObjectName(p.ipAddress.toString()); // This is where we keep the IP filter setting
        newUDPSocket->bind(p.portNumber);

        if(newUDPSocket->open(QIODevice::ReadWrite)) {
            for(int c=0; c<instancePool.size(); c++)
                if(instancePool[c] == nullptr) { //  || (instancePool[c] != nullptr && instancePool[c]->port == nullptr) ) {
                    instancePool[c] = new ConnPoolUDPInstance(newUDPSocket, purposeFlag, this);
//                    instancePool[c]->purposeFlags = (uint8_t)purposeFlag;
//                    instancePool[c]->UDPSocket = newUDPSocket;
                    qDebug() << "Previously abandoned UDP pool instance added to pool again.";
                    return c;
                }

            // Otherwise, we add the instance pointer to the pool vector
            ConnPoolUDPInstance *connPoolUDPInstance = new ConnPoolUDPInstance(newUDPSocket, purposeFlag, this);
            instancePool.push_back(connPoolUDPInstance);
            qDebug() << "Opened new UDP port";
            qDebug() << "p.ipAddress" << p.ipAddress;
            qDebug() << "p.portNumber" << p.portNumber;
            return((int)(instancePool.size()-1));

        } else {
            qDebug() << "Could not open UDP port.";
        }

        return -1;
    };

    void closeConnection(int idx, ConnPoolPurposeFlag purposeFlag) {
        if(instancePool.size() > idx && instancePool[idx]->UDPSocket != nullptr) {
            //qDebug() << "purposeFlags before resetting: " << instancePool[idx]->purposeFlags;
            instancePool[idx]->purposeFlags = instancePool[idx]->purposeFlags & ~(uint8_t)purposeFlag;

            if(instancePool[idx]->getNumSubscribedListeners() > 0) {
                qDebug() << "There is/are still subscribed method(s) listening to this UDP pool instance. Cannot remove from pool.";
                return;
            }
            if(instancePool[idx]->purposeFlags == 0) {
                qDebug() << "No purpose left for this COM pool instance. Removing from pool.";

                // Try to send the last message before closing, though not sure it this succeeds
                // See: https://forum.qt.io/topic/112651/send-a-serial-message-before-closing-the-serial-port-in-qt/15
                instancePool[idx]->UDPSocket->waitForBytesWritten(1000); //
                instancePool[idx]->UDPSocket->flush(); //
                instancePool[idx]->UDPSocket->close();
                delete instancePool[idx]->UDPSocket;

                delete instancePool[idx];
                instancePool[idx] = nullptr;
            }
        }
            
    };

    QUdpSocket* getInstance(int idx) {
        if(instancePool.size() > idx && instancePool[idx] != nullptr && instancePool[idx]->UDPSocket != nullptr)
            return instancePool[idx]->UDPSocket;
        qDebug() << "No instance found.";
        return nullptr;
    };

    qint64 writeToInstance(int idx, const QByteArray &byteArray) {
        if(instancePool.size() > idx && instancePool[idx] != nullptr && instancePool[idx]->UDPSocket != nullptr) {
            QUdpSocket *inst = instancePool[idx]->UDPSocket;
//            qDebug() << inst->objectName();
//            qDebug() << inst->localPort();
            return inst->writeDatagram( byteArray, QHostAddress(inst->objectName()), inst->localPort() );
        }
        qDebug() << "No instance found.";
        return 0;
    };

    bool subscribeListener(int idx, QObject *receiver, const char *method, Qt::ConnectionType type = Qt::AutoConnection) {
        if(getInstance(idx)==nullptr || !instancePool[idx]->startListening()) {
            qDebug() << "Could not start listening on this UDP port.";
            return false;
        }
        return connect(instancePool[idx], SIGNAL(messageReceived(QString, quint64)), receiver, method, type);
    };

    void unsubscribeListener(int idx, QObject *receiver, const char *method) {
        disconnect(instancePool[idx], SIGNAL(messageReceived(QString, quint64)), receiver, method);
    };

private:
    std::vector<ConnPoolUDPInstance*> instancePool;

};

