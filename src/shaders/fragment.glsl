#version 330 core

in vec2 vTexCoord;
out vec4 FragColor;

uniform vec3 spriteColor;
uniform int shapeType;
uniform float turboGlow;

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
    if (shapeType == 1) {
        FragColor = renderRocket(vTexCoord);
        return;
    }

    FragColor = vec4(spriteColor, 1.0);
}
