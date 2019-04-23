#version 330
out vec4 fColor;

varying vec2 texcoord;
uniform sampler2D tex;
uniform vec4 color;

void main(void) {
  fColor = vec4(1, 1, 1, texture2D(tex, texcoord).r) * color;
}
