#version 330 core

in vec2 v_ScreenUV;
out vec4 FragColor;

// 相机参数（由 CPU 传入）
uniform vec3  u_CameraPos;      // 相机世界位置
uniform vec3  u_CameraForward;  // 相机前方向（归一化）
uniform vec3  u_CameraRight;    // 相机右方向（归一化）
uniform vec3  u_CameraUp;       // 相机上方向（归一化）
uniform float u_FOV;            // 垂直 FOV（度）
uniform float u_AspectRatio;    // 宽 / 高

// 网格参数
const float MINOR_SPACING = 1.0;   // 次级网格间距（1米）
const float MAJOR_SPACING = 10.0;  // 主级网格间距（10米）
const float FADE_DISTANCE = 80.0;  // 淡出距离
const float PLANE_Y       = 0.0;   // 地面平面 Y 坐标

// 计算单轴网格线强度（带抗锯齿）
float GridLine(float coord, float spacing)
{
    float p = coord / spacing;
    float fw = fwidth(p) * 0.5;
    float line = abs(fract(p - 0.5) - 0.5);
    return smoothstep(fw, 0.0, line);
}

void main()
{
    // 将 FOV 转为弧度
    float fovRad = radians(u_FOV);

    // 计算近平面半高（使用归一化的焦距 d=1）
    float halfH = tan(fovRad * 0.5);
    float halfW = halfH * u_AspectRatio;

    // 屏幕坐标 -> [-1, 1]
    vec2 ndc = v_ScreenUV * 2.0 - 1.0;

    // 构造从相机出发的射线方向（世界空间）
    vec3 rayDir = normalize(
        u_CameraForward
        + u_CameraRight * ndc.x * halfW
        + u_CameraUp    * ndc.y * halfH
    );

    // 射线与 y=PLANE_Y 平面求交
    // t = (planeY - originY) / dirY
    float dirY = rayDir.y;
    if (abs(dirY) < 1e-6)
    {
        // 射线几乎与地面平行，不绘制网格
        FragColor = vec4(0.0);
        return;
    }

    float t = (PLANE_Y - u_CameraPos.y) / dirY;
    if (t < 0.0)
    {
        // 交点在相机后方
        FragColor = vec4(0.0);
        return;
    }

    // 计算交点世界坐标
    vec3 hitPoint = u_CameraPos + rayDir * t;

    // 两级网格线
    float minorLine = max(
        GridLine(hitPoint.x, MINOR_SPACING),
        GridLine(hitPoint.z, MINOR_SPACING)
    );
    float majorLine = max(
        GridLine(hitPoint.x, MAJOR_SPACING),
        GridLine(hitPoint.z, MAJOR_SPACING)
    );

    // 距离淡出
    float dist = length(hitPoint - u_CameraPos);
    float fade = 1.0 - smoothstep(FADE_DISTANCE * 0.6, FADE_DISTANCE, dist);

    // 合成颜色：主线更亮，次级线更暗
    vec3 minorColor = vec3(0.35, 0.35, 0.35);
    vec3 majorColor = vec3(0.6, 0.6, 0.6);

    vec3 gridColor = minorColor * minorLine + majorColor * majorLine;
    float alpha = (minorLine * 0.5 + majorLine) * fade;

    // 背景色（深色地面）
    vec3 bgColor = vec3(0.08, 0.08, 0.08);
    vec3 finalColor = mix(bgColor, gridColor, clamp(alpha, 0.0, 1.0));

    FragColor = vec4(finalColor, 1.0);
}
