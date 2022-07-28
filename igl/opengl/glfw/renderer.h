#pragma once
#ifndef IGL_OPENGL_GLFW_RENDERER_H
#define IGL_OPENGL_GLFW_RENDERER_H

#include <igl/igl_inline.h>
#include <vector>
#include <functional>
//#include <igl/opengl/ViewerCore.h>
#include "DrawInfo.h"
#include <igl/opengl/glfw/Viewer.h>
#include <igl/opengl/glfw/imgui/ImGuiMenu.h>
#include <igl/opengl/glfw/imgui/ImGuiHelpers.h>
#include <../imgui/imgui.h>
#include "../DrawBuffer.h"
#include "../Camera.h"
#include "../ViewerData.h"
#include <igl/opengl/glfw/WindowSection.h>
#include <igl/opengl/glfw/SectionLayer.h>

struct GLFWwindow;



class Renderer 
{
public:
    Renderer(igl::opengl::CameraData cameraData);

    enum buffersMode {
        COLOR, DEPTH, STENCIL, BACK, FRONT, NONE
    };
    enum transformations {
        xTranslate,
        yTranslate,
        zTranslate,
        xRotate,
        yRotate,
        zRotate,
        xScale,
        yScale,
        zScale,
        scaleAll,
        xCameraTranslate,
        yCameraTranslate,
        zCameraTranslate
    };
    enum drawFlags {
        toClear = 1,
        is2D = 2,
        inAction = 4,
        scissorTest = 8,
        depthTest = 16,
        stencilTest = 32,
        blend = 64,
        blackClear = 128,
        clearDepth = 256,
        backdraw = 512,
        clearStencil = 1024,
        passStencil = 2048,
        inAction2 = 4096,
        none = 8192,
        scaleAbit = 16384,
        stencil2 = 32768,
        onPicking = 65536
    };
    enum {
        preRot, postRot, phiRot, thetaRot, psiRot, psiPhiRot
    };
    enum {
        non, viewport, camera, shader, buffer,
    };

	~Renderer();
	IGL_INLINE void draw( GLFWwindow* window);

	//IGL_INLINE bool key_pressed(unsigned int unicode_key, int modifiers);

		// Returns **true** if action should be cancelled.
	std::function<bool(GLFWwindow* window)> callback_init;
	std::function<bool(GLFWwindow* window)> callback_pre_draw;
	std::function<bool(GLFWwindow* window)> callback_post_draw;
	std::function<bool(GLFWwindow* window, int button, int modifier)> callback_mouse_down;
	std::function<bool(GLFWwindow* window, int button, int modifier)> callback_mouse_up;
	std::function<bool(GLFWwindow* window, int mouse_x, int mouse_y)> callback_mouse_move;
	std::function<bool(GLFWwindow* window, float delta_y)> callback_mouse_scroll;
	std::function<bool(GLFWwindow* window, unsigned int key, int modifiers)> callback_key_pressed;
	std::function<bool(GLFWwindow* window, int w, int h)> callback_post_resize;
	// THESE SHOULD BE DEPRECATED:
	std::function<bool(GLFWwindow* window, unsigned int key, int modifiers)> callback_key_down;
	std::function<bool(GLFWwindow* window, unsigned int key, int modifiers)> callback_key_up;
	// Pointers to per-callback data
	void* callback_init_data;
	void* callback_pre_draw_data;
	void* callback_post_draw_data;
	void* callback_mouse_down_data;
	void* callback_mouse_up_data;
	void* callback_mouse_move_data;
	void* callback_mouse_scroll_data;
	void* callback_key_pressed_data;
	void* callback_key_down_data;
	void* callback_key_up_data;

// Callbacks
//	 double Picking(double x, double y);
	 inline void Animate() { scn->Animate(); };
	//IGL_INLINE bool key_pressed(unsigned int unicode_key, int modifier);
	IGL_INLINE void resize(GLFWwindow* window,int w, int h); // explicitly set window size
	IGL_INLINE void post_resize(GLFWwindow* window, int w, int h); // external resize due to user interaction
    void SetScene(igl::opengl::glfw::Viewer* scn);
	//void UpdatePosition(double xpos, double ypos);
	inline igl::opengl::glfw::Viewer* GetScene() {
		return scn;
	}

    float UpdatePosition(float xpos, float ypos);

    void UpdatePress(float xpos, float ypos);

    int AddCamera(const Eigen::Vector3d &pos, igl::opengl::CameraData cameraData);

    int AddSection(int left, int bottom, int width, int height, int buffIndex,
        bool createStencilLayer, bool createScissorsLayer, bool clearBuffer, 
        bool autoAddToSection=true,
        bool allowRotation=true);

    inline void ActivateSection(int sectionIndex) { windowSections[sectionIndex]->Activate(); }
    inline void DeactivateSection(int sectionIndex) { windowSections[sectionIndex]->Deactivate(); }
    inline bool IsSectionActive(int sectionIndex) { return windowSections[sectionIndex]->isActive(); }
    //unsigned int AddBuffer(int infoIndx);

    //int Create2Dmaterial(int infoIndx, int code);

    void AddDraw(int sectionIndex, int layerIndex, int buffIndx, unsigned int flags);

    //void CopyDraw(int infoIndx, int property, int indx);

    //void SetViewport(int left, int bottom, int width, int height, int indx);

    //inline void BindViewport2D(int indx) { drawInfos[indx]->SetFlags(is2D); }

    void MoveCamera(int cameraIndx, int type, float amt);

    bool Picking(int x, int y);

    //void OutLine();

    void PickMany(int x, int y);

    void MouseProccessing(int button);

    inline size_t CamerasCount() const { return cameras.size(); }

    inline const igl::opengl::Camera& GetCamera(int index) const { return *cameras[index]; }

    inline igl::opengl::Camera &GetCamera(int index) { return *cameras[index]; }

    inline float GetNear(int cameraIndx) { return cameras[cameraIndx]->GetNear(); }

    inline float GetFar(int cameraIndx) { return cameras[cameraIndx]->GetFar(); }

    inline float GetAngle(int cameraIndx) { return cameras[cameraIndx]->GetAngle(); }

    inline void Pressed() { isPressed = !isPressed; }

    inline bool IsPressed() const { return isPressed; }

    //inline void FreeShapes(int viewportIndx) { scn->ClearPickedShapes(viewportIndx); };

    bool CheckSection(int x, int y, int sectionIndex);

    bool UpdateSection(int section);

    inline int GetSectionsSize() { return (int)windowSections.size(); }

    inline WindowSection& GetSection(int sectionIndex) { return *windowSections[sectionIndex]; }

    inline int GetCurrentSectionIndex() { return currentSection; }

    inline WindowSection& GetCurrentSection() { return *windowSections[currentSection]; }

    inline std::vector<WindowSection*>& GetSections() { return windowSections; }

    float CalcMoveCoeff(int cameraIndx, int width);

    //void SetBuffers();

    inline void UpdateZpos(int ypos) { zrel = ypos; }

    inline void UnPick() {
        // Changed: clear isMany also
        isPicked = false;
        isMany = false;
        scn->ClearPickedShapes(GetStencilTestLayersIndexes());
    }
    inline bool IsPicked() { return isPicked; }
    inline bool IsMany() const { return isMany; }
    void Init(igl::opengl::glfw::Viewer *scene, igl::opengl::CameraData cameraData, int pickingBits,igl::opengl::glfw::imgui::ImGuiMenu *_menu);
    // Added: functions for selection mode and to try single pick
    bool isInSelectMode() {
        return isSelecting;
    }
    inline void StartSelect() {
        isSelecting = true;
    }
    inline void finishSelect() {
        isSelecting = false;
    }
    bool TrySinglePicking(int x, int y);

    std::vector<std::pair<int, int>> GetSceneLayersIndexes(bool onlyAutoAdd=true) {
        std::vector<std::pair<int, int>> layers;
        for (int i = 0; i < GetSectionsSize(); i++) {
            if ((!onlyAutoAdd || windowSections[i]->IsAutoAddSection()) && windowSections[i]->GetSceneLayerIndex() != -1) {
                layers.push_back(std::pair<int, int>(i, windowSections[i]->GetSceneLayerIndex()));
            }
        }
        return layers;
    }

    std::vector<std::pair<int, int>> GetScissorsTestLayersIndexes() {
        std::vector<std::pair<int, int>> layers;
        for (int i = 0; i < GetSectionsSize(); i++) {
            if(windowSections[i]->GetScissorTestLayerIndex() != -1)
                layers.push_back(std::pair<int, int>(i, windowSections[i]->GetScissorTestLayerIndex()));
        }
        return layers;
    }

    std::vector<std::pair<int, int>> GetStencilTestLayersIndexes() {
        std::vector<std::pair<int, int>> layers;
        for (int i = 0; i < GetSectionsSize(); i++) {
            if (windowSections[i]->GetScissorTestLayerIndex() != -1)
                layers.push_back(std::pair<int, int>(i, windowSections[i]->GetStencilTestLayerIndex()));
        }
        return layers;
    }

private:
    // Stores all the viewing options
//    std::vector<igl::opengl::ViewerCore> core_list;
    std::vector<WindowSection*> windowSections;
    std::vector<igl::opengl::Camera*> cameras;
    igl::opengl::glfw::Viewer* scn;
    //std::vector<Eigen::Vector4i> viewports;
    //std::vector<DrawInfo *> drawInfos;
    std::vector<igl::opengl::DrawBuffer*> buffers;
	size_t selected_core_index;
	int next_core_id;
    int xold, yold, xrel, yrel, zrel;
    int maxPixX, maxPixY;
    int xWhenPress, yWhenPress;
    bool isMany;
    bool isPicked;
    // Added: added selection flag
    bool isSelecting;
    int materialIndx2D;
    bool isPressed;
    int currentSection;
	unsigned int next_property_id = 1;
	float highdpi;
	float depth;
	unsigned int left_view, right_view;
	double doubleVariable;
	igl::opengl::glfw::imgui::ImGuiMenu* menu;
	double z;

    void draw_by_info(int sectionIndex, int layerIndex, int info_index, int width, int height);

    //void ActionDraw(int viewportIndx);

    void Clear(float r, float g, float b, float a, unsigned int flags);

    //void SwapDrawInfo(int indx1, int indx2);
};
#endif
