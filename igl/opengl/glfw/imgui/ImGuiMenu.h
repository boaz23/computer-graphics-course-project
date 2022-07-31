// This file is part of libigl, a simple c++ geometry processing library.
//
// Copyright (C) 2018 Jérémie Dumas <jeremie.dumas@ens-lyon.org>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
#ifndef IGL_OPENGL_GLFW_IMGUI_IMGUIMENU_H
#define IGL_OPENGL_GLFW_IMGUI_IMGUIMENU_H

////////////////////////////////////////////////////////////////////////////////
#include "igl/opengl/glfw/Viewer.h"
class Renderer;
class Project;
//#include <igl/opengl/glfw/ViewerPlugin.h>

//#include "igl/opengl/ViewerCore.h"
#include "igl/opengl/ViewerData.h"
#include "igl/opengl/glfw/Display.h"
#include <igl/igl_inline.h>
#include <igl/opengl/Camera.h>
#include "../DrawInfo.h"
#include <memory>
#include <igl/opengl/glfw/WindowSection.h>
////////////////////////////////////////////////////////////////////////////////




// Forward declarations
struct ImGuiContext;

namespace igl
{
namespace opengl
{
namespace glfw
{
namespace imgui
{

class ImGuiMenu //: public igl::opengl::glfw::ViewerPlugin
{
protected:
  // Hidpi scaling to be used for text rendering.
  float hidpi_scaling_;

  // Ratio between the framebuffer size and the window size.
  // May be different from the hipdi scaling!
  float pixel_ratio_;

  // ImGui Context
  ImGuiContext * context_ = nullptr;

public:
    float _trans_slidebar_val;

    IGL_INLINE virtual void init(Display* disp);// override;

  IGL_INLINE virtual void reload_font(int font_size = 13);

  IGL_INLINE virtual void shutdown();// override;

  IGL_INLINE virtual bool pre_draw();// override;

  IGL_INLINE  virtual bool post_draw();// override;

  IGL_INLINE virtual void post_resize(int width, int height);// override;

  // Mouse IO
  IGL_INLINE virtual bool mouse_down(GLFWwindow* window, int button, int modifier);// override;

  IGL_INLINE virtual bool mouse_up(GLFWwindow* window, int button, int modifier);// override;

  IGL_INLINE virtual bool mouse_move(GLFWwindow* window, int mouse_x, int mouse_y);// override;

  IGL_INLINE virtual bool mouse_scroll(GLFWwindow* window, float delta_y);// override;

  // Keyboard IO
  IGL_INLINE virtual bool key_pressed(GLFWwindow* window, unsigned int key, int modifiers);// override;

  IGL_INLINE virtual bool key_down(GLFWwindow* window, int key, int modifiers);// override;

  IGL_INLINE virtual bool key_up(GLFWwindow* window, int key, int modifiers);// override;

  // Draw menu
    //  TODO: return this after creating is set staff and remove core from here
//  IGL_INLINE virtual void draw_menu(igl::opengl::glfw::Viewer* viewer, std::vector<igl::opengl::ViewerCore> &core);

  // Can be overwritten by `callback_draw_viewer_window`
  //  TODO: return this after creating is set staff and remove core from here
//  IGL_INLINE virtual void draw_viewer_window(igl::opengl::glfw::Viewer* viewer, std::vector<igl::opengl::ViewerCore> &core);

  // Can be overwritten by `callback_draw_viewer_menu`
  IGL_INLINE virtual void draw_viewer_menu(Renderer* rndr, igl::opengl::glfw::Viewer &viewer, std::vector<igl::opengl::Camera*> &camera, igl::opengl::CameraData cameraData, Eigen::Vector4i& viewWindow);

  // Can be overwritten by `callback_draw_custom_window`
  IGL_INLINE virtual void draw_custom_window() { }

  // Easy-to-customize callbacks
  std::function<void(void)> callback_draw_viewer_window;
  std::function<void(void)> callback_draw_viewer_menu;
  std::function<void(void)> callback_draw_custom_window;

  IGL_INLINE void draw_labels_window(Renderer* rndr, igl::opengl::glfw::Viewer& viewer, std::vector<WindowSection*>& sections);

  IGL_INLINE void draw_labels(Project& project, 
      const igl::opengl::ViewerData &data, WindowSection& section, Eigen::Matrix4d Proj,
      Eigen::Matrix4d View);

  IGL_INLINE void draw_text(
    Eigen::Vector3d pos,
    Eigen::Vector3d normal,
    const std::string &text,
    WindowSection& section,
      Eigen::Matrix4d Proj,
      Eigen::Matrix4d View,
    const Eigen::Vector4f color = Eigen::Vector4f(0, 0, 0.04f, 1)); // old default color

  IGL_INLINE float pixel_ratio();

  IGL_INLINE float hidpi_scaling();

  float menu_scaling() { return hidpi_scaling_ / pixel_ratio_; }
};

} // end namespace
} // end namespace
} // end namespace
} // end namespace

#ifndef IGL_STATIC_LIBRARY
#  include "ImGuiMenu.cpp"
#endif

#endif // IGL_OPENGL_GLFW_IMGUI_IMGUIMENU_H
