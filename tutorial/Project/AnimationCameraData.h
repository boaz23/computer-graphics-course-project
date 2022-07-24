#pragma once
#ifndef PROJECT_ANIMATION_CAMERA_H
#define PROJECT_ANIMATION_CAMERA_H

#include "igl/opengl/ViewerData.h"

class AnimationCameraData : public igl::opengl::ViewerData {
public:
	AnimationCameraData(int cameraIndex) : cameraIndex{cameraIndex} {}

	int cameraIndex;
};

#endif
