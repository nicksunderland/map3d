/* PickWindow.cxx */

#ifdef _WIN32
#include <windows.h>
#pragma warning(disable:4505)
#undef TRACE
#endif
#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif
#include <float.h>
#include <iostream>
using namespace std;

#include "PickWindow.h"
#include "Contour_Info.h"
#include "GeomWindow.h"
#include "LegendWindow.h"
#include "WindowManager.h"
#include "dialogs.h"
#include "ContourDialog.h"
#include "eventdata.h"
#include "glprintf.h"
#include "map3d-struct.h"
#include "pickinfo.h"
#include "Map3d_Geom.h"
#include "Surf_Data.h"
#include "scalesubs.h"
#include "savescreen.h"
#include "MainWindow.h"
#include "FileDialog.h"

#include "GetMatrixSlice.h"

#include <QMouseEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>


#define PICK_INSIDE  0
#define PICK_OUTSIDE 1
#define PICK_LEFT    2
#define PICK_RIGHT   3

extern Map3d_Info map3d_info;
extern const char *units_strings[5];
extern int fstep, cur, fstart, fend;
extern int key_pressed;
extern int pick;
extern int delay;
extern MainWindow *masterWindow;

extern GetMatrixSlice* getmatrixslice;

extern QTimer *play_timer;


PickInfo *pickstack[100] = { 0 };
int pickstacktop = -1;



#define CHAR_SIZE 70
#define CHAR_BIG CHAR_SIZE * 4
#define CHAR_MED CHAR_SIZE * 3
#define PROJ_SIZE 1000.f

static const int min_width = 100;
static const int min_height = 100;
static const int default_width = 328;
static const int default_height = 144;

GLuint selectbufferPick[2048];

enum pickmenu { axes_color, graph_color, full_screen, graph_width_menu, toggle_subseries_mode, draw_nearest_electrogram};


PickWindow::PickWindow(QWidget* parent) : Map3dGLWidget(parent)
{
    SetStyle(0);
    mesh = 0;
    pick = 0;
}

PickWindow::PickWindow(QWidget* parent, bool rms) : Map3dGLWidget(parent, (rms?RMSWINDOW:TIMEWINDOW),"Time Signal", 260, 120), rms(rms)
{
    if (wintype == TIMEWINDOW) {
        SetStyle(1);
    }
    else {
        SetStyle(0);
    }
    mesh = 0;
    fileWidget = 0;

    Map3DsetupGUI = map3d_info.GUIchoice;   //Nick
    window1electrogram = true;
    window2electrogram = true;
    window3electrogram = true;                        //Nick
}

//static
PickWindow* PickWindow::PickWindowCreate(int _width, int _height, int _x, int _y)
{

    //std::cout<<"picking window crash test entering  PickWindowCreate  "<<std::endl;


    if (map3d_info.numPickwins >= MAX_PICKS) {
        printf("Warning: cannot create more than %d Time Series Windows\n", MAX_PICKS);
        return 0;
    }
    PickWindow* win = new PickWindow(masterWindow ? masterWindow->childrenFrame : NULL, false);
    win->positionWindow(_width, _height, _x, _y, default_width, default_height);

    map3d_info.pickwins[map3d_info.numPickwins] = win;
    map3d_info.numPickwins++;



    // don't show until mesh and pick have been set up
    //win->show();
    return win;
}

PickWindow::~PickWindow()
{
}

void PickWindow::initializeGL()
{

    //std::cout<<"picking window crash test entering  initializeGL  "<<std::endl;


    Map3dGLWidget::initializeGL();
    axiscolor[0] = 0;
    axiscolor[1] = 0;
    axiscolor[2] = 0;
    axiscolor[3] = 1;

    graphcolor[0] = .1f;
    graphcolor[1] = .75f;
    graphcolor[2] = .1f;
    graphcolor[3] = 1.0f;
    graph_width = 2;



    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glShadeModel(GL_FLAT);

    glDepthMask(GL_FALSE);
    glSelectBuffer(2048, selectbufferPick);

    //    if (mesh && mesh->gpriv) {

    //        //std::cout<<"picking window crash test entering  initializeGL  if (mesh && mesh->gpriv)   "<<std::endl;

    //        GeomWindow* geom = (GeomWindow *) mesh->gpriv;
    //        bgcolor[0] = geom->bgcolor[0];
    //        bgcolor[1] = geom->bgcolor[1];
    //        bgcolor[2] = geom->bgcolor[2];
    //        bgcolor[3] = geom->bgcolor[3];

    //        fgcolor[0] = geom->fgcolor[0];
    //        fgcolor[1] = geom->fgcolor[1];
    //        fgcolor[2] = geom->fgcolor[2];
    //        fgcolor[3] = geom->fgcolor[3];

    //        bgcolor[0] = bgcolor[1] = bgcolor[2] = bgcolor[3] = 1;
    //        fgcolor[0] = fgcolor[1] = fgcolor[2] = fgcolor[3] = 0;

    //    }
    //    else {

    //        //std::cout<<"picking window crash test entering  initializeGL  else   "<<std::endl;

    bgcolor[0] = bgcolor[1] = bgcolor[2] = bgcolor[3] = 1;
    fgcolor[0] = fgcolor[1] = fgcolor[2] = fgcolor[3] = 0;
    // }


}

void PickWindow::Destroy()
{
    int i, j = -1;

    for (i = 0; i <= mesh->pickstacktop; i++) {
        if (mesh->pickstack[i]->pickwin == this) {
            delete mesh->pickstack[i];

            mesh->pickstacktop--;
            for (j = i; j <= mesh->pickstacktop; j++)
                mesh->pickstack[j] = mesh->pickstack[j + 1];

            break;
        }
    }
    //    for (i = 0; i < map3d_info.numPickwins; i++) {
    //        if (map3d_info.pickwins[i] == this) {
    //            map3d_info.numPickwins--;
    //            for (j = i; j < map3d_info.numPickwins; j++)
    //                map3d_info.pickwins[j] = map3d_info.pickwins[j + 1];
    //            break;
    //        }
    //    }
    //    if (j != -1) {
    //        map3d_info.pickwins[j] = this;
    //        DestroyWindow(this);
    //    }
    //    mesh->gpriv->update();
    //    pick = NULL;
    //    mesh = NULL;
}

void PickWindow::paintGL()
{

    if (mesh == NULL)
        return;
    if (!rms && pick == NULL)
        return;
    // this compensates for the "Retina" display ratio.  See http://doc.qt.io/qt-5/highdpi.html
    //  (for some reason the picking doesn't need this)
    int pixelFactor = QApplication::desktop()->devicePixelRatio();
    glViewport(0, 0, width()*pixelFactor, height()*pixelFactor);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    //projection in pixels
    gluOrtho2D(0, width(), 0, height());
    glMatrixMode(GL_MODELVIEW);

    glClearColor(bgcolor[0], bgcolor[1], bgcolor[2], 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    DrawNode();
}

void PickWindow::enterEvent(QEvent*)
{
    //std::cout<<"picking window crash test entering enterEvent   "<<std::endl;

    if (rms) return;
    mesh->curpicknode = pick->node;
    mesh->gpriv->update();
}

void PickWindow::leaveEvent(QEvent*)
{

    //std::cout<<"picking window crash test entering leaveEvent    "<<std::endl;

    if (rms) return;
    mesh->curpicknode = -1;
    mesh->gpriv->update();
}

void PickWindow::mouseReleaseEvent(QMouseEvent* event)
{

    //std::cout<<"picking window crash test entering mouseReleaseEvent    "<<std::endl;

    if (rms)
    {
        RMSButtonRelease(event);
        return;
    }

    if(!mesh)
        return;

    // redraw all relevant windows.  The frame should already be selected
    if (click && event->button() == Qt::LeftButton && mesh->data) {
        if (!map3d_info.lockframes) {
            // if advancing in time only affects this surface
            if (map3d_info.scale_scope != GLOBAL_FRAME && map3d_info.scale_scope != GROUP_FRAME &&
                    map3d_info.scale_scope != SLAVE_FRAME) {
                mesh->gpriv->UpdateAndRedraw();
            }
            // it affects at least one other surface...
            else {
                Broadcast(MAP3D_UPDATE);
            }
        }
        else
            Broadcast(MAP3D_UPDATE);

        // if animating, save since we redrew the new frame
        if (map3d_info.saving_animations) {
            SaveScreen();
        }
    }
}



// in here we hack the original values of event->x and event->y
void PickWindow::mousePressEvent(QMouseEvent* event)
{
    //std::cout<<"picking window crash test entering mousePressEvent    "<<std::endl;

    if (rms)
    {
        RMSButtonPress(event);
        return;
    }

    state = 1;

    setMoveCoordinates(event);

    if(!mesh)
        return;

    float distance;
    button = event->button();
    int x = (int)event->x();
    int y = (int)(height() - event->y());

    int deltaFrames = 0;

    click = false;
    if (event->button() == Qt::RightButton) { // right click - menu popup
        int menu_data = OpenMenu(mapToGlobal(event->pos()));
        if (menu_data >= 0)
            MenuEvent(menu_data);
    }

    /* LEFT MOUSE DOWN = select frame in graph */
    else if (event->button() == Qt::LeftButton && event->modifiers() == Qt::NoModifier && mesh->data) {
        map3d_info.scale_frame_set = 0;
        if (y < height() * topoffset && y > height() * bottomoffset &&
                x > width() * leftoffset && x < width() * rightoffset) {
            click = true;

            int left, right;
            getFrameRange(mesh->data->CurrentSubseries(), left, right);
            int numFrames = right - left;

            distance = (x - width() * leftoffset) / (rightoffset - leftoffset) / width(); // percentage of the graph the click is over
            int newFrame = left + distance * (numFrames-1);
            deltaFrames = -(mesh->data->framenum - newFrame);

            map3d_info.selected_group = (map3d_info.lockframes == LOCK_GROUP)
                    ? mesh->groupid : -1;
            ComputeLockFrameData();

            // clamp delta frames to the lock frame data
            if (deltaFrames < 0 && map3d_info.lockframes)
                deltaFrames = MAX(deltaFrames, fstart-cur);
            else if (map3d_info.lockframes)
                deltaFrames = MIN(deltaFrames, fend-cur);


            if (!map3d_info.lockframes) {
                mesh->data->FrameAdvance(deltaFrames);
                // FIX updateContourDialogValues(mesh);
            }
            else
                Broadcast(MAP3D_PICK_FRAMES, this, event, deltaFrames);

        }

    }
    update();
}



void PickWindow::RMSButtonRelease(QMouseEvent * event)
{

}

void PickWindow::RMSButtonPress(QMouseEvent * event)
{

    //std::cout<<"picking window crash test entering RMSButtonEvent    "<<std::endl;

    state = 1;

    if(!mesh)
        return;

    //ComputeLockFrameData();
    float distance;
    int button = event->button();
    int x = (int)event->x();
    int y = (int)(height() - event->y());

    click = false;

    if ((button == Qt::LeftButton) && mesh->data) {
        map3d_info.scale_frame_set = 0;
        if (y < height() * topoffset && y > height() * bottomoffset &&
                x > width() * leftoffset && x < width() * rightoffset) {
            click = true;
            distance = (x - width() * leftoffset) / (rightoffset - leftoffset) / width();
            distance *= (mesh->data->numframes-1);

            // set window_line to 0 if the click is closer to the left line, 1 if closer to right
            int clicked_frame = (int) (distance+1);

            Q_ASSERT(fileWidget);
            int start = fileWidget->startFrameSpinBox->value();
            int end = fileWidget->endFrameSpinBox->value();
            window_line = (abs(clicked_frame-start) > abs(clicked_frame-end)) ? 1 : 0;

            if(window_line == 0){
                if(distance > end) {
                    fileWidget->startFrameSpinBox->setValue(end);
                }else{
                    fileWidget->startFrameSpinBox->setValue(distance+1);
                }
            }
            else if(window_line == 1){
                if(distance < start){
                    fileWidget->endFrameSpinBox->setValue(start);
                }else{
                    fileWidget->endFrameSpinBox->setValue(distance+1);
                }
            }
        }
    }

    update();
}

void PickWindow::mouseMoveEvent(QMouseEvent* event)
{
    //std::cout<<"picking window crash test entering mouseMoveEvent    "<<std::endl;

    play_timer->stop();

    if (rms)
    {
        RMSMotion(event);
        return;
    }

    if(!mesh)
        return;

    float distance;
    int deltaFrames = 0;
    int x = (int)event->x();
    int y = (int)(height() - event->y());

    if (event->buttons() == Qt::LeftButton && event->modifiers() & Qt::AltModifier) {
        moveEvent(event);
    }
    else if (event->buttons() == Qt::MidButton && event->modifiers() & Qt::AltModifier)
    {
        sizeEvent(event);
    }

    else if (event->buttons() == Qt::LeftButton && mesh->data) {
        //x -= width / 10;
        int left, right;
        getFrameRange(mesh->data->CurrentSubseries(), left, right);
        int numFrames = right - left;

        distance = (x - width() * leftoffset) / (rightoffset - leftoffset) / width(); // percentage of the graph the click is over
        int newFrame = left + distance * (numFrames - 1);
        deltaFrames = -(mesh->data->framenum - newFrame);

        deltaFrames = -(mesh->data->framenum - newFrame);


        map3d_info.scale_frame_set = 0;

        map3d_info.selected_group = (map3d_info.lockframes == LOCK_GROUP)
                ? mesh->groupid : -1;
        ComputeLockFrameData();

        if (y < height() * topoffset && y > height() * bottomoffset &&
                x > width() * leftoffset && x < width() * rightoffset) {
            click= true; // to signify that we are dragging inside the pick window

            // clamp delta frames to the lock frame data
            if (deltaFrames < 0 && map3d_info.lockframes)
                deltaFrames = MAX(deltaFrames, fstart-cur);
            else if (map3d_info.lockframes)
                deltaFrames = MIN(deltaFrames, fend-cur);
        }

        // we used to be in the frame but we're not anymore, so redraw the relevant windows
        else if (click) {
            click = false;
            //      for (i = 0; i < length; i++) {
            if (x <= width() * .1) {
                deltaFrames = (map3d_info.lockframes ? fstart - cur : mesh->data->ts_start - mesh->data->framenum);
            }
            else if (x >= width() * .95) {
                deltaFrames = (map3d_info.lockframes ? fend - cur : mesh->data->ts_end - mesh->data->framenum);
            }

            if (!map3d_info.lockframes) {
                mesh->data->FrameAdvance(deltaFrames);
                // if advancing in time only affects this surface
                if (map3d_info.scale_scope != GLOBAL_FRAME && map3d_info.scale_scope != GROUP_FRAME &&
                        map3d_info.scale_scope != SLAVE_FRAME) {
                    mesh->gpriv->UpdateAndRedraw();
                }
                // it affects at least one other surface...
                else {
                    Broadcast(MAP3D_UPDATE, this, event);
                }
            }
            else {
                Broadcast(MAP3D_PICK_FRAMES, this, event, deltaFrames);
                Broadcast(MAP3D_UPDATE, this, event);
            }

            // if animating, save since we redrew the new frame
            if (map3d_info.saving_animations) {
                SaveScreen();
            }
            return;
        }
        else {
            return;
        }

        // update the frames but don't redraw
        if (!map3d_info.lockframes)
            mesh->data->FrameAdvance(deltaFrames);
        else
            Broadcast(MAP3D_PICK_FRAMES, this, event, deltaFrames);
        update();
    }
}

void PickWindow::RMSMotion(QMouseEvent* event)
{

    //std::cout<<"picking window crash test entering RMSMotion   "<<std::endl;

    if(!mesh)
        return;

    float distance;
    int x = (int)event->x();
    int y = (int)(height() - event->y());


    int stat = PICK_INSIDE;


    if (event->buttons() == Qt::LeftButton && mesh->data) {
        //x -= width / 10;
        distance = (x - width() * leftoffset) / (rightoffset - leftoffset) / width();

        distance = distance * (mesh->data->numframes-1);


        if (y < height() * topoffset && y > height() * bottomoffset &&
                x > width() * leftoffset && x < width() * rightoffset) {
            click = true; // to signify that we are dragging inside the pick window
        }

        // we used to be in the frame but we're not anymore, so set frames to max extent
        else if (click) {
            stat = PICK_OUTSIDE;
            click = false;
            //      for (i = 0; i < length; i++) {
            if (x <= width() * .1) {

                stat = PICK_LEFT;
                distance = 0;
            }
            else if (x >= width() * .95) {
                stat = PICK_RIGHT;
                distance = 1;
            }

        }

        int start = fileWidget->startFrameSpinBox->value();
        int end = fileWidget->endFrameSpinBox->value();
        // set the frame vals - window_line was set in ButtonPress based on which line we were closer to
        if(window_line == 0)
        {
            if(distance > end){
                fileWidget->startFrameSpinBox->setValue(end);
            }else{
                fileWidget->startFrameSpinBox->setValue(distance+1);
            }
        }
        if(window_line == 1)
        {
            if(distance < start){
                fileWidget->endFrameSpinBox->setValue(start);
            }else{
                fileWidget->endFrameSpinBox->setValue(distance+1);
            }
        }
    }

    //delete Data;
}

void PickWindow::keyReleaseEvent(QKeyEvent* event)
{
    //std::cout<<"picking window crash test entering keyReleaseEvent    "<<std::endl;

    if (rms) return;
    GeomWindow *gpriv = mesh->gpriv;
    if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right) {
        mesh->gpriv->keyReleaseEvent(event);
    }
}

void PickWindow::keyPressEvent(QKeyEvent* event)
{

    //std::cout<<"picking window crash test entering keyPressEvent    "<<std::endl;

    if (rms) return;

    int key = event->key();
    char keyChar = event->text().toLatin1()[0];
    if (keyChar == 'p') {
        MenuEvent(full_screen);
    }
    else if (keyChar == 'q') {
        pick->show = 0;
        setVisible(!pick->show);
    }
    else if (key == Qt::Key_Escape) {
        pick->show = 0;
        Destroy();
    }
    else if (key == Qt::Key_Left || key == Qt::Key_Right || keyChar == 'f' || key == Qt::Key_Plus || key == Qt::Key_Minus)
    {
        mesh->gpriv->keyPressEvent(event);
    }
}


bool PickWindow::MatrixOnlyContainZero(Surf_Data* data,float **matrixvals)

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


void PickWindow::DrawNode()
{


    //std::cout<<"picking window crash test entering DrawNode   "<<std::endl;

    int loop;
    float a, b;
    float d;
    float min = FLT_MAX, max = -FLT_MAX;
    float right = rightoffset;
    float left = leftoffset;
    float top = topoffset;
    float bottom = bottomoffset;

    float pos[3] = { 5.f, (float)height(), 0 };
    //float norm[3]={0,0,-1}, up[3]={0,1,0};
    //float aspect = .62f * height / width;
    float coloroffset = .5f;

    // this is for the case in the files dialog with an empty row
    if (!mesh)
        return;
    Surf_Data* data = mesh->data;


    /* Find the extrema of the time signal */
    if (data && rms) {
        for (loop = 0; loop < data->numframes; loop++) {
            if (data->rmspotvals[loop] < min)
                min = data->rmspotvals[loop];
            if (data->rmspotvals[loop] > max)
                max = data->rmspotvals[loop];
        }
    }
    else if (data){

        if ( MatrixOnlyContainZero(data,data->potvals)==0)

        {
            max = map3d_info.global_potmax;
            min = map3d_info.global_potmin;
        }


        if (( MatrixOnlyContainZero(data,data->forwardvals)==0) && (MatrixOnlyContainZero(data,data->potvals)==1))

        {
            long framenum, leadnum, numvals;

            float val, minval, maxval;
            double sum, rmsSum;

            /*** Loop through all the frames.  ***/

            for (framenum = 0; framenum < data->numframes; framenum++) {
                minval = 1.0e10;
                maxval = -1.e10;

                /*** Loop through all the nodes in the geometry = values of potential. ***/

                sum = 0.0;
                rmsSum = 0.0;
                numvals = 0;//Check for channels later
                for (leadnum = 0; leadnum < data->numleads; leadnum++) {
                    val =data->forwardvals[framenum][leadnum];
                    if (val == UNUSED_DATA)
                        continue;
                    numvals++;
                    sum += val;
                    rmsSum += val * val;
                    if (val < minval) {
                        minval = val;
                    }
                    if (val > maxval) {
                        maxval = val;

                    }
                }

                for (MeshIterator mi2(0,0); !mi2.isDone(); ++mi2) {
                    Surf_Data* cursurf = mi2.getMesh()->data;
                    if (cursurf) {
                        max = MAX(maxval, max);
                        min = MIN(minval, min);
                    }
                }
            }
        }


        if (( MatrixOnlyContainZero(data,data->inversevals)==0) && (MatrixOnlyContainZero(data,data->potvals)==1))

        {
            long framenum, leadnum, numvals;

            float val, minval, maxval;
            double sum, rmsSum;

            /*** Loop through all the frames.  ***/

            for (framenum = 0; framenum < data->numframes; framenum++) {
                minval = 1.0e10;
                maxval = -1.e10;

                /*** Loop through all the nodes in the geometry = values of potential. ***/

                sum = 0.0;
                rmsSum = 0.0;
                numvals = 0;//Check for channels later
                for (leadnum = 0; leadnum < data->numleads; leadnum++) {
                    val =data->inversevals[framenum][leadnum];
                    if (val == UNUSED_DATA)
                        continue;
                    numvals++;
                    sum += val;
                    rmsSum += val * val;
                    if (val < minval) {
                        minval = val;
                    }
                    if (val > maxval) {
                        maxval = val;

                    }
                }

                for (MeshIterator mi2(0,0); !mi2.isDone(); ++mi2) {
                    Surf_Data* cursurf = mi2.getMesh()->data;
                    if (cursurf) {
                        max = MAX(maxval, max);
                        min = MIN(minval, min);
                    }
                }
            }
        }
    }
    else {
        min = 0;
        max = 0;
    }
    //ComputeLinearMappingCoefficients(min, max, -.6 * PROJ_SIZE, .5 * PROJ_SIZE, a, b);

    a = ((top - bottom) * height()) / (max - min);
    b = (bottom * height() * max - min * top * height()) / (max - min);


    if (bgcolor[0] + .3 > 1 || bgcolor[1] + .3 > 1 || bgcolor[2] + .3 > 1)
        coloroffset = -.5;

    glColor3f(fgcolor[0], fgcolor[1], fgcolor[2]);

    QString toRender;

    if (showinfotext && !rms) {

        pos[1] = height() - getFontHeight(mesh->gpriv->med_font);

        if (data) {
            // print real frame num if start is not beginning
            int real_frame = data->getRealFrameNum();
            int zero_frame = data->zerotimeframe * data->ts_sample_step + data->ts_start;
            if (data->ts_start != 0 || data->ts_sample_step != 1)
                toRender = QString("Frame: %1 (%2)   Time: %3%4").arg(data->framenum + 1).arg(real_frame + 1)
                        .arg((real_frame-zero_frame) * map3d_info.frames_per_time_unit).arg(map3d_info.time_unit);
            else
                toRender = QString("Frame: %1   Time: %2%3").arg(data->framenum + 1)
                        .arg((real_frame-zero_frame) * map3d_info.frames_per_time_unit).arg(map3d_info.time_unit);


        }
        else {
            toRender = "Frame: ---";
        }
        renderString3f(pos[0], pos[1], pos[2], mesh->gpriv->med_font, toRender);
        toRender = "";

        if (data) {
            toRender = "Pot(G): " + QString::number(data->potvals[data->framenum][pick->node], 'g', 2);
        }
        else {
            toRender = "Pot Value: ---";
        }
        pos[0] = width() - getFontWidth(mesh->gpriv->med_font, toRender) - 2;

        renderString3f(pos[0], pos[1], pos[2], mesh->gpriv->med_font, toRender);
        toRender = "";

        if (data->inversevals[data->framenum][pick->node]!=0){
            toRender = "Inverse(R): " + QString::number(data->inversevals[data->framenum][pick->node], 'g', 2);
        }

        pos[0] = width() - getFontWidth(mesh->gpriv->med_font, toRender) - 2;

        renderString3f(pos[0], pos[1]-12, pos[2], mesh->gpriv->med_font, toRender);
        toRender = "";



        if (data->CCvals[pick->node]!=0){
            toRender = "CC: " + QString::number(data->CCvals[pick->node], 'g', 2);
        }

        pos[0] = width() - getFontWidth(mesh->gpriv->med_font, toRender) - 2;

        renderString3f(pos[0], pos[1]-22, pos[2], mesh->gpriv->med_font, toRender);
        toRender = "";

        if (data->RMSEvals[pick->node]!=0){
            toRender = "RMSE: " + QString::number(data->RMSEvals[pick->node], 'g', 2);
        }

        pos[0] = width() - getFontWidth(mesh->gpriv->med_font, toRender) - 2;

        renderString3f(pos[0], pos[1]-32, pos[2], mesh->gpriv->med_font, toRender);
        toRender = "";


        if (data->forwardvals[data->framenum][pick->node]!=0){
            toRender = "Forward(Y) Value: " + QString::number(data->forwardvals[data->framenum][pick->node], 'g', 2);
        }

        pos[0] = width() - getFontWidth(mesh->gpriv->med_font, toRender) - 2;

        renderString3f(pos[0], pos[1]-24, pos[2], mesh->gpriv->med_font, toRender);
        toRender = "";



        if ((mesh->geom->surfnum>1)&&(!shownearestrecording)&&(pick->nearestDis!=0)){
            toRender = "Closest Dis: " + QString::number(pick->nearestDis, 'g', 2);
        }

        pos[0] = width() - getFontWidth(mesh->gpriv->med_font, toRender) - 2;
        renderString3f(pos[0], 3+12, pos[2], mesh->gpriv->med_font, toRender);




        pos[0] = width()/2 - getFontWidth(mesh->gpriv->med_font, "Time")/2;
        pos[1] = height()/7.5f-18;
        for (int i = 0; i <= mesh->pickstacktop; i++)
        {
            if (pick->node==mesh->pickstack[i]->node){
                toRender = "Pick# " + QString::number(i + 1);

                renderString3f(pos[0], pos[1], pos[2], mesh->gpriv->med_font, toRender);
            }
        }


        pos[0] = 5;
        pos[1] = 3;

        if (mesh->geom->channels)
            //fix channel printing
            //            toRender = "Node# " + QString::number(pick->node + 1) +
            //                    " (Ch " + QString::number(mesh->geom->channels[pick->node] + 1) + ")";
            toRender = "Node# " + QString::number(pick->node + 1);

        else
            toRender = "Node# " + QString::number(pick->node + 1);

        renderString3f(pos[0], pos[1], pos[2], mesh->gpriv->med_font, toRender);
        pos[0] = width() - getFontWidth(mesh->gpriv->med_font, toRender);


        toRender = "Surface# " + QString::number(mesh->geom->surfnum);
        if (mesh->geom->subsurf > 0)
            toRender += "-" + QString::number(mesh->geom->subsurf);

        pos[0] = width() - getFontWidth(mesh->gpriv->med_font, toRender) - 2;
        renderString3f(pos[0], pos[1], pos[2], mesh->gpriv->med_font, toRender);








        /* axis labels */
        pos[0] = width()/2 - getFontWidth(mesh->gpriv->med_font, "Time")/2;
        pos[1] = height()/7.5f;

        renderString3f(pos[0], pos[1], pos[2], mesh->gpriv->med_font, "Time");

        pos[0] = 2;
        pos[1] = b;
        if (data && data->units != 0) {
            renderString3f(pos[0], pos[1], pos[2], mesh->gpriv->med_font, units_strings[data->units - 1]);

        }
        else {
            renderString3f(pos[0], pos[1], pos[2], mesh->gpriv->med_font, "data");

        }


        toRender = QString::number(max, 'g', 2);
        renderString3f(left * width() + d*(float)data->zerotimeframe, top * height()-5, 0, mesh->gpriv->med_font, toRender);

        toRender = QString::number(min, 'g', 2);
        renderString3f(left * width() + d*(float)data->zerotimeframe, bottom * height()-5, 0, mesh->gpriv->med_font, toRender);



    }
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    /* draw vertical axis line, and zero horizontal line */
    glLineWidth(1);
    glColor3f(axiscolor[0], axiscolor[1], axiscolor[2]);
    glBegin(GL_LINES);
    if (data) {
        d = width() / (float)(data->numframes-1) * (right - left); //graph's domain

        glVertex3f(left * width() + d*(float)data->zerotimeframe, top * height(), 0);
        glVertex3f(left * width() + d*(float)data->zerotimeframe, bottom * height(), 0);
        glVertex3f((left - .02f) * width(), b, 0);
        glVertex3f((right + .02f) * width(), b, 0);
        glEnd();
    }
    else{
        glVertex3f(left * width(), top * height(), 0);
        glVertex3f(left * width(), bottom * height(), 0);
        glVertex3f((left - .02f) * width(), b, 0);
        glVertex3f((right + .02f) * width(), b, 0);
        glEnd();
    }
    /* draw time signal */
    if (data) {


        //std::cout<<"picking window crash test if(data)   "<<std::endl;


        // this is a lambda because of all the dumb little variables we need
        auto DrawPlot = [=](int leftFrame, int rightFrame)
        {
            float d = width() / (float)(rightFrame - leftFrame - 1) * (right - left); //graph's domain for each segment
            glEnable(GL_LINE_SMOOTH);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
            glEnable(GL_POLYGON_SMOOTH);

            glLineWidth(graph_width);

            glBegin(GL_LINE_STRIP);// Shu Meng


            // loop is which frame to draw, counter is which position to draw it at
            for (int counter = 0, loop = leftFrame; loop <= rightFrame; loop++, counter++) {
                // draw to the next frame if it exists
                if (loop >= data->numframes)
                    break;
                if (rms) {
                    glColor3f(graphcolor[0], graphcolor[1], graphcolor[0]);
                    glVertex3f(left * width() + d * counter, data->rmspotvals[loop] * a + b, 0);
                }
                else {


                    //Nick
                    if(Map3DsetupGUI == false){
                                if(window1electrogram == true){
                                    glColor3f(graphcolor[0], graphcolor[1], graphcolor[0]);
                                    glVertex3f(left * width() + d * counter, data->potvals[loop][pick->node] * a + b, 0);
                                }else{
                                    ;
                                }
                    }else{
                    glColor3f(graphcolor[0], graphcolor[1], graphcolor[0]);
                    glVertex3f(left * width() + d * counter, data->potvals[loop][pick->node] * a + b, 0);
                    }

                }
            }

            glEnd();

            glBegin(GL_LINE_STRIP);
            // draw inversevals
            for (int counter = 0, loop = leftFrame; loop <= rightFrame; loop++, counter++) {
                // draw to the next frame if it exists
                if (loop >= data->numframes)
                    break;
                if (MatrixOnlyContainZero(data,data->inversevals)==0)
                {

                    //Nick
                    if(Map3DsetupGUI == false){
                                if(window3electrogram == true){
                                    glColor3f(graphcolor[1], graphcolor[0], graphcolor[0]);
                                    glVertex3f(left * width() + d * counter, data->inversevals[loop][pick->node] * a + b, 0);
                                }else{
                                    ;
                                }
                    }else{
                    glColor3f(graphcolor[5], graphcolor[0], graphcolor[0]);
                    glVertex3f(left * width() + d * counter, data->inversevals[loop][pick->node] * a + b, 0);
                    }

                }
            }

            glEnd();

            //Nick-----------------------------------------------------------------------------------
            if(Map3DsetupGUI == false){
                        Surf_Data* data = GetGeomWindow(1)->meshes[1]->data;
                        glBegin(GL_LINE_STRIP);
                        // draw inversevals for second window
                        for (int counter = 0, loop = leftFrame; loop <= rightFrame; loop++, counter++) {
                            // draw to the next frame if it exists
                            if (loop >= data->numframes)
                                break;
                            if (MatrixOnlyContainZero(data,data->inversevals)==0)
                            {
                                //Nick
                               if(window2electrogram == true){
                                                glColor3f(graphcolor[0], graphcolor[0], graphcolor[1]);
                                                glVertex3f(left * width() + d * counter, data->inversevals[loop][pick->node] * a + b, 0);
                                            }else{
                                                ;
                                            }
                             }
                         }
                        glEnd();
            }
            //Nick-----------------------------------------------------------------------------------





            glBegin(GL_LINE_STRIP);
            // draw forwardvals
            for (int counter = 0, loop = leftFrame; loop <= rightFrame; loop++, counter++) {
                // draw to the next frame if it exists
                if (loop >= data->numframes)
                    break;

                if (MatrixOnlyContainZero(data,data->forwardvals)==0)
                {
                    glColor3f(graphcolor[5], graphcolor[5], graphcolor[0]);
                    glVertex3f(left * width() + d * counter, data->forwardvals[loop][pick->node] * a + b, 0);
                    ////std::cout<<loop  <<"forwardvals[loop][pick->node]= "<<data->forwardvals[loop][pick->node]<<std::endl;
                }
            }

            glEnd();

            glBegin(GL_LINE_STRIP);

            for (int counter = 0, loop = leftFrame; loop <= rightFrame; loop++, counter++) {
                // draw to the next frame if it exists
                if (loop >= data->numframes)
                    break;

                if ((mesh->geom->surfnum>1)&&(!shownearestrecording)&&(data->nearestrecordingvals[loop][pick->node]!=0))
                {
                    glColor3f(graphcolor[0], graphcolor[8], graphcolor[3]);
                    glVertex3f(left * width() + d * counter, data->nearestrecordingvals[loop][pick->node] * a + b, 0);
                }

            }
            glEnd();
            glDisable(GL_LINE_SMOOTH);
            glDisable(GL_BLEND);
        };

        if (wintype == TIMEWINDOW && map3d_info.subseries_mode && mesh->data->subseriesToStack.size() > 0)
        {
            // draw each stacked subseries on top of each other
            // TODO - some transparency or something
            // crashes: for (int subseriesNum : mesh->data->subseriesToStack)
            for (int i = 0; i < mesh->data->subseriesToStack.size(); i++)
            {
                int subseriesNum = mesh->data->subseriesToStack[i];
                int leftFrame, rightFrame;
                getFrameRange(subseriesNum, leftFrame, rightFrame);
                d = width() / (float)(rightFrame - leftFrame - 1) * (right - left); //graph's domain for each segment

                DrawPlot(leftFrame, rightFrame);
            }
        }
        else
        {
            int leftFrame, rightFrame;
            // will draw everything if not in subseries node, else the current subseries
            getFrameRange(mesh->data->CurrentSubseries(), leftFrame, rightFrame);
            d = width() / (float)(rightFrame - leftFrame - 1) * (right - left); //graph's domain for each segment
            DrawPlot(leftFrame, rightFrame);

            //draw fiducial markers
            if (wintype == TIMEWINDOW) {
                int index = 0;
                //    printf("mesh->fidConts.size() %d\n",mesh->fidConts.size());
                //    printf("1data->fids[fidsets].numfidleads %d\n", data->fids[fidsets].numfidleads);

                if (pick->node < data->fids.numfidleads) {
                    //      printf("2data->fids.numfidleads %d\n", data->fids[fidsets].numfidleads);
                    for (int i = 0; i < data->fids.leadfids[pick->node].numfids; i++) {
                        short fidType = data->fids.leadfids[pick->node].fidtypes[i];
                        for (unsigned j = 0; j < mesh->fidConts.size(); j++) {
                            if (mesh->fidConts[j]->datatype == fidType)
                                index = j;
                        }
                        glLineWidth(1);
                        glColor3f(mesh->fidConts[index]->fidcolor.redF(),
                                  mesh->fidConts[index]->fidcolor.greenF(),
                                  mesh->fidConts[index]->fidcolor.blueF());
                        glBegin(GL_LINES);
                        glVertex3f(left * width() + d * (data->fids.leadfids[pick->node].fidvals[i] - leftFrame), b + (.1f * height()), 0);
                        glVertex3f(left * width() + d * (data->fids.leadfids[pick->node].fidvals[i] - leftFrame), b - (.1f * height()), 0);
                        glEnd();
                        //            printf("index %d i %d\n", index, i);
                        index++;
                    }
                }
                else {
                    index += data->fids.numfidtypes;
                }

                if (data->subseriesStartFrames.size() > 0)
                {
                    glLineWidth(1);
                    glColor3f(axiscolor[0], axiscolor[1], axiscolor[2]);
                    glBegin(GL_LINES);

                    for (int i = 0; i < data->subseriesStartFrames.size(); i++)
                    {
                        int subseriesStart = data->subseriesStartFrames[i];
                        glVertex3f(left * width() + d * ((float)subseriesStart - leftFrame), top * height(), 0);
                        glVertex3f(left * width() + d * ((float)subseriesStart - leftFrame), bottom * height(), 0);
                    }
                    glEnd();
                }

                // draw vertical frame line
                glLineWidth(1);
                glColor3f(bgcolor[0] + coloroffset, bgcolor[1] + coloroffset, bgcolor[2] + coloroffset);
                glBegin(GL_LINES);
                glVertex3f(left * width() + d * ((float)data->framenum - leftFrame), (top + .02f) * height(), 0);
                glVertex3f(left * width() + d * ((float)data->framenum - leftFrame), (bottom - .02f) * height(), 0);
                glEnd();
                getmatrixslice->indexSpinBox->setValue(cur+1);


            }
            else if (wintype == RMSWINDOW) {
                // vertical frame line
                glLineWidth(1);
                glColor3f(0, 1, 0);
                glBegin(GL_LINES);

                int start = fileWidget->startFrameSpinBox->value();
                int end = fileWidget->endFrameSpinBox->value();
                glVertex3f(left * width() + d * (start - 1), (top + .02f) * height(), 0);
                glVertex3f(left * width() + d * (start - 1), (bottom - .02f) * height(), 0);
                glColor3f(1, 0, 0);
                glVertex3f(left * width() + d * (end - 1), (top + .02f) * height(), 0);
                glVertex3f(left * width() + d * (end - 1), (bottom - .02f) * height(), 0);
                glEnd();
            }
        }

    }
}

// the range [left, right) - right is exclusive - as the condition of a for loop
void PickWindow::getFrameRange(int subseriesNum, int& left, int& right)
{

    //std::cout<<"picking window crash test entering getFrameRange    "<<std::endl;

    if (mesh->data == NULL)
    {
        left = right = 0;
        return;
    }
    if (map3d_info.subseries_mode && subseriesNum < mesh->data->subseriesStartFrames.size())
    {
        left = mesh->data->subseriesStartFrames[subseriesNum];

        if (subseriesNum < mesh->data->subseriesStartFrames.size() - 1)
            right = mesh->data->subseriesStartFrames[subseriesNum + 1];
        else
            right = mesh->data->numframes;
    }
    else
    {
        left = 0;
        right = mesh->data->numframes;
    }
}

void PickWindow::SetStyle(int x)
{
    //std::cout<<"picking window crash test entering SetStyle    "<<std::endl;

    switch (x) {
    case 0:                      //full size
        showinfotext = 0;

        leftoffset = bottomoffset = .025f;
        topoffset = rightoffset = .975f;
        break;

    case 1:                      //details
        showinfotext = 1;

        leftoffset = 0.1f;
        rightoffset = 0.95f;
        topoffset = .83f;
        bottomoffset = .27f;
        break;
    }
}

void PickWindow::SetNearestElec(int x)
{

    //std::cout<<"picking window crash test entering SetNearestElec    "<<std::endl;
    switch (x) {
    case 0:
        shownearestrecording = 0;

        break;

    case 1:
        shownearestrecording = 1;

        break;
    }
}



int PickWindow::OpenMenu(QPoint point)
{
    //std::cout<<"picking window crash test entering OpenMenu   "<<std::endl;


    QMenu menu(this);
    menu.addAction("Axes Color")->setData(axes_color);
    menu.addAction("Graph Color")->setData(graph_color);
    menu.addAction("Graph Width")->setData(graph_width_menu);

    QAction* fullscreenAction = menu.addAction("Toggle Display Mode"); fullscreenAction->setData(full_screen);
    fullscreenAction->setCheckable(true); fullscreenAction->setChecked(showinfotext == 0);

    QAction* subseriesModeAction = menu.addAction("Toggle Subseries Mode"); subseriesModeAction->setData(toggle_subseries_mode);
    subseriesModeAction->setCheckable(true); subseriesModeAction->setChecked(map3d_info.subseries_mode);

    QAction* shownearRcdAction = menu.addAction("Hide nearest recording drawing"); shownearRcdAction->setData(draw_nearest_electrogram);
    shownearRcdAction->setCheckable(true); shownearRcdAction->setChecked(shownearestrecording == 0);

    QAction* action = menu.exec(point);
    if (action)
        return action->data().toInt();
    else
        return -1;

}

void PickWindow::MenuEvent(int menu_data)
{

    //std::cout<<"picking window crash test entering MenuEvent   "<<std::endl;

    switch (menu_data) {
    case axes_color:
        PickColor(axiscolor);
        break;
    case graph_color:
        PickColor(graphcolor);
        break;
    case graph_width_menu:
        PickSize(&graph_width, 10, "Graph Width");
        break;
    case toggle_subseries_mode:
        // Force redraw of all pick windows
        map3d_info.subseries_mode = !map3d_info.subseries_mode;
        Broadcast(MAP3D_UPDATE);
        break;
    case full_screen:
        SetStyle(showinfotext == 1 ? 0 : 1); // pass the opposite of what it currently is
        break;
    case draw_nearest_electrogram:
        SetNearestElec(shownearestrecording ==1 ? 0 : 1);
        break;
    }
    update();
}

void PickWindow::closeEvent(QCloseEvent *event)
{
    Destroy();
}

void PickWindow::switchWindowElectrogram(bool checkState, int window)          //function to switch between showing the electrogram in the pick window or not.
{
    if(window == 1){window1electrogram = checkState;}
    if(window == 2){window2electrogram = checkState;}
    if(window == 3){window3electrogram = checkState;}
}


