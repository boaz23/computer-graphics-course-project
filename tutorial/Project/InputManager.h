#pragma once   //maybe should be static class
#include "igl/opengl/glfw/Display.h"
#include "igl/opengl/glfw/renderer.h"
#include "Project.h"
#include "imgui/imgui.h"
#include "igl/opengl/util.h"


	void glfw_mouse_callback(GLFWwindow* window,int button, int action, int mods)
	{	
		if (ImGui::GetIO().WantCaptureMouse) {
			return;
		}
		bool shiftPressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
		Renderer* rndr = (Renderer*)glfwGetWindowUserPointer(window);
		Project* scn = (Project*)rndr->GetScene();
		double x2, y2;
		glfwGetCursorPos(window, &x2, &y2);
		if (action == GLFW_PRESS)
		{
			rndr->UpdatePress(x2, y2);
			if (button == GLFW_MOUSE_BUTTON_LEFT)
				rndr->Pressed();
			// if not in select many mode and shift not pressed -> single picking
			if (!shiftPressed && !rndr->IsMany()) {
				if (rndr->Picking((int)x2, (int)y2))
				{
					rndr->UpdatePosition(x2, y2);
				}
			}
			// start select many mode
			else if (shiftPressed) {
				rndr->StartSelect();
			}
		}
		else
		{
			Renderer* rndr = (Renderer*)glfwGetWindowUserPointer(window);
			// if exiting select many mode apply selection
			if (rndr->isInSelectMode()) {
				rndr->PickMany(2);
				rndr->finishSelect();
			}
			else {
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
		
		if (rndr->IsPicked())
		{
			rndr->UpdateZpos((int)yoffset);
			rndr->MouseProccessing(GLFW_MOUSE_BUTTON_MIDDLE);
		}
		else
		{
			rndr->MoveCamera(scn->selectedCameraIndex, rndr->zTranslate, (float)yoffset);
		}
		
	}
	
	void glfw_cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
	{
		if (ImGui::GetIO().WantCaptureMouse) {
			return;
		}
		Renderer* rndr = (Renderer*)glfwGetWindowUserPointer(window);
		Project* scn = (Project*)rndr->GetScene();

		rndr->UpdatePosition((float)xpos,(float)ypos);

		if (rndr->CheckViewport(xpos,ypos, 0))
		{
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
	}

	void glfw_window_size_callback(GLFWwindow* window, int width, int height)
	{
		Renderer* rndr = (Renderer*)glfwGetWindowUserPointer(window);

        rndr->resize(window,width,height);
		
	}

	void ChangeCameraIndex_ByDelta(Renderer* rndr, Project* scn, int delta);
	void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (ImGui::GetIO().WantCaptureKeyboard) {
			return;
		}
		Renderer* rndr = (Renderer*)glfwGetWindowUserPointer(window);
		Project* scn = (Project*)rndr->GetScene();
		if (action == GLFW_PRESS || action == GLFW_REPEAT)
		{
			switch (key)
			{
			case GLFW_KEY_ESCAPE:
				rndr->UnPick(2);
				break;
			case GLFW_KEY_SPACE:
				if (scn->IsActive())
					scn->Deactivate();
				else
					scn->Activate();
				break;

			case GLFW_KEY_UP:
				rndr->MoveCamera(scn->selectedCameraIndex, scn->xRotate, 0.05f);
				break;
			case GLFW_KEY_DOWN:
				//scn->shapeTransformation(scn->xGlobalRotate,-5.f);
				//cout<< "down: "<<endl;
				rndr->MoveCamera(scn->selectedCameraIndex, scn->xRotate, -0.05f);
				break;
			case GLFW_KEY_LEFT:
				rndr->MoveCamera(scn->selectedCameraIndex, scn->yRotate, 0.05f);
				break;
			case GLFW_KEY_RIGHT:
				//scn->shapeTransformation(scn->xGlobalRotate,-5.f);
				//cout<< "down: "<<endl;
				rndr->MoveCamera(scn->selectedCameraIndex, scn->yRotate, -0.05f);
				break;
			case GLFW_KEY_Q:
				rndr->MoveCamera(scn->selectedCameraIndex, scn->yTranslate, 0.25f);
				break;
			case GLFW_KEY_E:
				rndr->MoveCamera(scn->selectedCameraIndex, scn->yTranslate, -0.25f);
				break;
			case GLFW_KEY_A:
				rndr->MoveCamera(scn->selectedCameraIndex, scn->xTranslate, -0.25f);
				break;
			
			case GLFW_KEY_D:
				rndr->MoveCamera(scn->selectedCameraIndex, scn->xTranslate, 0.25f);
				break;
			
			case GLFW_KEY_S:
				rndr->MoveCamera(scn->selectedCameraIndex, scn->zTranslate, 0.5f);
				break;
			case GLFW_KEY_W:
				rndr->MoveCamera(scn->selectedCameraIndex, scn->zTranslate, -0.5f);
				break;
			case GLFW_KEY_1:
				std::cout << "picked 1\n";
				scn->selected_data_index = 1;
				break;
			case GLFW_KEY_2:
				std::cout << "picked 2\n";
				scn->selected_data_index = 2;
				break;
			case GLFW_KEY_3:
				std::cout << "picked 3\n";
				scn->selected_data_index = 3;
				break;
			case GLFW_KEY_O:
				ChangeCameraIndex_ByDelta(rndr, scn, -1);
				break;
			case GLFW_KEY_P:
				ChangeCameraIndex_ByDelta(rndr, scn, 1);
				break;
			default:
				break;

			}
		}
	}

	void SetDrawCamera_DefaultViewport(Renderer* rndr, Project* scn, int cameraIndex)
	{
		rndr->GetDrawInfo(0).cameraIndx = cameraIndex;
		rndr->GetDrawInfo(1).cameraIndx = cameraIndex;
	}

	void ChangeCameraIndex_ByDelta(Renderer* rndr, Project* scn, int delta)
	{
		size_t selectedCameraIndex = scn->selectedCameraIndex;
		size_t cameraIndex = addCyclic<int>(static_cast<int>(selectedCameraIndex), delta, rndr->CamerasCount());
		scn->selectedCameraIndex = cameraIndex;
		scn->CameraMeshUnhide(selectedCameraIndex, rndr->GetCamera(selectedCameraIndex));
		scn->CameraMeshHide(cameraIndex);
		SetDrawCamera_DefaultViewport(rndr, scn, static_cast<int>(cameraIndex));
	}


void Init(Display& display, igl::opengl::glfw::imgui::ImGuiMenu *menu)
{
    display.AddKeyCallBack(glfw_key_callback);
    display.AddMouseCallBacks(glfw_mouse_callback, glfw_scroll_callback, glfw_cursor_position_callback);
    display.AddResizeCallBack(glfw_window_size_callback);
    menu->init(&display);
}
