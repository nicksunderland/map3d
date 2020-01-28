/* GeomWindowRepaint.cxx */

#include <stddef.h>
#ifdef _WIN32
#  include <windows.h>
#  pragma warning(disable:4505)
#  undef TRACE
#else
#  include <unistd.h>
#endif

#ifdef __APPLE__
#  include <OpenGL/glu.h>
#else
#  include <GL/glu.h>
#endif
#include <float.h>
#include <limits.h>
#include <deque>
#include <algorithm>
#include <set>
#include <math.h>

using std::set;

#include "Contour_Info.h"
#include "Map3d_Geom.h"
#include "Surf_Data.h"
#include "drawlandmarks.h"
#include "GeomWindow.h"
#include "WindowManager.h"
#include "LegendWindow.h"
#include "PickWindow.h"
#include "BallMath.h"
#include "Transforms.h"
#include "dialogs.h"
#include "glprintf.h"
#include "colormaps.h"
#include "texture.h"
#include "pickinfo.h"
#include "lock.h"
#include "map3dmath.h"
#include "reportstate.h"
#include "GeomWindowMenu.h"
#include "scalesubs.h"
#include "DrawTransparentPoints.h"

#include <algorithm>
#include <iterator>
#include <QMessageBox>

#include <QApplication>
#include <QDesktopWidget>

#include <iostream>

#include <fstream>
#include <sstream>


#include <vector>
#include <cmath>
#include <tuple>

#include <functional>

#include <boost/foreach.hpp>
#include <boost/range.hpp>     // begin(), end()
#include <boost/tr1/tuple.hpp> // get<>, tuple<>, cout <<

#include <QDebug>
#include <engine.h>
#include <matrix.h>
#include <mex.h>


#define foreach BOOST_FOREACH
using namespace std;


extern Map3d_Info map3d_info;
extern vector<Surface_Group> surf_group;
#define CHAR_WIDTH .07
#define CHAR_HEIGHT .07
extern MainWindow *masterWindow;

extern int unlock_transparency_surfnum[];
extern int unlock_electrode_surfnum[];
extern int unlock_datacloud_surfnum[];
extern int unlock_forward_surfnum[];
extern int unlock_MFS_surfnum[];
extern int unlock_Indeflate_surfnum[];

extern bool plot_nearest_electrode;
extern int unlock_Phase_surfnum[30];

//Engine *ep = engOpen(NULL);

void GeomWindow::paintGL()
{

    int loop = 0;
    int length = meshes.size();

    Mesh_Info *curmesh = 0;

    Mesh_Info *sourcemesh = 0;

    Mesh_Info *recordingmesh = 0;

    /* clear the screen */
    glClearColor(bgcolor[0], bgcolor[1], bgcolor[2], 1);
    glFogfv(GL_FOG_COLOR, bgcolor);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // if it exists, draw a background image
    if (map3d_info.bg_texture)
        DrawBGImage();

    if(gradientBackground == true){                             //Nick - draw simple gradient background to geomWindow
        DrawBackground();
    }

    /* the fog's extent is a per window attribute */
    //printf("fog1 = %f\n",fog1);
    //printf("fog2 = %f\n",fog2);
    glFogf(GL_FOG_START, l2norm * fog1);
    glFogf(GL_FOG_END, l2norm * fog2);

    if (length > 1 && !map3d_info.lockgeneral) {
        loop = dominantsurf;
        length = loop + 1;
    }

#ifdef ROTATING_LIGHT
    // draw rotated light
    HMatrix mNow;
    Ball_Value(&light_pos, mNow);
    glMultMatrixf((float *)mNow);
#endif
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    if (clip->front_enabled)
        glEnable(GL_CLIP_PLANE0);
    else
        glDisable(GL_CLIP_PLANE0);

    if (clip->back_enabled)
        glEnable(GL_CLIP_PLANE1);
    else
        glDisable(GL_CLIP_PLANE1);

    /* render each mesh in this window : OPAQUE PASS */
    for (; loop < length; loop++) {
        /* setup a bunch of variables for quick acces */
        curmesh = (meshes[loop]);

        if (curmesh->lighting)
            glEnable(GL_LIGHTING);
        if (curmesh->fogging)
            glEnable(GL_FOG);

        /* compute the mesh position (rotations, translations, etc.) */
        Transform(curmesh, 0, true);

        Map3d_Geom *curgeom = 0;
        curgeom = curmesh->geom;

        if (curgeom->surfnum==unlock_electrode_surfnum[curgeom->surfnum-1]){
            curmesh->mark_all_sphere=1;
            curmesh->mark_all_sphere_value=1;
            curmesh->mark_all_size=4;
            DrawElectrodesOnly(curmesh);

        }
        else{
            curmesh->mark_all_sphere=0;
            curmesh->mark_all_sphere_value=0;
            DrawElectrodesOnly(curmesh);
        }


        if (curgeom->surfnum==unlock_forward_surfnum[curgeom->surfnum-1]){



            curmesh->mark_all_sphere=1;
            curmesh->mark_all_sphere_value=1;
            curmesh->mark_all_size=4;

            if ((length>1) && (loop+1<length))
            {
                sourcemesh=meshes[loop+1];

                Map3d_Geom *sourcegeom = 0;
                Surf_Data *sourcesurf = 0;
                sourcegeom = sourcemesh->geom;
                sourcesurf = sourcemesh->data;

                if ((sourcegeom->numdatacloud!=0)&&(curmesh->lock_forward!=1))
                {
                    CalculateForwardValue(curmesh,sourcemesh);

                }
            }

            DrawForwardOnly(curmesh);
        }
        else{
            curmesh->mark_all_sphere=0;
            curmesh->mark_all_sphere_value=0;
            // DrawForwardOnly(curmesh);
        }


        if (length>loop+1)
        {



            if ((curgeom->surfnum==unlock_Indeflate_surfnum[curgeom->surfnum-1])||(curgeom->surfnum==unlock_electrode_surfnum[curgeom->surfnum-1])||(curgeom->surfnum==unlock_forward_surfnum[curgeom->surfnum-1]))
            {
                Mesh_Info *sourcemesh = 0;
                sourcemesh=meshes[loop+1];
                UpdateNearestPoints(curmesh,sourcemesh);
            }
        }


        if (curgeom->surfnum==unlock_datacloud_surfnum[curgeom->surfnum-1]){
            curmesh->mark_all_sphere=1;
            curmesh->mark_all_sphere_value=1;
            curmesh->mark_all_size=1;
            DrawDatacloudOnly(curmesh);
        }
        else{
            curmesh->mark_all_sphere=0;
            curmesh->mark_all_sphere_value=0;
            DrawDatacloudOnly(curmesh);
        }


        //       if (curgeom->surfnum==unlock_MFS_surfnum[curgeom->surfnum-1]){



        //            if ((length>1) && (loop-1>=0))
        //            {

        //                recordingmesh=meshes[loop-1];
        //                Map3d_Geom *recordinggeom = 0;
        //                recordinggeom = recordingmesh->geom;
        //                Surf_Data *recordingsurf = 0;
        //                recordingsurf = recordingmesh->data;
        //                CalculateMFSValue(recordingmesh,curmesh);

        //            }

        // std::cout<<"curgeom->surfnum "<< curgeom->surfnum<<std::endl;

        //            std::cout<<"curgeom->surfnum-1 "<< curgeom->surfnum-1<<std::endl;

        //            std::cout<<"unlock_MFS_surfnum[curgeom->surfnum-1] "<< unlock_MFS_surfnum[curgeom->surfnum-1]<<std::endl;


        //            Surf_Data *cursurf = 0;
        //            cursurf = curmesh->data;
        //            cursurf-> inversevals =   cursurf->MFSvals;
        //      }



        /* draw the color mapped surface */
        if (curmesh->shadingmodel != SHADE_NONE && curmesh->geom->points[curmesh->geom->geom_index] && !curmesh->shadefids &&
                curmesh->data && curmesh->drawmesh != RENDER_MESH_ELTS && curmesh->drawmesh != RENDER_MESH_ELTS_CONN) {
            glEnable(GL_POLYGON_OFFSET_FILL);

            //            Map3d_Geom *curgeom = 0;
            //            curgeom = curmesh->geom;

            if ((curgeom->surfnum!=unlock_electrode_surfnum[curgeom->surfnum-1])&& (curgeom->surfnum!= unlock_forward_surfnum[curgeom->surfnum-1]))
            {
                if ((curgeom->surfnum==unlock_transparency_surfnum[curgeom->surfnum-1])&& (curmesh->transparent==1)){

                    glDepthMask(false);
                    glEnable (GL_BLEND);
                    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                }

                if ((curgeom->surfnum==unlock_MFS_surfnum[curgeom->surfnum-1])&&(curgeom->surfnum!=unlock_Phase_surfnum[curgeom->surfnum-1]))
                    DrawMFS(curmesh);
                else if (curgeom->surfnum==unlock_Phase_surfnum[curgeom->surfnum-1])
                {
                   // map3d_info.scale_scope = GLOBAL_GLOBAL_USER_DEFINE;
                   // map3d_info.global_user_potmax=3.14;
                   // map3d_info.global_user_potmin=-3.14;
                    DrawPhase(curmesh);
                }
                else
                    DrawSurf(curmesh);
                glDisable(GL_POLYGON_OFFSET_FILL);
            }



        }

        /* draw fiducial map surface*/
        if (curmesh->data && curmesh->drawfids){
            if(curmesh->drawfidmap < curmesh->fidMaps.size()){
                if (curmesh->shadefids && curmesh->shadingmodel != SHADE_NONE){
                    glEnable(GL_POLYGON_OFFSET_FILL);
                    DrawFidMapSurf(curmesh,curmesh->fidMaps[curmesh->drawfidmap]);
                    glDisable(GL_POLYGON_OFFSET_FILL);
                }
            }
        }

        /* draw the mesh */
        if (curmesh->drawmesh && curmesh->geom->points[curmesh->geom->geom_index]) {
            glEnable(GL_POLYGON_OFFSET_FILL);
            if (curmesh->drawmesh == RENDER_MESH_ELTS || curmesh->drawmesh == RENDER_MESH_NONDATA_ELTS) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
            else if (curmesh->drawmesh == RENDER_MESH_ELTS_CONN) {
                glPolygonMode(GL_FRONT, GL_FILL);
                glPolygonMode(GL_BACK, GL_LINE);
            }
            else if (curmesh->drawmesh >= RENDER_MESH_ELTS)
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

            //if it is secondary mesh draw in secondary color
            if (loop == secondarysurf && length > 1)
                DrawMesh(curmesh, 1);
            else
                DrawMesh(curmesh, 0);

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDisable(GL_POLYGON_OFFSET_FILL);
        }

        glDisable(GL_LIGHTING);

        /* draw the contour lines */
        if (curmesh->drawcont && curmesh->data)
            DrawCont(curmesh);

        /* draw the fiducial contour lines */
        if (curmesh->data && curmesh->drawfids){
            for(unsigned i = 0; i<curmesh->fidConts.size();i++){
                if (curmesh->drawFidConts[i])
                    DrawFidCont(curmesh,curmesh->fidConts[i]);
            }
            if(curmesh->drawfids && curmesh->drawfidmap < curmesh->fidMaps.size() && !curmesh->drawcont){
                DrawFidMapCont(curmesh,curmesh->fidMaps[curmesh->drawfidmap]);
            }
        }

        glDisable(GL_FOG);
    }

    int start = 0;
    loop = 0;
    length = meshes.size();
    if (length > 1 && !map3d_info.lockgeneral) {
        loop = dominantsurf;
        length = loop + 1;
        start = loop;
    }

    /* render each mesh in this window : TRANSPARENT PASS */
    for (; loop < length; loop++) {
        /* setup a bunch of variables for quick access */
        curmesh = (meshes[loop]);

        /* compute the mesh position (rotations, translations, etc.) */
        Transform(curmesh, 0, true);

        if (curmesh->fogging)
            glEnable(GL_FOG);

        /* draw all nodes (and extrema markings) */
        if (curmesh->qshowpnts)
            DrawNodes(curmesh);

        //glDisable(GL_FOG);
        //draw all axes or one per window, based on all_axes
        if (curmesh->axes && (all_axes || start == loop)) {
            DrawAxes(curmesh);
        }
        glDisable(GL_FOG);

        /* draw the landmarks */
        if (curmesh->lighting)
            glEnable(GL_LIGHTING);
        if (curmesh->fogging)
            glEnable(GL_FOG);

        if (curmesh->geom->landmarks && curmesh->landmarkdraw.qshowlmark)
            DrawLandMarks(curmesh->geom->landmarks, &curmesh->landmarkdraw, this);

        glDisable(GL_LIGHTING);
        glDisable(GL_FOG);
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    /* print the window's info text */
    if (showinfotext && !clip->back_enabled && !clip->front_enabled) {
        DrawInfo();
    }

    /* draw lock icons */
    if (showlocks && !clip->back_enabled && !clip->front_enabled)
    {
        if (map3d_info.lockgeneral != LOCK_OFF)
            DrawLockSymbol(0, map3d_info.lockgeneral == LOCK_FULL);
        if (map3d_info.lockrotate != LOCK_OFF)
            DrawLockSymbol(1, map3d_info.lockrotate == LOCK_FULL);
        if (map3d_info.lockframes != LOCK_OFF)
            DrawLockSymbol(2, map3d_info.lockframes == LOCK_FULL);
    }




#if SHOW_OPENGL_ERRORS
    GLenum e = glGetError();
    if (e)
        printf("GeomWindow Repaint OpenGL Error: %s\n", gluErrorString(e));
#endif
}


void GeomWindow::DrawElectrodesOnly(Mesh_Info * curmesh) //show the catheters only in nodes, not in surface ,shu meng
{

    int length = 0, loop = 0;
    float min = 0, max = 0, value = 0;
    float mNowI[16];
    float **modelpts = 0;
    Map3d_Geom *curgeom = 0;
    Surf_Data *cursurf = 0;
    HMatrix mNow;

    ColorMap *curmap = 0;
    curgeom = curmesh->geom;
    cursurf = curmesh->data;

    modelpts = curgeom->points[curgeom->geom_index];

    if (cursurf) {
        curmap = curmesh->cmap;
    }
    length = curgeom->numpts;

    /* set the transform for billboarding */
    Ball_Value(&curmesh->tran->rotate, mNow);
    TransposeMatrix16((float *)mNow, mNowI);
    Transform(curmesh, 0.01f, true);
    glPushMatrix();

    if (cursurf)
        cursurf->get_minmax(min, max);
    unsigned char color[3];

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GEQUAL, .4f);
    float sphere_size = 1.0f;
    for (loop = 0; loop < length; loop++) {
        if (cursurf ) {
            value = cursurf->potvals[cursurf->framenum][loop];
            getContColor(value, min, max, curmap, color, curmesh->invert);
        }

        if (curmesh->mark_all_sphere) {
            if (curmesh->mark_all_sphere_value && cursurf && value != UNUSED_DATA) {
                glColor3ubv(color);
            }
            else {
                glColor3f(curmesh->mark_all_color[0], curmesh->mark_all_color[1], curmesh->mark_all_color[2]);
            }
            glPointSize(height() / 200 * curmesh->mark_all_size);
            sphere_size = curmesh->mark_all_size;
        }
        else {
            continue;
        }
        glTranslatef(modelpts[loop][0], modelpts[loop][1], modelpts[loop][2]);
        glMultMatrixf((float *)mNowI);
        glTranslatef(-modelpts[loop][0], -modelpts[loop][1], -modelpts[loop][2]);

        // try to convert the sphere size from geometry units to pixels
        // 400 is a good number to use to normalize the l2norm
        sphere_size = sphere_size*l2norm/400;
        if (curmesh->draw_marks_as_spheres)
            DrawDot(modelpts[loop][0], modelpts[loop][1], modelpts[loop][2], sphere_size);
        else {
            glBegin(GL_POINTS);
            glVertex3f(modelpts[loop][0], modelpts[loop][1], modelpts[loop][2]);
            glEnd();
        }
        glPopMatrix();
        glPushMatrix();
    }

    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);


    glPopMatrix();

#if SHOW_OPENGL_ERRORS
    GLenum e = glGetError();
    if (e)
        printf("GeomWindow DrawNodes OpenGL Error: %s\n", gluErrorString(e));
#endif
}


void GeomWindow::DrawForwardOnly(Mesh_Info * curmesh) //show the catheters only in nodes, not in surface ,shu meng
{



    int length = 0, loop = 0;
    float min = 0, max = 0, value = 0;
    float mNowI[16];
    float **modelpts = 0;


    Map3d_Geom *curgeom = 0;
    Surf_Data *cursurf = 0;
    HMatrix mNow;

    ColorMap *curmap = 0;
    curgeom = curmesh->geom;
    cursurf = curmesh->data;

    modelpts = curgeom->points[curgeom->geom_index];

    if (cursurf) {
        curmap = curmesh->cmap;
    }
    length = curgeom->numpts;

    /* set the transform for billboarding */
    Ball_Value(&curmesh->tran->rotate, mNow);
    TransposeMatrix16((float *)mNow, mNowI);
    Transform(curmesh, 0.01f, true);
    glPushMatrix();

    if (cursurf)
        cursurf->get_minmax(min, max);
    unsigned char color[3];

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GEQUAL, .4f);
    float sphere_size = 1.0f;
    for (loop = 0; loop < length; loop++) {
        if (cursurf ) {
            value = cursurf->forwardvals[cursurf->framenum][loop];
            getContColor(value, min, max, curmap, color, curmesh->invert);
        }

        if (curmesh->mark_all_sphere) {
            if (curmesh->mark_all_sphere_value && cursurf && value != UNUSED_DATA) {
                glColor3ubv(color);
            }
            else {
                glColor3f(curmesh->mark_all_color[0], curmesh->mark_all_color[1], curmesh->mark_all_color[2]);
            }
            glPointSize(height() / 200 * curmesh->mark_all_size);
            sphere_size = curmesh->mark_all_size;
        }
        else {
            continue;
        }
        glTranslatef(modelpts[loop][0], modelpts[loop][1], modelpts[loop][2]);
        glMultMatrixf((float *)mNowI);
        glTranslatef(-modelpts[loop][0], -modelpts[loop][1], -modelpts[loop][2]);

        // try to convert the sphere size from geometry units to pixels
        // 400 is a good number to use to normalize the l2norm
        sphere_size = sphere_size*l2norm/400;
        if (curmesh->draw_marks_as_spheres)
        {
            DrawDot(modelpts[loop][0], modelpts[loop][1], modelpts[loop][2], sphere_size);

        }
        else {
            glBegin(GL_POINTS);
            glVertex3f(modelpts[loop][0], modelpts[loop][1], modelpts[loop][2]);
            glEnd();
        }
        glPopMatrix();
        glPushMatrix();
    }


    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);


    glPopMatrix();

#if SHOW_OPENGL_ERRORS
    GLenum e = glGetError();
    if (e)
        printf("GeomWindow DrawNodes OpenGL Error: %s\n", gluErrorString(e));
#endif
}










void GeomWindow::CalculateForwardValue(Mesh_Info * curmesh, Mesh_Info * sourcemesh)
{

    //std::cout<<"enter CalculateForwardValue  "<<std::endl;


    int length1 = 0, loop1 = 0, length2 =0, loop2=0, loop_idx=0;

    Map3d_Geom *curgeom = 0;
    Surf_Data *cursurf = 0;
    curgeom = curmesh->geom;
    cursurf = curmesh->data;
    length1 = curgeom->numpts;


    Map3d_Geom *sourcegeom = 0;
    Surf_Data *sourcesurf = 0;
    sourcegeom = sourcemesh->geom;
    sourcesurf = sourcemesh->data;
    length2 = sourcegeom->numdatacloud;


    // this part is to rotate the catheter. if map3d_info.lockrotate==LOCK_OFF, only apply transform matrix to catheter
    // if map3d_info.lockrotate==LOCK_FULL, apply both transform matrix to catheter and atrium, corresponding matrix is different.
    float** pts = curgeom->points[curgeom->geom_index];
    float** geom_temp_forward_pts=pts;
    float **rotated_forward_pts = 0;
    rotated_forward_pts= Alloc_fmatrix(curgeom->numpts, 3);

    GeomWindow* priv_forward = curmesh->gpriv;
    HMatrix mNow_forward /*, original */ ;  // arcball rotation matrices
    Transforms *tran_forward = curmesh->tran;
    //translation matrix in column-major
    float centerM_forward[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},
                                   {-priv_forward->xcenter,-priv_forward->ycenter,-priv_forward->zcenter,1}};
    float invCenterM_forward[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},
                                      {priv_forward->xcenter,priv_forward->ycenter,priv_forward->zcenter,1}};
    float translateM_forward[4][4] = { {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0},
                                       {tran_forward->tx, tran_forward->ty, tran_forward->tz, 1}
                                     };
    float temp_forward[16];
    float product_forward[16];

    //rotation matrix
    Ball_Value(&tran_forward->rotate, mNow_forward);
    // apply translation
    // translate curmesh's center to origin
    MultMatrix16x16((float *)translateM_forward, (float *)invCenterM_forward, (float*)product_forward);
    // rotate
    MultMatrix16x16((float *)product_forward, (float *)mNow_forward, (float*)temp_forward);
    // revert curmesh translation to origin
    MultMatrix16x16((float*)temp_forward, (float *) centerM_forward, (float*)product_forward);



    for (loop1 = 0; loop1 < length1; loop1++)
    {

        float rhs_forward[4];
        float result_forward[4];
        rhs_forward[0] = pts[loop1][0];
        rhs_forward[1] = pts[loop1][1];
        rhs_forward[2] = pts[loop1][2];
        rhs_forward[3] = 1;

        MultMatrix16x4(product_forward, rhs_forward, result_forward);

        rotated_forward_pts[loop1][0] = result_forward[0];
        rotated_forward_pts[loop1][1] = result_forward[1];
        rotated_forward_pts[loop1][2] = result_forward[2];
    }

    geom_temp_forward_pts=rotated_forward_pts;

    //this part is to rotate the source surface (atrium).transform matrix is not applied if map3d_info.lockrotate==LOCK_OFF

    float** pts_source = sourcegeom->datacloud[sourcegeom->geom_index];
    float** geom_temp_source_pts=pts_source;
    float **rotated_source_pts = 0;
    rotated_source_pts= Alloc_fmatrix(sourcegeom->numdatacloud, 3);


    GeomWindow* priv_source = sourcemesh->gpriv;
    HMatrix mNow_source /*, original */ ;  // arcball rotation matrices
    Transforms *tran_source = sourcemesh->tran;
    //translation matrix in column-major
    float centerM_source[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},
                                  {-priv_source->xcenter,-priv_source->ycenter,-priv_source->zcenter,1}};
    float invCenterM_source[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},
                                     {priv_source->xcenter,priv_source->ycenter,priv_source->zcenter,1}};
    float translateM_source[4][4] = { {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0},
                                      {tran_source->tx, tran_source->ty, tran_source->tz, 1}
                                    };
    float temp_source[16];
    float product_source[16];

    //rotation matrix
    Ball_Value(&tran_source->rotate, mNow_source);
    // apply translation
    // translate sourcemesh's center to origin
    MultMatrix16x16((float *)translateM_source, (float *)invCenterM_source, (float*)product_source);
    // rotate
    MultMatrix16x16((float *)product_source, (float *)mNow_source, (float*)temp_source);
    // revert sourcemesh translation to origin
    MultMatrix16x16((float*)temp_source, (float *) centerM_source, (float*)product_source);


    for (loop2 = 0; loop2 < length2; loop2++)
    {

        float rhs_source[4];
        float result_source[4];
        rhs_source[0] = pts_source[loop2][0];
        rhs_source[1] = pts_source[loop2][1];
        rhs_source[2] = pts_source[loop2][2];
        rhs_source[3] = 1;

        MultMatrix16x4(product_source, rhs_source, result_source);

        rotated_source_pts[loop2][0] = result_source[0];
        rotated_source_pts[loop2][1] = result_source[1];
        rotated_source_pts[loop2][2] = result_source[2];

    }
    geom_temp_source_pts=rotated_source_pts;


    vector<point_t> data_points;
    for (loop2 = 0; loop2 < length2; loop2++)
    {
        coord_t x,y,z;
        x = geom_temp_source_pts[loop2][0];
        y = geom_temp_source_pts[loop2][1];
        z = geom_temp_source_pts[loop2][2];
        data_points.push_back(tr1::make_tuple(x,y,z));
    }


    const size_t nneighbours = 1; // number of nearest neighbours to find
    point_t points[nneighbours];


    float **nearest_index = 0;
    nearest_index= Alloc_fmatrix(curgeom->numpts, 1);



    //        string filename;
    //        ofstream files;
    //        stringstream a;
    //        a << cursurf->framenum;
    //        filename = "forward_64_" + a.str();
    //        filename += ".txt";
    //        files.open(filename.c_str(), ios::out);


    for (loop1 = 0; loop1 < length1; loop1++)
    {
        point_t point(geom_temp_forward_pts[loop1][0],geom_temp_forward_pts[loop1][1], geom_temp_forward_pts[loop1][2]);

        less_distance nearer(point);

        foreach (point_t& p, points)

            for (loop2 = 0; loop2 < length2; loop2++)
            {
                point_t current_point(geom_temp_source_pts[loop2][0],geom_temp_source_pts[loop2][1],geom_temp_source_pts[loop2][2]);
                foreach (point_t& p, points)

                    if (nearer(current_point, p))
                        std::swap(current_point, p);
            }

        sort(boost::begin(points), boost::end(points), nearer);
        foreach (point_t p, points)

        {
            for (loop_idx = 0; loop_idx< length2; loop_idx++)
            {
                if (p == data_points[loop_idx])
                {


                    for (int loop_frame = 0; loop_frame <sourcemesh->data->numframes; loop_frame++){

                        cursurf->forwardvals[loop_frame][loop1]= sourcesurf->datacloudvals[loop_frame][loop_idx];
                        nearest_index[loop1][0]=loop_idx;
                        // cout<<"index "<<loop1<<"frame "<< cursurf->framenum<<" forward values   "<<cursurf->forwardvals[cursurf->framenum][loop1]<<std::endl;
                        //                    cout<<loop_idx <<std::endl;

                        //                    files << cursurf->forwardvals[cursurf->framenum][loop1] << " " ;
                        //                    files << "\n";
                    }
                }
            }
        }

    }

}




void GeomWindow::DrawDatacloudOnly(Mesh_Info * curmesh) //show the catheters only in nodes, not in surface ,shu meng
{
    int length = 0, loop = 0;
    float min = 0, max = 0, value = 0;
    float mNowI[16];
    float **modelpts = 0;
    Map3d_Geom *curgeom = 0;
    Surf_Data *cursurf = 0;
    HMatrix mNow;

    ColorMap *curmap = 0;
    curgeom = curmesh->geom;
    cursurf = curmesh->data;

    modelpts = curgeom->datacloud[curgeom->geom_index];

    if (cursurf) {
        curmap = curmesh->cmap;
    }

    length = curgeom->numdatacloud;

    /* set the transform for billboarding */
    Ball_Value(&curmesh->tran->rotate, mNow);
    TransposeMatrix16((float *)mNow, mNowI);
    Transform(curmesh, 0.01f, true);
    glPushMatrix();

    if (cursurf)
        cursurf->get_minmax(min, max);
    unsigned char color[3];

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GEQUAL, .4f);
    float sphere_size = 1.0f;

    for (loop = 0; loop < length; loop++) {
        if (cursurf ) {
            value = cursurf->datacloudvals[cursurf->framenum][loop];
            getContColor(value, min, max, curmap, color, curmesh->invert);

        }

        if (curmesh->mark_all_sphere) {
            if (curmesh->mark_all_sphere_value && cursurf && value != UNUSED_DATA) {
                glColor3ubv(color);
            }
            else {
                glColor3f(curmesh->mark_all_color[0], curmesh->mark_all_color[1], curmesh->mark_all_color[2]);
            }
            glPointSize(height() / 200 * curmesh->mark_all_size);
            sphere_size = curmesh->mark_all_size;
        }
        else {
            continue;
        }
        glTranslatef(modelpts[loop][0], modelpts[loop][1], modelpts[loop][2]);
        glMultMatrixf((float *)mNowI);
        glTranslatef(-modelpts[loop][0], -modelpts[loop][1], -modelpts[loop][2]);

        // try to convert the sphere size from geometry units to pixels
        // 400 is a good number to use to normalize the l2norm
        sphere_size = sphere_size*l2norm/400;
        if (curmesh->draw_marks_as_spheres)
        {
            DrawDot(modelpts[loop][0], modelpts[loop][1], modelpts[loop][2], sphere_size);

        }

        else {
            glBegin(GL_POINTS);
            glVertex3f(modelpts[loop][0], modelpts[loop][1], modelpts[loop][2]);
            glEnd();
        }
        glPopMatrix();
        glPushMatrix();


    }

    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);


    glPopMatrix();

#if SHOW_OPENGL_ERRORS
    GLenum e = glGetError();
    if (e)
        printf("GeomWindow DrawNodes OpenGL Error: %s\n", gluErrorString(e));
#endif
}

void DrawSurf(Mesh_Info * curmesh)
{

    int curframe = 0;
    int length = 0;
    int index;
    int loop2, loop3;
    float a = 1, b = 0;
    float mean = 0;
    float **modelpts = 0;
    float **ptnormals = 0;
    float **fcnormals = 0;
    Map3d_Geom *curgeom = 0;
    Surf_Data *cursurf = 0;
    Contour_Info *curcont = 0;


    curgeom = curmesh->geom;
    cursurf = curmesh->data;

    modelpts = curgeom->points[curgeom->geom_index];
    ptnormals = curgeom->ptnormals;
    fcnormals = curgeom->fcnormals;

    if (cursurf) {
        curframe = cursurf->framenum;
        curcont = curmesh->cont;
        //if (cursurf->minmaxframes)
        //compute_mapping(curmesh, cursurf, a, b);
    }

    if ((int)a == INT_MAX || (int)b == INT_MAX) //change by BJW to avoid crash
        a = b = 0;                  //when there is no data

    float potmin, potmax;
    cursurf->get_minmax(potmin, potmax);

    if (map3d_info.scale_mapping == SYMMETRIC) {
        if (fabs(potmax) > fabs(potmin))
            potmin = -potmax;
        else
            potmax = -potmin;
    }
    if (map3d_info.scale_mapping == SEPARATE) {
        if (potmax < 0)
            potmax = 0;
        if (potmin > 0)
            potmin = 0;
    }

    unsigned char color[3];

    //band shading
    if (curmesh->shadingmodel == SHADE_BANDED && curcont) {

        if (curmesh->lighting)
            glShadeModel(GL_SMOOTH);

        float** pts;
        float** normals;
        float potval;

        for (loop2 = 0; loop2 < curcont->numbandpolys; loop2++) {

            length = curcont->bandpolys[loop2].numpts;
            pts = curcont->bandpolys[loop2].nodes;
            normals = curcont->bandpolys[loop2].normals;

            int contnum = curcont->bandpolys[loop2].bandcol;
            float* conts = curcont->isolevels;
            if (contnum == -1)
                potval = potmin;
            else if (contnum == curcont->numlevels - 1)
                potval = potmax;
            else {
                potval = conts[contnum] + (conts[contnum+1]-conts[contnum])*(contnum+1)/(curcont->numlevels);
            }
            getContColor(potval, potmin, potmax, curmesh->cmap, color, curmesh->invert);
            glColor4ub(color[0],color[1],color[2],color[3]);
            glBegin(GL_POLYGON);
            for (loop3 = 0; loop3 < length; loop3++) {
                glNormal3fv(normals[loop3]);
                glVertex3fv(pts[loop3]);
            }
            glNormal3fv(normals[0]);
            glVertex3fv(pts[0]);
            glEnd();
        }

        if (curmesh->lighting)
            glEnable(GL_LIGHTING);
    }

    bool use_textures = false;
    // gouraud shading
    if (curmesh->shadingmodel == SHADE_GOURAUD) {

        glShadeModel(GL_SMOOTH);
        use_textures = curmesh->gouraudstyle == SHADE_TEXTURED &&
                (curmesh->cmap->type == RAINBOW_CMAP || curmesh->cmap->type == JET_CMAP);

        if (use_textures) {
            glColor4f(1,1,1,0.4);
            glEnable(GL_TEXTURE_1D);
            if (curmesh->cmap->type == RAINBOW_CMAP)
                UseTexture(map3d_info.rainbow_texture);
            else
                UseTexture(map3d_info.jet_texture);
        }
    }
    else if (curmesh->shadingmodel == SHADE_FLAT){
        glShadeModel(GL_FLAT);
        glColor4ub(color[0],color[1],color[2],color[3]);
    }

    if (curgeom->elementsize == 3 && curmesh->shadingmodel != SHADE_BANDED) {
        length = curgeom->numelements;
        glBegin(GL_TRIANGLES);

        float potval;
        for (loop2 = 0; loop2 < length; loop2++) {
            if (curmesh->shadingmodel == SHADE_GOURAUD) {
                // avoid repeating code 3 times
                for (loop3 = 0; loop3 < 3; loop3++) {
                    index = curgeom->elements[loop2][loop3];
                    if (cursurf->potvals[curframe][index] == UNUSED_DATA)
                        break;
                }
                if (loop3 < 3)
                    // we have "UNUSED_DATA" on a node in this triangle, so don't draw here
                    continue;
                for (loop3 = 0; loop3 < 3; loop3++) {
                    index = curgeom->elements[loop2][loop3];
                    potval = cursurf->potvals[curframe][index];

                    if (use_textures)
                    {
                        glColor4f(1,1,1,0.4);
                        glEnable(GL_TEXTURE_1D);
                        if (curmesh->cmap->type == RAINBOW_CMAP){
                            UseTexture(map3d_info.rainbow_texture);
                        }
                        else
                            UseTexture(map3d_info.jet_texture);
                        glTexCoord1f(getContNormalizedValue(potval, potmin, potmax, curmesh->invert));
                    }

                    else {
                        getContColor(potval, potmin, potmax, curmesh->cmap, color, curmesh->invert);

                        glColor4ub(color[0],color[1],color[2],color[3]);
                    }
                    glNormal3fv(ptnormals[index]);
                    glVertex3fv(modelpts[index]);
                }
            }
            else {
                mean = 0;
                for (loop3 = 0; loop3 < 3; loop3++) {
                    index = curgeom->elements[loop2][loop3];
                    potval = cursurf->potvals[curframe][index];
                    if (potval == UNUSED_DATA)
                        break;
                    mean += potval;
                }
                if (loop3 < 3)
                    // we have "UNUSED_DATA" on a node in this triangle, so don't draw here
                    continue;

                mean /= 3;
                getContColor(mean, potmin, potmax, curmesh->cmap, color, curmesh->invert);
                glColor4ub(color[0],color[1],color[2],color[3]);
                glNormal3fv(fcnormals[loop2]);

                for (loop3 = 0; loop3 < 3; loop3++) {
                    index = curgeom->elements[loop2][loop3];
                    glVertex3fv(modelpts[index]);
                }
            }
        }
        glEnd();
    }
    else if (curgeom->elementsize == 4 && curmesh->shadingmodel != SHADE_BANDED) {

        length = curgeom->numelements;
        glBegin(GL_TRIANGLES);
        for (loop2 = 0; loop2 < length; loop2++) {
            // this inner loop is a hack to avoid repeating the
            // following index/glNormal/glVertex code 4 times
            for (loop3 = 0; loop3 < 4; loop3++) {
                int idx1, idx2, idx3;
                if (loop3 == 3) {
                    idx1 = 1;
                    idx2 = 3;
                    idx3 = 2;
                }
                else if (loop3 == 2) {
                    idx1 = 0;
                    idx2 = loop3;
                    idx3 = 1;
                }
                else {
                    idx1 = 0;
                    idx2 = loop3;
                    idx3 = loop2 + 1;
                }
                if (curmesh->shadingmodel == SHADE_GOURAUD) {
                    index = curgeom->elements[loop2][idx1];
                    glNormal3fv(ptnormals[index]);
                    glVertex3fv(modelpts[index]);
                    index = curgeom->elements[loop2][idx2];
                    glNormal3fv(ptnormals[index]);
                    glVertex3fv(modelpts[index]);
                    index = curgeom->elements[loop2][idx3];
                    glNormal3fv(ptnormals[index]);
                    glVertex3fv(modelpts[index]);
                }
                else {
                    glNormal3fv(fcnormals[loop2]);
                    glVertex3fv(modelpts[curgeom->elements[loop2][idx1]]);
                    glVertex3fv(modelpts[curgeom->elements[loop2][idx2]]);
                    glVertex3fv(modelpts[curgeom->elements[loop2][idx3]]);
                }
            }
        }

        glEnd();
    }



    glDisable(GL_TEXTURE_1D);
#if SHOW_OPENGL_ERRORS
    GLenum e = glGetError();
    if (e)
        printf("GeomWindow DrawSurf OpenGL Error: %s\n", gluErrorString(e));
#endif
}


void DrawMesh(Mesh_Info * curmesh, bool secondary)
{
    //  int curframe = 0;
    int length = 0;
    int index;
    int loop2, loop3;
    float **modelpts = 0;
    float **ptnormals = 0;
    float **fcnormals = 0;
    //  float **contpts1 = 0;
    //  float **contpts2 = 0;
    //  unsigned char* curmap = 0;
    Map3d_Geom *curgeom = 0;
    //  Contour_Info* curcont = 0;
    //  Transforms* curtran = 0;

    curgeom = curmesh->geom;

    // draw points if we are in points mode or a PTS_ONLY geom and not in no-render mode
    bool drawpts = curmesh->drawmesh == RENDER_MESH_PTS || curmesh->drawmesh == RENDER_MESH_PTS_CONN ||
            (curmesh->drawmesh >= RENDER_MESH_ELTS && !curgeom->elements);

    modelpts = curgeom->points[curgeom->geom_index];
    ptnormals = curgeom->ptnormals;
    fcnormals = curgeom->fcnormals;

    glLineWidth(curmesh->meshsize);
    glPointSize(curmesh->meshsize);

    if (secondary)
        glColor3fv(curmesh->secondarycolor);
    else
        glColor3fv(curmesh->meshcolor);


    if (curmesh->drawmesh >= RENDER_MESH_ELTS && curgeom->elementsize == 4) {
        //drawpts = false;
        length = curgeom->numelements;

        if (curmesh->shadingmodel == SHADE_GOURAUD)
            glShadeModel(GL_SMOOTH);
        else
            glShadeModel(GL_FLAT);

        glBegin(GL_TRIANGLES);

        for (loop2 = 0; loop2 < length; loop2++) {
            // this inner loop is  to avoid repeating the
            // following index/glNormal/glVertex code 4 times
            for (loop3 = 0; loop3 < 4; loop3++) {
                int idx1, idx2, idx3;
                if (loop3 == 3) {
                    idx1 = 1;
                    idx2 = 3;
                    idx3 = 2;
                }
                else if (loop3 == 2) {
                    idx1 = 0;
                    idx2 = loop3;
                    idx3 = 1;
                }
                else {
                    idx1 = 0;
                    idx2 = loop3;
                    idx3 = loop2 + 1;
                }
                if (curmesh->shadingmodel == SHADE_GOURAUD) {
                    index = curgeom->elements[loop2][idx1];
                    glNormal3fv(ptnormals[index]);
                    glVertex3fv(modelpts[index]);
                    index = curgeom->elements[loop2][idx2];
                    glNormal3fv(ptnormals[index]);
                    glVertex3fv(modelpts[index]);
                    index = curgeom->elements[loop2][idx3];
                    glNormal3fv(ptnormals[index]);
                    glVertex3fv(modelpts[index]);
                }
                else {
                    index = curgeom->elements[loop2][idx1];
                    glNormal3fv(fcnormals[index]);
                    glVertex3fv(modelpts[index]);
                    index = curgeom->elements[loop2][idx2];
                    glVertex3fv(modelpts[index]);
                    index = curgeom->elements[loop2][idx3];
                    glVertex3fv(modelpts[index]);
                }
            }

        }
        glEnd();
    }
    else if (curmesh->drawmesh >= RENDER_MESH_ELTS && curgeom->elementsize == 3) {
        length = curgeom->numelements;

        //std::cout<<"12-06-2018 elementsize == 3"<<std::endl;
        // std::cout<<"12-06-2018 the length is (numelements)"<<length<<std::endl;

        if (curmesh->shadingmodel != SHADE_FLAT)
            glShadeModel(GL_SMOOTH);
        else
            glShadeModel(GL_FLAT);
        glBegin(GL_TRIANGLES);

        for (loop2 = 0; loop2 < length; loop2++) {
            // if we have no data on at least one node in draw-nondata-mode, then draw
            if (curmesh->drawmesh == RENDER_MESH_NONDATA_ELTS && !(curgeom->channels[curgeom->elements[loop2][0]] <= -1 ||
                                                                   curgeom->channels[curgeom->elements[loop2][1]] <= -1 || curgeom->channels[curgeom->elements[loop2][2]] <= -1))
                continue;
            if (curmesh->shadingmodel != SHADE_FLAT) {
                index = curgeom->elements[loop2][0];
                glNormal3fv(ptnormals[index]);
                glVertex3fv(modelpts[index]);
                index = curgeom->elements[loop2][1];
                glNormal3fv(ptnormals[index]);
                glVertex3fv(modelpts[index]);
                index = curgeom->elements[loop2][2];
                glNormal3fv(ptnormals[index]);
                glVertex3fv(modelpts[index]);
            }
            else {
                glNormal3fv(fcnormals[loop2]);
                glVertex3fv(modelpts[curgeom->elements[loop2][0]]);
                glVertex3fv(modelpts[curgeom->elements[loop2][1]]);
                glVertex3fv(modelpts[curgeom->elements[loop2][2]]);
            }
        }

        glEnd();
    }
    else if (curmesh->drawmesh >= RENDER_MESH_ELTS  && curgeom->elementsize == 2) {
        drawpts = false;
        length = curgeom->numelements;

        glDisable(GL_LIGHTING);
        glBegin(GL_LINES);

        for (loop2 = 0; loop2 < length; loop2++) {
            glVertex3fv(modelpts[curgeom->elements[loop2][0]]);
            glVertex3fv(modelpts[curgeom->elements[loop2][1]]);
        }

        glEnd();

        if (curmesh->lighting)
            glEnable(GL_LIGHTING);

    }
    if (drawpts) {

        length = curgeom->numpts;
        glDisable(GL_LIGHTING);
        glBegin(GL_POINTS);
        if (curmesh->shadingmodel != SHADE_NONE)
            glColor3f(1., 1., 1.);

        for (loop2 = 0; loop2 < length; loop2++)
            glVertex3fv(modelpts[loop2]);

        glEnd();
        if (curmesh->lighting)
            glEnable(GL_LIGHTING);
    }
#if SHOW_OPENGL_ERRORS
    GLenum e = glGetError();
    if (e)
        printf("GeomWindow DrawMesh OpenGL Error: %s\n", gluErrorString(e));
#endif
}



void DrawCont(Mesh_Info * curmesh)
{
    int length = 0;
    int loop2;
    float **contpts1 = 0;
    float **contpts2 = 0;
    Surf_Data *cursurf = 0;
    Contour_Info *curcont = 0;

    cursurf = curmesh->data;

    if (cursurf) {
        curcont = curmesh->cont;
    }

    float potmin, potmax;
    cursurf->get_minmax(potmin, potmax);

    if (map3d_info.scale_mapping == SYMMETRIC) {
        if (fabs(potmax) > fabs(potmin))
            potmin = -potmax;
        else
            potmax = -potmin;
    }

    if (map3d_info.scale_mapping == SEPARATE) {
        if (potmax < 0)
            potmax = 0;
        if (potmin > 0)
            potmin = 0;
    }

    unsigned char color[3];

    if (curcont) {
        contpts1 = curcont->contpt1;
        contpts2 = curcont->contpt2;
        length = curcont->numisosegs;

    }

    glLineWidth(curmesh->contsize);
    glPointSize(curmesh->contsize);


    float potval;
    for (loop2 = 0; loop2 < length; loop2++) {
        potval = curcont->isolevels[curcont->contcol[loop2]];
        getContColor(potval, potmin, potmax, curmesh->cmap, color, curmesh->invert);

        if(map3d_info.contour_antialiasing){
            glEnable (GL_LINE_SMOOTH);
            glEnable (GL_BLEND);
            glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
        }

        if (curmesh->shadingmodel == SHADE_NONE)
            glColor3ubv(color);
        else
            glColor3ub(0,0,0);
        if (curcont->isolevels[curcont->contcol[loop2]] < 0 && curmesh->negcontdashed)
            glEnable(GL_LINE_STIPPLE);

        glBegin(GL_LINES);
        glVertex3fv(contpts1[loop2]);
        glVertex3fv(contpts2[loop2]);
        glEnd();
        glDisable(GL_LINE_STIPPLE);
        if(map3d_info.contour_antialiasing){
            glDisable (GL_LINE_SMOOTH);
            glDisable (GL_BLEND);
        }

    }
#if SHOW_OPENGL_ERRORS
    GLenum e = glGetError();
    if (e)
        printf("GeomWindow DrawCont OpenGL Error: %s\n", gluErrorString(e));
#endif
}

void DrawFidMapCont(Mesh_Info * curmesh, Contour_Info *cont)
{
    int length = 0;
    int loop2;
    float **contpts1 = 0;
    float **contpts2 = 0;
    Surf_Data *cursurf = 0;
    Contour_Info *curcont = 0;

    cursurf = curmesh->data;

    if (cursurf) {
        curcont = cont;    //used to be curframe instead of 0
        //if (cursurf->minmaxframes)
        //compute_mapping(curmesh, cursurf, a, b);
    }

    float fidmin, fidmax;
    cursurf->get_fid_minmax(fidmin, fidmax, cont->datatype);

    //   if (map3d_info.scale_mapping == SYMMETRIC) {
    //     if (fabs(potmax) > fabs(potmin))
    //       potmin = -potmax;
    //     else
    //       potmax = -potmin;
    //   }

    //   if (map3d_info.scale_mapping == SEPARATE) {
    //     if (potmax < 0)
    //       potmax = 0;
    //     if (potmin > 0)
    //       potmin = 0;
    //   }

    unsigned char color[3];

    if (curcont) {
        contpts1 = curcont->contpt1;
        contpts2 = curcont->contpt2;
    }

    glLineWidth(curmesh->contsize);
    glPointSize(curmesh->contsize);

    length = curcont->numisosegs;
    //printf("length = %d\n", length);
    //printf("levels = %d\n", curcont->numlevels);


    //  if (curmesh->shadingmodel != SHADE_NONE) {
    float fidval;
    for (loop2 = 0; loop2 < length; loop2++) {
        //printf("curcont->contcol[%d] = %d\n",loop2,curcont->contcol[loop2]);
        //printf("curcont->numtrisegs[curcont->trinum[%d] = %d\n",curcont->trinum[loop2],curcont->numtrisegs[curcont->trinum[loop2]]);
        //printf("curcont->trinum[%d] = %d\n",loop2,curcont->trinum[loop2]);

        fidval = curcont->isolevels[curcont->contcol[loop2]];
        getContColor(fidval, fidmin, fidmax, curmesh->cmap, color, curmesh->invert);

        if(map3d_info.contour_antialiasing){
            glEnable (GL_LINE_SMOOTH);
            glEnable (GL_BLEND);
            glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
        }

        if (curmesh->shadingmodel == SHADE_NONE)
            glColor3ubv(color);
        else
            glColor3ub(0,0,0);
        if (curcont->isolevels[curcont->contcol[loop2]] < 0 && curmesh->negcontdashed)
            glEnable(GL_LINE_STIPPLE);

        //     printf("contpts1[%d][0] = %f\n",loop2,contpts1[loop2][0]);
        //     printf("contpts1[%d][1] = %f\n",loop2,contpts1[loop2][1]);
        //     printf("contpts1[%d][2] = %f\n",loop2,contpts1[loop2][2]);

        //     printf("contpts2[%d][0] = %f\n",loop2,contpts2[loop2][0]);
        //     printf("contpts2[%d][1] = %f\n",loop2,contpts2[loop2][1]);
        //     printf("contpts2[%d][2] = %f\n",loop2,contpts2[loop2][2]);

        glBegin(GL_LINES);
        glVertex3fv(contpts1[loop2]);
        glVertex3fv(contpts2[loop2]);
        glEnd();
        glDisable(GL_LINE_STIPPLE);
        if(map3d_info.contour_antialiasing){
            glDisable (GL_LINE_SMOOTH);
            glDisable (GL_BLEND);
        }
    }
    //  }
    /*  else {
    glColor4ub(0, 0, 0, 128);

  for (loop2 = 0; loop2 < length; loop2++) {
    if (curcont->isolevels[curcont->contcol[loop2]] < 0 && curmesh->negcontdashed)
      glEnable(GL_LINE_STIPPLE);
    glBegin(GL_LINES);
    
    glVertex3fv(contpts1[loop2]);
    glVertex3fv(contpts2[loop2]);
    glEnd();
    glDisable(GL_LINE_STIPPLE);
  }
  }
*/
#if SHOW_OPENGL_ERRORS
    GLenum e = glGetError();
    if (e)
        printf("GeomWindow DrawCont OpenGL Error: %s\n", gluErrorString(e));
#endif
}

void DrawFidMapSurf(Mesh_Info * curmesh,Contour_Info *cont)
{
    int curframe = 0;
    int length = 0;
    int index;
    int loop2, loop3;
    float a = 1, b = 0;
    float mean = 0;
    float **modelpts = 0;
    float **ptnormals = 0;
    float **fcnormals = 0;
    Map3d_Geom *curgeom = 0;
    Surf_Data *cursurf = 0;
    Contour_Info *curcont = 0;

    curgeom = curmesh->geom;
    cursurf = curmesh->data;

    modelpts = curgeom->points[curgeom->geom_index];
    ptnormals = curgeom->ptnormals;
    fcnormals = curgeom->fcnormals;

    if (cursurf) {
        curframe = cursurf->framenum;
        curcont = cont;
        //if (cursurf->minmaxframes)
        //compute_mapping(curmesh, cursurf, a, b);
    }

    if ((int)a == INT_MAX || (int)b == INT_MAX) //change by BJW to avoid crash
        a = b = 0;                  //when there is no data

    float fidmin, fidmax;
    cursurf->get_fid_minmax(fidmin, fidmax, cont->datatype);
    //  if (map3d_info.scale_mapping == SYMMETRIC) {
    //    if (fabs(potmax) > fabs(potmin))
    //      potmin = -potmax;
    //    else
    //      potmax = -potmin;
    //  }
    //  if (map3d_info.scale_mapping == SEPARATE) {
    //    if (potmax < 0)
    //      potmax = 0;
    //    if (potmin > 0)
    //      potmin = 0;
    //  }

    unsigned char color[3];

    bool use_textures = false;
    // gouraud shading
    if (curmesh->shadingmodel == SHADE_GOURAUD) {
        use_textures = curmesh->gouraudstyle == SHADE_TEXTURED &&
                (curmesh->cmap->type == RAINBOW_CMAP || curmesh->cmap->type == JET_CMAP);

        if (use_textures) {
            glShadeModel(GL_SMOOTH);
            glEnable(GL_TEXTURE_1D);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            glColor3f(1,1,1);
            glBindTexture(GL_TEXTURE_1D, curmesh->cmap->type);
        }
        else
            glShadeModel(GL_SMOOTH);
    }
    else if (curmesh->shadingmodel == SHADE_FLAT)
        glShadeModel(GL_FLAT);

    if (curgeom->elementsize == 3 && curmesh->shadingmodel != SHADE_BANDED) {
        length = curgeom->numelements;
        glBegin(GL_TRIANGLES);

        float fidval;
        for (loop2 = 0; loop2 < length; loop2++) {
            if (curmesh->shadingmodel == SHADE_GOURAUD) {
                // avoid repeating code 3 times
                for (loop3 = 0; loop3 < 3; loop3++) {
                    index = curgeom->elements[loop2][loop3];
                    fidval = 0;
                    //for(int fidsets = 0; fidsets < cursurf->numfs; fidsets++){
                    if(index < cursurf->fids.numfidleads){
                        for(int numfids = 0; numfids < cursurf->fids.leadfids[index].numfids; numfids++){
                            if((cursurf->fids.leadfids[index].fidtypes[numfids] == cont->datatype)){
                                fidval = cursurf->fids.leadfids[index].fidvals[numfids];
                            }
                        }
                    }
                    //}

                    if (use_textures)
                        glTexCoord1f(getContNormalizedValue(fidval, fidmin, fidmax, curmesh->invert));
                    else {
                        getContColor(fidval, fidmin, fidmax, curmesh->cmap, color, curmesh->invert);
                        glColor3ubv(color);
                    }
                    glNormal3fv(ptnormals[index]);
                    glVertex3fv(modelpts[index]);
                }
            }
            else {
                mean = 0;
                for (loop3 = 0; loop3 < 3; loop3++) {
                    index = curgeom->elements[loop2][loop3];
                    //for(int fidsets = 0; fidsets < cursurf->numfs; fidsets++){
                    if(index < cursurf->fids.numfidleads){
                        for(int numfids = 0; numfids < cursurf->fids.leadfids[index].numfids; numfids++){
                            if((cursurf->fids.leadfids[index].fidtypes[numfids] == cont->datatype)){
                                mean += cursurf->fids.leadfids[index].fidvals[numfids];
                            }
                        }
                    }
                    //}
                }
                mean /= 3;
                getContColor(mean, fidmin, fidmax, curmesh->cmap, color, curmesh->invert);
                glColor3ubv(color);
                glNormal3fv(fcnormals[loop2]);

                for (loop3 = 0; loop3 < 3; loop3++) {
                    index = curgeom->elements[loop2][loop3];
                    glVertex3fv(modelpts[index]);
                }
            }
        }
        glEnd();
    }
    else if (curgeom->elementsize == 4 && curmesh->shadingmodel != SHADE_BANDED) {
        length = curgeom->numelements;
        glBegin(GL_TRIANGLES);
        for (loop2 = 0; loop2 < length; loop2++) {
            // this inner loop is a hack to avoid repeating the
            // following index/glNormal/glVertex code 4 times
            for (loop3 = 0; loop3 < 4; loop3++) {
                int idx1, idx2, idx3;
                if (loop3 == 3) {
                    idx1 = 1;
                    idx2 = 3;
                    idx3 = 2;
                }
                else if (loop3 == 2) {
                    idx1 = 0;
                    idx2 = loop3;
                    idx3 = 1;
                }
                else {
                    idx1 = 0;
                    idx2 = loop3;
                    idx3 = loop2 + 1;
                }
                if (curmesh->shadingmodel == SHADE_GOURAUD) {
                    index = curgeom->elements[loop2][idx1];
                    glNormal3fv(ptnormals[index]);
                    glVertex3fv(modelpts[index]);
                    index = curgeom->elements[loop2][idx2];
                    glNormal3fv(ptnormals[index]);
                    glVertex3fv(modelpts[index]);
                    index = curgeom->elements[loop2][idx3];
                    glNormal3fv(ptnormals[index]);
                    glVertex3fv(modelpts[index]);
                }
                else {
                    glNormal3fv(fcnormals[loop2]);
                    glVertex3fv(modelpts[curgeom->elements[loop2][idx1]]);
                    glVertex3fv(modelpts[curgeom->elements[loop2][idx2]]);
                    glVertex3fv(modelpts[curgeom->elements[loop2][idx3]]);
                }
            }
        }

        glEnd();
    }

    //band shading
    if (curmesh->shadingmodel == SHADE_BANDED) {
        //if (curmesh->lighting)
        //glShadeModel(GL_SMOOTH);

        float** pts;

        //glDisable(GL_LIGHTING);
        float fidval;
        for (loop2 = 0; loop2 < curcont->numbandpolys; loop2++) {
            length = curcont->bandpolys[loop2].numpts;
            pts = curcont->bandpolys[loop2].nodes;
            //normals = curcont->bandpolys[loop2].normals;

            //potval = curcont->bandpolys[loop2].bandpotval;
            int contnum = curcont->bandpolys[loop2].bandcol;
            float* conts = curcont->isolevels;
            if (contnum == -1)
                fidval = fidmin;
            else if (contnum == curcont->numlevels - 1)
                fidval = fidmax;
            else {
                fidval = conts[contnum] + (conts[contnum+1]-conts[contnum])*(contnum+1)/(curcont->numlevels);
            }
            getContColor(fidval, fidmin, fidmax, curmesh->cmap, color, curmesh->invert);
            glColor3ubv(color);

            glBegin(GL_POLYGON);


            for (loop3 = 0; loop3 < length; loop3++) {
                //glNormal3fv(normals[loop3]);
                glVertex3fv(pts[loop3]);
            }
            //glNormal3fv(normals[0]);
            glVertex3fv(pts[0]);

            glEnd();
        }

        if (curmesh->lighting)
            glEnable(GL_LIGHTING);
    }
    glDisable(GL_TEXTURE_1D);
#if SHOW_OPENGL_ERRORS
    GLenum e = glGetError();
    if (e)
        printf("GeomWindow DrawSurf OpenGL Error: %s\n", gluErrorString(e));
#endif
}




void DrawFidCont(Mesh_Info * curmesh, Contour_Info *cont)
{
    //printf("drawing fids!!!\n");
    //printf("fidtype %d\n",cont->datatype);
    int length = 0;
    int loop2;
    float **contpts1 = 0;
    float **contpts2 = 0;
    Surf_Data *cursurf = 0;
    Contour_Info *curcont = 0;

    cursurf = curmesh->data;

    if (cursurf) {
        curcont = cont;    //used to be curframe instead of 0
        //if (cursurf->minmaxframes)
        //compute_mapping(curmesh, cursurf, a, b);
    }


    if (curcont) {
        contpts1 = curcont->contpt1;
        contpts2 = curcont->contpt2;
    }

    glLineWidth(curcont->fidContSize);
    glPointSize(curcont->fidContSize);

    length = curcont->numisosegs;
    if(curcont->fidContSize > 0){
        for (loop2 = 0; loop2 < length; loop2++) {
            if(map3d_info.contour_antialiasing){
                glEnable (GL_LINE_SMOOTH);
                glEnable (GL_BLEND);
                glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
            }
            glColor3ub(curcont->fidcolor.red(),curcont->fidcolor.green(),curcont->fidcolor.blue());
            //          if(curcont->datatype == 10)
            //            glColor3ub(255,0,0);
            //          if(curcont->datatype == 13)
            //            glColor3ub(255,0,255);
            //          if(curcont->datatype == 11)
            //            glColor3ub(0,0,255);
            if (curcont->isolevels[curcont->contcol[loop2]] < 0 && curmesh->negcontdashed)
                glEnable(GL_LINE_STIPPLE);

            glBegin(GL_LINES);
            glVertex3fv(contpts1[loop2]);
            glVertex3fv(contpts2[loop2]);
            glEnd();
            glDisable(GL_LINE_STIPPLE);
            if(map3d_info.contour_antialiasing){
                glDisable (GL_LINE_SMOOTH);
                glDisable (GL_BLEND);
            }
        }
    }

#if SHOW_OPENGL_ERRORS
    GLenum e = glGetError();
    if (e)
        printf("GeomWindow DrawCont OpenGL Error: %s\n", gluErrorString(e));
#endif
}



void GeomWindow::Transform(Mesh_Info * curmesh, float factor, bool compensateForRetinaDisplay)
{
    HMatrix mNow, cNow;           // arcball rotation matrices

    GLdouble front_plane[] = { 0, 0, 1, clip->front };
    GLdouble back_plane[] = { 0, 0, -1, clip->back };


    int pixelFactor = 1;

    if (compensateForRetinaDisplay)
    {
        pixelFactor=QApplication::desktop()->devicePixelRatio();
        // this compensates for the "Retina" display ratio.  See http://doc.qt.io/qt-5/highdpi.html
        //  (for some reason the picking doesn't need this)
    }
    glViewport(0, 0, width()*pixelFactor, height()*pixelFactor);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(vfov, width() / (float)height(), l2norm, 3 * l2norm);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* get the arcall rotation */
    Ball_Value(&curmesh->tran->rotate, mNow);
    Ball_Value(&clip->bd, cNow);

    /* current mousing translation */
    glTranslatef(curmesh->tran->tx, curmesh->tran->ty, curmesh->tran->tz);

    /* finally, move the mesh to in front of the eye */
    glTranslatef(0, 0, l2norm * (factor - 2));

    /* draw clipping planes (if enabled) */
    glPushMatrix();
    glMultMatrixf((float *)cNow);
    glTranslatef(xcenter, ycenter, zcenter);

    glClipPlane(GL_CLIP_PLANE0, front_plane);
    glClipPlane(GL_CLIP_PLANE1, back_plane);

    glPopMatrix();

    /* include the arcball rotation */
    glMultMatrixf((float *)mNow);

    /* move center of mesh to origin */
    glTranslatef(-xcenter, -ycenter, -zcenter);
}

// draw all types of node marks, in this precedence:
//   triangulating nodes, leads, picks, extrema, all
void GeomWindow::DrawNodes(Mesh_Info * curmesh)
{
    //  int curframe = 0;
    int length = 0, loop = 0;
    float min = 0, max = 0, value = 0;
    float mNowI[16];
    float **modelpts = 0;
    Map3d_Geom *curgeom = 0;
    Surf_Data *cursurf = 0;
    HMatrix mNow;

    ColorMap *curmap = 0;
    curgeom = curmesh->geom;
    cursurf = curmesh->data;

    modelpts = curgeom->points[curgeom->geom_index];

    if (cursurf) {
        curmap = curmesh->cmap;
    }

    length = curgeom->numpts;

    /* set the transform for billboarding */
    Ball_Value(&curmesh->tran->rotate, mNow);
    TransposeMatrix16((float *)mNow, mNowI);
    Transform(curmesh, 0.01f, true);
    glPushMatrix();

    //if (!cursurf)
    //return;
    if (cursurf)
        cursurf->get_minmax(min, max);
    unsigned char color[3];

    map<int, char*> lead_labels;
    set<int> pick_nodes;

    for (int i = 0; i <= curmesh->pickstacktop; i++)
    {
        pick_nodes.insert(curmesh->pickstack[i]->node);

        //        cout<<"pickstacktop is"<<curmesh->pickstacktop<<std::endl;
        //        cout<<"curmesh->pickstack[i]->node"<<i<<"  is  "<<curmesh->pickstack[i]->node<<std::endl;
        //        cout<<"curmesh->pickstack[i]->nearestIdx"<<i<<"  is  "<<curmesh->pickstack[i]->nearestIdx<<std::endl;

    }
    for (int i  = 0; i < curgeom->numleadlinks; i++) {
        lead_labels[curgeom->leadlinks[i]] = curgeom->leadlinklabels[i];
    }

    if (curmesh->mark_all_sphere || curmesh->mark_extrema_sphere || curmesh->mark_ts_sphere || curmesh->mark_lead_sphere ||
            ((map3d_info.pickmode == TRIANGULATE_PICK_MODE || map3d_info.pickmode == EDIT_NODE_PICK_MODE) && curmesh->num_selected_pts > 0)) {
        //glEnable(GL_BLEND);
        //glBlendFunc(GL_ZERO, GL_SRC_COLOR);
        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(GL_GEQUAL, .4f);
        float sphere_size = 1.0f;

        for (loop = 0; loop < length; loop++) {

            if (cursurf ) {
                value = cursurf->potvals[cursurf->framenum][loop];
                getContColor(value, min, max, curmap, color, curmesh->invert);
            }

            // in the process of triangulating these nodes (if we're in the right pick mode)
            if ((curmesh->num_selected_pts > 0 && curmesh->selected_pts[0] == loop) ||
                    (curmesh->num_selected_pts > 1 && curmesh->selected_pts[1] == loop)) {

                float max_size;
                max_size = MAX(curmesh->mark_all_size, curmesh->mark_extrema_size);
                max_size = MAX(max_size, curmesh->mark_ts_size);
                max_size = MAX(max_size, curmesh->mark_lead_size);

                glColor3f(fabs(curmesh->meshcolor[0] - .4), fabs(curmesh->meshcolor[1] - .4), fabs(curmesh->meshcolor[2] - .4));
                glPointSize(height() / 200 * curmesh->mark_triangulate_size);
                sphere_size = curmesh->mark_triangulate_size;

            }

            // leadlink node
            else if (curmesh->mark_lead_sphere && lead_labels[loop] != 0) {
                glColor3f(curmesh->mark_lead_color[0], curmesh->mark_lead_color[1], curmesh->mark_lead_color[2]);
                glPointSize(height() / 200 * curmesh->mark_lead_size);
                sphere_size = curmesh->mark_lead_size;
            }

            // pick node
            else if (curmesh->mark_ts_sphere && pick_nodes.size() > 0 && pick_nodes.find(loop) != pick_nodes.end()) {

                if (loop == curmesh->curpicknode)
                    glColor3f(1.0, 0.1, 1.f);
                else
                    glColor3f(curmesh->mark_ts_color[2], curmesh->mark_ts_color[2], curmesh->mark_ts_color[2]); //picked nodes in black

                glPointSize(height() / 200 * curmesh->mark_ts_size);
                sphere_size = curmesh->mark_ts_size;
            }

            // extrema node
            else if (cursurf && (value == max || value == min) && curmesh->mark_extrema_sphere) {
                // switch for proper color-mapping
                if (value == max)
                    value = min;
                else if (value == min)
                    value = max;
                // if shading is on, assign the extrema to the opposite color or else you won't see it
                if (curmesh->shadingmodel != SHADE_NONE)
                    getContColor(value, min, max, curmap, color, curmesh->invert);
                glColor3ubv(color);
                sphere_size = curmesh->mark_extrema_size;
                glPointSize(height() / 200 * curmesh->mark_extrema_size);
            }

            // 'all' node
            else if (curmesh->mark_all_sphere) {
                if (curmesh->mark_all_sphere_value && cursurf && value != UNUSED_DATA) {
                    glColor3ubv(color);
                }
                else {
                    glColor3f(curmesh->mark_all_color[0], curmesh->mark_all_color[1], curmesh->mark_all_color[2]);
                }
                glPointSize(height() / 200 * curmesh->mark_all_size);
                sphere_size = curmesh->mark_all_size;
            }
            else {
                continue;
            }

            if (plot_nearest_electrode!=1)
            {
                glTranslatef(modelpts[loop][0], modelpts[loop][1], modelpts[loop][2]);
                glMultMatrixf((float *)mNowI);
                glTranslatef(-modelpts[loop][0], -modelpts[loop][1], -modelpts[loop][2]);
            }

            // try to convert the sphere size from geometry units to pixels
            // 400 is a good number to use to normalize the l2norm
            sphere_size = sphere_size*l2norm/400;

            if (curmesh->draw_marks_as_spheres)
            {
                if (plot_nearest_electrode!=1)
                {
                    DrawDot(modelpts[loop][0], modelpts[loop][1], modelpts[loop][2], sphere_size);
                }
                else
                {
                    glEnable(GL_POINT_SMOOTH);
                    glEnable(GL_PROGRAM_POINT_SIZE_EXT);
                    glPointSize(10);
                    glBegin(GL_POINTS);
                    glColor3f(curmesh->mark_ts_color[2], curmesh->mark_ts_color[2], curmesh->mark_ts_color[2]);
                    glVertex3f(modelpts[loop][0], modelpts[loop][1],modelpts[loop][2]);




                    //                    for (int j = 0; j <= curmesh->pickstacktop; j++)
                    //                    {
                    //                        if (curmesh->toggle_electrode==0)
                    //                        {
                    //                            glColor3f(curmesh->mark_ts_color[0], curmesh->mark_ts_color[2], curmesh->mark_ts_color[0]);
                    //                            glVertex3f(recording_all_pts[curmesh->pickstack[j]->nearestIdx][0], recording_all_pts[curmesh->pickstack[j]->nearestIdx][1],recording_all_pts[curmesh->pickstack[j]->nearestIdx][2]);
                    //                        }
                    //                    }
                    glEnd();
                    glDisable(GL_POINT_SMOOTH);
                    glDisable(GL_PROGRAM_POINT_SIZE_EXT);
                }
            }
            else {
                glBegin(GL_POINTS);
                glVertex3f(modelpts[loop][0], modelpts[loop][1], modelpts[loop][2]);
                glEnd();
            }
            glPopMatrix();
            glPushMatrix();
        }

        glDisable(GL_ALPHA_TEST);
        glDisable(GL_BLEND);
    }

    if (map3d_info.pickmode == TRIANGULATE_PICK_MODE && curmesh->num_selected_pts == 2) {
        // draw a line between the picked nodes
        glColor3f(fabs(curmesh->meshcolor[0] - .4), fabs(curmesh->meshcolor[1] - .4), fabs(curmesh->meshcolor[2] - .4));
        glBegin(GL_LINES);
        int pt1 = curmesh->selected_pts[0];
        int pt2 = curmesh->selected_pts[1];
        glVertex3f(modelpts[pt1][0], modelpts[pt1][1], modelpts[pt1][2]);
        glVertex3f(modelpts[pt2][0], modelpts[pt2][1], modelpts[pt2][2]);
        glEnd();
    }

    float pos[3];

    if (curmesh->mark_all_number || (curmesh->mark_extrema_number && curmesh->data) ||
            curmesh->mark_ts_number || curmesh->mark_lead_number) {
        //glDepthMask(GL_FALSE);

        for (loop = 0; loop < length; loop++) {
            if (cursurf) {
                value = cursurf->potvals[cursurf->framenum][loop];
                getContColor(value, min, max, curmap, color, curmesh->invert);
            }

            if (plot_nearest_electrode!=1)
            {
                glTranslatef(modelpts[loop][0], modelpts[loop][1], modelpts[loop][2]);
                glMultMatrixf((float *)mNowI);
                glTranslatef(-modelpts[loop][0], -modelpts[loop][1], -modelpts[loop][2]);
            }


            pos[0] = modelpts[loop][0];
            pos[1] = modelpts[loop][1];
            pos[2] = modelpts[loop][2];

            int number = 0;

            // leadlink node
            if (curmesh->mark_lead_number && lead_labels[loop] != 0) {
                glColor3f(curmesh->mark_lead_color[0], curmesh->mark_lead_color[1], curmesh->mark_lead_color[2]);
                number = curmesh->mark_lead_number;
            }

            // pick node
            else if (curmesh->mark_ts_number && pick_nodes.find(loop) != pick_nodes.end()) {
                if (loop == curmesh->curpicknode)
                    glColor3f(1.0, 0.1, 1.f);
                else
                    glColor3f(curmesh->mark_ts_color[0], curmesh->mark_ts_color[1], curmesh->mark_ts_color[2]);
                number = curmesh->mark_ts_number;
            }
            else if (cursurf && (value == max || value == min) && curmesh->mark_extrema_number && cursurf->potvals) {
                if (value == max)
                    value = min;
                else if (value == min)
                    value = max;
                // if shading is on, assign the extrema to the opposite color or else you won't see it
                if (curmesh->shadingmodel != SHADE_NONE)
                    getContColor(value, min, max, curmap, color, curmesh->invert);
                glColor3ubv(color);
                number = curmesh->mark_extrema_number;
            }
            else if (curmesh->mark_all_number) {
                // this is a function of the fov (zoom), the ratio of
                // mesh's l2norm to the window's l2norm and the window
                // height to determine whether the numbers will be too
                // close together or not

                glColor3f(curmesh->mark_all_color[0], curmesh->mark_all_color[1], curmesh->mark_all_color[2]);
                number = curmesh->mark_all_number;
            }

            float scale = fontScale();
            switch (number) {
            case 1:
                renderString3f(pos[0], pos[1], pos[2], (int)small_font, QString::number(loop + 1), scale);
                break;
            case 2:
                if (curgeom->channels[loop]+1 > 0)
                    renderString3f(pos[0], pos[1], pos[2], (int)small_font, QString::number(curgeom->channels[loop] + 1), scale);
                break;
            case 3:
                if (cursurf && cursurf->potvals && cursurf->potvals[cursurf->framenum][loop] != UNUSED_DATA)

                    for (int i = 0; i <= curmesh->pickstacktop; i++)
                    {
                        if (loop==curmesh->pickstack[i]->node){
                            renderString3f(pos[0], pos[1], pos[2], (int)small_font,QString::number(i+1, 'g', 2), scale+0.2);

                            //                            if ((plot_nearest_electrode==1)&& (curmesh->toggle_electrode==0))
                            //                            {
                            //                                renderString3f(recording_all_pts[curmesh->pickstack[i]->nearestIdx][0], recording_all_pts[curmesh->pickstack[i]->nearestIdx][1],recording_all_pts[curmesh->pickstack[i]->nearestIdx][2], (int)small_font,
                            //                                        QString::number(i+1, 'g', 2), scale);
                            //                            }
                        }
                    }

                break;
            case 4:
                // case 4 is dependent on which type of mark it is
                //   if it is a leadlink, and its value is 4, then print the lead label.
                //   if it is a fid, and its value is 4, then print the fid label
                if (curmesh->mark_lead_number == 4 && lead_labels[loop]) {
                    renderString3f(pos[0], pos[1], pos[2], (int)small_font, lead_labels[loop], scale);
                    break;
                }
                else if (curmesh->mark_all_number == 4 && cursurf->fids.numfidleads > 0){
                    float fid = 0;
                    if(loop < cursurf->fids.numfidleads){
                        for(int numfids = 0; numfids < cursurf->fids.leadfids[loop].numfids; numfids++){
                            if((cursurf->fids.leadfids[loop].fidtypes[numfids] == curmesh->drawfidmapcont)){
                                fid = cursurf->fids.leadfids[loop].fidvals[numfids];
                            }
                        }
                    }

                    renderString3f(pos[0], pos[1], pos[2], (int)small_font, QString::number(fid, 'g', 2), scale);
                    break;
                }
            }
            glPopMatrix();
            glPushMatrix();
        }
        glDepthMask(GL_TRUE);
    }

    glPopMatrix();

#if SHOW_OPENGL_ERRORS
    GLenum e = glGetError();
    if (e)
        printf("GeomWindow DrawNodes OpenGL Error: %s\n", gluErrorString(e));
#endif
}


void GeomWindow::DrawInfo()
{
    int nummesh = meshes.size();
    int surfnum = 0;
    float position[3] = { -1.f, static_cast<float>(height() - 15.0), 0.f };
    Mesh_Info *dommesh = 0;

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, width(), 0, height());
    glColor3f(fgcolor[0], fgcolor[1], fgcolor[2]);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glPopMatrix();
    if (nummesh == 1 || (dominantsurf != -1 && nummesh > dominantsurf)) {
        dommesh = nummesh == 1 ? meshes[0] : meshes[dominantsurf];
        char surfstr[50];
        if (dommesh->geom->subsurf <= 0)
            sprintf(surfstr, "Potential map Surface #%d", dommesh->geom->surfnum);
        else
            sprintf(surfstr, "Potential map Surface #%d-%d", dommesh->geom->surfnum, dommesh->geom->subsurf);
        surfnum = dommesh->geom->surfnum;
        position[0] = (float)width()/2.0 -((float)getFontWidth((int)large_font, surfstr)/2.0);
        position[1] = height() - getFontHeight((int)large_font);

        renderString3f(position[0], position[1], position[2], (int)large_font, surfstr);

        position[1] -= getFontHeight((int)med_font)*.8 ;
        if (dommesh->data){
            char * slash = 0;
        }
        else{
            char * slash = 0;
        }
    }
    else if (nummesh > 1) {
        position[0] = (float)width()/2.0 -((float)getFontWidth((int)large_font, "All Surfaces")/2.0);
        renderString3f(position[0], position[1], position[2], (int)large_font, "All Surfaces");
    }
}

void GeomWindow::DrawLockSymbol(int which, bool full)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    float aspect = (float)height() / width();
    float offset = .1 * aspect * which;
    float width;
    float texmax;
    if (full) {
        width = .1;
        texmax = 1;
    }
    else {
        width = .05;
        texmax = .5;
    }

    UseTexture(map3d_info.lock_texture);
    switch (which) {
    case 0:
        glColor3f(.8, .8, 0);
        break;
    case 1:
        glColor3f(.8, 0, 0);
        break;
    case 2:
        glColor3f(0, .8, 0);
        break;
    case 3:
        glColor3f(0, .5, .9);
        break;

    }

    glEnable(GL_TEXTURE_2D);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2f(-.95 + offset, -.85);
    glTexCoord2f(0, 1);
    glVertex2f(-.95 + offset, -.95);
    glTexCoord2f(texmax, 1);
    glVertex2f(-.95 + width * aspect + offset, -.95);
    glTexCoord2f(texmax, 0);
    glVertex2f(-.95 + width * aspect + offset, -.85);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}

void DrawDot(float x, float y, float z, float size)
{
    glEnable(GL_TEXTURE_2D);
    UseTexture(map3d_info.dot_texture);
#if 0
    glBindTexture(GL_TEXTURE_2D, DOT_TEXTURE);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
#endif

    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex3f(x-size, y-size, z);
    glTexCoord2f(0, 1);
    glVertex3f(x-size, y+size, z);
    glTexCoord2f(1, 1);
    glVertex3f(x+size, y+size, z);
    glTexCoord2f(1, 0);
    glVertex3f(x+size, y-size, z);
    glEnd();

    glDisable(GL_TEXTURE_2D);

}

void GeomWindow::DrawAxes(Mesh_Info * mesh)
{
    //float normal[3]={0.f,0.f,-1.f};
    //float up[3]={0.f,1.f,0.f};
    float pos[3];
    float delta_x, delta_y, delta_z;

    bool xneg = false, yneg = false, zneg = false;

    if (mesh->geom->xmax >= xcenter)
        delta_x = mesh->geom->xmax + .1 * (mesh->geom->xmax - mesh->geom->xmin);
    else {
        delta_x = mesh->geom->xmin - .15 * (mesh->geom->xmax - mesh->geom->xmin);
        xneg = true;
    }
    if (mesh->geom->ymax >= ycenter)
        delta_y = mesh->geom->ymax + .15 * (mesh->geom->ymax - mesh->geom->ymin);
    else {
        delta_y = mesh->geom->ymin - .15 * (mesh->geom->ymax - mesh->geom->ymin);
        yneg = true;
    }
    if (mesh->geom->zmax >= zcenter)
        delta_z = mesh->geom->zmax + .15 * (mesh->geom->zmax - mesh->geom->zmin);
    else {
        delta_z = mesh->geom->zmin - .15 * (mesh->geom->zmax - mesh->geom->zmin);
        zneg = true;
    }

    //glColor3f(mesh->axescolor[0], mesh->axescolor[1], mesh->axescolor[2]);

    glLineWidth(3);
    glBegin(GL_LINES);

    //float d = l2norm*mesh->mark_all_size * 25;

    //draw axes
    if(rgb_axes){
        glColor3f(255,0,0);
    }
    else{
        glColor3f(mesh->axescolor[0], mesh->axescolor[1], mesh->axescolor[2]);
    }
    glVertex3f(xcenter, ycenter, zcenter);
    glVertex3f(delta_x, ycenter, zcenter);
    if(rgb_axes){
        glColor3f(0,255,0);
    }
    else{
        glColor3f(mesh->axescolor[0], mesh->axescolor[1], mesh->axescolor[2]);
    }
    glVertex3f(xcenter, ycenter, zcenter);
    glVertex3f(xcenter, delta_y, zcenter);
    if(rgb_axes){
        glColor3f(0,0,255);
    }
    else{
        glColor3f(mesh->axescolor[0], mesh->axescolor[1], mesh->axescolor[2]);
    }
    glVertex3f(xcenter, ycenter, zcenter);
    glVertex3f(xcenter, ycenter, delta_z);

    glEnd();

    //write axes labels - negative if dimension max < window's dimension center
    if (showinfotext) {
        pos[0] = xcenter + delta_x;
        pos[1] = ycenter;
        pos[2] = zcenter;

        if(rgb_axes){
            glColor3f(255,0,0);
        }
        else{
            glColor3f(mesh->axescolor[0], mesh->axescolor[1], mesh->axescolor[2]);
        }
        if (xneg)
            renderString3f(pos[0], pos[1], pos[2], (int)small_font, "-X");
        else
            renderString3f(pos[0], pos[1], pos[2], (int)small_font, "X");
        pos[0] = xcenter;
        pos[1] = delta_y;

        if(rgb_axes){
            glColor3f(0,255,0);
        }
        else{
            glColor3f(mesh->axescolor[0], mesh->axescolor[1], mesh->axescolor[2]);
        }
        if (yneg)
            renderString3f(pos[0], pos[1], pos[2], (int)small_font, "-Y");
        else
            renderString3f(pos[0], pos[1], pos[2], (int)small_font, "Y");
        pos[1] = ycenter;
        pos[2] = delta_z;

        if(rgb_axes){
            glColor3f(0,0,255);
        }
        else{
            glColor3f(mesh->axescolor[0], mesh->axescolor[1], mesh->axescolor[2]);
        }
        if (zneg)
            renderString3f(pos[0], pos[1], pos[2], (int)small_font, "-Z");
        else
            renderString3f(pos[0], pos[1], pos[2], (int)small_font, "Z");
    }

}

void GeomWindow::DrawBGImage()
{
    if (map3d_info.gi->bgcoords[0] != 0 || map3d_info.gi->bgcoords[3] != 1) {
        // user used a manual orientation - line it up as such
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(vfov, width() / (float)height(), l2norm, 3 * l2norm);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(0, 0, -2 * l2norm);
        glTranslatef(-xcenter, -ycenter, -zcenter);
    }
    else {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(0, 1, 0, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }
    float* pmin = &map3d_info.gi->bgcoords[0];
    float* pmax = &map3d_info.gi->bgcoords[3];

    UseTexture(map3d_info.bg_texture);

    glEnable(GL_TEXTURE_2D);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);  glVertex3f(pmin[0], pmin[1], pmin[2]);
    glTexCoord2f(0, 1);  glVertex3f(pmin[0], pmax[1], pmax[2]);
    glTexCoord2f(1, 1);  glVertex3f(pmax[0], pmax[1], pmax[2]);
    glTexCoord2f(1, 0);  glVertex3f(pmax[0], pmin[1], pmin[2]);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glClear(GL_DEPTH_BUFFER_BIT);

}


void GeomWindow::DrawMFS(Mesh_Info * curmesh)
{

    int curframe = 0;
    int length = 0;
    int index;
    int loop2, loop3;
    float a = 1, b = 0;
    float mean = 0;
    float **modelpts = 0;
    float **ptnormals = 0;
    float **fcnormals = 0;
    Map3d_Geom *curgeom = 0;
    Surf_Data *cursurf = 0;
    Contour_Info *curcont = 0;


    curgeom = curmesh->geom;
    cursurf = curmesh->data;

    modelpts = curgeom->points[curgeom->geom_index];
    ptnormals = curgeom->ptnormals;
    fcnormals = curgeom->fcnormals;

    if (cursurf) {
        curframe = cursurf->framenum;
        curcont = curmesh->cont;
        //if (cursurf->minmaxframes)
        //compute_mapping(curmesh, cursurf, a, b);
    }

    if ((int)a == INT_MAX || (int)b == INT_MAX) //change by BJW to avoid crash
        a = b = 0;                  //when there is no data

    float potmin, potmax;
    cursurf->get_minmax(potmin, potmax);

    if (map3d_info.scale_mapping == SYMMETRIC) {
        if (fabs(potmax) > fabs(potmin))
            potmin = -potmax;
        else
            potmax = -potmin;
    }
    if (map3d_info.scale_mapping == SEPARATE) {
        if (potmax < 0)
            potmax = 0;
        if (potmin > 0)
            potmin = 0;
    }

    unsigned char color[3];

    //band shading
    if (curmesh->shadingmodel == SHADE_BANDED && curcont) {

        if (curmesh->lighting)
            glShadeModel(GL_SMOOTH);

        float** pts;
        float** normals;
        float MFSval;

        for (loop2 = 0; loop2 < curcont->numbandpolys; loop2++) {

            length = curcont->bandpolys[loop2].numpts;
            pts = curcont->bandpolys[loop2].nodes;
            normals = curcont->bandpolys[loop2].normals;

            int contnum = curcont->bandpolys[loop2].bandcol;
            float* conts = curcont->isolevels;
            if (contnum == -1)
                MFSval = potmin;
            else if (contnum == curcont->numlevels - 1)
                MFSval = potmax;
            else {
                MFSval = conts[contnum] + (conts[contnum+1]-conts[contnum])*(contnum+1)/(curcont->numlevels);
            }
            getContColor(MFSval, potmin, potmax, curmesh->cmap, color, curmesh->invert);
            glColor4ub(color[0],color[1],color[2],color[3]);
            glBegin(GL_POLYGON);
            for (loop3 = 0; loop3 < length; loop3++) {
                glNormal3fv(normals[loop3]);
                glVertex3fv(pts[loop3]);
            }
            glNormal3fv(normals[0]);
            glVertex3fv(pts[0]);
            glEnd();
        }

        if (curmesh->lighting)
            glEnable(GL_LIGHTING);
    }

    bool use_textures = false;
    // gouraud shading
    if (curmesh->shadingmodel == SHADE_GOURAUD) {

        glShadeModel(GL_SMOOTH);
        use_textures = curmesh->gouraudstyle == SHADE_TEXTURED &&
                (curmesh->cmap->type == RAINBOW_CMAP || curmesh->cmap->type == JET_CMAP);

        if (use_textures) {
            glColor4f(1,1,1,0.4);
            glEnable(GL_TEXTURE_1D);
            if (curmesh->cmap->type == RAINBOW_CMAP)
                UseTexture(map3d_info.rainbow_texture);
            else
                UseTexture(map3d_info.jet_texture);
        }
    }
    else if (curmesh->shadingmodel == SHADE_FLAT){
        glShadeModel(GL_FLAT);
        glColor4ub(color[0],color[1],color[2],color[3]);
    }

    if (curgeom->elementsize == 3 && curmesh->shadingmodel != SHADE_BANDED) {
        length = curgeom->numelements;
        glBegin(GL_TRIANGLES);

        float MFSval;
        for (loop2 = 0; loop2 < length; loop2++) {
            if (curmesh->shadingmodel == SHADE_GOURAUD) {
                // avoid repeating code 3 times
                for (loop3 = 0; loop3 < 3; loop3++) {
                    index = curgeom->elements[loop2][loop3];
                    if (cursurf->MFSvals[curframe][index] == UNUSED_DATA)
                        break;
                }
                if (loop3 < 3)
                    // we have "UNUSED_DATA" on a node in this triangle, so don't draw here
                    continue;
                for (loop3 = 0; loop3 < 3; loop3++) {
                    index = curgeom->elements[loop2][loop3];
                    MFSval = cursurf->MFSvals[curframe][index];

                    if (use_textures)
                    {
                        glColor4f(1,1,1,0.4);
                        glEnable(GL_TEXTURE_1D);
                        if (curmesh->cmap->type == RAINBOW_CMAP){
                            UseTexture(map3d_info.rainbow_texture);
                        }
                        else
                            UseTexture(map3d_info.jet_texture);
                        glTexCoord1f(getContNormalizedValue(MFSval, potmin, potmax, curmesh->invert));
                    }

                    else {
                        getContColor(MFSval, potmin, potmax, curmesh->cmap, color, curmesh->invert);

                        glColor4ub(color[0],color[1],color[2],color[3]);
                    }
                    glNormal3fv(ptnormals[index]);
                    glVertex3fv(modelpts[index]);
                }
            }
            else {
                mean = 0;
                for (loop3 = 0; loop3 < 3; loop3++) {
                    index = curgeom->elements[loop2][loop3];
                    MFSval = cursurf->MFSvals[curframe][index];
                    if (MFSval == UNUSED_DATA)
                        break;
                    mean += MFSval;
                }
                if (loop3 < 3)
                    // we have "UNUSED_DATA" on a node in this triangle, so don't draw here
                    continue;

                mean /= 3;
                getContColor(mean, potmin, potmax, curmesh->cmap, color, curmesh->invert);
                glColor4ub(color[0],color[1],color[2],color[3]);
                glNormal3fv(fcnormals[loop2]);

                for (loop3 = 0; loop3 < 3; loop3++) {
                    index = curgeom->elements[loop2][loop3];
                    glVertex3fv(modelpts[index]);
                }
            }
        }
        glEnd();
    }
    else if (curgeom->elementsize == 4 && curmesh->shadingmodel != SHADE_BANDED) {

        length = curgeom->numelements;
        glBegin(GL_TRIANGLES);
        for (loop2 = 0; loop2 < length; loop2++) {
            // this inner loop is a hack to avoid repeating the
            // following index/glNormal/glVertex code 4 times
            for (loop3 = 0; loop3 < 4; loop3++) {
                int idx1, idx2, idx3;
                if (loop3 == 3) {
                    idx1 = 1;
                    idx2 = 3;
                    idx3 = 2;
                }
                else if (loop3 == 2) {
                    idx1 = 0;
                    idx2 = loop3;
                    idx3 = 1;
                }
                else {
                    idx1 = 0;
                    idx2 = loop3;
                    idx3 = loop2 + 1;
                }
                if (curmesh->shadingmodel == SHADE_GOURAUD) {
                    index = curgeom->elements[loop2][idx1];
                    glNormal3fv(ptnormals[index]);
                    glVertex3fv(modelpts[index]);
                    index = curgeom->elements[loop2][idx2];
                    glNormal3fv(ptnormals[index]);
                    glVertex3fv(modelpts[index]);
                    index = curgeom->elements[loop2][idx3];
                    glNormal3fv(ptnormals[index]);
                    glVertex3fv(modelpts[index]);
                }
                else {
                    glNormal3fv(fcnormals[loop2]);
                    glVertex3fv(modelpts[curgeom->elements[loop2][idx1]]);
                    glVertex3fv(modelpts[curgeom->elements[loop2][idx2]]);
                    glVertex3fv(modelpts[curgeom->elements[loop2][idx3]]);
                }
            }
        }

        glEnd();
    }



    glDisable(GL_TEXTURE_1D);
#if SHOW_OPENGL_ERRORS
    GLenum e = glGetError();
    if (e)
        printf("GeomWindow DrawSurf OpenGL Error: %s\n", gluErrorString(e));
#endif
}



void GeomWindow::DrawPhase(Mesh_Info * curmesh)
{

    int curframe = 0;
    int length = 0;
    int index;
    int loop2, loop3;
    float a = 1, b = 0;
    float mean = 0;
    float **modelpts = 0;
    float **ptnormals = 0;
    float **fcnormals = 0;
    Map3d_Geom *curgeom = 0;
    Surf_Data *cursurf = 0;
    Contour_Info *curcont = 0;


    curgeom = curmesh->geom;
    cursurf = curmesh->data;

    modelpts = curgeom->points[curgeom->geom_index];
    ptnormals = curgeom->ptnormals;
    fcnormals = curgeom->fcnormals;

    if (cursurf) {
        curframe = cursurf->framenum;
        curcont = curmesh->cont;
        //if (cursurf->minmaxframes)
        //compute_mapping(curmesh, cursurf, a, b);
    }

    if ((int)a == INT_MAX || (int)b == INT_MAX) //change by BJW to avoid crash
        a = b = 0;                  //when there is no data

    float potmin, potmax;
    cursurf->get_minmax(potmin, potmax);

    if (map3d_info.scale_mapping == SYMMETRIC) {
        if (fabs(potmax) > fabs(potmin))
            potmin = -potmax;
        else
            potmax = -potmin;
    }
    if (map3d_info.scale_mapping == SEPARATE) {
        if (potmax < 0)
            potmax = 0;
        if (potmin > 0)
            potmin = 0;
    }

    unsigned char color[3];

    //band shading
    if (curmesh->shadingmodel == SHADE_BANDED && curcont) {

        if (curmesh->lighting)
            glShadeModel(GL_SMOOTH);

        float** pts;
        float** normals;
        float Phaseval;

        for (loop2 = 0; loop2 < curcont->numbandpolys; loop2++) {

            length = curcont->bandpolys[loop2].numpts;
            pts = curcont->bandpolys[loop2].nodes;
            normals = curcont->bandpolys[loop2].normals;

            int contnum = curcont->bandpolys[loop2].bandcol;
            float* conts = curcont->isolevels;
            if (contnum == -1)
                Phaseval = potmin;
            else if (contnum == curcont->numlevels - 1)
                Phaseval = potmax;
            else {
                Phaseval = conts[contnum] + (conts[contnum+1]-conts[contnum])*(contnum+1)/(curcont->numlevels);
            }
            getContColor(Phaseval, potmin, potmax, curmesh->cmap, color, curmesh->invert);
            glColor4ub(color[0],color[1],color[2],color[3]);
            glBegin(GL_POLYGON);
            for (loop3 = 0; loop3 < length; loop3++) {
                glNormal3fv(normals[loop3]);
                glVertex3fv(pts[loop3]);
            }
            glNormal3fv(normals[0]);
            glVertex3fv(pts[0]);
            glEnd();
        }

        if (curmesh->lighting)
            glEnable(GL_LIGHTING);
    }

    bool use_textures = false;
    // gouraud shading
    if (curmesh->shadingmodel == SHADE_GOURAUD) {

        glShadeModel(GL_SMOOTH);
        use_textures = curmesh->gouraudstyle == SHADE_TEXTURED &&
                (curmesh->cmap->type == RAINBOW_CMAP || curmesh->cmap->type == JET_CMAP);

        if (use_textures) {
            glColor4f(1,1,1,0.4);
            glEnable(GL_TEXTURE_1D);
            if (curmesh->cmap->type == RAINBOW_CMAP)
                UseTexture(map3d_info.rainbow_texture);
            else
                UseTexture(map3d_info.jet_texture);
        }
    }
    else if (curmesh->shadingmodel == SHADE_FLAT){
        glShadeModel(GL_FLAT);
        glColor4ub(color[0],color[1],color[2],color[3]);
    }

    if (curgeom->elementsize == 3 && curmesh->shadingmodel != SHADE_BANDED) {
        length = curgeom->numelements;
        glBegin(GL_TRIANGLES);

        float Phaseval;
        for (loop2 = 0; loop2 < length; loop2++) {
            if (curmesh->shadingmodel == SHADE_GOURAUD) {
                // avoid repeating code 3 times
                for (loop3 = 0; loop3 < 3; loop3++) {
                    index = curgeom->elements[loop2][loop3];
                    if (cursurf->Phasevals[curframe][index] == UNUSED_DATA)
                        break;
                }
                if (loop3 < 3)
                    // we have "UNUSED_DATA" on a node in this triangle, so don't draw here
                    continue;
                for (loop3 = 0; loop3 < 3; loop3++) {
                    index = curgeom->elements[loop2][loop3];
                    Phaseval = cursurf->Phasevals[curframe][index];

                    if (use_textures)
                    {
                        glColor4f(1,1,1,0.4);
                        glEnable(GL_TEXTURE_1D);
                        if (curmesh->cmap->type == RAINBOW_CMAP){
                            UseTexture(map3d_info.rainbow_texture);
                        }
                        else
                            UseTexture(map3d_info.jet_texture);
                        glTexCoord1f(getContNormalizedValue(Phaseval, potmin, potmax, curmesh->invert));
                    }

                    else {
                        getContColor(Phaseval, potmin, potmax, curmesh->cmap, color, curmesh->invert);

                        glColor4ub(color[0],color[1],color[2],color[3]);
                    }
                    glNormal3fv(ptnormals[index]);
                    glVertex3fv(modelpts[index]);
                }
            }
            else {
                mean = 0;
                for (loop3 = 0; loop3 < 3; loop3++) {
                    index = curgeom->elements[loop2][loop3];
                    Phaseval = cursurf->Phasevals[curframe][index];
                    if (Phaseval == UNUSED_DATA)
                        break;
                    mean += Phaseval;
                }
                if (loop3 < 3)
                    // we have "UNUSED_DATA" on a node in this triangle, so don't draw here
                    continue;

                mean /= 3;
                getContColor(mean, potmin, potmax, curmesh->cmap, color, curmesh->invert);
                glColor4ub(color[0],color[1],color[2],color[3]);
                glNormal3fv(fcnormals[loop2]);

                for (loop3 = 0; loop3 < 3; loop3++) {
                    index = curgeom->elements[loop2][loop3];
                    glVertex3fv(modelpts[index]);
                }
            }
        }
        glEnd();
    }
    else if (curgeom->elementsize == 4 && curmesh->shadingmodel != SHADE_BANDED) {

        length = curgeom->numelements;
        glBegin(GL_TRIANGLES);
        for (loop2 = 0; loop2 < length; loop2++) {
            // this inner loop is a hack to avoid repeating the
            // following index/glNormal/glVertex code 4 times
            for (loop3 = 0; loop3 < 4; loop3++) {
                int idx1, idx2, idx3;
                if (loop3 == 3) {
                    idx1 = 1;
                    idx2 = 3;
                    idx3 = 2;
                }
                else if (loop3 == 2) {
                    idx1 = 0;
                    idx2 = loop3;
                    idx3 = 1;
                }
                else {
                    idx1 = 0;
                    idx2 = loop3;
                    idx3 = loop2 + 1;
                }
                if (curmesh->shadingmodel == SHADE_GOURAUD) {
                    index = curgeom->elements[loop2][idx1];
                    glNormal3fv(ptnormals[index]);
                    glVertex3fv(modelpts[index]);
                    index = curgeom->elements[loop2][idx2];
                    glNormal3fv(ptnormals[index]);
                    glVertex3fv(modelpts[index]);
                    index = curgeom->elements[loop2][idx3];
                    glNormal3fv(ptnormals[index]);
                    glVertex3fv(modelpts[index]);
                }
                else {
                    glNormal3fv(fcnormals[loop2]);
                    glVertex3fv(modelpts[curgeom->elements[loop2][idx1]]);
                    glVertex3fv(modelpts[curgeom->elements[loop2][idx2]]);
                    glVertex3fv(modelpts[curgeom->elements[loop2][idx3]]);
                }
            }
        }

        glEnd();
    }



    glDisable(GL_TEXTURE_1D);
#if SHOW_OPENGL_ERRORS
    GLenum e = glGetError();
    if (e)
        printf("GeomWindow DrawSurf OpenGL Error: %s\n", gluErrorString(e));
#endif
}



void GeomWindow::UpdateNearestPoints(Mesh_Info* recordingmesh,Mesh_Info* sourcemesh)
{

    // std::cout<<"enter update nearest points  "<<std::endl;


    int length1 = 0, loop1 = 0, length2 =0, loop2=0,  loop_idx_nearest=0,atria_num=0;

    Map3d_Geom *sourcegeom = 0;
    Surf_Data *sourcesurf = 0;
    sourcegeom = sourcemesh->geom;
    sourcesurf = sourcemesh->data;
    length1 = sourcemesh->pickstacktop+1;
    atria_num=sourcegeom->numpts;


    Map3d_Geom *recordinggeom = 0;
    Surf_Data *recordingsurf = 0;
    recordinggeom = recordingmesh->geom;
    recordingsurf = recordingmesh->data;
    length2 = recordinggeom->numpts;


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



    for (loop1 = 0; loop1 < length2; loop1++)
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

    vector<point_t> data_points;
    for (loop2 = 0; loop2 < length2; loop2++)
    {
        coord_t x,y,z;
        x = geom_temp_catheter_pts[loop2][0];
        y = geom_temp_catheter_pts[loop2][1];
        z = geom_temp_catheter_pts[loop2][2];
        data_points.push_back(tr1::make_tuple(x,y,z));
    }

    const size_t neighbours = 1; // number of nearest neighbours to find
    point_t points[neighbours];

    for  (int i = 0; i< length1;i++){

        point_t point(geom_temp_atria_pts[sourcemesh->pickstack[i]->node][0],geom_temp_atria_pts[sourcemesh->pickstack[i]->node][1], geom_temp_atria_pts[sourcemesh->pickstack[i]->node][2]);

        less_distance nearer(point);


        foreach (point_t& p, points)

            for (loop1 = 0; loop1 < length2; loop1++)
            {
                point_t current_point(geom_temp_catheter_pts[loop1][0],geom_temp_catheter_pts[loop1][1],geom_temp_catheter_pts[loop1][2]);
                foreach (point_t& p, points)

                    if (nearer(current_point, p))
                        std::swap(current_point, p);
            }

        sort(boost::begin(points), boost::end(points), nearer);

        foreach (point_t p, points)
        {
            point_t nearestpoint(p);

            for (loop_idx_nearest = 0; loop_idx_nearest< length2; loop_idx_nearest++)
            {
                if (nearestpoint == data_points[loop_idx_nearest])
                {

                    sourcemesh->pickstack[i]->nearestIdx = loop_idx_nearest;
                    //std::cout<<"updated nearestIdx  "<<sourcemesh->pickstack[i]->nearestIdx<<std::endl;


                    double x_diff=geom_temp_atria_pts[sourcemesh->pickstack[i]->node][0]-geom_temp_catheter_pts[loop_idx_nearest][0];
                    double y_diff=geom_temp_atria_pts[sourcemesh->pickstack[i]->node][1]-geom_temp_catheter_pts[loop_idx_nearest][1];
                    double z_diff=geom_temp_atria_pts[sourcemesh->pickstack[i]->node][2]-geom_temp_catheter_pts[loop_idx_nearest][2];


                    sourcemesh->pickstack[i]->nearestDis =sqrt(x_diff*x_diff+y_diff*y_diff+z_diff*z_diff);
                    //std::cout<<"updated nearest distance  "<<sourcemesh->pickstack[i]->nearestDis<<std::endl;
                }
            }
        }

    }


    float scale = fontScale();
    // QString toRender;

    for (int j = 0; j <= sourcemesh->pickstacktop; j++)
    {
        if (sourcemesh->toggle_electrode==0)
        {

            glEnable(GL_POINT_SMOOTH);
            glEnable(GL_PROGRAM_POINT_SIZE_EXT);
            glPointSize(12);
            glBegin(GL_POINTS);
            glColor3f(sourcemesh->mark_ts_color[0], sourcemesh->mark_ts_color[2], sourcemesh->mark_ts_color[0]);
            glVertex3f(pts[sourcemesh->pickstack[j]->nearestIdx][0], pts[sourcemesh->pickstack[j]->nearestIdx][1],pts[sourcemesh->pickstack[j]->nearestIdx][2]);

            glEnd();
            glDisable(GL_POINT_SMOOTH);
            glDisable(GL_PROGRAM_POINT_SIZE_EXT);

            renderString3f(pts[sourcemesh->pickstack[j]->nearestIdx][0], pts[sourcemesh->pickstack[j]->nearestIdx][1],pts[sourcemesh->pickstack[j]->nearestIdx][2], (int)small_font, QString::number(j+1), scale+0.2);
            glDepthMask(GL_TRUE);


        }
    }



}

void FindNearestRecording(PickInfo* pick, Mesh_Info* recordingmesh)
{

    qDebug()<<"GeomWindowRepaint - FindNearestRecording";

    int length1 = 0, loop1 = 0, loop_idx_nearest=0, loop_frame=0;

    float **modelpts = 0;
    Mesh_Info* curmesh = pick->mesh;
    Map3d_Geom* curgeom = 0;
    Surf_Data* cursurf = 0;
    curgeom = curmesh->geom;
    cursurf = curmesh->data;
    modelpts = curgeom->points[curgeom->geom_index];
    //std::cout<< "modelpts" <<modelpts[pick->node][0]<<"   "<<modelpts[pick->node][1]<<"     "<<modelpts[pick->node][2]<<std::endl;


    qDebug()<<"1";

    float **recordingpts=0;
    Map3d_Geom *recordinggeom = 0;
    Surf_Data *recordingsurf = 0;
    recordinggeom = recordingmesh->geom;
    recordingsurf = recordingmesh->data;
    recordingpts = recordinggeom->points[recordinggeom->geom_index];
    length1 = recordinggeom->numpts;

    // std::cout<< "recordingpts" <<recordingpts[0][0]<<"   "<<recordingpts[0][1]<<"     "<<recordingpts[0][2]<<std::endl;

    qDebug()<<"2";


    vector<point_t> data_points;

    for (loop1 = 0; loop1 < length1; loop1++)
    {
        coord_t x,y,z;
        x = recordingpts[loop1][0];
        y = recordingpts[loop1][1];
        z = recordingpts[loop1][2];
        data_points.push_back(tr1::make_tuple(x,y,z));
    }


    qDebug()<<"3";

    const size_t nneighbours = 1; // number of nearest neighbours to find

    point_t points[nneighbours];

    point_t point(modelpts[pick->node][0],modelpts[pick->node][1], modelpts[pick->node][2]);

    less_distance nearer(point);


    qDebug()<<"4";

    foreach (point_t& p, points)
        for (loop1 = 0; loop1 < length1; loop1++)
        {
            point_t current_point(recordingpts[loop1][0],recordingpts[loop1][1],recordingpts[loop1][2]);
            foreach (point_t& p, points)

                if (nearer(current_point, p))
                    std::swap(current_point, p);
        }

    sort(boost::begin(points), boost::end(points), nearer);


    qDebug()<<"5";


    foreach (point_t p, points)
    {
        point_t nearestpoint(p);

        for (loop_idx_nearest = 0; loop_idx_nearest< length1; loop_idx_nearest++)
        {
            if (nearestpoint == data_points[loop_idx_nearest])
            {
                pick->nearestIdx = loop_idx_nearest;

                //cout<<"pick->nearestIdx "<<pick->nearestIdx<<std::endl;
                double x_diff=modelpts[pick->node][0]-recordingpts[loop_idx_nearest][0];
                double y_diff=modelpts[pick->node][1]-recordingpts[loop_idx_nearest][1];
                double z_diff=modelpts[pick->node][2]-recordingpts[loop_idx_nearest][2];

                pick->nearestDis=sqrt(x_diff*x_diff+y_diff*y_diff+z_diff*z_diff);

                for (loop_frame = 0; loop_frame <curmesh->data->numframes; loop_frame++)
                {
                    cursurf->nearestrecordingvals[loop_frame][pick->node]= recordingsurf->potvals[loop_frame][loop_idx_nearest];


                }
                qDebug()<<"x,y,z: "<<x_diff<<" "<<y_diff<<" "<<z_diff;

            qDebug()<<"6";
            }
            qDebug()<<"7";
            qDebug()<<"loop_idx_nearest: "<<loop_idx_nearest;
            qDebug()<<"length1: "<<length1;

        }

        qDebug()<<"8";
    }

    qDebug()<<"GeomWindowRepaint - FindNearestRecording - EXIT";
}



void GeomWindow::DrawBackground()                               //Nick - draw simple gradient background.
{
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(0, 1, 0, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        float* pmin = &map3d_info.gi->bgcoords[0];
        float* pmax = &map3d_info.gi->bgcoords[3];

    glBegin(GL_QUADS);
    glColor3f( 0.24f, 0.24f, 0.22f );
    glVertex3f(pmax[0], pmin[1], pmin[2]);
    glVertex3f(pmin[0], pmin[1], pmin[2]);
    glColor3f( 0.94f, 0.93f, 0.91f );
    glVertex3f(pmin[0], pmax[1], pmax[2]);
    glVertex3f(pmax[0], pmax[1], pmax[2]);

    glEnd();
    glClear(GL_DEPTH_BUFFER_BIT);
}

void GeomWindow::switchGradientBackground(bool gradBg)                  //function to switch between gradient and normal Bg - connects to signal from mainWindow.
{
    gradientBackground = gradBg;
}


//void GeomWindow::CalculateMFSValue(Mesh_Info * recordingmesh, Mesh_Info * curmesh)

//{
//    engSetVisible(ep, false);

//    int catheter_num = 0, atria_num =0, loop1 = 0,loop2 = 0;


//    // float **modelpts,**atriapts;
//    long **catheterelement, **atriaelement;


//    Map3d_Geom *recordinggeom = 0;
//    Surf_Data *recordingsurf = 0;
//    recordinggeom = recordingmesh->geom;
//    recordingsurf = recordingmesh->data;
//    catheter_num = recordinggeom->numpts;
//    // modelpts = recordinggeom->points[recordinggeom->geom_index];
//    catheterelement = recordinggeom->elements;


//    Map3d_Geom *curgeom = 0;
//    Surf_Data *cursurf = 0;
//    curgeom = curmesh->geom;
//    cursurf = curmesh->data;
//    atria_num = curgeom->numpts;
//    // atriapts = curgeom->points[curgeom->geom_index];
//    atriaelement = curgeom->elements;

//    // this part is to rotate the catheter. if map3d_info.lockrotate==LOCK_OFF, only apply transform matrix to catheter
//    // if map3d_info.lockrotate==LOCK_FULL, apply both transform matrix to catheter and atrium, corresponding matrix is different.
//    float** pts = recordinggeom->points[recordinggeom->geom_index];
//    float** geom_temp_catheter_pts=pts;
//    float **rotated_catheter_pts = 0;
//    rotated_catheter_pts= Alloc_fmatrix(recordinggeom->numpts, 3);

//    GeomWindow* priv_catheter = recordingmesh->gpriv;
//    HMatrix mNow_catheter /*, original */ ;  // arcball rotation matrices
//    Transforms *tran_catheter = recordingmesh->tran;
//    //translation matrix in column-major
//    float centerM_catheter[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},
//                                    {-priv_catheter->xcenter,-priv_catheter->ycenter,-priv_catheter->zcenter,1}};
//    float invCenterM_catheter[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},
//                                       {priv_catheter->xcenter,priv_catheter->ycenter,priv_catheter->zcenter,1}};
//    float translateM_catheter[4][4] = { {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0},
//                                        {tran_catheter->tx, tran_catheter->ty, tran_catheter->tz, 1}
//                                      };
//    float temp_catheter[16];
//    float product_catheter[16];

//    //rotation matrix
//    Ball_Value(&tran_catheter->rotate, mNow_catheter);
//    // apply translation
//    // translate recordingmesh's center to origin
//    MultMatrix16x16((float *)translateM_catheter, (float *)invCenterM_catheter, (float*)product_catheter);
//    // rotate
//    MultMatrix16x16((float *)product_catheter, (float *)mNow_catheter, (float*)temp_catheter);
//    // revert recordingmesh translation to origin
//    MultMatrix16x16((float*)temp_catheter, (float *) centerM_catheter, (float*)product_catheter);



//    for (loop1 = 0; loop1 < catheter_num; loop1++)
//    {

//        float rhs_catheter[4];
//        float result_catheter[4];
//        rhs_catheter[0] = pts[loop1][0];
//        rhs_catheter[1] = pts[loop1][1];
//        rhs_catheter[2] = pts[loop1][2];
//        rhs_catheter[3] = 1;

//        MultMatrix16x4(product_catheter, rhs_catheter, result_catheter);

//        rotated_catheter_pts[loop1][0] = result_catheter[0];
//        rotated_catheter_pts[loop1][1] = result_catheter[1];
//        rotated_catheter_pts[loop1][2] = result_catheter[2];
//    }

//    geom_temp_catheter_pts=rotated_catheter_pts;

//    //this part is to rotate the source surface (atrium).transform matrix is not applied if map3d_info.lockrotate==LOCK_OFF

//    float** pts_atria = curgeom->points[curgeom->geom_index];
//    float** geom_temp_atria_pts=pts_atria;
//    float **rotated_atria_pts = 0;
//    rotated_atria_pts= Alloc_fmatrix(curgeom->numpts, 3);


//    GeomWindow* priv_atria = curmesh->gpriv;
//    HMatrix mNow_atria /*, original */ ;  // arcball rotation matrices
//    Transforms *tran_atria = curmesh->tran;
//    //translation matrix in column-major
//    float centerM_atria[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},
//                                 {-priv_atria->xcenter,-priv_atria->ycenter,-priv_atria->zcenter,1}};
//    float invCenterM_atria[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},
//                                    {priv_atria->xcenter,priv_atria->ycenter,priv_atria->zcenter,1}};
//    float translateM_atria[4][4] = { {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0},
//                                     {tran_atria->tx, tran_atria->ty, tran_atria->tz, 1}
//                                   };
//    float temp_atria[16];
//    float product_atria[16];

//    //rotation matrix
//    Ball_Value(&tran_atria->rotate, mNow_atria);
//    // apply translation
//    // translate curmesh's center to origin
//    MultMatrix16x16((float *)translateM_atria, (float *)invCenterM_atria, (float*)product_atria);
//    // rotate
//    MultMatrix16x16((float *)product_atria, (float *)mNow_atria, (float*)temp_atria);
//    // revert curmesh translation to origin
//    MultMatrix16x16((float*)temp_atria, (float *) centerM_atria, (float*)product_atria);


//    for (loop2 = 0; loop2 < atria_num; loop2++)
//    {

//        float rhs_atria[4];
//        float result_atria[4];
//        rhs_atria[0] = pts_atria[loop2][0];
//        rhs_atria[1] = pts_atria[loop2][1];
//        rhs_atria[2] = pts_atria[loop2][2];
//        rhs_atria[3] = 1;

//        MultMatrix16x4(product_atria, rhs_atria, result_atria);

//        rotated_atria_pts[loop2][0] = result_atria[0];
//        rotated_atria_pts[loop2][1] = result_atria[1];
//        rotated_atria_pts[loop2][2] = result_atria[2];

//    }
//    geom_temp_atria_pts=rotated_atria_pts;



//    // pass coordinates of catheters
//    double pot_temp[catheter_num], cath_x[catheter_num],cath_y[catheter_num],cath_z[catheter_num];

//    for (int i=0; i< catheter_num; i++)
//    {

//        if (recordingsurf->forwardvals[recordingsurf->framenum][i]==0)
//        {pot_temp[i] =recordingsurf->potvals[recordingsurf->framenum][i];}
//        else
//        {pot_temp[i] =recordingsurf->forwardvals[recordingsurf->framenum][i];}


//        cath_x[i] =  geom_temp_catheter_pts[i][0];
//        cath_y[i] =  geom_temp_catheter_pts[i][1];
//        cath_z[i] =  geom_temp_catheter_pts[i][2];
//    }

//    mxArray *catheter_potential_matlab = mxCreateDoubleMatrix(1,catheter_num, mxREAL);
//    memcpy(mxGetPr(catheter_potential_matlab), pot_temp, catheter_num*sizeof(double));
//    engPutVariable(ep, "catheter_potential",catheter_potential_matlab);

//    mxArray *cath_x_matlab = mxCreateDoubleMatrix(1,catheter_num, mxREAL);
//    memcpy(mxGetPr(cath_x_matlab), cath_x, catheter_num*sizeof(double));
//    engPutVariable(ep, "c_x",cath_x_matlab);

//    mxArray *cath_y_matlab = mxCreateDoubleMatrix(1,catheter_num, mxREAL);
//    memcpy(mxGetPr(cath_y_matlab), cath_y, catheter_num*sizeof(double));
//    engPutVariable(ep, "c_y",cath_y_matlab);

//    mxArray *cath_z_matlab = mxCreateDoubleMatrix(1,catheter_num, mxREAL);
//    memcpy(mxGetPr(cath_z_matlab), cath_z, catheter_num*sizeof(double));
//    engPutVariable(ep, "c_z",cath_z_matlab);

//    // pass coordinates of atria
//    double atria_x[atria_num],atria_y[atria_num],atria_z[atria_num];
//    for (int i=0; i< atria_num; i++)
//    {

//        atria_x[i] =  geom_temp_atria_pts[i][0];
//        atria_y[i] =  geom_temp_atria_pts[i][1];
//        atria_z[i] =  geom_temp_atria_pts[i][2];
//    }
//    mxArray *atria_x_matlab = mxCreateDoubleMatrix(1,atria_num, mxREAL);
//    memcpy(mxGetPr(atria_x_matlab), atria_x, atria_num*sizeof(double));
//    engPutVariable(ep, "a_x",atria_x_matlab);

//    mxArray *atria_y_matlab = mxCreateDoubleMatrix(1,atria_num, mxREAL);
//    memcpy(mxGetPr(atria_y_matlab), atria_y, atria_num*sizeof(double));
//    engPutVariable(ep, "a_y",atria_y_matlab);

//    mxArray *atria_z_matlab = mxCreateDoubleMatrix(1,atria_num, mxREAL);
//    memcpy(mxGetPr(atria_z_matlab), atria_z, atria_num*sizeof(double));
//    engPutVariable(ep, "a_z",atria_z_matlab);



//    // pass elements of catheters
//    double c_ele_1[recordinggeom->numelements],c_ele_2[recordinggeom->numelements],c_ele_3[recordinggeom->numelements];
//    for (int j=0; j< recordinggeom->numelements; j++)
//    {
//        c_ele_1[j] = catheterelement[j][0]+1;
//        c_ele_2[j] = catheterelement[j][1]+1;
//        c_ele_3[j] = catheterelement[j][2]+1;
//    }

//    mxArray *cath_e1_matlab = mxCreateDoubleMatrix(1,recordinggeom->numelements, mxREAL);
//    memcpy(mxGetPr(cath_e1_matlab), c_ele_1, recordinggeom->numelements*sizeof(double));
//    engPutVariable(ep, "c_ele_1",cath_e1_matlab);

//    mxArray *cath_e2_matlab = mxCreateDoubleMatrix(1,recordinggeom->numelements, mxREAL);
//    memcpy(mxGetPr(cath_e2_matlab), c_ele_2, recordinggeom->numelements*sizeof(double));
//    engPutVariable(ep, "c_ele_2",cath_e2_matlab);

//    mxArray *cath_e3_matlab = mxCreateDoubleMatrix(1,recordinggeom->numelements, mxREAL);
//    memcpy(mxGetPr(cath_e3_matlab), c_ele_3, recordinggeom->numelements*sizeof(double));
//    engPutVariable(ep, "c_ele_3",cath_e3_matlab);




//    // pass elements of atria
//    double a_ele_1[curgeom->numelements],a_ele_2[curgeom->numelements],a_ele_3[curgeom->numelements];
//    for (int k=0; k< curgeom->numelements; k++)
//    {
//        a_ele_1[k] = atriaelement[k][0]+1;
//        a_ele_2[k] = atriaelement[k][1]+1;
//        a_ele_3[k] = atriaelement[k][2]+1;

//    }

//    mxArray *atria_e1_matlab = mxCreateDoubleMatrix(1,curgeom->numelements, mxREAL);
//    memcpy(mxGetPr(atria_e1_matlab), a_ele_1, curgeom->numelements*sizeof(double));
//    engPutVariable(ep, "a_ele_1",atria_e1_matlab);

//    mxArray *atria_e2_matlab = mxCreateDoubleMatrix(1,curgeom->numelements, mxREAL);
//    memcpy(mxGetPr(atria_e2_matlab), a_ele_2, curgeom->numelements*sizeof(double));
//    engPutVariable(ep, "a_ele_2",atria_e2_matlab);

//    mxArray *atria_e3_matlab = mxCreateDoubleMatrix(1,curgeom->numelements, mxREAL);
//    memcpy(mxGetPr(atria_e3_matlab), a_ele_3, curgeom->numelements*sizeof(double));
//    engPutVariable(ep, "a_ele_3",atria_e3_matlab);


//    engEvalString(ep, "addpath(genpath('/hpc_ntot/smen974/Map3d/MFS_Functions'))");
//    engEvalString(ep, "mfsEGM=solve_MFS(c_x,c_y,c_z,c_ele_1,c_ele_2,c_ele_3,a_x,a_y,a_z,a_ele_1,a_ele_2,a_ele_3, catheter_potential)");

//    mxArray *mfsEGM_matlab = engGetVariable(ep, "mfsEGM");
//    double *mfsEGM = mxGetPr(mfsEGM_matlab);

//    string filename;
//    ofstream files;
//    stringstream a;
//    a << cursurf->framenum;
//    filename = "inverse_96_" + a.str();
//    filename += ".txt";
//    files.open(filename.c_str(), ios::out);


//    for (int i = 0; i <atria_num; i++)
//    {
//        cursurf->MFSvals[cursurf->framenum][i]=mfsEGM[i];


//        files << cursurf->MFSvals[cursurf->framenum][i] << " " ;
//        files << "\n";
//    }
//}


