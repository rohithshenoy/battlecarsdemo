#version 330 core

in vec2 vTexCoord;
out vec4 FragColor;

uniform vec3 spriteColor;
uniform int shapeType;
uniform float turboGlow;
uniform int backgroundTheme;
uniform float timeSeconds;
uniform vec3 resolution;

float hash21(vec2 p)
{
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

float starField(vec2 uv, float density, float twinkle)
{
    vec2 grid = floor(uv * density);
    vec2 cell = fract(uv * density) - 0.5;
    float n = hash21(grid);
    float star = smoothstep(0.45, 0.0, length(cell + (hash21(grid + 17.0) - 0.5) * 0.35));
    float brightness = step(0.92, n) * star;
    brightness *= 0.55 + 0.45 * sin(timeSeconds * (2.0 + n * 4.0) + n * 20.0) * twinkle
                  + (1.0 - twinkle);
    return brightness * (0.5 + n * 0.5);
}

float softNoise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    float a = hash21(i);
    float b = hash21(i + vec2(1.0, 0.0));
    float c = hash21(i + vec2(0.0, 1.0));
    float d = hash21(i + vec2(1.0, 1.0));
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

float fbm(vec2 p)
{
    float value = 0.0;
    float amplitude = 0.5;
    for (int i = 0; i < 5; ++i) {
        value += amplitude * softNoise(p);
        p *= 2.02;
        amplitude *= 0.5;
    }
    return value;
}

vec3 classicBackground(vec2 uv)
{
    return vec3(0.0);
}

vec3 nebulaBackground(vec2 uv)
{
    vec2 p = uv * vec2(resolution.x / max(resolution.y, 1.0), 1.0);
    float drift = timeSeconds * 0.015;

    float n1 = fbm(p * 2.2 + vec2(drift, -drift * 0.7));
    float n2 = fbm(p * 3.1 + vec2(-drift * 1.3, drift * 0.4) + 8.0);
    float n3 = fbm(p * 1.4 + vec2(drift * 0.5, drift) + 20.0);

    vec3 deep = vec3(0.02, 0.01, 0.06);
    vec3 purple = vec3(0.45, 0.12, 0.55);
    vec3 cyan = vec3(0.08, 0.35, 0.65);
    vec3 magenta = vec3(0.55, 0.08, 0.28);

    vec3 color = deep;
    color = mix(color, purple, smoothstep(0.35, 0.75, n1) * 0.85);
    color = mix(color, cyan, smoothstep(0.4, 0.8, n2) * 0.55);
    color = mix(color, magenta, smoothstep(0.45, 0.85, n3) * 0.4);
    color += vec3(0.08, 0.05, 0.12) * n1 * n2;

    float stars = starField(p + vec2(0.0, drift * 0.2), 48.0, 1.0)
                + starField(p * 1.7 + 3.0, 90.0, 1.0) * 0.65;
    color += vec3(stars);

    return color;
}

vec3 neonBackground(vec2 uv)
{
    vec2 p = uv;
    float horizon = 0.42;
    vec3 color;

    if (p.y > horizon) {
        float t = (p.y - horizon) / (1.0 - horizon);
        vec3 top = vec3(0.05, 0.0, 0.18);
        vec3 mid = vec3(0.55, 0.05, 0.45);
        vec3 glow = vec3(1.0, 0.25, 0.55);
        color = mix(glow, mid, smoothstep(0.0, 0.35, t));
        color = mix(color, top, smoothstep(0.25, 1.0, t));

        float sunY = horizon + 0.12;
        float sunDist = length((p - vec2(0.5, sunY)) * vec2(1.4, 2.2));
        float sun = smoothstep(0.18, 0.0, sunDist);
        color = mix(color, vec3(1.0, 0.55, 0.2), sun * 0.9);

        float band = abs(fract((p.y - horizon) * 18.0 - timeSeconds * 0.15) - 0.5);
        color *= 1.0 - (1.0 - smoothstep(0.0, 0.08, band)) * sun * 0.35;

        float stars = starField(p * vec2(1.6, 1.0) + vec2(0.0, timeSeconds * 0.01), 55.0, 0.6);
        color += vec3(0.8, 0.9, 1.0) * stars * smoothstep(0.55, 0.95, p.y) * 0.55;
    } else {
        float groundT = p.y / horizon;
        vec3 near = vec3(0.08, 0.0, 0.14);
        vec3 far = vec3(0.25, 0.02, 0.28);
        color = mix(near, far, groundT);

        float perspective = 1.0 / max(0.08, (horizon - p.y) + 0.08);
        float gridX = abs(fract((p.x - 0.5) * perspective * 8.0 + 0.5) - 0.5);
        float gridZ = abs(fract((horizon - p.y) * perspective * 3.5 - timeSeconds * 0.8) - 0.5);
        float lineX = 1.0 - smoothstep(0.0, 0.04, gridX);
        float lineZ = 1.0 - smoothstep(0.0, 0.035, gridZ);
        float grid = max(lineX, lineZ) * smoothstep(0.0, 0.15, groundT);

        vec3 neonCyan = vec3(0.1, 0.95, 1.0);
        vec3 neonPink = vec3(1.0, 0.15, 0.7);
        color = mix(color, mix(neonPink, neonCyan, p.x), grid * 0.85);

        float haze = exp(-pow((horizon - p.y) * 6.0, 2.0));
        color = mix(color, vec3(1.0, 0.3, 0.65), haze * 0.35);
    }

    return color;
}

vec3 auroraBackground(vec2 uv)
{
    vec2 p = uv * vec2(resolution.x / max(resolution.y, 1.0), 1.0);
    vec3 color = vec3(0.01, 0.02, 0.06);

    for (int i = 0; i < 4; ++i) {
        float fi = float(i);
        float warp = fbm(vec2(p.x * 1.8 + timeSeconds * (0.12 + fi * 0.04), p.y * 0.8 + fi));
        float ridge = abs(p.x - 0.35 - fi * 0.18 + sin(p.y * 3.0 + timeSeconds * 0.4 + fi) * 0.08
                          + (warp - 0.5) * 0.35);
        float curtain = exp(-ridge * ridge * 28.0)
                        * smoothstep(0.08, 0.45, p.y)
                        * (1.0 - smoothstep(0.72, 1.05, p.y));
        float shimmer = 0.65 + 0.35 * sin(p.y * 18.0 - timeSeconds * 2.0 + fi * 2.0);
        vec3 tint = mix(vec3(0.1, 0.95, 0.45), vec3(0.55, 0.2, 0.95), fi / 3.0);
        tint = mix(tint, vec3(0.2, 0.95, 0.9), 0.5 + 0.5 * sin(p.x * 5.0 + timeSeconds + fi));
        color += tint * curtain * shimmer * (0.55 - fi * 0.08);
    }

    float stars = starField(p, 42.0, 1.0) + starField(p * 2.1 + 5.0, 80.0, 1.0) * 0.5;
    color += vec3(stars) * 0.85;
    return color;
}

vec3 deepSpaceBackground(vec2 uv)
{
    vec2 p = uv * vec2(resolution.x / max(resolution.y, 1.0), 1.0);
    float n = fbm(p * 1.8 + timeSeconds * 0.01);
    vec3 color = mix(vec3(0.0, 0.01, 0.04), vec3(0.04, 0.02, 0.12), n);
    color = mix(color, vec3(0.08, 0.12, 0.28), smoothstep(0.55, 0.9, n) * 0.4);

    float mist = fbm(p * 3.0 - timeSeconds * 0.02 + 10.0);
    color += vec3(0.05, 0.08, 0.18) * mist * 0.35;

    float stars = starField(p, 36.0, 0.8) + starField(p * 2.4 + 9.0, 100.0, 1.0) * 0.7;
    color += vec3(0.75, 0.85, 1.0) * stars;
    return color;
}

vec3 renderBackground(vec2 uv)
{
    if (backgroundTheme == 1) {
        return nebulaBackground(uv);
    }
    if (backgroundTheme == 2) {
        return neonBackground(uv);
    }
    if (backgroundTheme == 3) {
        return auroraBackground(uv);
    }
    if (backgroundTheme == 4) {
        return deepSpaceBackground(uv);
    }
    return classicBackground(uv);
}

float rocketMask(vec2 uv, float bodyHalfWidth)
{
    float cx = abs(uv.x - 0.5);

    if (uv.y > 0.66) {
        float t = (uv.y - 0.66) / 0.34;
        float noseWidth = bodyHalfWidth * (1.0 - t);
        return 1.0 - smoothstep(noseWidth - 0.008, noseWidth + 0.008, cx);
    }

    if (uv.y > 0.24) {
        return 1.0 - smoothstep(bodyHalfWidth - 0.008, bodyHalfWidth + 0.008, cx);
    }

    if (uv.y > 0.14) {
        float finOuter = 0.24;
        float finInner = 0.09;
        float body = 1.0 - smoothstep(bodyHalfWidth * 0.75 - 0.008, bodyHalfWidth * 0.75 + 0.008, cx);
        float fin = (1.0 - smoothstep(finOuter - 0.008, finOuter + 0.008, cx)) *
                    smoothstep(finInner - 0.008, finInner + 0.008, cx);
        return max(body, fin);
    }

    return 0.0;
}

vec4 renderRocket(vec2 uv)
{
    const float bodyHalfWidth = 0.13;
    float mask = rocketMask(uv, bodyHalfWidth);

    if (mask <= 0.0) {
        float exhaustHeight = mix(0.14, 0.22, turboGlow);
        if (uv.y < exhaustHeight) {
            float cx = abs(uv.x - 0.5);
            float t = 1.0 - uv.y / exhaustHeight;
            float plumeWidth = bodyHalfWidth * 0.55 + 0.1 * t;
            float plume = (1.0 - smoothstep(plumeWidth - 0.01, plumeWidth + 0.01, cx)) * t;
            if (plume > 0.0) {
                vec3 core = mix(vec3(1.0, 0.95, 0.75), vec3(1.0, 0.55, 0.08), turboGlow);
                vec3 edge = mix(vec3(1.0, 0.75, 0.25), vec3(1.0, 0.25, 0.02), turboGlow);
                vec3 exhaustColor = mix(edge, core, (1.0 - cx / plumeWidth) * 0.7);
                return vec4(exhaustColor, plume * mix(0.75, 0.95, turboGlow));
            }
        }
        return vec4(0.0);
    }

    float cx = abs(uv.x - 0.5);
    vec3 color;

    if (uv.y > 0.66) {
        float t = (uv.y - 0.66) / 0.34;
        vec3 tip = vec3(0.82, 0.18, 0.12);
        vec3 shoulder = vec3(0.72, 0.75, 0.8);
        color = mix(shoulder, tip, t);
    } else if (uv.y > 0.24) {
        float highlight = 1.0 - cx / bodyHalfWidth;
        color = vec3(0.5, 0.53, 0.58) + vec3(0.18, 0.2, 0.22) * highlight;
        float stripe = smoothstep(0.025, 0.0, abs(cx - 0.045));
        color = mix(color, vec3(0.88, 0.2, 0.14), stripe * 0.55);
        float panelLine = smoothstep(0.004, 0.0, abs(fract(uv.y * 7.0) - 0.5) - 0.46);
        color *= 1.0 - panelLine * 0.08;
    } else {
        color = vec3(0.38, 0.4, 0.44);
    }

    return vec4(color, mask);
}

void main()
{
    if (shapeType == 2) {
        FragColor = vec4(renderBackground(vTexCoord), 1.0);
        return;
    }

    if (shapeType == 1) {
        FragColor = renderRocket(vTexCoord);
        return;
    }

    FragColor = vec4(spriteColor, 1.0);
}
