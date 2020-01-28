#include "DrawTransparentPoints.h"
#include "MeshList.h"
#include "Surf_Data.h"
#include "Map3d_Geom.h"
#include "WindowManager.h"
#include "eventdata.h"
#include <math.h>
#include <QCheckBox>
#include "FileDialog.h"
#include "MainWindow.h"
#include "ProcessCommandLineOptions.h"
#include "Transforms.h"
#include "eventdata.h"
#include "map3d-struct.h"
#include "matlabarray.h"
#include "GetMatrixSlice.h"
#include "ActivationLegendWindow.h"
#include "GeomWindow.h"

#include "ActivationMapWindow.h"
#include "CCMapWindow.h"
#include "RMSEMapWindow.h"
#include <QPushButton>
#include <QMessageBox>
#include <QLabel>
#include "pickinfo.h"
#include <iostream>
using namespace std;

#include <algorithm>
#include <vector>
#include <cmath>
#include <tuple>

#include <functional>

#include <QDebug>
#include <QScreen>
#include <QDesktopWidget>
#include <engine.h>
#include <matrix.h>
#include <mex.h>
#include "map3dmath.h"
#include <cstdlib>

#include <fstream>
#include <sstream>

extern Map3d_Info map3d_info;


int unlock_transparency_surfnum[30];
int unlock_electrode_surfnum[30];
int unlock_datacloud_surfnum[30];
int unlock_forward_surfnum[30];
int unlock_MFS_surfnum[30];
int unlock_Indeflate_surfnum[30];
int unlock_Phase_surfnum[30];


const char* MeshProperty_trans_Points = "MeshProperty_trans_Points";

Engine *ep_matrix = engOpen(NULL);


enum contTableCols{
    SurfNum, Transparencycol, PointsOnlyCols, ActivationCols, DatacloudCols, ForwardCols, MFSCols,PhaseCols, ChangeSizeCol, InDeflateCols,RecalCol
};


DrawTransparentPoints::DrawTransparentPoints()
{
    qDebug()<<"DrawTransparentPoints ctor";

    setupUi(this);
    int row=1;

    for (MeshIterator mi; !mi.isDone(); ++mi, ++row)
    {
        Mesh_Info* mesh = mi.getMesh();
        int index = row-1;

        // FIX - subsurf, NULL data

        bool origPointsOnly = false;
        bool origtransparent = false;
        bool origDatacloudIni = false;
        bool origForwardIni = false;
        bool origMFSIni = false;
        bool origPhaseIni =false;
        bool origSize =false;
        float origInDeIni=1;

        if (mesh->data)
        {
            origPointsOnly = mesh->data->user_pointsonly;
            origtransparent = mesh->data->user_transparent;
            origDatacloudIni = mesh->data->user_datacloud;
            origForwardIni = mesh->data->user_forward;
            origMFSIni = mesh->data->user_MFS;
            origPhaseIni =mesh->data->user_Phase;
            origSize =mesh->data->user_fixmeshsize;
            origInDeIni=mesh->data->user_InDe_parameter;

        }

        origPointsOnlyFix << origPointsOnly;
        origFixedTranparent << origtransparent;
        origForward << origForwardIni;
        origDatacloud << origDatacloudIni;
        origMFS << origMFSIni;
        origPhase << origPhaseIni;
        origChangesize << origSize;
        origInDeflate << origInDeIni;

        meshes << mesh;

        QLabel* label = new QLabel(QString::number(mesh->geom->surfnum), this);
        gridLayout->addWidget(label, row, SurfNum);

        QCheckBox* fixedPoints = new QCheckBox(this);
        fixedPoints->setChecked(origPointsOnly);
        fixedPoints->setProperty(MeshProperty_trans_Points, index);
        fixedPointsOnlyBoxes << fixedPoints;
        gridLayout->addWidget(fixedPoints, row, PointsOnlyCols);


        QCheckBox* fixedTrans = new QCheckBox(this);
        fixedTrans->setChecked(origtransparent);
        fixedTrans->setProperty(MeshProperty_trans_Points, index);
        fixedTransparentBoxes << fixedTrans;
        gridLayout->addWidget(fixedTrans, row, Transparencycol);

        QPushButton* fixedbutton =new QPushButton ("Validation map #" + QString::number(mesh->geom->surfnum), this);
        ActivationButton << fixedbutton;
        fixedbutton->setProperty(MeshProperty_trans_Points, index);
        gridLayout->addWidget(fixedbutton, row, ActivationCols);

        QCheckBox* fixdatacloud = new QCheckBox(this);
        fixdatacloud->setChecked(origDatacloudIni);
        fixdatacloud->setProperty(MeshProperty_trans_Points, index);
        fixedDatacloudBoxes << fixdatacloud;
        gridLayout->addWidget(fixdatacloud, row, DatacloudCols);


        QCheckBox* fixforward = new QCheckBox(this);
        fixforward->setChecked(origForwardIni);
        fixforward->setProperty(MeshProperty_trans_Points, index);
        fixedForwardBoxes << fixforward;
        gridLayout->addWidget(fixforward, row, ForwardCols);


        QCheckBox* fixMFS = new QCheckBox(this);
        fixMFS->setChecked(origMFSIni);
        fixMFS->setProperty(MeshProperty_trans_Points, index);
        fixedMFSBoxes << fixMFS;
        gridLayout->addWidget(fixMFS, row, MFSCols);


        QCheckBox* fixPhase = new QCheckBox(this);
        fixPhase->setChecked(origPhaseIni);
        fixPhase->setProperty(MeshProperty_trans_Points, index);
        fixedPhaseBoxes << fixPhase;
        gridLayout->addWidget(fixPhase, row, PhaseCols);



        QCheckBox* fixedsize = new QCheckBox(this);
        fixedsize->setChecked(origSize);
        fixedsize->setProperty(MeshProperty_trans_Points, index);
        fixedSizeBoxes << fixedsize;
        gridLayout->addWidget(fixedsize, row, ChangeSizeCol);


        QDoubleSpinBox* InDe = new QDoubleSpinBox(this);
        InDe->setRange(0.1, 10);
        InDe->setSingleStep(0.1);
        InDe->setValue(origInDeIni);
        InDe->setProperty(MeshProperty_trans_Points, index);
        InDeflateBoxes << InDe;
        gridLayout->addWidget(InDe, row, InDeflateCols);


        QPushButton* recalbutton =new QPushButton ("Recal MFS");
        ActivationButton << recalbutton;
        recalbutton->setProperty(MeshProperty_trans_Points, index);
        gridLayout->addWidget(recalbutton, row, RecalCol);


        connect(fixedPoints , SIGNAL(toggled(bool)), this, SLOT(Transp_Points_Callback()));
        connect(fixedTrans, SIGNAL(toggled(bool)), this, SLOT(Transp_Points_Callback()));
        connect(fixedbutton, SIGNAL(clicked()), this, SLOT(Activation_Callback()));
        connect(fixdatacloud, SIGNAL(toggled(bool)), this, SLOT(Transp_Points_Callback()));
        connect(fixforward, SIGNAL(toggled(bool)), this, SLOT(Transp_Points_Callback()));
        connect(fixMFS, SIGNAL(toggled(bool)), this, SLOT(Transp_Points_Callback()));
        connect(fixPhase, SIGNAL(toggled(bool)), this, SLOT(Transp_Points_Callback()));
        connect(fixedsize, SIGNAL(toggled(bool)), this, SLOT(Transp_Points_Callback()));
        connect(InDe, SIGNAL(valueChanged(double)), this, SLOT(Transp_Points_Callback()));
        connect(recalbutton, SIGNAL(clicked()), this, SLOT(Recal_MFS_Callback()));

    }
}

void DrawTransparentPoints::Transp_Points_Callback()
{    Q_ASSERT(sender());

     qDebug()<<"DrawTransparentPoints::Transp_Points_Callback()";

     QVariant rowProp = sender()->property(MeshProperty_trans_Points);

      int row = rowProp.toInt();
       Mesh_Info* mesh = meshes[row];

        if (mesh->data) {

            if (fixedPointsOnlyBoxes[row]->isChecked())
            {
                mesh->data->user_pointsonly = true;
                unlock_electrode_surfnum[row]=row+1;
            }

            else {
                mesh->data->user_pointsonly = false;
                unlock_electrode_surfnum[row]=0;
            }


            if (fixedTransparentBoxes[row]->isChecked())
            {
                mesh->data->user_transparent = true;
                unlock_transparency_surfnum[row]=row+1;
            }
            else {
                mesh->data->user_transparent = false;
                unlock_transparency_surfnum[row]=0;
            }


            if (fixedDatacloudBoxes[row]->isChecked())
            {
                mesh->data->user_datacloud = true;
                unlock_datacloud_surfnum[row]=row+1;
            }
            else {
                mesh->data->user_datacloud = false;
                unlock_datacloud_surfnum[row]=0;
            }


            if (fixedForwardBoxes[row]->isChecked())
            {
                mesh->data->user_forward = true;
                unlock_forward_surfnum[row]=row+1;
            }
            else {
                mesh->data->user_forward = false;
                unlock_forward_surfnum[row]=0;
            }


            if (fixedMFSBoxes[row]->isChecked())
            {


                mesh->data->user_MFS = true;
                unlock_MFS_surfnum[row]=row+1;


                int length = meshes.size();

                if ((length>1) && (row-1>=0))
                {
                    Mesh_Info *recordingmesh = 0;
                    recordingmesh=meshes[row-1];
                    CalculateMFSTransformMatrix(recordingmesh,mesh);

                    Surf_Data* data = mesh->data;
                    if (checkArray2D(data,data->potvals)==0)
                    {
                        CalculateCC(mesh);
                        CalculateRMSE(mesh);
                        normalize1D(data,data->RMSEvals);
                    }
                }
            }
            else {
                mesh->data->user_MFS = false;
                unlock_MFS_surfnum[row]=0;
            }


            if (fixedPhaseBoxes[row]->isChecked())
            {
                 mesh->data->user_Phase = true;

                 CalculatePhaseMap(mesh);

                unlock_Phase_surfnum[row]=row+1;
            }
            else {
                mesh->data->user_Phase = false;
                unlock_Phase_surfnum[row]=0;
            }




            if (fixedSizeBoxes[row]->isChecked())

            {   std::cout<<"/////////////////////////////////////////////////////////////////////////////"<<std::endl;

                mesh->data->user_fixmeshsize = true;
                mesh->data->user_InDe_parameter=(float)InDeflateBoxes[row]->value();

                int length = meshes.size();

                if (length>row+1)
                {
                    // only change catheter size.
                    Mesh_Info *sourcemesh = 0;
                    sourcemesh=meshes[row+1];
                    InDeflateMesh_touching(mesh,sourcemesh);
                }
                else
                {
                    InDeflateMesh(mesh);
                }

                unlock_Indeflate_surfnum[row]=row+1;
            }
            else {
                mesh->data->user_fixmeshsize = false;
                unlock_Indeflate_surfnum[row]=0;
            }
        }

        Broadcast(MAP3D_UPDATE);
}




void DrawTransparentPoints::InDeflateMesh(Mesh_Info * curmesh)

{
    qDebug()<<"DrawTransparentPoints::InDeflateMesh";

    Map3d_Geom *curgeom = 0;
    Surf_Data *cursurf = 0;
    curgeom = curmesh->geom;
    cursurf = curmesh->data;

    int  meshpoint_num =0;
    meshpoint_num = curgeom->numpts;

    float** pts = curgeom->original_points[curgeom->geom_index];


    double sum_x,sum_y,sum_z,center_x,center_y,center_z;

    for (int i=0; i< meshpoint_num; i++)
    {
        sum_x += pts[i][0];
        sum_y += pts[i][1];
        sum_z += pts[i][2];

    }

    center_x = (sum_x)/meshpoint_num;
    center_y = (sum_y)/meshpoint_num;
    center_z = (sum_z)/meshpoint_num;

    std::cout<<"sun_x  "<<sum_x<<"    "<<"center_x   "<<center_x<<std::endl;
    std::cout<<"sun_y  "<<sum_y<<"    "<<"center_y   "<<center_y<<std::endl;
    std::cout<<"sun_z  "<<sum_z<<"    "<<"center_z   "<<center_z<<std::endl;
    std::cout<<"---------------------------------------------------------------------------"<<std::endl;

    float **InDeflated_pts = 0;
    InDeflated_pts= Alloc_fmatrix(curgeom->numpts, 3);
    for (int j=0; j< meshpoint_num; j++)
    {
        InDeflated_pts[j][0] = pts[j][0]+(curmesh->data->user_InDe_parameter-1)*(pts[j][0]-center_x);
        InDeflated_pts[j][1] = pts[j][1]+(curmesh->data->user_InDe_parameter-1)*(pts[j][1]-center_y);
        InDeflated_pts[j][2] = pts[j][2]+(curmesh->data->user_InDe_parameter-1)*(pts[j][2]-center_z);
    }
    curgeom->points[curgeom->geom_index]=InDeflated_pts;



}


void DrawTransparentPoints::InDeflateMesh_touching(Mesh_Info * curmesh,Mesh_Info * sourcemesh)
{

    engSetVisible(ep_matrix, false);

    std::cout<<"enter indeflatemesh_touching  "<<std::endl;
    Map3d_Geom *curgeom = 0;
    Surf_Data *cursurf = 0;
    curgeom = curmesh->geom;
    cursurf = curmesh->data;

    int  meshpoint_num =0;
    meshpoint_num = curgeom->numpts;
    float** ori_pts = curgeom->original_points[curgeom->geom_index];


    int atria_pts_num=0,atria_elem_num=0,atria_datacloud_pts_num=0;
    float **atriapts,**atriadatacloud;
    long **atriaelement;
    Map3d_Geom *sourcegeom = 0;
    Surf_Data *sourcesurf = 0;
    sourcegeom = sourcemesh->geom;
    sourcesurf = sourcemesh->data;
    atria_pts_num = sourcegeom->numpts;
    atriapts = sourcegeom->points[sourcegeom->geom_index];
    atriaelement = sourcegeom->elements;
    atria_elem_num=sourcegeom->numelements;
    atria_datacloud_pts_num=sourcegeom->numdatacloud;
    atriadatacloud = sourcegeom->datacloud[sourcegeom->geom_index];

    //    // this part is to rotate the catheter. if map3d_info.lockrotate==LOCK_OFF, only apply transform matrix to catheter
    //    // if map3d_info.lockrotate==LOCK_FULL, apply both transform matrix to catheter and atrium, corresponding matrix is different.
        float** pts = 0;
        float** geom_temp_catheter_pts=0;
        float **rotated_catheter_pts = 0;
        rotated_catheter_pts= Alloc_fmatrix(curgeom->numpts, 3);

        GeomWindow* priv_catheter = curmesh->gpriv;
        HMatrix mNow_catheter /*, original */ ;  // arcball rotation matrices
        Transforms *tran_catheter = curmesh->tran;
        //translation matrix in column-major
        float centerM_catheter[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},
                                        {-priv_catheter->xcenter,-priv_catheter->ycenter,-priv_catheter->zcenter,1}};
        float invCenterM_catheter[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},
                                           {priv_catheter->xcenter,priv_catheter->ycenter,priv_catheter->zcenter,1}};
        float translateM_catheter[4][4] = { {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0},
                                            {tran_catheter->tx, tran_catheter->ty, tran_catheter->tz, 1}
                                          };
        float temp_catheter[16];
        float product_catheter[16];

        //rotation matrix
        Ball_Value(&tran_catheter->rotate, mNow_catheter);
        // apply translation
        // translate recordingmesh's center to origin
        MultMatrix16x16((float *)translateM_catheter, (float *)invCenterM_catheter, (float*)product_catheter);
        // rotate
        MultMatrix16x16((float *)product_catheter, (float *)mNow_catheter, (float*)temp_catheter);
        // revert recordingmesh translation to origin
        MultMatrix16x16((float*)temp_catheter, (float *) centerM_catheter, (float*)product_catheter);



        for (int loop1 = 0; loop1 < meshpoint_num; loop1++)
        {

            float rhs_catheter[4];
            float result_catheter[4];
            rhs_catheter[0] = ori_pts[loop1][0];
            rhs_catheter[1] = ori_pts[loop1][1];
            rhs_catheter[2] = ori_pts[loop1][2];
            rhs_catheter[3] = 1;

            MultMatrix16x4(product_catheter, rhs_catheter, result_catheter);

            rotated_catheter_pts[loop1][0] = result_catheter[0];
            rotated_catheter_pts[loop1][1] = result_catheter[1];
            rotated_catheter_pts[loop1][2] = result_catheter[2];
        }

        geom_temp_catheter_pts=rotated_catheter_pts;

        pts=geom_temp_catheter_pts;

        curgeom->original_points[curgeom->geom_index]=rotated_catheter_pts;

        //this part is to rotate the source surface (atrium).transform matrix is not applied if map3d_info.lockrotate==LOCK_OFF

        float** pts_atria = sourcegeom->points[sourcegeom->geom_index];
        float** geom_temp_atria_pts=pts_atria;
        float **rotated_atria_pts = 0;
        rotated_atria_pts= Alloc_fmatrix(sourcegeom->numpts, 3);


        GeomWindow* priv_atria = sourcemesh->gpriv;
        HMatrix mNow_atria /*, original */ ;  // arcball rotation matrices
        Transforms *tran_atria = sourcemesh->tran;
        //translation matrix in column-major
        float centerM_atria[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},
                                     {-priv_atria->xcenter,-priv_atria->ycenter,-priv_atria->zcenter,1}};
        float invCenterM_atria[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},
                                        {priv_atria->xcenter,priv_atria->ycenter,priv_atria->zcenter,1}};
        float translateM_atria[4][4] = { {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0},
                                         {tran_atria->tx, tran_atria->ty, tran_atria->tz, 1}
                                       };
        float temp_atria[16];
        float product_atria[16];

        //rotation matrix
        Ball_Value(&tran_atria->rotate, mNow_atria);
        // apply translation
        // translate sourcemesh's center to origin
        MultMatrix16x16((float *)translateM_atria, (float *)invCenterM_atria, (float*)product_atria);
        // rotate
        MultMatrix16x16((float *)product_atria, (float *)mNow_atria, (float*)temp_atria);
        // revert sourcemesh translation to origin
        MultMatrix16x16((float*)temp_atria, (float *) centerM_atria, (float*)product_atria);


        for (int loop2 = 0; loop2 < atria_pts_num; loop2++)
        {

            float rhs_atria[4];
            float result_atria[4];
            rhs_atria[0] = pts_atria[loop2][0];
            rhs_atria[1] = pts_atria[loop2][1];
            rhs_atria[2] = pts_atria[loop2][2];
            rhs_atria[3] = 1;

            MultMatrix16x4(product_atria, rhs_atria, result_atria);

            rotated_atria_pts[loop2][0] = result_atria[0];
            rotated_atria_pts[loop2][1] = result_atria[1];
            rotated_atria_pts[loop2][2] = result_atria[2];

        }
        geom_temp_atria_pts=rotated_atria_pts;


        sourcegeom->points[sourcegeom->geom_index]=rotated_atria_pts;



       //rotate the datacloud


        float** pts_atria_datacloud = sourcegeom->datacloud[sourcegeom->geom_index];
                float** geom_temp_atria_datacloud_pts=pts_atria_datacloud;
                float **rotated_atria_datacloud_pts = 0;
                rotated_atria_datacloud_pts= Alloc_fmatrix(sourcegeom->numdatacloud, 3);


                GeomWindow* priv_atria_datacloud = sourcemesh->gpriv;
                HMatrix mNow_atria_datacloud /*, original */ ;  // arcball rotation matrices
                Transforms *tran_atria_datacloud = sourcemesh->tran;
                //translation matrix in column-major
                float centerM_atria_datacloud[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},
                                             {-priv_atria_datacloud->xcenter,-priv_atria_datacloud->ycenter,-priv_atria_datacloud->zcenter,1}};
                float invCenterM_atria_datacloud[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},
                                                {priv_atria_datacloud->xcenter,priv_atria_datacloud->ycenter,priv_atria_datacloud->zcenter,1}};
                float translateM_atria_datacloud[4][4] = { {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0},
                                                 {tran_atria_datacloud->tx, tran_atria_datacloud->ty, tran_atria_datacloud->tz, 1}
                                               };
                float temp_atria_datacloud[16];
                float product_atria_datacloud[16];

                //rotation matrix
                Ball_Value(&tran_atria_datacloud->rotate, mNow_atria_datacloud);
                // apply translation
                // translate sourcemesh's center to origin
                MultMatrix16x16((float *)translateM_atria_datacloud, (float *)invCenterM_atria_datacloud, (float*)product_atria_datacloud);
                // rotate
                MultMatrix16x16((float *)product_atria_datacloud, (float *)mNow_atria_datacloud, (float*)temp_atria_datacloud);
                // revert sourcemesh translation to origin
                MultMatrix16x16((float*)temp_atria_datacloud, (float *) centerM_atria_datacloud, (float*)product_atria_datacloud);


                for (int loop2 = 0; loop2 < atria_datacloud_pts_num; loop2++)
                {

                    float rhs_atria_datacloud[4];
                    float result_atria_datacloud[4];
                    rhs_atria_datacloud[0] = pts_atria_datacloud[loop2][0];
                    rhs_atria_datacloud[1] = pts_atria_datacloud[loop2][1];
                    rhs_atria_datacloud[2] = pts_atria_datacloud[loop2][2];
                    rhs_atria_datacloud[3] = 1;

                    MultMatrix16x4(product_atria_datacloud, rhs_atria_datacloud, result_atria_datacloud);

                    rotated_atria_datacloud_pts[loop2][0] = result_atria_datacloud[0];
                    rotated_atria_datacloud_pts[loop2][1] = result_atria_datacloud[1];
                    rotated_atria_datacloud_pts[loop2][2] = result_atria_datacloud[2];

                }


        sourcegeom->datacloud[sourcegeom->geom_index]=rotated_atria_datacloud_pts;

// std::cout<<"initial original mesh point 1  "<<"x-----"<<curgeom->original_points[curgeom->geom_index][7][0]<<" y-----"<<curgeom->original_points[curgeom->geom_index][7][1]<<" z-----"<<curgeom->original_points[curgeom->geom_index][7][2]<<std::endl;


    double sum_x,sum_y,sum_z,center_x,center_y,center_z;

    for (int i=0; i< meshpoint_num; i++)
    {
        sum_x += pts[i][0];
        sum_y += pts[i][1];
        sum_z += pts[i][2];
    }

    center_x = (sum_x)/meshpoint_num;
    center_y = (sum_y)/meshpoint_num;
    center_z = (sum_z)/meshpoint_num;

//    std::cout<<"sum_x  "<<sum_x<<"    "<<"center_x   "<<center_x<<std::endl;
//    std::cout<<"sum_y  "<<sum_y<<"    "<<"center_y   "<<center_y<<std::endl;
//    std::cout<<"sum_z  "<<sum_z<<"    "<<"center_z   "<<center_z<<std::endl;
//    std::cout<<"-----------------------------------------------------------------------------------------------------------------------------------------------------"<<std::endl;

    float **InDeflated_pts = 0;
    InDeflated_pts= Alloc_fmatrix(curgeom->numpts, 3);

    double catehter_points[meshpoint_num][3];
    double ori_catheter_points[meshpoint_num][3];


    for (int j=0; j< meshpoint_num; j++)
    {
        InDeflated_pts[j][0] = pts[j][0]+(curmesh->data->user_InDe_parameter-1)*(pts[j][0]-center_x);
        InDeflated_pts[j][1] = pts[j][1]+(curmesh->data->user_InDe_parameter-1)*(pts[j][1]-center_y);
        InDeflated_pts[j][2] = pts[j][2]+(curmesh->data->user_InDe_parameter-1)*(pts[j][2]-center_z);
    }


    for (int j=0; j< meshpoint_num; j++)
    {
        catehter_points[j][0]=InDeflated_pts[j][0];
        catehter_points[j][1]=InDeflated_pts[j][1];
        catehter_points[j][2]=InDeflated_pts[j][2];

        ori_catheter_points[j][0]=pts[j][0];
        ori_catheter_points[j][1]=pts[j][1];
        ori_catheter_points[j][2]=pts[j][2];
    }


//   std::cout<<"original catheter point 1  "<<"x-----"<<ori_catheter_points[7][0]<<" y-----"<<ori_catheter_points[7][1]<<" z-----"<<ori_catheter_points[7][2]<<std::endl;
//   std::cout<<"in-deflated catheter point 1  "<<"x-----"<<catehter_points[7][0]<<" y-----"<<catehter_points[7][1]<<" z-----"<<catehter_points[7][2]<<std::endl;


  // std::cout<<"initial mesh point 1  "<<"x-----"<<curgeom->points[curgeom->geom_index][0][0]<<" y-----"<<curgeom->points[curgeom->geom_index][0][1]<<" z-----"<<curgeom->points[curgeom->geom_index][0][2]<<std::endl;

    mxArray *catheter_points_matlab = mxCreateDoubleMatrix(3,meshpoint_num, mxREAL);
    memcpy(mxGetPr(catheter_points_matlab), catehter_points, meshpoint_num*3*sizeof(double));
    engPutVariable(ep_matrix, "catheter_points",catheter_points_matlab);


    mxArray *ori_catheter_points_matlab = mxCreateDoubleMatrix(3,meshpoint_num, mxREAL);
    memcpy(mxGetPr(ori_catheter_points_matlab), ori_catheter_points, meshpoint_num*3*sizeof(double));
    engPutVariable(ep_matrix, "ori_catheter_points",ori_catheter_points_matlab);

    double atria_points[atria_pts_num][3];

    for (int i=0; i< atria_pts_num; i++)
    {
        atria_points[i][0]= geom_temp_atria_pts[i][0];
        atria_points[i][1]= geom_temp_atria_pts[i][1];
        atria_points[i][2]= geom_temp_atria_pts[i][2];

    }

    mxArray *atria_points_matlab = mxCreateDoubleMatrix(3,atria_pts_num, mxREAL);
    memcpy(mxGetPr(atria_points_matlab), atria_points, atria_pts_num*3*sizeof(double));
    engPutVariable(ep_matrix, "atria_points",atria_points_matlab);



    double atria_elements[atria_elem_num][3];

    for (int i=0; i< atria_elem_num; i++)
    {
        atria_elements[i][0]= atriaelement[i][0]+1;
        atria_elements[i][1]= atriaelement[i][1]+1;
        atria_elements[i][2]= atriaelement[i][2]+1;
    }

    mxArray *atria_elements_matlab = mxCreateDoubleMatrix(3,atria_elem_num, mxREAL);
    memcpy(mxGetPr(atria_elements_matlab), atria_elements, atria_elem_num*3*sizeof(double));
    engPutVariable(ep_matrix, "atria_elements",atria_elements_matlab);

    engEvalString(ep_matrix, "addpath(genpath('/hpc_ntot/smen974/Map3d/MFS_Functions'))");

   // engEvalString(ep_matrix, "mfsEGM=testest(atria_points,atria_elements,catheter_points,ori_catheter_points)");

    engEvalString(ep_matrix, "newpoints= find_touching_points_map3d(atria_points,atria_elements,catheter_points,ori_catheter_points)");



    mxArray *points_touching_matlab = engGetVariable(ep_matrix, "newpoints");
    double *new_points = mxGetPr(points_touching_matlab);


    double final_points[meshpoint_num][3];


    for (int i=0; i<meshpoint_num; i++)
    {
        final_points[i][0] =new_points[i+0*meshpoint_num];
        final_points[i][1] =new_points[i+1*meshpoint_num];
        final_points[i][2] =new_points[i+2*meshpoint_num];

    }

    float **InDeflated_pts_final = 0;
    InDeflated_pts_final= Alloc_fmatrix(curgeom->numpts, 3);

    for (int j=0; j< meshpoint_num; j++)
    {
        InDeflated_pts_final[j][0] = final_points[j][0];
        InDeflated_pts_final[j][1] = final_points[j][1];
        InDeflated_pts_final[j][2] = final_points[j][2];
    }

   curmesh->tran->reset();
   sourcemesh->tran->reset();
   curgeom->points[curgeom->geom_index]=InDeflated_pts_final;


    std::cout<<"curgeom->points[curgeom->geom_index] point 1  "<<"x-----"<<curgeom->points[curgeom->geom_index][7][0]<<" y-----"<<curgeom->points[curgeom->geom_index][7][1]<<" z-----"<<curgeom->points[curgeom->geom_index][7][2]<<std::endl;
    std::cout<<"-----------------------------------------------------------------------------------------------------------------------------------------------------"<<std::endl;

}



bool DrawTransparentPoints::checkArray1D(Surf_Data* data, float *matrixvals)

{
    long  leadnum;

    for (leadnum = 0; leadnum < data->numleads; leadnum++) {

        if(matrixvals[leadnum] != 0)
        {
            return false;
        }
    }
    return true;
}


void DrawTransparentPoints::Recal_MFS_Callback()

{
    qDebug()<<"DrawTransparentPoints::Recal_MFS_Callback()";

    Q_ASSERT(sender());
    QVariant rowProp = sender()->property(MeshProperty_trans_Points);
    int row = rowProp.toInt();

    Mesh_Info* mesh = meshes[row];

    // only change catheter size.
    Mesh_Info *sourcemesh = 0;
    sourcemesh=meshes[row+1];
    int length = meshes.size();


    if (((length>=row+1))&&(length>1)&&(fixedMFSBoxes[row+1]->isChecked()))
    {
        CalculateMFSTransformMatrix(mesh,sourcemesh);
        CalculatePhaseMap(sourcemesh);
    }
    Surf_Data* data = sourcemesh->data;

    if (checkArray2D(data,data->potvals)==0)
    {
        CalculateCC(sourcemesh);
        CalculateRMSE(sourcemesh);
        normalize1D(data,data->RMSEvals);
    }

    Broadcast(MAP3D_UPDATE);
}


void DrawTransparentPoints::Activation_Callback()

{
     qDebug()<<"DrawTransparentPoints::Activation_Callback()";

    Q_ASSERT(sender());
    QVariant rowProp = sender()->property(MeshProperty_trans_Points);
    int row = rowProp.toInt();

    QWidget *actiWidget;
    QHBoxLayout *gLayout;
    actiWidget = new QWidget();

    gLayout = new QHBoxLayout(actiWidget);

    Mesh_Info* mesh = meshes[row];
    Surf_Data* data = mesh->data;

    if (( checkArray2D(data,data->inversevals)==0) || (checkArray2D(data,data->potvals)==0)|| (checkArray2D(data,data->forwardvals)==0))
    {

        CalculateActivation(mesh);

    }

    if (checkArray1D(data,data->activationvals)==0)

    {
        Surf_Data *s=0;
        s=mesh->data;

        ActivationMapWindow* actiwin;
        actiwin = new ActivationMapWindow(actiWidget);
        actiwin->addMesh(mesh);
        actiwin->setWindowFlags(Qt::WindowTransparentForInput);


        /* create colormap legend window */
        ActivationLegendWindow *lpriv = NULL;
        lpriv = new ActivationLegendWindow(actiWidget);

        int width, height;
        width = mesh->lw_xmax - mesh->lw_xmin;
        height = mesh->lw_ymax - mesh->lw_ymin;


        gLayout->addWidget(lpriv);
        gLayout->setStretch(0,1);
        gLayout->addWidget(actiwin);
        gLayout->setStretch(1,3);


        if ((checkArray1D(data,data->CCvals)==0)&&(checkArray1D(data,data->RMSEvals)==0))
        {
            CCMapWindow* CCwin;
            CCwin = new CCMapWindow(actiWidget);
            CCwin->addMesh(mesh);
            CCwin->setWindowFlags(Qt::WindowTransparentForInput);

            RMSEMapWindow* RMSEwin;
            RMSEwin = new RMSEMapWindow(actiWidget);
            RMSEwin->addMesh(mesh);
            RMSEwin->setWindowFlags(Qt::WindowTransparentForInput);

            gLayout->addWidget(CCwin);
            gLayout->setStretch(2,3);
            gLayout->addWidget(RMSEwin);
            gLayout->setStretch(3,3);

            actiWidget->setMinimumSize(1000,380);
        }
        else
        {
            actiWidget->setMinimumSize(500,380);
        }

        lpriv->setVisible(true);
        actiwin->show();
        actiWidget->show();


        if (lpriv != NULL) {
            // can fail if more than MAX_SURFS l-wins.
            lpriv->orientation = 1;
            if (mesh->mysurf->legendticks != -1) {
                lpriv->nticks = mesh->mysurf->legendticks;
                lpriv->matchContours = true;
            }

            lpriv->surf = s;
            lpriv->mesh = mesh;
            lpriv->map = &mesh->cmap;


            // background color
            lpriv->bgcolor[0] = mesh->mysurf->colour_bg[0] / 255.f;
            lpriv->bgcolor[1] = mesh->mysurf->colour_bg[1] / 255.f;
            lpriv->bgcolor[2] = mesh->mysurf->colour_bg[2] / 255.f;
            lpriv->fgcolor[0] = mesh->mysurf->colour_fg[0] / 255.f;
            lpriv->fgcolor[1] = mesh->mysurf->colour_fg[1] / 255.f;
            lpriv->fgcolor[2] = mesh->mysurf->colour_fg[2] / 255.f;
        }
    }

    else {

        QMessageBox::warning(this,QString("Warning"),QString("No enough input values for this surface!"));
    }
}




void DrawTransparentPoints::CalculateActivation(Mesh_Info * curmesh)

{
    qDebug()<<"DrawTransparentPoints::CalculateActivation";

    engSetVisible(ep_matrix, false);

    int  meshpoint_num =0;

    Map3d_Geom *curgeom = 0;
    Surf_Data *cursurf = 0;
    curgeom = curmesh->geom;
    cursurf = curmesh->data;
    meshpoint_num = curgeom->numpts;

    //std::cout<<"enter CalculateActivation  meshpoint_num "<<meshpoint_num<<std::endl;


    float** pts_atria = curgeom->points[curgeom->geom_index];

    double mesh_x[meshpoint_num],mesh_y[meshpoint_num],mesh_z[meshpoint_num];
    for (int i=0; i< meshpoint_num; i++)
    {
        mesh_x[i] =  pts_atria[i][0];
        mesh_y[i] =  pts_atria[i][1];
        mesh_z[i] =  pts_atria[i][2];
    }


    // std::cout<<"enter CalculateActivation numframes "<<curmesh->data->numframes<<std::endl;


    double pot_temp[meshpoint_num][curmesh->data->numframes];

    // std::cout<<"pot_temp[meshpoint_num][curmesh->data->numframes]   "<<pot_temp[0][0]<<std::endl;


    for (int i=0; i< meshpoint_num; i++)
    {
        for (int j=0; j< curmesh->data->numframes; j++)

            if (checkArray2D(curmesh->data,curmesh->data->inversevals)==0)
            {pot_temp[i][j] =cursurf->inversevals[j][i];}
            else if (checkArray2D(curmesh->data,curmesh->data->potvals)==0)
            {pot_temp[i][j] =cursurf->potvals[j][i];}
            else if(checkArray2D(curmesh->data,curmesh->data->forwardvals)==0)
            {pot_temp[i][j] =cursurf->forwardvals[j][i];}

    }


    mxArray *potential_matlab = mxCreateDoubleMatrix(curmesh->data->numframes,meshpoint_num, mxREAL);
    memcpy(mxGetPr(potential_matlab), pot_temp, meshpoint_num*curmesh->data->numframes*sizeof(double));
    engPutVariable(ep_matrix, "potential",potential_matlab);


    mxArray *x_matlab = mxCreateDoubleMatrix(1,meshpoint_num, mxREAL);
    memcpy(mxGetPr(x_matlab), mesh_x, meshpoint_num*sizeof(double));
    engPutVariable(ep_matrix, "c_x",x_matlab);

    mxArray *y_matlab = mxCreateDoubleMatrix(1,meshpoint_num, mxREAL);
    memcpy(mxGetPr(y_matlab), mesh_y, meshpoint_num*sizeof(double));
    engPutVariable(ep_matrix, "c_y",y_matlab);

    mxArray *z_matlab = mxCreateDoubleMatrix(1,meshpoint_num, mxREAL);
    memcpy(mxGetPr(z_matlab), mesh_z, meshpoint_num*sizeof(double));
    engPutVariable(ep_matrix, "c_z",z_matlab);

    engEvalString(ep_matrix, "addpath(genpath('/hpc_ntot/smen974/Map3d/MFS_Functions'))");
    engEvalString(ep_matrix, "[activation]=ActivationCalculation(c_x,c_y,c_z, potential)");



    mxArray *activation_matlab = engGetVariable(ep_matrix, "activation");
    double *activation = mxGetPr(activation_matlab);


    for (int i=0; i< meshpoint_num; i++)
    {
        cursurf->activationvals[i] =activation[i];


    }

}


void DrawTransparentPoints::CalculateCC(Mesh_Info * curmesh)

{
    int  meshpoint_num =0,frame_num=0;

    Map3d_Geom *curgeom = 0;
    Surf_Data *cursurf = 0;
    curgeom = curmesh->geom;
    cursurf = curmesh->data;
    meshpoint_num = curgeom->numpts;
    frame_num = curmesh->data->numframes;


    double inverse[frame_num],gold_standard[frame_num], CC[meshpoint_num];

    for (int i=0; i< meshpoint_num; i++)
    {
        for (int j=0; j< frame_num; j++)
        {
            inverse[j]=cursurf->inversevals[j][i];
            gold_standard[j]=cursurf->potvals[j][i];

            // std::cout<< "inverse value is "<<inverse[j]<<std::endl;
            //  std::cout<< "gold_standard value is "<<gold_standard[j]<<std::endl;
        }

        float corr = correlationCoefficient(inverse, gold_standard, frame_num);
        CC[i]=corr;
        //std::cout<< "CC value is "<<CC[i]<<std::endl;
        cursurf->CCvals[i] =CC[i];

    }


}




void DrawTransparentPoints::CalculateRMSE(Mesh_Info * curmesh)

{
    int  meshpoint_num =0,frame_num=0;

    Map3d_Geom *curgeom = 0;
    Surf_Data *cursurf = 0;
    curgeom = curmesh->geom;
    cursurf = curmesh->data;
    meshpoint_num = curgeom->numpts;
    frame_num = curmesh->data->numframes;
    double inverse[frame_num],gold_standard[frame_num], RMSE[meshpoint_num];
    for (int i=0; i< meshpoint_num; i++)
    {
        for (int j=0; j< frame_num; j++)
        {
            inverse[j]=cursurf->inversevals[j][i];
            gold_standard[j]=cursurf->potvals[j][i];
        }

        float rmse = rootmeansquareerror(inverse, gold_standard, frame_num);
        RMSE[i]=rmse;
        //  std::cout<< "RMSE value is "<<RMSE[i]<<std::endl;
        cursurf->RMSEvals[i] =RMSE[i];
    }



}



void DrawTransparentPoints::on_applyButton_clicked()
{
    qDebug()<<"DrawTransparentPoints::on_applyButton_clicked()";
    // Broadcast(MAP3D_UPDATE);
    close();
}

void DrawTransparentPoints::on_cancelButton_clicked()
{

    close();
}



void DrawTransparentPoints::CalculatePhaseMap(Mesh_Info * curmesh)
{
    engSetVisible(ep_matrix, false);

    Map3d_Geom *curgeom = 0;
    Surf_Data *cursurf = 0;
    curgeom = curmesh->geom;
    cursurf = curmesh->data;
    int meshpoint_num = curgeom->numpts;

    double pot_temp[meshpoint_num][curmesh->data->numframes];


    for (int i=0; i< meshpoint_num; i++)
    {
        for (int j=0; j< curmesh->data->numframes; j++)

            if (checkArray2D(curmesh->data,curmesh->data->inversevals)==0)
            {pot_temp[i][j] =cursurf->inversevals[j][i];}
            else if (checkArray2D(curmesh->data,curmesh->data->potvals)==0)
            {pot_temp[i][j] =cursurf->potvals[j][i];}
            else if(checkArray2D(curmesh->data,curmesh->data->forwardvals)==0)
            {pot_temp[i][j] =cursurf->forwardvals[j][i];}

    }

    mxArray *potential_matlab = mxCreateDoubleMatrix(curmesh->data->numframes,meshpoint_num, mxREAL);
    memcpy(mxGetPr(potential_matlab), pot_temp, meshpoint_num*curmesh->data->numframes*sizeof(double));
    engPutVariable(ep_matrix, "potential",potential_matlab);

    engEvalString(ep_matrix, "addpath(genpath('/hpc_ntot/smen974/Map3d/MFS_Functions'))");
    engEvalString(ep_matrix, "[phaseangles]= calculate_phase_angles_map3d(potential)");

    mxArray *phase_matlab = engGetVariable(ep_matrix, "phaseangles");
    double *phase = mxGetPr(phase_matlab);


    for (int j=0; j< curmesh->data->numframes; j++)
    {
        for (int i=0; i< meshpoint_num; i++)
        {
            cursurf->Phasevals[j][i] =phase[i+j*meshpoint_num];

        }
    }

}


void DrawTransparentPoints::CalculateMFSTransformMatrix(Mesh_Info * recordingmesh, Mesh_Info * curmesh)

{
    qDebug()<<"DrawTransparentPoints::CalculateMFSTransformMatrix";

    engSetVisible(ep_matrix, false);

    int catheter_num = 0, atria_num =0, loop1 = 0,loop2 = 0;


    // float **modelpts,**atriapts;
    long **catheterelement, **atriaelement;


    Map3d_Geom *recordinggeom = 0;
    Surf_Data *recordingsurf = 0;
    recordinggeom = recordingmesh->geom;
    recordingsurf = recordingmesh->data;
    catheter_num = recordinggeom->numpts;
    // modelpts = recordinggeom->points[recordinggeom->geom_index];
    catheterelement = recordinggeom->elements;


    Map3d_Geom *curgeom = 0;
    Surf_Data *cursurf = 0;
    curgeom = curmesh->geom;
    cursurf = curmesh->data;
    atria_num = curgeom->numpts;
    // atriapts = curgeom->points[curgeom->geom_index];
    atriaelement = curgeom->elements;

    // this part is to rotate the catheter. if map3d_info.lockrotate==LOCK_OFF, only apply transform matrix to catheter
    // if map3d_info.lockrotate==LOCK_FULL, apply both transform matrix to catheter and atrium, corresponding matrix is different.
    float** pts = recordinggeom->points[recordinggeom->geom_index];
    float** geom_temp_catheter_pts=pts;
    float **rotated_catheter_pts = 0;
    rotated_catheter_pts= Alloc_fmatrix(recordinggeom->numpts, 3);

    GeomWindow* priv_catheter = recordingmesh->gpriv;
    HMatrix mNow_catheter /*, original */ ;  // arcball rotation matrices
    Transforms *tran_catheter = recordingmesh->tran;
    //translation matrix in column-major
    float centerM_catheter[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},
                                    {-priv_catheter->xcenter,-priv_catheter->ycenter,-priv_catheter->zcenter,1}};
    float invCenterM_catheter[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},
                                       {priv_catheter->xcenter,priv_catheter->ycenter,priv_catheter->zcenter,1}};
    float translateM_catheter[4][4] = { {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0},
                                        {tran_catheter->tx, tran_catheter->ty, tran_catheter->tz, 1}
                                      };
    float temp_catheter[16];
    float product_catheter[16];

    //rotation matrix
    Ball_Value(&tran_catheter->rotate, mNow_catheter);
    // apply translation
    // translate recordingmesh's center to origin
    MultMatrix16x16((float *)translateM_catheter, (float *)invCenterM_catheter, (float*)product_catheter);
    // rotate
    MultMatrix16x16((float *)product_catheter, (float *)mNow_catheter, (float*)temp_catheter);
    // revert recordingmesh translation to origin
    MultMatrix16x16((float*)temp_catheter, (float *) centerM_catheter, (float*)product_catheter);



    for (loop1 = 0; loop1 < catheter_num; loop1++)
    {

        float rhs_catheter[4];
        float result_catheter[4];
        rhs_catheter[0] = pts[loop1][0];
        rhs_catheter[1] = pts[loop1][1];
        rhs_catheter[2] = pts[loop1][2];
        rhs_catheter[3] = 1;

        MultMatrix16x4(product_catheter, rhs_catheter, result_catheter);

        rotated_catheter_pts[loop1][0] = result_catheter[0];
        rotated_catheter_pts[loop1][1] = result_catheter[1];
        rotated_catheter_pts[loop1][2] = result_catheter[2];
    }

    geom_temp_catheter_pts=rotated_catheter_pts;

    //this part is to rotate the source surface (atrium).transform matrix is not applied if map3d_info.lockrotate==LOCK_OFF

    float** pts_atria = curgeom->points[curgeom->geom_index];
    float** geom_temp_atria_pts=pts_atria;
    float **rotated_atria_pts = 0;
    rotated_atria_pts= Alloc_fmatrix(curgeom->numpts, 3);


    GeomWindow* priv_atria = curmesh->gpriv;
    HMatrix mNow_atria /*, original */ ;  // arcball rotation matrices
    Transforms *tran_atria = curmesh->tran;
    //translation matrix in column-major
    float centerM_atria[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},
                                 {-priv_atria->xcenter,-priv_atria->ycenter,-priv_atria->zcenter,1}};
    float invCenterM_atria[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},
                                    {priv_atria->xcenter,priv_atria->ycenter,priv_atria->zcenter,1}};
    float translateM_atria[4][4] = { {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0},
                                     {tran_atria->tx, tran_atria->ty, tran_atria->tz, 1}
                                   };
    float temp_atria[16];
    float product_atria[16];

    //rotation matrix
    Ball_Value(&tran_atria->rotate, mNow_atria);
    // apply translation
    // translate curmesh's center to origin
    MultMatrix16x16((float *)translateM_atria, (float *)invCenterM_atria, (float*)product_atria);
    // rotate
    MultMatrix16x16((float *)product_atria, (float *)mNow_atria, (float*)temp_atria);
    // revert curmesh translation to origin
    MultMatrix16x16((float*)temp_atria, (float *) centerM_atria, (float*)product_atria);


    for (loop2 = 0; loop2 < atria_num; loop2++)
    {

        float rhs_atria[4];
        float result_atria[4];
        rhs_atria[0] = pts_atria[loop2][0];
        rhs_atria[1] = pts_atria[loop2][1];
        rhs_atria[2] = pts_atria[loop2][2];
        rhs_atria[3] = 1;

        MultMatrix16x4(product_atria, rhs_atria, result_atria);

        rotated_atria_pts[loop2][0] = result_atria[0];
        rotated_atria_pts[loop2][1] = result_atria[1];
        rotated_atria_pts[loop2][2] = result_atria[2];

    }
    geom_temp_atria_pts=rotated_atria_pts;



    // pass coordinates of catheters
    double pot_temp[catheter_num][recordingmesh->data->numframes], cath_x[catheter_num],cath_y[catheter_num],cath_z[catheter_num];

    for (int i=0; i< catheter_num; i++)
    {

        cath_x[i] =  geom_temp_catheter_pts[i][0];
        cath_y[i] =  geom_temp_catheter_pts[i][1];
        cath_z[i] =  geom_temp_catheter_pts[i][2];
    }


    for (int i=0; i< catheter_num; i++)
    {
        for (int j=0; j< recordingmesh->data->numframes; j++)

            if (recordingsurf->forwardvals[recordingsurf->framenum][i]==0)
            {pot_temp[i][j] =recordingsurf->potvals[j][i];}
            else
            {pot_temp[i][j] =recordingsurf->forwardvals[j][i];}
    }



    mxArray *catheter_potential_matlab = mxCreateDoubleMatrix(recordingmesh->data->numframes,catheter_num, mxREAL);
    memcpy(mxGetPr(catheter_potential_matlab), pot_temp, catheter_num*recordingmesh->data->numframes*sizeof(double));
    engPutVariable(ep_matrix, "catheter_potential",catheter_potential_matlab);


    mxArray *cath_x_matlab = mxCreateDoubleMatrix(1,catheter_num, mxREAL);
    memcpy(mxGetPr(cath_x_matlab), cath_x, catheter_num*sizeof(double));
    engPutVariable(ep_matrix, "c_x",cath_x_matlab);

    mxArray *cath_y_matlab = mxCreateDoubleMatrix(1,catheter_num, mxREAL);
    memcpy(mxGetPr(cath_y_matlab), cath_y, catheter_num*sizeof(double));
    engPutVariable(ep_matrix, "c_y",cath_y_matlab);

    mxArray *cath_z_matlab = mxCreateDoubleMatrix(1,catheter_num, mxREAL);
    memcpy(mxGetPr(cath_z_matlab), cath_z, catheter_num*sizeof(double));
    engPutVariable(ep_matrix, "c_z",cath_z_matlab);

    // pass coordinates of atria
    double atria_x[atria_num],atria_y[atria_num],atria_z[atria_num];
    for (int i=0; i< atria_num; i++)
    {

        atria_x[i] =  geom_temp_atria_pts[i][0];
        atria_y[i] =  geom_temp_atria_pts[i][1];
        atria_z[i] =  geom_temp_atria_pts[i][2];
    }
    mxArray *atria_x_matlab = mxCreateDoubleMatrix(1,atria_num, mxREAL);
    memcpy(mxGetPr(atria_x_matlab), atria_x, atria_num*sizeof(double));
    engPutVariable(ep_matrix, "a_x",atria_x_matlab);

    mxArray *atria_y_matlab = mxCreateDoubleMatrix(1,atria_num, mxREAL);
    memcpy(mxGetPr(atria_y_matlab), atria_y, atria_num*sizeof(double));
    engPutVariable(ep_matrix, "a_y",atria_y_matlab);

    mxArray *atria_z_matlab = mxCreateDoubleMatrix(1,atria_num, mxREAL);
    memcpy(mxGetPr(atria_z_matlab), atria_z, atria_num*sizeof(double));
    engPutVariable(ep_matrix, "a_z",atria_z_matlab);



    // pass elements of catheters
    double c_ele_1[recordinggeom->numelements],c_ele_2[recordinggeom->numelements],c_ele_3[recordinggeom->numelements];
    for (int j=0; j< recordinggeom->numelements; j++)
    {
        c_ele_1[j] = catheterelement[j][0]+1;
        c_ele_2[j] = catheterelement[j][1]+1;
        c_ele_3[j] = catheterelement[j][2]+1;
    }

    mxArray *cath_e1_matlab = mxCreateDoubleMatrix(1,recordinggeom->numelements, mxREAL);
    memcpy(mxGetPr(cath_e1_matlab), c_ele_1, recordinggeom->numelements*sizeof(double));
    engPutVariable(ep_matrix, "c_ele_1",cath_e1_matlab);

    mxArray *cath_e2_matlab = mxCreateDoubleMatrix(1,recordinggeom->numelements, mxREAL);
    memcpy(mxGetPr(cath_e2_matlab), c_ele_2, recordinggeom->numelements*sizeof(double));
    engPutVariable(ep_matrix, "c_ele_2",cath_e2_matlab);

    mxArray *cath_e3_matlab = mxCreateDoubleMatrix(1,recordinggeom->numelements, mxREAL);
    memcpy(mxGetPr(cath_e3_matlab), c_ele_3, recordinggeom->numelements*sizeof(double));
    engPutVariable(ep_matrix, "c_ele_3",cath_e3_matlab);

    // pass elements of atria
    double a_ele_1[curgeom->numelements],a_ele_2[curgeom->numelements],a_ele_3[curgeom->numelements];
    for (int k=0; k< curgeom->numelements; k++)
    {
        a_ele_1[k] = atriaelement[k][0]+1;
        a_ele_2[k] = atriaelement[k][1]+1;
        a_ele_3[k] = atriaelement[k][2]+1;

    }

    mxArray *atria_e1_matlab = mxCreateDoubleMatrix(1,curgeom->numelements, mxREAL);
    memcpy(mxGetPr(atria_e1_matlab), a_ele_1, curgeom->numelements*sizeof(double));
    engPutVariable(ep_matrix, "a_ele_1",atria_e1_matlab);

    mxArray *atria_e2_matlab = mxCreateDoubleMatrix(1,curgeom->numelements, mxREAL);
    memcpy(mxGetPr(atria_e2_matlab), a_ele_2, curgeom->numelements*sizeof(double));
    engPutVariable(ep_matrix, "a_ele_2",atria_e2_matlab);

    mxArray *atria_e3_matlab = mxCreateDoubleMatrix(1,curgeom->numelements, mxREAL);
    memcpy(mxGetPr(atria_e3_matlab), a_ele_3, curgeom->numelements*sizeof(double));
    engPutVariable(ep_matrix, "a_ele_3",atria_e3_matlab);


    engEvalString(ep_matrix, "addpath(genpath('/hpc_ntot/smen974/Map3d/MFS_Functions'))");
    engEvalString(ep_matrix, "mfsEGM=solve_MFS(c_x,c_y,c_z,c_ele_1,c_ele_2,c_ele_3,a_x,a_y,a_z,a_ele_1,a_ele_2,a_ele_3, catheter_potential)");



    mxArray *mfsEGM_matlab = engGetVariable(ep_matrix, "mfsEGM");
    double *mfsEGM = mxGetPr(mfsEGM_matlab);



    //    string filename;
    //    ofstream files;
    //    stringstream a;
    //    a << recordingmesh->data->user_InDe_parameter;
    //    filename = "inverse_130_" + a.str();
    //    filename += ".txt";
    //    files.open(filename.c_str(), ios::out);



    //                ofstream myfile;
    //                myfile.open ("inverse_128.txt");

    for (int j=0; j< curmesh->data->numframes; j++)
    {
        for (int i=0; i< atria_num; i++)
        {
            cursurf->MFSvals[j][i] =mfsEGM[i+j*atria_num];

            //            files << cursurf->MFSvals[j][i];
            //            files << "\n";

        }
    }


    cursurf->inversevals = cursurf->MFSvals;

    std::cout<<"mesh->data->user_InDe_parameter in MFS "<< recordingmesh->data->user_InDe_parameter<<std::endl;
    std::cout<<"/////////////////////////////////////////////////////////////////////////////"<<std::endl;

}



float DrawTransparentPoints::correlationCoefficient(double X[], double Y[], int n)
{

    double sum_X = 0, sum_Y = 0, sum_XY = 0;
    double squareSum_X = 0, squareSum_Y = 0;

    for (int i = 0; i < n; i++)
    {
        // sum of elements of array X.
        sum_X = sum_X + X[i];

        // sum of elements of array Y.
        sum_Y = sum_Y + Y[i];

        // sum of X[i] * Y[i].
        sum_XY = sum_XY + X[i] * Y[i];

        // sum of square of array elements.
        squareSum_X = squareSum_X + X[i] * X[i];
        squareSum_Y = squareSum_Y + Y[i] * Y[i];
    }

    // use formula for calculating correlation coefficient.
    float corr = (float)(n * sum_XY - sum_X * sum_Y)
            / sqrt((n * squareSum_X - sum_X * sum_X)
                   * (n * squareSum_Y - sum_Y * sum_Y));

    return corr;
}


float DrawTransparentPoints::rootmeansquareerror(double X[], double Y[], int n)
{

    double sum_XY = 0;


    for (int i = 0; i < n; i++)
    {
        sum_XY = sum_XY+(X[i] - Y[i])*(X[i] - Y[i]);
    }

    float rmse =  sqrt(sum_XY/n);
    return rmse;
}




bool DrawTransparentPoints::checkArray2D(Surf_Data* data,float **matrixvals)

{
    long framenum, leadnum;
    for (framenum = 0; framenum < data->numframes; framenum++) {

        for (leadnum = 0; leadnum < data->numleads; leadnum++) {

            if(matrixvals[framenum][leadnum] != 0)
            {
                return false;
            }
        }
        return true;
    }
}

void DrawTransparentPoints::normalize1D(Surf_Data* data,float *matrixvals)

{
    long leadnum=data->numleads;

    double normalized_matrix[leadnum];

    float maxval = *std::max_element(matrixvals,matrixvals+leadnum);
    float minval = *std::min_element(matrixvals,matrixvals+leadnum);


    for (int i = 0; i < leadnum; i++)
    {
        normalized_matrix[i]= (matrixvals[i]-minval)/(maxval-minval);

        matrixvals[i]=normalized_matrix[i];
    }



}


//Nick
void DrawTransparentPoints::makeSurfaceTransparent(int row, bool checkState)
{
    fixedTransparentBoxes[row]->setChecked(checkState);
}
void DrawTransparentPoints::makeSurfacePoints(int row)
{
    fixedPointsOnlyBoxes[row]->setChecked(true);
}
void DrawTransparentPoints::makePhaseAnalysis(bool phaseAnalysisCheckState)
{
    fixedPhaseBoxes[1]->setChecked(phaseAnalysisCheckState);
    fixedPhaseBoxes[3]->setChecked(phaseAnalysisCheckState);
    fixedPhaseBoxes[5]->setChecked(phaseAnalysisCheckState);
}

void DrawTransparentPoints::updateMFSresult()
{
    Mesh_Info *cathWin2 = meshes[2];
    Mesh_Info *cathWin3 = meshes[4];
    Mesh_Info *atriumWin2 = meshes[3];
    Mesh_Info *atriumWin3 = meshes[5];
    Surf_Data* dataWin2 = atriumWin2->data;
    Surf_Data* dataWin3 = atriumWin3->data;

    int mesh2point_num = atriumWin2->geom->numpts;
    int mesh3point_num = atriumWin3->geom->numpts;

    if(!(fixedForwardBoxes[4]->isChecked())){
            fixedForwardBoxes[2]->setChecked(true);
            fixedForwardBoxes[4]->setChecked(true);
            QTimer::singleShot(50, [=](){                                              //Currently the DTPs forwardvalue calculation is linked to the repainting of the display window; for now I have got round this by waiting for 50ms for the repaint.
                fixedMFSBoxes[3]->setChecked(true);
                fixedMFSBoxes[5]->setChecked(true);
              });

            QTimer::singleShot(50, [=](){                                              //Again, seem to need to wait for the MFS result from Map3D to return before calulating the statistics values
                if (checkArray2D(dataWin2,dataWin2->potvals)==0)
                {
                    CalculateCCforGUI(atriumWin2,2);
                    CalculateRMSEforGUI(atriumWin2);
                    normalize1D(dataWin2,dataWin2->RMSEvals);

                    double sum = 0;                                                          //Need to average the normalised RMSE, therefore the averaging is done here instead of in the function like with CC value.
                    for(int i = 0; i<mesh2point_num; i++)\
                           {sum = sum + dataWin2->RMSEvals[i];}
                    double averageNRMSE = sum/mesh2point_num;
                    emit averageNRMSEvalue(averageNRMSE,2);
                }
                if (checkArray2D(dataWin3,dataWin3->potvals)==0)
                {
                    CalculateCCforGUI(atriumWin3,3);
                    CalculateRMSEforGUI(atriumWin3);
                    normalize1D(dataWin3,dataWin3->RMSEvals);

                    double sum = 0;                                                          //Need to average the normalised RMSE, therefore the averaging is done here instead of in the function like with CC value.
                    for(int i = 0; i<mesh3point_num; i++)\
                           {sum = sum + dataWin3->RMSEvals[i];}
                    double averageNRMSE = sum/mesh3point_num;
                    emit averageNRMSEvalue(averageNRMSE,3);
                }
              });

    }else{

        CalculateMFSTransformMatrix(cathWin2,atriumWin2);
        CalculatePhaseMap(atriumWin2);
        CalculateMFSTransformMatrix(cathWin3,atriumWin3);
        CalculatePhaseMap(atriumWin3);

        if (checkArray2D(dataWin2,dataWin2->potvals)==0)
        {
            CalculateCCforGUI(atriumWin2,2);
            CalculateRMSEforGUI(atriumWin2);
            normalize1D(dataWin2,dataWin2->RMSEvals);

            double sum = 0;                                                          //Need to average the normalised RMSE, therefore the averaging is done here instead of in the function like with CC value.
            for(int i = 0; i<mesh2point_num; i++)\
                   {sum = sum + dataWin2->RMSEvals[i];}
            double averageNRMSE = sum/mesh2point_num;
            emit averageNRMSEvalue(averageNRMSE,2);
        }

        if (checkArray2D(dataWin3,dataWin3->potvals)==0)
        {
            CalculateCCforGUI(atriumWin3,3);
            CalculateRMSEforGUI(atriumWin3);
            normalize1D(dataWin3,dataWin3->RMSEvals);

            double sum = 0;                                                          //Need to average the normalised RMSE, therefore the averaging is done here instead of in the function like with CC value.
            for(int i = 0; i<mesh3point_num; i++)\
                   {sum = sum + dataWin3->RMSEvals[i];}
            double averageNRMSE = sum/mesh3point_num;
            emit averageNRMSEvalue(averageNRMSE,3);
        }
    }
}
void DrawTransparentPoints::cathSizeChange(int row, float size)
{
    fixedSizeBoxes[row]->setChecked(true);
    InDeflateBoxes[row]->setValue(size);
    Broadcast(MAP3D_UPDATE);
}

void DrawTransparentPoints::CalculateCCforGUI(Mesh_Info *curmesh, int window)
{
    int  meshpoint_num =0,frame_num=0;
    Map3d_Geom *curgeom = 0;
    Surf_Data *cursurf = 0;
    curgeom = curmesh->geom;
    cursurf = curmesh->data;
    meshpoint_num = curgeom->numpts;
    frame_num = curmesh->data->numframes;
    double inverse[frame_num],gold_standard[frame_num], CC[meshpoint_num];
    for (int i=0; i< meshpoint_num; i++)
    {
        for (int j=0; j< frame_num; j++)
        {
            inverse[j]=cursurf->inversevals[j][i];
            gold_standard[j]=cursurf->potvals[j][i];
        }
        float corr = correlationCoefficient(inverse, gold_standard, frame_num);
        CC[i]=corr;
        cursurf->CCvals[i] =CC[i];
    }
    double sum = 0;
    for(int i = 0; i<meshpoint_num; i++)\
    {
        sum = sum + cursurf->CCvals[i];
    }
    double averageCC = sum/meshpoint_num;
    emit averageCCvalue(averageCC,window);
}

void DrawTransparentPoints::CalculateRMSEforGUI(Mesh_Info *curmesh)
{
    int  meshpoint_num =0,frame_num=0;
    Map3d_Geom *curgeom = 0;
    Surf_Data *cursurf = 0;
    curgeom = curmesh->geom;
    cursurf = curmesh->data;
    meshpoint_num = curgeom->numpts;
    frame_num = curmesh->data->numframes;
    double inverse[frame_num],gold_standard[frame_num], RMSE[meshpoint_num];
    for (int i=0; i< meshpoint_num; i++)
    {
        for (int j=0; j< frame_num; j++)
        {
            inverse[j]=cursurf->inversevals[j][i];
            gold_standard[j]=cursurf->potvals[j][i];
        }
        float rmse = rootmeansquareerror(inverse, gold_standard, frame_num);
        RMSE[i]=rmse;
        cursurf->RMSEvals[i] =RMSE[i];
    }
}

void DrawTransparentPoints::callForAnalysisFromGUI(int rowSignal)

{
     qDebug()<<"DrawTransparentPoints::callForAnalysisFromGUI(int rowSignal)";

    int row = rowSignal;

    qDebug()<<"WidgetsList.size() "<<widgetList.size();

    if(widgetList.size() == 0){
        analysisHolder = new QWidget();
        holderLayout = new QVBoxLayout;
        analysisHolder->setLayout(holderLayout);
        analysisHolder->showMaximized();
    }

    if(widgetList.size() == 3){

        widgetList.clear();
        delete analysisHolder;

        analysisHolder = new QWidget();
        holderLayout = new QVBoxLayout;
        analysisHolder->setLayout(holderLayout);
        analysisHolder->showMaximized();
    }




    QWidget *actiWidget;
    QHBoxLayout *gLayout;
    actiWidget = new QWidget(analysisHolder);
    gLayout = new QHBoxLayout(actiWidget);
    widgetList.push_back(actiWidget);
    holderLayout->addWidget(actiWidget);

    Mesh_Info* mesh = meshes[row];
    Surf_Data* data = mesh->data;

    if (( checkArray2D(data,data->inversevals)==0) || (checkArray2D(data,data->potvals)==0)|| (checkArray2D(data,data->forwardvals)==0))
    {

        CalculateActivation(mesh);

    }

    if (checkArray1D(data,data->activationvals)==0)

    {
        Surf_Data *s=0;
        s=mesh->data;

        ActivationMapWindow* actiwin;
        actiwin = new ActivationMapWindow(actiWidget);
        actiwin->addMesh(mesh);
        actiwin->setWindowFlags(Qt::WindowTransparentForInput);


        /* create colormap legend window */
        ActivationLegendWindow *lpriv = NULL;
        lpriv = new ActivationLegendWindow(actiWidget);

        int width, height;
        width = mesh->lw_xmax - mesh->lw_xmin;
        height = mesh->lw_ymax - mesh->lw_ymin;


        gLayout->addWidget(lpriv);
        gLayout->setStretch(0,1);
        gLayout->addWidget(actiwin);
        gLayout->setStretch(1,3);


        if ((checkArray1D(data,data->CCvals)==0)&&(checkArray1D(data,data->RMSEvals)==0))
        {
            CCMapWindow* CCwin;
            CCwin = new CCMapWindow(actiWidget);
            CCwin->addMesh(mesh);
            CCwin->setWindowFlags(Qt::WindowTransparentForInput);

            RMSEMapWindow* RMSEwin;
            RMSEwin = new RMSEMapWindow(actiWidget);
            RMSEwin->addMesh(mesh);
            RMSEwin->setWindowFlags(Qt::WindowTransparentForInput);

            gLayout->addWidget(CCwin);
            gLayout->setStretch(2,3);
            gLayout->addWidget(RMSEwin);
            gLayout->setStretch(3,3);
        }
        else
        {
            CCMapWindow* CCwin;
            CCwin = new CCMapWindow(actiWidget);
            RMSEMapWindow* RMSEwin;
            RMSEwin = new RMSEMapWindow(actiWidget);
            gLayout->addWidget(CCwin);
            gLayout->setStretch(2,3);
            gLayout->addWidget(RMSEwin);
            gLayout->setStretch(3,3);
        }

        lpriv->setVisible(true);
        actiwin->show();
        actiWidget->show();


        if (lpriv != NULL) {
            // can fail if more than MAX_SURFS l-wins.
            lpriv->orientation = 1;
            if (mesh->mysurf->legendticks != -1) {
                lpriv->nticks = mesh->mysurf->legendticks;
                lpriv->matchContours = true;
            }

            lpriv->surf = s;
            lpriv->mesh = mesh;
            lpriv->map = &mesh->cmap;


            // background color
            lpriv->bgcolor[0] = mesh->mysurf->colour_bg[0] / 255.f;
            lpriv->bgcolor[1] = mesh->mysurf->colour_bg[1] / 255.f;
            lpriv->bgcolor[2] = mesh->mysurf->colour_bg[2] / 255.f;
            lpriv->fgcolor[0] = mesh->mysurf->colour_fg[0] / 255.f;
            lpriv->fgcolor[1] = mesh->mysurf->colour_fg[1] / 255.f;
            lpriv->fgcolor[2] = mesh->mysurf->colour_fg[2] / 255.f;
        }
    }

    else {

        QMessageBox::warning(this,QString("Warning"),QString("No enough input values for this surface!"));
    }






}



