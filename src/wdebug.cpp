#include <QDebug>
#include "wdebug.h"
#include "ui_wdebug.h"

wDebug::wDebug(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::wDebug)
{
    ui->setupUi(this);
}

wDebug::~wDebug(){
    delete ui;
}

void wDebug::text(QString str){
    ui->textEdit->append(str);

    QTextCursor c =  ui->textEdit->textCursor();
    c.movePosition(QTextCursor::End);
    ui->textEdit->setTextCursor(c);
}

QString wDebug::getText(){
    return(ui->textEdit->toPlainText());
}
