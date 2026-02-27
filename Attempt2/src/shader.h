#pragma once

// actual shaders //
std::string vertexShader =
"#version 330 core\n"
"\n"
"layout(location = 0) in vec3 position;"
"\n"
"uniform mat4 u_model;\n"
"uniform mat4 u_projection;\n"
"uniform mat4 u_view;\n"
"\n"
"void main()\n"
"{\n"
"    gl_Position = u_projection * u_view * u_model * vec4(position, 1.0);\n"
"}\n";

std::string vertexShader2D =
"#version 330 core\n"
"\n"
"layout(location = 0) in vec3 position;"
"\n"
"uniform mat4 u_model;\n"
"uniform mat4 u_projection;\n"
"\n"
"void main()\n"
"{\n"
"    gl_Position = u_projection * u_model * vec4(position, 1.0);\n"
"}\n";

std::string fragmentShader =
"#version 330 core\n"
"\n"
"layout(location = 0) out vec4 color;\n"
"uniform vec3 u_color;\n"
"\n"
"void main()\n"
"{\n"
"    color = vec4(u_color, 1.0);\n"
"}\n";



static unsigned int CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_COMPILE_STATUS, &length);

    }

    return id;
}

static int CreateShader()
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

static int CreateShader2D()
{
    unsigned int program = glCreateProgram();
    unsigned int vs2d = CompileShader(GL_VERTEX_SHADER, vertexShader2D);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs2d);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs2d);
    glDeleteShader(fs);

    return program;
}