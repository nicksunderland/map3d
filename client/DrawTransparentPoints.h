#ifndef DRAWTRANSPARENTPOINTS_H
#define DRAWTRANSPARENTPOINTS_H

#include "ui_DrawTransparentPoints.h"
#include "dialogs.h"
#include <QList>

class QPushButton;
class QDoubleSpinBox;
class QCheckBox;


class DrawTransparentPoints : public QDialog, public Ui::DrawTransparentPoints
{
    Q_OBJECT

public:
    DrawTransparentPoints();

public slots:
    void on_applyButton_clicked ();
    void on_cancelButton_clicked ();
    void Transp_Points_Callback();
    void Activation_Callback();
    void CalculateMFSTransformMatrix(Mesh_Info * recordingmesh, Mesh_Info * curmesh);
    void CalculatePhaseMap(Mesh_Info * curmesh);
    void CalculateActivation(Mesh_Info * curmesh);
    bool checkArray1D(Surf_Data* data, float *matrixvals);
    bool checkArray2D(Surf_Data* data,float **matrixvals);
    void InDeflateMesh(Mesh_Info * curmesh);
    void InDeflateMesh_touching(Mesh_Info * curmesh,Mesh_Info * sourcemesh);
    void Recal_MFS_Callback();
    void CalculateCC(Mesh_Info * curmesh);
    void CalculateRMSE(Mesh_Info * curmesh);
    void normalize1D(Surf_Data* data,float *matrixvals);
    float correlationCoefficient(double X[], double Y[], int n);
    float rootmeansquareerror(double X[], double Y[], int n);

private slots:
    void makeSurfaceTransparent(int row, bool checkState);       //Nick
    void makeSurfacePoints(int row);                                            //Nick
    void updateMFSresult();                                                           //Nick
    void cathSizeChange(int row, float size);                                 //Nick
    void CalculateCCforGUI(Mesh_Info *curmesh, int window);     //Nick
    void CalculateRMSEforGUI(Mesh_Info *curmesh); //Nick
    void makePhaseAnalysis(bool phaseAnalysisCheckState);      //Nick
    void callForAnalysisFromGUI(int rowSignal);                            //Nick
signals:
    void averageCCvalue(double averageCC, int row);                     //Nick
    void averageNRMSEvalue(double averageNRMSE, int window);  //Nick

private:
    QList<bool> origFixedTranparent;
    QList<bool> origPointsOnlyFix;
    QList<bool> origForward;
    QList<bool> origDatacloud;
    QList<bool> origMFS;
    QList<bool> origPhase;
    QList<bool> origChangesize;
    QList<double> origInDeflate;

    QList<Mesh_Info*> meshes;

    QList<QWidget *>widgetList;                                              //Nick
    QWidget *analysisHolder;
    QVBoxLayout *holderLayout;


    // widgets
    QList<QCheckBox*> fixedPointsOnlyBoxes;
    QList<QCheckBox*> fixedTransparentBoxes;
    QList<QCheckBox*> fixedDatacloudBoxes;
    QList<QCheckBox*> fixedForwardBoxes;
    QList<QCheckBox*> fixedMFSBoxes;
    QList<QCheckBox*> fixedPhaseBoxes;
    QList<QPushButton*> ActivationButton;
    QList<QCheckBox*> fixedSizeBoxes;
    QList<QDoubleSpinBox*> InDeflateBoxes;



};

#if 0

//void Transp_PointsDialogCreate(bool show = true);
//void Transp_PointsCancel();
//void Transp_PointsPreview(bool okay);
//void Transp_PointsOkay();
//void Transp_Pointscallback(FilesDialogRowData* rowdata);
//void Transp_PointsDialogChangeLock();
//void Transp_Pointsdestroycallback();


//void updateTransp_PointsDialogValues(Mesh_Info* mesh);

#endif


#endif // DRAWTRANSPARENTPOINTS_H
