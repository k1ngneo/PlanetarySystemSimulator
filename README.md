# 🌌 Planetary System Simulator

A C++ hobby project that simulates a **planetary system** with **Newtonian gravity** and renders it in **real-time 3D** using **OpenGL**.  
The project combines **physics**, **graphics**, and **user interactivity** into a cohesive educational tool for visualizing orbital mechanics.

---

## ✨ Features

- **Physics Simulation**
  - Newtonian gravity (`F = G * m1 * m2 / r^2`)
  - Dynamic body integration with velocity + position updates
  - Predictive path calculation (future orbit trajectory estimation)
  - Support for **static** and **dynamic** bodies

- **Celestial Bodies**
  - **Planets**: Have physical properties (mass, radius, velocity) and visual ones (color, size)
  - **Stars**: Special planets that also act as light sources for rendering

- **Rendering**
  - OpenGL-based real-time rendering
  - **Shader pipeline** with vertex + fragment shaders
  - **Orbit path rendering** (predicted trajectories visualized as lines)
  - **Skybox** for immersive space backgrounds
  - **Debug renderer** for development visualization

- **Camera System**
  - **First-person mode** (free-fly navigation)
  - **Orbital mode** (rotate around a point of interest)
  - Smooth motion with mouse & keyboard input
  - Configurable movement speed and sensitivity

- **Performance**
  - Time-step clamping (ensures stable simulation at different frame rates)
  - Optional iteration skipping (prevents instability during lag spikes)
  - Efficient vector math using **GLM**

---

## 🏗️ Project Structure

```
StarSystemSim/
│
├── physics/              # Physics simulation
│   ├── body.h/.cpp       # Base Body class (position, velocity, mass, type)
│   ├── engine.h/.cpp     # Physics engine (gravity, updates, prediction)
│
├── render/               # Rendering system
│   ├── camera.h/.cpp     # First-person + orbital camera
│   ├── renderer.h/.cpp   # OpenGL renderer (shaders, skybox, orbit paths)
│
├── objects/              # Celestial bodies
│   ├── planet.h/.cpp     # Planet class (radius, color, extends Body)
│   ├── star.h/.cpp       # Star class (extends Planet, emits light)
│
├── shaders/              # GLSL shaders for rendering
│
├── resources/            # Skybox textures, models, configs
│
└── main.cpp              # Application entry point
```

---

## ⚙️ How It Works

### 1. Physics Engine
The **Engine** manages all simulation steps:
- Updates time delta using a `Timer`
- Applies gravitational forces between all pairs of bodies
- Advances positions by velocity × delta time
- Calculates **future positions** for orbit visualization (by simulating ahead in small timesteps)

Gravity is applied symmetrically:

```cpp
F = G * (m1 * m2) / r^2
```

Bodies gain acceleration based on their mass and relative position.  
Predicted paths are stored in `m_PosPrediction` and later rendered.

---

### 2. Camera System
Two camera types are available:
- **First-person** (fly freely like in an FPS)
- **Orbital** (rotate around a planet or star with a fixed pivot)

Controlled with mouse + keyboard:
- WASD → movement
- Mouse → look
- Scroll → zoom

---

### 3. Rendering Pipeline
The **Renderer** handles all visuals:
- Initializes shaders (vertex + fragment)
- Loads and renders a **skybox** cube map
- Draws planets and stars (as colored spheres)
- Renders predicted orbit paths (line segments)
- Sets up OpenGL states (depth test, blending, etc.)

The **Star** also provides a light source for illumination.

---

### 4. Celestial Bodies
- **Body (base)** → mass, position, velocity, type
- **Planet** → adds radius + color, rendered as a sphere
- **Star** → extends Planet, acts as a **light source**

This allows flexibility: e.g., a binary star system, moons orbiting planets, etc.

---

## 🖥️ Dependencies

- **C++17 or newer**
- [GLM](https://github.com/g-truc/glm) – OpenGL Mathematics
- [OpenGL 3.3+] – Graphics rendering
- [GLEW / GLAD](https://github.com/Dav1dde/glad) – OpenGL function loader
- [GLFW](https://www.glfw.org/) – Windowing and input
- [stb_image](https://github.com/nothings/stb) – Image loading (skybox textures)

---

## 🚀 Building

### Linux / macOS
```bash
git clone git@github.com:k1ngneo/PlanetarySystemSimulator.git
cd PlanetarySystemSimulator/
mkdir build/ && cd build/
cmake .. && make
cd ..
./PlanetarySystemSimulator
```

### Windows (Visual Studio)
1. Install dependencies (GLFW, GLM, stb, OpenGL)
2. Open project in Visual Studio
3. Build in Release mode
4. Run from the IDE or build directory

---

## 🎮 Usage

- **Start** → Simulation begins with a set of planets and a star
- **Controls**:
  - `WASD` → Move camera
  - `Mouse` → Look around
  - `Space` → Pause/Resume simulation
  - `Shift` → Speed up time
  - `Ctrl` → Slow down time
- **Predicted Paths** → Displayed as glowing lines showing orbits

---

## 🔮 Future Improvements

- Add collision detection (planet impacts, mergers)
- Support for elliptical orbit initialization
- GPU-based physics (compute shaders)
- Textured planets with rotation
- UI overlay for controlling simulation parameters

---

## 📸 Screenshots

![planetarysim1](https://user-images.githubusercontent.com/13601585/189571157-ca51ec3e-ecaf-4b3d-b3ca-1403a55486df.png)
![planetarysim2](https://user-images.githubusercontent.com/13601585/189571177-807017fc-7fa3-4cca-bc37-1739a6571c86.png)

---

## 🧑‍💻 Author

Developed by **Kacper Gruszka**  
This is a hobby project for learning **C++**, **OpenGL**, and **physics simulation**.
