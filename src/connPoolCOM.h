
#pragma once

/**
    @author Gabor Benyei
*/

#include <QtCore/QObject>
#include <QByteArray>
#include <QUdpSocket>
#include <QSerialPort>

Q_DECLARE_METATYPE(QSerialPort::SerialPortError)

/**
    
    This enum contains flags that can be used to manage sockets/ports used for multiple purposes, by different classes.

*/
enum class ConnPoolPurposeFlag
{
    CAMERA_TRIGGER = 1 << 0, // 1
    STREAMING = 1 << 1, // 2
    REMOTE_CONTROL = 1 << 2 //, // 4
};


/**
    
    This struct holds a bundle of settings that are necessary for opening a COM port

*/
struct ConnPoolCOMInstanceSettings {
        QString name;
        qint32 baudRate;
        QString stringBaudRate;
        QSerialPort::DataBits dataBits;
        QString stringDataBits;
        QSerialPort::Parity parity;
        QString stringParity;
        QSerialPort::StopBits stopBits;
        QString stringStopBits;
        QSerialPort::FlowControl flowControl;
        QString stringFlowControl;
        bool localEchoEnabled;
    };

/**
    
    This class is necessary for the COM pool to work. Each of these instances can start a separate listener.

*/
class ConnPoolCOMInstance : public QObject {
    Q_OBJECT

    public:
        explicit ConnPoolCOMInstance(QSerialPort *port, ConnPoolPurposeFlag purposeFlag, QObject *parent) : QObject(parent), port(port), purposeFlags((uint8_t)purposeFlag) {};
        ~ConnPoolCOMInstance() override {};

        bool startListening() {
            bool isOpen = port->isOpen();
            bool handleReadyReadWorking = connect(port, SIGNAL(readyRead()), this, SLOT(handleReadyRead()), Qt::QueuedConnection);
            bool handleErrorWorking = connect(port, SIGNAL(errorOccurred(QSerialPort::SerialPortError)), this, SLOT(handleError(QSerialPort::SerialPortError)), Qt::QueuedConnection);
            return isOpen && handleReadyReadWorking && handleErrorWorking;
        };

    QSerialPort *port;
    uint8_t purposeFlags;
        
    private slots:
        void handleReadyRead() {
            quint64 timestamp  = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            //qDebug() << "handleReadyRead()";
            QByteArray data = port->readAll();
        //    emit messageReceived(QString(data), timestamp);
            // TODO: should this be in a separate thread?

            // The following is needed because unlike UDP datagrams, which arrive always separately,
            // serial messages (if sent very fast), arrive in our buffer here many at once,
            // just as if we concatenated them. So here we take them apart, and pass on each separately.
            // Important that we assume each separate message ends with a CR or LF character, or their un-encoded strings
            QString dataStr = QString(data);

            // De-canonize if contains un-encoded control characters
            while(dataStr.length()>1 && dataStr.contains("\\n"))
                dataStr = dataStr.replace("\\n","\n");
            while(dataStr.length()>1 && dataStr.contains("\\r"))
                dataStr = dataStr.replace("\\r","\r");

            // Trim newlines from the beginning and end
            while(!dataStr.isEmpty() && (dataStr[0] == '\n' || dataStr[0] == '\r'))
                dataStr = dataStr.remove(0, 1);
            while(!dataStr.isEmpty() && (dataStr[dataStr.length()-1] == '\n' || dataStr[dataStr.length()-1] == '\r'))
                dataStr = dataStr.remove(dataStr.length()-1, 1);

            // Split by newline (CR or LF)
            QRegExp separators("(\r|\n)");
            QStringList msgList = dataStr.split(separators);
            for(int cc=0; cc<msgList.length(); cc++) {
                if(!msgList[cc].isEmpty())
                    emit messageReceived(msgList[cc], timestamp);
            }
        };

        void handleError(QSerialPort::SerialPortError serialPortError) {
            //qDebug() << "handleError()";
            if (serialPortError == QSerialPort::ReadError) {
                qDebug() << "An I/O error occurred while reading";
            }
        }
    
    public slots:
        int getNumSubscribedListeners() {
            return this->receivers(SIGNAL(messageReceived(QString, quint64)));
        };
        
    signals:
        void messageReceived(QString msg, quint64 timestamp);
};

/**
    
    This class helps handling a COM connection to another machine, in a way that different classes can use the same connection for a purpose they specify. 
    
    Incoming data is read once, and any method that has subcribed beforehand to get the message, will receive it as a signal. 
    If a method is not interested in the connection anymore, it can unsubscribe from the messages listened, or even prompt the pool class to close the connection. 
    But a connection is only closed if the number of its subscribed "listener" methods is 0. 
    
    This class makes it very easy to manage when for example PupilEXT is running and listening to e.g. a COM port, and gets a remote command 
    from that port to start streaming (on that same port, or any other port). Also it can be used to manage listening to the same port (pool instance)
    from different classes, with no need to worry for failing to open an already opened port.

    For sending data, or accessing socket directly, the socket instance pointer can be gotten from outside this class, via getInstance().
    This should be used with caution, as the connection should not be closed directly, but only via closeConnection() method of the pool class.

    IMPORTANT: 
        When this class is told to open a new connection (and assign an instance) to a COM port, whose NAME matches 
        that of an already opened port /existing instance:
            - if other COM port settings match, only NAME differs, it will:
                - not open a new port and assign a pool COM instance, 
                - but return with the identifier (vector index) of the matching instance.
                => Connection will be seen as unsuccessful from the method that called setupAndOpenConnection().
            - if other COM Settings are also different, it will follow a conservative strategy:
                - not open a new port and assign a pool COM instance, 
                - and neither return with the identifier (vector index) of the matching instance. 
                => Connection will be seen as successful from the method that called setupAndOpenConnection().

        One opened COM port /pool COM instance can be used for multiple purposes, marked by purposeFlags. 
        Also - in this version - multiple ports can be used for the same purpose.
        Classes that use these pools must internally keep track of their pool instance IDs assigned, returned by 
        setupAndOpenConnection(), and reset it to e.g. -1 or other invalid value when the connection is closed.

*/
class ConnPoolCOM : public QObject {
    Q_OBJECT

public:
    explicit ConnPoolCOM(QObject *parent = 0) : QObject(parent) {};
    ~ConnPoolCOM() override {};

    std::vector<QString> getOpenedNames() {
        std::vector<QString> found;
        for(int c=0; c<instancePool.size(); c++)
            if( instancePool[c] && instancePool[c]->port && instancePool[c]->port->isOpen() )
                found.push_back(instancePool[c]->port->portName());
        return found;
    }

    int setupAndOpenConnection(ConnPoolCOMInstanceSettings p, ConnPoolPurposeFlag purposeFlag) {

        qDebug() << instancePool.size();

        for(int c=0; c<instancePool.size(); c++)
            if( instancePool[c] && instancePool[c]->port && instancePool[c]->port->isOpen() && 
                instancePool[c]->port->portName() == p.name ) {
                // If there is already an open port here, return with its index

                if( instancePool[c]->port->baudRate() != p.baudRate || 
                    instancePool[c]->port->dataBits() != p.dataBits || 
                    instancePool[c]->port->parity() != p.parity || 
                    instancePool[c]->port->stopBits() != p.stopBits || 
                    instancePool[c]->port->flowControl() != p.flowControl ) {
                    
                    qDebug() << "Attempted to open a new port with the same port name, but different Baud/DataBits/Parity/StopBits/FlowControl settings. \nNo new port was opened, nor was an existing reassigned.";
                    return -1;
                }

                if(instancePool[c]->purposeFlags & (uint8_t)purposeFlag)
                    qDebug() << "This COM pool instance is already in use for the same purpose.";
                else {
                    qDebug() << "Already used COM pool instance assigned for another purpose too.";
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

        QSerialPort *serialPort = new QSerialPort();

        serialPort->setPortName(p.name);
        serialPort->setBaudRate(p.baudRate);
        serialPort->setDataBits(p.dataBits);
        serialPort->setParity(p.parity);
        serialPort->setStopBits(p.stopBits);
        serialPort->setFlowControl(p.flowControl);
        

        if(serialPort->open(QIODevice::ReadWrite)) {
            for(int c=0; c<instancePool.size(); c++)
                if(instancePool[c] == nullptr) { //  || (instancePool[c] != nullptr && instancePool[c]->port == nullptr) ) {
                    instancePool[c] = new ConnPoolCOMInstance(serialPort, purposeFlag, this);
                    instancePool[c]->purposeFlags = (uint8_t)purposeFlag;
                    instancePool[c]->port = serialPort;
                    qDebug() << "Previously abandoned COM pool instance added to pool again.";
                    return c;
                }

            // Otherwise, we add the instance pointer to the pool vector
            ConnPoolCOMInstance *connPoolCOMInstance = new ConnPoolCOMInstance(serialPort, purposeFlag, this);
            instancePool.push_back(connPoolCOMInstance);
            qDebug() << "Opened new COM port";
            qDebug() << "p.baudRate" << p.baudRate;
            qDebug() << "p.dataBits" << p.dataBits;
            qDebug() << "p.parity" << p.parity;
            qDebug() << "p.stopBits" << p.stopBits;
            qDebug() << "p.flowControl" << p.flowControl;
            return((int)(instancePool.size()-1));

        } else {
            qDebug() << "Could not open COM port.";
        }

        return -1;
    };

    void closeConnection(int idx, ConnPoolPurposeFlag purposeFlag) {
        if(instancePool.size() > idx && instancePool[idx]->port != nullptr) {
            //qDebug() << "purposeFlags before resetting: " << instancePool[idx]->purposeFlags;
            instancePool[idx]->purposeFlags = instancePool[idx]->purposeFlags & ~(uint8_t)purposeFlag;

            if(instancePool[idx]->getNumSubscribedListeners() > 0) {
                qDebug() << "There is/are still subscribed method(s) listening to this COM pool instance. Cannot remove from pool.";
                return;
            }
            if(instancePool[idx]->purposeFlags == 0) {
                qDebug() << "No purpose left for this COM pool instance. Removing from pool.";

                // Try to send the last message before closing, though not sure it this succeeds
                // See: https://forum.qt.io/topic/112651/send-a-serial-message-before-closing-the-serial-port-in-qt/15
                instancePool[idx]->port->waitForBytesWritten(1000);
                instancePool[idx]->port->flush();
                instancePool[idx]->port->close();
                delete instancePool[idx]->port;

                delete instancePool[idx];
                instancePool[idx] = nullptr;
            }
        }
            
    };

    QSerialPort* getInstance(int idx) {
        if(instancePool.size() > idx && instancePool[idx] != nullptr && instancePool[idx]->port != nullptr)
            return instancePool[idx]->port;
        qDebug() << "No instance found.";
        // ideally this line should never be reached
        return nullptr;
    };

    bool subscribeListener(int idx, QObject *receiver, const char *method, Qt::ConnectionType type = Qt::AutoConnection) {
        if(getInstance(idx)==nullptr || !instancePool[idx]->startListening()) {
            qDebug() << "Could not start listening on this COM port.";
            return false;
        }
        return connect(instancePool[idx], SIGNAL(messageReceived(QString, quint64)), receiver, method, type);
    };

    void unsubscribeListener(int idx, QObject *receiver, const char *method) {
        disconnect(instancePool[idx], SIGNAL(messageReceived(QString, quint64)), receiver, method);
    };

private:
    std::vector<ConnPoolCOMInstance*> instancePool;

};

