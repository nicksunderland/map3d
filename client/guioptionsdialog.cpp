#include "guioptionsdialog.h"
#include "ui_guioptionsdialog.h"
#include <QDebug>
#include <QMessageBox>
#include "map3d-struct.h"

extern Map3d_Info map3d_info;

GUIoptionsDialog::GUIoptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GUIoptionsDialog)
{
    qDebug()<<"GUIoptionsDialog ctor";
    ui->setupUi(this);
}

GUIoptionsDialog::~GUIoptionsDialog()
{
    qDebug()<<"GUIoptionsDialog dtor";
    delete ui;
}

void GUIoptionsDialog::on_pushButton_clicked()
{
    if(ui->ABIradioButton->isChecked()){
        qDebug()<<"ABI radioButton checked. Set GUIchoice to 'false'.";
        map3d_info.GUIchoice = false;
    }else{
        qDebug()<<"Map3D radioButton checked. Set GUIchoice to 'true'.";
        map3d_info.GUIchoice = true;
    }
    close();
}
