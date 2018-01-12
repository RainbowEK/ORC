#include <QDebug>
#include "wmain.h"
#include "wdebug.h"
#include "ui_wmain.h"
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QTimer>
#include <QMessageBox>
#include <QPropertyAnimation>

#include "globals.h"

#define BOOL_STR(b) (b?"true":"false")

void logFile(QString *logData, QString logPath, QString logFirst, QString logExtCur, QString logExtBak, int logSize);
QString Seconds2TimeStamp(int iSeconds);

wMain::wMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::wMain)
{
    ORC_Connected=false;
    ORC_Stream=false;
    ORC_Record=false;
    ORC_CurrentProfile="";
    ORC_CurrentScene="";
    ORC_IconChange=false;
    ORC_Exiting=false;

    ui->setupUi(this);

    QMessageBox msgBox;

    connect(&ws, &QWebSocket::connected, this, &wMain::onConnected);
    connect(&ws, &QWebSocket::disconnected, this, &wMain::onClosed);
    ws.open(QUrl("ws://" + ORC_IP + ":" + ORC_Port));

    ORC_Connected=ORC_Connected;

    wD = new wDebug();
    wD->hide();
    if (ORC_Debug) wD->show();

    debug("Constructor wMain: wMain form created.\n");

    ui->btnRecord->setStyleSheet("background-color:#800000; color:black; border: 2px solid gray; border-radius: 10px;");//Dark Red
    ui->btnStream->setStyleSheet("background-color:#006600; color:black; border: 2px solid gray; border-radius: 10px;");//Dark Green
    ui->lblPulse->setStyleSheet("border-radius: 10px; color: LightGray; font: 10pt;");

    if(ORC_LockScenes) ui->lstScenes->setEnabled(false);
    if(ORC_LockSources) ui->lstSources->setEnabled(false);
    this->setWindowTitle(ORC_Header);

    QApplication::setWindowIcon(QIcon(":/orc-logo.ico"));

    QTimer *IconTimer = new QTimer(this);
    connect(IconTimer, SIGNAL(timeout()), this, SLOT(onIconTime()));
    IconTimer->start(800);

    this->setFixedSize(this->geometry().width(),this->geometry().height());
    this->show();

    this->raise();
    this->setFocus();
    this->activateWindow();


    ui->btnRecord->installEventFilter(this);
    ui->btnStream->installEventFilter(this);

    wD->text("#ORC(OBS Remote Control) started on: " + QDateTime::currentDateTime().toString("yyyy'-'MM'-'dd','HH':'mm':'ss'.'"));
    log("#ORC(OBS Remote Control) started on: " + QDateTime::currentDateTime().toString("yyyy'-'MM'-'dd','HH':'mm':'ss'.'"));
    log("#" + QDateTime::currentDateTime().toString("HH':'mm':'ss': '") + "ORC version <" + ORC_Version + ">.");
    log("#" + QDateTime::currentDateTime().toString("HH':'mm':'ss': '") + "ORC path <" + ORC_Path + ">.");
}

wMain::~wMain() {
    delete ui;
}

void wMain::closeEvent(QCloseEvent *event) {
    debug("::closeEvent entered\n");

    QMessageBox::StandardButton reply;
    if(ORC_Connected) reply = msgBox.question(this,"ORC Exiting","\nAre you sure you want to EXIT the remote control?\n",QMessageBox::Yes | QMessageBox::No);
    if ((reply == QMessageBox::Yes) || !ORC_Connected) {
        ORC_Exiting=true;
        event->accept();
        log("#" + QDateTime::currentDateTime().toString("HH':'mm':'ss': '") + "ORC application closing...");

        logFile(new QString(ui->txtLog->toPlainText()), ORC_Path, "log", "txt", "bak", 100000);
        logFile(new QString(wD->getText()), ORC_Path, "debug", "txt", "bak", 1000000);

        wD->close();
        ws.close();
    } else {
        event->ignore();
    }
}

void wMain::onClosed() {
    debug("::onClosed entered\n");

    if (ORC_Connected) {
        debug("::onClosed: IP connection closed\n");
        log("#" + QDateTime::currentDateTime().toString("HH':'mm':'ss': '") + "IP connection closed.");
        if (!ORC_Exiting) msgBox.warning(this,"ORC WARNING","Connection with OBS host lost.\n\nEnding program.",QMessageBox::Close);
    } else {
        debug("::onClosed: Cannot connect to OBS host. Was OBS running?\n");
        log("#" + QDateTime::currentDateTime().toString("HH':'mm':'ss': '") + "Cannot connect to OBS host.");
        if (!ORC_Exiting) msgBox.warning(this,"ORC WARNING","Cannot connect to OBS host.\n\nCheck if OBS is running.\n\nEnding program.",QMessageBox::Close);
    }
    ORC_Connected = false;
    this->close();
}

void wMain::onConnected() {
    connect(&ws, &QWebSocket::textMessageReceived, this, &wMain::onReceived);

    debug("::onConnected: WebSocket connected.\n");
    ui->lblIPhost->setText("IP host: " + ORC_IP + ":" + ORC_Port);
    log("#" + QDateTime::currentDateTime().toString("HH':'mm':'ss': '") + "Connected to host <" + ORC_IP + ":" + ORC_Port +">.");

    // Load all the lists:
    ws.sendTextMessage("{\"request-type\":\"GetStreamingStatus\",\"message-id\":\"idGetStreamingStatus\"}"); //Request as first to get Stream and Record status correct
    ws.sendTextMessage("{\"request-type\":\"GetVersion\",\"message-id\":\"idGetVersion\"}");
    ws.sendTextMessage("{\"request-type\":\"GetCurrentScene\",\"message-id\":\"idGetCurrentScene\"}");
    ws.sendTextMessage("{\"request-type\":\"GetSceneList\",\"message-id\":\"idGetSceneList\"}");
    ws.sendTextMessage("{\"request-type\":\"GetCurrentProfile\",\"message-id\":\"idGetCurrentProfile\"}");
    ws.sendTextMessage("{\"request-type\":\"ListProfiles\",\"message-id\":\"idListProfiles\"}");
    ws.sendTextMessage("{\"request-type\":\"GetStreamingStatus\",\"message-id\":\"idGetStreamingStatus\"}");

    ws.sendTextMessage("{\"request-type\":\"SetHeartbeat\",\"message-id\":\"idSetHeartbeat\",\"enable\":true}");

    ORC_Connected = true;
}

void wMain::onReceived(QString message) {
    debug("::onReceived: " + message);

    static bool boolFirstRunProfile = true;
    static bool boolFirstRunScene = true;
    std::string str;

    str = message.toStdString();

    QMessageBox msgBox;
    QJsonDocument jdoc = QJsonDocument::fromJson(message.toUtf8());

    if (jdoc.isObject()) {
        QJsonObject jobj = jdoc.object();
        QString jkey, jstr, jstr2, jstr3;
        bool jbool;

        //Dedicated messages-id parsing:
        //==============================
        jkey ="message-id";
        if (jobj.contains(jkey)) {
            jstr = jobj[jkey].toString();

            if(jstr=="idGetVersion") { // only one time requested at start-up.
                log("#" + QDateTime::currentDateTime().toString("HH':'mm':'ss': '") + "HOST OBS version <" + jobj["obs-studio-version"].toString() + ">.");
                log("#" + QDateTime::currentDateTime().toString("HH':'mm':'ss': '") + "HOST Websocket version <" + jobj["obs-websocket-version"].toString() + ">.");

                if(ORC_Record) log("#" + QDateTime::currentDateTime().toString("HH':'mm':'ss': '") + "RECORDING.");
                else           log("#" + QDateTime::currentDateTime().toString("HH':'mm':'ss': '") + "record off.");

                if(ORC_Stream) log("#" + QDateTime::currentDateTime().toString("HH':'mm':'ss': '") + "STREAMING.");
                else           log("#" + QDateTime::currentDateTime().toString("HH':'mm':'ss': '") + "stream off.");
            } else if(jstr=="idGetCurrentProfile") {
                ORC_CurrentProfile = jobj["profile-name"].toString();
                ws.sendTextMessage("{\"request-type\":\"ListProfiles\",\"message-id\":\"idListProfiles\"}"); //GetProfiles
                if (boolFirstRunProfile){
                    log("#" + QDateTime::currentDateTime().toString("HH':'mm':'ss': '") + "Current Profile <" + ORC_CurrentProfile + ">.");
                    boolFirstRunProfile = false;
                }
            } else if(jstr=="idGetCurrentScene") {
                ORC_CurrentScene = jobj["name"].toString();
                if (boolFirstRunScene){
                    log("#" + QDateTime::currentDateTime().toString("HH':'mm':'ss': '") + "Current Scene <" + ORC_CurrentScene + ">.");
                    boolFirstRunScene = false;
                }
                QJsonArray jarr = jobj["sources"].toArray();
                QJsonObject jobj2;
                ui->lstSources->clear();
                for (int i=0; i< jarr.size(); i++) {
                    jobj2 = jarr.at(i).toObject();
                    jstr2 = jobj2["name"].toString();
                    ui->lstSources->addItem(jstr2);
                    ui->lstSources->item(i)->setBackground(Qt::white);
                    ui->lstSources->item(i)->setForeground(Qt::black);

                    if (!jobj2["render"].toBool()) {
                        ui->lstSources->item(i)->setBackground(Qt::black);
                        ui->lstSources->item(i)->setForeground(Qt::darkGray);
                    } else {
                        ui->lstSources->item(i)->setBackground(Qt::white);
                        ui->lstSources->item(i)->setForeground(Qt::black);
                        if(ORC_LockSources) ui->lstSources->item(i)->setBackground(Qt::gray);
                    }
                }
                ws.sendTextMessage("{\"request-type\":\"GetSceneList\",\"message-id\":\"idGetSceneList\"}"); //GetScenes
            }else if(jstr=="idListProfiles") {
                QJsonArray jarr = jobj["profiles"].toArray();
                QJsonObject jobj2;
                ui->lstProfiles->clear();
                for (int i=0; i< jarr.size(); i++) {
                    jobj2 = jarr.at(i).toObject();
                    jstr2 = jobj2["profile-name"].toString();
                    ui->lstProfiles->addItem(jstr2);
                    ui->lstProfiles->item(i)->setBackground(Qt::white);
                    ui->lstProfiles->item(i)->setForeground(Qt::black);

                    if (ORC_CurrentProfile == jstr2) {
                        ui->lstProfiles->item(i)->setBackground(Qt::blue);
                        ui->lstProfiles->item(i)->setForeground(Qt::yellow);

                        //Auto selected scene when profile is changed, when the IDs are matching:
                        //If an ID is used it must be put as first characters of Profile and Scene name and starting with "#" and ending with "-"
                        if (ORC_MatchScene) {
                            bool intPosBeginProfile = false;
                            int intPosEndProfile = -1;
                            QString strProfileID  = "";
                            QString strScene = "";
                            intPosBeginProfile = ORC_CurrentProfile.startsWith("#");
                            intPosEndProfile = ORC_CurrentProfile.indexOf("-");
                            if (intPosBeginProfile  && (intPosEndProfile > 0)) {
                                strProfileID = ORC_CurrentProfile.mid(0, intPosEndProfile);
                                for (int x=0; x < ui->lstScenes->count(); x++ ) {
                                    strScene = ui->lstScenes->item(x)->text();
                                    if (strScene.contains(strProfileID)) {
                                        ws.sendTextMessage("{\"request-type\":\"SetCurrentScene\",\"message-id\":\"SetScenes\",\"scene-name\":\"" + strScene + "\" }");
                                        ws.sendTextMessage("{\"request-type\":\"GetCurrentScene\",\"message-id\":\"idGetCurrentScene\"}"); //GetCurScene
                                    }
                                }
                            }
                        }
                    }
                }
            }else if(jstr=="idGetSceneList") {
                QJsonArray jarr = jobj["scenes"].toArray();
                QJsonObject jobj2;
                ui->lstScenes->clear();
                for (int i=0; i< jarr.size(); i++) {
                    jobj2 = jarr.at(i).toObject();
                    jstr2 = jobj2["name"].toString();
                    ui->lstScenes->addItem(jstr2);
                    ui->lstScenes->item(i)->setBackground(Qt::white);
                    ui->lstScenes->item(i)->setForeground(Qt::black);

                    if(ORC_LockScenes) ui->lstScenes->item(i)->setBackground(Qt::gray);

                    if (ORC_CurrentScene == jstr2) {
                        ui->lstScenes->item(i)->setBackground(Qt::blue);
                        ui->lstScenes->item(i)->setForeground(Qt::yellow);
                    }
                }
            }
        }

        //Dedicated update-type parsing:
        //==============================
        jkey ="update-type";
        if (jobj.contains(jkey)) {
            jstr = jobj[jkey].toString();

            if(jstr=="Heartbeat") {
                if (jobj["pulse"].toBool() == true) ui->lblPulse->setStyleSheet("QLabel {background-color: Gray; color: LightGray; font: 10pt; border-radius: 10px;}");
                else                                ui->lblPulse->setStyleSheet("QLabel {background-color: LightGray; color: Gray; font: 16pt; border-radius: 10px;}");
                if (jobj["streaming"].toBool() == true) {
                    ui->lblTimeStream->setText("time: " + Seconds2TimeStamp(jobj["total-stream-time"].toInt()));
                    ui->lblBytesStream->setText("Mbytes: " + QString::number(jobj["total-stream-bytes"].toInt()/1048576));
                }
                if (jobj["recording"].toBool() == true) {
                    ui->lblTimeRecord->setText("time: " + Seconds2TimeStamp(jobj["total-record-time"].toInt()));
                    ui->lblFramesRecord->setText("frames: " + QString::number(jobj["total-record-frames"].toInt()));
                    ui->lblBytesRecord->setText("Mbytes: " + QString::number(jobj["total-record-bytes"].toInt()/1048576));
                }
            } else if(jstr=="StreamStatus") {
                ui->lblKbps->setText("kbps: " + QString::number(jobj["kbits-per-sec"].toInt()));
                ui->lblFps->setText("fps: " + QString::number(jobj["fps"].toInt()));
                ui->lblFrames->setText("frames: " + QString::number(jobj["num-total-frames"].toInt()));
                ui->lblDrops->setText("drops: " + QString::number(jobj["num-dropped-frames"].toInt()) + " (" + QString::number(jobj["strain"].toInt()) + "%)");
                ui->lblTimeStream->setText("time: " + jobj["stream-timecode"].toString());
            }else if(jstr=="RecordingStarted") {
                ORC_Record=true;
                ui->btnRecord->setStyleSheet("background-color:red; color: #ffff00; border: 2px solid gray; border-radius: 10px;");
                ui->btnRecord->setText("RECORDING");
                log("#" + QDateTime::currentDateTime().toString("HH':'mm':'ss': '") + "RECORDING.");
            }else if(jstr=="RecordingStopped") {
                ORC_Record=false;
                ui->btnRecord->setStyleSheet("background-color:#800000; color:black; border: 2px solid gray; border-radius: 10px;");//Dark Red
                ui->btnRecord->setText("record off");
                log("#" + QDateTime::currentDateTime().toString("HH':'mm':'ss': '") + "record off. "+ ui->lblTimeRecord->text() + ".");
            }else if(jstr=="StreamStarted") {
                ORC_Stream=true;
                ui->btnStream->setStyleSheet("background-color:green; color: #ffff00; border: 2px solid gray; border-radius: 10px;");
                ui->btnStream->setText("STREAMING");
                log("#" + QDateTime::currentDateTime().toString("HH':'mm':'ss': '") + "STREAMING.");
            }else if(jstr=="StreamStopped") {
                ORC_Stream=false;
                ui->btnStream->setStyleSheet("background-color:#006600; color:black; border: 2px solid gray; border-radius: 10px;");//Dark Green
                ui->btnStream->setText("stream off");
                log("#" + QDateTime::currentDateTime().toString("HH':'mm':'ss': '") + "stream off. " + ui->lblTimeStream->text() + ".");
            }else if(jstr=="ProfileChanged") {
                ws.sendTextMessage("{\"request-type\":\"GetCurrentProfile\",\"message-id\":\"idGetCurrentProfile\"}");
            }else if(jstr=="SwitchScenes") {
                ws.sendTextMessage("{\"request-type\":\"GetCurrentScene\",\"message-id\":\"idGetCurrentScene\"}");
            }else if(jstr=="SceneItemVisibilityChanged") {
                jstr2 = jobj["item-name"].toString();
                jbool = jobj["item-visible"].toBool();
                for (int i=0; i<ui->lstSources->count(); i++) {
                    jstr3 = ui->lstSources->item(i)->text();
                    if (jstr3 == jstr2) {
                        if(!jbool) {
                            ui->lstSources->item(i)->setBackground(Qt::black);
                            ui->lstSources->item(i)->setForeground(Qt::darkGray);
                        } else {
                            ui->lstSources->item(i)->setBackground(Qt::white);
                            ui->lstSources->item(i)->setForeground(Qt::black);
                            if(ORC_LockSources) ui->lstSources->item(i)->setBackground(Qt::gray);
                        }
                    }
                }
            }
        }

        //Flat key parsing for every received json string:
        //================================================
        jkey ="recording";
        if (jobj.contains(jkey)) {
            jbool = jobj[jkey].toBool();
            if(jbool) {
                ORC_Record=true;
                ui->btnRecord->setStyleSheet("background-color:red; color: #ffff00; border: 2px solid gray; border-radius: 10px;");
                ui->btnRecord->setText("RECORDING");
            } else {
                ORC_Record=false;
                ui->btnRecord->setStyleSheet("background-color:#800000; color:black; border: 2px solid gray; border-radius: 10px;");//Dark Red
                ui->btnRecord->setText("record off");
            }
        }

        jkey ="streaming";
        if (jobj.contains(jkey)) {
            jbool = jobj[jkey].toBool();
            if(jbool) {
                ORC_Stream=true;
                ui->btnStream->setStyleSheet("background-color:green; color: #ffff00; border: 2px solid gray; border-radius: 10px;");
                ui->btnStream->setText("STREAMING");
            } else {
                ORC_Stream=false;
                ui->btnStream->setStyleSheet("background-color:#006600; color:black; border: 2px solid gray; border-radius: 10px;");//Dark Green
                ui->btnStream->setText("stream off");
            }
        }
    }
}

void wMain::debug(QString txt) {
    if (ORC_Debug) wD->text(txt);
}

void wMain::log(QString str) {
    ui->txtLog->append(str);
    QTextCursor c =  ui->txtLog->textCursor();
    c.movePosition(QTextCursor::End);
    ui->txtLog->setTextCursor(c);
}

bool wMain::eventFilter(QObject *obj, QEvent *event) {
    QMessageBox::StandardButton reply(QMessageBox::No);
    QPropertyAnimation *animation;

    if (obj == ui->btnRecord) {
        if (event->type() == QEvent::MouseButtonDblClick) {
            debug("::eventFilter btnRecord entered\n");
            animation = new QPropertyAnimation(ui->btnRecord, "geometry");
            animation->setDuration(500);
            animation->setStartValue(QRect(235, 30, 140, 20));
            animation->setEndValue(QRect(220, 20, 171, 41));
            animation->setEasingCurve(QEasingCurve::OutBounce);
            animation->start();

            if (ORC_Record) reply = msgBox.question(this,"ORC Recording","\nAre you sure you want to STOP recording?\n",QMessageBox::Yes | QMessageBox::No);
            else            reply = msgBox.question(this,"ORC Recording","\nAre you sure you want to START recording?\n",QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::Yes) ws.sendTextMessage("{\"request-type\":\"StartStopRecording\",\"message-id\":\"ToggleRecord\"}");
            return true;
        } else {
            return false;
        }
    } else if (obj == ui->btnStream) {
        if (event->type() == QEvent::MouseButtonDblClick) {
            debug("::eventFilter btnStream entered\n");
            animation = new QPropertyAnimation(ui->btnStream, "geometry");
            animation->setDuration(500);
            animation->setStartValue(QRect(35, 30, 140, 20));
            animation->setEndValue(QRect(20, 20, 171, 41));
            animation->setEasingCurve(QEasingCurve::OutBounce);
            animation->start();

            if (ORC_Stream) reply = msgBox.question(this,"ORC Streaming","\nAre you sure you want to STOP streaming?\n",QMessageBox::Yes | QMessageBox::No);
            else            reply = msgBox.question(this,"ORC Streaming","\nAre you sure you want to START streaming?\n",QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::Yes) ws.sendTextMessage("{\"request-type\":\"StartStopStreaming\",\"message-id\":\"ToggleStream\"}");
            return true;
        } else {
            return false;
        }
    } else {
        // pass the event on to the parent class
        return QMainWindow::eventFilter(obj, event);
    }
}

void wMain::on_lstProfiles_itemDoubleClicked(QListWidgetItem *item) {
    debug("::on_lstProfiles_itemDoubleClicked().\n");
    log("#" + QDateTime::currentDateTime().toString("HH':'mm':'ss': '") + "Profile selected <" + item->text() + ">.");
    ws.sendTextMessage("{\"request-type\":\"SetCurrentProfile\",\"message-id\":\"SetProfile\",\"profile-name\":\"" + item->text() + "\" }");
    ws.sendTextMessage("{\"request-type\":\"GetCurrentProfile\",\"message-id\":\"idGetCurrentProfile\"}");
}

void wMain::on_lstScenes_itemDoubleClicked(QListWidgetItem *item) {
    debug("::on_lstScenes_itemDoubleClicked().\n");
    log("#" + QDateTime::currentDateTime().toString("HH':'mm':'ss': '") + "Scene selected <" + item->text() + ">.");
    ws.sendTextMessage("{\"request-type\":\"SetCurrentScene\",\"message-id\":\"idSetScenes\",\"scene-name\":\"" + item->text() + "\" }");
    ws.sendTextMessage("{\"request-type\":\"GetCurrentScene\",\"message-id\":\"idGetCurrentScene\"}");
}

void wMain::on_lstSources_itemDoubleClicked(QListWidgetItem *item) {
    debug("::on_lstSources_itemDoubleClicked().\n");
    if(item->backgroundColor()==Qt::white) {
        ws.sendTextMessage("{\"request-type\":\"SetSourceRender\",\"message-id\":\"SetRender\",\"render\":false,\"source\":\"" + item->text() + "\" }");
        ws.sendTextMessage("{\"request-type\":\"GetCurrentScene\",\"message-id\":\"idGetCurrentScene\"}");
        log("#" + QDateTime::currentDateTime().toString("HH':'mm':'ss': '") + "Source set invisible <" + item->text() + ">.");
    } else {
        ws.sendTextMessage("{\"request-type\":\"SetSourceRender\",\"message-id\":\"SetRender\",\"render\":true,\"source\":\"" + item->text() + "\" }");
        ws.sendTextMessage("{\"request-type\":\"GetCurrentScene\",\"message-id\":\"idGetCurrentScene\"}");
        log("#" + QDateTime::currentDateTime().toString("HH':'mm':'ss': '") + "Source set visible <" + item->text() + ">.");
    }
}


void wMain::onIconTime() {
    //Change state every pulse of the timer:
    ORC_IconChange = !ORC_IconChange;

    //Select the icon to be displayed:
    if (ORC_IconChange) {
        if (ORC_Record && ORC_Stream) {
            QApplication::setWindowIcon(QIcon(":/obs-both.png"));
        } else {
            if (ORC_Record) QApplication::setWindowIcon(QIcon(":/obs-record.png"));
            if (ORC_Stream) QApplication::setWindowIcon(QIcon(":/obs-stream.png"));
        }
    } else {
        QApplication::setWindowIcon(QIcon(":/obs-idle.png"));
    }
}


void logFile(QString *logData, QString logPath, QString logFirst, QString logExtCur, QString logExtBak, int logSize) {

    QString FileNameCur(logPath + "/" + logFirst + "." + logExtCur);
    QString FileNameBak(logPath + "/" + logFirst + "." + logExtBak);
    QFile *fileCur = new QFile(FileNameCur);
    QFile *fileBak = new QFile(FileNameBak);

    if(fileCur->size()>logSize) {
        if(QFileInfo::exists(FileNameBak)) fileBak->remove();
        fileCur->rename(FileNameBak);
        fileCur = new QFile(FileNameCur);
    }

    QTextStream *stream;
    if (fileCur->open(QFile::WriteOnly|QFile::Append|QFile::Text)) {
        stream = new QTextStream(fileCur);
        *stream << *logData << endl;
        fileCur->close();
    }
}

QString Seconds2TimeStamp(int iSeconds){
    QString qsHours = "";
    QString qsMinutes = "";
    QString qsSeconds = "";
    int iHours =0;
    int iMinutes =0;
    int iRemainder =0;

    iMinutes = iSeconds / 60;
    iRemainder = iSeconds % 60;
    qsSeconds= QString::number(iRemainder).rightJustified(2, '0');

    iHours = iMinutes / 60;
    iRemainder = iMinutes % 60;
    qsMinutes= QString::number(iRemainder).rightJustified(2, '0');
    qsHours= QString::number(iHours).rightJustified(2, '0');

    return (qsHours + ":" + qsMinutes + ":" + qsSeconds);
}

