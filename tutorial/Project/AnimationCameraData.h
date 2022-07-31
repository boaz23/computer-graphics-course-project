#pragma once
#ifndef PROJECT_ANIMATION_CAMERA_H
#define PROJECT_ANIMATION_CAMERA_H
#include "./AnimatedMesh.h"

#define CAMERA_COLOR Eigen::Vector3d(0.90625, 0.90625, 0.90625)
class AnimationCameraData : public AnimatedMesh {
public:
	AnimationCameraData(int layer, int cameraIndex)
		: 
		AnimatedMesh(layer, true, true, false, CAMERA_COLOR), 
		cameraIndex{cameraIndex} {}

	void AfterInit() override {
		AnimatedMesh::AfterInit();
		add_label(Eigen::RowVector3d::Zero(), "C" + std::to_string(cameraIndex));
	}

	int cameraIndex;
};

#endif
