#pragma once
#ifndef PROJECT_BEZIER_CONTROL_POINT_H
#define PROJECT_BEZIER_CONTROL_POINT_H
#include "./ProjectMesh.h"

#define CONTROL_POINT_COLOR Eigen::Vector3d(1.0*0xcc/0xff, 0.0, 0.0)
#define PICKING_SCALE 4

class BezierControlPointMesh : public ProjectMesh {
public:
	BezierControlPointMesh(int pointNumber)
		:
		ProjectMesh(0, true, false, false, CONTROL_POINT_COLOR),
		pointNumber{pointNumber}
	{}

	double GetPickingScaleFactor() override { return PICKING_SCALE; }
	int GetPointNumber() { return pointNumber; }

	int pointNumber;
};

#endif
