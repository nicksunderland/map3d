#ifndef GETMATRIXSLICE_H
#define GETMATRIXSLICE_H
#include "dialogs.h"
#include "ui_GetMatrixSlice.h"
#include "GenericWindow.h"
#include "MeshList.h"
#include <iostream>
#include <GeomWindow.h>
using namespace std;

#include "WindowManager.h"

#include <QTimer>
#include <QList>

class FileDialogWidget;
class Mesh_Info;
class QSpinBox;

class GetMatrixSlice: public QDialog, public Ui::GetMatrixSlice
{
    Q_OBJECT;

public:
    GetMatrixSlice();

    Mesh_List findMeshesFromSameInput(Mesh_Info* mesh);

    Mesh_Info *mesh;
    Mesh_List meshes;

public slots:

    void on_firstIndexButton_clicked();
    void on_previousIndexButton_clicked();
    void on_playButton_clicked();
    void on_pauseButton_clicked();
    void on_nextIndexButton_clicked();
    void on_lastIndexButton_clicked();

    void on_indexSlider_actionTriggered(int action);

    void on_indexSpinBox_valueChanged(int arg1);



private:
  QList<FileDialogWidget*> _widgets;

};


#endif // GETMATRIXSLICE_H
