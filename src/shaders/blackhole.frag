#version 330 core

// ============================================================
//  Schwarzschild Black Hole — GPU Ray Tracer v4
//  Color model tuned to match M87* EHT observation
//
//  Key: deep orange-red → golden-yellow palette
//  Soft emissive glow, Doppler crescent, pure black shadow
// ============================================================

in vec2 fragUV;
out vec4 FragColor;

// --- Uniforms ---
uniform vec2  uResolution;
uniform float uTime;
uniform vec3  uCamPos;
uniform vec3  uCamForward;
uniform vec3  uCamRight;
uniform vec3  uCamUp;
uniform float uFovScale;
uniform float uStepSize;

// --- Physics Constants ---
const float M          = 1.0;
const float RS         = 2.0;
const float DISK_INNER = 3.0;
const float DISK_OUTER = 15.0;
const float ESCAPE_R   = 50.0;
const int   MAX_STEPS  = 1000;
const float PHOTON_R   = 3.0;

// ============================================================
//  Hash & Noise
// ============================================================
float hash(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix(mix(hash(i), hash(i + vec2(1, 0)), f.x),
               mix(hash(i + vec2(0, 1)), hash(i + vec2(1, 1)), f.x), f.y);
}

float fbm(vec2 p) {
    float v = 0.0, a = 0.5;
    for (int i = 0; i < 4; i++) {
        v += a * noise(p);
        p *= 2.0;
        a *= 0.5;
    }
    return v;
}

// ============================================================
//  Schwarzschild geodesic acceleration
// ============================================================
vec3 accel(vec3 pos, vec3 vel) {
    float r2 = dot(pos, pos);
    float r  = sqrt(r2);
    vec3 h   = cross(pos, vel);
    float h2 = dot(h, h);
    float r5 = r2 * r2 * r;
    return pos * (-3.0 * M * h2 / r5);
}

// ============================================================
//  RK4 integrator
// ============================================================
void stepRK4(inout vec3 pos, inout vec3 vel, float dt) {
    vec3 k1v = accel(pos, vel);
    vec3 k1p = vel;
    vec3 k2v = accel(pos + k1p * (dt * 0.5), vel + k1v * (dt * 0.5));
    vec3 k2p = vel + k1v * (dt * 0.5);
    vec3 k3v = accel(pos + k2p * (dt * 0.5), vel + k2v * (dt * 0.5));
    vec3 k3p = vel + k2v * (dt * 0.5);
    vec3 k4v = accel(pos + k3p * dt, vel + k3v * dt);
    vec3 k4p = vel + k3v * dt;
    vel += (k1v + 2.0 * k2v + 2.0 * k3v + k4v) * (dt / 6.0);
    pos += (k1p + 2.0 * k2p + 2.0 * k3p + k4p) * (dt / 6.0);
}

// ============================================================
//  M87-matched color ramp
//  Maps normalized temperature [0,1] to the orange-red palette
//  observed in EHT imagery
//
//  0.0 = deep dark red (outer, coolest)
//  0.5 = rich orange
//  0.8 = golden yellow
//  1.0 = bright yellow-white (inner, hottest)
// ============================================================
vec3 m87ColorRamp(float t) {
    t = clamp(t, 0.0, 1.0);

    // 5-stop gradient matched to M87* EHT palette
    // Stop 0: very dark red-brown   (0.15, 0.02, 0.0)
    // Stop 1: deep red              (0.6,  0.08, 0.01)
    // Stop 2: rich orange           (0.95, 0.35, 0.04)
    // Stop 3: golden yellow         (1.0,  0.65, 0.12)
    // Stop 4: yellow-white          (1.0,  0.85, 0.45)

    vec3 c;
    if (t < 0.25) {
        float s = t / 0.25;
        c = mix(vec3(0.15, 0.02, 0.0), vec3(0.6, 0.08, 0.01), s);
    } else if (t < 0.5) {
        float s = (t - 0.25) / 0.25;
        c = mix(vec3(0.6, 0.08, 0.01), vec3(0.95, 0.35, 0.04), s);
    } else if (t < 0.75) {
        float s = (t - 0.5) / 0.25;
        c = mix(vec3(0.95, 0.35, 0.04), vec3(1.0, 0.65, 0.12), s);
    } else {
        float s = (t - 0.75) / 0.25;
        c = mix(vec3(1.0, 0.65, 0.12), vec3(1.0, 0.88, 0.5), s);
    }
    return c;
}

// ============================================================
//  Starfield
// ============================================================
vec3 starfield(vec3 dir) {
    vec2 uv = vec2(atan(dir.z, dir.x), asin(clamp(dir.y, -1.0, 1.0)));
    vec3 stars = vec3(0.0);

    vec2 g1 = floor(uv * 200.0);
    float s1 = hash(g1);
    stars += smoothstep(0.994, 1.0, s1) * mix(vec3(0.6, 0.65, 0.8), vec3(0.9, 0.85, 0.7), hash(g1 + 73.0)) * 0.8;

    vec2 g2 = floor(uv * 500.0);
    stars += smoothstep(0.997, 1.0, hash(g2)) * vec3(0.3, 0.3, 0.4) * 0.3;

    return stars;
}

// ============================================================
//  Disk shading — The disk IS flowing particles
//  Each dot orbits at ω(r) = √(M / r³), colored by M87 gradient.
//  Angular grid scaled by r to prevent arc-length stretching.
// ============================================================

// Helper: generate one particle layer
// rScale = radial grid density
// aScale = angular grid per unit radius (multiplied by diskR internally)
float particleLayer(float diskR, float angle, float time,
                    float rScale, float aScale,
                    float dotSize, float threshold, float seed) {
    float omega = sqrt(M / (diskR * diskR * diskR));
    float flowAngle = angle + time * omega;

    // KEY FIX: angular cells scale with r so dots have uniform arc-length
    vec2 cell = vec2(diskR * rScale, flowAngle * aScale * diskR);
    vec2 cellId = floor(cell);
    vec2 cellUV = fract(cell);

    // Random position within cell (avoids grid artifacts)
    float rnd = hash(cellId + seed);
    float rnd2 = hash(cellId + seed + 37.0);
    vec2 particlePos = vec2(rnd * 0.6 + 0.2, rnd2 * 0.6 + 0.2);
    vec2 delta = cellUV - particlePos;

    // Uniform small dot
    float dist = length(delta);
    float particle = smoothstep(dotSize, dotSize * 0.15, dist);

    // Spawn probability
    float spawn = smoothstep(threshold, threshold + 0.04, hash(cellId + seed + 71.0));

    // Flicker
    float flicker = 0.65 + 0.35 * sin(rnd * 50.0 + time * (2.0 + rnd * 3.0));

    return particle * spawn * flicker;
}

vec3 diskShade(vec3 hitPos, float diskR, vec3 camPos) {

    float r_ratio  = DISK_INNER / diskR;
    float tempNorm = pow(r_ratio, 0.75);
    float angle    = atan(hitPos.z, hitPos.x);

    // --- Doppler beaming ---
    vec3 radialDir = normalize(vec3(hitPos.x, 0.0, hitPos.z));
    vec3 orbitDir  = normalize(cross(vec3(0.0, 1.0, 0.0), radialDir));
    float v_orb    = sqrt(M / diskR);
    vec3 toCamera  = normalize(camPos - hitPos);
    float v_dot_n  = dot(orbitDir * v_orb, toCamera);
    float gamma    = 1.0 / sqrt(max(1.0 - v_orb * v_orb, 0.01));
    float doppler  = 1.0 / (gamma * (1.0 - v_dot_n));

    float dopplerTemp = clamp(tempNorm * doppler, 0.0, 1.0);
    vec3 baseColor = m87ColorRamp(dopplerTemp);

    // === BUILD DISK FROM PARTICLES (8 layers, uniform small dots) ===
    float density = 0.0;

    // Dense base layers (many tiny dots — form the body of the disk)
    density += particleLayer(diskR, angle, uTime, 15.0, 5.0, 0.10, 0.20, 0.0)   * 0.30;
    density += particleLayer(diskR, angle, uTime, 13.0, 4.5, 0.10, 0.22, 53.0)  * 0.30;
    density += particleLayer(diskR, angle, uTime, 11.0, 4.0, 0.11, 0.25, 113.0) * 0.35;
    density += particleLayer(diskR, angle, uTime, 9.0,  3.5, 0.11, 0.28, 197.0) * 0.35;

    // Medium density layers (visible individual dots)
    density += particleLayer(diskR, angle, uTime, 7.0,  3.0, 0.11, 0.40, 257.0) * 0.45;
    density += particleLayer(diskR, angle, uTime, 5.5,  2.5, 0.12, 0.45, 337.0) * 0.50;

    // Sparse bright dots (stand out, bloom catches them)
    density += particleLayer(diskR, angle, uTime, 4.0,  2.0, 0.12, 0.65, 431.0) * 0.70;
    density += particleLayer(diskR, angle, uTime, 3.0,  1.5, 0.12, 0.80, 619.0) * 1.0;

    // Faint diffuse glow underneath
    float omega = sqrt(M / (diskR * diskR * diskR));
    float flowAngle = angle + uTime * omega;
    density += fbm(vec2(diskR * 3.0, flowAngle * 5.0)) * 0.08;

    density = clamp(density, 0.0, 2.5);

    // --- Apply color and physics ---
    vec3 color = baseColor * density;

    float intensity = pow(clamp(doppler, 0.15, 3.5), 3.0);
    color *= intensity;

    float grav = sqrt(max(1.0 - RS / diskR, 0.0));
    color *= grav;

    float emission = pow(r_ratio, 1.5);
    color *= (0.3 + 0.7 * emission);

    float outerFade = smoothstep(DISK_OUTER, DISK_OUTER - 3.0, diskR);
    float innerFade = smoothstep(DISK_INNER - 0.3, DISK_INNER + 0.5, diskR);
    color *= outerFade * innerFade;

    return color;
}

// ============================================================
//  Ray tracer with multiple disk crossings
// ============================================================
vec3 traceRay(vec3 rayPos, vec3 rayDir) {
    vec3 pos = rayPos;
    vec3 vel = normalize(rayDir);

    vec3 accumulated = vec3(0.0);
    float transmittance = 1.0;
    int diskHits = 0;

    for (int i = 0; i < MAX_STEPS; i++) {
        float old_y = pos.y;
        float r = length(pos);

        // --- Capture ---
        if (r <= RS) {
            return accumulated; // Pure black shadow — no glow inside
        }

        // --- Escape ---
        if (r > ESCAPE_R) {
            accumulated += transmittance * starfield(normalize(vel));
            return accumulated;
        }

        // --- Adaptive step ---
        float dt;
        if (r < PHOTON_R * 1.2)
            dt = uStepSize * 0.15;
        else if (r < PHOTON_R * 2.0)
            dt = uStepSize * 0.4;
        else if (r < 10.0)
            dt = uStepSize * 0.7;
        else
            dt = uStepSize;

        stepRK4(pos, vel, dt);
        float new_y = pos.y;

        // --- Disk crossing ---
        if ((old_y > 0.0 && new_y <= 0.0) || (old_y < 0.0 && new_y >= 0.0)) {
            float t_hit = old_y / (old_y - new_y);
            vec3 hitPos = pos - vel * dt * (1.0 - t_hit);
            float diskR = length(vec2(hitPos.x, hitPos.z));

            if (diskR >= DISK_INNER && diskR <= DISK_OUTER) {
                diskHits++;

                vec3 dColor = diskShade(hitPos, diskR, rayPos);

                // Opacity decreases for higher-order crossings (photon ring)
                float opacity;
                if (diskHits == 1) opacity = 0.85;
                else if (diskHits == 2) opacity = 0.6;
                else opacity = 0.4;

                // Front-to-back compositing
                accumulated += transmittance * dColor * opacity;
                transmittance *= (1.0 - opacity);

                if (transmittance < 0.01 || diskHits >= 4) {
                    return accumulated;
                }
            }
        }
    }

    // Didn't terminate — add faint background
    accumulated += transmittance * vec3(0.002, 0.001, 0.003);
    return accumulated;
}

// ============================================================
//  Post-process: photon sphere glow (HDR values for bloom)
// ============================================================
vec3 photonGlow(vec3 rayDir, vec3 camPos) {
    vec3 cp = cross(camPos, rayDir);
    float b = length(cp);
    float shadowR = 2.6 * RS;

    float dist = abs(b - shadowR);
    // Stronger glow — bloom will spread this
    float ring = exp(-dist * dist * 2.0) * 0.15;
    float halo = exp(-dist * 0.3) * 0.03;

    return vec3(1.0, 0.6, 0.2) * (ring + halo);
}

// ============================================================
//  MAIN — outputs raw HDR linear color (no tone mapping here)
//  Tone mapping + gamma happen in bloom_final.frag
// ============================================================
void main() {
    vec2 uv = fragUV * 2.0 - 1.0;
    float aspect = uResolution.x / uResolution.y;
    uv.x *= aspect;

    vec3 rayDir = normalize(
        uCamForward +
        uCamRight * (uv.x * uFovScale) +
        uCamUp    * (uv.y * uFovScale)
    );

    vec3 color = traceRay(uCamPos, rayDir);

    // Add photon sphere glow (HDR — bloom will spread this)
    color += photonGlow(rayDir, uCamPos);

    // Output raw HDR linear — DO NOT tone map here
    // Tone mapping + gamma happen in the bloom composite pass
    FragColor = vec4(color, 1.0);
}
