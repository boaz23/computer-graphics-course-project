#pragma once
#ifndef PROJECT_ANIMATED_MESH_H
#define PROJECT_ANIMATED_MESH_H

#include "ProjectMesh.h"
#include "Bezier1d.h"

class AnimatedMesh: public ProjectMesh {
public:
	AnimatedMesh(int layer, bool isPickable, bool outline,
		bool allowTransparent,
		Eigen::Vector3d color = Eigen::Vector3d(1, 0, 0))
		: ProjectMesh(layer, isPickable, outline, allowTransparent, color),
		bezierCurves{NULL}
	{}

	bool AllowAnimations() override { return true; }
	void AddNewSegment(){ 
		if (bezierCurves == NULL) {
			bezierCurves = new Bezier1d_D_3_2();
		}
		else {
			bezierCurves->CreateSegment();
		}
	}
	void RemoveSegment() {
		if (bezierCurves != NULL) {
			bezierCurves->RemoveLastSegment();
			if (bezierCurves->SegmentsCount() == 0) {
				delete bezierCurves;
				bezierCurves = NULL;
			}
		}
	}
	Eigen::Vector3d GetPoint(double t) {
		if (bezierCurves != NULL) {
			int segment = (int)t;
			segment = segment % bezierCurves->SegmentsCount();
			double segmentT = t - segment;
			return bezierCurves->GetPoint(segment, segmentT);
		}
		return Eigen::Vector3d::Zero();
	}
	void GetEdges(Eigen::MatrixX2i& E, Eigen::MatrixX3d& P, Eigen::VectorXi &ESMAP) {
		if (bezierCurves != NULL) {
			bezierCurves->GetEdges(P, E, ESMAP);
			ESMAPCache = Eigen::VectorXi(ESMAP);
		}
	}

	Eigen::Vector3d GetControlPoint(int segment, int index) {
		if (bezierCurves != NULL) {
			return bezierCurves->GetControlPoints(segment).row(index).head(3);
		}
		return Eigen::Vector3d::Zero();
	}

	int GetSegmentsCount() {
		if (bezierCurves != NULL) {
			return bezierCurves->SegmentsCount();
		}
		return 0;
	}

	void MoveControlPoint(int segment, int index, Eigen::Vector3d translation) {
		if (bezierCurves != NULL) {
			bezierCurves->TranslateControlPoint_C1(segment, index, translation);
		}
	}

	Eigen::VectorXi GetCachedMap() {
		return ESMAPCache;
	}
private:
	Bezier1d<double, 3, 2>* bezierCurves;
	Eigen::VectorXi ESMAPCache;
};
#endif