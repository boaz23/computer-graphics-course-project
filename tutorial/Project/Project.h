#pragma once
#include <utility>
#include "igl/opengl/glfw/Viewer.h"
#include "igl/opengl/Camera.h"

class Renderer;

const constexpr int DISPLAY_WIDTH = 1200;
const constexpr int DISPLAY_HEIGHT = 800;
const constexpr float DISPLAY_RATIO = (float)DISPLAY_WIDTH / (float)DISPLAY_HEIGHT;
const constexpr float CAMERA_ANGLE = 45.0f;
const constexpr float NEAR = 1.0f;
const constexpr float FAR = 150.0f;
//const constexpr int infoIndx = 2;

enum class CameraKind {
	Animation,
	Design,
};

class Project : public igl::opengl::glfw::Viewer
{

public:
	/// <summary>
	/// Whether the scene is currently playing or not
	/// </summary>
	bool isInDesignMode;
	/// <summary>
	/// Whether the view in design mode shows design mode artifacts or only scene objects
	/// </summary>
	bool isDesignModeView;

	std::array<std::pair<int, std::string>, 4> availableMaterials;

	Project();
//	Project(float angle,float relationWH,float zNear, float zFar);
	void Init();
	void Update(const Eigen::Matrix4f& Proj, const Eigen::Matrix4f& View, const Eigen::Matrix4f& Model, unsigned int  shaderIndx, unsigned int shapeIndx);
	void WhenRotate();
	void WhenTranslate();
	void Animate() override;
	void AddCamera(const Eigen::Vector3d position, const igl::opengl::CameraData cameraData, const CameraKind kind);
	void CameraMeshHide(int cameraIndex);
	void CameraMeshUnhide(int cameraIndex, const Movable &transformations);

	IGL_INLINE bool EffectiveDesignModeView() const { return isInDesignMode && isDesignModeView; }

	int AddShapeFromMenu(const std::string& filePath);
	
	float Picking(const Eigen::Matrix4d& PV, const Eigen::Vector4i& viewportDims, int sectionIndex, int layerIndex, const std::vector<std::pair<int, int>> &stencilLayers, int x, int y) override;
	~Project(void);
	inline int GetPickingShaderIndex() override { return pickingShaderIndex; }

protected:
	bool ShouldRenderViewerData(const igl::opengl::ViewerData& data, const int sectionIndex, const int layerIndex) const override;

private:
	int shaderIndex_basic;
	int pickingShaderIndex;
};


