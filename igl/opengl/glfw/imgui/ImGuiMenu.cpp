// This file is part of libigl, a simple c++ geometry processing library.
//
// Copyright (C) 2018 Jérémie Dumas <jeremie.dumas@ens-lyon.org>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
////////////////////////////////////////////////////////////////////////////////
//#include "ImGuiMenu.h"
//#include "ImGuiHelpers.h"
#include <igl/project.h>
#include "ImGuiHelpers.h"
#include "igl/opengl/glfw/renderer.h"

#include "ImGuiMenu.h"
#include "igl/opengl/glfw/imgui/imgui_impl_glfw.h"
#include "igl/opengl/glfw/imgui/imgui_impl_opengl3.h"

#include "tutorial/Project/Project.h"

#include "igl/file_dialog_open.h"

//#include <imgui_fonts_droid_sans.h>
//#include <GLFW/glfw3.h>
#include <iostream>
////////////////////////////////////////////////////////////////////////////////

namespace igl
{
namespace opengl
{
namespace glfw
{
namespace imgui
{

IGL_INLINE void ImGuiMenu::init(Display* disp)
{
  // Setup ImGui binding
  if (disp->window)
  {
    IMGUI_CHECKVERSION();
    if (!context_)
    {
      // Single global context by default, but can be overridden by the user
      static ImGuiContext * __global_context = ImGui::CreateContext();
      context_ = __global_context;
    }
    const char* glsl_version = "#version 150";

    ImGui_ImplGlfw_InitForOpenGL(disp->window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    ImGui::GetIO().IniFilename = nullptr;
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameRounding = 5.0f;
    reload_font();

    _trans_slidebar_val = 1.0f;
    _is_multipicking = false;
  }
}

IGL_INLINE void ImGuiMenu::reload_font(int font_size)
{
  hidpi_scaling_ = hidpi_scaling();
  pixel_ratio_ = pixel_ratio();
  ImGuiIO& io = ImGui::GetIO();
  io.Fonts->Clear();
 // io.Fonts->AddFontFromMemoryCompressedTTF(droid_sans_compressed_data,
 //   droid_sans_compressed_size, font_size * hidpi_scaling_);
  io.FontGlobalScale = 1.0 / pixel_ratio_;
}

IGL_INLINE void ImGuiMenu::shutdown()
{
  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  // User is responsible for destroying context if a custom context is given
  // ImGui::DestroyContext(*context_);
}

IGL_INLINE bool ImGuiMenu::pre_draw()
{
  glfwPollEvents();

  // Check whether window dpi has changed
  float scaling = hidpi_scaling();
  if (std::abs(scaling - hidpi_scaling_) > 1e-5)
  {
    reload_font();
    ImGui_ImplOpenGL3_DestroyDeviceObjects();
  }

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  return false;
}

IGL_INLINE bool ImGuiMenu::post_draw()
{
  //draw_menu(viewer,core);
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  return false;
}

IGL_INLINE void ImGuiMenu::post_resize(int width, int height)
{
  if (context_)
  {
    ImGui::GetIO().DisplaySize.x = float(width);
    ImGui::GetIO().DisplaySize.y = float(height);
  }
}

// Mouse IO
IGL_INLINE bool ImGuiMenu::mouse_down(GLFWwindow* window, int button, int modifier)
{
  ImGui_ImplGlfw_MouseButtonCallback(window, button, GLFW_PRESS, modifier);
  return ImGui::GetIO().WantCaptureMouse;
}

IGL_INLINE bool ImGuiMenu::mouse_up(GLFWwindow* window,int button, int modifier)
{
  //return ImGui::GetIO().WantCaptureMouse;
  // !! Should not steal mouse up
  return false;
}

IGL_INLINE bool ImGuiMenu::mouse_move(GLFWwindow* window,int mouse_x, int mouse_y)
{
  return ImGui::GetIO().WantCaptureMouse;
}

IGL_INLINE bool ImGuiMenu::mouse_scroll(GLFWwindow* window,float delta_y)
{
  ImGui_ImplGlfw_ScrollCallback(window, 0.f, delta_y);
  return ImGui::GetIO().WantCaptureMouse;
}

// Keyboard IO
IGL_INLINE bool ImGuiMenu::key_pressed(GLFWwindow* window,unsigned int key, int modifiers)
{
  ImGui_ImplGlfw_CharCallback(nullptr, key);
  return ImGui::GetIO().WantCaptureKeyboard;
}

IGL_INLINE bool ImGuiMenu::key_down(GLFWwindow* window, int key, int modifiers)
{
  ImGui_ImplGlfw_KeyCallback(window, key, 0, GLFW_PRESS, modifiers);
  return ImGui::GetIO().WantCaptureKeyboard;
}

IGL_INLINE bool ImGuiMenu::key_up(GLFWwindow* window,int key, int modifiers)
{
  ImGui_ImplGlfw_KeyCallback(window, key, 0, GLFW_RELEASE, modifiers);
  return ImGui::GetIO().WantCaptureKeyboard;
}



IGL_INLINE void ImGuiMenu::draw_viewer_menu(Renderer *rndr, igl::opengl::glfw::Viewer &viewer, std::vector<igl::opengl::Camera*> &camera, igl::opengl::CameraData cameraData, Eigen::Vector4i& viewWindow,std::vector<DrawInfo *> drawInfos)
{
    bool* p_open = NULL;
    static bool no_titlebar = false;
    static bool no_scrollbar = false;
    static bool no_menu = true;
    static bool no_move = false;
    static bool no_resize = false;
    static bool no_collapse = false;
    static bool no_close = false;
    static bool no_nav = false;
    static bool no_background = false;
    static bool no_bring_to_front = false;

    ImGuiWindowFlags window_flags = 0;
    if (no_titlebar)        window_flags |= ImGuiWindowFlags_NoTitleBar;
    if (no_scrollbar)       window_flags |= ImGuiWindowFlags_NoScrollbar;
    if (!no_menu)           window_flags |= ImGuiWindowFlags_MenuBar;
    if (no_move)            window_flags |= ImGuiWindowFlags_NoMove;
    if (no_resize)          window_flags |= ImGuiWindowFlags_NoResize;
    if (no_collapse)        window_flags |= ImGuiWindowFlags_NoCollapse;
    if (no_nav)             window_flags |= ImGuiWindowFlags_NoNav;
    if (no_background)      window_flags |= ImGuiWindowFlags_NoBackground;
    if (no_bring_to_front)  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::Begin(
        "Viewer", p_open,
        window_flags
    );

    ImGui::SetWindowPos(ImVec2((float)0, (float)0), ImGuiCond_Always);
    ImGui::SetWindowSize(ImVec2((float)0, (float)0), ImGuiCond_Always);
    ImGui::End();
    no_move = true;
    no_resize = true;
    ImGui::Begin(
        "Viewer", p_open,
        window_flags
    );

    float w = ImGui::GetContentRegionAvailWidth();
    float p = ImGui::GetStyle().FramePadding.x;
    ImVec2 halfWidthVec2((w - p) / 2.f, 0);
    ImVec2 fullWidthVec2(-1, 0);

  Project* project = dynamic_cast<Project*>(&viewer);
  if (project && ImGui::CollapsingHeader("Cameras", ImGuiTreeNodeFlags_DefaultOpen))
  {
    if (ImGui::Button("Add camera", fullWidthVec2))
    {
        project->AddCamera(Eigen::Vector3d(0, 0, 0), cameraData, CameraKind::Animation);
    }

    if (project->isInDesignMode && ImGui::Checkbox("Design mode", &project->isDesignModeView))
    {
        // Nothing to do for now
    }
  }

  // Mesh
  if (project && ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen))
  {
    if (project && ImGui::Button("Load##Mesh", fullWidthVec2))
    {
      std::string filePath = igl::file_dialog_open();
      project->AddShapeFromMenu(filePath);
    }
    //ImGui::SameLine(0, p);
    //if (ImGui::Button("Save##Mesh", halfWidthVec2))
    //{
    //  viewer.open_dialog_save_mesh();
    //}

    if (project)
    {
        ImGui::PushItemWidth(100 * menu_scaling());

        ImGui::Text("Material:");
        ImGui::SameLine(0, p);

        static int selectedMaterialComboIndex = 0;
        auto& availableMaterials = project->availableMaterials;

        bool allSameLayer = viewer.pShapes.size() == 0 ? false :
            viewer.AllPickedShapesSameValue<unsigned int>([](const igl::opengl::ViewerData& shape)
            {
                return shape.GetMaterial();
            });
        bool fieldChanged{ false };
        int materialComboIndex{ -1 };
        if (allSameLayer)
        {
            unsigned int commonMaterialIndex = viewer.GetViewerDataAt(viewer.pShapes[0]).GetMaterial();
            for (size_t i = 0; i < availableMaterials.size(); ++i)
            {
                const auto& materialPair = availableMaterials[i];
                if (materialPair.first == commonMaterialIndex)
                {
                    materialComboIndex = i;
                    break;
                }
            }

            if (selectedMaterialComboIndex != materialComboIndex)
            {
                selectedMaterialComboIndex = materialComboIndex;
                fieldChanged = true;
            }
        }

        const std::string& materialComboPreview = allSameLayer ? availableMaterials[materialComboIndex].second : "";
        if (ImGui::BeginCombo("##material", materialComboPreview.c_str()))
        {
            bool valueChanged{ false };
            for (size_t i = 0; i < availableMaterials.size(); ++i)
            {
                const bool item_selected = i == selectedMaterialComboIndex && allSameLayer;
                const char* item_text = availableMaterials[i].second.c_str();
                if (ImGui::Selectable(item_text, item_selected))
                {
                    valueChanged = true;
                    selectedMaterialComboIndex = i;
                }
                if (item_selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();

            if (valueChanged && !fieldChanged)
            {
                int materialIndex = availableMaterials[selectedMaterialComboIndex].first;
                for (int shapeIndex : viewer.pShapes)
                {
                    viewer.SetShapeMaterial(shapeIndex, materialIndex);
                }
            }
        }

        ImGui::PopItemWidth();
    }
  }

  if (ImGui::CollapsingHeader("Layers", ImGuiTreeNodeFlags_DefaultOpen))
  {
      ImGui::PushItemWidth(80 * menu_scaling());

      if (ImGui::Button("Add layer", fullWidthVec2))
      {
          viewer.AddLayer();
      }

      int maxLayer = viewer.LayersCount() - 1;
      const int step = 1, step_fast = 5;

      {
          ImGui::Text("Editing layer:");
          ImGui::SameLine(0, p);
          int& layerField = viewer.currentEditingLayer;
          int prevEditingLayer = layerField;
          if (ImGui::InputInt("##editingLayer", &layerField, step, step_fast))
          {
              if (layerField < 1 || layerField > maxLayer)
              {
                  layerField = prevEditingLayer;
              }
          }
          ImGui::SameLine(0, 0);
          ImGui::Text(" / %d", maxLayer);

          bool isCurrentLayerHidden = viewer.IsLayerHidden(layerField);

          ImGui::Text("State:");
          ImGui::SameLine(0, p);
          ImGui::Text(isCurrentLayerHidden ? "Hidden" : "Shown");

          if (ImGui::Button(isCurrentLayerHidden ? "Show layer" : "Hide layer", fullWidthVec2))
          {
              viewer.ToggleLayerVisibility(layerField);
          }
      }

      {
          ImGui::Text("Object%s layer:", viewer.pShapes.size() <= 1 ? "" : "s");
          ImGui::SameLine(0, p);
          int& layerField = viewer.currentObjectLayer;
          bool allSameLayer = viewer.pShapes.size() == 0 ? false :
              viewer.AllPickedShapesSameValue<int>([](const igl::opengl::ViewerData& shape)
              {
                  return shape.layer;
              });
          bool fieldChanged{ false };
          if (allSameLayer)
          {
              int commonLayer = viewer.GetViewerDataAt(viewer.pShapes[0]).layer;
              if (layerField != commonLayer)
              {
                  layerField = commonLayer;
                  fieldChanged = true;
              }
          }
          int prevObjectLayer = layerField;
          ImGuiInputTextCallback callback = [](ImGuiInputTextCallbackData* data)
          {
              auto viewer = static_cast<igl::opengl::glfw::Viewer*>(data->UserData);
              viewer->isEditingObjectLayer = true;
              return 0;
          };
          if
          (
              ImGui::InputScalar
              (
                "##objectLayer", ImGuiDataType_S32, (void*)&layerField,
                (void*)&step, (void*)&step_fast,
                (allSameLayer || viewer.isEditingObjectLayer) ? "%d" : "",
                ImGuiInputTextFlags_CallbackEdit,
                callback, &viewer
              ) && !fieldChanged
          )
          {
              viewer.isEditingObjectLayer = false;
               if (layerField < 1 || layerField > maxLayer)
               {
                   layerField = prevObjectLayer;
               }
               for (int shapeIndex : viewer.pShapes)
               {
                   igl::opengl::ViewerData& shape = viewer.GetViewerDataAt(shapeIndex);
                   shape.layer = layerField;
               }
          }
          ImGui::SameLine(0, 0);
          ImGui::Text(" / %d", maxLayer);
      }

      ImGui::PopItemWidth();
  }

  _slidebar_changed = ImGui::SliderFloat("Set Transperancy", &_trans_slidebar_val, 0.0, 1.0, _is_multipicking ? "" : "%.2f");


  // Viewing options
//  if (ImGui::CollapsingHeader("Viewing Options"))
//  {
//    if (ImGui::Button("Center object", ImVec2(-1, 0)))
//    {
//      std::cout << "not implemented yet" << std::endl;
////      core[1].align_camera_center(viewer.data().V, viewer.data().F); TODO: add function like this to camera
//    }
//    //if (ImGui::Button("Snap canonical view", ImVec2(-1, 0)))
//    //{
//    //  core[1].snap_to_canonical_quaternion();
//    //}
//
//    // Zoom
//    ImGui::PushItemWidth(80 * menu_scaling());
//    if (camera[0]->_ortho)
//      ImGui::DragFloat("Zoom", &(camera[0]->length), 0.05f, 0.1f, 20.0f);
//    else
//      ImGui::DragFloat("Fov", &(camera[0]->data.fov), 0.05f, 30.0f, 90.0f);
//
//      // Select rotation type
//    static Eigen::Quaternionf trackball_angle = Eigen::Quaternionf::Identity();
//    static bool orthographic = true;
//
//    // Orthographic view
//    ImGui::Checkbox("Orthographic view", &(camera[0]->_ortho));
//    if (camera[0]->_ortho) {
//        camera[0]->SetProjection(0,camera[0]->GetRelationWH());
//      }
//    else {
//        camera[0]->SetProjection(camera[0]->GetAngle() > 0 ? camera[0]->GetAngle() : 45,camera[0]->GetRelationWH());
//      }
//
//      ImGui::PopItemWidth();
//  }

  if (ImGui::CollapsingHeader("Cubemap", ImGuiTreeNodeFlags_DefaultOpen))
  {
      if (ImGui::Button("Change image", fullWidthVec2))
      {
          std::string filePath = igl::file_dialog_open();
          if (filePath.length() > 0)
          {
              viewer.ChangeCubemapImage(filePath);
          }
      }
  }

    // Helper for setting viewport specific mesh options
    auto make_checkbox = [&](const char* label, unsigned int& option)
    {
        return ImGui::Checkbox(label,
            [&]() { return drawInfos[1]->is_set(option); },
            [&](bool value) { return drawInfos[1]->set(option, value); }
        );
    };

  // Draw options
//  if (ImGui::CollapsingHeader("Draw Options"))
//  {
//    if (ImGui::Checkbox("Face-based", &(viewer.data()->face_based)))
//    {
//      viewer.data()->dirty = MeshGL::DIRTY_ALL;
//    }
////
////    make_checkbox("Show texture", viewer.data().show_texture);
////    if (ImGui::Checkbox("Invert normals", &(viewer.data().invert_normals)))
////    {
////      viewer.data().dirty |= igl::opengl::MeshGL::DIRTY_NORMAL;
////    }
//    make_checkbox("Show overlay", viewer.data()->show_overlay);
//    make_checkbox("Show overlay depth", viewer.data()->show_overlay_depth);
//
//    ImGui::ColorEdit4("Line color", viewer.data()->line_color.data(),
//        ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueWheel);
//    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.3f);
//    ImGui::DragFloat("Shininess", &(viewer.data()->shininess), 0.05f, 0.0f, 100.0f);
//    ImGui::PopItemWidth();
//  }

  // Overlays
  //if (ImGui::CollapsingHeader("Overlays"))
  //{
  //  make_checkbox("Wireframe", viewer.data()->show_lines);
  //  make_checkbox("Fill", viewer.data()->show_faces);

  //}
  ImGui::End();
}



IGL_INLINE float ImGuiMenu::pixel_ratio()
{
  // Computes pixel ratio for hidpi devices
  int buf_size[2];
  int win_size[2];
  GLFWwindow* window = glfwGetCurrentContext();
  glfwGetFramebufferSize(window, &buf_size[0], &buf_size[1]);
  glfwGetWindowSize(window, &win_size[0], &win_size[1]);
  return (float) buf_size[0] / (float) win_size[0];
}

IGL_INLINE float ImGuiMenu::hidpi_scaling()
{
  // Computes scaling factor for hidpi devices
  float xscale, yscale;
  GLFWwindow* window = glfwGetCurrentContext();
  glfwGetWindowContentScale(window, &xscale, &yscale);
  return 0.5 * (xscale + yscale);
}

} // end namespace
} // end namespace
} // end namespace
} // end namespace
