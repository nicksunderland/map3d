#ifndef GUI_MAINWINDOW_H
#define GUI_MAINWINDOW_H

#include <QMainWindow>
#include "PickWindow.h"
#include <QLayout>
#include <QTimer>
#include <vector>
#include "DrawTransparentPoints.h"
#include "guioptionsdialog.h"
#include "FileInputScreen.h"
#include "FileDialog.h"
#include "RescaleColorMap.h"

namespace Ui {
class GUI_MainWindow;
}

class GUI_MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit GUI_MainWindow(QWidget *parent = nullptr);
    ~GUI_MainWindow();









private slots:
    void on_transparency_win1_checkBox_stateChanged();
    void on_transparency_win2_checkBox_stateChanged();
    void on_transparency_win3_checkBox_stateChanged();
    void on_sixtyFourElectrogram_checkBox_stateChanged();
    void on_ninetySixElectrogram_checkBox_stateChanged();
    void on_goldStandElectrogram_checkBox_stateChanged();
    void on_cathTranslation_checkBox_stateChanged();
    void on_MFSUpdate_pushButton_clicked();
    void on_cathSize_SpinBox_valueChanged();
    void on_reset_button_clicked();
    void on_actionE_xit_triggered();
    void on_plainBG_radioButton_clicked();
    void on_gradBG_radioButton_clicked();
    void on_plusZoom_toolButton_pressed();
    void on_minusZoom_toolButton_pressed();
    void on_plusZoom_toolButton_2_clicked();
    void on_minusZoom_toolButton_2_clicked();
    void on_plusZoom_toolButton_3_clicked();
    void on_minusZoom_toolButton_3_clicked();
    void on_scaleMin_spinBox_valueChanged();
    void on_scaleMax_spinBox_valueChanged();
    void on_nodePicking_checkBox_stateChanged();
    void on_closeElectrograms_button_clicked();
    void on_comboBox_currentIndexChanged();
    void slotTimeout();
    void slotStart();
    void receiveCCaverage(double averageCC, int window);
    void receiveNRMSEaverage(double averageNRMSE, int window);
    void receivePick(PickInfo *pick);










    void on_analysis_pushButton_clicked();







    void on_catheterLock_win1_stateChanged();
    void on_catheterLock_win2_stateChanged();
    void on_catheterLock_win3_stateChanged();
    void on_cathRotation_checkBox_stateChanged();

    void on_phaseAnlysis_checkBox_stateChanged();

signals:
    void sendTransparencySignal(int row,bool checkState);
    void sendPointsOnlySignal(int row);
    void updateMFSresult();
    void maxValueChanged(float max);
    void minValueChanged(float min);
    void updateScale();
    void switchBackground(bool gradBg);
    void cathSizeChanged(int, float);
    void sendNodeLockState(bool nodePickStateFromGUI);
    void updateStatsBoxes();
    void updatePickWindows();
    void sendCathTransLockState(bool cathTransStateFromGUI);
    void sendCathWinLockState(bool cathWinStateFromGUI);
    void sendCathRotationLockState(bool cathRotationStateFromGUI);
    void sendPhaseAnalysisState(bool phaseAnalysisState);
    void askForAnalysis(int row);



private:
    void setUpGUIWindows();
    QString CCPickAverage(int atrialMeshIndex);
    QString NRMSEPickAverage(int atrialMeshIndex);


    QTimer timer;
    RescaleColorMap rescaleObject;              //Access all scaling via this object
    DrawTransparentPoints *dtpPtr;              //Access all DTP function via this pointer.

    QList <GeomWindow *> geomWins;
    QList <Mesh_Info *> atrialMeshes;
    QList <Mesh_Info *> cathMeshes;
    vector <PickWindow *> pickWindowsList;

    Ui::GUI_MainWindow *ui;
    QWidget *scrollWidget;
    QVBoxLayout *verticalScrollLayout;

    double NRMSEwin2;
    double NRMSEwin3;
    double CCwin2;
    double CCwin3;

    float storeMaxScale;
    float storeMinScale;





};

#endif // GUI_MAINWINDOW_H
