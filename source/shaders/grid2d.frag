#version 330 core

in vec3 v_WorldPos;
out vec4 FragColor;

const float MINOR_SPACING = 50.0;
const float MAJOR_SPACING = 200.0;
const float LINE_WIDTH = 0.02;

// 计算单轴网格线强度
float GridLine(float coord, float spacing)
{
    float p = coord / spacing;
    float fw = fwidth(p) * 0.5;
    float line = abs(fract(p - 0.5) - 0.5);
    return 1.0 - smoothstep(0.0, fw + LINE_WIDTH, line);
}

void main()
{
    float minorLine = max(
        GridLine(v_WorldPos.x, MINOR_SPACING),
        GridLine(v_WorldPos.y, MINOR_SPACING)
    );
    float majorLine = max(
        GridLine(v_WorldPos.x, MAJOR_SPACING),
        GridLine(v_WorldPos.y, MAJOR_SPACING)
    );

    vec3 minorColor = vec3(0.16, 0.16, 0.16);
    vec3 majorColor = vec3(0.30, 0.30, 0.30);
    vec3 bgColor = vec3(0.12, 0.12, 0.12);

    vec3 gridColor = minorColor * minorLine + majorColor * majorLine;
    float alpha = clamp(minorLine * 0.15 + majorLine * 0.8, 0.0, 1.0);

    vec3 finalColor = mix(bgColor, gridColor, alpha);
    FragColor = vec4(finalColor, 1.0);
}