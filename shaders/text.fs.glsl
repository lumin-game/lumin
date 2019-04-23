#version 330
out vec2 texpos;
uniform sampler2D tex;
uniform vec4 color;
layout(location = 0) out  vec4 fragColor;

void main(void) {
  fragColor = vec4(1, 1, 1, texture(tex, texpos).a) * color;
}
