#ifndef ACTIVATIONLEGENDWINDOW_H
#define ACTIVATIONLEGENDWINDOW_H

#include "GenericWindow.h"
class ColorMap;
class Surf_Data;
class Mesh_Info;



class ActivationLegendWindow:public Map3dGLWidget
{

public:
    ActivationLegendWindow(QWidget* parent);

    static ActivationLegendWindow* ActivationLegendWindowCreate(Mesh_Info* mesh, int _width, int _height, int _x, int _y, bool hidden);

    ColorMap **map;
    int orientation;              /* 0 = horizontal, 1 = vertical */
    bool matchContours;           /* 0 = does not match 1 = does match */
    int nticks;                   /* number of tick marks on legend */

    Surf_Data *surf;
    Mesh_Info *mesh;

    // if we start hidden and have -al coordinates set, set to true
    bool specifiedCoordinates;

    void initializeGL();
    void paintGL();

    };

#endif // ACTIVATIONLEGENDWINDOW_H
