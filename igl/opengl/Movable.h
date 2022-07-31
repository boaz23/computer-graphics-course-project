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
	const Eigen::Vector3d GetPosition() const { return Tout.translation(); }
	const Eigen::Matrix3d GetRotation() const { return Tout.rotation(); }
	const Eigen::Matrix3d GetLinear() const { return Tout.linear(); }
	inline const void GetVectorInAxisDirection(Eigen::Vector3d &out, double d, Eigen::Index i) const
	{
		out = d * Tout.linear().col(i).normalized();
	}
	void SetCenterOfRotation(Eigen::Vector3d amt);
    void MyRotate(const Eigen::Vector3d& rotAxis, double angle, int mode);
    void MyRotate(Eigen::Vector3d rotAxis, double angle);
    void RotateInSystem(Eigen::Vector3d rotAxis, double angle);
    void MyRotate(const Eigen::Matrix3d &rot);
    void SetRotation(const Eigen::Matrix3d& rot);
    void MyScale(Eigen::Vector3d amt);

	void ZeroTrans();
    virtual ~Movable() {}

private:
	Eigen::Affine3d Tout,Tin;
};

