#include <windows.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <FreeImage.h>
#include <iostream>
#include <vector>
#include <map>

#ifndef _DEBUG
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

using namespace std;

void error_callback(int error, const char* description);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
string read_file(const char *filePath);
GLuint load_texture(string filenameString, float &width, float &height, GLenum minificationFilter, GLenum magnificationFilter);
void link_program();
bool replace(string& str, const string& from, const string& to);
void set_effect();
int main(void);

struct texture
{
  int width, height;
  GLuint id;
};


const vector<vector<string>> effects = { { "cypher" }, { "digitalbrain", "tex16.png" }, { "flame" }, { "galaxy" }, { "interstellar", "tex16.png" }, { "noise_anim_flow", "tex16.png" }, /*{"sodki_canady", "tex16.png"},*/{ "starnest" }, { "topologica", "tex16.png" }, /*{"volcanic", "tex16.png", "tex06.jpg", "tex09.jpg"},*//*{ "voxel_edges", "tex07.jpg", "tex06.jpg" },*/ { "warping", "tex16.png" }, { "waves", "tex16.png" } };
int num_effects;
int effect;
float* channelRes;
map<string, texture> textures;


void error_callback(int error, const char* description)
{
  fputs(description, stderr);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);

  if (key == GLFW_KEY_SPACE && action == GLFW_PRESS){
    effect = ++effect % num_effects;
    set_effect();
    fprintf(stdout, "Using effect: %s\n", effects[effect][0].c_str());
  }
}

string read_file(const char *filePath) {
  ifstream myReadFile;
  myReadFile.open(filePath);
  string output;
  if (myReadFile.is_open()) {
    while (!myReadFile.eof()) // To get you all the lines.
    {
      string line;
      getline(myReadFile, line); // Saves the line in STRING.
      output.append(line);
      output.append("\n");
    }
  }
  myReadFile.close();
  return output;
}

GLuint load_texture(string filenameString, float &width, float &height, GLenum minificationFilter = GL_LINEAR, GLenum magnificationFilter = GL_LINEAR)
{
  filenameString = "Shader/" + filenameString;

  if (textures.count(filenameString)) {
    texture t = textures[filenameString];
    width = t.width;
    height = t.height;
    return t.id;
  }

  const char* filename = filenameString.c_str();
  FREE_IMAGE_FORMAT format = FreeImage_GetFileType(filename, 0);

  if (format == -1)
  {
    cout << "Could not find image: " << filenameString << " - Aborting." << endl;
    exit(-1);
  }

  if (format == FIF_UNKNOWN)
  {
    cout << "Couldn't determine file format - attempting to get from file extension..." << endl;

    format = FreeImage_GetFIFFromFilename(filename);

    if (!FreeImage_FIFSupportsReading(format))
    {
      cout << "Detected image format cannot be read!" << endl;
      exit(-1);
    }
  }

  FIBITMAP* bitmap = FreeImage_Load(format, filename);

  int bitsPerPixel = FreeImage_GetBPP(bitmap);

  FIBITMAP* bitmap32;
  if (bitsPerPixel == 32) bitmap32 = bitmap;
  else  bitmap32 = FreeImage_ConvertTo32Bits(bitmap);

  width = FreeImage_GetWidth(bitmap32);
  height = FreeImage_GetHeight(bitmap32);
  GLubyte* textureData = FreeImage_GetBits(bitmap32);

  GLuint tempTextureID;
  glGenTextures(1, &tempTextureID);
  glBindTexture(GL_TEXTURE_2D, tempTextureID);

  glTexImage2D(GL_TEXTURE_2D,    // Type of texture
    0,                // Mipmap level (0 being the top level i.e. full size)
    GL_RGBA,          // Internal format
    width,       // Width of the texture
    height,      // Height of the texture,
    0,                // Border in pixels
    GL_BGRA,          // Data format
    GL_UNSIGNED_BYTE, // Type of texture data
    textureData);     // The image data to use for this texture

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minificationFilter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magnificationFilter);

  if (minificationFilter == GL_LINEAR_MIPMAP_LINEAR ||
    minificationFilter == GL_LINEAR_MIPMAP_NEAREST ||
    minificationFilter == GL_NEAREST_MIPMAP_LINEAR ||
    minificationFilter == GL_NEAREST_MIPMAP_NEAREST)
  {
    glGenerateMipmap(GL_TEXTURE_2D);
  }

  GLenum glError = glGetError();
  if (glError) cout << "There was an error loading the texture: " << filenameString << endl;

  FreeImage_Unload(bitmap32);

  if (bitsPerPixel != 32)
  {
    FreeImage_Unload(bitmap);
  }

  texture t = { width, height, tempTextureID };
  textures[filenameString] = t;

  return tempTextureID;
}

GLuint link_program(GLuint vs, GLuint effect)
{
  GLuint program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, effect);
  glLinkProgram(program);

  GLint isLinked = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
  if (isLinked == GL_FALSE)
  {
    GLint maxLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
    GLchar* strInfoLog = new GLchar[maxLength + 1];
    glGetProgramInfoLog(program, maxLength, &maxLength, strInfoLog);
    fputs(strInfoLog, stderr);
  }

  return program;
}

void set_effect()
{
  if (effects[effect].size() > 1)
  {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, load_texture(effects[effect][1], channelRes[0], channelRes[1]));

    if (effects[effect].size() > 2)
    {
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, load_texture(effects[effect][2], channelRes[3], channelRes[4]));

      if (effects[effect].size() == 4)
      {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, load_texture(effects[effect][3], channelRes[6], channelRes[7]));
      }
    }
  }
}

bool replace(string& str, const string& from, const string& to) {
  size_t start_pos = str.find(from);
  if (start_pos == string::npos)
    return false;
  str.replace(start_pos, from.length(), to);
  return true;
}

int main(void)
{
  GLFWwindow* window;

  glfwSetErrorCallback(error_callback);

  if (!glfwInit())
    exit(EXIT_FAILURE);

  int screenWidth = 0, screenHeight = 0;
  int count;
  int xpos, ypos;
  int minX = 0, minY = 0;
  GLFWmonitor** monitors = glfwGetMonitors(&count);
  for (int i = 0; i < count; i++)
  {
    GLFWmonitor* monitor = monitors[i];
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    glfwGetMonitorPos(monitor, &xpos, &ypos);
    fprintf(stdout, "monitor #%d: width: %d, height: %d, x: %d, y: %d\n", i, mode->width, mode->height, xpos, ypos);
    screenWidth += mode->width;
    if (xpos == 0 && ypos == 0) screenHeight += mode->height;
    else screenHeight += abs(ypos);

    if (xpos < minX) minX = xpos;
    if (ypos < minY) minY = ypos;
  }

  fprintf(stdout, "width: %d, height: %d\n", screenWidth, screenHeight);

#ifdef _DEBUG
  screenWidth = 800;
  screenHeight = 600;
  minX = 200;
  minY = 200;
#endif

  glfwWindowHint(GLFW_DECORATED, GL_FALSE);
  window = glfwCreateWindow(screenWidth, screenHeight, "ShaderWall", nullptr, nullptr);
  if (!window)
  {
    fputs("Could not create Window", stdout);
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwSetWindowPos(window, minX, 0);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

  glfwMakeContextCurrent(window);
  glfwSwapInterval(0);
  glfwSetKeyCallback(window, key_callback);

  // start GLEW extension handler
  glewExperimental = GL_TRUE;
  glewInit();

  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  const char* vertex_shader = "in vec3 position;\n"
    "out vec2 coords;\n"
    "void main()\n"
    "{\n"
    "gl_Position=vec4(position,1.),coords=gl_MultiTexCoord0.rg;\n"
    "}\n";
  glShaderSource(vs, 1, &vertex_shader, nullptr);
  glCompileShader(vs);

  num_effects = effects.size();

  GLuint* programs = new GLuint[num_effects];

  for (int i = 0; i < num_effects; i++)
  {
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    string filename = "Shader/" + effects[i][0] + ".glsl";
    fputs(("Loading effect: " + filename + "\n").c_str(), stdout);

    string shader = read_file(filename.c_str());
    replace(shader, "void mainImage( out vec4 fragColor, in vec2 fragCoord ) {", "vec4 " + effects[i][0] + "( vec2 fragCoord ) {\nvec4 fragColor;");
    shader = shader.substr(0, shader.length() - 3) + "return fragColor;\n}\n";

    string fshader =
      "out vec4 frag_color;\n"
      "in vec2 coords;\n"
      "uniform vec3 iResolution;\n"
      "uniform float iGlobalTime;\n"
      "uniform vec4 iMouse;\n"
      "uniform sampler2D iChannel0;\n"
      "uniform sampler2D iChannel1;\n"
      "uniform sampler2D iChannel2;\n"
      "uniform sampler2D iChannel3;\n"
      "uniform vec3 iChannelResolution[4];\n"
      "%shader%\n"
      "void main()\n"
      "{\n"
      "frag_color=%main_image%(coords*iResolution.rg);\n"
      "}\n";

    replace(fshader, "%shader%", shader);
    replace(fshader, "%main_image%", effects[i][0]);

    const char* fragment_shader = fshader.c_str();
    glShaderSource(fs, 1, &fragment_shader, nullptr);
    glCompileShader(fs);

    programs[i] = link_program(vs, fs);
  }

  channelRes = new float[4 * 3];
  ZeroMemory(channelRes, 12 * 4);

  effect = 6;
  set_effect();

  int frame = 0;
  int width = 0, height = 0;

  while (!glfwWindowShouldClose(window))
  {
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    double xpos = 0, ypos = 0;
    //glfwGetCursorPos(window, &xpos, &ypos);
    ypos = height - ypos;

    glClear(GL_COLOR_BUFFER_BIT);

    int program = programs[effect];

    glUseProgram(program);
    GLint iResolution = glGetUniformLocation(program, "iResolution");
    GLint iGlobalTime = glGetUniformLocation(program, "iGlobalTime");
    GLint iMouse = glGetUniformLocation(program, "iMouse");
    GLint iChannel0 = glGetUniformLocation(program, "iChannel0");
    GLint iChannel1 = glGetUniformLocation(program, "iChannel1");
    GLint iChannel2 = glGetUniformLocation(program, "iChannel2");
    GLint iChannel3 = glGetUniformLocation(program, "iChannel3");
    GLint iChannelResolution = glGetUniformLocation(program, "iChannelResolution");

    glUniform3f(iResolution, width, height, 0);
    glUniform1f(iGlobalTime, glfwGetTime());
    glUniform4f(iMouse, xpos, ypos, 0, 0);
    glUniform1i(iChannel0, 0);
    glUniform1i(iChannel1, 1);
    glUniform1i(iChannel2, 2);
    glUniform1i(iChannel3, 3);
    glUniform3fv(iChannelResolution, 4, channelRes);

    glEnable(GL_TEXTURE_2D);

    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0.f, 0.f);
    glVertex3f(-1.f, -1.f, 0.f);

    glTexCoord2f(1.f, 0.f);
    glVertex3f(1.f, -1.f, 0.f);

    glTexCoord2f(0.f, 1.f);
    glVertex3f(-1.f, 1.f, 0.f);

    glTexCoord2f(1.f, 1.f);
    glVertex3f(1.f, 1.f, 0.f);
    glEnd();

    glDisable(GL_TEXTURE_2D);

    glUseProgram(0);

    /*glReadBuffer(GL_FRONT);
    GLubyte* pixels = new GLubyte[3 * width * height];

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, pixels);

    FIBITMAP* image = FreeImage_ConvertFromRawBits(pixels, width, height, 3 * width, 24, 0xff0000, 0x00ff00, 0x0000ff, false);

    string s("E:/cache/");
    s.append(to_string(frame++));
    s.append(".bmp");
    FreeImage_Save(FIF_BMP, image, s.c_str(), 0);

    frame = frame % 16;

    FreeImage_Unload(image);
    delete[] pixels;*/

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
