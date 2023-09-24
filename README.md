![trailmesh-logo](https://github.com/tcmug/godot_trailmesh/assets/567207/e4db668e-1a7f-43e4-8489-b978dfe04f4c)

TrailMesh allows you to add 3D trails via an emitter (TrailEmitter), have the shape defined by a curve and colored with a gradient.

Used to implement continuous trails, where an emitter is attached to a physics body which leaves a trail behind when it is moved (for example missile or airplane trails).

Caveats:
  - Does not yet work in editor
  - Visibility is a bit buggy
  - Editor missing UI hints

| ![trails](https://github.com/tcmug/godot_trailmesh/assets/567207/25cadf5d-d880-481d-be84-c553869ed996) | ![trails2](https://github.com/tcmug/godot_trailmesh/assets/567207/3f8cb376-6ef4-4b0c-8dc8-f42c09f5e3e0) |
|-|-|

## Getting Started

1. Add the trail emitter as a child of the object. This could be a ship, a character, a sword, or a projectile.

![nodetree](./docs/assets/nodetree.png)

2. In the inspector, set the `GeometryNodepath` to some parent of the object. Usually, this will be your root node for the scene.

![emittersettings](./docs/assets/emittersettings.png)

3. Move the object during

![basic-example](./docs/assets/basic-example.gif)

Setup implementation from: <https://github.com/nathanfranke/gdextension>
