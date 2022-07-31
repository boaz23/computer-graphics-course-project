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
#include "ImGuiMenu.h"
#include <igl/project.h>
#include "ImGuiHelpers.h"
#include "igl/opengl/glfw/renderer.h"
#include "igl/opengl/glfw/WindowSection.h"
#include "tutorial/Project/Project.h"

#include "igl/opengl/glfw/imgui/imgui_impl_glfw.h"
#include "igl/opengl/glfw/imgui/imgui_impl_opengl3.h"
#include "igl/file_dialog_open.h"
#include "igl/project.h"

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


IGL_INLINE void ImGuiMenu::DrawAnimationMenu(Renderer* rndr, igl::opengl::glfw::Viewer& viewer, std::vector<igl::opengl::Camera*>& camera, igl::opengl::CameraData cameraData, Eigen::Vector4i& viewWindow)
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
    if (project->IsActive()) {
        if (ImGui::Button("Pause animation")) {
            project->PauseAnimation();
        }
    }
    else {
        if (ImGui::Button("Resume animation")) {
            project->ResumeAnimation();
        }
    }
    if (ImGui::Button("Stop animation")) {
        project->StopAnimation();
    }
    ImGui::End();
}


IGL_INLINE void ImGuiMenu::DrawDesignMenu(Renderer *rndr, igl::opengl::glfw::Viewer &viewer, std::vector<igl::opengl::Camera*> &camera, igl::opengl::CameraData cameraData, Eigen::Vector4i& viewWindow)
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
  if (project && ImGui::CollapsingHeader("Viewports", ImGuiTreeNodeFlags_DefaultOpen))
  {
      bool splitMode = project->IsSplitMode();
      bool editBezierMode = project->IsEditBezierMode();
      if (project->isInDesignMode && ImGui::Checkbox("Split", &splitMode))
      {
          project->ToggleSplitMode();
      }
      if (project->isInDesignMode && (project->IsEditBezierMode() || project->CanEnterBezierMode()) && ImGui::Checkbox("Edit Bezier", &editBezierMode))
      {
          project->ToggleEditBezierMode();
      }
  }
  if (project && project->IsEditBezierMode() && ImGui::CollapsingHeader("Bezier editor", ImGuiTreeNodeFlags_DefaultOpen))
  {
      if (project && ImGui::Button("Add curve", fullWidthVec2))
      {
          project->AddBezierSegment();
      }
      if (project && project->GetCurrentBezierMesh()->GetSegmentsCount() > 0 && ImGui::Button("Delete last curve", fullWidthVec2))
      {
          project->RemoveBezierSegment();
      }
  }
  if (project && ImGui::CollapsingHeader("Animations", ImGuiTreeNodeFlags_DefaultOpen))
  {
      if (ImGui::Button("Start Animation", fullWidthVec2)) {
          project->EnterAnimation();
      }
      float animationTime = (float) project->CalcAnimationTime();
      float currentAnimationTime = (float) project->GetCurrentAnimationTimeInSeconds();
      if (animationTime > 0) {
          ImGui::Text("Animation time:");
          if (ImGui::SliderFloat("##animationTimeSlider", &currentAnimationTime, 0.0f, animationTime, "%.2f s")) {
              project->SetCurrentAnimationTime(currentAnimationTime);
          }
      }
      if (project && ImGui::Button("Toggle animation direction\nedit mode", fullWidthVec2))
      {
          project->ToggleAnimationRotationMode();
      }
      if (project && project->CanEditMeshDelay()) {
          double delay = project->GetCurrentMeshDelay();
          ImGui::Text("Current Mesh Delay: ");
          if (ImGui::InputDouble("##meshdelay", &delay, 0.1, 0.1, "%.2f s")) {
              project->SetCurrentMeshDelay(delay);
          }
      }
      if (project && ImGui::CollapsingHeader("Animation Segments", ImGuiTreeNodeFlags_DefaultOpen)) {
          std::map<int, std::string> options = project->GetAnimationCamerasNames();
          std::vector<AnimationSegment*> segments = project->GetAnimationSegments();
          auto createAnimationSegmentComboBoxHelper = [&project](int segmentIndex, std::map<int, std::string>& options, int currentSelectedOption) {
              const std::string& comboPreview = options[currentSelectedOption];
              if (ImGui::BeginCombo(("##segment" + std::to_string(segmentIndex) + "combo").c_str(), comboPreview.c_str()))
              {
                  bool valueChanged{ false };
                  int selectedOption = currentSelectedOption;
                  for (auto& option : options)
                  {
                      bool itemSelected = option.first == currentSelectedOption;
                      const char* item_text = option.second.c_str();
                      if (ImGui::Selectable(item_text, itemSelected))
                      {
                          valueChanged = true;
                          selectedOption = option.first;
                      }
                      if (itemSelected)
                      {
                          ImGui::SetItemDefaultFocus();
                      }
                  }
                  ImGui::EndCombo();

                  if (valueChanged)
                  {
                      project->SetAnimationSegmentCamera(segmentIndex, selectedOption);
                  }
              }
          };
          for (int i = 0; i < (int)segments.size(); i++) {
              ImGui::Text(("Segment " + std::to_string(i + 1) + ":").c_str());
              ImGui::Text("Camera:   ");
              ImGui::SameLine();
              createAnimationSegmentComboBoxHelper(i, options, segments[i]->cameraIndex);
              ImGui::Text("Duration: ");
              ImGui::SameLine();
              double duration = segments[i]->GetDuration();
              if (ImGui::InputDouble(("##input" + std::to_string(i) + "duration").c_str(), &duration, 0.1, 0.1, "%.2f s")) {
                  project->SetAnimationSegmentDuration(i, duration);
              }
              if (ImGui::Button(("Delete segment##" + std::to_string(i)).c_str(), fullWidthVec2))
              {
                  project->RemoveAnimationSegment(i);
              }
          }
          if (project && ImGui::Button("Create new segment", fullWidthVec2))
          {
              project->CreateNewAnimationSegment();
          }
      }
  }
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
    if (ImGui::Checkbox("Area selection picks", &rndr->shouldAreaSelectPick))
    {
    }
    if (ImGui::Button("Reset active camera", fullWidthVec2))
    {
        project->ResetActiveCamera();
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
    if (project)
    {
        ImGui::PushItemWidth(100 * menu_scaling());

        ImGui::Text("Material:");
        ImGui::SameLine(0, p);

        static int selectedMaterialComboIndex = 0;
        auto& availableMaterials = project->availableMaterials;

        bool allSameMaterial = viewer.pShapes.size() == 0 ? false :
            viewer.AllPickedShapesSameValue<unsigned int>([](const igl::opengl::ViewerData& shape)
            {
                return shape.GetMaterial();
            });
        bool fieldChanged{ false };
        int materialComboIndex{ -1 };
        if (allSameMaterial)
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

            if (materialComboIndex >= 0)
            {
                if (selectedMaterialComboIndex != materialComboIndex)
                {
                    selectedMaterialComboIndex = materialComboIndex;
                    fieldChanged = true;
                }
            }
            else
            {
                allSameMaterial = false;
            }
        }

        const std::string& materialComboPreview = allSameMaterial ? availableMaterials[materialComboIndex].second : "";
        if (ImGui::BeginCombo("##material", materialComboPreview.c_str()))
        {
            bool valueChanged{ false };
            for (size_t i = 0; i < availableMaterials.size(); ++i)
            {
                const bool item_selected = i == selectedMaterialComboIndex && allSameMaterial;
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
  bool selectedTransparentableShapes = false;
  if (project != nullptr) {
      for (int pshape : project->pShapes) {
          if (project->GetProjectMeshByIndex(pshape)->IsTransparentAllowed()) {
              selectedTransparentableShapes = true;
              break;
          }
      }
  }
  if (project != nullptr && selectedTransparentableShapes && ImGui::CollapsingHeader("Transperancy", ImGuiTreeNodeFlags_DefaultOpen))
  {
      ImGui::Text("Alpha:");
      ImGui::SameLine(0, p);

      // Objects Transperancy
      if (project->pShapes.size() > 0)
          _trans_slidebar_val = project->GetProjectMeshByIndex(project->pShapes[project->pShapes.size() - 1])->GetAlpha();

      bool _hide_slide_val = false;

      if (project->pShapes.size() > 1)
      {
          for (int pshape : project->pShapes)
              if (project->GetProjectMeshByIndex(pshape)->GetAlpha() != project->GetProjectMeshByIndex(project->pShapes[0])->GetAlpha())
                  _hide_slide_val = true;
      }

      if (ImGui::SliderFloat("##alphaSlider", &_trans_slidebar_val, 0.0, 1.0, _hide_slide_val ? "" : "%.2f")) {
          if (project->pShapes.size() > 0) {
              for (int pshape : project->pShapes) {
                  if (project->GetProjectMeshByIndex(pshape)->IsTransparentAllowed()) {
                      project->GetProjectMeshByIndex(pshape)->SetAlpha(_trans_slidebar_val);
                  }
              }
          }
      }
  }
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
    //auto make_checkbox = [&](const char* label, unsigned int& option)
    //{
    //    return ImGui::Checkbox(label,
    //        [&]() { return drawInfos[1]->is_set(option); },
    //        [&](bool value) { return drawInfos[1]->set(option, value); }
    //    );
    //};

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


IGL_INLINE void ImGuiMenu::draw_viewer_menu(Renderer* rndr, igl::opengl::glfw::Viewer& viewer, std::vector<igl::opengl::Camera*>& camera, igl::opengl::CameraData cameraData, Eigen::Vector4i& viewWindow)
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
    Project* project = dynamic_cast<Project*>(&viewer);
    if (project->IsDesignMode()) {
        DrawDesignMenu(rndr, viewer, camera, cameraData, viewWindow);
    }
    else {
        DrawAnimationMenu(rndr, viewer, camera, cameraData, viewWindow);
    }

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

IGL_INLINE void ImGuiMenu::draw_labels_window(Renderer* rndr, igl::opengl::glfw::Viewer& viewer, std::vector<WindowSection*>& sections)
{
    Project* project = dynamic_cast<Project*>(&viewer);
    // Text labels
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);
    bool visible = true;
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::Begin("ViewerLabels", &visible,
        ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_NoScrollWithMouse
        | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoSavedSettings
        | ImGuiWindowFlags_NoInputs);
    for (int i = 0; i < rndr->GetSectionsSize(); i++) {
        WindowSection& section = rndr->GetSection(i);
        igl::opengl::Camera& camera = rndr->GetCamera(section.GetCamera());
        Eigen::Vector4i viewport = section.GetViewportSize();
        Eigen::Matrix4d Proj = camera.CalcProjection(viewport.z() * 1.0 / viewport.w()).cast<double>();
        Eigen::Matrix4d View = project->MakeCameraTransScaled(section.GetCamera()).inverse() * project->MakeTransScaled();
        if (section.isActive()) {
            int index = 0;
            for (const auto& data : project->data_list)
            {
                if (project->ShouldRenderViewerData(*data, i, section.GetSceneLayerIndex(), index)) {
                    AnimationCameraData* mesh = dynamic_cast<AnimationCameraData*>(const_cast<igl::opengl::ViewerData*>(data));
                    Eigen::Matrix4d Model = mesh == nullptr ? View * project->MakeMeshTransScaled(index) : View * project->MakeMeshTransd(index);
                    draw_labels(*project, *data, section, Proj, Model);
                }
                index++;
            }
        }
    }
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

IGL_INLINE void ImGuiMenu::draw_labels(Project& project, const igl::opengl::ViewerData& data, WindowSection& section, Eigen::Matrix4d Proj,
    Eigen::Matrix4d View)
{
    if (data.show_custom_labels != 0 && data.labels_positions.rows() > 0)
    {
        ProjectMesh* mesh = dynamic_cast<ProjectMesh*>(const_cast<igl::opengl::ViewerData*>(&data));
        for (int i = 0; i < data.labels_positions.rows(); ++i)
        {
            Eigen::Matrix4d viewToSend = View;
            // draw axis only in edit animation mode
            if (mesh->AllowAnimations()) {
                AnimatedMesh* animatedMesh = dynamic_cast<AnimatedMesh*>(const_cast<ProjectMesh*>(mesh));
                if (!project.IsAnimationRotationMode() && animatedMesh->GetAxisLabelsStart() != -1 && i >= animatedMesh->GetAxisLabelsStart() && i < animatedMesh->GetAxisLabelsStart() + 3) {
                    continue;
                }
                else if (project.IsAnimationRotationMode() && animatedMesh->GetAxisLabelsStart() != -1 &&
                    i >= animatedMesh->GetAxisLabelsStart() && i < animatedMesh->GetAxisLabelsStart() + 3) {
                    viewToSend *= animatedMesh->GetAnimationDirection();
                }
            }
            draw_text(
                data.labels_positions.row(i),
                Eigen::Vector3d(0.0, 0.0, 0.0),
                data.labels_strings[i],
                section,
                Proj,
                viewToSend,
                data.label_color);
        }
    }
}

IGL_INLINE void ImGuiMenu::draw_text(
    Eigen::Vector3d pos,
    Eigen::Vector3d normal,
    const std::string& text,
    WindowSection& section,
    Eigen::Matrix4d Proj,
    Eigen::Matrix4d View,
    const Eigen::Vector4f color)
{
    pos += normal * 0.005f;
    //igl::project(Eigen::Matrix)    
    Eigen::Vector3d coord = 
        igl::project<double>(pos, View, Proj, section.GetViewportSize().cast<double>());

    // Draw text labels slightly bigger than normal text
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddText(ImGui::GetFont(), ImGui::GetFontSize(),
        ImVec2(coord[0] / pixel_ratio_, (section.GetViewportSize().w() - coord[1]) / pixel_ratio_),
        ImGui::GetColorU32(ImVec4(
            color(0),
            color(1),
            color(2),
            color(3))),
        &text[0], &text[0] + text.size());
}



} // end namespace
} // end namespace
} // end namespace
} // end namespace
