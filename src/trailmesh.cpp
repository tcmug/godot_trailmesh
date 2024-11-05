#include "trailmesh.hpp"
// #include <godot_cpp/core/class_db.hpp>

#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)

using namespace godot;

void TrailMesh::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_num_points", "value"), &TrailMesh::set_num_points);
	ClassDB::bind_method(D_METHOD("get_num_points"), &TrailMesh::get_num_points);
	ClassDB::bind_method(D_METHOD("set_curve", "new_curve"), &TrailMesh::set_curve);
	ClassDB::bind_method(D_METHOD("get_curve"), &TrailMesh::get_curve);

	ClassDB::add_property("TrailMesh", PropertyInfo(Variant::INT, "num_points"), "set_num_points", "get_num_points");
	ClassDB::add_property("TrailMesh", PropertyInfo(Variant::OBJECT, "curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_curve", "get_curve");
}

TrailMesh::TrailMesh() {
	fade_away_timer = 0.0;
	num_points = 200;
	size = 1.0;
	uv_shift = 0.0;
	noise_scale = 0.0;
	elapsed = 0.0;
	total_elapsed = 0.0;
	update_interval = 0.1;
	trail_emitter = nullptr;
	trail_points = nullptr;
}

void TrailMesh::set_num_points(int value) {
	num_points = value;
	initialize_arrays();
}

void TrailMesh::initialize_arrays() {
	vertex_buffer.resize(num_points * 2);
	normal_buffer.resize(num_points * 2);
	tangent_buffer.resize(num_points * 2 * 4);
	uv_buffer.resize(num_points * 2);
	color_buffer.resize(num_points * 2);
	if (trail_points) {
		delete[] trail_points;
	}
	trail_points = new TrailPoint[num_points];
}

void TrailMesh::offset_mesh_points(Vector3 offset) {
	emitter_transform.origin += offset;
	for (int i = 0; i < num_points; i++) {
		trail_points[i].center += offset;
	}
}

int TrailMesh::get_num_points() const {
	return num_points;
}

void TrailMesh::set_curve(Ref<Curve> new_curve) {
	curve = new_curve;
}

Ref<Curve> TrailMesh::get_curve() const {
	return curve;
}

void TrailMesh::set_gradient(Ref<Gradient> new_gradient) {
	gradient = new_gradient;
}

Ref<Gradient> TrailMesh::get_gradient() const {
	return gradient;
}

TrailMesh::~TrailMesh() {
	if (trail_emitter) {
		trail_emitter->trail_mesh = nullptr;
	}
	delete[] trail_points;
}

void TrailMesh::update_transform() {
	if (trail_emitter) {
		emitter_transform = trail_emitter->get_global_transform();
	}
}

void TrailMesh::_ready() {
	initialize_arrays();
	update_transform();

	vertex_buffer.fill(emitter_transform.origin);

	geometry.resize(ArrayMesh::ARRAY_MAX);
	geometry[ArrayMesh::ARRAY_VERTEX] = vertex_buffer;
	geometry[ArrayMesh::ARRAY_NORMAL] = normal_buffer;
	geometry[ArrayMesh::ARRAY_TANGENT] = tangent_buffer;
	geometry[ArrayMesh::ARRAY_TEX_UV] = uv_buffer;
	if (gradient.is_valid()) {
		geometry[ArrayMesh::ARRAY_COLOR] = color_buffer;
	}
	ArrayMesh *mesh = memnew(ArrayMesh);
	set_mesh(Ref(mesh));

	// Initialize points.
	for (int i = 0; i < num_points; i++) {
		trail_points[i].center = to_local(emitter_transform.origin);
		trail_points[i].direction_vector.zero();
		trail_points[i].size = 0;
	}
}

void TrailMesh::_process(double delta) {
	Transform3D previous_emitter_transform = emitter_transform;
	update_transform();

	if (!trail_emitter) {
		// Handle removal
		size = 0.0;
		fade_away_timer += delta;
		if (fade_away_timer >= num_points * update_interval) {
			queue_free();
		}
	}

	Vector3 previous_position = to_local(previous_emitter_transform.origin);
	Vector3 current_position = to_local(emitter_transform.origin);
	Vector3 new_direction_vector = previous_position.direction_to(current_position);
	if (new_direction_vector.length() > 0.0) {
		direction_vector = new_direction_vector;
	}

	int num_vertices = vertex_buffer.size();
	elapsed += delta;
	total_elapsed += delta;
	if (elapsed >= update_interval) {
		elapsed -= update_interval;
		memmove(&trail_points[1], trail_points, sizeof(TrailPoint) * (num_points - 1));
	}

	float update_fraction = elapsed / update_interval;

	// Update active point.
	float spawn_size = size;
	if (noise_scale != 0.0) {
		noise_scale *= UtilityFunctions::randf_range(1.0 - noise_scale, 1.0 + noise_scale);
	}
	trail_points[0].center = current_position;
	trail_points[0].direction_vector = direction_vector;
	trail_points[0].size = spawn_size;
	Camera3D *camera = get_viewport()->get_camera_3d();

	if (camera) {
		// Transform points to the vertex buffer.
		const Vector3 camera_position = to_local(camera->get_global_position());

		int vi = 0, ci = 0, ni = 0, uvi = 0, ti = 0;

		// These are for visibility AABB:
		Vector3 min_pos = Vector3(10000, 10000, 10000);
		Vector3 max_pos = Vector3(-10000, -10000, -10000);

		for (int i = 0; i < num_points; i++) {
			Vector3 normal = trail_points[i].center.direction_to(camera_position);
			// Normalize for keeping sizes consistent.
			Vector3 orientation = normal.cross(trail_points[i].direction_vector).normalized();
			Vector3 tangent = trail_points[i].direction_vector;
			double sz = trail_points[i].size;

			if (curve.is_valid()) {
				sz *= curve->sample_baked((double(i + update_fraction) / double(num_points)));
			}

			Vector3 edge_vector = orientation * sz;
			Vector3 &position = trail_points[i].center;

			// Keep track of min and max points.
			min_pos.x = min(position.x, min_pos.x);
			min_pos.y = min(position.y, min_pos.y);
			min_pos.z = min(position.z, min_pos.z);
			max_pos.x = max(position.x, max_pos.x);
			max_pos.y = max(position.y, max_pos.y);
			max_pos.z = max(position.z, max_pos.z);

			vertex_buffer[vi++] = position + edge_vector;
			vertex_buffer[vi++] = position - edge_vector;

			normal_buffer[ni++] = normal;
			normal_buffer[ni++] = normal;

			tangent_buffer[ti++] = tangent.x;
			tangent_buffer[ti++] = tangent.y;
			tangent_buffer[ti++] = tangent.z;
			tangent_buffer[ti++] = 1;
			tangent_buffer[ti++] = tangent.x;
			tangent_buffer[ti++] = tangent.y;
			tangent_buffer[ti++] = tangent.z;
			tangent_buffer[ti++] = 1;

			double ux = i / double(num_points);
			ux -= ((uv_shift + 1.0) * ((total_elapsed / update_interval) / num_points));
			double x = ux + (update_fraction * (1.0 / num_points));
			uv_buffer[uvi++] = Vector2(x, 0);
			uv_buffer[uvi++] = Vector2(x, 1);

			if (gradient.is_valid()) {
				Color color = gradient->sample((double(i + update_fraction) / double(num_points)));
				color_buffer[ci++] = color;
				color_buffer[ci++] = color;
			}
		}

		set_custom_aabb(AABB(min_pos, max_pos - min_pos));

		Ref<ArrayMesh> mesh = get_mesh();
		if (mesh.is_valid()) {
			mesh->clear_surfaces();
			geometry[ArrayMesh::ARRAY_VERTEX] = vertex_buffer;
			geometry[ArrayMesh::ARRAY_NORMAL] = normal_buffer;
			geometry[ArrayMesh::ARRAY_TANGENT] = tangent_buffer;
			geometry[ArrayMesh::ARRAY_TEX_UV] = uv_buffer;
			if (gradient.is_valid()) {
				geometry[ArrayMesh::ARRAY_COLOR] = color_buffer;
			}
			mesh->add_surface_from_arrays(Mesh::PrimitiveType::PRIMITIVE_TRIANGLE_STRIP, geometry);
		}

		Ref<ShaderMaterial> material = get_material_override();
		if (material.is_valid()) {
			material->set_shader_parameter("MAX_VERTICES", float(num_vertices));
			material->set_shader_parameter("SPAWN_INTERVAL_SECONDS", float(update_interval));
		}
	}
}
