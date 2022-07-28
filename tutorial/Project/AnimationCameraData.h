#pragma once
#ifndef PROJECT_ANIMATION_CAMERA_H
#define PROJECT_ANIMATION_CAMERA_H
#include "./ProjectMesh.h"

class AnimationCameraData : public ProjectMesh {
public:
	AnimationCameraData(int layer, int cameraIndex)
		: ProjectMesh(layer, true, true), cameraIndex{cameraIndex} {}

	int cameraIndex;
};

#endif
