#version 450 core

in vec2 vUV;
out vec4 FragColor;

void main()
{
    // Placeholder: nice visible gradient (proves your shader + quad + UVs work)
    FragColor = vec4(vUV.x, vUV.y, 0.25, 1.0);
}
