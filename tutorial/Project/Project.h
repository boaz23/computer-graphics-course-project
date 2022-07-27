#pragma once
#include "igl/opengl/glfw/Viewer.h"
#include "igl/opengl/Camera.h"

class Renderer;

const constexpr int DISPLAY_WIDTH = 1200;
const constexpr int DISPLAY_HEIGHT = 800;
const constexpr float DISPLAY_RATIO = (float)DISPLAY_WIDTH / (float)DISPLAY_HEIGHT;
const constexpr float CAMERA_ANGLE = 45.0f;
const constexpr float NEAR = 1.0f;
const constexpr float FAR = 150.0f;
const constexpr int infoIndx = 2;

enum class CameraKind {
	Animation,
	Design,
};

class Project : public igl::opengl::glfw::Viewer
{

public:
	size_t selectedCameraIndex;

	/// <summary>
	/// Whether the scene is currently playing or not
	/// </summary>
	bool isInDesignMode;
	/// <summary>
	/// Whether the view in design mode shows design mode artifacts or only scene objects
	/// </summary>
	bool isDesignModeView;

	Project(float&);
//	Project(float angle,float relationWH,float zNear, float zFar);
	void Init();
	void Update(const Eigen::Matrix4f& Proj, const Eigen::Matrix4f& View, const Eigen::Matrix4f& Model, unsigned int  shaderIndx, unsigned int shapeIndx);
	void WhenRotate();
	void WhenTranslate();
	void Animate() override;
	void ScaleAllShapes(float amt, int viewportIndx);

	void AddCamera(const Eigen::Vector3d position, const igl::opengl::CameraData cameraData, const CameraKind kind);
	void CameraMeshHide(int cameraIndex);
	void CameraMeshUnhide(int cameraIndex, const Movable &transformations);

	inline bool EffectiveDesignModeView() const { return isInDesignMode && isDesignModeView; }
	
	float Picking(const Eigen::Matrix4d& PV, const Eigen::Vector4i& viewportDims, int viewport, int pickingViewport, int x, int y);
	~Project(void);

protected:
	bool ShouldRenderViewerData(const igl::opengl::ViewerData& data, const int viewportIndx) const override;

private:
	int shaderIndex_basic;
	float& _transperancy_param;
};


