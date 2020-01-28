#include "RescaleColorMap.h"
#include "ContourDialog.h"
#include "Contour_Info.h"
#include "LegendWindow.h"
#include "MeshList.h"
#include "Surf_Data.h"
#include "Map3d_Geom.h"
#include "WindowManager.h"
#include "eventdata.h"
#include <math.h>

extern Map3d_Info map3d_info;


RescaleColorMap::RescaleColorMap()
{
  setupUi(this);

  connect(this,SIGNAL(maxValueChanged(float)),this,SLOT(setMaxValue(float)));
  connect(this,SIGNAL(minValueChanged(float)),this,SLOT(setMinValue(float)));
}

void RescaleColorMap::on_minSpinBox_valueChanged(double arg1)
{
    emit maxValueChanged((float)maxSpinBox->value());
}

void RescaleColorMap::on_maxSpinBox_valueChanged(double arg1)
{
    emit maxValueChanged((float)maxSpinBox->value());
}

void RescaleColorMap::setMaxValue(float max)
{
    map3d_info.global_user_potmax=max;
}
void RescaleColorMap::setMinValue(float min)
{
    map3d_info.global_user_potmin=min;
}

void RescaleColorMap::on_ApplyScaleButton_clicked()
{

    map3d_info.scale_scope = GLOBAL_GLOBAL_USER_DEFINE;
    Broadcast(MAP3D_UPDATE);
    close();
}

void RescaleColorMap::on_CancleScaleButton_clicked()
{
    close();
    deleteLater();

}
