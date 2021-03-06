#version 430 core

out vec4 FragColor;

vec4 pack (float depth) {
    const vec4 bitShift = vec4(1.0, 256.0, 256.0 * 256.0, 256.0 * 256.0 * 256.0);
    const vec4 bitMask = vec4(1.0/256.0, 1.0/256.0, 1.0/256.0, 0.0);

    vec4 rgbaDepth = fract(depth * bitShift);
    rgbaDepth -= rgbaDepth.gbaa * bitMask;
    return rgbaDepth;
}

void main()
{
    // gl_FragColor = vec4(gl_FragCoord.z);
    FragColor = pack(gl_FragCoord.z);
}