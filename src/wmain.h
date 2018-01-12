#ifndef WMAIN_H
#define WMAIN_H

#include <QMainWindow>
#include <QtWebSockets/QWebSocket>
#include <QList>
#include <QtWidgets>
#include <QWidgetItem>
#include <QListWidgetItem>
#include "wdebug.h"
//#include <QTimer>

namespace Ui {
class wMain;
}

class wMain : public QMainWindow
{
    Q_OBJECT

public:
    explicit wMain(QWidget *parent = 0);
    ~wMain();
    void closeEvent(QCloseEvent *event);
    void log(QString str);
    bool eventFilter(QObject *obj, QEvent *event);


private Q_SLOTS:
    void onConnected();
    void onClosed();
    void onReceived(QString message);
    void debug(QString DebugText);
    void on_lstProfiles_itemDoubleClicked(QListWidgetItem *item);
    void on_lstScenes_itemDoubleClicked(QListWidgetItem *item);
    void on_lstSources_itemDoubleClicked(QListWidgetItem *item);
    void onIconTime();

private:
    Ui::wMain *ui;
    wDebug *wD;
    QWebSocket ws;
    bool ORC_Connected;
    bool ORC_Stream;
    bool ORC_Record;
    QString ORC_CurrentProfile;
    QString ORC_CurrentScene;
    bool ORC_IconChange;
    QMessageBox msgBox;
    bool ORC_Exiting;
};

#endif // WMAIN_H
