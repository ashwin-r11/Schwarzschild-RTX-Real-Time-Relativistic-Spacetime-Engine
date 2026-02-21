<div align="center">

# 🕳️ Schwarzschild-RTX

### Real-Time Relativistic Spacetime Engine

[![C++](https://img.shields.io/badge/C%2B%2B-20-blue?logo=cplusplus)](https://isocpp.org/)
[![OpenGL](https://img.shields.io/badge/OpenGL-3.3%20Core-green?logo=opengl)](https://www.opengl.org/)
[![GLSL](https://img.shields.io/badge/GLSL-330-orange)](https://www.khronos.org/opengl/wiki/OpenGL_Shading_Language)
[![GPU](https://img.shields.io/badge/NVIDIA-RTX%204070-76B900?logo=nvidia)](https://www.nvidia.com/)
[![License](https://img.shields.io/badge/License-MIT-yellow)](LICENSE)

A high-fidelity, GPU-accelerated Schwarzschild black hole simulator built from the ground up in **C++20** and **OpenGL**. This engine migrates heavy numerical integration from the CPU to **GLSL Fragment Shaders**, leveraging the **RTX 4070** to visualize the complex light-warping effects of General Relativity in real-time.

<!-- Replace with your actual screenshot -->
![Black Hole Simulation](docs/screenshots/hero.png)

</div>

---

## 🌌 Inspired by Reality: Beyond the "Interstellar" Look

Unlike basic artistic shaders, this simulation is grounded in the actual physics captured by the **Event Horizon Telescope (EHT)**. The color model is directly matched to the real M87* observation — not generic color choices.

<!-- Side-by-side comparison -->
<div align="center">

| Simulation Output                              | EHT M87* Observation                       |
| ---------------------------------------------- | ------------------------------------------ |
| ![Simulation](docs/screenshots/simulation.png) | ![M87](docs/screenshots/m87_reference.png) |

</div>

---

## 📐 Physics

### Schwarzschild Metric

The simulation models a **static, non-rotating** black hole using the Schwarzschild solution to Einstein's field equations. In natural units ($G = M = c = 1$), the metric takes the form:

$$ds^2 = -\left(1 - \frac{r_s}{r}\right)c^2\,dt^2 + \left(1 - \frac{r_s}{r}\right)^{-1}dr^2 + r^2\,d\Omega^2$$

where the Schwarzschild radius is:

$$r_s = \frac{2GM}{c^2} = 2.0 \quad \text{(natural units)}$$

### Geodesic Equation

Photons follow **null geodesics** — curves of zero spacetime interval. The effective acceleration that bends a photon's path is derived from the geodesic equation:

$$\vec{a} = -\frac{3M \cdot h^2}{r^5} \hat{r}$$

where:
- $h = |\vec{r} \times \vec{v}|$ is the specific angular momentum
- $r = |\vec{r}|$ is the radial distance from the singularity
- $\hat{r}$ is the radial unit vector

### RK4 Integration

The geodesic ODE is integrated using the **4th-order Runge-Kutta** method with adaptive step size:

$$\vec{y}_{n+1} = \vec{y}_n + \frac{dt}{6}\left(\vec{k}_1 + 2\vec{k}_2 + 2\vec{k}_3 + \vec{k}_4\right)$$

| Region                     | Step Size        | Purpose                                                  |
| -------------------------- | ---------------- | -------------------------------------------------------- |
| $r < 1.2 \cdot r_{photon}$ | $0.15 \times dt$ | Captures sharp geodesic curvature near the photon sphere |
| $r < 2.0 \cdot r_{photon}$ | $0.40 \times dt$ | Resolves photon ring sub-images                          |
| $r < 10M$                  | $0.70 \times dt$ | Accurate disk intersection detection                     |
| $r > 10M$                  | $1.0 \times dt$  | Full speed where curvature is minimal                    |

### Photon Sphere & Shadow

The **photon sphere** exists at:

$$r_{photon} = \frac{3}{2} r_s = 3M$$

Photons at this radius orbit the black hole in unstable circular orbits. The **black hole shadow** — the dark region visible in the simulation — has an apparent radius of:

$$r_{shadow} \approx \frac{3\sqrt{3}}{2} r_s \approx 2.6 \cdot r_s$$

### Accretion Disk

The disk lies in the equatorial plane ($y = 0$) between the **Innermost Stable Circular Orbit (ISCO)** and the outer boundary:

$$r_{ISCO} = 3 r_s = 6M \quad \xrightarrow{\text{natural units}} \quad 3.0$$

$$r_{outer} = 15M$$

Disk intersection is detected via **Y-plane crossing** — when a photon's $y$-coordinate changes sign during an RK4 step.

### Blackbody Temperature Gradient

The disk temperature follows the standard thin-disk profile:

$$T(r) = T_{max} \cdot \left(\frac{r_{inner}}{r}\right)^{3/4}$$

where $T_{max} \approx 9000\text{K}$ at the ISCO. The color is mapped via a **5-stop gradient** calibrated to the M87* EHT observation:

| Temperature            | Color          | Region       |
| ---------------------- | -------------- | ------------ |
| Coolest ($\sim 1500K$) | Dark red-brown | Outer edge   |
|                        | Deep red       | Mid-outer    |
|                        | Rich orange    | Mid disk     |
|                        | Golden yellow  | Inner region |
| Hottest ($\sim 9000K$) | Yellow-white   | ISCO         |

### Doppler Beaming

The accretion disk gas orbits at the **Keplerian velocity**:

$$v_{orb} = \sqrt{\frac{M}{r}}$$

The relativistic Doppler factor is:

$$\delta = \frac{1}{\gamma\left(1 - \vec{v}_{gas} \cdot \hat{n}\right)}$$

where $\gamma = (1 - v^2)^{-1/2}$ is the Lorentz factor and $\hat{n}$ is the direction toward the observer. The observed intensity scales as:

$$I_{obs} \propto \delta^3$$

This creates the characteristic **asymmetric brightness crescent** — the approaching side of the disk is dramatically brighter than the receding side.

### Gravitational Redshift

Photons escaping from near the event horizon lose energy. The redshift factor is:

$$z_{grav} = \sqrt{1 - \frac{r_s}{r}}$$

This dims and reddens light emitted close to the horizon, creating the smooth fade into the shadow.

### Photon Ring

The simulation traces each ray through **up to 4 disk crossings** using front-to-back alpha compositing. This produces:

1. **Primary image** — direct view of the disk
2. **Secondary image** — light bent 180° over the top (far side)
3. **Tertiary image** — light bent under the black hole (underside)
4. **Photon ring** — thin, bright ring at the shadow boundary from higher-order orbits

<!-- Photon ring diagram -->
![Photon Ring Anatomy](docs/screenshots/photon_ring_anatomy.png)

---

## 🏗️ Architecture

```
BlackHoleSim/
├── src/
│   ├── main.cpp                  # Entry point — input loop + uniform dispatch
│   ├── core/
│   │   ├── display.hpp           # GLFW window, shader compilation, fullscreen quad
│   │   └── camera.hpp            # Spherical orbit camera (CAD-style)
│   ├── math/
│   │   ├── Vec3.hpp              # Custom 3D vector (operator overloaded)
│   │   └── Vec4.hpp              # Custom 4D vector (homogeneous coords)
│   ├── physics/
│   │   └── raytracer.hpp         # Original C++ RK4 engine (reference/fallback)
│   └── shaders/
│       ├── blackhole.vert        # Vertex shader — passes UVs to fragment
│       └── blackhole.frag        # Fragment shader — ALL physics on GPU
├── tests/
│   └── math/
│       ├── vec3_test.cpp
│       └── vec4_test.cpp
├── CMakeLists.txt
└── README.md
```

### Rendering Pipeline

```
┌─────────────────────┐
│    CPU (main.cpp)    │
│                      │
│  1. Poll input       │
│  2. Update camera    │
│  3. Set 8 uniforms   │
│  4. glDrawArrays(6)  │  ──→  2 triangles (fullscreen quad)
└─────────────────────┘
           │
           ▼
┌─────────────────────────────────────────────────────┐
│              GPU — Fragment Shader                   │
│                                                     │
│  Per pixel (800×600 = 480,000 parallel threads):    │
│                                                     │
│  1. Compute ray direction from camera basis         │
│  2. RK4 integrate geodesic (up to 1000 steps)       │
│  3. Test disk crossings (up to 4)                   │
│  4. Compute blackbody color + Doppler + redshift    │
│  5. ACES tone map + gamma correct                   │
│  6. Output FragColor                                │
└─────────────────────────────────────────────────────┘
```

### CPU vs GPU Utilization

| Metric         | Before (Phase 4)                        | After (Phase 5)                  |
| -------------- | --------------------------------------- | -------------------------------- |
| CPU Usage      | **97%** (24 threads via `std::jthread`) | **< 2%** (uniform dispatch only) |
| GPU Usage      | **0%** (idle)                           | **Active** (RTX 4070)            |
| Framerate      | ~1 FPS                                  | **60 FPS** (VSync)               |
| Responsiveness | Unresponsive to input                   | Instant interaction              |

---

## 🔧 Build & Run

### Prerequisites

| Dependency        | Version         | Purpose                                    |
| ----------------- | --------------- | ------------------------------------------ |
| **CMake**         | ≥ 3.10          | Build system                               |
| **GCC / Clang**   | C++20 support   | Compiler                                   |
| **GLFW3**         | ≥ 3.3           | Window management + input                  |
| **GLAD**          | OpenGL 3.3 Core | OpenGL loader (included in `third_party/`) |
| **NVIDIA Driver** | ≥ 470           | GPU acceleration                           |

### Install Dependencies (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install cmake g++ libglfw3-dev
```

### Build

```bash
git clone https://github.com/Silvera0218/BlackHole-Simulation.git
cd BlackHole-Simulation
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Run

**On hybrid GPU laptops** (like the Lenovo Legion Pro with Intel iGPU + NVIDIA dGPU), you must force the NVIDIA GPU using PRIME offload:

```bash
# Force RTX 4070 (required on dual-GPU laptops)
__NV_PRIME_RENDER_OFFLOAD=1 __GLX_VENDOR_LIBRARY_NAME=nvidia ./BlackHoleSim
```

On desktop systems with only an NVIDIA GPU:

```bash
./BlackHoleSim
```

You should see in the terminal output:
```
OpenGL Version: 3.3.0 NVIDIA 570.211.01
GPU: NVIDIA GeForce RTX 4070 Laptop GPU/PCIe/SSE2
```

> [!WARNING]
> If you see `Mesa Intel(R) Graphics` instead, the simulation is running on the integrated GPU. Use the PRIME offload command above.

---

## 🎮 Controls

| Input                 | Action                              |
| --------------------- | ----------------------------------- |
| **Left-click + Drag** | Orbit camera around the black hole  |
| **Scroll Wheel**      | Zoom in / out (radius: 2.5 — 200)   |
| **W / S**             | Pan orbit center forward / backward |
| **A / D**             | Pan orbit center left / right       |
| **Q / E**             | Pan orbit center up / down          |
| **ESC**               | Quit                                |

The camera uses **spherical coordinates** (yaw, pitch, radius) with pitch clamped to ±89° to avoid gimbal lock — the standard approach used in CAD software.

---

## 🧮 Core Engine (Unmodified)

The mathematical foundation of this engine was hand-written from scratch. Phase 5 altered **only the rendering and display layers** — the core engine remains untouched:

| File                        | Status       | Description                                                      |
| --------------------------- | ------------ | ---------------------------------------------------------------- |
| `src/math/Vec3.hpp`         | ✅ Unmodified | 3D vector with operator overloading, `dot`, `cross`, `normalize` |
| `src/math/Vec4.hpp`         | ✅ Unmodified | 4D homogeneous vector                                            |
| `src/physics/raytracer.hpp` | ✅ Unmodified | Original C++ RK4 + Schwarzschild physics (reference)             |
| `CMakeLists.txt`            | ✅ Unmodified | Build configuration                                              |
| `tests/`                    | ✅ Unmodified | Vec3 / Vec4 unit tests                                           |

The GLSL fragment shader ports the **exact same equations** to the GPU — no physics were changed, only the execution platform.

---

## 📊 Shader Uniforms

These are the values passed from C++ to the GPU every frame:

| Uniform       | Type    | Description                           |
| ------------- | ------- | ------------------------------------- |
| `uResolution` | `vec2`  | Window dimensions (dynamic on resize) |
| `uTime`       | `float` | Elapsed time (drives disk animation)  |
| `uCamPos`     | `vec3`  | Camera position in world space        |
| `uCamForward` | `vec3`  | Camera look direction                 |
| `uCamRight`   | `vec3`  | Camera right basis vector             |
| `uCamUp`      | `vec3`  | Camera up basis vector                |
| `uFovScale`   | `float` | $\tan(\text{FOV}/2)$ — field of view  |
| `uStepSize`   | `float` | RK4 integration step (default: 0.08)  |

---

## 📸 Gallery

<!-- Replace these with actual screenshots from your simulation -->

<div align="center">

| Side View                                                                  | Face-On View                                                               |
| -------------------------------------------------------------------------- | -------------------------------------------------------------------------- |
| ![Side view showing gravitational lensing](docs/screenshots/side_view.png) | ![Face-on view showing temperature gradient](docs/screenshots/face_on.png) |

| Zoomed In                                                               | Zoomed Out                                                              |
| ----------------------------------------------------------------------- | ----------------------------------------------------------------------- |
| ![Close-up of accretion disk structure](docs/screenshots/zoomed_in.png) | ![Wide field showing lensed starfield](docs/screenshots/zoomed_out.png) |

</div>

---

## 🗺️ Roadmap

- [x] **Phase 1** — Linear Algebra Core (`Vec3`, `Vec4`)
- [x] **Phase 2** — Schwarzschild Metric + RK4 Integrator
- [x] **Phase 3** — Accretion Disk + Gravitational Lensing
- [x] **Phase 4** — CPU Multi-threading (`std::jthread`)
- [x] **Phase 5** — GPU Fragment Shader Migration
- [ ] **Phase 6** — Kerr Metric (rotating black hole, frame dragging)
- [ ] **Phase 7** — HDR Bloom Post-Processing
- [ ] **Phase 8** — Real Star Catalog Background (Hipparcos/Tycho)

---

## 📚 References

1. **Schwarzschild, K.** (1916). *Über das Gravitationsfeld eines Massenpunktes nach der Einsteinschen Theorie*. Sitzungsberichte der Königlich Preußischen Akademie der Wissenschaften.
2. **Event Horizon Telescope Collaboration** (2019). *First M87 Event Horizon Telescope Results. I. The Shadow of the Supermassive Black Hole*. The Astrophysical Journal Letters, 875(1), L1.
3. **James, O., von Tunzelmann, E., Franklin, P., & Thorne, K. S.** (2015). *Gravitational lensing by spinning black holes in astrophysics, and in the movie Interstellar*. Classical and Quantum Gravity, 32(6), 065001.
4. **Luminet, J.-P.** (1979). *Image of a spherical black hole with thin accretion disk*. Astronomy and Astrophysics, 75, 228-235.

---

## 📄 License

This project is licensed under the [MIT License](LICENSE).

---

<div align="center">

*Built with frustration, physics, and an underutilized GPU.*

**Ashwin** · 2026

</div>
