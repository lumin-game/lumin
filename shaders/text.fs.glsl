#version 330
out vec4 fColor;

in vec2 texcoord;
uniform sampler2D tex;
uniform vec4 color;
layout(location = 0) out  vec4 fragColor;

void main(void) {
  fColor = vec4(1, 1, 1, texture2D(tex, texcoord).r) * color;
}
