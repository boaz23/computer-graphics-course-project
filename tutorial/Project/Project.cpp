#include "Project.h"
#include <iostream>


static void printMat(const Eigen::Matrix4d& mat)
{
	std::cout<<" matrix:"<<std::endl;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			std::cout<< mat(j,i)<<" ";
		std::cout<<std::endl;
	}
}

Project::Project()
{
}

//Project::Project(float angle ,float relationWH, float zNear, float zFar) : Scene(angle,relationWH,zNear,zFar)
//{ 	
//}

void Project::Init()
{		
	unsigned int texIDs[3] = { 0 , 1, 2};
	unsigned int slots[3] = { 0 , 1, 2 };
	
	int shaderIndex_picking = AddShader("shaders/pickingShader");
	int shaderIndex_cubemap = AddShader("shaders/cubemapShader");
	int shaderIndex_basicTex = AddShader("shaders/basicShaderTex");
	int shaderIndex_basic = AddShader("shaders/basicShader");
	
	int textureIndex_plane = AddTexture("textures/plane.png",2);
	int textureIndex_cubeMap_daylightBox = AddTexture("textures/cubemaps/Daylight Box_", 3);
	int textureIndex_grass = AddTexture("textures/grass.bmp", 2);

	int materialIndex_basic = AddMaterial(texIDs + 0, slots + 0, 1);
	int materialIndex_cube = AddMaterial(texIDs + 1, slots + 1, 1);
	int materialIndex_2 = AddMaterial(texIDs + 2, slots + 2, 1);
	
	int shapeIndex_cube = AddShape(Cube, -2, TRIANGLES);
	int shapeIndex_zCylinder0 = AddShape(zCylinder, -1, TRIANGLES);
	int shapeIndex_zCylinder1 = AddShape(zCylinder, 1, TRIANGLES);
	int shapeIndex_zCylinder2 = AddShape(zCylinder, 2, TRIANGLES);
	int shapeIndex_axis = AddShape(Axis, -1, LINES);
	
	SetShapeShader(shapeIndex_zCylinder0, shaderIndex_basicTex);
	SetShapeShader(shapeIndex_zCylinder1, shaderIndex_basicTex);
	SetShapeShader(shapeIndex_zCylinder2, shaderIndex_basicTex);
	SetShapeShader(shapeIndex_axis, shaderIndex_basicTex);


	SetShapeMaterial(shapeIndex_zCylinder0, materialIndex_basic);
	SetShapeMaterial(shapeIndex_zCylinder1, materialIndex_basic);
	SetShapeMaterial(shapeIndex_zCylinder2, materialIndex_basic);
	SetShapeMaterial(shapeIndex_axis, materialIndex_basic);

	SetShapeMaterial(shapeIndex_cube, materialIndex_cube);

	selectedCameraIndex = 0;

	selected_data_index = shapeIndex_cube;
	float cylinderLen = 1.6f;
	float s = 60;
	ShapeTransformation(scaleAll, s, 0);
	selected_data_index = shapeIndex_zCylinder0;
	data()->SetCenterOfRotation(Eigen::Vector3d(0, 0, -cylinderLen / 2.0));
	ShapeTransformation(zTranslate, cylinderLen / 2.0, 1);
	
	selected_data_index = shapeIndex_zCylinder1;
	ShapeTransformation(zTranslate, cylinderLen , 1);
	data()->SetCenterOfRotation(Eigen::Vector3d(0, 0, -cylinderLen / 2.0));
	
	selected_data_index = shapeIndex_zCylinder2;
	ShapeTransformation(zTranslate, cylinderLen, 1);
	data()->SetCenterOfRotation(Eigen::Vector3d(0, 0, -cylinderLen / 2.0));

	selected_data_index = shapeIndex_cube;
	SetShapeStatic(shapeIndex_cube);

	//SetShapeViewport(6, 1);
//	ReadPixel(); //uncomment when you are reading from the z-buffer
}

void Project::Update(const Eigen::Matrix4f& Proj, const Eigen::Matrix4f& View, const Eigen::Matrix4f& Model, unsigned int  shaderIndx, unsigned int shapeIndx)
{
	Shader *s = shaders[shaderIndx];
	int r = ((shapeIndx+1) & 0x000000FF) >>  0;
	int g = ((shapeIndx+1) & 0x0000FF00) >>  8;
	int b = ((shapeIndx+1) & 0x00FF0000) >> 16;


		s->Bind();
	s->SetUniformMat4f("Proj", Proj);
	s->SetUniformMat4f("View", View);
	s->SetUniformMat4f("Model", Model);
	if (data_list[shapeIndx]->GetMaterial() >= 0 && !materials.empty())
	{
//		materials[shapes[selected_data_index]->GetMaterial()]->Bind(textures);
		BindMaterial(s, data_list[shapeIndx]->GetMaterial());
	}
	if (shaderIndx == 0)
		s->SetUniform4f("lightColor", r / 255.0f, g / 255.0f, b / 255.0f, 0.0f);
	else
		s->SetUniform4f("lightColor", 4/100.0f, 60 / 100.0f, 99 / 100.0f, 0.5f);
	//textures[0]->Bind(0);

	
	

	//s->SetUniform1i("sampler2", materials[shapes[selected_data_index]->GetMaterial()]->GetSlot(1));
	//s->SetUniform4f("lightDirection", 0.0f , 0.0f, -1.0f, 0.0f);
//	if(shaderIndx == 0)
//		s->SetUniform4f("lightColor",r/255.0f, g/255.0f, b/255.0f,1.0f);
//	else 
//		s->SetUniform4f("lightColor",0.7f,0.8f,0.1f,1.0f);
	s->Unbind();
}


void Project::WhenRotate()
{
}

void Project::WhenTranslate()
{
}

void Project::Animate() {
    if(isActive)
	{
		if(selected_data_index > 0 )
			data()->MyRotate(Eigen::Vector3d(0, 1, 0), 0.01);
	}
}

void Project::ScaleAllShapes(float amt,int viewportIndx)
{
	for (int i = 1; i < data_list.size(); i++)
	{
		if (data_list[i]->Is2Render(viewportIndx))
		{
            data_list[i]->MyScale(Eigen::Vector3d(amt, amt, amt));
		}
	}
}

Project::~Project(void)
{
}

