#ifndef SHADER_SOURCES_H
#define SHADER_SOURCES_H

namespace ShaderSources {
    // Vertex shader for fullscreen quad
    const char* vs_shader = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
out vec2 uv;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    uv = aUV;
}
)";

    // Fragment shader for displaying textures
    const char* display_fs = R"(
#version 330 core
out vec4 FragColor;
in vec2 uv;
uniform sampler2D tex;
void main() {
    vec3 color = texture(tex, uv).rgb;
    FragColor = vec4(color, 1.0);
}
)";

    // Advection shader
    const char* advect_fs = R"(
#version 330 core
out vec4 FragColor;
in vec2 uv;
uniform sampler2D field;
uniform sampler2D velocity;
uniform float dt;
uniform vec2 texelSize;

void main() {
    vec2 vel = texture(velocity, uv).xy;
    vec2 prevUV = uv - dt * vel * texelSize;
    vec4 result = texture(field, prevUV);
    FragColor = result;
}
)";

    // Divergence computation shader
    const char* divergence_fs = R"(
#version 330 core
out vec4 FragColor;
in vec2 uv;
uniform sampler2D velocity;
uniform vec2 texelSize;

void main() {
    vec2 left = texture(velocity, uv - vec2(texelSize.x, 0.0)).xy;
    vec2 right = texture(velocity, uv + vec2(texelSize.x, 0.0)).xy;
    vec2 bottom = texture(velocity, uv - vec2(0.0, texelSize.y)).xy;
    vec2 top = texture(velocity, uv + vec2(0.0, texelSize.y)).xy;
    
    float div = 0.5 * ((right.x - left.x) + (top.y - bottom.y));
    FragColor = vec4(div, 0.0, 0.0, 1.0);
}
)";

    // Pressure solve (Jacobi iteration)
    const char* pressure_fs = R"(
#version 330 core
out vec4 FragColor;
in vec2 uv;
uniform sampler2D pressure;
uniform sampler2D divergence;
uniform vec2 texelSize;
uniform float alpha;
uniform float beta;

void main() {
    float left = texture(pressure, uv - vec2(texelSize.x, 0.0)).r;
    float right = texture(pressure, uv + vec2(texelSize.x, 0.0)).r;
    float bottom = texture(pressure, uv - vec2(0.0, texelSize.y)).r;
    float top = texture(pressure, uv + vec2(0.0, texelSize.y)).r;
    float div = texture(divergence, uv).r;
    
    float result = (left + right + bottom + top + alpha * div) * beta;
    FragColor = vec4(result, 0.0, 0.0, 1.0);
}
)";

    // Vorticity computation shader
    const char* vorticity_fs = R"(
#version 330 core
out vec4 FragColor;
in vec2 uv;
uniform sampler2D velocity;
uniform vec2 texelSize;

void main() {
    vec2 left = texture(velocity, uv - vec2(texelSize.x, 0.0)).xy;
    vec2 right = texture(velocity, uv + vec2(texelSize.x, 0.0)).xy;
    vec2 bottom = texture(velocity, uv - vec2(0.0, texelSize.y)).xy;
    vec2 top = texture(velocity, uv + vec2(0.0, texelSize.y)).xy;
    
    float vorticity = 0.5 * ((right.y - left.y) - (top.x - bottom.x));
    FragColor = vec4(vorticity, 0.0, 0.0, 1.0);
}
)";

    // Subtract pressure gradient
    const char* gradient_fs = R"(
#version 330 core
out vec4 FragColor;
in vec2 uv;
uniform sampler2D velocity;
uniform sampler2D pressure;
uniform vec2 texelSize;

void main() {
    float left = texture(pressure, uv - vec2(texelSize.x, 0.0)).r;
    float right = texture(pressure, uv + vec2(texelSize.x, 0.0)).r;
    float bottom = texture(pressure, uv - vec2(0.0, texelSize.y)).r;
    float top = texture(pressure, uv + vec2(0.0, texelSize.y)).r;
    
    vec2 vel = texture(velocity, uv).xy;
    vel.x -= 0.5 * (right - left);
    vel.y -= 0.5 * (top - bottom);
    
    FragColor = vec4(vel, 0.0, 1.0);
}
)";

    // Vorticity confinement shader
    const char* confinement_fs = R"(
#version 330 core
out vec4 FragColor;
in vec2 uv;
uniform sampler2D velocity;
uniform sampler2D vorticity;
uniform vec2 texelSize;
uniform float dt;
uniform float strength;

void main() {
    vec2 vel = texture(velocity, uv).xy;
    
    float left = texture(vorticity, uv - vec2(texelSize.x, 0.0)).r;
    float right = texture(vorticity, uv + vec2(texelSize.x, 0.0)).r;
    float bottom = texture(vorticity, uv - vec2(0.0, texelSize.y)).r;
    float top = texture(vorticity, uv + vec2(0.0, texelSize.y)).r;
    float center = texture(vorticity, uv).r;
    
    vec2 gradient = vec2(abs(right) - abs(left), abs(top) - abs(bottom)) * 0.5;
    float len = length(gradient) + 1e-5;
    gradient = gradient / len;
    
    vec2 force = vec2(gradient.y, -gradient.x) * center * strength;
    vel += force * dt;
    
    FragColor = vec4(vel, 0.0, 1.0);
}
)";

    // Splat shader for adding forces/dye
    const char* splat_fs = R"(
#version 330 core
out vec4 FragColor;
in vec2 uv;
uniform sampler2D base;
uniform vec2 point;
uniform vec3 color;
uniform float radius;
uniform float strength;

void main() {
    vec4 baseColor = texture(base, uv);
    float dist = distance(gl_FragCoord.xy, point);
    float splat = exp(-dist * dist / radius) * strength;
    FragColor = baseColor + vec4(color * splat, 0.0);
}
)";
}

#endif