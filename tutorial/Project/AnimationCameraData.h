#pragma once
#ifndef PROJECT_ANIMATION_CAMERA_H
#define PROJECT_ANIMATION_CAMERA_H
#include "./ProjectMesh.h"

#define CAMERA_COLOR Eigen::Vector3d(0.9375, 0.9375, 0.9375)
class AnimationCameraData : public ProjectMesh {
public:
	AnimationCameraData(int layer, int cameraIndex)
		: 
		ProjectMesh(layer, true, true, false, CAMERA_COLOR), 
		cameraIndex{cameraIndex} {}

	int cameraIndex;
};

#endif
