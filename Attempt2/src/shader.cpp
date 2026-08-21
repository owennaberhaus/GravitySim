#include "shader.h"
#include <iostream>
#include <vector>


// actual shaders 
std::string vertexShader =
"#version 330 core\n"
"\n"
"layout(location = 0) in vec3 position;\n"
"layout(location = 1) in vec3 normal;\n"
"\n"
"uniform mat4 u_model;\n"
"uniform mat4 u_projection;\n"
"uniform mat4 u_view;\n"
"\n"
"out vec3 v_normal;\n"
"\n"
"void main()\n"
"{\n"
"    // View-space normal. Lighting in view space means the light rides with\n"
"    // the camera, so spheres read as spheres from any angle.\n"
"    v_normal = mat3(u_view * u_model) * normal;\n"
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

// Flat fill - used by the 2D menu shader.
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

// Lit fill Without a shading term a solid color sphere was indistinguishable from a flat disc this is the source of light for the 3d objects HUGE UPDATE
std::string fragmentShader3D =
"#version 330 core\n"
"\n"
"layout(location = 0) out vec4 color;\n"
"in vec3 v_normal;\n"
"uniform vec3 u_color;\n"
"\n"
"void main()\n"
"{\n"
"    float lit = 1.0;\n"
"    // Path lines share this program but their VAO never enables attribute 1,\n"
"    // so their normal arrives as (0,0,0). Detect that and draw them unshaded.\n"
"    if (dot(v_normal, v_normal) > 1e-8)\n"
"    {\n"
"        vec3 n = normalize(v_normal);\n"
"        vec3 l = normalize(vec3(0.35, 0.45, 1.0)); // slightly off-axis headlight\n"
"        lit = 0.25 + 0.75 * max(dot(n, l), 0.0);   // ambient + diffuse\n"
"    }\n"
"    color = vec4(u_color * lit, 1.0);\n"
"}\n";

Shader::Shader() : m_shader(CreateShader()), m_shader2D(CreateShader2D()),
                m_modelLoc(glGetUniformLocation(m_shader, "u_model")), m_colorLoc(glGetUniformLocation(m_shader, "u_color")), 
                m_projLoc(glGetUniformLocation(m_shader, "u_projection")), m_viewLoc(glGetUniformLocation(m_shader, "u_view")),
                m_modelLoc2D(glGetUniformLocation(m_shader2D, "u_model")), m_colorLoc2D(glGetUniformLocation(m_shader2D, "u_color")),
                m_projLoc2D(glGetUniformLocation(m_shader2D, "u_projection"))
{

}

Shader::~Shader()
{
    glDeleteProgram(m_shader);
    glDeleteProgram(m_shader2D);
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        // query GL_COMPILE_STATUS for the length and then throw the result away
        int length = 0;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> message(length > 0 ? length : 1);
        glGetShaderInfoLog(id, length, &length, message.data());

        std::cerr << "Failed to compile "
            << (type == GL_VERTEX_SHADER ? "vertex" : "fragment")
            << " shader:\n" << message.data() << '\n';

        glDeleteShader(id);
        return 0;
    }

    return id;
}

int Shader::CreateShader()
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader3D);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

int Shader::CreateShader2D()
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
