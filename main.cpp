#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>

int main()
{
    if (!glfwInit())
    {
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//设置OpenGL上下文版本为3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//设置OpenGL上下文次要版本为3
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//设置OpenGL上下文为核心模式

    //创建窗口，窗口大小为1280*720，标题为"Creator Engine"
    GLFWwindow* window = glfwCreateWindow(1280,720,"Creator Engine",nullptr,nullptr);
    
    //如果窗口创建失败，输出错误信息并终止程序
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    //设置窗口位置
    glfwSetWindowPos(window,150,150);
    glfwMakeContextCurrent(window);//设置当前上下文为新创建的窗口

    //初始化GLEW,如果初始化失败，输出错误信息并终止程序
    if(glewInit()!= GLEW_OK)
    {
        glfwTerminate();
        return -1;
    }

    //定义顶点着色器源代码
    std:: string vertexShaderSource =R"(
        #version 330 core
        layout (location = 0) in vec3 position;
        void main()
        {
            gl_Position = vec4(position.x,position.y,position.z, 1.0);
        }
    )";

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);//创建顶点着色器对象
    const char* vertexShaderCStr = vertexShaderSource.c_str();//将顶点着色器源代码转换为C风格字符串
    glShaderSource(vertexShader, 1, &vertexShaderCStr, nullptr);//将顶点着色器源代码附加到顶点着色器对象上
    glCompileShader(vertexShader);//编译顶点着色器

    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);//检查顶点着色器是否编译成功
    //如果编译失败，输出错误信息
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);//获取编译错误信息
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED:" << infoLog << std::endl;
    }

    //定义片段着色器源代码
    std:: string fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        void main()
        {
            FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
        }
    )";

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);//创建片段着色器对象
    const char* fragmentShaderSourceCStr = fragmentShaderSource.c_str();//将片段着色器源代码转换为C风格字符串
    glShaderSource(fragmentShader, 1, &fragmentShaderSourceCStr, nullptr);//将片段着色器源代码附加到片段着色器对象上
    glCompileShader(fragmentShader);//编译片段着色器

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);//检查片段着色器是否编译成功
    //如果编译失败，输出错误信息
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);//获取编译错误信息
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED:" << infoLog << std::endl;
    }

    GLuint shaderProgram = glCreateProgram();//创建着色器程序对象
    glAttachShader(shaderProgram, vertexShader);//将顶点着色器附加到着色器程序对象上
    glAttachShader(shaderProgram, fragmentShader);//将片段着色器附加到着色器程序对象上
    glLinkProgram(shaderProgram);//链接着色器程序对象

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);//检查着色器程序是否链接成功
    //如果链接失败，输出错误信息
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);//获取链接错误信息
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED:" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);//删除顶点着色器对象
    glDeleteShader(fragmentShader);//删除片段着色器对象

    //创建顶点数据，定义一个三角形的三个顶点
    std:: vector<float> vertices = {
        0.0f, 0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f
    };

    //创建顶点缓冲对象和顶点数组对象
    GLuint VBO;
    glGenBuffers(1, &VBO);//生成一个顶点缓冲对象
    glBindBuffer(GL_ARRAY_BUFFER, VBO);//绑定顶点缓冲对象
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);//将顶点数据传输到顶点缓冲对象中
    glBindBuffer(GL_ARRAY_BUFFER, 0);//解绑顶点缓冲对象

    //创建顶点数组对象
    GLuint VAO;
    glGenVertexArrays(1, &VAO);//生成一个顶点数组对象
    glBindVertexArray(VAO);//绑定顶点数组对象
    glBindBuffer(GL_ARRAY_BUFFER, VBO);//绑定顶点缓冲对象

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);//设置顶点属性指针
    glEnableVertexAttribArray(0);//启用顶点属性数组

    glBindBuffer(GL_ARRAY_BUFFER, 0);//解绑顶点缓冲对象
    glBindVertexArray(0);//解绑顶点数组对象

    //如果窗口创建成功，设置当前上下文为新创建的窗口
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.0f,0.0f,0.0f,1.0f);//设置清屏颜色为黑色
        glClear(GL_COLOR_BUFFER_BIT);//清除颜色缓冲区

        glUseProgram(shaderProgram);//使用着色器程序对象
        glBindVertexArray(VAO);//绑定顶点数组对象
        glDrawArrays(GL_TRIANGLES, 0, 3);//绘制三角形
        glBindVertexArray(0);//解绑顶点数组对象

        glfwSwapBuffers(window);//交换前后缓冲区
        glfwPollEvents();//处理窗口事件
    }

    glfwTerminate();//终止GLFW，释放资源
    return 0;
}
