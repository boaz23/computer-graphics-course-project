#pragma once
#ifndef PROJECT_SCENE_H
#define PROJECT_SCENE_H
#include <utility>
#include "igl/opengl/glfw/Viewer.h"
#include "igl/opengl/Camera.h"
#include "./ProjectMesh.h"
#include "AnimationCameraData.h"
#include "BezierControlPointMesh.h"
#include "AnimationSegment.h"

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
#define ANIMATION_SEGMENT_DELTA 0.1
#define ANIMATION_SEGMENT_DEFAULT_DURATION 2.0
#define MESH_DELAY_DELTA 0.1
#define ANIMATION_DELTA 0.01

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
	void Animate() override;
	int AddCamera(const Eigen::Vector3d position, const igl::opengl::CameraData cameraData, const CameraKind kind);
	void ChangeCameraIndex_ByDelta(int delta);
	void MoveCamera(std::function<void(Movable&)> transform);
	void ResetActiveCamera();

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

	void TranslateCamera(Eigen::Vector3d d) override;
	void RotateCamera(const std::vector<std::pair<Eigen::Vector3d, double>> &angledAxes) override;
	void WhenTranslate(const Eigen::Matrix4d &preMat, float dx, float dy) override;
	void WhenRotate(const Eigen::Matrix4d &preMat, float dx, float dy) override;
	void WhenScroll(const Eigen::Matrix4d &preMat, float dy) override;
	bool ShouldRenderViewerData(const igl::opengl::ViewerData& data, const int sectionIndex, const int layerIndex, int meshIndex) const override;
	void Transform(Movable& movable, std::function<void(Movable&)> transform) override;
	void InitScene();

	int GetCurrentBezierMeshIndex() { return currentBezierMeshIndex; }
	int GetCurrentSelectedBezierSegment() { return currentSelectedBezierSegment; }
	int GetBezierSectionIndex() { return editBezierSection; }
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

	void DrawShape
	(
		size_t shapeIndex, int shaderIndx,
		const Eigen::Matrix4f& Normal, const Eigen::Matrix4f& Proj, const Eigen::Matrix4f& View,
		int sectionIndex, int layerIndex,
		unsigned int flgs, unsigned int property_id
	) override;
	void ClearPickedShapes(const std::vector<std::pair<int, int>>& stencilLayers) override;

	bool IsAnimationRotationMode() { return animationDirectionMode; }
	void ToggleAnimationRotationMode() { animationDirectionMode = !animationDirectionMode; }

	bool CanEditMeshDelay() {
		if (pShapes.size() == 1) {
			ProjectMesh& mesh = *GetProjectMeshByIndex(pShapes[0]);
			if (mesh.AllowAnimations()) {
				return true;
			}
		}
		return false;
	}
	double GetCurrentMeshDelay() {
		return GetProjectMeshByIndex(selected_data_index)->GetMeshDelay();
	}
	void SetCurrentMeshDelay(double delay) {
		ProjectMesh* mesh = GetProjectMeshByIndex(selected_data_index);
		mesh->SetMeshDelay(std::max(0.0, delay));
	}

	std::map<int, std::string> GetAnimationCamerasNames();
	std::vector<AnimationSegment*> GetAnimationSegments() {
		return segments;
	}
	void SetAnimationSegmentDuration(int animationSegmentIndex, double newDuration) {
		segments[animationSegmentIndex]->SetDuration(std::max(ANIMATION_SEGMENT_DELTA, newDuration));
	}
	void SetAnimationSegmentCamera(int segmentIndex, int cameraIndex) {
		segments[segmentIndex]->SetCameraIndex(cameraIndex);
	}
	void CreateNewAnimationSegment() {
		segments.push_back(new AnimationSegment(defaultAnimationCamera, ANIMATION_SEGMENT_DEFAULT_DURATION));
	}
	void RemoveAnimationSegment(int segmentIndex) {
		AnimationSegment* toDelete = segments[segmentIndex];
		delete toDelete;
		segments.erase(segments.begin() + segmentIndex);
	}
	double GetCurrentAnimationTimeInSeconds() { 
		return currentTime/0.3;
	}
	void SetCurrentAnimationTime(double newTimeInSeconds) {
		currentTime = std::max(0.0, newTimeInSeconds*0.3);
	}
	Eigen::Matrix4d MakeCameraTransScaled(int cameraIndex) override;
	Eigen::Matrix4d MakeCameraTransd(int cameraIndex) override;
	Eigen::Matrix4f MakeCameraTransScale(int cameraIndex) override;
	Eigen::Matrix4f MakeCameraTrans(int cameraIndex) override;
	Eigen::Vector3d GetCameraPosition(int cameraIndex) override;
	Eigen::Matrix3d GetCameraLinear(int cameraIndex) override;
	Eigen::Matrix4d MakeMeshTransScaled(int meshIndex) override;
	Eigen::Matrix4d MakeMeshTransd(int meshIndex) override;
	Eigen::Matrix4f MakeMeshTransScale(int meshIndex) override;
	Eigen::Matrix4f MakeMeshTrans(int meshIndex) override;
	Eigen::Vector3d GetMeshPosition(int meshIndex) override;
	Eigen::Matrix3d GetMeshLinear(int meshIndex) override;
	double CalcAnimationTime();

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
	bool animationDirectionMode;
	int defaultAnimationCamera;
	std::vector<AnimationSegment*> segments;
	double currentTime;
};
#endif // !PROJECT_SCENE_H