#include "igl/opengl/glfw/renderer.h"
#include "Project.h"
#include "InputManager.h"

int main(int argc,char *argv[])
{
	const int DISPLAY_WIDTH = 1200;
	const int DISPLAY_HEIGHT = 800;
	const float CAMERA_ANGLE = 45.0f;
	const float NEAR = 1.0f;
	const float FAR = 120.0f;
	const int infoIndx = 2; 
	std::list<int> x, y;
	x.push_back(DISPLAY_WIDTH);
	y.push_back(DISPLAY_HEIGHT);
    Display disp = Display(DISPLAY_WIDTH, DISPLAY_HEIGHT, "OPENGL");
    igl::opengl::glfw::imgui::ImGuiMenu* menu = new igl::opengl::glfw::imgui::ImGuiMenu();
    Renderer* rndr = new Renderer(CAMERA_ANGLE, (float)DISPLAY_WIDTH/(float)DISPLAY_HEIGHT, NEAR, FAR);
	Project *scn = new Project();  //initializing scene
	
    Init(disp,menu); //adding callback functions
	scn->Init();    //adding shaders, textures, shapes to scene
    rndr->Init(scn,x,y,1,menu); // adding scene and viewports to the renderer
	disp.SetRenderer(rndr);

	rndr->AddViewport(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
	rndr->CopyDraw(1, rndr->viewport, 1);
	rndr->ClearDrawFlag(2, rndr->toClear | rndr->stencilTest);
	rndr->SetDrawFlag(2, rndr->blend | rndr->inAction2 | rndr->scissorTest);

	rndr->AddViewport(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
	rndr->AddDraw(2, 0, 0, 0, rndr->inAction | rndr->depthTest | rndr->blend | rndr->clearStencil | rndr->passStencil | rndr->onPicking | rndr->stencilTest | rndr->scaleAbit);
    disp.launch_rendering(rndr);

	delete scn;
	delete rndr;
	delete menu;
	
	return 0;
}

