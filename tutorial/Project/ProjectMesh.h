#pragma once
#ifndef PROJECT_MESH_H
#define PROJECT_MESH_H

#include "igl/opengl/ViewerData.h"

class ProjectMesh: public igl::opengl::ViewerData {
public:
	ProjectMesh(int layer, bool isPickable, bool outline,
		bool allowTransparent,
		Eigen::Vector3d color=Eigen::Vector3d(1, 0, 0))
		: igl::opengl::ViewerData(layer), 
		pickable{isPickable},
		outline{outline},
		color{color},
		allowTransparent{allowTransparent},	
		alpha{1.0},
		delay{0.0}
	{}

	inline bool IsPickable() { return pickable; }
	inline bool DrawOutline() { return outline; }

	inline void SetAlpha(double newAlpha) { alpha = newAlpha; }
	inline void SetColor(Eigen::Vector3d newColor) { color = newColor; }

	inline double GetAlpha() { return allowTransparent ? alpha : 1.0; }
	inline Eigen::Vector3d GetColor() { return color; }
	inline bool IsTransparentAllowed() { return allowTransparent; }

	virtual double GetPickingScaleFactor() { return 1; }
	virtual bool AllowAnimations() { return false; }

	double GetMeshDelay() { return delay; }
	void SetMeshDelay(double newDelay) { delay = newDelay; }

	virtual double CalcAnimationTime() {
		return delay;
	}

	virtual Eigen::Matrix4d MakeAnimatedTransScaled(double t) {
		return MakeTransScaled();
	}
	virtual Eigen::Matrix4d MakeAnimatedTransd(double t) {
		return MakeTransd();
	};
	virtual Eigen::Matrix4f MakeAnimatedTransScale(double t) {
		return MakeTransScale();
	};
	virtual Eigen::Matrix4f MakeAnimatedTrans(double t) {
		return MakeTransd().cast<float>();
	};
	virtual Eigen::Matrix3d GetAnimatedLinear(double t) {
		return GetLinear();
	};
	virtual Eigen::Vector3d GetAnimatedPosition(double t) {
		return GetPosition();
	};

	bool pickable;
	bool outline;
	bool allowTransparent;
	Eigen::Vector3d color;
	double alpha;
	double delay;
};
#endif