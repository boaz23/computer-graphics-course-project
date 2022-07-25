#pragma once
#ifndef PROJECT_ANIMATION_CAMERA_H
#define PROJECT_ANIMATION_CAMERA_H

#include "igl/opengl/ViewerData.h"

class AnimationCameraData : public igl::opengl::ViewerData {
public:
	AnimationCameraData(int layer, int cameraIndex)
		: igl::opengl::ViewerData(layer), cameraIndex{cameraIndex} {}

	int cameraIndex;
};

#endif
