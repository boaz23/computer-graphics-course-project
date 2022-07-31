// This file is part of libigl, a simple c++ geometry processing library.
//
// Copyright (C) 2014 Daniele Panozzo <daniele.panozzo@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
#ifndef IGL_OPENGL_GLFW_VIEWER_H
#define IGL_OPENGL_GLFW_VIEWER_H

#ifndef IGL_OPENGL_4
#define IGL_OPENGL_4
#endif

#include "../../igl_inline.h"
#include "../MeshGL.h"

#include "../ViewerData.h"
#include "ViewerPlugin.h"
#include "igl/opengl/Movable.h"
#include "igl/opengl/Camera.h"
#include "igl/opengl/glfw/Material.h"


#include <Eigen/Core>
#include <Eigen/Geometry>

#include <array>
#include <vector>
#include <list>
#include <string>
#include <cstdint>
#include <unordered_set>

#define IGL_MOD_SHIFT           0x0001
#define IGL_MOD_CONTROL         0x0002
#define IGL_MOD_ALT             0x0004
#define IGL_MOD_SUPER           0x0008

class Renderer;

namespace igl
{
namespace opengl
{
namespace glfw
{
    using ViewerDataCreateFunc = std::function<ViewerData*()>;

  // GLFW-based mesh viewer
  class Viewer : public Movable
  {
  public:
      Renderer* renderer;
      std::vector<bool> layers;
      int currentEditingLayer;
      int currentObjectLayer;
      bool isEditingObjectLayer;

      enum axis { xAxis, yAxis, zAxis };
      enum transformations { xTranslate, yTranslate, zTranslate, xRotate, yRotate, zRotate, xScale, yScale, zScale,scaleAll,reset };
      enum modes { POINTS, LINES, LINE_LOOP, LINE_STRIP, TRIANGLES, TRIANGLE_STRIP, TRIANGLE_FAN, QUADS };
      enum shapes {Axis, xCylinder,yCylinder,zCylinder, Plane, Cube, Octahedron, Tethrahedron, LineCopy, MeshCopy, Sphere, OtherShape };
      enum buffers { COLOR, DEPTH, STENCIL, BACK, FRONT, NONE };
    // UI Enumerations
   // enum class MouseButton {Left, Middle, Right};
   // enum class MouseMode { None, Rotation, Zoom, Pan, Translation} mouse_mode;
    virtual void Init(const std::string config);
	virtual void Animate() {}
	virtual void WhenTranslate() {}

    IGL_INLINE ViewerData* DefualtViewerDataCreator() { return new ViewerData(currentEditingLayer); }

	//IGL_INLINE void init_plugins();
    //IGL_INLINE void shutdown_plugins();
    Viewer();
    virtual ~Viewer();
      virtual void Update(const Eigen::Matrix4f& Proj, const Eigen::Matrix4f& View, const Eigen::Matrix4f& Model, unsigned int  shaderIndx, unsigned int shapeIndx){};
      virtual void Update_overlay(const Eigen::Matrix4f& Proj, const Eigen::Matrix4f& View, const Eigen::Matrix4f& Model, unsigned int shapeIndx,bool is_points);

      IGL_INLINE bool IsLayerHidden(int layer) const { return layers[layer]; }
      IGL_INLINE size_t LayersCount() const { return layers.size(); }
      void ToggleLayerVisibility(int layer);
      int AddLayer();

      void ChangeCubemapImage(std::string filePath);
  protected:
      virtual int Viewer::InitSelectedShape(int type, int parent, unsigned int mode, int shaderIndex, const std::vector<std::pair<int, int>> &sectionLayers);
  public:
      virtual int AddShape
      (
          int type, int parent, unsigned int mode, int shaderIndex,
          const std::vector<std::pair<int, int>> &sectionLayers,
          const ViewerDataCreateFunc dataCreator
      );
      IGL_INLINE int AddShape(int type, int parent, unsigned int mode, int shaderIndex, const std::vector<std::pair<int, int>> &sectionLayers)
      {
          return AddShape(type, parent, mode, shaderIndex, sectionLayers, std::bind(&Viewer::DefualtViewerDataCreator, this));
      }
      virtual int AddShapeFromFile
      (
          const std::string& fileName,
          int parent, unsigned int mode, int shaderIndex,
          const std::vector<std::pair<int, int>> &sectionLayers,
          const ViewerDataCreateFunc dataCreator
      );
      IGL_INLINE int AddShapeFromFile(const std::string& fileName, int parent, unsigned int mode, int shaderIndex, const std::vector<std::pair<int, int>> &sectionLayers)
      {
          return AddShapeFromFile(fileName, parent, mode, shaderIndex, sectionLayers, std::bind(&Viewer::DefualtViewerDataCreator, this));
      }

      int AddShapeFromData(const Eigen::MatrixXd &V, const Eigen::MatrixXi &F, const Eigen::MatrixXd &UV_V,
          const Eigen::MatrixXi &UV_F, int type, int parent, unsigned int mode, int shaderIndex, const std::vector<std::pair<int, int>> &sectionLayers, const ViewerDataCreateFunc dataCreator);
      int AddShapeCopy(int shpIndx, int parent, unsigned int mode, const std::vector<std::pair<int, int>> &sectionLayers, const ViewerDataCreateFunc dataCreator);
    // Mesh IO
    IGL_INLINE bool load_mesh_from_file(const std::string & mesh_file_name, const ViewerDataCreateFunc dataCreator);
    IGL_INLINE bool load_mesh_from_file(const std::string& mesh_file_name)
    {
        return load_mesh_from_file(mesh_file_name, std::bind(&Viewer::DefualtViewerDataCreator, this));
    }
    bool load_mesh_from_data(const Eigen::MatrixXd &V, const Eigen::MatrixXi &F, const Eigen::MatrixXd &UV_V,
        const Eigen::MatrixXi &UV_F, const ViewerDataCreateFunc dataCreator);
    IGL_INLINE bool save_mesh_to_file(const std::string & mesh_file_name);
   
    // Scene IO
    IGL_INLINE bool load_scene();
    IGL_INLINE bool load_scene(std::string fname);
    IGL_INLINE bool save_scene();
    IGL_INLINE bool save_scene(std::string fname);
    // Draw everything
   // IGL_INLINE void draw();
    // OpenGL context resize
   
    // Helper functions

    IGL_INLINE void open_dialog_load_mesh();
    IGL_INLINE void open_dialog_save_mesh();

	IGL_INLINE void draw() {}
    ////////////////////////
    // Multi-mesh methods //
    ////////////////////////

    // Return the current mesh, or the mesh corresponding to a given unique identifier
    //
    // Inputs:
    //   mesh_id  unique identifier associated to the desired mesh (current mesh if -1)
    IGL_INLINE ViewerData* data(int mesh_id = -1);
    IGL_INLINE const ViewerData* data(int mesh_id = -1) const;
    IGL_INLINE ViewerData& GetViewerDataAt(int index) { return *data_list[index]; }
    IGL_INLINE const ViewerData& GetViewerDataAt(int index) const { return *data_list[index]; }

    // Append a new "slot" for a mesh (i.e., create empty entries at the end of
    // the data_list and opengl_state_list.
    //
    // Inputs:
    //   visible  If true, the new mesh is set to be visible on all existing viewports
    // Returns the id of the last appended mesh
    //
    // Side Effects:
    //   selected_data_index is set this newly created, last entry (i.e.,
    //   #meshes-1)
    IGL_INLINE int append_mesh(const ViewerDataCreateFunc dataCreator);

    // Erase a mesh (i.e., its corresponding data and state entires in data_list
    // and opengl_state_list)
    //
    // Inputs:
    //   index  index of mesh to erase
    // Returns whether erasure was successful <=> cannot erase last mesh
    //
    // Side Effects:
    //   If selected_data_index is greater than or equal to index then it is
    //   decremented
    // Example:
    //   // Erase all mesh slots except first and clear remaining mesh
    //   viewer.selected_data_index = viewer.data_list.size()-1;
    //   while(viewer.erase_mesh(viewer.selected_data_index)){};
    //   viewer.data().clear();
    //
    IGL_INLINE bool erase_mesh(const size_t index);

    // Retrieve mesh index from its unique identifier
    // Returns 0 if not found
    IGL_INLINE size_t mesh_index(const int id) const;

	Eigen::Matrix4d CalcParentsTrans(int indx) const;
	inline bool SetAnimation() { return isActive = !isActive; }
    inline  bool  IsActive() const { return isActive; }
    inline void Activate() { isActive = true; }
    inline void Deactivate() { isActive = false; }
    int AddShader(const std::string& fileName);
public:
    //////////////////////
    // Member variables //
    //////////////////////

    // Alec: I call this data_list instead of just data to avoid confusion with
    // old "data" variable.
    // Stores all the data that should be visualized
    std::vector<ViewerData*> data_list;
    std::vector<int> pShapes;
	std::vector<int> parents;
    std::vector<Texture*> textures;
    std::vector<Material*> materials;
    Eigen::Vector3d pickedNormal;
    int selected_data_index;
    int next_data_id;
    int next_shader_id; // for flags to mack sure all shaders are initlize with data
	bool isActive;
    unsigned int staticScene;

    Shader* overlay_shader;
    Shader* overlay_point_shader;
    std::vector<Shader*> shaders;

    

    // List of registered plugins
//    std::vector<ViewerPlugin*> plugins;

    // Keep track of the global position of the scrollwheel
    float scroll_position;

  protected:
      int materialIndex_cube;
      std::unordered_map<std::string, std::array<int, 3>> textureCache;

  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

      void Draw
      (
          int shaderIndx, const Eigen::Matrix4f &Proj, const Eigen::Matrix4f &View,
          int sectionIndex, int layerIndex,
          Eigen::Vector3d cameraPosition,
          unsigned int flgs,unsigned int property_id
      );
      virtual void Viewer::DrawShape
      (
          size_t shapeIndex, int shaderIndx,
          const Eigen::Matrix4f &Normal, const Eigen::Matrix4f &Proj, const Eigen::Matrix4f &View,
          int sectionIndex, int layerIndex,
          unsigned int flgs, unsigned int property_id
      ) = 0;


      virtual void ClearPickedShapes(const std::vector<std::pair<int, int>> &stencilLayers);

      int AddMaterial(unsigned int *texIndices, unsigned int *slots, unsigned int size);

      Eigen::Matrix4d GetPriviousTrans(const Eigen::Matrix4d &View, unsigned int index) const;

      IGL_INLINE Eigen::Matrix4d CalculatePosMatrix(int shapeIndex, const Eigen::Matrix4d &MVP);

      virtual bool AddPickedShapes(const Eigen::Matrix4d& PV, const Eigen::Vector4i& viewport, int sectionIndex, int layerIndex, int left, int right,
          int up, int bottom, const std::vector<std::pair<int, int>>& stencilLayers, std::vector<double> &depths) = 0;

      double CalculateDepthOfMesh(const ViewerData &mesh, const Eigen::Matrix4d &posMatrix) const;
      void AppendDepthsOfPicked(std::vector<double> &depths, const Eigen::Matrix4d &MVP);

      template<typename T> bool AllPickedShapesSameValue(std::function<T(const ViewerData&)> valueFunc) const
      {
          if (pShapes.size() <= 1)
          {
              return true;
          }

          T prevValue = valueFunc(GetViewerDataAt(pShapes[0]));
          for (auto &it = pShapes.begin() + 1; it < pShapes.end(); ++it)
          {
              T value = valueFunc(GetViewerDataAt(*it));
              if (prevValue != value)
              {
                  return false;
              }
              prevValue = value;
          }

          return true;
      }

      void MouseProccessing
      (
          int button,
          int xrel,
          int yrel,
          const igl::opengl::Camera &camera,
          int cameraIndex,
          int viewpoertSize,
          const std::vector<double> &depths
      );

      virtual void WhenTranslate(const Eigen::Matrix4d &preMat, float dx, float dy);
      virtual void WhenScroll(const Eigen::Matrix4d &preMat, float dy);
      virtual void WhenRotate(const Eigen::Matrix4d &preMat, float dx, float dy);

    Movable &GetMovableTransformee(int shapeIndex);
    IGL_INLINE Movable &GetMovableTransformee()
    {
        return GetMovableTransformee(selected_data_index);
    }

    Eigen::Matrix4d GetTransformationMatrix(int dataIndex) const;
    IGL_INLINE Eigen::Matrix4d GetTransformationMatrix() const
    {
        return GetTransformationMatrix(selected_data_index);
    }

protected:
      virtual void Transform(Movable &movable, std::function<void(Movable &)> transform);

public:
      int AddTexture(const std::string& textureFileName, int dim);
      int AddTexture(int width, int height, unsigned char* data, int mode);
      void BindMaterial(Shader* s, unsigned int materialIndx);
      void BindTexture(int texIndx, int slot) { textures[texIndx]->Bind(slot); }
      IGL_INLINE void SetShapeShader(int shpIndx, int shdrIndx) { data_list[shpIndx]->SetShader(shdrIndx); }
      IGL_INLINE void SetShapeStatic(int shpIndx) { data_list[shpIndx]->SetStatic(); }
      IGL_INLINE void AddShapeSectionLayers(int shpIndx, const std::vector<std::pair<int, int>> &sectionLayers) { data_list[shpIndx]->AddSectionLayers(sectionLayers); }
      IGL_INLINE void RemoveShapeSectionLayers(int shpIndx, const std::vector<std::pair<int, int>> &sectionLayers) { data_list[shpIndx]->RemoveSectionLayers(sectionLayers); }
      inline void UpdateNormal(unsigned char data[]) { pickedNormal = (Eigen::Vector3d(data[0], data[1], data[2])).normalized(); }
      IGL_INLINE void SetShapeMaterial(int shpIndx, int materialIndx) { data_list[shpIndx]->SetMaterial(materialIndx); }

      void SetShader_overlay(const std::string &fileName);

      void SetShader_point_overlay(const std::string &fileName);

      void ShapeTransformation(int type, float amt, int mode);

      virtual float Picking(const Eigen::Matrix4d& PV, const Eigen::Vector4i& viewportDims, int sectionIndex, int layerIndex, const std::vector<std::pair<int, int>> &stencilLayers, int x, int y) = 0;
      //inline void UnPick() { selected_data_index = -1; pickedShapes.clear(); }

      int AddShader(const std::string &Vertex_Shader, const std::string &Fragment_shader);

      void SetParent(int indx, int newValue, bool savePosition);

      virtual int GetPickingShaderIndex() {
          return -1;
      };

      virtual void MoveCamera(std::function<void(Movable&)> transform) {};

      virtual void TranslateCamera(double dx, double dy, double dz) = 0;
      virtual void RotateCamera(double dx, double dy) {};

      virtual double GetShapeAlpha(int index) = 0;
      virtual bool ShouldRenderViewerData(const ViewerData& data, const int sectionIndex, const int layerIndex, int meshIndex) const = 0;
      Texture* Viewer::AddTexture_Core(const std::string& textureFileName, int dim);
      virtual Eigen::Matrix4d MakeCameraTransScaled(int cameraIndex) = 0;
      virtual Eigen::Matrix4d MakeCameraTransd(int cameraIndex) = 0;
      virtual Eigen::Matrix4f MakeCameraTransScale(int cameraIndex) = 0;
      virtual Eigen::Matrix4f MakeCameraTrans(int cameraIndex) = 0;
      virtual Eigen::Vector3d GetCameraPosition(int cameraIndex) = 0;
      virtual Eigen::Matrix3d GetCameraLinear(int cameraIndex) = 0;
      virtual Eigen::Matrix4d MakeMeshTransScaled(int meshIndex) = 0;
      virtual Eigen::Matrix4d MakeMeshTransd(int meshIndex) = 0;
      virtual Eigen::Matrix4f MakeMeshTransScale(int meshIndex) = 0;
      virtual Eigen::Matrix4f MakeMeshTrans(int meshIndex) = 0;
      virtual Eigen::Vector3d GetMeshPosition(int meshIndex) = 0;
      virtual Eigen::Matrix3d GetMeshLinear(int meshIndex) = 0;

  };

} // end namespace
} // end namespace
} // end namespace

#ifndef IGL_STATIC_LIBRARY
#  include "Viewer.cpp"
#endif

#endif
