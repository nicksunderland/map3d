#include "FileInputScreen.h"
#include "ui_FileInputScreen.h"
#include <QFileDialog>
#include <QDebug>
#include <QDesktopWidget>


/* This window just gets the file names of the atrial and catheter geometry and data, then passes them to the underlying Map3D
 * filedialog window (not shown in the ABI setup) and enters it in the correct format for the ABI GUI windows. */


FileInputScreen::FileInputScreen(QWidget *parent, FileDialog *fdPtrTransfer) :
    QDialog(parent),
    ui(new Ui::FileInputScreen)
{
    qDebug()<<"FileInputScreen ctor";

    ui->setupUi(this);
    this->move(QApplication::desktop()->screen()->rect().center() - this->rect().center());

    fdPtr = fdPtrTransfer;
    connect(this, SIGNAL(sendLineEditText(Mesh_Info*, QString, QString, int)),fdPtr,SLOT(autoAddRow(Mesh_Info*, QString, QString, int)));
    connect(this, SIGNAL(applyFileDialog()),fdPtr,SLOT(on_applyButton_clicked()));

    setPlaceHolderText();
}

FileInputScreen::~FileInputScreen()
{
    qDebug()<<"FileInputScreen dtor";
    delete ui;
}

void FileInputScreen::on_loadFiles_pushButton_clicked()
{

    //Window 1 or index '0':  "gold standard view"
        emit sendLineEditText(nullptr, catheterGeomFileName, catheterDataFileName, 0);
        emit sendLineEditText(nullptr, atrialGeomFileName, atrialDataFileName, 0);

    //Window 2 or index '1':  "inverse of second catheter, or same as window 3"
    if(ui->additionalCath_checkBox->isChecked()){
        emit sendLineEditText(nullptr, additionalCatheterGeomFileName, additionalCatheterDataFileName, 1);
        emit sendLineEditText(nullptr, atrialGeomFileName, atrialDataFileName, 1);
    }else{
        emit sendLineEditText(nullptr, catheterGeomFileName, catheterDataFileName, 1);
        emit sendLineEditText(nullptr, atrialGeomFileName, atrialDataFileName, 1);
    }

    //Window 3 or index '2': 'inverse of primary catheter"
        emit sendLineEditText(nullptr, catheterGeomFileName, catheterDataFileName, 2);
        emit sendLineEditText(nullptr, atrialGeomFileName, atrialDataFileName, 2);

    emit applyFileDialog();

     close();
}

void FileInputScreen::setPlaceHolderText()
{
    ui->atrial_geom_lineEdit->setPlaceholderText("Select atrial geometry file");
    ui->atrial_data_lineEdit->setPlaceholderText("Select atrial potentials file");
    ui->catheter_geom_lineEdit->setPlaceholderText("Select catheter geometry file");
    ui->catheter_data_lineEdit->setPlaceholderText("Select catheter potentials file");
    ui->catheter_geom_lineEdit_2->setPlaceholderText("Select additional catheter geometry file");
    ui->catheter_data_lineEdit_2->setPlaceholderText("Select additional catheter potentials file");

    ///delete after development///
    atrialGeomFileName = "/hpc/nsun286/Matlab stuff/AtrialMesh_Jichao/Endocardial_Meshes/humanLA_endo_2245pts.mat";     //hpc/nsun286/map3d_new_data/compare_64_96/geom_LA_Datacloud_map3d.mat";
    ui->atrial_geom_lineEdit->setText(atrialGeomFileName);
    atrialDataFileName = "/hpc/nsun286/Matlab stuff/AtrialMesh_Jichao/Endocardial_Meshes/APdata.mat";    //LAdata_datdacloud_rotor_map3d.mat"; //
    ui->atrial_data_lineEdit->setText(atrialDataFileName);
    //64 electrode catheter
    catheterGeomFileName = "/hpc/nsun286/map3d_new_data/compare_64_96/catheter_geo_130_updated_map3d.mat"; //hpc/nsun286/map3d_new_data/compare_64_96/catheter_geo_66_updated_map3d.mat";    //catheter_geo_130_updated_map3d.mat"; //
    ui->catheter_geom_lineEdit->setText(catheterGeomFileName);
    catheterDataFileName = "/hpc/nsun286/map3d_new_data/compare_64_96/catheterdata_130_map3d.mat";  //catheterdata_130_map3d.mat"; //
    ui->catheter_data_lineEdit->setText(catheterDataFileName);
    //96 electrode catheter
    additionalCatheterGeomFileName = "/hpc/nsun286/map3d_new_data/compare_64_96/catheter_geo_96_map3d.mat";
    ui->catheter_geom_lineEdit_2->setText(additionalCatheterGeomFileName);
    additionalCatheterDataFileName = "/hpc/nsun286/map3d_new_data/compare_64_96/catheterdata_96_map3d.mat";
    ui->catheter_data_lineEdit_2->setText(additionalCatheterDataFileName);
    ui->additionalCath_checkBox->setChecked(false);
    ui->catheter_geom_lineEdit_2->setEnabled(false);
    ui->catheter_data_lineEdit_2->setEnabled(false);
    ///////////////////////
}

void FileInputScreen::on_atrial_geom_toolButton_clicked()
{
    atrialGeomFileName = QFileDialog::getOpenFileName(this, "Select atrial geometry file", "/hpc/nsun286/map3d_new_data/compare_64_96");
    ui->atrial_geom_lineEdit->setText(atrialGeomFileName);
}

void FileInputScreen::on_atrial_data_toolButton_clicked()
{
    atrialDataFileName = QFileDialog::getOpenFileName(this, "Select atrial potentials file", "/hpc/nsun286/map3d_new_data/compare_64_96");
    ui->atrial_data_lineEdit->setText(atrialDataFileName);
}

void FileInputScreen::on_catheter_geom_toolButton_clicked()
{
    catheterGeomFileName = QFileDialog::getOpenFileName(this, "Select catheter geometry file", "/hpc/nsun286/map3d_new_data/compare_64_96");
    ui->catheter_geom_lineEdit->setText(catheterGeomFileName);
}

void FileInputScreen::on_catheter_data_toolButton_clicked()
{
    catheterDataFileName = QFileDialog::getOpenFileName(this, "Select catheter potentials file", "/hpc/nsun286/map3d_new_data/compare_64_96");
    ui->catheter_data_lineEdit->setText(catheterDataFileName);
}

void FileInputScreen::on_catheter_data_toolButton_2_clicked()
{
    additionalCatheterGeomFileName = QFileDialog::getOpenFileName(this, "Select additional catheter geometry file", "/hpc/nsun286/map3d_new_data/compare_64_96");
    ui->catheter_geom_lineEdit_2->setText(additionalCatheterGeomFileName);
}

void FileInputScreen::on_catheter_geom_toolButton_2_triggered(QAction *arg1)
{
    additionalCatheterDataFileName = QFileDialog::getOpenFileName(this, "Select additional catheter potentials file", "/hpc/nsun286/map3d_new_data/compare_64_96");
    ui->catheter_data_lineEdit_2->setText(additionalCatheterDataFileName);
}

void FileInputScreen::on_additionalCath_checkBox_stateChanged(int arg1)
{
    if(ui->additionalCath_checkBox->isChecked())
    {
        ui->catheter_geom_lineEdit_2->setEnabled(true);
        ui->catheter_data_lineEdit_2->setEnabled(true);
    }else{
        ui->catheter_geom_lineEdit_2->setEnabled(false);
        ui->catheter_data_lineEdit_2->setEnabled(false);
    }
}

