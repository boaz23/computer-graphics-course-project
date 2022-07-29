#include "renderer.h"
#include <GLFW/glfw3.h>

#include <igl/unproject_onto_mesh.h>
#include <utility>
#include "igl/look_at.h"

//#include <Eigen/Dense>

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif


Renderer::Renderer(igl::opengl::CameraData cameraData)
{

    callback_init = nullptr;
    callback_pre_draw = nullptr;
    callback_post_draw = nullptr;
    callback_mouse_down = nullptr;
    callback_mouse_up = nullptr;
    callback_mouse_move = nullptr;
    callback_mouse_scroll = nullptr;
    callback_key_down = nullptr;
    callback_key_up = nullptr;

    callback_init_data = nullptr;
    callback_pre_draw_data = nullptr;
    callback_post_draw_data = nullptr;
    callback_mouse_down_data = nullptr;
    callback_mouse_up_data = nullptr;
    callback_mouse_move_data = nullptr;
    callback_mouse_scroll_data = nullptr;
    callback_key_down_data = nullptr;
    callback_key_up_data = nullptr;
    glLineWidth(5);
    //cameras.push_back(new igl::opengl::Camera(cameraData));
    isPressed = false;
    isMany = false;
    xold = 0;
    yold = 0;
    xrel = 0;
    yrel = 0;
    zrel = 0;
    currentSection = 0;
    isPicked = false;
    // Added: flag for selection mode
    isSelecting = false;
}



void Renderer::Clear(float r, float g, float b, float a,unsigned int flags)
{
    glClearColor(r, g, b, a);

    glClear(GL_COLOR_BUFFER_BIT | ((GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT) & flags));
}

//void Renderer::SwapDrawInfo(int indx1, int indx2)
//{
//    DrawInfo* info = drawInfos[indx1];
//    drawInfos[indx1] = drawInfos[indx2];
//    drawInfos[indx2] = info;
//}

IGL_INLINE void Renderer::draw_by_info(int sectionIndex, int layerIndex, int info_index, int width, int height){
    WindowSection& section = *windowSections[sectionIndex];
    igl::opengl::Camera& camera = *cameras[section.GetCamera()];
    DrawInfo& info = section.GetDrawInfo(layerIndex, info_index);
    Eigen::Vector4i viewportSize = section.GetViewportSize();
    buffers[info.bufferIndx]->Bind();
    glViewport(viewportSize.x(), height - viewportSize.w() - viewportSize.y(), viewportSize.z(), viewportSize.w());
    if (info.flags & scissorTest)
    {
        int x = std::min(xWhenPress, xold);
        int y = height - std::max(yWhenPress, yold);
        glEnable(GL_SCISSOR_TEST);
        glScissor(x, y, std::abs(xWhenPress - xold), std::abs( yWhenPress - yold));
    }
    else
        glDisable(GL_SCISSOR_TEST);

    if (info.flags & stencilTest)
    {
        glEnable(GL_STENCIL_TEST);
        if (info.flags & passStencil)
        {
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        }
        else
        {
            if (info.flags & stencil2)
            {
                glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
            }
            else
            {
                glStencilFunc(GL_EQUAL, 0, 0xFF);
                glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            }
        }
    }
    else
        glDisable(GL_STENCIL_TEST);

    if (info.flags & depthTest)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);

    if (info.flags & blend)
    {
        glEnable(GL_BLEND);
        // TODO changed order?
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
        glDisable(GL_BLEND);

    Eigen::Matrix4f Proj = camera.GetViewProjection().cast<float>();
    Eigen::Matrix4f View = camera.MakeTransScaled().inverse().cast<float>();

    if (info.flags & toClear)
    {
        if (info.flags & blackClear)
            Clear(0, 0, 0, 0,info.flags);
        else
            Clear(info.Clear_RGBA.x(), info.Clear_RGBA.y(), info.Clear_RGBA.z(), info.Clear_RGBA.w(),info.flags);
    }
    scn->Draw(scn->GetPickingShaderIndex(), Proj, View, sectionIndex, layerIndex, camera.GetPosition(), info.flags, info.property_id);
}

IGL_INLINE void Renderer::draw( GLFWwindow* window)
{
	using namespace std;
	using namespace Eigen;

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	int width_window, height_window;
	glfwGetWindowSize(window, &width_window, &height_window);
	
	auto highdpi_tmp = (width_window == 0 || width == 0) ? highdpi : (width / width_window);

	if (fabs(highdpi_tmp - highdpi) > 1e-8)
	{
		post_resize(window,width, height);
		highdpi = highdpi_tmp;
	}
	if (menu)
	{
		menu->pre_draw();
		menu->callback_draw_viewer_menu();
    }
    int sectionIndex = 0;
    for (auto& section : windowSections)
    {
        if (!section->isActive()) {
            sectionIndex++;
            continue;
        }
        int layerIndex = 0;
        for (auto& layer : section->GetLayers()) {
            int infoIndex = 0;
            for (auto& info : layer->getInfos()) {
                // Added: changed multipicking to work when inAction2 && !stencilTest && isSelecting and single picking to work when inAction && stencilTest && isPicked
                if (
                    // regular info
                    !(info->flags & (inAction | inAction2)) || 
                    // scissors test info
                    ((info->flags & inAction2) && !(info->flags & stencilTest) && isSelecting && CheckSection(xWhenPress, yWhenPress, sectionIndex)) || 
                    // stencil test info
                    ((info->flags & inAction) && (info->flags & stencilTest) && isPicked))
                    draw_by_info(sectionIndex, layerIndex, infoIndex, width, height);
                infoIndex++;
            }
            layerIndex++;
        }
        sectionIndex++;
    }

    if (menu)
	{
		menu->post_draw();

	}


}

void Renderer::SetScene(igl::opengl::glfw::Viewer* viewer)
{
	scn = viewer;
}


//void Renderer::UpdatePosition(double xpos, double ypos)
//{
//	xrel = xold - xpos;
//	yrel = yold - ypos;
//	xold = xpos;
//	yold = ypos;
//}

float Renderer::UpdatePosition(float xpos, float ypos)
{
    xrel = xold - xpos;
    yrel = yold - ypos;
    xold = xpos;
    yold = ypos;
    return yrel;
}

void Renderer::UpdatePress(float xpos, float ypos)
{
    if (currentSection < 0 || !windowSections[currentSection]->isActive() || !CheckSection(xpos, ypos, currentSection)) {
        for (int i = 0; i < GetSectionsSize(); i++) {
            if (windowSections[i]->isActive() && CheckSection(xpos, ypos, i)) {
                UpdateSection(i);
                break;
            }
        }
    }
    xWhenPress = xpos;
    yWhenPress = ypos;
}

int Renderer::AddCamera(const Eigen::Vector3d& pos, igl::opengl::CameraData cameraData)
{
    int cameraIndex = cameras.size();
    cameras.push_back(new igl::opengl::Camera(cameraData));
    cameras.back()->MyTranslate(pos, false);
    return cameraIndex;
}

int Renderer::AddSection(int left, int bottom, int width, int height, int buffIndex,
    bool createStencilLayer, bool createScissorsLayer, bool clearBuffer, bool autoAddToSection,
    bool allowRotation)
{
    windowSections.emplace_back(new WindowSection(left, bottom, width, height, 
        buffIndex, next_property_id, GetSectionsSize(), 
        createStencilLayer, createScissorsLayer, clearBuffer, autoAddToSection, allowRotation));
    next_property_id <<= windowSections.back()->GetLayers().size();
    windowSections.back()->SetCamera(0);
    glViewport(left, bottom, width, height);
    return (int)windowSections.size() - 1;
}

void Renderer::AddDraw(int sectionIndex, int layerIndex, int buffIndx, unsigned int flags)
{
    windowSections[sectionIndex]->GetLayers()[layerIndex]->AddDraw(buffIndx, flags, next_property_id);
    next_property_id <<= 1;
}

Renderer::~Renderer()
{
    for (auto& section : windowSections) {
        delete section;
    }
	//if (scn)
	//	delete scn;
}


bool Renderer::Picking(int x, int y)
{
    // Added: call picking of the scene
    UnPick();
    WindowSection& section = *windowSections[currentSection];
    igl::opengl::Camera& currentCamera = *cameras[section.GetCamera()];
    Eigen::Matrix4d Proj = currentCamera.GetViewProjection().cast<double>();
    Eigen::Matrix4d View = currentCamera.MakeTransScaled().inverse();
    depth = GetScene()->Picking(Proj*View, section.GetViewportSize(), currentSection, section.GetSceneLayerIndex(), GetStencilTestLayersIndexes(), x, y);
    if (depth != -1)
    {
        isMany = false;
        isPicked = true;
        return true;
    }
    else {
        depth = 0;
        return false;
    }
}

bool Renderer::TrySinglePicking(int x, int y)
{
    // Added: try to single picking when in many selected mode if the click is singular
    double dist = sqrt(pow(xWhenPress - x, 2) + pow(yWhenPress - y, 2));
    if (IsMany() && dist <= 3) {
        return Picking(x, y);
    }
    return false;
}


//void Renderer::OutLine()
//{
//    ActionDraw(0);
//}

void Renderer::PickMany(int x, int y)
{
    // Changed: pick allways
    WindowSection& section = *windowSections[currentSection];
    Eigen::Vector4i viewportSize = section.GetViewportSize();
    igl::opengl::Camera& currentCamera = *cameras[section.GetCamera()];
    int localPressY = yWhenPress - viewportSize.y();
    int localReleaseY = yold - viewportSize.y();
    int xMin = std::min(xWhenPress, xold) - viewportSize.x();
    int yMin = viewportSize.w() - std::max(localPressY, localReleaseY);
    int xMax = std::max(xWhenPress, xold) - viewportSize.x();
    int yMax = viewportSize.w() - std::min(localPressY, localReleaseY);
    UnPick();
    Eigen::Matrix4d Proj = currentCamera.GetViewProjection().cast<double>();
    Eigen::Matrix4d View = currentCamera.MakeTransScaled().inverse();
    depth = scn->AddPickedShapes(Proj*View, viewportSize, currentSection, section.GetSceneLayerIndex(), xMin, xMax, yMin, yMax, GetStencilTestLayersIndexes());
    if (depth != -1)
    {
        depth = (depth*2.0f - currentCamera.GetFar()) / (currentCamera.GetNear() - currentCamera.GetFar());
        isMany = true;
        isPicked = true;
    }
    else {
        depth = 0;
    }
}

//void Renderer::ActionDraw(int viewportIndx)
//{
//    if (menu)
//    {
//        menu->pre_draw();
//        menu->callback_draw_viewer_menu();
//    }
//    for (int i = 0; i < drawInfos.size(); i++)
//    {
//        if ((drawInfos[i]->flags & inAction) && viewportIndx == drawInfos[i]->viewportIndx)
//            draw_by_info(i);
//    }
//    if (menu)
//    {
//        menu->post_draw();
//
//    }
//}

IGL_INLINE void Renderer::resize(GLFWwindow* window,int w, int h)
	{
		post_resize(window,w, h);
	}
// _________________________
// |           *            |
// |           *            |
// |************************|
// |           *            |
// |           *            |
// _________________________

IGL_INLINE void Renderer::post_resize(GLFWwindow* window, int w, int h)
	{
        // hold old windows size
        int x = 0;
        int y = 0;
        for(auto & section : windowSections){
            Eigen::Vector4i viewport = section->GetViewportSize();
            x = std::max(x,viewport.x()+viewport.z());
            // TODO check this
            y = std::max(y,viewport.y()+viewport.w());
        }
        float ratio_x = (float)w/(float)x;
        float ratio_y = (float)h/(float)y;
        for(auto & section : windowSections){
            Eigen::Vector4i viewport = section->GetViewportSize();
            Eigen::Vector4i newViewport = Eigen::Vector4i((int)((float)viewport.x()*ratio_x),(int)((float)viewport.y()*ratio_y),(int)((float)viewport.z()*ratio_x),(int)((float)viewport.w()*ratio_y));
            section->SetViewportSize(newViewport);
        }
		if (callback_post_resize)
		{
			callback_post_resize(window, w, h);
		}
	}


void Renderer::MoveCamera(int cameraIndx, int type, float amt)
{
    switch (type)
    {
        case xTranslate:
            cameras[cameraIndx]->MyTranslate( Eigen::Vector3d(amt, 0, 0),1); //MakeTransNoScale was here
            break;
        case yTranslate:
            cameras[cameraIndx]->MyTranslate( Eigen::Vector3d(0, amt, 0),1); //MakeTransNoScale was here
            break;
        case zTranslate:
            cameras[cameraIndx]->MyTranslate(Eigen::Vector3d(0, 0, amt),1); //MakeTransNoScale was here
            break;
        case xRotate:
            cameras[cameraIndx]->MyRotate(Eigen::Vector3d(1, 0, 0), amt);
            break;
        case yRotate:
            cameras[cameraIndx]->RotateInSystem(Eigen::Vector3d(0, 1, 0), amt);
            break;
        case zRotate:
            cameras[cameraIndx]->MyRotate(Eigen::Vector3d(0, 0, 1), amt);
            break;
        case scaleAll:
            cameras[cameraIndx]->MyScale( Eigen::Vector3d(amt, amt,  amt));
            break;
        default:
            break;
    }
}

bool Renderer::CheckSection(int x, int y, int sectionIndex)
{
    Eigen::Vector4i sectionViewport = windowSections[sectionIndex]->GetViewportSize();
    return (sectionViewport.x() < x && sectionViewport.y() < y && 
        sectionViewport.z() + sectionViewport.x() > x && 
        sectionViewport.w() + sectionViewport.y() > y);
}

bool Renderer::UpdateSection(int newSection)
{
    if (currentSection != newSection)
    {
        std::cout << "replacing section " << currentSection << " with " << newSection << std::endl;
        isSelecting = false;
        currentSection = newSection;
        Pressed();
		UnPick();
        return false;
    }
    return true;
}

void Renderer::MouseProccessing(int button)
{
    // Changed: allways process mouse input
    WindowSection& section = *windowSections[currentSection];
    igl::opengl::Camera& camera = *cameras[section.GetCamera()];
    int xrel{}, yrel{};
    if (button == GLFW_MOUSE_BUTTON_MIDDLE)
    {
        xrel = zrel;
        yrel = zrel;
    }
    else
    {
        xrel = this->xrel;
        yrel = this->yrel;
    }

    Eigen::Vector4i viewport = section.GetViewportSize();
    scn->MouseProccessing
    (
        button,
        xrel, yrel,
        CalcMoveCoeff(camera, viewport.w()),
        camera.MakeTransd()
    );
}

float Renderer::CalcMoveCoeff(igl::opengl::Camera &camera, int size)
{
    return camera.CalcMoveCoeff(depth, size);
}

//unsigned int Renderer::AddBuffer(int infoIndx)
//{
//    CopyDraw(infoIndx, buffer, buffers.size());
//
//    DrawInfo* info = drawInfos.back();
//    info->SetFlags(stencilTest );
// 
//    info->SetFlags( clearDepth | clearStencil);
//    int width = viewports[info->viewportIndx].z(), height = viewports[info->viewportIndx].w();
//
//    unsigned int texId;
//    texId = scn->AddTexture(width, height, 0, COLOR);
//    scn->AddTexture(width, height, 0, DEPTH);
//    buffers.push_back(new igl::opengl::DrawBuffer(width, height, texId));
//
//    return texId;
//}

//int Renderer::Create2Dmaterial(int infoIndx, int code)
//{
//    std::vector<unsigned int> texIds;
//    std::vector<unsigned int> slots;
//    
//    unsigned int texId = AddBuffer(infoIndx);
//    texIds.push_back(texId);
//    slots.push_back(texId);
//    texIds.push_back(texId + 1);
//    slots.push_back(texId + 1);
//    
//    materialIndx2D = scn->AddMaterial((unsigned int*)&texIds[0], (unsigned int*)&slots[0], 2);
//
//    return materialIndx2D;
//}


//void Renderer::SetBuffers()
//{
//    AddCamera(Eigen::Vector3d(0, 0, 1), igl::opengl::CameraData(0, 1, 1, 10), 2);
//    int materialIndx = Create2Dmaterial(1,1);
//    scn->SetShapeMaterial(6, materialIndx);
//    SwapDrawInfo(2, 3);
//}

IGL_INLINE void Renderer::Init(igl::opengl::glfw::Viewer* scene, igl::opengl::CameraData cameraData, int pickingBits,igl::opengl::glfw::imgui::ImGuiMenu *_menu)
{
    scn = scene;
    menu = _menu;
    //MoveCamera(0, zTranslate, 10);
    Eigen::Vector4i viewport;
    glGetIntegerv(GL_VIEWPORT, viewport.data());
    buffers.push_back(new igl::opengl::DrawBuffer());
    maxPixX = viewport.z();
    maxPixY = viewport.w();
    if (menu)
    {
        menu->callback_draw_viewer_menu = [&]()
        {
            // Draw parent menu content
            auto temp = Eigen::Vector4i(0,0,0,0); // set imgui to min size and top left corner
            menu->draw_viewer_menu(this, *scn,cameras, cameraData, temp);
        };
    }
}


