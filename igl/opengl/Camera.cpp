//
// Created by hedi1 on 24/01/2022.
//

#include "Camera.h"
#include "../frustum.h"
#include "../ortho.h"
#include "../PI.h"
#include <iostream>




IGL_INLINE igl::opengl::Camera::Camera(CameraData data) : data{data}, length{2.0f}
{
    if(data.fov > 0) //prerspective
    {
        float fH = tan(data.fov / 360.0 * igl::PI) * data.zNear;
        float fW = fH * data.relationWH;
        frustum(-fW, fW, -fH, fH, data.zNear, data.zFar, _projection);
        _ortho = false;
    }
    else  //ortho
    {
        float camera_view_angle = 45.0;
        float h = tan(camera_view_angle/360.0 * igl::PI) * (length);
        ortho(-h* data.relationWH, h* data.relationWH, -h, h, data.zNear, data.zFar,_projection);
        _ortho = true;
    }
}

IGL_INLINE igl::opengl::Camera::Camera():Camera(CameraData(45.0f, 1.0f, 1.0f, 100.0f))
{
}

IGL_INLINE void igl::opengl::Camera::SetProjection(float fov, float relationWH)
{
    if( fov>0)
    {
        float fH = tan(fov / 360.0 * igl::PI) * data.zNear;
        float fW = fH * relationWH;
        frustum(-fW, fW, -fH, fH, data.zNear, data.zFar, _projection);
        data.fov = fov;
        _ortho = false;
    }
    else{
        float camera_view_angle = 45.0;
        float h = tan(camera_view_angle/360.0 * igl::PI) * (length);
        ortho(-h*relationWH, h*relationWH, -h, h, data.zNear, data.zFar, _projection);
        _ortho = true;
    }
    data.relationWH = relationWH;
}

IGL_INLINE Eigen::Matrix4f igl::opengl::Camera::CalcProjection(float relationWH) const
{
    Eigen::Matrix4f toRet;
    if (data.fov > 0)
    {
        float fH = tan(data.fov / 360.0 * igl::PI) * data.zNear;
        float fW = fH * relationWH;
        frustum(-fW, fW, -fH, fH, data.zNear, data.zFar, toRet);
    }
    else {
        float camera_view_angle = 45.0;
        float h = tan(camera_view_angle / 360.0 * igl::PI) * (length);
        ortho(-h * relationWH, h * relationWH, -h, h, data.zNear, data.zFar, toRet);
    }
    return toRet;
}



IGL_INLINE float igl::opengl::Camera::CalcMoveCoeff(float depth,int width) const
{
    float z = data.zFar + depth * (data.zNear - data.zFar);
    if (data.fov > 0)
        return width / data.zFar * z * data.zNear / 2.0f / tanf(data.fov / 360 * igl::PI); // / camera z translate;
    else
        return 0.5f*width;
}

