#pragma once
#ifndef PROJECT_MESH_H
#define PROJECT_MESH_H

#include "igl/opengl/ViewerData.h"

class ProjectMesh: public igl::opengl::ViewerData {
public:
	ProjectMesh(int layer, bool isPickable, bool outline)
		: igl::opengl::ViewerData(layer), 
		pickable{isPickable},
		outline{outline}
	{}

	inline bool IsPickable() { return pickable; }
	inline bool DrawOutline() { return outline; }

	bool pickable;
	bool outline;
};
#endif