#pragma once
#include "igl/opengl/glfw/Viewer.h"

const constexpr int DISPLAY_WIDTH = 1200;
const constexpr int DISPLAY_HEIGHT = 800;
const constexpr float DISPLAY_RATIO = (float)DISPLAY_WIDTH / (float)DISPLAY_HEIGHT;
const constexpr float CAMERA_ANGLE = 45.0f;
const constexpr float NEAR = 1.0f;
const constexpr float FAR = 150.0f;
const constexpr int infoIndx = 2;

class Project : public igl::opengl::glfw::Viewer
{

public:
	size_t selectedCameraIndex;

	Project();
//	Project(float angle,float relationWH,float zNear, float zFar);
	void Init();
	void Update(const Eigen::Matrix4f& Proj, const Eigen::Matrix4f& View, const Eigen::Matrix4f& Model, unsigned int  shaderIndx, unsigned int shapeIndx);
	void WhenRotate();
	void WhenTranslate();
	void Animate() override;
	void ScaleAllShapes(float amt, int viewportIndx);
	
	~Project(void);
};


