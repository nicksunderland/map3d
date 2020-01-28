/* GeomWindow.h */

#ifndef GEOMWINDOW_H
#define GEOMWINDOW_H


#ifdef _WIN32
#define MAP3D_SLEEP Sleep
#define MULTIPLIER 1
#else
#define MAP3D_SLEEP usleep
#define MULTIPLIER 1000
#endif

#include "GenericWindow.h"
#include "MeshList.h"
#include "GeomWindowMenu.h"


#include <QTimer>

#include <QAbstractButton>

class Surf_Data;
struct PickInfo;
struct Clip_Planes;


#include <boost/foreach.hpp>
#include <boost/range.hpp>     // begin(), end()
#include <boost/tr1/tuple.hpp> // get<>, tuple<>, cout <<


namespace {
typedef double coord_t;
typedef std::tr1::tuple<coord_t,coord_t,coord_t> point_t;

// calculate distance (squared) between points `a` & `b`
coord_t distance_sq(const point_t& a, const point_t& b);

// read from input stream `in` to the point `point_out`
std::istream& getpoint(std::istream& in, point_t& point_out);

// Adaptable binary predicate that defines whether the first
// argument is nearer than the second one to given reference point
class less_distance : public std::binary_function<point_t, point_t, bool> {
    const point_t& point;
public:
    explicit less_distance(const point_t& reference_point)
        : point(reference_point) {}
    bool operator () (const point_t& a, const point_t& b) const {
        return distance_sq(a, point) < distance_sq(b, point);
    }
};
}


namespace {

coord_t distance_sq(const point_t& a, const point_t& b) {
    // boost::geometry::distance() squared
    using std::tr1::get;
    coord_t x = get<0>(a) - get<0>(b);
    coord_t y = get<1>(a) - get<1>(b);
    coord_t z = get<2>(a) - get<2>(b);
    return x*x + y*y + z*z;
}

std::istream& getpoint(std::istream& in, point_t& point_out) {
    using std::tr1::get;
    return (in >> get<0>(point_out) >> get<1>(point_out) >> get<2>(point_out));
}
}


class GeomWindow : public Map3dGLWidget
{
    Q_OBJECT;

public:
    GeomWindow(QWidget* parent);
    static GeomWindow* GeomWindowCreate(int _width, int _height, int _x, int _y);

    virtual void mouseMoveEvent ( QMouseEvent * event );
    virtual void mousePressEvent ( QMouseEvent * event );
    virtual void mouseReleaseEvent ( QMouseEvent * event );
    virtual void keyPressEvent ( QKeyEvent * event );
    virtual void keyReleaseEvent ( QKeyEvent * event );
    virtual void closeEvent(QCloseEvent * event);




    int OpenMenu(QPoint point);
    void MenuEvent(int menu_data);

    virtual void initializeGL();
    virtual void paintGL ();

    // versions to call post-broadcast
    void HandleButtonPress(QMouseEvent * event, float xn, float yn);
    void HandleButtonRelease(QMouseEvent * event, float xn, float yn);
    void HandleMouseMotion(QMouseEvent * event, float xn, float yn);
    bool testKeyboardRepeat(bool active);
    void UpdateAndRedraw();
    void TransformKeyboard(Mesh_Info * curmesh, QKeyEvent* event);
    void HandleKeyPress(QKeyEvent * event);
    void HandleKeyRelease(QKeyEvent * event);
    void HandleMenu(int menu_data);
    bool MenuGlobalOptions(int menu);

    void UpdateNearestPoints(Mesh_Info* recordingmesh,Mesh_Info* sourcemesh);


    void addMesh(Mesh_Info* mesh);
    void removeMesh(Mesh_Info* mesh);

    Mesh_List findMeshesFromSameInput(Mesh_Info* mesh);
    void recalcMinMax();
    float vfov;                   /* vertical field of view */
    Mesh_List meshes;             /* info for all the meshes in this
                                   window: geom, data, contours, colormaps,... */

    int geomWinId;

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

    char useonepickwindow;        /* when picking, show only one pick at a time */
    char showlocks;

    float large_font;                // size of font for the large, medium, small fonts
    float med_font;                  //   use MAP3D_FONT_SIZE_* from glprintf.h
    float small_font;                //   these are floats to appease dynamic changing ( +/- by size, which are float*)

    bool Pick(int meshnum, int x, int y, bool del = false);

    // drawing helper functions
    void Transform(Mesh_Info * curmesh, float factor, bool compensateForRetinaDisplay);
    void DrawNodes(Mesh_Info * curmesh);
    void DrawInfo();
    void DrawLockSymbol(int which, bool full);
    void DrawAxes(Mesh_Info * curmesh);
    void DrawBGImage();
    void DrawBackground();                               //Nick - draw simple gradient background.
    bool checkArrayZero(int unlock_MFS_surfnum[]);

    void DrawElectrodesOnly(Mesh_Info * curmesh);
    void DrawDatacloudOnly(Mesh_Info * curmesh);
    void DrawForwardOnly(Mesh_Info * curmesh);
    void CalculateForwardValue(Mesh_Info * curmesh, Mesh_Info * sourcemesh);

    //void CalculateMFSValue(Mesh_Info *recordingmesh, Mesh_Info * curmesh);

    void DrawMFS(Mesh_Info * curmesh);
    void DrawPhase(Mesh_Info * curmesh);


    // how much to scale the node marks by
    float fontScale();
    QTimer frameAdvanceTimer;
public slots:
    bool HandleFrameAdvances();
    bool AdvanceButtonControl();
    bool BackwardButtonControl();
    bool ButtonControlFirstFrame();
    bool ButtonControlLastFrame();
    bool mouseSliderEvent();
    bool PauseButtonControl();
    bool PlayButtonControl();
    bool SpinBoxEvent();
    void switchGradientBackground(bool gradBg);                    //Nick - draw simple gradient background to geomWindow
    void setNodePickLock(bool nodePickStateFromGUI);           //Nick - set the Node picking lock from the ABI GUI
    void setCathTransLock(bool cathTransLockFromGUI);         //Nick - set the Node picking lock from the ABI GUI
    void setCathRotationLock(bool cathRotationLockFromGUI);//Nick - set the Node picking lock from the ABI GUI
    void setCathWinLock(bool cathWinLockFromGUI);

signals:
    void sendPicktoGUI(PickInfo *pick);

private:
    bool gradientBackground;                                                    //Nick
    bool nodePickLock;                                                              //Nick
    bool cathTransLock;                                                             //Nick
    bool cathRotationLock;                                                         //Nick
    bool cathWinLock;                                                               //Nick

};

#ifdef __cplusplus
extern "C"
{
#endif

struct mouse_button_data;
struct mouse_motion_data;
struct special_key_data;
struct key_data;

// to avoid including gtk.h here
typedef union _GdkEvent GdkEvent;
typedef struct _GdkEventButton GdkEventButton;
typedef struct _GdkEventMotion GdkEventMotion;
typedef struct _GdkEventKey GdkEventKey;

void GeomWindowIdleFunc(void* data);
void DrawSurf(Mesh_Info * curmesh);

void DrawMesh(Mesh_Info * curmesh, bool secondary);
void DrawCont(Mesh_Info * curmesh);
void DrawFidCont(Mesh_Info * curmesh, Contour_Info *cont);
void DrawFidMapCont(Mesh_Info * curmesh, Contour_Info *cont);
void DrawFidMapSurf(Mesh_Info * curmesh,Contour_Info *cont);
void DrawPicks(Mesh_Info * curmesh);
void DrawLeads(Mesh_Info * curmesh);
void DrawNodePick(PickInfo * pick);
void DrawExtrema(Mesh_Info * curmesh);
void DrawDot(float x, float y, float z, float size);
void GeneratePick(PickInfo * pick);

void FindNearestRecording(PickInfo * pick, Mesh_Info * recordingmesh);



void DelTriangle(Mesh_Info * curmesh, int nodenum);
void DelNode(Mesh_Info * curmesh, int nodenum);
void Triangulate(Mesh_Info * curmesh, int nodenum);
void SaveGeomToDisk(Mesh_Info * mesh, bool trans);
bool SaveMeshes(Mesh_List& ml, vector<bool> transforms, char* filename);
#ifdef __cplusplus
}
#endif

#endif
