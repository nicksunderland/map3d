#include "ActivationLegendWindow.h"
/* LegendWindow.cxx */

#include <stddef.h>
#ifdef _WIN32
#include <windows.h>
#pragma warning(disable:4505)
#undef TRACE
#endif
#include <math.h>
#include "glprintf.h"
#ifdef __APPLE__
#  include <OpenGL/glu.h>
#else
#  include <GL/glu.h>
#endif

#include "Map3d_Geom.h"
#include "Contour_Info.h"
#include "LegendWindow.h"
#include "MeshList.h"
#include "Surf_Data.h"
#include "WindowManager.h"
#include "colormaps.h"
#include "dialogs.h"
#include "eventdata.h"
#include "glprintf.h"
#include "reportstate.h"
#include "scalesubs.h"
#include "MainWindow.h"
#include "GeomWindow.h"

#include <QMenu>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <iostream>
#include "ActivationMapWindow.h"
using namespace std;


extern Map3d_Info map3d_info;
extern MainWindow *masterWindow;

#define FONT_ADJUST 1.75
#define CHAR_WIDTH .07 * FONT_ADJUST
#define CHAR_HEIGHT .07 * FONT_ADJUST

static const int min_width = 100;
static const int min_height = 100;
static const int default_width = 170;
static const int default_height = 256;

extern double  maxactivation, minactivation;

ActivationLegendWindow::ActivationLegendWindow(QWidget *parent): Map3dGLWidget(parent, LEGENDWINDOW, "Colormap Legend",120,135)
{
    // since we can't guarantee the order of initialization, or when the window will be created, we need
    //   to initialize mesh to NULL, and set it later
    mesh = NULL;
    bgcolor[0] = bgcolor[1] = bgcolor[2] = 0;
    fgcolor[0] = fgcolor[1] = fgcolor[2] = 1;
    //showinfotext = 1;
    matchContours = 0;
}


ActivationLegendWindow* ActivationLegendWindow::ActivationLegendWindowCreate(Mesh_Info* mesh, int _width, int _height, int _x, int _y, bool hidden)
{
    ActivationLegendWindow* win = new ActivationLegendWindow(masterWindow ? masterWindow->childrenFrame : NULL);
    win->positionWindow(_width, _height, _x, _y, default_width, default_height);
    win->setVisible(!hidden);
    //win->mesh = mesh;
    return win;
}


void ActivationLegendWindow::initializeGL()
{
    mesh = NULL;
    Map3dGLWidget::initializeGL();
    glLineWidth(3);
    glDisable(GL_DEPTH_TEST);
}



void ActivationLegendWindow::paintGL()
{
    if (mesh == NULL || mesh->cont == NULL)
        return;

    int nticks=9;

    // for the top and bottom
    int actualTicks = nticks + 2;

    int length = (*(map))->max;


    float nextval;

    int loop;

    unsigned char color[3];

    //number of pixels to take off for intermediate lines on graph
    int size_adjuster;
    float delta;
    unsigned char *map = (*(this->map))->map;
    float position[3] = { -1, height() - 20.f, 0 };
    float factor;



    // contvals will be all the lines drawn, whether in matchContours mode or not, including the min and max line
    vector<float> contvals;
    contvals.push_back(minactivation);




    if (matchContours) {
        nticks = mesh->cont->numlevels;
        for (int i = 0; i < mesh->cont->numlevels; i++)
            contvals.push_back(mesh->cont->isolevels[i]);
    }
    else {
        float tick_range = (maxactivation-minactivation)/(nticks+1);
        for (int i = 0; i < nticks; i++)
            contvals.push_back(minactivation + (tick_range*(i+1)));
    }
    contvals.push_back(maxactivation);

    float coloroffset = .5;

    if (bgcolor[0] + .3 > 1 || bgcolor[1] + .3 > 1 || bgcolor[2] + .3 > 1)
        coloroffset = -coloroffset;

    glClearColor(bgcolor[0], bgcolor[1], bgcolor[2], 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int pixelFactor = QApplication::desktop()->devicePixelRatio();
    // this compensates for the "Retina" display ratio.  See http://doc.qt.io/qt-5/highdpi.html
    //  (for some reason the picking doesn't need this)
    glViewport(0, 0, width()*pixelFactor, height()*pixelFactor);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, width(), 0, height());
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3f(fgcolor[0], fgcolor[1], fgcolor[2]);

    // link info to geom window
    if (mesh->actipriv->showinfotext) {

        QString toRender;
        float fontWidth;
        if (mesh->geom->subsurf <= 0)
            toRender = "Surface # " + QString::number(mesh->geom->surfnum);
        else
            toRender = "Surface # " + QString::number(mesh->geom->surfnum) + "-" + QString::number(mesh->geom->subsurf);

        fontWidth = (float)getFontWidth(mesh->actipriv->large_font, toRender);
        if(!orientation)//Horizontal
            position[0] = (width() /3) - fontWidth/2;
        else//vertical
            position[0] = (width() / 2) - fontWidth/2;

        position[1] = (float) (height() - getFontHeight(mesh->actipriv->large_font));
        renderString3f(position[0], position[1], position[2], mesh->actipriv->large_font, toRender);


        // output spatial scaling information (which surfaces)
        if(!orientation)//Horizontal
            position[1] = (float) (height() - getFontHeight(mesh->actipriv->med_font));
        else//vertical
            position[1] -= getFontHeight(mesh->actipriv->med_font) ;

       // toRender = "Activation times";

        fontWidth = (float)getFontWidth(mesh->actipriv->med_font, toRender);
        if(!orientation)//Horizontal
            position[0] = (width() *.8f) - fontWidth/2;
        else//vertical
            position[0] = (width() / 2.0f) - fontWidth/2;

        renderString3f(position[0], position[1], position[2], mesh->actipriv->med_font, toRender);
        position[1] -= getFontHeight(mesh->actipriv->med_font)*.8f ;
    }

    //set up variables for vertical and horizontal
    int bottom, left, size, vsize, hsize;

    vsize = hsize = 0;
    //horiz
    if (!orientation) {
        bottom = 40;
        //vsize = ((height() > 150) ? (int)(height() * .75 - 30) : (int)(height() - 80));
        left = 20;
        size = width() - 40;
        if (mesh->actipriv->showinfotext)
            vsize = height() - (getFontHeight(mesh->actipriv->large_font) +
                                getFontHeight(mesh->actipriv->med_font)+10);
        else
            vsize = height() - 20;
        //     size = height - (getFontHeight(mesh->actipriv->large_font) +
        //  			   3*getFontHeight(mesh->actipriv->med_font)+20);
    }
    //vertical
    else {
        if (mesh->actipriv->showinfotext)
            size = height() - (getFontHeight(mesh->actipriv->large_font) +
                               3*getFontHeight(mesh->actipriv->med_font)+20);
        else
            size = height() - 40;
        hsize = width() - getFontWidth(mesh->actipriv->small_font, "-XXX.XX")-20;
        if (hsize < 40) hsize = 40; // make at least 20 pizels for the color bar
        bottom = 20;
        left = 20;
    }

    //set up window vars
    if (size / contvals.size() > 4)
        size_adjuster = 0;
    else if (size / contvals.size() > 3)
        size_adjuster = 1;
    else
        size_adjuster = 2;
    delta = ((float)size) / length;
    factor = (float)(size / (actualTicks - 1));

    //draw surface color if mesh is displayed as solid color
    if (mesh->drawmesh == RENDER_MESH_ELTS || mesh->drawmesh == RENDER_MESH_ELTS_CONN) {
        if (mesh->actipriv->secondarysurf == mesh->geom->surfnum - 1)
            glColor3f(mesh->secondarycolor[0], mesh->secondarycolor[1], mesh->secondarycolor[2]);
        else
            glColor3f(mesh->meshcolor[0], mesh->meshcolor[1], mesh->meshcolor[2]);
        //vertical
        if (orientation) {
            glBegin(GL_QUADS);
            glVertex2d(left, bottom);
            glVertex2d(left, size + bottom);
            glVertex2d(hsize, size + bottom);
            glVertex2d(hsize, bottom);
            glEnd();
        }
        //horizontal
        else {
            glBegin(GL_QUADS);
            glVertex2d(left, vsize);
            glVertex2d(left, bottom);
            glVertex2d(left + size, bottom);
            glVertex2d(left + size, vsize);
            glEnd();
        }
    }
    //gouraud shading (or band-shading without matching contours - it wouldn't make sense to
    // draw bands that don't correspond to the display)
    else if (mesh->shadingmodel == SHADE_GOURAUD ||
             mesh->shadingmodel == SHADE_FLAT ||
             (mesh->shadingmodel == SHADE_BANDED && matchContours)) {

        glBegin(GL_QUAD_STRIP);

        for (loop = 0; loop < length; loop++) {
            getContColor(minactivation + loop * (maxactivation - minactivation) / length, minactivation, maxactivation, *(this->map), color, mesh->invert);
            //glColor3ub(map[loop*3],map[loop*3+1],map[loop*3+2]);
            glColor3ubv(color);
            if (!orientation) {
                glVertex2d(left + loop * delta, vsize);
                glVertex2d(left + loop * delta, bottom);
            }
            else {
                glVertex2d(left, bottom + loop * delta);
                glVertex2d(hsize, bottom + loop * delta);
            }
        }
        glEnd();
    }


    /* draw legend outline */
    glColor3f(bgcolor[0] + coloroffset, bgcolor[1] + coloroffset, bgcolor[2] + coloroffset);
    glBegin(GL_LINES);

    //horiz
    if (!orientation) {
        glVertex2d(left, vsize);
        glVertex2d(left + size, vsize);
        glVertex2d(left, bottom);
        glVertex2d(left + size, bottom);
    }
    //vert
    else {
        glVertex2d(left, bottom);
        glVertex2d(left, bottom + size);
        glVertex2d(hsize, bottom + size);
        glVertex2d(hsize, bottom);
    }

    glEnd();

    glLineWidth((float)(3 - size_adjuster));
    glBegin(GL_LINES);


    //draw contour lines in legend window
    for (loop = 0; loop < (int) contvals.size(); loop++) {
        nextval = contvals[loop];

       // std::cout<<"nextval in activation legend window  "<<nextval<<std::endl;


        if (mesh->shadingmodel == SHADE_NONE) {
            if (loop == 0 || loop == contvals.size()-1){
                glColor3f(bgcolor[0] + coloroffset, bgcolor[1] + coloroffset, bgcolor[2] + coloroffset);
            }
            else {
                getContColor(nextval, minactivation, maxactivation, *(this->map), color, mesh->invert);
                glColor3ubv(color);
            }
        }
        //horiz
        if (!orientation) {
            glVertex2d(left + (nextval - minactivation) / (maxactivation - minactivation) * size, vsize);  // -.85 to .4
            glVertex2d(left + (nextval - minactivation) / (maxactivation - minactivation) * size, bottom - 5);
        }
        //vert
        else {
            glVertex2d(left, bottom + (nextval - minactivation) / (maxactivation - minactivation) * size); // -.85 to .4
            int extension = (loop == 0 || loop == contvals.size()-1) ? 5 : 0;
            glVertex2d(hsize + extension, bottom + (nextval - minactivation) / (maxactivation - minactivation) * size);
        }

    }
    glEnd();
    //glLineWidth(3);

    if (orientation) {
        position[0] = (float)hsize + 10;
        position[1] = 0;
        position[2] = 0;
    }
    else {
        position[0] = (float)left;
        position[1] = (float)bottom - 15;
        position[2] = 0;
    }

    //write contour values
    // vars used in inner loop
    char string[256] = { '\0' };
    int prevcont = bottom;
    int lastcont = bottom + size;
    int stagger = 0;            //which row you're on
    int rowpos[2] = { -15, -100 };  //
    int endpos = (int)(left - getFontWidth(mesh->actipriv->small_font, "XXX") + size);
    int font_height = getFontHeight(mesh->actipriv->small_font);

    for (loop = 0; loop < (unsigned) contvals.size(); loop++) {
        glColor3f(fgcolor[0], fgcolor[1], fgcolor[2]);
        nextval = contvals[loop];

        if (nextval > -.1 && nextval < .1) // 3-decimal precision
            sprintf(string, "%.3f", nextval);
        else if (nextval >= .1 && nextval < 10) //2 decimal precision
            sprintf(string, "%.2f", nextval);
        else                      //1 decimal precision
            sprintf(string, "%.1f", nextval);
        //horizontal
        if (!orientation) {
            position[0] = left - getFontWidth(mesh->actipriv->small_font, "XXX") + ((nextval - minactivation) / (maxactivation - minactivation) * size);
            //glprintf(position,normal,up,mod_width*aspect,mod_height,"%.2f\0",nextval);
            if (loop == 0 || loop == contvals.size()-1 || (rowpos[stagger] + getFontWidth(mesh->actipriv->small_font, "XXXXXX") < position[0] &&
                                                           (position[0] + getFontWidth(mesh->actipriv->small_font, "XXXXXX") < endpos ||
                                                            rowpos[(stagger + 1) % 2] + getFontWidth(mesh->actipriv->small_font, "XXXXXX") < endpos))) {
                glColor3f(fgcolor[0], fgcolor[1], fgcolor[2]);
                renderString3f(position[0], position[1] - 12 * stagger, position[2], mesh->actipriv->small_font, string);
                //Extend the contour line if number is staggered(on the bottom)
                if(stagger == 1){
                    glColor3f(bgcolor[0] + coloroffset, bgcolor[1] + coloroffset, bgcolor[2] + coloroffset);
                    if (mesh->shadingmodel == SHADE_NONE && loop != 0 && loop != contvals.size()-1) {
                        getContColor(nextval, minactivation, maxactivation, *(this->map), color, mesh->invert);
                        glColor3ubv(color);
                    }
                    glBegin(GL_LINES);
                    glVertex2d(left + (nextval - minactivation) / (maxactivation - minactivation) * size, bottom - 5);
                    glVertex2d(left + (nextval - minactivation) / (maxactivation - minactivation) * size, position[1] - 2 * stagger);
                    glEnd();
                    glLineWidth(3);
                }
                //end draw contour line
                rowpos[stagger] = (int)position[0];
                stagger = (stagger + 1) % 2;
            }
        }
        //vertical
        else {
            //int numconts = actualTicks;
            position[1] = bottom - font_height/4 + ((nextval - minactivation) / (maxactivation - minactivation) * size);
            //determines which contour val to write
            if (loop == 0 || loop == contvals.size()-1 ||
                    position[1] >= prevcont + font_height && position[1] <= lastcont - font_height) {
                renderString3f(position[0], position[1], position[2], mesh->actipriv->small_font, string);
                prevcont = (int)position[1];
                if (loop != 0 && loop != contvals.size()-1) {
                    // extend the contour line, so we can see which value it points to
                    glBegin(GL_LINES);
                    if (mesh->shadingmodel != SHADE_NONE) {
                        glColor3f(bgcolor[0] + coloroffset, bgcolor[1] + coloroffset, bgcolor[2] + coloroffset);
                    }
                    else {
                        getContColor(nextval, minactivation, maxactivation, *(this->map), color, mesh->invert);
                        glColor3ubv(color);
                    }

                    glVertex2d(left, bottom + (nextval - minactivation) / (maxactivation - minactivation) * size); // -.85 to .4
                    glVertex2d(hsize + 5, bottom + (nextval - minactivation) / (maxactivation - minactivation) * size);
                    glEnd();
                }
            }
        }
    }

    glColor3f(1, 1, 1);

#if SHOW_OPENGL_ERRORS
    GLenum e = glGetError();
    if (e)
        printf("LegendWindow OpenGL Error: %s\n", gluErrorString(e));
#endif
}


