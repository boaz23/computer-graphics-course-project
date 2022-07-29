#pragma once
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/Dense>


class Movable
{
public:
	enum{preRot,postRot,phiRot,thetaRot,psiRot,psiPhiRot};

	Movable();
	Movable(const Movable& mov);
	Eigen::Matrix4f MakeTransScale() const;
	Eigen::Matrix4d MakeTransd() const;
	Eigen::Matrix4d MakeTransScaled() const;
	void MyTranslate(Eigen::Vector3d amt, bool preRotation);
	void TranslateInSystem(Eigen::Matrix3d rot, Eigen::Vector3d amt);
	void SetPosition(Eigen::Vector3d newPosition) { Tout.pretranslate(newPosition - Tout.translation()); }
	Eigen::Vector3d GetPosition() const { return Tout.translation(); }
	Eigen::Matrix3d GetRotation() { return Tout.rotation(); }
	void SetCenterOfRotation(Eigen::Vector3d amt);
    void MyRotate(const Eigen::Vector3d& rotAxis, double angle, int mode);
    void MyRotate(Eigen::Vector3d rotAxis, double angle);
    void RotateInSystem(Eigen::Vector3d rotAxis, double angle);
    void MyRotate(const Eigen::Matrix3d &rot);
    void SetRotation(const Eigen::Matrix3d& rot);
    void MyScale(Eigen::Vector3d amt);

	void ZeroTrans();

	Eigen::Matrix3d GetRotation() const{ return Tout.rotation().matrix(); }
    virtual ~Movable() {}
private:

	Eigen::Affine3d Tout,Tin;
};

