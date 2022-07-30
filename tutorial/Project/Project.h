#pragma once
#ifndef PROJECT_SCENE_H
#define PROJECT_SCENE_H
#include <utility>
#include "igl/opengl/glfw/Viewer.h"
#include "igl/opengl/Camera.h"
#include "./ProjectMesh.h"
#include "AnimationCameraData.h"
#include "BezierControlPointMesh.h"

class Renderer;

const constexpr int DISPLAY_WIDTH = 1200;
const constexpr int DISPLAY_HEIGHT = 800;
const constexpr float DISPLAY_RATIO = (float)DISPLAY_WIDTH / (float)DISPLAY_HEIGHT;
const constexpr float CAMERA_ANGLE = 45.0f;
const constexpr float NEAR = 1.0f;
const constexpr float FAR = 150.0f;
//const constexpr int infoIndx = 2;
#define BEZIER_CURVE_COLOR Eigen::RowVector3d(1, 1, 1);
#define BEZIER_SELECTED_CURVE_COLOR Eigen::RowVector3d(1, 0, 0);

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

	Project(igl::opengl::CameraData camera);
	//	Project(float angle,float relationWH,float zNear, float zFar);
	void Init();
	void Update(const Eigen::Matrix4f& Proj, const Eigen::Matrix4f& View, const Eigen::Matrix4f& Model, unsigned int  shaderIndx, unsigned int shapeIndx);
	void WhenRotate();
	void WhenTranslate();
	void Animate() override;
	void AddCamera(const Eigen::Vector3d position, const igl::opengl::CameraData cameraData, const CameraKind kind);
	void ChangeCameraIndex_ByDelta(int delta);
	void MoveCamera(std::function<void(Movable&)> transform);
	void InitRenderer();
	void InitBezierSection();
	IGL_INLINE bool EffectiveDesignModeView() const { return isInDesignMode && isDesignModeView; }
	bool AddPickedShapes(const Eigen::Matrix4d& PV, const Eigen::Vector4i& viewport, int sectionIndex, int layerIndex,
		int left, int right, int up, int bottom,
		const std::vector<std::pair<int, int>>& stencilLayers, std::vector<double>& depths) override;
	int AddShapeFromMenu(const std::string& filePath);

	float Picking(const Eigen::Matrix4d& PV, const Eigen::Vector4i& viewportDims, int sectionIndex, int layerIndex, const std::vector<std::pair<int, int>>& stencilLayers, int x, int y) override;
	~Project(void);
	IGL_INLINE int GetPickingShaderIndex() override { return pickingShaderIndex; }
	ProjectMesh* GetProjectMeshByIndex(int index);
	void ToggleSplitMode();
	void WhenRotate(const Eigen::Matrix4d& preMat, float dx, float dy) override;
	void RotateCamera(double dx, double dy) override;

	void ToggleEditBezierMode();
	inline bool IsSplitMode() { return splitMode; };
	inline bool IsEditBezierMode() { return editBezierMode; }
	void InitResources();
	igl::opengl::glfw::ViewerDataCreateFunc GetAnimatedDataCreator(int layer, bool isPickable, bool outline,
		bool allowTransparent, Eigen::Vector3d color = Eigen::Vector3d(1, 0, 0));
	igl::opengl::glfw::ViewerDataCreateFunc GetStaticDataCreator(int layer, bool isPickable, bool outline,
		bool allowTransparent, Eigen::Vector3d color = Eigen::Vector3d(1, 0, 0));
	ProjectMesh* CreateProjectMesh();
	double GetShapeAlpha(int index) override {
		return GetProjectMeshByIndex(index)->GetAlpha();
	};
	void TranslateCamera(double dx, double dy, double dz);
	void WhenScroll(const Eigen::Matrix4d& preMat, float dy) override;
	void WhenTranslate(const Eigen::Matrix4d& preMat, float dx, float dy) override;
	bool ShouldRenderViewerData(const igl::opengl::ViewerData& data, const int sectionIndex, const int layerIndex) const override;
	void Transform(Movable& movable, std::function<void(Movable&)> transform) override;
	void InitScene();

	int GetCurrentBezierMeshIndex() { return currentBezierMeshIndex; }
	int GetCurrentSelectedBezierSegment() { return currentSelectedBezierSegment; }

	void SetCurrentBezierMeshIndex(int index) { currentBezierMeshIndex = index; }
	void SetCurrentSelectedBezierSegment(int segment) { currentSelectedBezierSegment = segment; }
	void DrawBezierCurves();
	bool EnterBezierMode();
	void ExitBezierMode();
	bool CanEnterBezierMode();
	void HideControlPoints();
	void SetControlPointsPosition();
	void TryPickSegment(const Eigen::Matrix4d& PV, const Eigen::Vector4i& viewportDims, int x, int y);
	AnimatedMesh* GetCurrentBezierMesh(){
		return dynamic_cast<AnimatedMesh*>(const_cast<igl::opengl::ViewerData*>
			(data_list[GetCurrentBezierMeshIndex()]));
	}
	inline void AddBezierSegment(){
		GetCurrentBezierMesh()->AddNewSegment();
		DrawBezierCurves();
	}
	inline void RemoveBezierSegment() {
		GetCurrentBezierMesh()->RemoveSegment();
		if (currentSelectedBezierSegment == GetCurrentBezierMesh()->GetSegmentsCount()) {
			currentSelectedBezierSegment--;
		}
		DrawBezierCurves();
	}
private:
	int shaderIndex_cubemap;
	int shaderIndex_basic;
	int shaderIndex_basicColor;
	int materialIndex_grass;
	int materialIndex_box0;
	int pickingShaderIndex;
	int fullScreenSection;
	int leftSection;
	int rightSection;
	int editBezierSection;
	int designCameraIndex;
	int editBezierCameraIndex;
	igl::opengl::CameraData cameraData;
	int controlPointsMeshIndexes[4];
	bool splitMode;
	bool editBezierMode;
	std::unordered_map<int, int> camerasToMesh;
	int currentBezierMeshIndex;
	int currentSelectedBezierSegment;
	int bezierCurvePlane;
	int GetMeshIndex(int cameraIndex);


};
#endif // !PROJECT_SCENE_H