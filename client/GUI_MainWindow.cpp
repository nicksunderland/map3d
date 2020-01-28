#include "GUI_MainWindow.h"
#include "ui_GUI_MainWindow.h"
#include "guioptionsdialog.h"
#include "FileInputScreen.h"
#include "FileDialog.h"


#include "GetMatrixSlice.h"
#include "DrawTransparentPoints.h"

#include "LegendWindow.h"
#include "dialogs.h"
#include "GeomWindow.h"
#include "PickWindow.h"
#include "RescaleColorMap.h"
#include "WindowManager.h"
#include "eventdata.h"
#include "Surf_Data.h"
#include "pickinfo.h"
#include "Map3d_Geom.h"


#include <QKeyEvent>
#include <QMessageBox>
#include <QTest>
#include <QDebug>
#include <QScrollBar>
#include <algorithm>

extern GetMatrixSlice* getmatrixslice;                       //Used this global pointer as it's was already built into Map3d - should remove at somepoint.

/* NOTES:
The calculateforwardsvals function in geomwindow repaint is currently calculating the forward vals on every repaint - ?need to optimise this.
Need to fix the updateMFSresult function in DTP - currently works, but is a mess.
Short out the picking controls - ideally convert to SIGNAL/SLOT


*/



GUI_MainWindow::GUI_MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::GUI_MainWindow)
{
    qDebug()<<"GUI_MainWindow ctor";

    ui->setupUi(this);
    setWindowState(Qt::WindowMaximized);

    FileDialog *fdPtr = new FileDialog;                                          //Only reason to create this dynamically is that Map3d does it dynamically and I want the close function (which uses 'deleteLater()') to work for both ABI and Map3d start ups.
    FileInputScreen fiObject(this, fdPtr);
    fiObject.exec();                                                                        //Here the file info is collected, passed to FileDialog and then executed. During this process a GetMatrixSlice object is made (dynmaically) in FileDialog on_applyButton_clicked
    ui->verticalLayout_16->insertWidget(2, getmatrixslice);        //...and we set it into the GUI here.
    dtpPtr = new DrawTransparentPoints;                                    //Here we create a DrawTransparentPoints window and assign it to a the *dtpPtr, through this the GUI will alter the Dtp functions. Created dynamically as needs to be created after meshes load.
    dtpPtr->setParent(this);                                                          //The mainwindow will deal with the memory now.
    dtpPtr->hide();                                                                        //Don't need to see the interface within the mainwindow GUI.

    setUpGUIWindows();                                                               //To not complicate things I just either: connect via SIGNAL/SLOTS, or replicate button presses to mimic normal Map3d function with <QTest>

}
GUI_MainWindow::~GUI_MainWindow()
{
    qDebug()<<"GUI_MainWindow dtor";
    delete ui;
}
void GUI_MainWindow::setUpGUIWindows()
{
    /*****LISTS OF POINTERS TO WINDOWS & MESHES*****/
    for(int i = 0; i<numGeomWindows(); i++){geomWins << GetGeomWindow(i);}
    for(int i = 0;  i<numGeomWindows(); i++){atrialMeshes << geomWins[i]->meshes[1];}
    for(int i = 0;  i<numGeomWindows(); i++){cathMeshes << geomWins[i]->meshes[0];}


    /*****APPEARANCE OF THE WINDOWS*****/
    connect(this,SIGNAL(sendTransparencySignal(int,bool)),dtpPtr,SLOT(makeSurfaceTransparent(int,bool)));               //Connect GUI transparency button to DTP
    QTest::keyClick(ui->Gold_Standard_Window_1,Qt::Key_B);                                                                                          //enable 'global' transparency settings
    ui->transparency_win1_checkBox->setChecked(true);                                                                                                //Make window 1 atria transparent on opening
    connect(this, SIGNAL(sendPointsOnlySignal(int)),dtpPtr,SLOT(makeSurfacePoints(int)));                                            //Connect to DTP points only function
    emit sendPointsOnlySignal(0);                                                                                                                                      //Send signals to make the catheter meshes points only rendering
    emit sendPointsOnlySignal(2);
    emit sendPointsOnlySignal(4);

    QTest::keyClick(ui->Gold_Standard_Window_1,Qt::Key_C);                                                                                          //remove contours from all windows
    QTest::keyClick(ui->Gold_Standard_Window_1,Qt::Key_M);                                                                                          //remove mesh from all windows
    QTest::keyClick(ui->Gold_Standard_Window_1,Qt::Key_Down);                                                                                   //unlock mesh surfaces from each other (can direct at any window)
    QTest::keyClick(ui->Gold_Standard_Window_1,Qt::Key_M);                                                                                          //change surface rendering in window 1 to mesh
    QTest::keyClick(ui->Gold_Standard_Window_1,Qt::Key_M);
    QTest::keyClick(ui->Gold_Standard_Window_1,Qt::Key_M);
    QTest::keyClick(ui->Gold_Standard_Window_1,Qt::Key_Up);                                                                                        //relock everything

    LegendWindow *lpriv = GetGeomWindow(0)->meshes[1]->legendwin;                                                                      //Get legend window for window 1 and place it in vertical layout 20 in the GUI
    lpriv->setMinimumWidth(120);
    ui->verticalLayout_20->addWidget(lpriv);

    for(int i = 0; i<geomWins.size(); i++){                                                                                                                      //Cycle through the window and alter the menu
            geomWins[i]->HandleMenu(window_locks);                                                                                                       //Turn the window locks off
            geomWins[i]->HandleMenu(graphics_light_front);                                                                                             //Lighting set from the front
            geomWins[i]->HandleMenu(window_attr_info_off);                                                                                            //Window titles and info off
            atrialMeshes[i]->meshcolor[0] = 0.498039;                                                                                                      //Mesh color to purple, although will only be showing in window 1
            atrialMeshes[i]->meshcolor[1] = 0;
            atrialMeshes[i]->meshcolor[2] = 0.498039;
            atrialMeshes[i]->meshcolor[3] = 0;
        }


    /*****BUTTONS*****/
    for(int i = 0; i<geomWins.size(); i++){
        connect(this, SIGNAL(switchBackground(bool)),geomWins[i],SLOT(switchGradientBackground(bool)));                  //Connect the switchBackground signal to al the geom windows.
        connect(geomWins[i], SIGNAL(sendPicktoGUI(PickInfo*)),this,SLOT(receivePick(PickInfo*)));                                   //Connect each geomwindow so that we can receive the information about the pick here
        connect(this, SIGNAL(sendNodeLockState(bool)),geomWins[i],SLOT(setNodePickLock(bool)));                                //Connect each geomwindow so we can control the node picking with the GUI checkBox
        connect(this,SIGNAL(sendCathTransLockState(bool)),geomWins[i],SLOT(setCathTransLock(bool)));                       //Connect each geomwindow so we can control the catheter translation with the GUI checkBox
        connect(this,SIGNAL(sendCathRotationLockState(bool)),geomWins[i],SLOT(setCathRotationLock(bool)));              //Connect each geomwindow so we can control the catheter rotation with the GUI checkBox
        connect(this,SIGNAL(sendCathWinLockState(bool)),geomWins[i],SLOT(setCathWinLock(bool)));                            //Connect the catheter movement isolation buttons to their respective windows
    }
    emit switchBackground(true);                                                                                                                                       //Start backgrounds off as gradient

    connect(ui->MFSUpdate_pushButton,SIGNAL(clicked()),this,SLOT(slotStart()));                                                           //Connect the update MFS button to the timer for the progressBar
    connect(&timer, SIGNAL(timeout()),this,SLOT(slotTimeout()));

    connect(this,SIGNAL(minValueChanged(float)),&rescaleObject,SLOT(setMinValue(float)));                                        //Connect the rescaling object to the GUI control for the legend scale
    connect(this,SIGNAL(maxValueChanged(float)),&rescaleObject,SLOT(setMaxValue(float)));
    connect(this,SIGNAL(updateScale()),&rescaleObject,SLOT(on_ApplyScaleButton_clicked()));
    ui->scaleMax_spinBox->setValue((float)10);
    ui->scaleMin_spinBox->setValue((float)-10);

    ui->plusZoom_toolButton->setAutoRepeat(true);                                                                                                       //set buttons that need button hold-press functionality to 'autorepeat'
    ui->minusZoom_toolButton->setAutoRepeat(true);
    ui->plusZoom_toolButton_2->setAutoRepeat(true);
    ui->minusZoom_toolButton_2->setAutoRepeat(true);
    ui->plusZoom_toolButton_3->setAutoRepeat(true);
    ui->minusZoom_toolButton_3->setAutoRepeat(true);
    ui->closeElectrograms_button->setAutoRepeat(true);
    ui->catheterLock_win1->setEnabled(false);                                                                                                                //Buttons that start disabled
    ui->catheterLock_win2->setEnabled(false);
    ui->catheterLock_win3->setEnabled(false);

    connect(this, SIGNAL(updateMFSresult()),dtpPtr,SLOT(updateMFSresult()));                                                              //Connect the MFS button to the forward and MFS buttons in DTPs
    connect(this, SIGNAL(cathSizeChanged(int,float)),dtpPtr,SLOT(cathSizeChange(int,float)));                                      //Connect cath size spin boxes to the size change functions in DTPs
    connect(this, SIGNAL(sendPhaseAnalysisState(bool)),dtpPtr,SLOT(makePhaseAnalysis(bool)));                                //Connect phase analysis checkBox to the phase analysis boxes of the DTP object
    connect(this, SIGNAL(askForAnalysis(int)),dtpPtr,SLOT(callForAnalysisFromGUI(int)));                                              //Connect the analysis button to the function that generates the activation, CC and RMSE windows.
    connect(this, SIGNAL(updateStatsBoxes()),this,SLOT(on_comboBox_currentIndexChanged()));                                 //Connect this GUI to itself to order updates on the stats boxes
    connect(this, SIGNAL(updatePickWindows()), this, SLOT(on_goldStandElectrogram_checkBox_stateChanged()));     //Connect this GUI to itself to update the pick objects with the show electrograms settings
    connect(this, SIGNAL(updatePickWindows()), this, SLOT(on_sixtyFourElectrogram_checkBox_stateChanged()));      // - need to change this to passing the states in the constructor of the Pick, but can't how at the min, also can't seem to connect SIGNALS/SLOTS here (no Q_OBJECT)
    connect(this, SIGNAL(updatePickWindows()), this, SLOT(on_ninetySixElectrogram_checkBox_stateChanged()));

    connect(dtpPtr,SIGNAL(averageCCvalue(double,int)),this,SLOT(receiveCCaverage(double,int)));
    connect(dtpPtr,SIGNAL(averageNRMSEvalue(double,int)),this,SLOT(receiveNRMSEaverage(double,int)));


    /*****VARIABLES*****/
    NRMSEwin2 = 0;
    NRMSEwin3 = 0;
    CCwin2 = 0;
    CCwin3 = 0;
    storeMaxScale = 10;
    storeMinScale = -10;
}
void GUI_MainWindow::on_actionE_xit_triggered()
{
    qApp->exit();
}
void GUI_MainWindow::on_MFSUpdate_pushButton_clicked()
{
    emit updateMFSresult();
    Broadcast(MAP3D_REPAINT_ALL);
}
void GUI_MainWindow::slotStart()
{
    ui->progressBar->setValue(0);
    timer.start(1);
    ui->MFSUpdate_pushButton->setText("Calculating...");
}
void GUI_MainWindow::slotTimeout()
{
    int p = ui->progressBar->value();
#if 100
    if(p == 100){
        timer.stop();
     ui->MFSUpdate_pushButton->setText("Update Inverse Solution");
     ui->progressBar->setValue(0);
     return;
    }
#endif
    ui->progressBar->setValue(++p);
}
void GUI_MainWindow::on_reset_button_clicked()
{
    QTest::keyClick(ui->Gold_Standard_Window_1,Qt::Key_R);
}
void GUI_MainWindow::receivePick(PickInfo *pick)
{
    qDebug()<<"GUI_MainWindow::receivePick(PickInfo *pick)";

    Mesh_Info* mesh = atrialMeshes[2];                                                                                                                          //Only want one pickwindow per pick, I have chosen to use window 3's as this will always have an inverse solution and we want to display the inverse electrogram
    PickWindow *ppriv = PickWindow::PickWindowCreate(-1, -1, -1, -1);                                                                        //Create new window - Map3D does so dynamically so we'll have to delete later (see close electrogram button).
    if (!ppriv) return; // can fail if more than MAX_PICKS
    pick->show = 1;                                                                                                                                                         //Associate the pickwindow's variables with those from the PickInfo object
    ppriv->pick = pick;
    ppriv->mesh = mesh;                                                                                                                                                 //Here is where we tell the pickwindow to use the data from window 3 / atrialMeshes[2]
    pick->pickwin = ppriv;
    ppriv->setFixedHeight(150);                                                                                                                                       //Set fixed height, otherwise we can't see it in the display
    ui->verticalLayout_4->insertWidget(0, ppriv);                                                                                                            //Insert into the scroll area

    for(int loop = 0; loop<atrialMeshes.size(); loop++){                                                                                                 //As we want all the meshes to display the picked node we have to pass a pointer to each of their pickstack[ ]s
        Mesh_Info *mesh = atrialMeshes[loop];
        mesh->pickstacktop++;                                                                                                                                        //Increase pickstack array size
        mesh->pickstack[mesh->pickstacktop] = pick;                                                                                                     //Add the pointer to the PickInfo object (which contains the correct node to display) to each meshes pickstack
    }

    if(ui->comboBox->currentIndex() == 1){                                                                                                                 //Update the stats boxes if we are showing the picked nodes average
        emit updateStatsBoxes();
    }
    emit updatePickWindows();                                                                                                                                        //As I can't seem to connect PickWindow and this GUI with SIGNAL/SLOTS I've done it with calling a function (see the checkBox methods), we then call this update when we make a new PickWindow.
}
void GUI_MainWindow::on_closeElectrograms_button_clicked()
{
    if(atrialMeshes[2]->pickstacktop >= 0){                                                                                                                  //Enter only if the pickstack has something in (I've used atrialMeshes[2] here but all of the pickstacks should be of the same size)
        Mesh_Info *mesh2 = atrialMeshes[2];
        delete mesh2->pickstack[mesh2->pickstacktop]->pickwin;                                                                                //Since we only made one pickwindow for window 3 / atrialMeshes[2] we only want to delete the one pickwindow
        mesh2->pickstack[mesh2->pickstacktop]->pickwin = nullptr;
        delete mesh2->pickstack[mesh2->pickstacktop];                                                                                               //Again since we only made one object dynamically and passed pointers to it to the other windows, we only need to delete the one pickInfo (I've used atrialMeshes'[2] pointer but you could use any of them
        for(int loop = 0; loop<atrialMeshes.size(); loop++){                                                                                           //But since we did increase all meshes pickstacktop size and add a pointer to their pickstacks[ ] we need to loop here to...
           Mesh_Info *mesh = atrialMeshes[loop];
           mesh->pickstack[mesh2->pickstacktop] = nullptr;                                                                                          //...set the pointers to null
           mesh->pickstacktop--;                                                                                                                                      //...and reduce the pickstacktop array size
        }
        Broadcast(MAP3D_UPDATE);
    }

    if(ui->comboBox->currentIndex() == 1){                                                                                                               //Update the stats boxes if we are showing the picked nodes average
        emit updateStatsBoxes();
    }
}
void GUI_MainWindow::on_nodePicking_checkBox_stateChanged()
{
    if(ui->nodePicking_checkBox->isChecked()){
        ui->cathTranslation_checkBox->setCheckState(Qt::Unchecked);
        ui->cathTranslation_checkBox->setEnabled(false);
        ui->cathRotation_checkBox->setCheckState(Qt::Unchecked);
        ui->cathRotation_checkBox->setEnabled(false);
        emit sendNodeLockState(true);
    }else{
        ui->cathTranslation_checkBox->setCheckState(Qt::Unchecked);
        ui->cathTranslation_checkBox->setEnabled(true);
        ui->cathRotation_checkBox->setCheckState(Qt::Unchecked);
        ui->cathRotation_checkBox->setEnabled(true);
        emit sendNodeLockState(false);
    }
}
void GUI_MainWindow::on_comboBox_currentIndexChanged()
{
    if(ui->comboBox->currentIndex() == 0)
    {
        ui->label_14->setText(QString::number(CCwin3));
        ui->label_15->setText(QString::number(CCwin2));
        ui->label_12->setText(QString::number(NRMSEwin3));
        ui->label_13->setText(QString::number(NRMSEwin2));
    }

    if(ui->comboBox->currentIndex() == 1)
    {
        ui->label_14->setText(CCPickAverage(2));
        ui->label_15->setText(CCPickAverage(1));
        ui->label_12->setText(NRMSEPickAverage(2));
        ui->label_13->setText(NRMSEPickAverage(1));
    }
}
void GUI_MainWindow::on_cathSize_SpinBox_valueChanged()
{
    float size = ui->cathSize_SpinBox->value();
    emit cathSizeChanged(0,size);
    emit cathSizeChanged(2,size);
    emit cathSizeChanged(4,size);
}
void GUI_MainWindow::receiveCCaverage(double averageCC, int window)
{
    if(window == 2){
        CCwin2 = averageCC;
        if(ui->comboBox->currentIndex() == 0){
            ui->label_15->setText(QString::number(averageCC));
        }
    }
    if(window == 3){
        CCwin3 = averageCC;
        if(ui->comboBox->currentIndex() == 0){
            ui->label_14->setText(QString::number(averageCC));
        }
    }
}
void GUI_MainWindow::receiveNRMSEaverage(double averageNRMSE, int window)
{
    if(window == 2){
        NRMSEwin2 = averageNRMSE;
        if(ui->comboBox->currentIndex() == 0){
            ui->label_13->setText(QString::number(averageNRMSE));
        }
    }
    if(window == 3){
        NRMSEwin3 = averageNRMSE;
        if(ui->comboBox->currentIndex() == 0){
            ui->label_12->setText(QString::number(averageNRMSE));
        }
    }
}
QString GUI_MainWindow::CCPickAverage(int atrialMeshIndex)
{
    double averagePickCCvalues = 0;
    int numPickWins = atrialMeshes[atrialMeshIndex]->pickstacktop + 1;
    if(numPickWins > 0 && atrialMeshes[atrialMeshIndex]->data->CCvals != 0){
        double sumPickCCvalues = 0;
        for(int loop = 0; loop<numPickWins; loop++){
            sumPickCCvalues += atrialMeshes[atrialMeshIndex]->data->CCvals[atrialMeshes[atrialMeshIndex]->pickstack[loop]->node];
        }
        averagePickCCvalues = sumPickCCvalues / numPickWins;
    }
    return QString::number(averagePickCCvalues);
}
QString GUI_MainWindow::NRMSEPickAverage(int atrialMeshIndex)
{
    double averagePickNRMSEvalues = 0;
    int numPickWins = atrialMeshes[atrialMeshIndex]->pickstacktop + 1;
    if(numPickWins > 0 && atrialMeshes[atrialMeshIndex]->data->RMSEvals != 0){
        double sumPickNRMSEvalues = 0;
        for(int loop = 0; loop<numPickWins; loop++){
            sumPickNRMSEvalues += atrialMeshes[atrialMeshIndex]->data->RMSEvals[atrialMeshes[atrialMeshIndex]->pickstack[loop]->node];
        }
        averagePickNRMSEvalues = sumPickNRMSEvalues / numPickWins;
    }
    return QString::number(averagePickNRMSEvalues);
}
void GUI_MainWindow::on_plainBG_radioButton_clicked()
{
    emit switchBackground(false);
    Broadcast(MAP3D_UPDATE);
}
void GUI_MainWindow::on_gradBG_radioButton_clicked()
{
    emit switchBackground(true);
    Broadcast(MAP3D_UPDATE);
}
void GUI_MainWindow::on_sixtyFourElectrogram_checkBox_stateChanged()
{
    bool checkState;
    if(ui->sixtyFourElectrogram_checkBox->isChecked()){
        checkState = true;
    }else{
        checkState = false;
    }
    for(int loop = 0; loop<=atrialMeshes[2]->pickstacktop; loop++){
        atrialMeshes[2]->pickstack[loop]->pickwin->switchWindowElectrogram(checkState,3);
    }
    Broadcast(MAP3D_REPAINT_ALL);;
}
void GUI_MainWindow::on_ninetySixElectrogram_checkBox_stateChanged()
{
    bool checkState;
    if(ui->ninetySixElectrogram_checkBox->isChecked()){
        checkState = true;
    }else{
        checkState = false;
    }
    for(int loop = 0; loop<=atrialMeshes[2]->pickstacktop; loop++){
        atrialMeshes[2]->pickstack[loop]->pickwin->switchWindowElectrogram(checkState,2);
    }
    Broadcast(MAP3D_REPAINT_ALL);
}
void GUI_MainWindow::on_goldStandElectrogram_checkBox_stateChanged()
{
    bool checkState;
    if(ui->goldStandElectrogram_checkBox->isChecked()){
        checkState = true;
    }else{
        checkState = false;
    }
    for(int loop = 0; loop<=atrialMeshes[2]->pickstacktop; loop++){
        atrialMeshes[2]->pickstack[loop]->pickwin->switchWindowElectrogram(checkState,1);
    }
    Broadcast(MAP3D_REPAINT_ALL);
}
void GUI_MainWindow::on_plusZoom_toolButton_2_clicked()
{
    QTest::keyClick(ui->Gold_Standard_Window_1,Qt::Key_End, Qt::ControlModifier);
}
void GUI_MainWindow::on_minusZoom_toolButton_2_clicked()
{
    QTest::keyClick(ui->Gold_Standard_Window_1,Qt::Key_PageDown, Qt::ControlModifier);
}
void GUI_MainWindow::on_plusZoom_toolButton_3_clicked()
{
        QTest::keyClick(ui->Gold_Standard_Window_2,Qt::Key_End, Qt::ControlModifier);
}
void GUI_MainWindow::on_minusZoom_toolButton_3_clicked()
{
    QTest::keyClick(ui->Gold_Standard_Window_2,Qt::Key_PageDown, Qt::ControlModifier);
}
void GUI_MainWindow::on_plusZoom_toolButton_pressed()
{
    QTest::keyClick(ui->Inverse_Solution_Window_2,Qt::Key_End, Qt::ControlModifier);
}
void GUI_MainWindow::on_minusZoom_toolButton_pressed()
{
    QTest::keyClick(ui->Inverse_Solution_Window_2,Qt::Key_PageDown, Qt::ControlModifier);
}
void GUI_MainWindow::on_transparency_win1_checkBox_stateChanged()
{
    bool checkState;
    if(ui->transparency_win1_checkBox->isChecked()){checkState = true;}else{checkState = false;}
    emit sendTransparencySignal(1,checkState);
}
void GUI_MainWindow::on_transparency_win2_checkBox_stateChanged()
{
    bool checkState;
    if(ui->transparency_win2_checkBox->isChecked()){checkState = true;}else{checkState = false;}
    emit sendTransparencySignal(3,checkState);
}
void GUI_MainWindow::on_transparency_win3_checkBox_stateChanged()
{
    bool checkState;
    if(ui->transparency_win3_checkBox->isChecked()){checkState = true;}else{checkState = false;}
    emit sendTransparencySignal(5,checkState);
}
void GUI_MainWindow::on_scaleMin_spinBox_valueChanged()
{
    emit minValueChanged(ui->scaleMin_spinBox->value());
    emit updateScale();
}
void GUI_MainWindow::on_scaleMax_spinBox_valueChanged()
{
    emit maxValueChanged(ui->scaleMax_spinBox->value());
    emit updateScale();
}
void GUI_MainWindow::on_cathTranslation_checkBox_stateChanged()
{
    if(ui->cathTranslation_checkBox->isChecked()){
        QTest::keyClick(ui->Gold_Standard_Window_1,Qt::Key_T);
        emit sendCathTransLockState(true);
        ui->catheterLock_win1->setCheckState(Qt::Unchecked);
        ui->catheterLock_win2->setCheckState(Qt::Unchecked);
        ui->catheterLock_win3->setCheckState(Qt::Unchecked);
        ui->catheterLock_win1->setEnabled(true);
        ui->catheterLock_win2->setEnabled(true);
        ui->catheterLock_win3->setEnabled(true);
        ui->cathRotation_checkBox->setCheckState(Qt::Unchecked);
        ui->cathRotation_checkBox->setEnabled(false);
        ui->nodePicking_checkBox->setCheckState(Qt::Unchecked);
        ui->nodePicking_checkBox->setEnabled(false);

    }else{
        QTest::keyClick(ui->Gold_Standard_Window_1,Qt::Key_T);
        emit sendCathTransLockState(false);
        ui->catheterLock_win1->setCheckState(Qt::Unchecked);
        ui->catheterLock_win2->setCheckState(Qt::Unchecked);
        ui->catheterLock_win3->setCheckState(Qt::Unchecked);
        ui->catheterLock_win1->setEnabled(false);
        ui->catheterLock_win2->setEnabled(false);
        ui->catheterLock_win3->setEnabled(false);
        ui->cathRotation_checkBox->setEnabled(true);
        ui->nodePicking_checkBox->setCheckState(Qt::Unchecked);
        ui->nodePicking_checkBox->setEnabled(true);
    }
}

void GUI_MainWindow::on_catheterLock_win1_stateChanged()
{
    if(ui->catheterLock_win1->isChecked()){
        emit sendCathWinLockState(true);
        ui->catheterLock_win2->setCheckState(Qt::Checked);
        ui->catheterLock_win3->setCheckState(Qt::Checked);
    }else{
        emit sendCathWinLockState(false);
        ui->catheterLock_win2->setCheckState(Qt::Unchecked);
        ui->catheterLock_win3->setCheckState(Qt::Unchecked);
    }
}

void GUI_MainWindow::on_catheterLock_win2_stateChanged()
{
    if(ui->catheterLock_win2->isChecked()){
        ui->catheterLock_win1->setCheckState(Qt::Checked);
        ui->catheterLock_win3->setCheckState(Qt::Checked);
    }else{
        ui->catheterLock_win1->setCheckState(Qt::Unchecked);
        ui->catheterLock_win3->setCheckState(Qt::Unchecked);
    }
}

void GUI_MainWindow::on_catheterLock_win3_stateChanged()
{
    if(ui->catheterLock_win3->isChecked()){
        ui->catheterLock_win2->setCheckState(Qt::Checked);
        ui->catheterLock_win1->setCheckState(Qt::Checked);
    }else{
        ui->catheterLock_win2->setCheckState(Qt::Unchecked);
        ui->catheterLock_win1->setCheckState(Qt::Unchecked);
    }
}

void GUI_MainWindow::on_cathRotation_checkBox_stateChanged()
{
    if(ui->cathRotation_checkBox->isChecked()){
        ui->catheterLock_win1->setCheckState(Qt::Unchecked);
        ui->catheterLock_win2->setCheckState(Qt::Unchecked);
        ui->catheterLock_win3->setCheckState(Qt::Unchecked);
        ui->catheterLock_win1->setEnabled(true);
        ui->catheterLock_win2->setEnabled(true);
        ui->catheterLock_win3->setEnabled(true);
        QTest::keyClick(ui->Gold_Standard_Window_1,Qt::Key_T);
        emit sendCathRotationLockState(true);
        ui->cathTranslation_checkBox->setCheckState(Qt::Unchecked);
        ui->cathTranslation_checkBox->setEnabled(false);
        ui->nodePicking_checkBox->setCheckState(Qt::Unchecked);
        ui->nodePicking_checkBox->setEnabled(false);

    }else{
        ui->catheterLock_win1->setCheckState(Qt::Unchecked);
        ui->catheterLock_win2->setCheckState(Qt::Unchecked);
        ui->catheterLock_win3->setCheckState(Qt::Unchecked);
        ui->catheterLock_win1->setEnabled(false);
        ui->catheterLock_win2->setEnabled(false);
        ui->catheterLock_win3->setEnabled(false);
        QTest::keyClick(ui->Gold_Standard_Window_1,Qt::Key_T);
        emit sendCathRotationLockState(false);
        ui->cathTranslation_checkBox->setEnabled(true);
        ui->nodePicking_checkBox->setCheckState(Qt::Unchecked);
        ui->nodePicking_checkBox->setEnabled(true);
    }
}

void GUI_MainWindow::on_phaseAnlysis_checkBox_stateChanged()
{
    if(ui->phaseAnlysis_checkBox->isChecked()){
        storeMaxScale = ui->scaleMax_spinBox->value();
        storeMinScale = ui->scaleMin_spinBox->value();
        emit sendPhaseAnalysisState(true);
        ui->scaleMax_spinBox->setValue(3.14);
        ui->scaleMin_spinBox->setValue(-3.14);

        ui->goldStandElectrogram_checkBox->setCheckState(Qt::Unchecked);
        ui->sixtyFourElectrogram_checkBox->setCheckState(Qt::Unchecked);
        ui->ninetySixElectrogram_checkBox->setCheckState(Qt::Unchecked);

    }else{
        emit sendPhaseAnalysisState(false);
        ui->scaleMax_spinBox->setValue(storeMaxScale);
        ui->scaleMin_spinBox->setValue(storeMinScale);

        ui->goldStandElectrogram_checkBox->setCheckState(Qt::Checked);
        ui->sixtyFourElectrogram_checkBox->setCheckState(Qt::Checked);
        ui->ninetySixElectrogram_checkBox->setCheckState(Qt::Checked);
    }
}

void GUI_MainWindow::on_analysis_pushButton_clicked()
{
    emit askForAnalysis(1);
    emit askForAnalysis(3);
    emit askForAnalysis(5);

}







