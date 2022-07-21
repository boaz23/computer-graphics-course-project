#pragma once   //maybe should be static class
#include "igl/opengl/glfw/Display.h"
#include "igl/opengl/glfw/renderer.h"
#include "Assignment3.h"
#include "imgui/imgui.h"


	void glfw_mouse_callback(GLFWwindow* window,int button, int action, int mods)
	{	
		if (action == GLFW_PRESS)
		{
			Renderer* rndr = (Renderer*)glfwGetWindowUserPointer(window);
			Assignment3* scn = (Assignment3*)rndr->GetScene();
			double x2, y2;
			
			glfwGetCursorPos(window, &x2, &y2);
			rndr->UpdatePress(x2, y2);
			if (rndr->Picking((int)x2, (int)y2))
			{
				rndr->UpdatePosition(x2, y2);
				if(button == GLFW_MOUSE_BUTTON_LEFT)
					rndr->Pressed();
			}
			else
			{
				rndr->UnPick(2);
			}
		
		}
		else
		{
			Renderer* rndr = (Renderer*)glfwGetWindowUserPointer(window);
			rndr->UnPick(2);
		}
	}
	
	void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		Renderer* rndr = (Renderer*)glfwGetWindowUserPointer(window);
		Assignment3* scn = (Assignment3*)rndr->GetScene();
		
		if (rndr->IsPicked())
		{
			rndr->UpdateZpos((int)yoffset);
			rndr->MouseProccessing(GLFW_MOUSE_BUTTON_MIDDLE);
		}
		else
		{
			rndr->MoveCamera(0, rndr->zTranslate, (float)yoffset);
		}
		
	}
	
	void glfw_cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
	{
		Renderer* rndr = (Renderer*)glfwGetWindowUserPointer(window);
		Assignment3* scn = (Assignment3*)rndr->GetScene();

		rndr->UpdatePosition((float)xpos,(float)ypos);

		if (rndr->CheckViewport(xpos,ypos, 0))
		{
			if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
			{

				rndr->MouseProccessing(GLFW_MOUSE_BUTTON_RIGHT);
			}
			else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
			{
				
				rndr->MouseProccessing(GLFW_MOUSE_BUTTON_LEFT);
			}
			else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE && rndr->IsPicked() && rndr->IsMany())
					rndr->MouseProccessing(GLFW_MOUSE_BUTTON_RIGHT);

		}
	}

	void glfw_window_size_callback(GLFWwindow* window, int width, int height)
	{
		Renderer* rndr = (Renderer*)glfwGetWindowUserPointer(window);

        rndr->resize(window,width,height);
		
	}
	
	void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		Renderer* rndr = (Renderer*)glfwGetWindowUserPointer(window);
		Assignment3* scn = (Assignment3*)rndr->GetScene();
		//rndr->FreeShapes(2);
		if (action == GLFW_PRESS || action == GLFW_REPEAT)
		{
			switch (key)
			{
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(window, GLFW_TRUE);
				break;
				
			case GLFW_KEY_SPACE:
				if (scn->IsActive())
					scn->Deactivate();
				else
					scn->Activate();
				break;

			case GLFW_KEY_UP:
				//rndr->MoveCamera(0, scn->xRotate, 0.05f);
				if (scn->pickedShape > 0)
				{
					Eigen::Matrix3d rot = scn->GetRotation();
					Eigen::Vector3d vec(0, 0.5f, 0);
					//scn->data_list[scn->pickedShape]->TranslateInSystem(rot, vec);
					//scn->data()
					scn->ShapeTransformation(scn->yTranslate, 0.5f, 1);
				}
				break;
			case GLFW_KEY_DOWN:
				if (scn->pickedShape > 0)
				{
					Eigen::Matrix3d rot = scn->GetRotation();
					Eigen::Vector3d vec(0, 0.5f, 0);
					//scn->data_list[scn->pickedShape]->TranslateInSystem(rot, -vec);
					scn->ShapeTransformation(scn->yTranslate, -0.5f, 1);
				}//rndr->MoveCamera(0, scn->xRotate, -0.05f);
				break;
			case GLFW_KEY_LEFT:
				//rndr->MoveCamera(0, scn->yRotate, 0.05f);
				if (scn->pickedShape > 0)
				{
					Eigen::Matrix3d rot = scn->GetRotation();
					Eigen::Vector3d vec(0.5f,0, 0);
					//scn->data_list[scn->pickedShape]->TranslateInSystem(rot, -vec);
					scn->ShapeTransformation(scn->xRotate, -0.1f, 0);
				}
				break;
			case GLFW_KEY_RIGHT:
				if (scn->pickedShape > 0)
				{
					Eigen::Matrix3d rot = scn->GetRotation();
					Eigen::Vector3d vec(0.5f, 0, 0);
					//scn->data_list[scn->pickedShape]->TranslateInSystem(rot, vec);
					scn->ShapeTransformation(scn->xRotate, 0.1f, 0);
				}
				//rndr->MoveCamera(0, scn->yRotate, -0.05f);
				break;
			case GLFW_KEY_U:
				rndr->MoveCamera(0, scn->yTranslate, 0.25f);
				break;
			case GLFW_KEY_D:
				rndr->MoveCamera(0, scn->yTranslate, -0.25f);
				break;
			case GLFW_KEY_L:
				rndr->MoveCamera(0, scn->xTranslate, -0.25f);
				break;
			
			case GLFW_KEY_R:
				rndr->MoveCamera(0, scn->xTranslate, 0.25f);
				break;
			
			case GLFW_KEY_B:
				rndr->MoveCamera(0, scn->zTranslate, 0.5f);
				break;
			case GLFW_KEY_F:
				rndr->MoveCamera(0, scn->zTranslate, -0.5f);
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
