#include <QCoreApplication>
#include <QString>
#include <QDebug>
#include <iostream>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QMessageBox>

#include "globals.h"

using namespace std;

int read_cfg_file(){
    QMessageBox msgBox;

    ORC_Path = QDir::currentPath();
    QFile fileConfig(ORC_Path + "/config.json");
    if (fileConfig.open(QFile::ReadOnly|QFile::Text)){
        QString jtext = fileConfig.readAll();
        fileConfig.close();

        QMessageBox msgBox;
        QJsonDocument jdoc = QJsonDocument::fromJson(jtext.toUtf8());

        if (!jdoc.isObject()){
            msgBox.critical(0,"ORC ERROR","File "+ ORC_Path +"/config.json is not in JSON format.\nEnding program.",QMessageBox::Close);
            return(1);
        }

        QJsonObject jobj = jdoc.object();
        QString jkey, jval;

        jkey ="auto-ip";
        if (jobj.contains(jkey)){
            jval = jobj[jkey].toString();
            if (jval == "on") ORC_AutoIP = true;
        }else{
            msgBox.critical(0,"ORC ERROR","File config.json misses key <auto-ip>.\nEnding program.",QMessageBox::Close);
            return (1);
        }

        jkey ="header";
        if (jobj.contains(jkey)){
            ORC_Header = jobj[jkey].toString();
        }else{
            msgBox.critical(0,"ORC ERROR","File config.json misses key <header>.\nEnding program.",QMessageBox::Close);
            return (1);
        }

        jkey ="lock-sources";
        if (jobj.contains(jkey)){
            jval = jobj[jkey].toString();
            if (jval == "on") ORC_LockSources = true;
        }else{
            msgBox.critical(0,"ORC ERROR","File config.json misses key <lock-sources>.\nEnding program.",QMessageBox::Close);
            return (1);
        }

        jkey ="lock-scenes";
        if (jobj.contains(jkey)){
            jval = jobj[jkey].toString();
            if (jval == "on") ORC_LockScenes = true;
        }else{
            msgBox.critical(0,"ORC ERROR","File config.json misses key <lock-scenes>.\nEnding program.",QMessageBox::Close);
            return (1);
        }

        jkey ="match-scene";
        if (jobj.contains(jkey)){
            jval = jobj[jkey].toString();
            if (jval == "on") ORC_MatchScene = true;
        }else{
            msgBox.critical(0,"ORC ERROR","File config.json misses key <match-scene>.\nEnding program.",QMessageBox::Close);
            return (1);
        }

        jkey ="myexe";
        if (jobj.contains(jkey)){
            ORC_Exe = jobj[jkey].toString();
        }else{
            msgBox.critical(0,"ORC ERROR","File config.json misses key <obs64>.\nEnding program.",QMessageBox::Close);
            return (1);
        }

        jkey ="myip";
        if (jobj.contains(jkey)){
            ORC_IP = jobj[jkey].toString();
        }else{
            msgBox.critical(0,"ORC ERROR","File config.json misses key <myip>.\nEnding program.",QMessageBox::Close);
            return (1);
        }

        jkey ="myport";
        if (jobj.contains(jkey)){
            ORC_Port = jobj[jkey].toString();
        }else{
            msgBox.critical(0,"ORC ERROR","File config.json misses key <myport>.\nEnding program.",QMessageBox::Close);
            return (1);
        }

        jkey ="debug";
        if (jobj.contains(jkey)){
            jval = jobj[jkey].toString();
            if (jval == "on") ORC_Debug = true;
        }else{
            msgBox.critical(0,"ORC ERROR","File config.json misses key <debug>.\nEnding program.",QMessageBox::Close);
            return (1);
        }

        jkey ="comment";
        if (jobj.contains(jkey)){
            ORC_Comment = jobj[jkey].toString();
        }else{
            msgBox.critical(0,"ORC ERROR","File config.json misses key <xcomment>.\nEnding program.",QMessageBox::Close);
            return (1);
        }

        return(0); // completed parsing.
    }

    msgBox.critical(0,"ORC ERROR","File " + ORC_Path + "/config.json not found.\nEnding program.",QMessageBox::Close);
    return(1);
}
