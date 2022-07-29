#pragma once   //maybe should be static class
#include "igl/opengl/glfw/Display.h"
#include "igl/opengl/glfw/renderer.h"
#include "Project.h"
#include "imgui/imgui.h"
#include "igl/opengl/util.h"


	void glfw_mouse_callback(GLFWwindow* window,int button, int action, int mods)
	{	
		if (ImGui::GetIO().WantCaptureMouse)
		{
			return;
		}
		bool shiftPressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
		Renderer* rndr = (Renderer*)glfwGetWindowUserPointer(window);
		Project* scn = (Project*)rndr->GetScene();
		double x2, y2;
		glfwGetCursorPos(window, &x2, &y2);
		if (action == GLFW_PRESS)
		{
			rndr->UpdatePress((float)x2, (float)y2);
			if (button == GLFW_MOUSE_BUTTON_LEFT)
				rndr->Pressed();
			// if not in select many mode and shift not pressed -> single picking
			if (rndr->IsMany())
			{
				rndr->RecalculateDepths();
			}
			else
			{
				if (shiftPressed)
				{
					rndr->StartSelect();
				}
				else if (rndr->Picking((int)x2, (int)y2))
				{
					rndr->UpdatePosition((float)x2, (float)y2);
				}
			}
		}
		else if (action == GLFW_RELEASE)
		{
			Renderer* rndr = (Renderer*)glfwGetWindowUserPointer(window);
			// if exiting select many mode apply selection
			if (rndr->isInSelectMode()) {
				rndr->PickMany((int)x2, (int)y2);
				rndr->finishSelect();
			}
			else
			{
				// if not in selection mode but many picked check if this is a click or drag and 
				// single pick if click(else nothing will happend and usual drag will continue)
				rndr->TrySinglePicking((int)x2, (int)y2);
			}
		}
	}
	
	void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		Renderer* rndr = (Renderer*)glfwGetWindowUserPointer(window);
		Project* scn = (Project*)rndr->GetScene();
		rndr->UpdateZpos((int)yoffset);
		// TODO zoom on object 
		rndr->MouseProccessing(GLFW_MOUSE_BUTTON_MIDDLE);
	}
	
	void glfw_cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
	{
		if (ImGui::GetIO().WantCaptureMouse)
		{
			return;
		}
		Renderer* rndr = (Renderer*)glfwGetWindowUserPointer(window);
		Project* scn = (Project*)rndr->GetScene();

		rndr->UpdatePosition((float)xpos,(float)ypos);

		bool shiftPressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
		if (!shiftPressed)
		{
			if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
			{
				rndr->MouseProccessing(GLFW_MOUSE_BUTTON_RIGHT);
			}
			else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
			{
				rndr->MouseProccessing(GLFW_MOUSE_BUTTON_LEFT);
			}
		}
	}

	void glfw_window_size_callback(GLFWwindow* window, int width, int height)
	{
		Renderer* rndr = (Renderer*)glfwGetWindowUserPointer(window);

        rndr->resize(window,width,height);
		
	}

	void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (ImGui::GetIO().WantCaptureKeyboard)
		{
			return;
		}
		Renderer* rndr = (Renderer*)glfwGetWindowUserPointer(window);
		Project* scn = (Project*)rndr->GetScene();
		int currentCamera = rndr->GetCurrentSection().GetCamera();
		bool allowRotation = rndr->GetCurrentSection().IsRotationAllowed();
		if (action == GLFW_PRESS || action == GLFW_REPEAT)
		{
			switch (key)
			{
			case GLFW_KEY_ESCAPE:
				rndr->UnPick();
				break;
			case GLFW_KEY_SPACE:
				if (scn->IsActive())
					scn->Deactivate();
				else
					scn->Activate();
				break;

			// TODO: transformation in camera plane
			case GLFW_KEY_UP:
				if (allowRotation) {
					scn->MoveCamera([](Movable& movable)
					{
						movable.MyRotate(Eigen::Vector3d(1, 0, 0), 0.05f);
					});
				}
				break;
			case GLFW_KEY_DOWN:
				if (allowRotation) {
					scn->MoveCamera([](Movable& movable)
					{
						movable.MyRotate(Eigen::Vector3d(1, 0, 0), -0.05f);
					});
				}
				break;
			case GLFW_KEY_LEFT:
				if (allowRotation) {
					scn->MoveCamera([](Movable& movable)
					{
						movable.MyRotate(Eigen::Vector3d(0, 1, 0), 0.05f);
					});
				}
				break;
			case GLFW_KEY_RIGHT:
				if (allowRotation) {
					scn->MoveCamera([](Movable& movable)
					{
						movable.MyRotate(Eigen::Vector3d(0, 1, 0), -0.05f);
					});
				}
				break;

			case GLFW_KEY_Q:
				scn->MoveCamera([](Movable &movable)
				{
					movable.MyTranslate(Eigen::Vector3d(0, 0.25f, 0), 1);
				});
				break;
			case GLFW_KEY_E:
				scn->MoveCamera([](Movable &movable)
				{
					movable.MyTranslate(Eigen::Vector3d(0, -0.25f, 0), 1);
				});
				break;
			case GLFW_KEY_A:
				scn->MoveCamera([](Movable &movable)
				{
					movable.MyTranslate(Eigen::Vector3d(-0.25f, 0, 0), 1);
				});
				break;
			case GLFW_KEY_D:
				scn->MoveCamera([](Movable &movable)
				{
					movable.MyTranslate(Eigen::Vector3d(0.25f, 0, 0), 1);
				});
				break;
			case GLFW_KEY_S:
				scn->MoveCamera([](Movable &movable)
				{
					movable.MyTranslate(Eigen::Vector3d(0, 0, 0.25f), 1);
				});
				break;
			case GLFW_KEY_W:
				scn->MoveCamera([](Movable &movable)
				{
					movable.MyTranslate(Eigen::Vector3d(0, 0, -0.25f), 1);
				});
				break;

			case GLFW_KEY_O:
				scn->ChangeCameraIndex_ByDelta(-1);
				break;
			case GLFW_KEY_P:
				scn->ChangeCameraIndex_ByDelta(1);
				break;
			default:
				break;

			}
		}
	}


void Init(Display& display, igl::opengl::glfw::imgui::ImGuiMenu *menu)
{
    display.AddKeyCallBack(glfw_key_callback);
    display.AddMouseCallBacks(glfw_mouse_callback, glfw_scroll_callback, glfw_cursor_position_callback);
    display.AddResizeCallBack(glfw_window_size_callback);
    menu->init(&display);
}
