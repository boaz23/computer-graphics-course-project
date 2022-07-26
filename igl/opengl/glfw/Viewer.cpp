// This file is part of libigl, a simple c++ geometry processing library.
//
// Copyright (C) 2014 Daniele Panozzo <daniele.panozzo@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.

#include "Viewer.h"

//#include <chrono>
#include <thread>

#include <Eigen/LU>

#include <utility>
#include <cmath>
#include <cstdio>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <limits>
#include <cassert>

#include <igl/project.h>
//#include <igl/get_seconds.h>
#include <igl/readOBJ.h>
#include <igl/readOFF.h>
#include <igl/adjacency_list.h>
#include <igl/writeOBJ.h>
#include <igl/writeOFF.h>
#include <igl/massmatrix.h>
#include <igl/file_dialog_open.h>
#include <igl/file_dialog_save.h>
#include <igl/quat_mult.h>
#include <igl/axis_angle_to_quat.h>
#include <igl/trackball.h>
#include <igl/two_axis_valuator_fixed_up.h>
#include <igl/snap_to_canonical_view_quat.h>
#include <igl/unproject.h>
#include <igl/serialize.h>
#include <igl/opengl/util.h>
#include "../gl.h"

// Internal global variables used for glfw event handling
//static igl::opengl::glfw::Viewer * __viewer;
static double highdpi = 1;
static double scroll_x = 0;
static double scroll_y = 0;


namespace igl
{
namespace opengl
{
namespace glfw
{

  void Viewer::Init(const std::string config)
  {
  }

  IGL_INLINE Viewer::Viewer():
    data_list(1),
    selected_data_index(0),
    next_data_id(1),
    next_shader_id(1),
	isActive(false),
      renderer{nullptr},
      layers{},
      currentEditingLayer{1},
      currentObjectLayer{1},
      isEditingObjectLayer{false}
  {
    staticScene = 0;
    overlay_point_shader = nullptr;
    overlay_shader = nullptr;


    // Temporary variables initialization
   // down = false;
  //  hack_never_moved = true;
    scroll_position = 0.0f;
    SetShader_overlay("shaders/overlay");
    SetShader_point_overlay("shaders/overlay_points");


    
//#ifndef IGL_VIEWER_VIEWER_QUIET
//    const std::string usage(R"(igl::opengl::glfw::Viewer usage:
//  [drag]  Rotate scene
//  A,a     Toggle animation (tight draw loop)
//  F,f     Toggle face based
//  I,i     Toggle invert normals
//  L,l     Toggle wireframe
//  O,o     Toggle orthographic/perspective projection
//  T,t     Toggle filled faces
//  [,]     Toggle between cameras
//  1,2     Toggle between models
//  ;       Toggle vertex labels
//  :       Toggle face labels)"
//);
//    std::cout<<usage<<std::endl;
//#endif
  }

  IGL_INLINE Viewer::~Viewer()
  {
  }

  void Viewer::ToggleLayerVisibility(int layer)
  {
      layers[layer] = !layers[layer];
  }

  int Viewer::AddLayer()
  {
      int newLayer = layers.size();
      layers.push_back(false);
      return newLayer;
  }

  void Viewer::ChangeCubemapImage(std::string filePath)
  {
      Material* cubemapMaterial = materials[materialIndex_cube];
      int textureIndex = AddTexture(filePath, 3);
      cubemapMaterial->ChangeTexture(0, textureIndex);
  }

IGL_INLINE bool
    Viewer::load_mesh_from_data(const Eigen::MatrixXd &V,
                                const Eigen::MatrixXi &F,
                                const Eigen::MatrixXd &UV_V,
                                const Eigen::MatrixXi &UV_F,
                                const ViewerDataCreateFunc dataCreator) {
    if(!(data()->F.rows() == 0  && data()->V.rows() == 0))
    {
        append_mesh(dataCreator);
    }
    data()->clear();
        data()->set_mesh(V,F);
    if (UV_V.rows() > 0)
    {
        data()->set_uv(UV_V, UV_F);
    }
    else
    {
        data()->grid_texture();
    }
    data()->compute_normals();
    data()->uniform_colors(Eigen::Vector3d(255.0/255.0,255.0/255.0,0.0/255.0),
                           Eigen::Vector3d(255.0/255.0,228.0/255.0,58.0/255.0),
                           Eigen::Vector3d(255.0/255.0,235.0/255.0,80.0/255.0));
    return true;
  }

  IGL_INLINE bool Viewer::load_mesh_from_file(
      const std::string & mesh_file_name_string, const ViewerDataCreateFunc dataCreator)
  {
      bool normal_read = false;
    // Create new data slot and set to selected
    if(!(data()->F.rows() == 0  && data()->V.rows() == 0))
    {
      append_mesh(dataCreator);
    }
    data()->clear();

    size_t last_dot = mesh_file_name_string.rfind('.');
    if (last_dot == std::string::npos)
    {
      std::cerr<<"Error: No file extension found in "<<
        mesh_file_name_string<<std::endl;
      return false;
    }

    std::string extension = mesh_file_name_string.substr(last_dot+1);

    if (extension == "off" || extension =="OFF")
    {
      Eigen::MatrixXd V;
      Eigen::MatrixXi F;
      if (!igl::readOFF(mesh_file_name_string, V, F))
        return false;
      data()->set_mesh(V,F);
    }
    else if (extension == "obj" || extension =="OBJ")
    {
      Eigen::MatrixXd corner_normals;
      Eigen::MatrixXi fNormIndices;
      Eigen::MatrixXd UV_V;
      Eigen::MatrixXi UV_F;
      Eigen::MatrixXd V,N;
      Eigen::MatrixXi F;


      if (!(
            igl::readOBJ(
              mesh_file_name_string,
              V, UV_V, corner_normals, F, UV_F, fNormIndices)))
      {
        return false;
      }
      else
          if (corner_normals.rows() > 0)
          {
              //std::cout << "normals: \n" << corner_normals << std::endl;
              //std::cout << "indices: \n" << fNormIndices << std::endl;
              N = Eigen::RowVector3d(0, 0, 1).replicate(fNormIndices.rows(), 1);
              for (size_t  k = 0;  k < N.rows();  k++)
              {
                  N.row(k) = corner_normals.row(fNormIndices(k, 0));
                  //std::cout << "faces normals:  " << corner_normals.row(fNormIndices(k, 0)) << std::endl;
              }
              
              //std::cout << "faces normals: \n" << N << std::endl;
             
              normal_read = true;
          }
      // TODO this will center meshes around (0, 0, 0)
      Eigen::RowVector3d minVals = V.colwise().minCoeff();
      Eigen::RowVector3d maxVals = V.colwise().maxCoeff();
      Eigen::RowVector3d fixVals = (maxVals + minVals).array()/2.0;
      V.rowwise() -= fixVals;
      data()->set_mesh(V,F);
      if(normal_read)
        data()->set_normals(N);
      if (UV_V.rows() > 0)
      {
          data()->set_uv(UV_V, UV_F);
      }

    }
    else
    {
      // unrecognized file type
      printf("Error: %s is not a recognized file type.\n",extension.c_str());
      return false;
    }
    if(!normal_read)
        data()->compute_normals();
    data()->uniform_colors(Eigen::Vector3d(255.0/255.0,255.0/255.0,0.0/255.0),
                   Eigen::Vector3d(255.0/255.0,228.0/255.0,58.0/255.0),
                   Eigen::Vector3d(255.0/255.0,235.0/255.0,80.0/255.0));

    // Elik: why?
    if (data()->V_uv.rows() == 0)
    {
      data()->grid_texture();
    }
    

    //for (unsigned int i = 0; i<plugins.size(); ++i)
    //  if (plugins[i]->post_load())
    //    return true;
    data()->AfterInit();
    return true;
  }

  IGL_INLINE bool Viewer::save_mesh_to_file(
      const std::string & mesh_file_name_string)
  {
    // first try to load it with a plugin
    //for (unsigned int i = 0; i<plugins.size(); ++i)
    //  if (plugins[i]->save(mesh_file_name_string))
    //    return true;

    size_t last_dot = mesh_file_name_string.rfind('.');
    if (last_dot == std::string::npos)
    {
      // No file type determined
      std::cerr<<"Error: No file extension found in "<<
        mesh_file_name_string<<std::endl;
      return false;
    }
    std::string extension = mesh_file_name_string.substr(last_dot+1);
    if (extension == "off" || extension =="OFF")
    {
      return igl::writeOFF(
        mesh_file_name_string,data()->V,data()->F);
    }
    else if (extension == "obj" || extension =="OBJ")
    {
      Eigen::MatrixXd corner_normals;
      Eigen::MatrixXi fNormIndices;

      Eigen::MatrixXd UV_V;
      Eigen::MatrixXi UV_F;

      return igl::writeOBJ(mesh_file_name_string,
          data()->V,
          data()->F,
          corner_normals, fNormIndices, UV_V, UV_F);
    }
    else
    {
      // unrecognized file type
      printf("Error: %s is not a recognized file type.\n",extension.c_str());
      return false;
    }
    return true;
  }
 
  IGL_INLINE bool Viewer::load_scene()
  {
    std::string fname = igl::file_dialog_open();
    if(fname.length() == 0)
      return false;
    return load_scene(fname);
  }

  IGL_INLINE bool Viewer::load_scene(std::string fname)
  {
   // igl::deserialize(core(),"Core",fname.c_str());
    igl::deserialize(*data(),"Data",fname.c_str());
    return true;
  }

  IGL_INLINE bool Viewer::save_scene()
  {
    std::string fname = igl::file_dialog_save();
    if (fname.length() == 0)
      return false;
    return save_scene(fname);
  }

  IGL_INLINE bool Viewer::save_scene(std::string fname)
  {
    //igl::serialize(core(),"Core",fname.c_str(),true);
    igl::serialize(data(),"Data",fname.c_str());

    return true;
  }

  IGL_INLINE void Viewer::open_dialog_load_mesh()
  {
    std::string fname = igl::file_dialog_open();

    if (fname.length() == 0)
      return;
    
    this->load_mesh_from_file(fname);
  }

  IGL_INLINE void Viewer::open_dialog_save_mesh()
  {
    std::string fname = igl::file_dialog_save();

    if(fname.length() == 0)
      return;

    this->save_mesh_to_file(fname.c_str());
  }

  IGL_INLINE ViewerData* Viewer::data(int mesh_id /*= -1*/)
  {
    assert(!data_list.empty() && "data_list should never be empty");
    int index;
    if (mesh_id == -1)
      index = selected_data_index;
    else
      index = mesh_index(mesh_id);

    assert((index >= 0 && index < data_list.size()) &&
      "selected_data_index or mesh_id should be in bounds");
    return data_list[index];
  }

  IGL_INLINE const ViewerData* Viewer::data(int mesh_id /*= -1*/) const
  {
    assert(!data_list.empty() && "data_list should never be empty");
    int index;
    if (mesh_id == -1)
      index = selected_data_index;
    else
      index = mesh_index(mesh_id);

    assert((index >= 0 && index < data_list.size()) &&
      "selected_data_index or mesh_id should be in bounds");
    return data_list[index];
  }

  IGL_INLINE int Viewer::append_mesh(const ViewerDataCreateFunc dataCreator)
  {
      assert(data_list.size() >= 1);
      data_list.emplace_back(dataCreator());
      selected_data_index = data_list.size() - 1;
      data_list.back()->id = next_data_id++;
      return data_list.back()->id;
  }

  IGL_INLINE bool Viewer::erase_mesh(const size_t index)
  {
    assert((index >= 0 && index < data_list.size()) && "index should be in bounds");
    assert(data_list.size() >= 1);
    if(data_list.size() == 1)
    {
      // Cannot remove last mesh
      return false;
    }
    data_list[index]->meshgl.free();
    data_list.erase(data_list.begin() + index);
    if(selected_data_index >= index && selected_data_index > 0)
    {
      selected_data_index--;
    }

    return true;
  }

  IGL_INLINE size_t Viewer::mesh_index(const int id) const {
    for (size_t i = 0; i < data_list.size(); ++i)
    {
      if (data_list[i]->id == id)
        return i;
    }
    return 0;
  }

  Eigen::Matrix4d Viewer::CalcParentsTrans(int indx) const
  {
	  Eigen::Matrix4d prevTrans = Eigen::Matrix4d::Identity();

	  for (int i = indx; parents[i] >= 0; i = parents[i])
	  {
		  prevTrans = data_list[parents[i]]->MakeTransd() * prevTrans;
	  }

	  return prevTrans;
  }


    IGL_INLINE void Viewer::Draw
    (
        int shaderIndx, const Eigen::Matrix4f &Proj, const Eigen::Matrix4f &View,
        int sectionIndex, int layerIndex,
        Eigen::Vector3d cameraPosition,
        unsigned int flgs, unsigned int property_id
    )
    {
        Eigen::Matrix4f Normal;
        if (!staticScene)
            Normal = MakeTransScale();
        else
            Normal = Eigen::Matrix4f::Identity();

        std::vector<size_t> opaqueDataIndices;
        std::map<double, size_t, std::greater<double>> sortedTransperantDataIndices;
        for (size_t i = 0; i < data_list.size(); ++i)
        {
            ViewerData *shape = data_list[i];
            if (GetShapeAlpha(i) == 1.0f)
            {
                opaqueDataIndices.push_back(i);
            }
            else
            {
                double distanceSq = (cameraPosition - GetMeshPosition(i)).squaredNorm();
                sortedTransperantDataIndices[distanceSq] = i;
            }
        }
        for (size_t shapeIndex : opaqueDataIndices)
        {
            DrawShape
            (
                shapeIndex, shaderIndx,
                Normal, Proj, View,
                sectionIndex, layerIndex,
                flgs, property_id
            );
        }
        for (const auto& shapeIndexKeyPair : sortedTransperantDataIndices)
        {
            size_t shapeIndex = shapeIndexKeyPair.second;
            DrawShape
            (
                shapeIndex, shaderIndx,
                Normal, Proj, View,
                sectionIndex, layerIndex,
                flgs, property_id
            );
        }
    }

    int Viewer::AddShader(const std::string &fileName) {
        shaders.push_back(new Shader(fileName,next_data_id));
        next_data_id +=1;
        return (shaders.size() - 1);
    }

    int Viewer::AddShader(const std::string &Vertex_Shader,const std::string &Fragment_shader ) {
        shaders.push_back(new Shader(Vertex_Shader,Fragment_shader, false,next_data_id));
        next_data_id +=1;
        return (shaders.size() - 1);
    }

    void Viewer::SetShader_overlay(const std::string &fileName) {
         overlay_shader = new Shader(fileName,true,next_data_id);
        next_data_id +=1;
    }

    void Viewer::SetShader_point_overlay(const std::string &fileName) {
        overlay_point_shader = new Shader(fileName,true,next_data_id);
        next_data_id +=1;
    }

    int Viewer::InitSelectedShape(int type, int parent, unsigned int mode, int shaderIndex, const std::vector<std::pair<int, int>> &sectionLayers)
    {
        data()->type = type;
        data()->mode = mode;
        data()->SetShader(shaderIndex);
        data()->AddSectionLayers(sectionLayers);
        data()->show_lines = 0;
        data()->show_overlay = 0;
        data()->hide = false;
        this->parents.emplace_back(parent);
        return data_list.size() - 1;
    }

    int Viewer::AddShapeFromFile
    (
        const std::string &fileName,
        int parent, unsigned int mode, int shaderIndex,
        const std::vector<std::pair<int, int>> &sectionLayers,
        const ViewerDataCreateFunc dataCreator
    )
    {
        this->load_mesh_from_file(fileName, dataCreator);
        return InitSelectedShape(MeshCopy, parent, mode, shaderIndex, sectionLayers);
    }

    int Viewer::AddShape(int type, int parent, unsigned int mode, int shaderIndex, const std::vector<std::pair<int, int>> &sectionLayers, const ViewerDataCreateFunc dataCreator)
    {
      switch(type){
// Axis, Plane, Cube, Octahedron, Tethrahedron, LineCopy, MeshCopy
          case Plane:
              this->load_mesh_from_file("./data/plane.obj", dataCreator);
              break;
          case Cube:
         case Axis:
              this->load_mesh_from_file("./data/cube.obj", dataCreator);
              break;
          case Octahedron:
              this->load_mesh_from_file("./data/octahedron.obj", dataCreator);
              break;
          case Tethrahedron:
              this->load_mesh_from_file("./data/Tetrahedron.obj", dataCreator);
              break;
          case Sphere:
              this->load_mesh_from_file("./data/sphere.obj", dataCreator);
              break;
          case xCylinder:
              this->load_mesh_from_file("./data/xcylinder.obj", dataCreator);
              break;
          case yCylinder:
              this->load_mesh_from_file("./data/ycylinder.obj", dataCreator);
              break;
          case zCylinder:
              this->load_mesh_from_file("./data/zcylinder.obj", dataCreator);
              break;
          default:
              break;
      }

      int shapeIndex = InitSelectedShape(type, parent, mode, shaderIndex, sectionLayers);

      if(type == Axis) {
          data()->show_faces = 0;
          data()->show_lines = 0;
          data()->show_overlay = unsigned(~0);
          data()->add_edges((Eigen::RowVector3d::UnitX()*4),-(Eigen::RowVector3d::UnitX()*4),Eigen::RowVector3d(1,0,0));
          data()->add_edges((Eigen::RowVector3d::UnitY()*4),-(Eigen::RowVector3d::UnitY()*4),Eigen::RowVector3d(0,1,0));
          data()->add_edges((Eigen::RowVector3d::UnitZ()*4),-(Eigen::RowVector3d::UnitZ()*4),Eigen::RowVector3d(0,0,1));
      }

      return shapeIndex;
    }

    int Viewer::AddShapeCopy
    (
        int shpIndx,
        int parent, unsigned int mode,
        const std::vector<std::pair<int, int>> &sectionLayers,
        const ViewerDataCreateFunc dataCreator
    )
    {
        const ViewerData &shape = *data_list[shpIndx];
        return AddShapeFromData
        (
            shape.V, shape.F,
            shape.V_uv, shape.F_uv,
            shape.type, parent, mode, shape.GetShader(),
            sectionLayers, dataCreator
        );
    }

    int Viewer::AddShapeFromData
    (
        const Eigen::MatrixXd &V, const Eigen::MatrixXi &F,
        const Eigen::MatrixXd &UV_V, const Eigen::MatrixXi &UV_F,
        int type, int parent, unsigned int mode, int shaderIndex,
        const std::vector<std::pair<int, int>> &sectionLayers,
        const ViewerDataCreateFunc dataCreator
    )
    {
        load_mesh_from_data(V, F, UV_V, UV_F, dataCreator);
        return InitSelectedShape(type, parent, mode, shaderIndex, sectionLayers);
    }

    void Viewer::ClearPickedShapes(const std::vector<std::pair<int, int>> &stencilLayers)
    {
        for (int pShape : pShapes)
        {
            data_list[pShape]->RemoveSectionLayers(stencilLayers);
        }
        selected_data_index = 0;
        pShapes.clear();
    }

    Eigen::Matrix4d Viewer::CalculatePosMatrix(int shapeIndex, const Eigen::Matrix4d &MVP)
    {
        const ViewerData &mesh = GetViewerDataAt(shapeIndex);
        Eigen::Matrix4d Model = MakeMeshTransScaled(shapeIndex);
        Model = CalcParentsTrans(shapeIndex) * Model;
        Eigen::Matrix4d posMatrix = MVP * Model;
        return posMatrix;
    }

    double Viewer::CalculateDepthOfMesh(const ViewerData &mesh, const Eigen::Matrix4d &posMatrix) const
    {
        double minDepth = std::numeric_limits<double>::infinity();
        for (size_t vi = 0; vi < mesh.V.rows(); ++vi)
        {
            Eigen::Vector3d vertex = mesh.V.row(vi).head<3>();
            Eigen::Vector4d transformed = posMatrix * vertex.homogeneous();
            double depth = transformed.z();
            if (minDepth > depth)
            {
                minDepth = depth;
            }
        }
        return minDepth;
    }

    void Viewer::AppendDepthsOfPicked(std::vector<double> &depths, const Eigen::Matrix4d &MVP)
    {
        for (int pShape : pShapes)
        {
            Eigen::Matrix4d posMatrix = CalculatePosMatrix(pShape, MVP);
            depths.push_back(CalculateDepthOfMesh(GetViewerDataAt(pShape), posMatrix));
        }
    }

    Eigen::Matrix4d Viewer::GetTransformationMatrix(int dataIndex) const
    {
        Eigen::Matrix4d scnMat = Eigen::Matrix4d::Identity();
        if (!staticScene)
        {
            scnMat = MakeTransd();
        }
        return scnMat;
    }

    //return coordinates in global system for a tip of arm position is local system
    void Viewer::MouseProccessing
    (
        int button,
        int xrel,
        int yrel,
        const igl::opengl::Camera &camera,
        int cameraIndex,
        int viewpoertSize,
        const std::vector<double> &depths
    )
    {
        Eigen::Matrix4d cameraMat = MakeCameraTransd(cameraIndex);

        // Changed: modified to support mesh transformations and multipicking
        Eigen::Matrix4d scnMat = GetTransformationMatrix();

        if (button == 1) // Right click
        {
            xrel *= -1;
            for (size_t i = 0; i < pShapes.size(); ++i)
            {
                Eigen::Matrix4d preMat = scnMat * cameraMat.inverse();
                int pShape = pShapes[i];
                double depth = depths[i];
                double movCoeff = camera.CalcMoveCoeff(depth, viewpoertSize);
                selected_data_index = pShape;
                WhenTranslate(preMat, xrel * movCoeff, yrel * movCoeff);
            }
            if (pShapes.size() == 0)
            {
                double cameraFar = camera.data.zFar, cameraNear = camera.data.zNear;
                double cameraDepth = cameraFar + 0.5f * (cameraNear - cameraFar);
                double movCoeff = 0.1 * camera.CalcMoveCoeff(cameraDepth, viewpoertSize);
                TranslateCamera(Eigen::Vector3d{ xrel * movCoeff, yrel * movCoeff, 0.0 });
            }
        }
        else
        {
            float movCoeff = 2.0f;
            if (button == 0) // Left click
            {
                Eigen::Matrix4d preMat = scnMat * cameraMat;
                double factor = movCoeff * EIGEN_PI / 8.0 / 180;
                float dx = xrel * factor;
                float dy = yrel * factor;
                for (int pShape : pShapes)
                {
                    selected_data_index = pShape;
                    WhenRotate(preMat, dx, dy);
                }
                if (pShapes.size() == 0)
                {
                    RotateCamera(dx, dy);
                }
            }
            else // Scrolling
            {
                double dy = -yrel * movCoeff;
                Eigen::Matrix4d preMat = scnMat * cameraMat.inverse();
                for (int pShape : pShapes)
                {
                    selected_data_index = pShape;
                    WhenScroll(preMat, dy);
                }
                if (pShapes.size() == 0)
                {
                    TranslateCamera(Eigen::Vector3d{ 0.0, 0.0, dy });
                }
            }
        }
    }

    void Viewer::TranslateCamera(const Movable &movable, const Eigen::Vector3d &d)
    {
        Eigen::Vector3d amt;
        std::vector<Eigen::Vector3d> amounts{};

        if (d.x() != 0)
        {
            movable.GetVectorInAxisDirection(amt, d.x(), 0);
            amounts.push_back(amt);
        }
        if (d.y() != 0)
        {
            movable.GetVectorInAxisDirection(amt, d.y(), 1);
            amounts.push_back(amt);
        }
        if (d.z() != 0)
        {
            movable.GetVectorInAxisDirection(amt, d.z(), 2);
            amounts.push_back(amt);
        }

        if (amounts.size() > 0)
        {
            MoveCamera([&amounts](Movable &movable)
            {
                for (const auto &amt : amounts)
                {
                    movable.MyTranslate(amt, true);
                }
            });
        }
    }

    void Viewer::RotateCamera(const std::vector<std::pair<Eigen::Vector3d, double>> &angledAxes)
    {
        if (angledAxes.size() > 0)
        {
            MoveCamera([&angledAxes](Movable &movable)
            {
                for (const auto &angledAxis : angledAxes)
                {
                    movable.MyRotate(angledAxis.first, angledAxis.second);
                }
            });
        }
    }

    void Viewer::RotateCamera(double dx, double dy)
    {
        RotateCamera
        ({
            std::pair<Eigen::Vector3d, double>{Eigen::Vector3d(0, 1, 0), dx},
            std::pair<Eigen::Vector3d, double>{Eigen::Vector3d(1, 0, 0), dy},
        });
    }

    void Viewer::WhenTranslate(const Eigen::Matrix4d& preMat, float dx, float dy)
    {
        Eigen::Matrix3d rot = preMat.block<3, 3>(0, 0);
        Transform(GetMovableTransformee(), [&rot, &dx, &dy](Movable &movable)
        {
            movable.TranslateInSystem(rot, Eigen::Vector3d(dx, 0, 0));
            movable.TranslateInSystem(rot, Eigen::Vector3d(0, dy, 0));
        });
    }

    void Viewer::WhenRotate(const Eigen::Matrix4d& preMat, float dx, float dy)
    {
        const Eigen::Matrix4d inverse = preMat.transpose();
        const Eigen::Vector4d &xRotation = inverse.col(1);
        const Eigen::Vector4d &yRotation = inverse.col(0);
        Transform(GetMovableTransformee(), [&xRotation, &dx, &yRotation, &dy](Movable &movable)
        {
            movable.RotateInSystem(xRotation.head(3), dx);
            movable.RotateInSystem(yRotation.head(3), dy);
        });
    }

    void Viewer::WhenScroll(const Eigen::Matrix4d& preMat, float dy)
    {
        Eigen::Matrix3d rot = preMat.block<3, 3>(0, 0);
        Transform(GetMovableTransformee(), [&rot, dy](Movable &movable)
        {
            movable.TranslateInSystem(rot, Eigen::Vector3d(0, 0, dy));
        });
    }

    Movable &Viewer::GetMovableTransformee(int shapeIndex)
    {
        if (shapeIndex > 0 && !data_list[shapeIndex]->IsStatic())
        {
            return *data_list[shapeIndex];
        }
        return *this;
    }

    void Viewer::Transform(Movable &movable, std::function<void(Movable &)> transform)
    {
        transform(movable);
    }

    void Viewer::ShapeTransformation(int type, float amt, int mode)
    {
        if (abs(amt) > 1e-5 && selected_data_index>=0 && !data()->IsStatic())
        {
            switch (type)
            {
                case xTranslate:
                    data()->MyTranslate(Eigen::Vector3d(amt, 0, 0), mode);
                    break;
                case yTranslate:
                    data()->MyTranslate(Eigen::Vector3d(0, amt, 0), mode);
                    break;
                case zTranslate:
                    data()->MyTranslate(Eigen::Vector3d(0, 0, amt), mode);
                    break;
                case xRotate:
                    data()->MyRotate(Eigen::Vector3d(1, 0, 0), amt, mode);
                    break;
                case yRotate:
                    data()->MyRotate(Eigen::Vector3d(0, 1, 0), amt, mode);
                    break;
                case zRotate:
                    data()->MyRotate(Eigen::Vector3d(0, 0, 1), amt, mode);
                    break;
                case xScale:
                    data()->MyScale(Eigen::Vector3d(amt, 1, 1));
                    break;
                case yScale:
                    data()->MyScale(Eigen::Vector3d(1, amt, 1));
                    break;
                case zScale:
                    data()->MyScale(Eigen::Vector3d(1, 1, amt));
                    break;
                case scaleAll:
                    data()->MyScale(Eigen::Vector3d(amt, amt, amt));
                    break;
                case reset:
                    data()->ZeroTrans();
                    break;
                default:
                    break;
            }
        }
    }

    int Viewer::AddMaterial(unsigned int texIndices[], unsigned int slots[], unsigned int size)
    {
        materials.push_back(new Material(texIndices, slots, size));
        return (materials.size() - 1);
    }

    Eigen::Matrix4d Viewer::GetPriviousTrans(const Eigen::Matrix4d& View, unsigned int index) const
    {
        Eigen::Matrix4d Model = Eigen::Matrix4d::Identity();
        int p = index >= 0 ? parents[index] : -1;
        for (; p >= 0; p = parents[p])
            Model = data_list[p]->MakeTransd() * Model;
        if (p == -2)
            return  View.inverse() * Model;
        else
            return Model;
    }

    bool Viewer::ShouldRenderViewerData(const ViewerData& data, const int sectionIndex, const int layerIndex, int shapeIndex) const
    {
        return !IsLayerHidden(data.layer) && data.Is2Render(sectionIndex, layerIndex);
    }

    Texture* Viewer::AddTexture_Core(const std::string& textureFileName, int dim)
    {
        Texture* texture = new Texture(textureFileName, dim);
        textures.push_back(texture);
        return texture;
    }

    int Viewer::AddTexture(const std::string& textureFileName, int dim)
    {
        bool c;
        std::string canonicalPath = CanonicalizePath(textureFileName, &c);
        auto fileTextures = textureCache.find(canonicalPath);
        if (fileTextures == textureCache.end())
        {
            textureCache.insert
            (
                std::pair<std::string, std::array<int, 3>>
                { canonicalPath, { -1, -1, -1 } }
            );
            fileTextures = textureCache.find(canonicalPath);
        }

        int textureIndex{ -1 };
        std::array<int, 3> &texturesArr = fileTextures->second;
        size_t dimIndex = static_cast<size_t>(dim) - 1;
        if (texturesArr[dimIndex] < 0)
        {
            textureIndex = textures.size();
            Texture* texture = AddTexture_Core(canonicalPath, dim);
            texturesArr[dimIndex] = textureIndex;
        }
        else
        {
            textureIndex = texturesArr[dimIndex];
        }

        return textureIndex;
    }

    void Viewer::BindMaterial(Shader* s, unsigned int materialIndx)
    {

        for (int i = 0; i < materials[materialIndx]->GetNumOfTexs(); i++)
        {
            materials[materialIndx]->Bind(textures, i);
            s->SetUniform1i("sampler" + std::to_string(i + 1), materials[materialIndx]->GetSlot(i));
        }
    }

    int Viewer::AddTexture(int width, int height, unsigned char* data, int mode)
    {
        textures.push_back(new Texture(width, height));

        if (mode)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, data);
        }
        else
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data); //note GL_RED internal format, to save memory.
        }
        glBindTexture(GL_TEXTURE_2D, 0);
        return(textures.size() -1);
    }

    void Viewer::Update_overlay(const Eigen::Matrix4f &Proj, const Eigen::Matrix4f &View, const Eigen::Matrix4f &Model, unsigned int shapeIndx,bool is_points) {
        auto data = data_list[shapeIndx];
        Shader * s = is_points ? overlay_point_shader : overlay_shader;
        if(s != nullptr) {
            s->Bind();
            s->SetUniformMat4f("Proj", Proj);
            s->SetUniformMat4f("View", View);
            s->SetUniformMat4f("Model", Model);
        }
    }


    void Viewer::SetParent(int indx, int newValue, bool savePosition)
    {
        parents[indx] = newValue;
        if (savePosition)
        {
            Eigen::Vector4d tmp = data_list[newValue]->MakeTransd() * (data_list[indx]->MakeTransd()).inverse() * Eigen::Vector4d(0,0,0,1);
            data_list[indx]->ZeroTrans();
            data_list[indx]->MyTranslate(-tmp.head<3>(), false);
        }
    }


} // end namespace
} // end namespace
}
