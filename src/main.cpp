#include <QtCore/QCoreApplication>
#include <QApplication>
#include <QDebug>
#include <QString>
#include "wmain.h"
#include "wdebug.h"

//GLOBAL ORC APPLICATION VARIABLES:
bool    ORC_AutoIP = true;
QString ORC_Header = "OBS Remote Control";
bool    ORC_LockSources = false;
bool    ORC_LockScenes = false;
bool    ORC_MatchScene = false;
QString ORC_Exe = "obs64";
QString ORC_IP = "127.0.0.1";
QString ORC_Port = "4444";
bool    ORC_Debug = false;
QString ORC_Comment = "";
QString ORC_Path = "";
QString ORC_Version = "v1.0";


int read_cfg_file();

//http://www.bogotobogo.com/index.php
//http://convertico.com/   PNG to ICO convertor.

// 1 nog eens time out methode checken om te zien of websocket tot stand komt. en de maximale wachttijd.

//1b) Test met installeren op Laptop en host vaste PC.
//    Bij test in verschillende folders uitproberen.

//2) MSI creeeren

//4) Host list toevoegen:
//- IP addressen komen in Array met een Host Naam.
//- Indien AutoIP -on dan wordt eerste gebruikt.
//- Testen dat multiple instances kunnen draaien.

//5) Stream view window toevoegen.

//6) Record info toevoegen:
//- Wissen als er gerecord wordt.
//- Bij stoppen de record tijd vermelden.

//7) In Log window toevoegen als source wijziging op de host gedaan wordt. Kijken of ChangeSources JSON bestaat.



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (read_cfg_file() != 0) return(0);

    wMain wM;

    //return a.exit(0);

    return a.exec();
}
