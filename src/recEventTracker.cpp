
#include "recEventTracker.h"

// buffer mode (used for live camera input, but vectors are still stored and can be written at closing of image recording)
RecEventTracker::RecEventTracker(QObject *parent) : QObject(parent),
                                                    applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent))
{

    mode = EventReplayMode::BUFFER;
}

// storage mode (vectors filled and can be read anytime)
RecEventTracker::RecEventTracker(const QString &fileName, QObject *parent) : QObject(parent),
                                                                             applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent))
{

    mode = EventReplayMode::STORAGE;

    // std::cout << "RecEventTracker(const QString& fileName, QObject *parent = 0)" << std::endl;
    // std::cout << "File name = " << fileName.toStdString() << std::endl;

    dataFile = new QFile(fileName);
    int numLines = 0;
    // NOTE: not necessarily an error if this file does not exist
    if (!dataFile->open(QIODevice::ReadOnly))
    { //| QIODevice::Text)
        std::cout << "Could not open offline event log XML file. Check file availability or file access permission." << std::endl;
        delete dataFile;
        dataFile = nullptr;
        return;
    }

    int errLn;
    int errCol;
    QString errStr;
    QDomDocument domDocument;
    if (!domDocument.setContent(dataFile, true, &errStr, &errLn,
                                &errCol))
    {
        std::cout << tr("Could open offline event log XML file, but persing failed at: line %1, column %2\nError: %3")
                         .arg(errLn)
                         .arg(errCol)
                         .arg(errStr)
                         .toStdString();
        return;
    }

    QDomElement root = domDocument.documentElement();
    if (root.tagName() != "RecordedEvents")
    {
        std::cout << "Could open offline event log XML file, but it does not contain recorded events.";
        return;
    }

    QDomElement child;
    quint64 temp_ts;
    uint temp_tr;
    double temp_val;
    double temp_val2;
    QString str;

    child = root.firstChildElement("TrialIncrement");
    while (!child.isNull())
    {
        temp_ts = 0;
        temp_tr = 0;
        str = child.attribute("TimestampMs", "");
        if (!str.isEmpty()){
            temp_ts = str.toULongLong();
        }

        str = child.attribute("TrialNumber", "");
        if (!str.isEmpty())
            temp_tr = str.toUInt();

        if (temp_ts != 0 && temp_tr != 0)
            addTrialIncrement(temp_ts, temp_tr);
        child = child.nextSiblingElement("TrialIncrement");
    }

    child = root.firstChildElement("CameraTempCheck");

    str = "";
    while (!child.isNull())
    {

        temp_ts = 0;
        temp_val = temp_val2 = 0;
        str = child.attribute("TimestampMs", "");
        if (!str.isEmpty())
            temp_ts = str.toULongLong();

        str = child.attribute("Camera1", "");
        if (!str.isEmpty())
            temp_val = str.toDouble();

        str = child.attribute("Camera2", "");
        if (str.isEmpty() || (!str.isEmpty() && str.toDouble() <= 0.0))
            temp_val2 = 0.0;
        else
            temp_val2 = str.toDouble();

        if (temp_ts != 0 && temp_val != 0)
            addTemperatureCheck(temp_ts, std::vector<double>{temp_val, temp_val2});
        child = child.nextSiblingElement("CameraTempCheck");
    }

    child = root.firstChildElement("Message");
    while (!child.isNull())
    {
        temp_ts = 0;
        str = child.attribute("TimestampMs", "");
        if (!str.isEmpty()){
            temp_ts = str.toULongLong();
        }

        str = child.attribute("MessageString", "");
        // TODO: simplify string content?

        if (temp_ts != 0 && !str.isEmpty())
            addMessage(temp_ts, str);
        child = child.nextSiblingElement("Message");
    }

    dataFile->close();
    storageReady = true;
}

// below is the version that reads csv files
/*
// storage mode (vectors filled and can be read anytime)
RecEventTracker::RecEventTracker(const QString& fileName, QObject *parent) :
    QObject(parent),
    applicationSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), parent)) {

    mode=EventReplayMode::STORAGE;

    //std::cout << "RecEventTracker(const QString& fileName, QObject *parent = 0)" << std::endl;
    //std::cout << "File name = " << fileName.toStdString() << std::endl;

    dataFile = new QFile(fileName);
    int numLines=0;
    if(!dataFile->open(QIODevice::ReadOnly)) { //| QIODevice::Text)
        std::cout << "Could not open file" << std::endl;
        return;
    }
    QTextStream in(dataFile);
    QString line = in.readLine(); // NOTE: here \n or \r doesnt get read at the end of line
    delim = determineDelimiter(line);

    // in case someone mixed up the column order, we can still read the file
    QStringList fieldNames = line.split(delim, Qt::SkipEmptyParts);

    char coli_timestamp = 0;
    char coli_eventType = 1;
    char coli_notationValue1 = 2;
    char coli_notationValue2 = 3;

    for(int b=0; b<3; b++) {
        if(fieldNames[b] == "timestamp_ms")
            coli_timestamp = b;
        else if(fieldNames[b] == "event_type")
            coli_eventType = b;
        else if(fieldNames[b] == "notation_value1")
            coli_notationValue1 = b;
        else if(fieldNames[b] == "notation_value2")
            coli_notationValue2 = b;
    }

    // fill up the vectors
    // NOTE: empty cells will be read as 0
    QString cell;
    QStringList splitRow;
    int currLine = 1;

    in.seek(0); //reset for getline()
    line = in.readLine(); //skip header line

    bool parseSuccess_timestamps;
    quint64 temp_ts;

    // read file by lines
    while (!in.atEnd()) {
        line = in.readLine();
        parseSuccess_timestamps = true;

        splitRow = line.split(delim);
        temp_ts = splitRow[coli_timestamp].toULongLong(&parseSuccess_timestamps, 10);
        if(parseSuccess_timestamps) {
            if(splitRow[coli_eventType] == "TRIAL_INCREMENT") {
                //std::cout << splitRow[coli_timestamp].toStdString() << std::endl;
                addTrialIncrement(temp_ts, splitRow[coli_notationValue1].toInt());
            } else if(splitRow[coli_eventType] == "TEMPERATURE_CHECK") {
                addTemperatureCheck(temp_ts, std::vector<double> {splitRow[coli_notationValue1].toDouble(), splitRow[coli_notationValue2].toDouble()});
            }
            storageReady=true;
        } else {
            qDebug() << "Could not parse timestamp value " << splitRow[coli_timestamp] << " at line " << QString::number(currLine);
            storageReady=false;
            return;
        }
        currLine++;
    }
    dataFile->close();
    storageReady=true;
}
*/

bool RecEventTracker::isStorageReady()
{
    return storageReady;
}

RecEventTracker::~RecEventTracker()
{
    close();
}

void RecEventTracker::close()
{
    if (dataFile)
    {
        dataFile->close();
        dataFile->deleteLater();
    }
    delete dataFile;
    dataFile = nullptr;
}

void RecEventTracker::saveOfflineEventLog(uint64 timestampFrom, uint64 timestampTo, const QString &fileName) {

    std::cout << fileName.toStdString() << std::endl;

    bool changedGiven = false;
    QString changedPath;
    bool pathWriteable = SupportFunctions::preparePath(fileName, changedGiven, changedPath);
    //if(changedGiven)
    //    QMessageBox::warning(nullptr, "Path name changed", "The given path/name contained nonstandard characters,\nwhich were changed automatically for the following: a-z, A-Z, 0-9, _");


    QByteArray textContent;

    dataFile = new QFile(fileName);
    bool exists = dataFile->exists();

    if(exists) {
        std::cout << "An offline event log file already exists with name: " << fileName.toStdString() << "" << std::endl;
    }

    bool opened = dataFile->open(QIODevice::ReadWrite | QIODevice::Text);
    //bool opened = dataFile->open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text); // OR-ing the Append kills it, and no exception or anything remains, be aware
    if(!opened) {
        std::cout << "Offline event log XML file recording failure. Could not open file for writing: " << fileName.toStdString() << std::endl;
        delete dataFile;
        dataFile = nullptr;
        // TODO: better exception handling, etc.
        return;
    }

    //bool readable = dataFile->isReadable();
    //textContent = dataFile->readAll();

    QDomDocument document;
    QDomElement root;
    bool existingRead = false;
    if(exists) {
        QString errorString;
        int errorLine;
        int errorColumn;
        existingRead = document.setContent(dataFile, false, &errorString, &errorLine, &errorColumn);
        if (!existingRead) {
            qDebug() << errorLine;
            qDebug() << errorColumn;
            qDebug() << errorString;
        }
    }

    if(exists && existingRead) {
        root = document.firstChildElement();
        QString temp_str = root.attribute("Version", "");
        foundEventLogVersion = 1;
        if (!temp_str.isEmpty())
            foundEventLogVersion = temp_str.toUShort();
        qDebug() << "Appending to found XML contents. Version: " << QString::number(foundEventLogVersion);
        //std::cout << root.nodeName().toStdString() << std::endl;
    } else {
        root = document.createElement("RecordedEvents");
        document.appendChild(root);
        qDebug() << "Creating a fresh XML. Version: " << QString::number(currentEventLogVersion);
        //std::cout << root.nodeName().toStdString() << std::endl;
    }

    if (!root.hasAttribute("Version") || root.attribute("Version","1").toUShort() < currentEventLogVersion)
    {
        // TODO: Safer logic for this? Also at reading
        root.setAttribute("Version", QString::number(currentEventLogVersion));
    }

    QDomElement currObj;

    // NOTE: Should this range inclusive on both sides? (Should be low inclusive high exclusive?) But now it surely prevents data loss
    for (size_t i = 0; i < trialIncrements.size(); i++) {
        if (trialIncrements[i].timestamp >= timestampFrom && trialIncrements[i].timestamp < timestampTo) {
            currObj = document.createElement("TrialIncrement");
            currObj.setAttribute("TimestampMs", QString::number(trialIncrements[i].timestamp));
            currObj.setAttribute("TrialNumber", QString::number(trialIncrements[i].trialNumber));
            root.appendChild(currObj);
        }
    }
    for (size_t i = 0; i < temperatureChecks.size(); i++) {
        if (temperatureChecks[i].temperatures[0] != 0 && temperatureChecks[i].timestamp >= timestampFrom && temperatureChecks[i].timestamp < timestampTo) {
            currObj = document.createElement("CameraTempCheck");
            currObj.setAttribute("TimestampMs", QString::number(temperatureChecks[i].timestamp));
            currObj.setAttribute("Camera1", temperatureChecks[i].temperatures[0]);
            if (temperatureChecks[i].temperatures.size() > 1 && temperatureChecks[i].temperatures[1] != 0)
                currObj.setAttribute("Camera2", temperatureChecks[i].temperatures[1]);
            root.appendChild(currObj);
        }
    }
    for (size_t i = 0; i < messages.size(); i++) {
        if (messages[i].timestamp >= timestampFrom && messages[i].timestamp < timestampTo) {
            currObj = document.createElement("Message");
            currObj.setAttribute("TimestampMs", QString::number(messages[i].timestamp));
            currObj.setAttribute("MessageString", messages[i].messageString);
            root.appendChild(currObj);
        }
    }

    // NOTE: search intervals are only inclusive on the left, but exclusive on the right. Consider this
    // TODO: clear file even if appended, as new XML is flushed into it

    QTextStream textStream(dataFile);
    textStream.seek(0); // rewrite the file

    // NOTE: the line below (XML processing instruction)  is not automatically added for some reason..
    // BUT if we add it like this, it will cumulatively add to the next file write, and it causes problems.. so we do not add it
    //*textStream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    textStream << document.toString();
    dataFile->close();
}

uint RecEventTracker::getLastCommissionedTrialNumber()
{
    return bufferTrialCounter;
}
QString RecEventTracker::getLastMessage()
{
    if(messages.size()==0)
        return "";

    return messages[messages.size()-1].messageString;
}
void RecEventTracker::resetBufferTrialCounter(const quint64 &timestamp)
{

    // NOTE: it is okay to reset the counter even if it is at counting 1, it can signal the beginning of a recording or whatever
    // could be however called a "TrialReset" rather than plainly "TrialIncrement"...
    // if(mode==STORAGE || bufferTrialCounter==1)
    if (mode == STORAGE)
        return;

    bufferTrialCounter = 0;

    // We do not clear the actual vector for safety!
    // The entries before reset will also be logged in the event log, and the user can later decide whether they want to use it
//    trialIncrements.clear();

    // will increase counter instantly to 1
    addTrialIncrement(timestamp);
}
void RecEventTracker::resetBufferMessageRegister(const quint64 &timestamp)
{
    if (mode == STORAGE)
        return;

    // We do not clear the actual vector for safety!
    // The entries before reset will also be logged in the event log, and the user can later decide whether they want to use it
//    messages.clear();

    // will add and empty element
    addMessage(timestamp,"");
}
bool RecEventTracker::isReady()
{
    return storageReady;
}
uint RecEventTracker::getTrialAtTimestamp(quint64 timestamp)
{
    for (size_t i = trialIncrements.size(); i >= 0; i--)
        if (trialIncrements[i].timestamp < timestamp)
        {
            return trialIncrements[i].trialNumber;
        }
    // qDebug() << "No trial found, returning assumed trial number 1";
    return 1;
}

RecEventTracker::TrialIncrement RecEventTracker::getTrialIncrement(quint64 timestamp)
{
    TrialIncrement emptyElem;
    if (trialIncrements.size() < 1)
        return emptyElem;

    size_t i = 1;
    while (i <= trialIncrements.size())
    { // GB: I dont use decremental indexing here, caused some weird "overflow", MSVC 2019 x86_amd64
        if (trialIncrements[trialIncrements.size() - i].timestamp < timestamp)
        {
            // if the gotten timestamp is just after an elem in the vector, that is the one we were looking for
            //    qDebug() << "BUFFER: found applicable TrialIncrement of: \n" <<
            //        "index " << trialIncrements.size()-i <<
            //        "timestamp " << trialIncrements[trialIncrements.size()-i].timestamp <<
            //        "trial number " << trialIncrements[trialIncrements.size()-i].trialNumber;
            return trialIncrements[trialIncrements.size() - i];
        }
        i++;
    }
    return (emptyElem);
}

RecEventTracker::Message RecEventTracker::getMessage(quint64 timestamp)
{
    Message emptyElem;
    if (messages.size() < 1)
        return emptyElem;

    size_t i = 1;
    while (i <= messages.size())
    {
        if (messages[messages.size() - i].timestamp < timestamp)
        {
            return messages[messages.size() - i];
        }
        i++;
    }
    return (emptyElem);
}

RecEventTracker::TemperatureCheck RecEventTracker::getTemperatureCheck(quint64 timestamp)
{
    TemperatureCheck emptyElem;
    if (temperatureChecks.size() < 1)
        return emptyElem;

    size_t i = 1;
    while (i <= temperatureChecks.size())
    { // GB: I dont use decremental indexing here, caused some weird "overflow", MSVC2019 x86_amd64
        if (temperatureChecks[temperatureChecks.size() - i].timestamp < timestamp)
        {
            // if the gotten timestamp is just after an elem in the vector, that is the one we were looking for
            //    qDebug() << "BUFFER: found applicable TemperatureCheck of: \n" <<
            //        "index " << temperatureChecks.size()-i <<
            //        "timestamp " << temperatureChecks[temperatureChecks.size()-i].timestamp <<
            //        "trial number " << temperatureChecks[temperatureChecks.size()-i].trialNumber;
            return temperatureChecks[temperatureChecks.size() - i];
        }
        i++;
    }
    return (emptyElem);
}

/*
Message RecEventTracker::getMessage(quint64 timestamp)
{
    Message emptyElem;
    if (messages.size() < 1)
        return emptyElem;

    size_t i = 1;
    while (i <= messages.size())
    { // GB: I dont use decremental indexing here, caused some weird "overflow", MSVC2019 x86_amd64
        if (messages[messages.size() - i].timestamp < timestamp)
        {
            // if the gotten timestamp is just after an elem in the vector, that is the one we were looking for
            //    qDebug() << "BUFFER: found applicable TemperatureCheck of: \n" <<
            //        "index " << temperatureChecks.size()-i <<
            //        "timestamp " << temperatureChecks[temperatureChecks.size()-i].timestamp <<
            //        "trial number " << temperatureChecks[temperatureChecks.size()-i].trialNumber;
            return messages[messages.size() - i];
        }
        i++;
    }
    return (emptyElem);
}
*/

void RecEventTracker::addTrialIncrement(const quint64 &timestamp)
{
    bufferTrialCounter++; // increment internal counter
    // qDebug() << "pushed back =   " << QString::number(timestamp);
    trialIncrements.push_back(TrialIncrement{timestamp, bufferTrialCounter});
}
void RecEventTracker::addTemperatureCheck(std::vector<double> d)
{
    quint64 timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    // qDebug() << "pushed back temperature check at = " << QString::number(timestamp);
    temperatureChecks.push_back(TemperatureCheck{timestamp, d});
}
/*
void RecEventTracker::updateGrabTimestamp(CameraImage cimg) {
    currentGrabTimestamp = cimg.timestamp; // ensures time fidelity to camera clock
    timeSinceLastGrab.start();
}
*/

void RecEventTracker::addTrialIncrement(quint64 timestamp, uint trialNumber)
{
    trialIncrements.push_back(TrialIncrement{timestamp, trialNumber});
    // NOTE: there is no increment here, so properly monotonically increasing trial numbering should be cared for in the caller class
}

void RecEventTracker::addMessage(const quint64 &timestamp, const QString &str)
{
    messages.push_back(Message{timestamp, str});
    // NOTE: there is no increment here, so properly monotonically increasing trial numbering should be cared for in the caller class
}

void RecEventTracker::addTemperatureCheck(quint64 timestamp, std::vector<double> d)
{ // TODO: HA CSAK 1 ADAT VAN
    temperatureChecks.push_back(TemperatureCheck{timestamp, d});
}

QChar RecEventTracker::determineDelimiter(QString _text)
{
    QChar delimiter = 'E';
    const QChar acceptedDelims[] = {
        (QChar)'\u002C', // COMMA \u002C
        (QChar)'\u003B', // SEMICOLON \u003B
        (QChar)'\u0009', // CHARACTER TABULATION \u0009
        (QChar)'\u0020', // SPACE \u0020
    };

    for (int pos = 0; pos < _text.size(); pos++)
        for (char hcx = 0; hcx < 4; hcx++)
            if (acceptedDelims[hcx] == _text[pos])
            {
                delimiter = _text[pos];
                goto getout;
            }
getout:

    // if (delimiter == 'E')
    //     throw new Exception();
    return delimiter;
}