#include "GetMatrixSlice.h"
#include <stddef.h>
#ifdef _WIN32
#include <windows.h>
#pragma warning(disable:4505)
#undef TRACE
#else
#include <unistd.h>
#endif
#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#include <float.h>
#include <limits.h>
#include <math.h>
#include <string>
#include "dialogs.h"
#include "map3dmath.h"
#include "GeomWindow.h"
#include "Map3d_Geom.h"
#include "Surf_Data.h"
#include "PickWindow.h"
#include "MainWindow.h"
#include "lock.h"
#include "GeomWindowMenu.h"
#include "FileDialog.h"
#include "MeshList.h"
#include "map3d-struct.h"
#include "eventdata.h"


#include <QFile>
#include <QDebug>
#include <QCloseEvent>
#include <QSlider>
#include <QTimer>

extern Map3d_Info map3d_info;
extern MainWindow *masterWindow;
extern FileDialog* filedialog;
extern vector<Surface_Group> surf_group;
extern int fstep, cur;

int sliderindex,indexSpinBox_munualValue;


#include <iostream>


GetMatrixSlice::GetMatrixSlice():
    QDialog(0, Qt::Dialog | Qt::WindowTitleHint)
{
    setupUi(this);

    nextIndexButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaSkipForward));
    //connect(nextIndexButton, SIGNAL(clicked()), this, SLOT(on_nextIndexButton_clicked()));

    previousIndexButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaSkipBackward));
    // connect(previousIndexButton, SIGNAL(clicked()), this, SLOT(on_previousIndexButton_clicked()));

    firstIndexButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaSeekBackward));
    // connect(firstIndexButton, SIGNAL(clicked()), this, SLOT(on_firstIndexButton_clicked()));

    lastIndexButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaSeekForward));
    //connect(lastIndexButton, SIGNAL(clicked()), this, SLOT(on_lastIndexButton_clicked()));

    connect(indexSlider, SIGNAL(actionTriggered()), this,SLOT(on_indexSlider_actionTriggered()));

    // connect(indexSpinBox, SIGNAL(valueChanged()), this, SIGNAL(executeFromStateChangeTriggered()));

    playButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
    //connect(playButton, SIGNAL(clicked()), this, SLOT(on_playButton_clicked()));

    pauseButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPause));
    // connect(pauseButton, SIGNAL(clicked()), this, SLOT(on_pauseButton_clicked()));


    indexSpinBox->setRange(1,map3d_info.maxnumframes);
    indexSlider->setRange(1,map3d_info.maxnumframes);

}


void GetMatrixSlice::on_firstIndexButton_clicked()
{
//    indexSpinBox->setValue(1);
//    indexSlider->setValue(0);
}

void GetMatrixSlice::on_lastIndexButton_clicked()
{

}


void GetMatrixSlice::on_previousIndexButton_clicked()
{

}



void GetMatrixSlice::on_nextIndexButton_clicked()
{


}


void GetMatrixSlice::on_playButton_clicked()
{


}

void GetMatrixSlice::on_pauseButton_clicked()
{

}


void GetMatrixSlice::on_indexSlider_actionTriggered(int action)
{
    sliderindex = indexSlider->value();

}

void GetMatrixSlice::on_indexSpinBox_valueChanged(int arg1)
{

    indexSpinBox_munualValue=indexSpinBox->value();
}

