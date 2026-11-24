# Isometric Tanks

A single C++23 game project with internal gameplay, graphics, and application modules, rendered with OpenGL 3.3 core. There is no separately built or distributed SDK. No Unity runtime, engine, imported models, or texture packs.

**Default match:** one human team versus three independent AI teams (four-way FFA). First to three round wins. Eliminate all enemy teams; a round has a 90-second limit. On timeout, living tank count then remaining health determine the winner; a tie is a draw. Rounds restart automatically; match results can be restarted without closing the process.

## Arena reference

The map is an original, procedural reconstruction of the **Motor Murder** arena structure from Pummel Party, based on [gameplay footage](https://www.youtube.com/watch?v=KjItZ4WXACY&t=5524s), not extracted level data or assets. It uses a seven-sided perimeter, five outer cover groups, a large front four-by-two crate block, and broken diagonal central cover with connecting lanes. Dimensions are approximate rather than an exact source-map replica.

`src/game/world/arena.cpp` owns the layout. Rendering, tank confinement, spawn clearance, bot navigation, and swept shell impacts use the same polygon and oriented cover footprints. `Rules::arenaHalfExtent` sets the polygon's circumradius and scales the cover layout.

## Build and run (Windows 10/11)

Requirements: CMake 3.20+, a toolchain with C++23 mode enabled by CMake (such as a recent LLVM-MinGW or Visual Studio 2022 C++ toolset), and an OpenGL 3.3-capable driver. Bundled FreeGLUT 3.8.0, GLEW 2.3.1, and Dear ImGui 1.93.0 **sources** are built with your compiler; precompiled libraries under dependencies are not used. No packages are downloaded by CMake.

From PowerShell with `cmake` and `g++`/`mingw32-make` on PATH:

```powershell
.\compile_win.ps1
.\build\tanks.exe
```

Equivalent commands:

```powershell
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Visual Studio alternative (use a different build directory when changing generators):

```powershell
cmake -S . -B build-vs -G "Visual Studio 17 2022" -A x64
cmake --build build-vs --config Release --parallel
.\build-vs\Release\tanks.exe
```

The executable finds `shaders/` next to itself, independently of the working directory. Ship the four scene/depth shader files alongside it. `--shader-dir PATH` overrides the location. Shader compilation/linking, missing files, framebuffer failures, and rendering errors are reported to the console.

For a source submission/clean clone, include `dependencies/freeglut-3.8.0`, `dependencies/glew-2.3.1`, and `dependencies/imgui` with their licenses and source files. Dear ImGui is pinned to the official [v1.93.0 release](https://github.com/ocornut/imgui/releases/tag/v1.93.0); only its core and GLUT/OpenGL3 backends are bundled. Build products are ignored, dependency sources are not. Keep the existing upstream license notices.

For binary distribution, ship `tanks.exe`, its `shaders/` folder, and the helper-library licenses, including `dependencies/imgui/LICENSE.txt`. LLVM-MinGW builds link the compiler runtime statically, so `libc++.dll` and `libunwind.dll` are not required. ImGui uses its built-in font and needs no separate DLL or font asset.

## Controls

| Input | Action |
| --- | --- |
| Enter | Deploy / restart after match |
| WASD | Move relative to the camera |
| Z / X | Rotate turret left / right |
| Left mouse / Space | Fire while held |
| Right mouse drag | Orbit camera |
| Mouse wheel | Zoom |
| C | Restore the isometric camera, including zoom and projection |
| O | Toggle between orthographic (default) and perspective |
| L | Rotate sunlight |
| P | Pause simulation |
| R | Restart the match and clear scores |
| F1 | Print keybind help to the console |
| F3 | Toggle shadow mapping |
| Esc | Quit |

The mouse does not aim. Releasing the turn keys keeps the turret heading, independently of hull movement or camera orbit. Each active human tank has an OpenGL guide from its barrel to the first predicted shell impact (or the shell's range limit). It uses the same wall, cover, and friendly-fire rules as a real shell; it does not predict future target movement.

Turrets turn at a finite rate; aim before firing. Shells travel in the direction of the barrel, collide with cover, and cause splash damage. Camera controls remain usable while paused. Eliminated players can watch the remaining teams until the next round. Keybind help prints to the console at startup and on F1. The ImGui HUD keeps only compact, team-colored player cards along the bottom (health and team round-win score), a round timer, and essential deployment/pause/results controls. Interactive menus capture their input; passive score/health cards do not block tank controls.

Bots aim at your current position rather than leading your movement. They wait at least 0.6 seconds after selecting a new target before firing and 1.4–1.8 seconds before attempting another shot.

## Internal module layout

The root `CMakeLists.txt` lists all game sources under `add_executable(tanks ...)`, grouped by module. Add new `.cpp` files there; no module needs its own CMake file.

```text
src/
  core/                   Shared math and color types
    object/               GameObject, Component, and built-in Transform
  game/
    ai/                   Bot decisions, pathfinding, navigation state
    input/                Control behaviours and player input
    combat/               Firing, projectiles, effects, combat state
    match/                Rules, rosters, validation, rounds, scores
    physics/              Tank movement and collision queries
    render/               MeshRenderer, concrete renderer components, and drawing primitives
    world/                Arena geometry and simulation orchestration
    command.hpp           Player input and device-independent tank commands
    identity.hpp          Team and tank IDs
    limits.hpp            Shared gameplay capacities and timestep
  graphics/
    camera/               Camera controls, projection, 3D math
    resources/            Mesh, shader-program, shadow-map ownership
    scene/                Camera and shadow passes; OpenGL primitive submission
    ui/                   Compact ImGui score/health cards and phase menus
    renderer.hpp/.cpp     Renderer coordinator
    frame_info.hpp        Frame settings
  app/
    runtime/              Application loop, callback routing, timing
    platform/             Window/context and ImGui GLUT input adapter
    input/                Raw keyboard and pointer state; no action bindings
    cli/                  Launch options and help
  shaders/    File-loaded GLSL programs
  main.cpp    Entry point only
```

Gameplay has no OpenGL/GLUT dependency. The application samples devices and advances the simulation; components interpret gameplay bindings; graphics reads world state. Module-qualified includes resolve from `src/`.

Use `.hpp` files for shared types and declarations, and keep gameplay and rendering logic in `.cpp` files. Trivial accessors can stay inline. Template implementations such as `getComponent<T>()` stay in headers so callers can use their own component types.

### Game objects and components

`Tank`, `Obstacle`, `Projectile`, and `Effect` are game objects. Each has a `Transform` for ground position, yaw, and visual elevation. Input is optional: tanks and objects from `world.createObject()` automatically run their attached `ControlBehaviour` components, in attachment order, once per fixed tick while active and playing. Other components do not receive input callbacks.

`PlayerInput` translates the keyboard, primary button and camera orientation into a tank command. `BotInput` produces commands from its own navigation state. Other components can read held keys directly. ImGui capture suppresses gameplay input under interactive menus.

For example, this component raises a scene marker while **U** is held, without changing application code or importing GLUT:

```cpp
struct LiftMarker : tanks::ControlBehaviour {
    void updateControls(const tanks::PlayerInputs &inputs, float dt, tanks::World &) override {
        if (inputs.keyboard.isDown('u'))
            owner().transform().elevation += dt;
    }
};

auto &marker = world.createObject();
marker.addComponent<LiftMarker>();
```

Use `world.tank(id).addComponent<YourBehaviour>()` to add an input reaction alongside a tank's normal controls. Override `resetControls()` only if the behaviour stores state that should reset between rounds. The app, not these components, owns pause, camera shortcuts, and ImGui capture.

`world.light()` is a `DirectionalLight` game object. Its attached `LightInput` rotates its azimuth with L and toggles `castsShadows` on an F3 press. These scene controls remain live while paused. OpenGL uses the same light direction for shading and the shadow map; disabling the object leaves ambient lighting only.

To compose an additional cover object in a `World`:

```cpp
auto &cover = world.createObject();
cover.transform().position = {0.0f, 4.5f};
cover.addComponent<tanks::BoxCollider>(tanks::Vec2{1.0f, 0.5f}, 1.8f);
cover.addComponent<tanks::CoverRenderer>();
```

`BoxCollider` participates in movement, navigation, line of sight, and shell collision. `MeshRenderer` makes the cover visible; its `visible` flag affects only drawing. `setActive(false)` removes the object from both collision and rendering without removing its components.

For each camera or shadow pass, `SceneRenderer` visits active objects and calls `draw()` on every visible `MeshRenderer`. C++ selects the override for the attached type. For example, `TankRenderer::draw` draws the hull and turret through `DrawContext` and omits the aim guide in the shadow pass.

To add a model, inherit from `MeshRenderer`, override `void draw(const World&, DrawContext&) const`, and attach the new component. Use `owner()` to read its transform or other components. No change to the scene renderer is needed when drawing with existing primitives.

`getComponent<T>()` returns the most recently attached component of type `T` **or a derived type**, or `nullptr` if none matches. To visit all attachments in order, start at `firstComponent()` and advance with `nextComponent()` until it returns `nullptr`. A const object grants read-only access. Each exact type may still be attached only once, so different renderers or input behaviours can coexist without silently replacing each other.

The world owns its objects, and objects keep runtime attachments until destruction. Tanks attach their selected controller when created. Projectiles and effects reuse fixed pools.

When reading the implementation, think of a reference (`T&`) as using an existing object, not making a copy. A `std::unique_ptr<T>` means one owner is responsible for destroying that object. In `addComponent`, `std::move` hands that responsibility to the game object's list; it does **not** move the component in memory. `friend class GameObject` lets just that class set the component's private ownership links. Normal game code uses `addComponent` and `getComponent`, not those links.

### Runtime ownership

- `World` owns the roster, scene objects, match state, and combat pools. Only bots store a navigation path. IDs and component addresses remain stable across rounds.
- Graphics subsystems own their camera, mesh, shader, shadow-map, and ImGui resources. GPU handles cannot be copied; the object that creates each resource releases it on destruction.
- `Application` composes a window/context, input controller, and fixed-step timing. GLUT callbacks carry an application pointer instead of relying on a global application object. Its ImGui adapter forwards events, begins GUI frames, and respects input capture; the application applies HUD requests without allowing UI code to mutate gameplay directly.

Use a real-time accumulator and advance `World::step` at `fixedStep` (1/120 s), independently of rendering. Team identity remains separate from player or bot control, so roster changes do not require different damage, targeting, or victory logic.

Start with `src/main.cpp` and `src/app/runtime/application.cpp` for program flow and window shortcuts. Change tank key bindings in `src/game/input/player_input.cpp`, light controls in `src/game/world/directional_light.hpp`, bot decisions in `src/game/ai/ai.cpp`, and shared tank actions in `src/game/tank.cpp`. Rules and rosters live in `src/game/match/config.cpp`.

The original aggregate headers and monolithic source files have been removed, and all callers use the focused module headers. The project does not assign fictional academic contributors; put your actual module owners in the report.

## Dear ImGui HUD and OpenGL scene

The HUD uses Dear ImGui's default dark style and font. Each tank has a passive bottom window with a team-colored name, native HP progress bar, and team win count. Windows wrap into rows when needed and do not capture gameplay input. The timer, deployment, pause and results panels also use standard windows, text and buttons. Keybind help stays in the console.

The arena, tanks, projectiles, lighting, and shadow map remain custom OpenGL. ImGui's official OpenGL3 backend draws the HUD after the 3D scene; its official GLUT backend receives input through the game's existing user-data callbacks rather than replacing them. Each frame begins the OpenGL3 and GLUT backends, starts ImGui, builds the HUD, and renders its draw data. Backend/context shutdown occurs before the window/context is destroyed.

This keeps the gameplay module independent of both ImGui and OpenGL. The UI uses a built-in font and disables disk persistence, so it does not create `imgui.ini` or depend on working-directory UI assets. The vendored helper's backend shaders are third-party UI code, not the project's headline graphics technique.

## Graphics and assignment boundary

The demo implements file-loaded vertex/fragment shaders, Blinn-Phong lighting, directional shadow mapping, depth testing, back-face culling, perspective and orthographic camera modes, resize-safe projection, orbit/zoom, movable sunlight, and a mandatory visible **shadows on/off** comparison using F3. Scene geometry is procedural; HUD widgets and text come from Dear ImGui. Include the helper-library licenses in the distribution and report.

The default orthographic camera looks along a cube diagonal (45° yaw, 35.26° elevation), giving all three world axes equal foreshortening. Dirt, crates, and tracks are matte; painted armor catches highlights. Shadows block the sun's diffuse and specular light but leave ambient light intact. Cylinder walls use radial vertex normals for smooth lighting without extra triangles; the end caps stay flat. The tank uses separate hull and turret transforms so its world-space aim survives hull turns. The barrel tip defines the origin for the muzzle flash and aim guide.

The supplied `CG_Final_Project_Requirements_2026-2027 (1).docx` contains the common requirements but **does not contain Section 6, the assigned topic number, or its exact title**. Shadow mapping is a baseline graphics feature, not a claim that an unknown assigned technique has been implemented. Add the actual topic's required pipeline/toggle once that title is known. The final report, real contributor names, performance measurements on the lab PC, screenshots, citations, and defence video are submission work, not fabricated implementation results. Targeting 30 FPS at 1280x720 is not a claim of a measured lab result; use an external profiler and compare F3 on/off on that machine.

The Unity reference supplies design inspiration (colored tanks, health, rounds, last-surviving-side scoring), not a renderer, code dependency, or copied assets. This implementation uses independent turret aiming and fixed-speed shells rather than the reference's charge-to-launch mechanic.

References/libraries:

- [FreeGLUT](https://freeglut.sourceforge.net/), permissive license in `dependencies/freeglut-3.8.0/COPYING`.
- [GLEW](https://glew.sourceforge.net/), license notices in `dependencies/glew-2.3.1/LICENSE.txt`.
- [Dear ImGui v1.93.0](https://github.com/ocornut/imgui/tree/v1.93.0), MIT license in `dependencies/imgui/LICENSE.txt`; official GLUT and OpenGL3 backends, unmodified.
- [Unity Tanks tutorial](https://learn.unity.com/project/tanks-tutorial), conceptual reference only. The provided Unity project is not needed to build.
- [Pummel Party — Motor Murder gameplay](https://www.youtube.com/watch?v=KjItZ4WXACY&t=5524s), structural reference for arena cover placement and compact bottom HUD; no imported Pummel Party assets.
- Team-authored/custom game, geometry, UI, and shader source must be understood and explainable at the defence; disclose substantial AI-assisted implementation as required by the assignment.
