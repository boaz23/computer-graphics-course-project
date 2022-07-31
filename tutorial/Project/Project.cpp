#include "Project.h"
#include "igl/opengl/glfw/renderer.h"
#include "igl/opengl/ViewerData.h"
#include "igl/opengl/util.h"
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <limits>


static void printMat(const Eigen::Matrix4d& mat)
{
	std::cout << " matrix:" << std::endl;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			std::cout << mat(j, i) << " ";
		std::cout << std::endl;
	}
}

Project::Project(igl::opengl::CameraData camera) :	
	controlPointsMeshIndexes{-1, -1, -1, -1},
	editBezierMode{false},
	splitMode{false},
	materialIndex_box0{-1},
	materialIndex_grass{-1},
	shaderIndex_cubemap{-1},
	isInDesignMode{true}, 
	isDesignModeView{true}, 
	shaderIndex_basic{-1},
	pickingShaderIndex(-1),
	camerasToMesh{},
	cameraData{camera},
	fullScreenSection{-1},
	leftSection{-1},
	rightSection{-1},
	editBezierSection{-1},
	designCameraIndex{-1},
	editBezierCameraIndex{-1},
	shaderIndex_basicColor{-1},
	currentBezierMeshIndex{-1},
	currentSelectedBezierSegment{-1},
	bezierCurvePlane{-1}
{
	data_list.front() = new ProjectMesh(0, false, false, false);
	data_list.front()->id = 0;
	// Per face
	data()->set_face_based(false);
}


void Project::InitRenderer() {
	fullScreenSection = renderer->AddSection(
		0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT,
		0, true, true, true);
	leftSection = renderer->AddSection(
		0, 0, DISPLAY_WIDTH/2, DISPLAY_HEIGHT,
		0, true, true, true);
	rightSection = renderer->AddSection(
		DISPLAY_WIDTH/2, 0, DISPLAY_WIDTH/2, DISPLAY_HEIGHT,
		0, true, true, false);
	editBezierSection = renderer->AddSection(
		DISPLAY_WIDTH / 2, 0, DISPLAY_WIDTH / 2, DISPLAY_HEIGHT,
		0, false, false, false, false, false);
	WindowSection& bezierSection = renderer->GetSection(editBezierSection);
	bezierSection.ClearDrawFlag(bezierSection.GetSceneLayerIndex(), 0, depthTest | blend);
	designCameraIndex = renderer->AddCamera(Eigen::Vector3d(0.0, 0.0, 10.0), cameraData);
	// bezier edit camera
	// TODO aspect ratio not realy matter
	igl::opengl::CameraData editBezierCameraData(45, 0.75, NEAR, FAR);
	editBezierCameraIndex = renderer->AddCamera(Eigen::Vector3d(0.0, 0.0, 10.0), editBezierCameraData);
	// Default full screen
	renderer->ActivateSection(fullScreenSection);
	renderer->GetSection(editBezierSection).SetCamera(editBezierCameraIndex);
}

void Project::InitBezierSection() {
	std::pair<int, int> bezierSectionSceneLayerKey = { editBezierSection, renderer->GetSection(editBezierSection).GetSceneLayerIndex() };
	int bezierPlane = AddShape(Plane, -2, TRIANGLES, shaderIndex_basicColor,
		{ bezierSectionSceneLayerKey }, GetStaticDataCreator(0, false, false, false,
			Eigen::Vector3d(1.0*0x18/0xff, 1.0*0x17/0xff, 1.0*0x18/0xff)));
	selected_data_index = bezierPlane;
	ShapeTransformation(zTranslate, -1.1f, 1);
	SetShapeStatic(bezierPlane);
	int bezierAxis = AddShape(Cube, -1, TRIANGLES, shaderIndex_basicColor,
		{ bezierSectionSceneLayerKey }, GetStaticDataCreator(0, false, false, false,
			Eigen::Vector3d(1.0 * 0x18 / 0xff, 1.0 * 0x17 / 0xff, 1.0 * 0x18 / 0xff)));
	data()->show_faces = 0;
	data()->show_lines = 0;
	data()->show_overlay = unsigned(~0);
	data()->add_edges((Eigen::RowVector3d::UnitX() * 60), -(Eigen::RowVector3d::UnitX() * 60), Eigen::RowVector3d(0, 0, 1));
	data()->add_edges((Eigen::RowVector3d::UnitY() * 60), -(Eigen::RowVector3d::UnitY() * 60), Eigen::RowVector3d(0, 1, 0));
	bezierCurvePlane = AddShape(Cube, -1, TRIANGLES, shaderIndex_basicColor,
		{ bezierSectionSceneLayerKey }, GetStaticDataCreator(0, false, false, false,
			Eigen::Vector3d(1.0 * 0x18 / 0xff, 1.0 * 0x17 / 0xff, 1.0 * 0x18 / 0xff)));
	data()->show_faces = 0;
	data()->show_lines = 0;
	data()->show_overlay = unsigned(~0);
	for (int i = 0; i < 4; i++) {
		controlPointsMeshIndexes[i] = AddShapeFromFile(
			"./data/cube.obj",
			-1, TRIANGLES, shaderIndex_basicColor,
			{ bezierSectionSceneLayerKey },
			[i]() {
				return new BezierControlPointMesh(i);
			}
		);
		ShapeTransformation(xTranslate, -1.5f + (float)i, 1);
		ShapeTransformation(scaleAll, 0.05f, 1);
		data()->show_custom_labels = unsigned(~0);
		data()->add_label(Eigen::RowVector3d(0, 2, 0.5), "P" + std::to_string(i));
		data()->label_color = Eigen::RowVector4f(1, 0, 0, 1);
	}
}

void Project::InitResources() {
	pickingShaderIndex = AddShader("shaders/pickingShader");
	shaderIndex_cubemap = AddShader("shaders/cubemapShader");
	shaderIndex_basic = AddShader("shaders/basicShader");
	shaderIndex_basicColor = AddShader("shaders/basicShaderColor");

	unsigned int textureIndex_plane = AddTexture("textures/plane.png", 2);
	unsigned int textureIndex_cubeMap_daylightBox = AddTexture("textures/cubemaps/Daylight Box UV.png", 3);
	unsigned int textureIndex_grass = AddTexture("textures/grass.bmp", 2);
	unsigned int textureIndex_box0 = AddTexture("textures/box0.bmp", 2);
	unsigned int textureIndex_bricks = AddTexture("textures/bricks.jpg", 2);
	unsigned int texIDs[] =
	{
		textureIndex_box0,
		textureIndex_cubeMap_daylightBox,
		textureIndex_grass,
		textureIndex_plane,
		textureIndex_bricks,
	};
	unsigned int slots[] = { 0, 1, 2, 3, 4 };

	int nullMaterial = AddMaterial(nullptr, nullptr, 0); // Avoid having a "default" material
	materialIndex_box0 = AddMaterial(texIDs + 0, slots + 0, 1);
	materialIndex_cube = AddMaterial(texIDs + 1, slots + 1, 1);
	materialIndex_grass = AddMaterial(texIDs + 2, slots + 2, 1);
	int materialIndex_plane = AddMaterial(texIDs + 3, slots + 3, 1);
	int materialIndex_bricks = AddMaterial(texIDs + 4, slots + 4, 1);
	availableMaterials =
	{
		std::pair<int, std::string>{materialIndex_box0, "Box"},
		std::pair<int, std::string>{materialIndex_grass, "Grass"},
		std::pair<int, std::string>{materialIndex_plane, "Plane"},
		std::pair<int, std::string>{materialIndex_bricks, "Bricks"}
	};

}

void Project::InitScene() {
	std::vector<std::pair<int, int>>& sceneLayers = renderer->GetSceneLayersIndexes();
	int sceneCube = AddShape(Cube, -2, TRIANGLES, shaderIndex_cubemap, sceneLayers, 
		GetStaticDataCreator(0, false, false, false));
	int scissorBox = AddShape(Plane, -2, TRIANGLES, pickingShaderIndex, renderer->GetScissorsTestLayersIndexes(), 
		GetStaticDataCreator(0, false, false, false));
	int cube1 = AddShape(Cube, -1, TRIANGLES, shaderIndex_basic, sceneLayers, 
		GetAnimatedDataCreator(currentEditingLayer, true, true, true));
	int cube2 = AddShape(Cube, -1, TRIANGLES, shaderIndex_basic, sceneLayers, 
		GetAnimatedDataCreator(currentEditingLayer, true, true, true));
	//int camera = AddShapeFromFile("./data/film_camera 2.obj", -1, TRIANGLES, shaderIndex_basic, sceneLayers);

	SetShapeMaterial(sceneCube, materialIndex_cube);
	//SetShapeMaterial(scissorBox, materialIndex_box0);
	SetShapeMaterial(cube1, materialIndex_grass);
	SetShapeMaterial(cube2, materialIndex_box0);
	//SetShapeMaterial(camera, materialIndex_box0);

	selected_data_index = sceneCube;
	float s = 60;
	ShapeTransformation(scaleAll, s, 0);
	SetShapeStatic(sceneCube);
	selected_data_index = scissorBox;
	ShapeTransformation(zTranslate, -1.1f, 1);
	SetShapeStatic(scissorBox);
	selected_data_index = cube2;
	ShapeTransformation(xTranslate, 2.f, 1);
	//selected_data_index = camera;
	//s = 1.f / 30.f;
	//ShapeTransformation(scaleAll, s, 0);
	//ShapeTransformation(yTranslate, 1.f, 0);
}
void Project::Init()
{		
	Viewer::AddLayer();
	Viewer::AddLayer();
	InitResources();
	InitRenderer();
	InitScene();
	InitBezierSection();
	// default animation camera
	AddCamera(Eigen::Vector3d(0.0, 0.0, 0.0), cameraData, CameraKind::Animation);
}

void Project::Update(const Eigen::Matrix4f& Proj, const Eigen::Matrix4f& View, const Eigen::Matrix4f& Model, unsigned int  shaderIndx, unsigned int shapeIndx)
{
	Shader* s = shaders[shaderIndx];
	int r = ((shapeIndx + 1) & 0x000000FF) >> 0;
	int g = ((shapeIndx + 1) & 0x0000FF00) >> 8;
	int b = ((shapeIndx + 1) & 0x00FF0000) >> 16;
	s->Bind();
	s->SetUniformMat4f("Proj", Proj);
	s->SetUniformMat4f("View", View);
	s->SetUniformMat4f("Model", Model);
	ProjectMesh& mesh = *GetProjectMeshByIndex(shapeIndx);

	if (mesh.GetMaterial() >= 0 && !materials.empty())
	{
		BindMaterial(s, mesh.GetMaterial());
	}
	if (shaderIndx == pickingShaderIndex)
	{
		s->SetUniform4f("lightColor", 0xc6 / 255.0f, 0x1e / 255.0f, 0x24 / 255.0f, 0.5f);
	}
	else if (shaderIndx == shaderIndex_basic)
	{
		s->SetUniform1f("Transperancy", (float)mesh.GetAlpha());
	}
	else if (shaderIndx == shaderIndex_basicColor)
	{
		s->SetUniform1f("Transperancy", (float)mesh.GetAlpha());
		s->SetUniform3f("myColor", mesh.GetColor().cast<float>());
	}
	s->Unbind();
}

void Project::Animate()
{
	if (isActive)
	{
		if (selected_data_index > 0)
			data()->MyRotate(Eigen::Vector3d(0, 1, 0), 0.01);
	}
}

bool Project::ShouldRenderViewerData(const igl::opengl::ViewerData& data, const int sectionIndex, const int layerIndex) const
{
	auto pAnimationCameraData = dynamic_cast<AnimationCameraData*>(const_cast<igl::opengl::ViewerData*>(&data));
	return (pAnimationCameraData == nullptr || EffectiveDesignModeView()) &&
		Viewer::ShouldRenderViewerData(data, sectionIndex, layerIndex);
}

int Project::AddShapeFromMenu(const std::string& filePath)
{
	if (filePath.length() == 0)
	{
		return -1;
	}

	int shapeIndex = AddShapeFromFile(filePath, -1, TRIANGLES, shaderIndex_basic, renderer->GetSceneLayersIndexes(),
	GetAnimatedDataCreator(currentEditingLayer, true, true, true));
	GetViewerDataAt(shapeIndex).SetMaterial(materialIndex_box0);
	return shapeIndex;
}

void Project::AddCamera(const Eigen::Vector3d position, const igl::opengl::CameraData cameraData, const CameraKind kind)
{
	int cameraIndex = renderer->AddCamera(position, cameraData);
	switch (kind)
	{
	case CameraKind::Design:
		throw std::exception("Not implemented");
		break;
	case CameraKind::Animation:
		int shapeIndex = AddShapeFromFile
		(
			"./data/film_camera.obj",
			-1,
			TRIANGLES,
			shaderIndex_basicColor,
			renderer->GetSceneLayersIndexes(),
			[this, &cameraIndex]()
			{
				return new AnimationCameraData(currentEditingLayer, cameraIndex);
			}
		);
		camerasToMesh.insert(std::pair<int, int>{cameraIndex, shapeIndex});
		const constexpr double camera_size = 1.0 / 32.0;
		igl::opengl::ViewerData *shape = data_list[shapeIndex];
		shape->MyScale(Eigen::Vector3d(camera_size, camera_size, camera_size));
		shape->MyRotate(Eigen::Vector3d(0, 1, 0), EIGEN_PI / 2);
		break;
	}
}

int Project::GetMeshIndex(int cameraIndex)
{
	auto meshIt = camerasToMesh.find(cameraIndex);
	int mesh{ -1 };
	if (meshIt != camerasToMesh.end())
	{
		mesh = meshIt->second;
	}
	return mesh;
}

void Project::ChangeCameraIndex_ByDelta(int delta)
{
	int currentSectionIndex = renderer->GetCurrentSectionIndex();
	if (currentSectionIndex == editBezierSection) {
		return;
	}
	WindowSection &section = renderer->GetSection(currentSectionIndex);
	int currentCameraIndex = section.GetCamera();
	int currentMesh = GetMeshIndex(currentCameraIndex);
	int tempCameraIndex = (currentCameraIndex+delta)%(renderer->CamerasCount());
	int nextCameraIndex = -1;
	while (nextCameraIndex == -1) {
		if (tempCameraIndex == 0 || camerasToMesh.find(tempCameraIndex) != camerasToMesh.end()) {
			nextCameraIndex = tempCameraIndex;
		}
		tempCameraIndex = (tempCameraIndex + 1) % renderer->CamerasCount();
	}
	int nextMesh = GetMeshIndex(nextCameraIndex);
	const auto &sceneKey = std::pair<int, int>{ currentSectionIndex, section.GetSceneLayerIndex() };
	const auto& stencilKey = std::pair<int, int>{ currentSectionIndex, section.GetStencilTestLayerIndex() };
	if (currentMesh >= 0)
	{
		data_list[currentMesh]->AddSectionLayers({ sceneKey });
	}
	if (nextMesh >= 0)
	{
		data_list[nextMesh]->RemoveSectionLayers({ sceneKey, stencilKey });
	}
	section.SetCamera(nextCameraIndex);
}

void Project::MoveCamera(std::function<void(Movable &)> transform)
{
	WindowSection &section = renderer->GetCurrentSection();
	int cameraIndex = section.GetCamera();
	auto meshIt = camerasToMesh.find(cameraIndex);
	igl::opengl::Camera &camera = renderer->GetCamera(cameraIndex);
	igl::opengl::ViewerData *shape = meshIt == camerasToMesh.end() ? nullptr : data_list[meshIt->second];
	if (shape != nullptr)
	{
		transform(*shape);
	}
	transform(camera);
}

void Project::Transform(Movable &movable, std::function<void(Movable &)> transform)
{
	Viewer::Transform(movable, transform);
	auto *cameraMesh = dynamic_cast<AnimationCameraData *>(&movable);
	if (cameraMesh != nullptr)
	{
		Movable &camera = renderer->GetCamera(cameraMesh->cameraIndex);
		Viewer::Transform(camera, transform);
	}
}

Project::~Project(void)
{
}

void ProjectScreenCoordToScene(double x, double y, float m_viewport[], const Eigen::Matrix4d& sceneViewInverse, Eigen::Vector3d& sceneSourceCoord, Eigen::Vector3d& sceneDir) {
	Eigen::Vector4d sceneCoord, destCoord;
	sceneCoord <<
		2 * ((x - m_viewport[0]) / m_viewport[2]) - 1.0,
		2 * ((y + m_viewport[1]) / m_viewport[3]) - 1.0,
		0, 1
		;
	destCoord <<
		2 * ((x - m_viewport[0]) / m_viewport[2]) - 1.0,
		2 * ((y + m_viewport[1]) / m_viewport[3]) - 1.0,
		1, 1
		;
	sceneCoord = sceneViewInverse * sceneCoord;
	destCoord = sceneViewInverse * destCoord;
	sceneCoord = sceneCoord / sceneCoord(3);
	destCoord = destCoord / destCoord(3);
	sceneSourceCoord = sceneCoord.head(3);
	sceneDir = (destCoord - sceneCoord).normalized().head(3);
}

bool TriangleIntersection(const Eigen::Vector3d& source, const Eigen::Vector3d& dir, const Eigen::Matrix3d& vertices, const Eigen::Vector3d& normal, Eigen::Vector3d& intersectionPoint) {
	Eigen::Vector3d vv1 = vertices.row(0).transpose() - source;
	double dist = (normal.dot(vv1)) / normal.dot(dir);
	if (dist > FLT_EPSILON && dist < INFINITY) {
		Eigen::Vector3d P = source + dir * dist;
		for (int i = 0; i < 3; i++) {
			Eigen::Vector3d v1 = vertices.row(i);
			Eigen::Vector3d v2 = vertices.row((i + 1) % 3);
			Eigen::Vector3d e1 = v2 - v1;
			Eigen::Vector3d e2 = P - v1;
			Eigen::Vector3d N = e1.cross(e2).normalized();
			if (normal.dot(N) < 0) {
				return false;
			}
		}
		intersectionPoint = P;
		return true;
	}
	return false;
}

bool GetClosestIntersectingFace(igl::opengl::ViewerData& mesh, const Eigen::Matrix4d& meshView, const Eigen::Vector3d& sourcePointScene, const Eigen::Vector3d& dirToScene, Eigen::Vector3d& closestIntersectionToRet, int& closestFaceIndexToRet) {
	int closestShapeFaceIndex = -1;
	double closestShapeFaceDist = -1;
	Eigen::Vector3d closestIntersectionPoint = Eigen::Vector3d::Ones();
	for (int i = 0; i < mesh.F.rows(); i++) {
		Eigen::Matrix3d vertices;
		// TODO this will truncate every other vertex of the face,
		// taking only a triangle of the first 3 vertices
		Eigen::Vector3i vIndexes = mesh.F.row(i).head(3);
		vertices.row(0) = mesh.V.row(vIndexes(0));
		vertices.row(1) = mesh.V.row(vIndexes(1));
		vertices.row(2) = mesh.V.row(vIndexes(2));
		Eigen::Vector3d intersectionPoint;
		bool intersect = TriangleIntersection(sourcePointScene, dirToScene, vertices, mesh.F_normals.row(i), intersectionPoint);
		if (intersect) {
			double dist = (intersectionPoint - sourcePointScene).norm();
			if (closestShapeFaceIndex == -1 || dist < closestShapeFaceDist) {
				closestShapeFaceIndex = i;
				closestShapeFaceDist = dist;
				closestIntersectionPoint = intersectionPoint;
			}
		}
	}
	closestIntersectionToRet = closestIntersectionPoint;
	closestFaceIndexToRet = closestShapeFaceIndex;
	return closestShapeFaceIndex != -1;
}

float Project::Picking(const Eigen::Matrix4d& PV, const Eigen::Vector4i& viewportDims, int sectionIndex, int layerIndex, const std::vector<std::pair<int, int>> &stencilLayers, int x, int y) {
	y = viewportDims.w() - y;
	float viewportf[] = { (float)viewportDims(0), (float)viewportDims(1), (float)viewportDims(2), (float)viewportDims(3) };
	Eigen::Matrix4d sceneView = PV * MakeTransScaled();
	int closestShapeIndex = -1;
	int closestFaceIndex = -1;
	double closestFaceDist = -1;
	for (int i = 0; i < data_list.size(); i++) {
		ProjectMesh& mesh = *GetProjectMeshByIndex(i);
		if (!ShouldRenderViewerData(mesh, sectionIndex, layerIndex) || !mesh.IsPickable()) {
			continue;
		}
		Eigen::Matrix4d meshView = sceneView * mesh.MakeTransScaled();
		Eigen::Affine3d scaleMat = Eigen::Affine3d::Identity();
		double scaleFactor = mesh.GetPickingScaleFactor();
		scaleMat.scale(Eigen::Vector3d(scaleFactor, scaleFactor, scaleFactor));
		Eigen::Matrix4d meshViewScaled = meshView * scaleMat.matrix();
		Eigen::Vector3d sourcePointScene, dirToScene;
		ProjectScreenCoordToScene(x, y, viewportf, meshViewScaled.inverse(), sourcePointScene, dirToScene);
		Eigen::Vector3d closestIntersectionPoint;
		int closestShapeFaceIndex;
		if (GetClosestIntersectingFace(mesh, meshViewScaled, sourcePointScene, dirToScene, closestIntersectionPoint, closestShapeFaceIndex)) {
			Eigen::Vector4d transformed = meshView * closestIntersectionPoint.homogeneous();
			if (closestShapeIndex == -1 || transformed(2) < closestFaceDist) {
				closestShapeIndex = i;
				closestFaceIndex = closestShapeFaceIndex;
				closestFaceDist = transformed(2);
			}
		}
	}
	if (closestShapeIndex != -1) {
		selected_data_index = closestShapeIndex;
		pShapes.push_back(selected_data_index);
		if (GetProjectMeshByIndex(selected_data_index)->DrawOutline()) {
			data_list[selected_data_index]->AddSectionLayers(stencilLayers);
		}
	}
	else if (sectionIndex == editBezierSection) {
		TryPickSegment(PV, viewportDims, x, y);
	}
	return (float)closestFaceDist;
}

double CalcDistanceBetweenPointAndLine(Eigen::Vector2d pointA, Eigen::Vector2d pointB, Eigen::Vector2d P) {
	double aside = (P - pointA).dot(pointB - pointA);
	if (aside < 0.0)
		return (P - pointA).norm();
	double bside = (P - pointB).dot(pointA - pointB);
	if (bside < 0.0) 
		return (P - pointB).norm();
	Eigen::Vector2d pointOnLine = (bside * pointA + aside * pointB) / pow((pointA - pointB).norm(), 2.0);
	return (P - pointOnLine).norm();
}

void Project::TryPickSegment(const Eigen::Matrix4d& PV, const Eigen::Vector4i& viewportDims, int x, int y) {
	double closeThreshold = 0.5;
	float viewportf[] = { (float)viewportDims(0), (float)viewportDims(1), (float)viewportDims(2), (float)viewportDims(3) };
	Eigen::Matrix4d sceneView = PV * MakeTransScaled();
	ProjectMesh& mesh = *GetProjectMeshByIndex(bezierCurvePlane);
	Eigen::Matrix4d meshView = sceneView * mesh.MakeTransScaled();
	Eigen::Vector3d sourcePointScene, dirToScene;
	ProjectScreenCoordToScene(x, y, viewportf, meshView.inverse(), sourcePointScene, dirToScene);
	double distanceToZ = -sourcePointScene(2);
	double factor = distanceToZ/dirToScene(2);
	Eigen::Vector2d intersectionPoint = (sourcePointScene + factor * dirToScene).head(2);
	int foundLineIndex = -1;
	for (int i = 0; i < mesh.lines.rows(); i++) {
		Eigen::Vector2d pointA = mesh.lines.row(i).head(2);
		Eigen::Vector2d pointB = mesh.lines.row(i).block(0, 3, 1, 2).head(2);
		double dist = CalcDistanceBetweenPointAndLine(pointA, pointB, intersectionPoint);
		if (abs(dist) < closeThreshold) {
			foundLineIndex = i;
			break;
		}
	}
	currentSelectedBezierSegment = foundLineIndex == -1 ? -1 : GetCurrentBezierMesh()->GetCachedMap().row(foundLineIndex)(0);
	SetControlPointsPosition();
	DrawBezierCurves();
	//float denom = Eigen::Vector3d::UnitZ().dot(dirToScene);
	//float t = (-sourcePointScene).dot(Eigen::Vector3d::UnitZ()) / denom;
	//Eigen::Vector3d intersectionPoint = sourcePointScene + t * dirToScene;
}

void Project::ToggleSplitMode() {
	if (splitMode) {
		renderer->DeactivateSection(leftSection);
		renderer->DeactivateSection(editBezierMode ? editBezierSection : rightSection);
		renderer->ActivateSection(fullScreenSection);
		editBezierMode = false;
	}
	else {
		renderer->DeactivateSection(fullScreenSection);
		renderer->ActivateSection(leftSection);
		renderer->ActivateSection(editBezierMode ? editBezierSection : rightSection);
	}
	splitMode = !splitMode;
}

void Project::ToggleEditBezierMode() {
	if (editBezierMode) {
		ExitBezierMode();
	}
	else if (!EnterBezierMode()) {
		return;
	}
	if (splitMode) {
		renderer->DeactivateSection(editBezierMode ? editBezierSection : rightSection);
		renderer->ActivateSection(editBezierMode ? rightSection : editBezierSection);
		editBezierMode = !editBezierMode;
	}
	else {
		editBezierMode = true;
		ToggleSplitMode();
	}
	if (editBezierMode) {
		renderer->UpdateSection(editBezierSection);
	}
}

bool Project::EnterBezierMode() {
	auto a = std::find_if(pShapes.begin(), pShapes.end(), [this](int arg) {
		return GetProjectMeshByIndex(arg)->AllowAnimations();
	});
	if (a != pShapes.end()) {
		currentBezierMeshIndex = *a;
		currentSelectedBezierSegment = -1;
		SetControlPointsPosition();
		DrawBezierCurves();
		return true;
	}
	else {
		return false;
	}
}

void Project::ExitBezierMode() {
	currentBezierMeshIndex = -1;
	currentSelectedBezierSegment = -1;
	data_list[bezierCurvePlane]->clear_edges();
	HideControlPoints();
}

bool Project::CanEnterBezierMode() {
	auto a = std::find_if(pShapes.begin(), pShapes.end(), [this](int arg) {
		return GetProjectMeshByIndex(arg)->AllowAnimations();
	});
	return a != pShapes.end();
}

ProjectMesh* Project::GetProjectMeshByIndex(int index) {
	auto projectMeshToRet = dynamic_cast<ProjectMesh*>(const_cast<igl::opengl::ViewerData*>(data_list[index]));
	return projectMeshToRet;
}

bool Project::AddPickedShapes
(
	const Eigen::Matrix4d& PV,
	const Eigen::Vector4i& viewport, int sectionIndex, int layerIndex,
	int left, int right, int up, int bottom,
	const std::vector<std::pair<int, int>>& stencilLayers,
	std::vector<double> &depths
)
{
	if (renderer->GetCurrentSectionIndex() == editBezierSection) {
		// Multi picking not allowed in bezier edit
		return false;
	}
	//not correct when the shape is scaled
	Eigen::Matrix4d MVP = PV * MakeTransd();
	//std::cout << "picked shapes  ";
	bool isFound = false;
	for (int i = 0; i < data_list.size(); i++)
	{ //add to pShapes if the center in range
		ProjectMesh& mesh = *GetProjectMeshByIndex(i);
		Eigen::Matrix4d posMatrix = CalculatePosMatrix(i, MVP);
		Eigen::Vector4d centerPos = posMatrix * Eigen::Vector4d(0, 0, 0, 1);
		double xpix = (1 + centerPos.x() / centerPos.z()) * viewport.z() / 2;
		double ypix = (1 + centerPos.y() / centerPos.z()) * viewport.w() / 2;
		if (ShouldRenderViewerData(mesh, sectionIndex, layerIndex) && mesh.IsPickable() && xpix < right && xpix > left && ypix < bottom && ypix > up)
		{
			pShapes.push_back(i);
			if (mesh.DrawOutline())
			{
				data_list[i]->AddSectionLayers(stencilLayers);
			}
			//std::cout << i << ", ";
			selected_data_index = i;
			isFound = true;

			depths.push_back(CalculateDepthOfMesh(mesh, posMatrix));
		}
	}
	return isFound;
}

igl::opengl::glfw::ViewerDataCreateFunc Project::GetStaticDataCreator(int layer, bool isPicking, bool outline, bool allowTransparent, Eigen::Vector3d color) {
	return [this, layer, isPicking, outline, allowTransparent, color]()
	{
		return new ProjectMesh(layer, isPicking, outline, allowTransparent, color);
	};
}
igl::opengl::glfw::ViewerDataCreateFunc Project::GetAnimatedDataCreator(int layer, bool isPicking, bool outline, bool allowTransparent, Eigen::Vector3d color) {
	return [this, layer, isPicking, outline, allowTransparent, color]()
	{
		return new AnimatedMesh(layer, isPicking, outline, allowTransparent, color);
	};
}

void Project::TranslateCamera(Eigen::Vector3d d) {
	WindowSection &section = renderer->GetCurrentSection();
	int cameraIndex = section.GetCamera();
	igl::opengl::Camera &camera = renderer->GetCamera(cameraIndex);

	if (renderer->GetCurrentSectionIndex() == GetBezierSectionIndex())
	{
		double currentTranslation = camera.GetPosition().z();
		d.z() = std::max(1 - currentTranslation, d.z());
	}

	Viewer::TranslateCamera(camera, d);
}

void Project::RotateCamera(const std::vector<std::pair<Eigen::Vector3d, double>> &angledAxes)
{
	if (renderer->GetCurrentSection().IsRotationAllowed())
	{
		Viewer::RotateCamera(angledAxes);
	}
}

void Project::WhenScroll(const Eigen::Matrix4d& preMat, float dy) {
	if (renderer->GetCurrentSectionIndex() == editBezierSection) {
		return;
	}
	igl::opengl::glfw::Viewer::WhenScroll(preMat, dy);
}

void Project::WhenTranslate(const Eigen::Matrix4d& preMat, float dx, float dy)
{
	Eigen::Matrix3d rot = preMat.block<3, 3>(0, 0);
	Transform(GetMovableTransformee(), [&rot, &dx, &dy](Movable& movable)
	{
		movable.TranslateInSystem(rot, Eigen::Vector3d(dx, 0, 0));
		movable.TranslateInSystem(rot, Eigen::Vector3d(0, dy, 0));
	});
	auto controlPointMesh = dynamic_cast<BezierControlPointMesh*>(const_cast<igl::opengl::ViewerData*>(data()));
	if (controlPointMesh != NULL) {
		AnimatedMesh* currentEditedMesh = GetCurrentBezierMesh();
		if (currentEditedMesh != NULL) {
			currentEditedMesh->MoveControlPoint(currentSelectedBezierSegment, controlPointMesh->GetPointNumber(), (rot.transpose() * Eigen::Vector3d(dx, dy, 0)));
			DrawBezierCurves();
		}
	}
}

void Project::WhenRotate(const Eigen::Matrix4d &preMat, float dx, float dy)
{
	if (renderer->GetCurrentSection().IsRotationAllowed())
	{
		Viewer::WhenRotate(preMat, dx, dy);
	}
}

void Project::DrawBezierCurves() {
	Eigen::MatrixX3d P;
	Eigen::MatrixX2i E;
	Eigen::VectorXi ESMAP;
	AnimatedMesh* currentEditedMesh = GetCurrentBezierMesh();
	currentEditedMesh->GetEdges(E, P, ESMAP);
	Eigen::MatrixXd C;
	C.resize(E.rows(), 3);
	for (int i = 0; i < E.rows(); i++) {
		if (ESMAP(i) == currentSelectedBezierSegment) {
			C.row(i) = BEZIER_SELECTED_CURVE_COLOR;
		}
		else {
			C.row(i) = BEZIER_CURVE_COLOR;
		}
	}
	ProjectMesh& bezierCurvePlaneMesh = *GetProjectMeshByIndex(bezierCurvePlane);
	bezierCurvePlaneMesh.set_edges(P, E, C);
}

void Project::HideControlPoints() {
	for (int i = 0; i < 4; i++) {
		data_list[controlPointsMeshIndexes[i]]->Hide();
	}
}

void Project::SetControlPointsPosition() {
	if (currentBezierMeshIndex == -1 || currentSelectedBezierSegment == -1) {
		HideControlPoints();
		return;
	}
	AnimatedMesh* currentEditedMesh = GetCurrentBezierMesh();
	for (int i = 0; i < 4; i++) {
		BezierControlPointMesh* controlPointMesh = dynamic_cast<BezierControlPointMesh*>(const_cast<igl::opengl::ViewerData*>
			(data_list[controlPointsMeshIndexes[i]]));
		controlPointMesh->SetPosition(currentEditedMesh->GetControlPoint(currentSelectedBezierSegment, controlPointMesh->GetPointNumber()));
		controlPointMesh->UnHide();
	}
}
