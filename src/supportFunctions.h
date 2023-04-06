#pragma once

/**
    @author Gábor Bényei
*/

#include <iostream>
#include <QtCore/QObject>
#include <QtCore/QFile>
#include <QDir>
#include <QtCore/qfileinfo.h>

/**
    
    Various little functions that are needed here and there, e.g. checking and simplifying strings of filenames and paths before using them for I/O

*/
class SupportFunctions : public QObject {
    Q_OBJECT

public: 

    static QString simplifyReceivedMessage(QString str) {
        // remove CR, LF and other strange characters
        str.replace("\r", "");
        str.replace("\n", "");
        //str.replace("\t", "");
        str.replace("\"", "");
        // Qt has this method to remove trailing whitespace from the beginning and the end of a string, and replace internal whitespaces with a single space
        str = str.simplified();
        // if there are "\" characters, change them to "/"
        str.replace("\\", "/");

        return str;
    };

    static QString simplifyPathName(QString str) {
        // remove CR, LF and other strange characters
        str.replace("\r", "");
        str.replace("\n", "");
        str.replace("\t", "");
        //fullPath.replace(":", ""); // todo: only one ":" should maximally occur
        str.replace("*", "");
        str.replace("\"", "");
        str.replace("<", "");
        str.replace(">", "");
        str.replace("|", "");
        // Qt has this method to remove trailing whitespace from the beginning and the end of a string, and replace internal whitespaces with a single space
        str = str.simplified();
        // if there are "\" characters, change them to "/"
        str.replace("\\", "/");
        //todo: remove "..." and replace with ".." as long as they persist, etc. Now it is just a workaround
        str.replace("/./", "/");
        str.replace("...", "..");

        return str;
    };

    static bool preparePath(const QString& target) {

        if(QFileInfo(target).absoluteDir().exists() == true) 
            return true;
        
        //std::cout << "Specified path does not point to an existing folder. Now we try to create the path iteratively: folder, sub-folder sub-sub-folder, ..." << std::endl;
        //std::cout << "target = " << target.toStdString() << std::endl;

        // GB NOTE: .absolutePath() causes undefined behaviour 
        // (without exception) when called upon a string that does NOT point to a file, 
        // but only a folder, on an external drive (not .exe path). 
        // I could not figure out the exact problem, but made a workaround. 
        // Problem can be diagnosed if desiredPath variable contains the .exe path and the target path combined.
        // I think it is some kind of memory problem in the background
        // Qt 5.15.2, MSVC 2019 v86_amd64
        QString desiredPath;
        if(target[target.length()-1] == '/') { // if "target" is a path itself
            //std::cout << "DESIRED PATH VAR = target "<< std::endl;
            desiredPath = target;
        } else {// if "target" is pointing to a file, not a folder/dir
            //std::cout << "DESIRED PATH VAR = QFileInfo(target).absolutePath()"<< std::endl;
            //desiredPath = QFileInfo(target).absolutePath(); 
            int idx = target.lastIndexOf("/");
            desiredPath = target.mid(0, idx);
        }
        //std::cout << "desiredPath = " << desiredPath.toStdString() << std::endl;
        QStringList subStrings = desiredPath.split('/');
        bool success = false;

        //std::cout << "subStrings 0-th elem = " << subStrings[0].toStdString() << std::endl;
        QString cumulatedPath = subStrings[0] + "/";
        for(int h=1; h < subStrings.length(); h++)
        {
            //std::cout << "subStrings n-th elem = " << subStrings[h].toStdString() << std::endl;
            cumulatedPath = cumulatedPath + subStrings[h] + "/";
            //std::cout << "Now creating: " << QFileInfo(cumulatedPath).absolutePath().toStdString() << std::endl;
            if(QFileInfo(cumulatedPath).absoluteDir().exists() == false) {
                success = QDir().mkdir(QFileInfo(cumulatedPath).absolutePath());
                if(!success)
                    break;
            }
        }

        if(success)
            return true;
        else 
            return false;
    
    };

};