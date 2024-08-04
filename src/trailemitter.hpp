#ifndef TRAILEMITTER__H
#define TRAILEMITTER__H

#include <godot_cpp/classes/curve.hpp>
#include <godot_cpp/classes/gradient.hpp>
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>

namespace godot {

class TrailMesh;

class TrailEmitter : public Node3D {
	GDCLASS(TrailEmitter, Node3D)

	friend class TrailMesh;

private:
	int num_points;
	float noise_scale;
	float size;
	double update_interval;
	double uv_shift;
	TrailMesh *trail_mesh;
	NodePath geometry_root;

	Ref<Gradient> gradient;
	Ref<Curve> curve;
	Ref<Material> material;

protected:
	static void _bind_methods();

public:
	TrailEmitter();
	~TrailEmitter();

	void _ready() override;

	NodePath get_geometry_root() const;
	void set_geometry_root(NodePath nodepath);

	Ref<Material> get_material() const;
	void set_material(const Ref<Material> material);

	int get_num_points() const;
	void set_num_points(int value);

	float get_size() const;
	void set_size(float value);

	Ref<Curve> get_curve() const;
	void set_curve(const Ref<Curve> new_curve);

	Ref<Gradient> get_gradient() const;
	void set_gradient(const Ref<Gradient> new_gradient);

	float get_noise_scale() const;
	void set_noise_scale(float value);

	double get_uv_shift() const;
	void set_uv_shift(double value);

	double get_update_interval() const;
	void set_update_interval(double value);

	void offset_mesh_points(Vector3 offset);
};
} //namespace godot

#endif
