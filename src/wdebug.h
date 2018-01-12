#ifndef WDEBUG_H
#define WDEBUG_H

#include <QDialog>

namespace Ui {
class wDebug;
}

class wDebug : public QDialog
{
    Q_OBJECT

public:
    explicit wDebug(QWidget *parent = 0);
    ~wDebug();
    void text(QString str);
    QString getText();

private:
    Ui::wDebug *ui;
};

#endif // WDEBUG_H
