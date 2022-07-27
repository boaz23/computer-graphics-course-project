#include "igl/opengl/glfw/renderer.h"
#include "Project.h"
#include "InputManager.h"

int main(int argc,char *argv[])
{
	std::list<int> x, y;
	x.push_back(DISPLAY_WIDTH/2);
	x.push_back(DISPLAY_WIDTH);
	y.push_back(DISPLAY_HEIGHT / 2);
	//y.push_back(DISPLAY_HEIGHT*2 / 3);
	y.push_back(DISPLAY_HEIGHT);
	Display disp = Display(DISPLAY_WIDTH, DISPLAY_HEIGHT, "OPENGL");
    igl::opengl::glfw::imgui::ImGuiMenu* menu = new igl::opengl::glfw::imgui::ImGuiMenu();
	igl::opengl::CameraData cameraData(CAMERA_ANGLE, DISPLAY_RATIO, NEAR, FAR);
    Renderer* rndr = new Renderer(cameraData);
	Project *scn = new Project();  //initializing scene
	scn->renderer = rndr;
	
    Init(disp,menu); //adding callback functions
	rndr->Init(scn, x, y, cameraData, 1, menu); // adding scene and viewports to the renderer
	scn->Init();    //adding shaders, textures, shapes to scene
    disp.SetRenderer(rndr);

	//rndr->AddViewport(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
	//rndr->CopyDraw(1, rndr->viewport, 1);
	//rndr->ClearDrawFlag(2, rndr->toClear | rndr->stencilTest);
	//rndr->SetDrawFlag(2, rndr->blend | rndr->inAction2 | rndr->scissorTest);

	//rndr->AddViewport(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
	//rndr->AddDraw(2, 0, 0, 0, rndr->inAction | rndr->onPicking | rndr->stencilTest | rndr->scaleAbit | rndr->stencil2);
    disp.launch_rendering(rndr);

	delete scn;
	delete rndr;
	delete menu;
	
	return 0;
}
