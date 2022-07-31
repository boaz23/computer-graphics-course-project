#pragma once
#ifndef PROJECT_ANIMATED_MESH_H
#define PROJECT_ANIMATED_MESH_H

#include "ProjectMesh.h"
#include "Bezier1d.h"

#define ANIMATION_DIRECTION_AXIS_COLOR Eigen::Vector3d(0, 1, 0)
#define ANIMATION_DIRECTION_COLOR Eigen::Vector3d(1, 0, 0)

class AnimatedMesh: public ProjectMesh {
public:
	AnimatedMesh(int layer, bool isPickable, bool outline,
		bool allowTransparent,
		Eigen::Vector3d color = Eigen::Vector3d(1, 0, 0))
		: ProjectMesh(layer, isPickable, outline, allowTransparent, color),
		bezierCurves{NULL},
		axisLabelsStart{-1}
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
			t = fmod(t, bezierCurves->SegmentsCount());
			int segment = (int)t;
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

	Eigen::Matrix4d GetAnimationDirection() {
		return animationDirection.MakeTransScaled();
	}

	void RotateDirection(const Eigen::Matrix4d& preMat, float dx, float dy)
	{
		Eigen::Matrix4d WithObjectRotation = preMat * MakeTransScaled();
		Eigen::Vector4d xRotation = preMat.transpose() * Eigen::Vector4d(0, 1, 0, 0);
		Eigen::Vector4d yRotation = preMat.transpose() * Eigen::Vector4d(1, 0, 0, 0);
		animationDirection.RotateInSystem(xRotation.head(3), dx);
		animationDirection.RotateInSystem(yRotation.head(3), dy);
	}

	void AfterInit() override {
		Eigen::MatrixXd P;
		P.resize(4, 3);
		P.row(0) = Eigen::Vector3d(0, 0, 0);
		P.row(1) = Eigen::Vector3d(0, 0, -3);
		P.row(2) = Eigen::Vector3d(0, 3, 0);
		P.row(3) = Eigen::Vector3d(3, 0, 0);
		Eigen::MatrixXi E;
		E.resize(3, 2);
		E.row(0) = Eigen::Vector2i(0, 1);
		E.row(1) = Eigen::Vector2i(0, 2);
		E.row(2) = Eigen::Vector2i(0, 3);
		Eigen::MatrixXd C;
		C.resize(3, 3);
		C.row(0) = ANIMATION_DIRECTION_AXIS_COLOR;
		C.row(1) = ANIMATION_DIRECTION_COLOR;
		C.row(2) = ANIMATION_DIRECTION_COLOR;
		set_edges(P, E, C);
		add_label(P.row(1) + Eigen::RowVector3d(0, 0.5, 0), "z");
		add_label(P.row(2) + Eigen::RowVector3d(0, 0.5, 0), "y");
		add_label(P.row(3) + Eigen::RowVector3d(0, 0.5, 0), "x");
		show_custom_labels = unsigned(~0);
		axisLabelsStart = labels_positions.rows() - 3;
		label_color = Eigen::RowVector4f(1, 0, 0, 1);
	}
	int GetAxisLabelsStart() { return axisLabelsStart; }

	double CalcAnimationTime() override {
		// 30 fps -> t += 30*0.01 per second -> 1/0.3 to complete one curve
		return delay + GetSegmentsCount() / 0.3;
	}

	Eigen::Vector3d GetAnchoredTranslation(double t) {
		double withDelay = std::max(0.0, t - delay*0.3);
		Eigen::Vector3d pointAtDelay = GetPoint(withDelay);
		Eigen::Vector3d anchorDiff = Eigen::Vector3d::Zero() - GetPoint(0.0);
		pointAtDelay += anchorDiff;
		return pointAtDelay;
	}

	Eigen::Vector3d GetRotatedAnchredTranslation(double t) {
		Eigen::Vector4d anchoredTranslation = GetAnchoredTranslation(t).homogeneous();
		return (animationDirection.MakeTransScaled() * anchoredTranslation).head(3);
	}

	Eigen::Matrix4d MakeAnimatedTransScaled(double t) override{
		Eigen::Matrix4d toRet = MakeTransScaled();
		toRet.col(3).head(3) += GetRotatedAnchredTranslation(t);
		return toRet;
	}
	Eigen::Matrix4d MakeAnimatedTransd(double t) override {
		Eigen::Matrix4d toRet = MakeTransd();
		toRet.col(3).head(3) += GetRotatedAnchredTranslation(t);
		return toRet;
	};
	Eigen::Matrix4f MakeAnimatedTransScale(double t) override {
		Eigen::Matrix4f toRet = MakeTransScale();
		toRet.col(3).head(3) += GetRotatedAnchredTranslation(t).cast<float>();
		return toRet;
	};
	Eigen::Matrix4f MakeAnimatedTrans(double t) override {
		Eigen::Matrix4f toRet = MakeTransd().cast<float>();
		toRet.col(3).head(3) += GetRotatedAnchredTranslation(t).cast<float>();
		return toRet;
	};
	Eigen::Matrix3d GetAnimatedLinear(double t) override {
		return GetLinear();
	};
	Eigen::Vector3d GetAnimatedPosition(double t) override {
		return GetPosition() + GetRotatedAnchredTranslation(t);
	};


private:
	Bezier1d<double, 3, 2>* bezierCurves;
	Eigen::VectorXi ESMAPCache;
	Movable animationDirection;
	int axisLabelsStart;
};
#endif