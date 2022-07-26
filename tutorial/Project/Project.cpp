#include "Project.h"
#include "AnimationCameraData.h"
#include "igl/opengl/glfw/renderer.h"
#include "igl/opengl/ViewerData.h"
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

Project::Project() : selectedCameraIndex{0}, isInDesignMode{true}, isDesignModeView{true}, shaderIndex_basic{-1}
{
}

void Project::Init()
{		
	Viewer::AddLayer();
	Viewer::AddLayer();

	int shaderIndex_picking = AddShader("shaders/pickingShader");
	int shaderIndex_cubemap = AddShader("shaders/cubemapShader");
	int shaderIndex_basicTex = AddShader("shaders/basicShaderTex");
	shaderIndex_basic = AddShader("shaders/basicShader");

	int textureIndex_plane = AddTexture("textures/plane.png", 2);
	int textureIndex_cubeMap_daylightBox = AddTexture("textures/cubemaps/Daylight Box UV.png", 3);
	int textureIndex_grass = AddTexture("textures/grass.bmp", 2);
	int textureIndex_box0 = AddTexture("textures/box0.bmp", 2);
	int textureIndex_bricks = AddTexture("textures/bricks.jpg", 2);

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

	int sceneCube = AddShape(Cube, -2, TRIANGLES);
	int scissorBox = AddShape(Plane, -2, TRIANGLES, 1);
	data_list[scissorBox]->layer = 0;
	int cube1 = AddShape(Cube, -1, TRIANGLES);
	int cube2 = AddShape(Cube, -1, TRIANGLES);
	SetShapeShader(sceneCube, shaderIndex_cubemap);
	SetShapeShader(scissorBox, shaderIndex_picking);
	SetShapeShader(cube1, shaderIndex_basic);
	SetShapeShader(cube2, shaderIndex_basic);
	SetShapeMaterial(sceneCube, materialIndex_cube);
	//SetShapeMaterial(scissorBox, materialIndex_box0);
	SetShapeMaterial(cube1, materialIndex_grass);
	SetShapeMaterial(cube2, materialIndex_box0);


	selected_data_index = sceneCube;
	float s = 60;
	ShapeTransformation(scaleAll, s,0);
	SetShapeStatic(sceneCube);
	selected_data_index = scissorBox;
	ShapeTransformation(zTranslate, -1.1, 1);
	SetShapeStatic(scissorBox);
	selected_data_index = cube2;
	ShapeTransformation(xTranslate, 2, 1);

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

void Project::ScaleAllShapes(float amt, int viewportIndx)
{
	for (int i = 1; i < data_list.size(); i++)
	{
		if (data_list[i]->Is2Render(viewportIndx))
		{
			data_list[i]->MyScale(Eigen::Vector3d(amt, amt, amt));
		}
	}
}

bool Project::ShouldRenderViewerData(const igl::opengl::ViewerData& data, const int viewportIndx) const
{
	auto pAnimationCameraData = dynamic_cast<AnimationCameraData*>(const_cast<igl::opengl::ViewerData*>(&data));
	return (pAnimationCameraData == nullptr || EffectiveDesignModeView()) &&
		Viewer::ShouldRenderViewerData(data, viewportIndx);
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
			"./data/arm.obj",
			-1,
			TRIANGLES,
			[this, &cameraIndex]()
			{
				return new AnimationCameraData(currentEditingLayer, cameraIndex);
			}
		);
		SetShapeShader(shapeIndex, shaderIndex_basic);
		igl::opengl::ViewerData *shape = data_list[shapeIndex];
		shape->MyRotate(Eigen::Vector3d(0, 1, 0), 90);
		break;
	}
}

void Project::CameraMeshHide(int cameraIndex)
{
	for (auto vd : data_list)
	{
		auto pAnimationCameraData = dynamic_cast<AnimationCameraData*>(vd);
		if (pAnimationCameraData && pAnimationCameraData->cameraIndex == cameraIndex)
		{
			pAnimationCameraData->Hide();
		}
	}
}

void Project::CameraMeshUnhide(int cameraIndex, const Movable& transformations)
{
	for (auto vd : data_list)
	{
		auto pAnimationCameraData = dynamic_cast<AnimationCameraData*>(vd);
		if (pAnimationCameraData && pAnimationCameraData->cameraIndex == cameraIndex)
		{
			pAnimationCameraData->SetPosition(transformations.GetPosition());
			pAnimationCameraData->SetRotation(transformations.GetRotation());
			pAnimationCameraData->MyRotate(Eigen::Vector3d(0, 1, 0), 90);
			pAnimationCameraData->UnHide();
		}
	}
}

Project::~Project(void)
{
}

void ProjectScreenCoordToScene(double x, double y, float m_viewport[], const Eigen::Matrix4d& sceneViewInverse, Eigen::Vector3d& sceneSourceCoord, Eigen::Vector3d& sceneDir) {
	Eigen::Vector4d sceneCoord, destCoord;
	sceneCoord <<
		2 * ((x - m_viewport[0]) / m_viewport[2]) - 1.0,
		2 * ((y - m_viewport[1]) / m_viewport[3]) - 1.0,
		0, 1
		;
	destCoord <<
		2 * ((x - m_viewport[0]) / m_viewport[2]) - 1.0,
		2 * ((y - m_viewport[1]) / m_viewport[3]) - 1.0,
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
	float dist = (normal.dot(vv1)) / normal.dot(dir);
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
	float closestShapeFaceDist = -1;
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
			float dist = (intersectionPoint - sourcePointScene).norm();
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

float Project::Picking(const Eigen::Matrix4d& PV, const Eigen::Vector4i& viewportDims, int viewport, int pickingViewport, int x, int y) {
	// TODO section
	ClearPickedShapes(pickingViewport);
	y = viewportDims(3) - y;
	float viewportf[] = { viewportDims(0), viewportDims(1), viewportDims(2), viewportDims(3) };
	Eigen::Matrix4d sceneView = PV * MakeTransScaled();
	int closestShapeIndex = -1;
	int closestFaceIndex = -1;
	float closestFaceDist = -1;
	for (int i = 1; i < data_list.size(); i++) {
		igl::opengl::ViewerData* mesh = data_list[i];
		if (!ShouldRenderViewerData(*mesh, viewport)) {
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
		data_list[selected_data_index]->AddViewport(pickingViewport);
	}
	return closestFaceDist;
}