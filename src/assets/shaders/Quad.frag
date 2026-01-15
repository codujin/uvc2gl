#version 450 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uTex;
uniform int uFlipY;

void main()
{
    vec2 uv = vUV;
    if (uFlipY == 1)
        uv.y = 1.0 - uv.y;

    FragColor = texture(uTex, uv);
}
