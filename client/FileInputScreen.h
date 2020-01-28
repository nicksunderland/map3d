#ifndef FILEINPUTSCREEN_H
#define FILEINPUTSCREEN_H

#include <QDialog>
#include "FileDialog.h"

namespace Ui {
class FileInputScreen;
}

class FileInputScreen : public QDialog
{
    Q_OBJECT

public:
    explicit FileInputScreen(QWidget *parent = nullptr, FileDialog *fdObject = nullptr);
    ~FileInputScreen();


signals:
    void sendLineEditText(Mesh_Info*, QString, QString, int);
    void applyFileDialog();


private slots:
    void on_loadFiles_pushButton_clicked();
    void on_atrial_geom_toolButton_clicked();
    void on_atrial_data_toolButton_clicked();
    void on_catheter_geom_toolButton_clicked();
    void on_catheter_data_toolButton_clicked();
    void on_additionalCath_checkBox_stateChanged(int arg1);
    void on_catheter_data_toolButton_2_clicked();
    void on_catheter_geom_toolButton_2_triggered(QAction *arg1);

    void setPlaceHolderText();


private:
    Ui::FileInputScreen *ui;
    QString atrialGeomFileName;
    QString atrialDataFileName;
    QString catheterGeomFileName;
    QString catheterDataFileName;
    QString additionalCatheterGeomFileName;
    QString additionalCatheterDataFileName;

    FileDialog *fdPtr;




};

#endif // FILEINPUTSCREEN_H
