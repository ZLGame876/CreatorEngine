#version 330 core

// 全屏四边形顶点：直接输出 NDC 坐标
layout(location = 0) in vec2 a_Position;

// 传递屏幕 UV：[-1, 1] -> [0, 1]（y 翻转，因为 OpenGL 屏幕坐标原点在左下）
out vec2 v_ScreenUV;

void main()
{
    v_ScreenUV = a_Position * 0.5 + 0.5;
    v_ScreenUV.y = 1.0 - v_ScreenUV.y;
    gl_Position = vec4(a_Position, 0.0, 1.0);
}
