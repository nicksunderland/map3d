#ifndef ACTIVATIONMAPWINDOW_H
#define ACTIVATIONMAPWINDOW_H


#include "GenericWindow.h"
#include "MeshList.h"
#include "GeomWindowMenu.h"

class Surf_Data;
struct Clip_Planes;



class ActivationMapWindow : public Map3dGLWidget
{

public:
    Q_OBJECT;
  public:
    ActivationMapWindow(QWidget* parent);
    static ActivationMapWindow* ActivationMapWindowCreate(int _width, int _height, int _x, int _y);

    virtual void mouseMoveEvent ( QMouseEvent * event );
    virtual void mousePressEvent ( QMouseEvent * event );
    virtual void mouseReleaseEvent ( QMouseEvent * event );
    virtual void closeEvent(QCloseEvent * event);

    int OpenMenu(QPoint point);
    void MenuEvent(int menu_data);
    void HandleMenu(int menu_data);
    void DrawNodes(Mesh_Info * curmesh);
    float fontScale();


    virtual void initializeGL();
    virtual void paintGL ();

    // versions to call post-broadcast
    void HandleButtonPress(QMouseEvent * event, float xn, float yn);
    void HandleButtonRelease(QMouseEvent * event, float xn, float yn);
    void HandleMouseMotion(QMouseEvent * event, float xn, float yn);

    void addMesh(Mesh_Info* mesh);


    void Transform(Mesh_Info * curmesh, float factor, bool compensateForRetinaDisplay);
    void DrawBGImage();

    void DrawSurf(Mesh_Info * curmesh);
    void DrawInfo();


   // Mesh_List findMeshesFromSameInput(Mesh_Info* mesh);

    void recalcMinMax(Mesh_Info *mesh);

    float vfov;                   /* vertical field of view */
    Mesh_List meshes;             /* info for all the meshes in this
                                     window: geom, data, contours, colormaps,... */

    int ActivationWinId;

    float l2norm;                 /* the "fit" info for this window */
    float xcenter, ycenter, zcenter;
    float xmin, xmax, ymin, ymax, zmin, zmax;
    float fog1,fog2;

    float light_position[4];
    int lighting_direction;        /* indicates above, below, front, etc. light direction - for menu check mark*/


    Clip_Planes *clip;
  #ifdef ROTATING_LIGHT
    BallData light_pos;
  #endif

    long dominantsurf; /*** Dominant surface number ***/
    long secondarysurf; /*** Surf when frames are unlocked and all displayed
                        in same window ***/

    char all_axes;                /* to display all meshes' axes or only one */
    bool rgb_axes;



    float large_font;                // size of font for the large, medium, small fonts
    float med_font;                  //   use MAP3D_FONT_SIZE_* from glprintf.h
    float small_font;                //   these are floats to appease dynamic changing ( +/- by size, which are float*)

};

#endif // ACTIVATIONMAPWINDOW_H
