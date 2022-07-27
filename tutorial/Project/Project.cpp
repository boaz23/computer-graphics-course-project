#include "Project.h"
#include "AnimationCameraData.h"
#include "igl/opengl/glfw/renderer.h"
#include "igl/opengl/ViewerData.h"
#include "igl/opengl/util.h"
#include <iostream>


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

Project::Project() : 
	isInDesignMode{true}, 
	isDesignModeView{true}, 
	shaderIndex_basic{-1},
	pickingShaderIndex(-1),
	camerasToMesh{}
{
}

void Project::Init()
{		
	Viewer::AddLayer();
	Viewer::AddLayer();
	pickingShaderIndex = AddShader("shaders/pickingShader");
	int shaderIndex_cubemap = AddShader("shaders/cubemapShader");
	int shaderIndex_basicTex = AddShader("shaders/basicShaderTex");
	shaderIndex_basic = AddShader("shaders/basicShader");

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

	int materialIndex_box0 = AddMaterial(texIDs + 0, slots + 0, 1);
	materialIndex_cube = AddMaterial(texIDs + 1, slots + 1, 1);
	int materialIndex_grass = AddMaterial(texIDs + 2, slots + 2, 1);
	int materialIndex_plane = AddMaterial(texIDs + 3, slots + 3, 1);
	int materialIndex_bricks = AddMaterial(texIDs + 4, slots + 4, 1);
	availableMaterials =
	{
		std::pair<int, std::string>{materialIndex_box0, "Box"},
		std::pair<int, std::string>{materialIndex_grass, "Grass"},
		std::pair<int, std::string>{materialIndex_plane, "Plane"},
		std::pair<int, std::string>{materialIndex_bricks, "Bricks"}
	};
	std::vector<std::pair<int, int>>& sceneLayers = renderer->GetSceneLayersIndexes();
	int sceneCube = AddShape(Cube, -2, TRIANGLES, shaderIndex_cubemap, sceneLayers);
	int scissorBox = AddShape(Plane, -2, TRIANGLES, pickingShaderIndex, renderer->GetScissorsTestLayersIndexes());
	int cube1 = AddShape(Cube, -1, TRIANGLES, shaderIndex_basic, sceneLayers);
	int cube2 = AddShape(Cube, -1, TRIANGLES, shaderIndex_basic, sceneLayers);
	data_list[scissorBox]->layer = 0;
	SetShapeMaterial(sceneCube, materialIndex_cube);
	//SetShapeMaterial(scissorBox, materialIndex_box0);
	SetShapeMaterial(cube1, materialIndex_grass);
	SetShapeMaterial(cube2, materialIndex_box0);


	selected_data_index = sceneCube;
	float s = 60;
	ShapeTransformation(scaleAll, s, 0);
	SetShapeStatic(sceneCube);
	selected_data_index = scissorBox;
	ShapeTransformation(zTranslate, -1.1f, 1);
	SetShapeStatic(scissorBox);
	selected_data_index = cube2;
	ShapeTransformation(xTranslate, 2.f, 1);

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

	if (data_list[shapeIndx]->GetMaterial() >= 0 && !materials.empty())
	{
		BindMaterial(s, data_list[shapeIndx]->GetMaterial());
	}
	if (shaderIndx == 0)
		s->SetUniform4f("lightColor", 0xc6 / 255.0f, 0x1e / 255.0f, 0x24 / 255.0f, 0.5f);
	if (shaderIndx == shaderIndex_basic) {
		s->SetUniform1f("Transperancy", data_list[shapeIndx]->alpha);
	}
	s->Unbind();
}


void Project::WhenRotate()
{
}

void Project::WhenTranslate()
{
}

void Project::Animate() {
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

	return AddShapeFromFile(filePath, -1, TRIANGLES, shaderIndex_basic, renderer->GetSceneLayersIndexes());
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
			"./data/arm.obj", // TODO: find a better mesh
			-1,
			TRIANGLES,
			shaderIndex_basic,
			renderer->GetSceneLayersIndexes(),
			[this, &cameraIndex]()
			{
				return new AnimationCameraData(currentEditingLayer, cameraIndex);
			}
		);
		camerasToMesh.insert(std::pair<int, int>{cameraIndex, shapeIndex});
		igl::opengl::ViewerData *shape = data_list[shapeIndex];
		shape->MyRotate(Eigen::Vector3d(0, 1, 0), 90);
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
	WindowSection &section = renderer->GetSection(currentSectionIndex);

	int currentCameraIndex = section.GetCamera();
	int currentMesh = GetMeshIndex(currentCameraIndex);

	int nextCameraIndex = (int)addCyclic<int>(currentCameraIndex, delta, renderer->CamerasCount());
	int nextMesh = GetMeshIndex(nextCameraIndex);

	const std::vector<SectionLayer *> &sectionLayers = section.GetLayers();
	for (size_t i = 0; i < sectionLayers.size(); ++i)
	{
		const auto &key = std::pair<int, int>{ currentSectionIndex, i };
		if (currentMesh >= 0)
		{
			data_list[currentMesh]->AddSectionLayers({ key });
		}
		if (nextMesh >= 0)
		{
			data_list[nextMesh]->RemoveSectionLayers({ key });
		}
	}
	section.SetCamera(nextCameraIndex);
}

void Project::MoveCamera(std::function<void(Movable &)> transform)
{
	WindowSection &section = renderer->GetCurrentSection();
	int cameraIndex = section.GetCamera();
	auto meshIt = camerasToMesh.find(cameraIndex);
	if (meshIt == camerasToMesh.end())
	{
		std::cerr << "Invalid camera index " << cameraIndex << std::endl;
		return;
	}
	int mesh = meshIt->second;
	igl::opengl::ViewerData &shape = *data_list[mesh];
	igl::opengl::Camera &camera = renderer->GetCamera(cameraIndex);
	transform(shape);
	transform(camera);
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

bool GetClosestIntersectingFace(igl::opengl::ViewerData* mesh, const Eigen::Matrix4d& meshView, const Eigen::Vector3d& sourcePointScene, const Eigen::Vector3d& dirToScene, Eigen::Vector3d& closestIntersectionToRet, int& closestFaceIndexToRet) {
	int closestShapeFaceIndex = -1;
	double closestShapeFaceDist = -1;
	Eigen::Vector3d closestIntersectionPoint = Eigen::Vector3d::Ones();
	for (int i = 0; i < mesh->F.rows(); i++) {
		Eigen::Matrix3d vertices;
		Eigen::Vector3i vIndexes = mesh->F.row(i);
		vertices.row(0) = mesh->V.row(vIndexes(0));
		vertices.row(1) = mesh->V.row(vIndexes(1));
		vertices.row(2) = mesh->V.row(vIndexes(2));
		Eigen::Vector3d intersectionPoint;
		bool intersect = TriangleIntersection(sourcePointScene, dirToScene, vertices, mesh->F_normals.row(i), intersectionPoint);
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
	for (int i = 1; i < data_list.size(); i++) {
		igl::opengl::ViewerData* mesh = data_list[i];
		if (!ShouldRenderViewerData(*mesh, sectionIndex, layerIndex)) {
			continue;
		}
		Eigen::Matrix4d meshView = sceneView * mesh->MakeTransScaled();
		Eigen::Vector3d sourcePointScene, dirToScene;
		ProjectScreenCoordToScene(x, y, viewportf, meshView.inverse(), sourcePointScene, dirToScene);
		Eigen::Vector3d closestIntersectionPoint;
		int closestShapeFaceIndex;
		if (GetClosestIntersectingFace(mesh, meshView, sourcePointScene, dirToScene, closestIntersectionPoint, closestShapeFaceIndex)) {
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
		data_list[selected_data_index]->AddSectionLayers(stencilLayers);
	}
	return (float)closestFaceDist;
}