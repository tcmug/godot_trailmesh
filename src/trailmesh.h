#ifndef TRAILMESH__H
#define TRAILMESH__H

#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/packed_color_array.hpp>

#include <trailemitter.h>
#include <trailpoint.h>
namespace godot {

  class TrailMesh : public MeshInstance3D {
    GDCLASS(TrailMesh, MeshInstance3D)

      friend class TrailEmitter;

  private:
    int num_points;
    double elapsed;
    double total_elapsed;
    double update_interval;
    double uv_shift;
    double fade_away_timer;
    float size;
    float noise_scale;

    Array geometry;
    TrailEmitter* trail_emitter;
    Transform3D emitter_transform;
    Vector3 direction_vector;

    PackedVector3Array vertex_buffer;
    PackedVector3Array normal_buffer;
    PackedFloat32Array tangent_buffer;
    PackedVector2Array uv_buffer;
    PackedColorArray color_buffer;

    TrailPoint* trail_points;

    Ref<Curve> curve;
    Ref<Gradient> gradient;

    Vector3 get_cam_vector() const;
    void update_transform();

  protected:
    static void _bind_methods();
    void initialize_arrays();

  public:
    TrailMesh();
    ~TrailMesh();

    void _ready() override;
    void _process(double delta) override;

    int get_num_points() const;
    void set_num_points(int value);

    Ref<Curve> get_curve() const;
    void set_curve(const Ref<Curve> new_curve);

    Ref<Gradient> get_gradient() const;
    void set_gradient(const Ref<Gradient> new_gradient);

  };
}

#endif