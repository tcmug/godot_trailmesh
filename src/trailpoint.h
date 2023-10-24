#ifndef TRAILPOINT_H
#define TRAILPOINT_H

#include <godot_cpp/variant/vector3.hpp>

using namespace godot;

class TrailPoint {
	public:
	  Vector3 center;
	  Vector3 direction_vector;
	  float size;
};

#endif
