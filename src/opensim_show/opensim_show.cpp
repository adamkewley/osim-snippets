﻿#include <SDL.h>
#undef main
#include "opensim_wrapper.hpp"
#include "OsimsnippetsConfig.h"

#include <SDL_ttf.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <imgui.h>

// imgui
#include "imgui.h"
#include "examples/imgui_impl_sdl.h"
#include "examples/imgui_impl_opengl3.h"

#include <string>
#include <exception>
#include <optional>
#include <vector>
#include <sstream>
#include <array>
#include <chrono>
#include <algorithm>
#include <iostream>
#include <filesystem>
#include <atomic>
#include <thread>
#include <fstream>

using std::string_literals::operator""s;
using std::chrono_literals::operator""ms;


// helper type for the visitor #4
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

static char rajagopal_model_path[] = "resources/opensim-models/Models/RajagopalModel/Rajagopal2015.osim";

namespace sdl {
    class Surface final {
        SDL_Surface* handle;
    public:
        Surface(SDL_Surface* _handle) :
            handle{_handle} {

            if (handle == nullptr) {
                throw std::runtime_error{"sdl::Surface: null handle passed into constructor"};
            }
        }
        Surface(Surface const&) = delete;
        Surface(Surface&&) = delete;
        Surface& operator=(Surface const&) = delete;
        Surface& operator=(Surface&&) = delete;
        ~Surface() noexcept {
            SDL_FreeSurface(handle);
        }

        operator SDL_Surface*() noexcept {
            return handle;
        }
        SDL_Surface* operator->() const noexcept {
            return handle;
        }
    };

    template<class ...Args>
    Surface CreateRGBSurface(Args... args) {
        SDL_Surface* handle = SDL_CreateRGBSurface(std::forward<Args>(args)...);

        if (handle == nullptr) {
            throw std::runtime_error{"SDL_CreateRGBSurface: "s + SDL_GetError()};
        }

        return Surface{handle};
    }

    class Surface_lock final {
        SDL_Surface* ptr;
    public:
        Surface_lock(SDL_Surface* s) : ptr{s} {
            if (SDL_LockSurface(ptr) != 0) {
                throw std::runtime_error{"SDL_LockSurface failed: "s + SDL_GetError()};
            }
        }
        Surface_lock(Surface_lock const&) = delete;
        Surface_lock(Surface_lock&&) = delete;
        Surface_lock& operator=(Surface_lock const&) = delete;
        Surface_lock& operator=(Surface_lock&&) = delete;
        ~Surface_lock() noexcept {
            SDL_UnlockSurface(ptr);
        }
    };

    class Texture final {
        SDL_Texture* handle;
    public:
        Texture(SDL_Texture* _handle) : handle{_handle} {
            if (handle == nullptr) {
                throw std::runtime_error{"sdl::Texture: null handle passed into constructor"};
            }
        }
        Texture(Texture const&) = delete;
        Texture(Texture&&) = delete;
        Texture& operator=(Texture const&) = delete;
        Texture& operator=(Texture&&) = delete;
        ~Texture() noexcept {
            SDL_DestroyTexture(handle);
        }

        operator SDL_Texture*() {
            return handle;
        }
    };

    Texture CreateTextureFromSurface(SDL_Renderer* r, SDL_Surface* s) {
        SDL_Texture* t = SDL_CreateTextureFromSurface(r, s);
        if (t == nullptr) {
            throw std::runtime_error{"SDL_CreateTextureFromSurface failed: "s + SDL_GetError()};
        }
        return Texture{t};
    }

    class Window final {
        SDL_Window* ptr;
    public:
        template<class ...Args>
        Window(Args... args) : ptr{SDL_CreateWindow(std::forward<Args>(args)...)} {
            if (ptr == nullptr) {
                throw std::runtime_error{"SDL_CreateWindow: "s + SDL_GetError()};
            }
        }
        Window(Window const&) = delete;
        Window(Window&&) = delete;
        Window& operator=(Window const&) = delete;
        Window& operator=(Window&&) = delete;
        ~Window() noexcept {
            SDL_DestroyWindow(ptr);
        }

        operator SDL_Window*() noexcept {
            return ptr;
        }
    };

    std::pair<int, int> window_size(SDL_Window* window) {
        int w;
        int h;
        SDL_GetWindowSize(window, &w, &h);
        return {w, h};
    }

    class Renderer final {
        SDL_Renderer* ptr;
    public:
        template<class ...Args>
        Renderer(SDL_Window* w, Args... args) :
            ptr{SDL_CreateRenderer(w, std::forward<Args>(args)...)} {
            if (ptr == nullptr) {
                throw std::runtime_error{"SDL_CreateRenderer failed: "s + SDL_GetError()};
            }
        }
        Renderer(Renderer const&) = delete;
        Renderer(Renderer&&) = delete;
        Renderer& operator=(Renderer const&) = delete;
        Renderer& operator=(Renderer&&) = delete;
        ~Renderer() noexcept {
            SDL_DestroyRenderer(ptr);
        }

        operator SDL_Renderer* () noexcept {
            return ptr;
        }
    };

    class GLContext final {
        SDL_GLContext ctx;
    public:
        GLContext(SDL_Window* w) : ctx{SDL_GL_CreateContext(w)} {
            if (ctx == nullptr) {
                throw std::runtime_error{"SDL_GL_CreateContext failed: "s + SDL_GetError()};
            }
        }
        GLContext(GLContext const&) = delete;
        GLContext(GLContext&&) = delete;
        GLContext& operator=(GLContext const&) = delete;
        GLContext& operator=(GLContext&&) = delete;
        ~GLContext() noexcept {
            SDL_GL_DeleteContext(ctx);
        }

        operator SDL_GLContext () noexcept {
            return ctx;
        }
    };

    struct Context final {
        Context() {
            if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)  != 0) {
                throw std::runtime_error{"SDL_Init: failed: "s + SDL_GetError()};
            }
        }
        Context(Context const&) = delete;
        Context(Context&&) = delete;
        Context& operator=(Context const&) = delete;
        Context& operator=(Context&&) = delete;
        ~Context() noexcept{
            SDL_Quit();
        }
    };
}

namespace sdl::ttf {
    struct Context final {
        Context() {
            if (TTF_Init() == -1) {
                throw std::runtime_error{"TTF_Init: failed: "s + TTF_GetError()};
            }
        }
        Context(Context const&) = delete;
        Context(Context&&) = delete;
        Context& operator=(Context const&) = delete;
        Context& operator=(Context&&) = delete;
        ~Context() noexcept {
            TTF_Quit();
        }
    };

    class Font final {
        TTF_Font* handle;

    public:
        Font(char const* path, int ptsize) :
            handle{TTF_OpenFont(path, ptsize)} {

            if (handle == nullptr) {
                throw std::runtime_error{"TTF_OpenFont failed for path '"s + path + "' error = "s + TTF_GetError()};
            }
        }
        Font(Font const&) = delete;
        Font(Font&&) = delete;
        Font& operator=(Font const&) = delete;
        Font& operator=(Font&&) = delete;
        ~Font() noexcept {
            TTF_CloseFont(handle);
        }

        operator TTF_Font* () noexcept {
            return handle;
        }
    };

    sdl::Surface RenderText_Blended_Wrapped(Font& font, char const* text, SDL_Color fg, Uint32 wrapLength) {
        SDL_Surface* s = TTF_RenderText_Blended_Wrapped(font, text, fg, wrapLength);

        if (s == nullptr) {
            throw std::runtime_error{"TTF_RenderText_Blended_Wrapped failed: "s + TTF_GetError()};
        }

        return sdl::Surface{s};
    }
}

namespace gl {
    std::string to_string(GLubyte const* err_string) {
        return std::string{reinterpret_cast<char const*>(err_string)};
    }

    void assert_no_errors(char const* func) {
        return ;
        std::vector<GLenum> errors;
        for (GLenum error = glGetError(); error != GL_NO_ERROR; error = glGetError()) {
            errors.push_back(error);
        }

        if (errors.empty()) {
            return;
        }

        std::stringstream msg;
        msg << func << " failed";
        if (errors.size() == 1) {
            msg << ": ";
        } else {
            msg << " with " << errors.size() << " errors: ";
        }
        for (auto it = errors.begin(); it != errors.end()-1; ++it) {
            msg << to_string(gluErrorString(*it)) << ", ";
        }
        msg << to_string(gluErrorString(errors.back()));

        throw std::runtime_error{msg.str()};
    }

    class Program final {
        GLuint handle = glCreateProgram();
    public:
        Program() {
            if (handle == 0) {
                throw std::runtime_error{"glCreateProgram() failed"};
            }
        }
        Program(Program const&) = delete;
        Program(Program&& tmp) :
            handle{tmp.handle} {
            tmp.handle = 0;
        }
        Program& operator=(Program const&) = delete;
        Program& operator=(Program&&) = delete;
        ~Program() noexcept {
            if (handle != 0) {
                glDeleteProgram(handle);
            }
        }

        operator GLuint () noexcept {
            return handle;
        }
    };

    void UseProgram(Program& p) {
        glUseProgram(p);
        assert_no_errors("glUseProgram");
    }

    void UseProgram() {
        glUseProgram(static_cast<GLuint>(0));
    }

    static std::optional<std::string> get_shader_compile_errors(GLuint shader) {
        GLint params = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &params);
        if (params == GL_FALSE) {
            GLint log_len = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);

            std::vector<GLchar> errmsg(log_len);
            glGetShaderInfoLog(shader, log_len, &log_len, errmsg.data());

            std::stringstream ss;
            ss << "glCompileShader() failed: ";
            ss << errmsg.data();
            return std::optional<std::string>{ss.str()};
        }
        return std::nullopt;
    }

    struct Shader {
        static Shader Compile(GLenum shaderType, char const* src) {
            Shader shader{shaderType};
            glShaderSource(shader, 1, &src, nullptr);
            glCompileShader(shader);

            if (auto compile_errors = get_shader_compile_errors(shader); compile_errors) {
                throw std::runtime_error{"error compiling vertex shader: "s + compile_errors.value()};
            }

            return shader;
        }
    private:
        GLuint handle;
    public:
        Shader(GLenum shaderType) :
            handle{glCreateShader(shaderType)} {
            assert_no_errors("glCreateShader");
            if (handle == 0) {
                throw std::runtime_error{"glCreateShader() failed"};
            }
        }
        Shader(Shader const&) = delete;
        Shader(Shader&& tmp) : handle{tmp.handle} {
            tmp.handle = 0;
        }
        Shader& operator=(Shader const&) = delete;
        Shader& operator=(Shader&&) = delete;
        ~Shader() noexcept {
            if (handle != 0) {
                glDeleteShader(handle);
            }
        }

        operator GLuint () noexcept {
            return handle;
        }
    };

    void AttachShader(Program& p, Shader& s) {
        glAttachShader(p, s);
        assert_no_errors("glAttachShader");
    }

    struct Vertex_shader final : public Shader {
        static Vertex_shader Compile(char const* src) {
            return Vertex_shader{Shader::Compile(GL_VERTEX_SHADER, src)};
        }
    };

    struct Fragment_shader final : public Shader {
        static Fragment_shader Compile(char const* src) {
            return Fragment_shader{Shader::Compile(GL_FRAGMENT_SHADER, src)};
        }
    };

    void LinkProgram(gl::Program& prog) {
        glLinkProgram(prog);

        GLint link_status = GL_FALSE;
        glGetProgramiv(prog, GL_LINK_STATUS, &link_status);
        if (link_status == GL_FALSE) {
            GLint log_len = 0;
            glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &log_len);

            std::vector<GLchar> errmsg(log_len);
            glGetProgramInfoLog(prog, errmsg.size(), nullptr, errmsg.data());

            std::stringstream ss;
            ss << "OpenGL: glLinkProgram() failed: ";
            ss << errmsg.data();
            throw std::runtime_error{ss.str()};
        }
    }

    class Uniform {
    protected:
        GLint handle;
    public:
        Uniform(Program& p, char const* name) :
            handle{glGetUniformLocation(p, name)} {

            if (handle == -1) {
                throw std::runtime_error{"glGetUniformLocation() failed: cannot get "s + name};
            }
        }

        operator GLint () noexcept {
            return handle;
        }
    };

    struct Uniform1f final : public Uniform {
        Uniform1f(Program& p, char const* name) :
            Uniform{p, name} {
        }
    };

    void Uniform(Uniform1f& u, GLfloat value) {
        glUniform1f(u, value);
    }

    struct Uniform1i final : public Uniform {
        Uniform1i(Program& p, char const* name) :
            Uniform{p, name} {
        }
    };

    void Uniform(Uniform1i& u, GLint value) {
        glUniform1i(u, value);
    }

    struct UniformMatrix4fv final : public Uniform {
        UniformMatrix4fv(Program& p, char const* name) :
            Uniform{p, name} {
        }
    };

    void Uniform(UniformMatrix4fv& u, GLfloat const* value) {
        glUniformMatrix4fv(u, 1, false, value);
    }

    struct UniformVec4f final : public Uniform {
        UniformVec4f(Program& p, char const* name) :
            Uniform{p, name} {
        }
    };

    struct UniformVec3f final : public Uniform {
        UniformVec3f(Program& p, char const* name) :
            Uniform{p, name} {
        }
    };

    class Attribute final {
        GLint handle;
    public:
        Attribute(Program& p, char const* name) :
            handle{glGetAttribLocation(p, name)} {
            if (handle == -1) {
                throw std::runtime_error{"glGetAttribLocation() failed: cannot get "s + name};
            }
        }

        operator GLint () noexcept {
            return handle;
        }
    };

    void VertexAttributePointer(Attribute& a,
                                GLint size,
                                GLenum type,
                                GLboolean normalized,
                                GLsizei stride,
                                const void * pointer) {
        glVertexAttribPointer(a, size, type, normalized, stride, pointer);
    }

    void EnableVertexAttribArray(Attribute& a) {
        glEnableVertexAttribArray(a);
    }

    class Buffer {
        GLuint handle = static_cast<GLuint>(-1);
    public:
        Buffer(GLenum target) {
            glGenBuffers(1, &handle);
        }
        Buffer(Buffer const&) = delete;
        Buffer(Buffer&& tmp) : handle{tmp.handle} {
            tmp.handle = static_cast<GLuint>(-1);
        }
        Buffer& operator=(Buffer const&) = delete;
        Buffer& operator=(Buffer&&) = delete;
        ~Buffer() noexcept {
            if (handle != static_cast<GLuint>(-1)) {
                glDeleteBuffers(1, &handle);
            }
        }

        operator GLuint () noexcept {
            return handle;
        }
    };

    void BindBuffer(GLenum target, Buffer& buffer) {
        glBindBuffer(target, buffer);
    }

    void BufferData(GLenum target, size_t num_bytes, void const* data, GLenum usage) {
        glBufferData(target, num_bytes, data, usage);
    }

    struct Array_buffer final : public Buffer {
        Array_buffer() : Buffer{GL_ARRAY_BUFFER} {
        }
    };

    void BindBuffer(Array_buffer& buffer) {
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
    }

    void BufferData(Array_buffer&, size_t num_bytes, void const* data, GLenum usage) {
        glBufferData(GL_ARRAY_BUFFER, num_bytes, data, usage);
    }

    struct Element_array_buffer final : public Buffer {
        Element_array_buffer() : Buffer{GL_ELEMENT_ARRAY_BUFFER} {
        }
    };

    void BindBuffer(Element_array_buffer& buffer) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    }

    void BufferData(Element_array_buffer&, size_t num_bytes, void const* data, GLenum usage) {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_bytes, data, usage);
    }

    class Vertex_array final {
        GLuint handle = static_cast<GLuint>(-1);
    public:
        Vertex_array() {
            glGenVertexArrays(1, &handle);
        }
        Vertex_array(Vertex_array const&) = delete;
        Vertex_array(Vertex_array&& tmp) :
            handle{tmp.handle} {
            tmp.handle = static_cast<GLuint>(-1);
        }
        Vertex_array& operator=(Vertex_array const&) = delete;
        Vertex_array& operator=(Vertex_array&&) = delete;
        ~Vertex_array() noexcept {
            if (handle == static_cast<GLuint>(-1)) {
                glDeleteVertexArrays(1, &handle);
            }
        }

        operator GLuint () noexcept {
            return handle;
        }
    };

    void BindVertexArray(Vertex_array& vao) {
        glBindVertexArray(vao);
    }

    void BindVertexArray() {
        glBindVertexArray(static_cast<GLuint>(0));
    }

    class Texture {
        GLuint handle = static_cast<GLuint>(-1);
    public:
        Texture() {
            glGenTextures(1, &handle);
        }
        Texture(Texture const&) = delete;
        Texture(Texture&& tmp) : handle{tmp.handle} {
            tmp.handle = static_cast<GLuint>(-1);
        }
        Texture& operator=(Texture const&) = delete;
        Texture& operator=(Texture&&) = delete;
        ~Texture() noexcept {
            if (handle != static_cast<GLuint>(-1)) {
                glDeleteTextures(1, &handle);
            }
        }

        operator GLuint () noexcept {
            return handle;
        }
    };

    struct Texture_2d final : public Texture {
        Texture_2d() : Texture{} {
        }
    };

    void BindTexture(Texture_2d& texture) {
        glBindTexture(GL_TEXTURE_2D, texture);
    }

    void BindTexture() {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void GenerateMipMap(Texture_2d&) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
}

namespace glglm {
    void Uniform(gl::UniformMatrix4fv& u, glm::mat4 const& mat) {
        gl::Uniform(u, glm::value_ptr(mat));
    }

    void Uniform(gl::UniformVec4f& u, glm::vec4 const& v) {
        glUniform4f(u, v.x, v.y, v.z, v.w);
    }

    void Uniform(gl::UniformVec3f& u, glm::vec3 const& v) {
        glUniform3f(u, v.x, v.y, v.z);
    }
}

namespace stbi {
    struct Image final {
        int width;
        int height;
        int nrChannels;
        unsigned char* data;

        Image(char const* path) :
            data{stbi_load(path, &width, &height, &nrChannels, 0)} {
            if (data == nullptr) {
                throw std::runtime_error{"stbi_load failed for '"s + path + "' : " + stbi_failure_reason()};
            }
        }
        Image(Image const&) = delete;
        Image(Image&&) = delete;
        Image& operator=(Image const&) = delete;
        Image& operator=(Image&&) = delete;
        ~Image() noexcept {
            stbi_image_free(data);
        }
    };
}

namespace stbigl {
    void TexImage2D(gl::Texture_2d const&, GLint level, stbi::Image const& image) {
        stbi_set_flip_vertically_on_load(true);
        glTexImage2D(GL_TEXTURE_2D,
                     level,
                     GL_RGB,
                     image.width,
                     image.height,
                     0,
                     image.nrChannels == 3 ? GL_RGB : GL_RGBA,
                     GL_UNSIGNED_BYTE,
                     image.data);
    }
}

namespace glm {
    std::ostream& operator<<(std::ostream& o, vec3 const& v) {
        return o << "[" << v.x << ", " << v.y << ", " << v.z << "]";
    }

    std::ostream& operator<<(std::ostream& o, vec4 const& v) {
        return o << "[" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << "]";
    }

    std::ostream& operator<<(std::ostream& o, mat4 const& m) {
        o << "[";
        for (auto i = 0U; i < 3; ++i) {
            o << m[i];
            o << ", ";
        }
        o << m[3];
        o << "]";
        return o;
    }
}

namespace ig {
    struct Context {
        Context() {
            ImGui::CreateContext();
        }
        Context(Context const&) = delete;
        Context(Context&&) = delete;
        Context& operator=(Context const&) = delete;
        Context& operator=(Context&&) = delete;
        ~Context() noexcept {
            ImGui::DestroyContext();
        }
    };

    struct SDL2_Context {
        SDL2_Context(SDL_Window* w, SDL_GLContext gl) {
            ImGui_ImplSDL2_InitForOpenGL(w, gl);
        }
        SDL2_Context(SDL2_Context const&) = delete;
        SDL2_Context(SDL2_Context&&) = delete;
        SDL2_Context& operator=(SDL2_Context const&) = delete;
        SDL2_Context& operator=(SDL2_Context&&) = delete;
        ~SDL2_Context() noexcept {
            ImGui_ImplSDL2_Shutdown();
        }
    };

    struct OpenGL3_Context {
        OpenGL3_Context(char const* version) {
            ImGui_ImplOpenGL3_Init(version);
        }
        OpenGL3_Context(OpenGL3_Context const&) = delete;
        OpenGL3_Context(OpenGL3_Context&&) = delete;
        OpenGL3_Context& operator=(OpenGL3_Context const&) = delete;
        OpenGL3_Context& operator=(OpenGL3_Context&&) = delete;
        ~OpenGL3_Context() noexcept {
            ImGui_ImplOpenGL3_Shutdown();
        }
    };
}

namespace  osim {
    std::ostream& operator<<(std::ostream& o, osim::Cylinder const& c) {
        o << "cylinder:"  << std::endl
          << "    scale = " << c.scale << std::endl
          << "    rgba = " << c.rgba << std::endl
          << "    transform = " << c.transform << std::endl;
        return o;
    }
    std::ostream& operator<<(std::ostream& o, osim::Line const& l) {
        o << "line:" << std::endl
          << "     p1 = " << l.p1 << std::endl
          << "     p2 = " << l.p2 << std::endl
          << "     rgba = " << l.rgba << std::endl;
        return o;
    }
    std::ostream& operator<<(std::ostream& o, osim::Sphere const& s) {
        o << "sphere:" << std::endl
          << "    transform = " << s.transform << std::endl
          << "    color = " << s.rgba << std::endl
          << "    radius = " << s.radius << std::endl;
        return o;
    }
    std::ostream& operator<<(std::ostream& o, osim::Mesh const& m) {
        o << "mesh:" << std::endl
          << "    transform = " << m.transform << std::endl
          << "    scale = " << m.scale << std::endl
          << "    rgba = " << m.rgba << std::endl
          << "    num_triangles = " << m.triangles.size() << std::endl;
        return o;
    }
    std::ostream& operator<<(std::ostream& o, osim::Geometry const& g) {
        std::visit([&](auto concrete) { o << concrete; }, g);
        return o;
    }
}

#if __APPLE__
    static const char* glsl_version = "#version 150";
#else
    static const char* glsl_version = "#version 130";
#endif

namespace ui {
    sdl::Window init_gl_window(sdl::Context&) {
#if __APPLE__
    // GL 3.2 Core + GLSL 150
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);
        SDL_GL_SetSwapInterval(1);  // vsync

        return sdl::Window{
                "Model Visualizer v" OSIMSNIPPETS_VERSION_STRING,
                SDL_WINDOWPOS_CENTERED,
                SDL_WINDOWPOS_CENTERED,
                1024,
                1024,
                SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE,
        };
    }

    sdl::Renderer init_gl_renderer(sdl::Window& window, sdl::GLContext& gl_ctx) {
        if (SDL_GL_MakeCurrent(window, gl_ctx) != 0) {
            throw std::runtime_error{"SDL_GL_MakeCurrent failed: "s  + SDL_GetError()};
        }

        if (auto err = glewInit(); err != GLEW_OK) {
            std::stringstream ss;
            ss << "glewInit() failed: ";
            ss << glewGetErrorString(err);
            throw std::runtime_error{ss.str()};
        }

        return sdl::Renderer{
                window,
                -1,
                SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
        };
    }

    struct State {
        sdl::Context context = {};
        sdl::ttf::Context ttf_context = {};
        sdl::Window window = init_gl_window(context);
        sdl::GLContext gl{window};
        sdl::Renderer renderer = init_gl_renderer(window, gl);
    };
}

namespace examples::imgui {
    static const char vertex_shader_src[] = R"(
        #version 410

        uniform mat4 projMat;
        uniform mat4 viewMat;
        uniform mat4 modelMat;

        in vec3 location;
        in vec3 in_normal;

        out vec3 normal;
        out vec3 frag_pos;

        void main() {
            // apply xforms (model, view, perspective) to vertex
            gl_Position = projMat * viewMat * modelMat * vec4(location, 1.0f);
            // passthrough the normals (used by frag shader)
            normal = in_normal;
            // pass fragment pos in world coordinates to frag shader
            frag_pos = vec3(modelMat * vec4(location, 1.0f));
        }
    )";

    static const char frag_shader_src[] = R"(
        #version 410

        uniform vec4 rgba;
        uniform vec3 lightPos;
        uniform vec3 lightColor;
        uniform vec3 viewPos;

        in vec3 normal;
        in vec3 frag_pos;

        out vec4 color;

        void main() {
            // normalized surface normal
            vec3 norm = normalize(normal);
            // direction of light, relative to fragment, in world coords
            vec3 light_dir = normalize(lightPos - frag_pos);

            // strength of diffuse (Phong model) lighting
            float diffuse_strength = 0.3f;
            float diff = max(dot(norm, light_dir), 0.0);
            vec3 diffuse = diffuse_strength * diff * lightColor;

            // strength of ambient (Phong model) lighting
            float ambient_strength = 0.5f;
            vec3 ambient = ambient_strength * lightColor;

            // strength of specular (Blinn-Phong model) lighting
            // Blinn-Phong is a modified Phong model that
            float specularStrength = 0.1f;
            vec3 lightDir = normalize(lightPos - frag_pos);
            vec3 viewDir = normalize(viewPos - frag_pos);
            vec3 halfwayDir = normalize(lightDir + viewDir);
            vec3 reflectDir = reflect(-light_dir, norm);
            float spec = pow(max(dot(normal, halfwayDir), 0.0), 32);
            vec3 specular = specularStrength * spec * lightColor;

            color = vec4((ambient + diffuse + specular) * rgba.rgb, rgba.a);
        }
    )";

    // Vector of 3 floats with no padding, so that it can be passed to OpenGL
    struct Vec3 {
        GLfloat x;
        GLfloat y;
        GLfloat z;
    };

    struct Mesh_point {
        Vec3 position;
        Vec3 normal;
    };

    // Returns triangles of a "unit" (radius = 1.0f, origin = 0,0,0) sphere
    std::vector<Mesh_point> unit_sphere_triangles() {
        // this is a shitty alg that produces a shitty UV sphere. I don't have
        // enough time to implement something better, like an isosphere, or
        // something like a patched sphere:
        //
        // https://www.iquilezles.org/www/articles/patchedsphere/patchedsphere.htm
        //
        // This one is adapted from:
        //    http://www.songho.ca/opengl/gl_sphere.html#example_cubesphere

        unsigned sectors = 32;
        unsigned stacks = 16;

        // polar coords, with [0, 0, -1] pointing towards the screen with polar
        // coords theta = 0, phi = 0. The coordinate [0, 1, 0] is theta = (any)
        // phi = PI/2. The coordinate [1, 0, 0] is theta = PI/2, phi = 0
        std::vector<Mesh_point> points;

        float theta_step = 2.0*M_PI / sectors;
        float phi_step = M_PI / stacks;

        for (unsigned stack = 0; stack <= stacks; ++stack) {
            float phi = M_PI/2.0f - stack*phi_step;
            float y = sin(phi);

            for (unsigned sector = 0; sector <= sectors; ++sector) {
                float theta = sector * theta_step;
                float x = sin(theta) * cos(phi);
                float z = -cos(theta) * cos(phi);
                points.push_back(Mesh_point{
                    .position = {x, y, z},
                    .normal = {x, y, z},  // sphere is at the origin, so nothing fancy needed
                });
            }
        }

        // the points are not triangles. They are *points of a triangle*, so the
        // points must be triangulated
        std::vector<Mesh_point> triangles;

        for (unsigned stack = 0; stack < stacks; ++stack) {
            unsigned k1 = stack * (sectors + 1);
            unsigned k2 = k1 + sectors + 1;

            for (unsigned sector = 0; sector < sectors; ++sector, ++k1, ++k2) {
                // 2 triangles per sector - excluding the first and last stacks
                // (which contain one triangle, at the poles)
                Mesh_point p1 = points.at(k1);
                Mesh_point p2 = points.at(k2);
                Mesh_point p1_plus1 = points.at(k1+1);
                Mesh_point p2_plus1 = points.at(k2+1);

                if (stack != 0) {
                    triangles.push_back(p1);
                    triangles.push_back(p2);
                    triangles.push_back(p1_plus1);
                }

                if (stack != (stacks-1)) {
                    triangles.push_back(p1_plus1);
                    triangles.push_back(p2);
                    triangles.push_back(p2_plus1);
                }
            }
        }

        return triangles;
    }

    // Returns triangles for a "unit" cylinder with `num_sides` sides.
    //
    // Here, "unit" means:
    //
    // - radius == 1.0f
    // - top == [0.0f, 0.0f, -1.0f]
    // - bottom == [0.0f, 0.0f, +1.0f]
    // - (so the height is 2.0f, not 1.0f)
    std::vector<Mesh_point> unit_cylinder_triangles(unsigned num_sides) {
        // TODO: this is dumb because a cylinder can be EBO-ed quite easily, which
        //       would reduce the amount of vertices needed
        if (num_sides < 3) {
            throw std::runtime_error{"cannot create a cylinder with fewer than 3 sides"};
        }

        std::vector<Mesh_point> rv;
        rv.reserve(2*3*num_sides + 6*num_sides);

        float step_angle = (2*M_PI)/num_sides;
        float top_z = -1.0f;
        float bottom_z = +1.0f;

        // top
        {
            Vec3 normal = {0.0f, 0.0f, -1.0f};
            Mesh_point top_middle = {
                .position = {0.0f, 0.0f, top_z},
                .normal = normal,
            };
            for (auto i = 0U; i < num_sides; ++i) {
                float theta_start = i*step_angle;
                float theta_end = (i+1)*step_angle;
                rv.push_back(top_middle);
                rv.push_back(Mesh_point {
                    .position = {
                        .x = sin(theta_start),
                        .y = cos(theta_start),
                        .z = top_z,
                    },
                    .normal = normal,
                });
                rv.push_back(Mesh_point {
                     .position = {
                        .x = sin(theta_end),
                        .y = cos(theta_end),
                        .z = top_z,
                    },
                    .normal = normal,
                });
            }
        }

        // bottom
        {
            Vec3 bottom_normal = {0.0f, 0.0f, -1.0f};
            Mesh_point top_middle = {
                .position = {0.0f, 0.0f, bottom_z},
                .normal = bottom_normal,
            };
            for (auto i = 0U; i < num_sides; ++i) {
                float theta_start = i*step_angle;
                float theta_end = (i+1)*step_angle;

                rv.push_back(top_middle);
                rv.push_back(Mesh_point {
                    .position = {
                        .x = sin(theta_start),
                        .y = cos(theta_start),
                        .z = bottom_z,
                    },
                    .normal = bottom_normal,
                });
                rv.push_back(Mesh_point {
                     .position = {
                        .x = sin(theta_end),
                        .y = cos(theta_end),
                        .z = bottom_z,
                    },
                    .normal = bottom_normal,
                });
            }
        }

        // sides
        {
            float norm_start = step_angle/2.0f;
            for (auto i = 0U; i < num_sides; ++i) {
                float theta_start = i * step_angle;
                float theta_end = theta_start + step_angle;
                float norm_theta = theta_start + norm_start;

                Vec3 normal = {sin(norm_theta), cos(norm_theta), 0.0f};
                Vec3 top1 = {sin(theta_start), cos(theta_start), top_z};
                Vec3 top2 = {sin(theta_end), cos(theta_end), top_z};
                Vec3 bottom1 = top1;
                bottom1.z = bottom_z;
                Vec3 bottom2 = top2;
                bottom2.z = bottom_z;

                rv.push_back(Mesh_point{top1, normal});
                rv.push_back(Mesh_point{top2, normal});
                rv.push_back(Mesh_point{bottom1, normal});

                rv.push_back(Mesh_point{bottom1, normal});
                rv.push_back(Mesh_point{bottom2, normal});
                rv.push_back(Mesh_point{top2, normal});
            }
        }

        return rv;
    }

    // Basic mesh composed of triangles with normals for all vertices
    struct Triangle_mesh {
        unsigned num_verts;
        gl::Array_buffer vbo;
        gl::Vertex_array vao;

        Triangle_mesh(gl::Attribute& in_attr,
                      gl::Attribute& normal_attr,
                      std::vector<Mesh_point> const& points) :
            num_verts(points.size()) {

            gl::BindVertexArray(vao);
            {
                gl::BindBuffer(vbo);
                gl::BufferData(vbo, sizeof(Mesh_point) * points.size(), points.data(), GL_STATIC_DRAW);
                gl::VertexAttributePointer(in_attr, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh_point), 0);
                gl::VertexAttributePointer(normal_attr, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh_point), (void*)sizeof(Vec3));
                gl::EnableVertexAttribArray(in_attr);
            }
            gl::BindVertexArray();
        }
    };

    Triangle_mesh gen_cylinder_mesh(gl::Attribute& in_attr,
                                    gl::Attribute& normal_attr,
                                    unsigned num_sides) {
        auto points = unit_cylinder_triangles(num_sides);
        return Triangle_mesh{in_attr, normal_attr, points};
    }

    Triangle_mesh gen_sphere_mesh(gl::Attribute& in_attr,
                                  gl::Attribute& normal_attr) {
        auto points = unit_sphere_triangles();
        return Triangle_mesh{in_attr, normal_attr, points};
    }

    struct App_static_glstate {
        gl::Program program;

        gl::UniformMatrix4fv projMat;
        gl::UniformMatrix4fv viewMat;
        gl::UniformMatrix4fv modelMat;
        gl::UniformVec4f rgba;
        gl::UniformVec3f light_pos;
        gl::UniformVec3f light_color;
        gl::UniformVec3f view_pos;

        gl::Attribute location;
        gl::Attribute in_normal;

        Triangle_mesh cylinder;
        Triangle_mesh sphere;
    };

    App_static_glstate initialize() {
        auto program = gl::Program{};
        auto vertex_shader = gl::Vertex_shader::Compile(vertex_shader_src);
        gl::AttachShader(program, vertex_shader);
        auto frag_shader = gl::Fragment_shader::Compile(frag_shader_src);
        gl::AttachShader(program, frag_shader);

        gl::LinkProgram(program);

        auto projMat = gl::UniformMatrix4fv{program, "projMat"};
        auto viewMat = gl::UniformMatrix4fv{program, "viewMat"};
        auto modelMat = gl::UniformMatrix4fv{program, "modelMat"};
        auto rgba = gl::UniformVec4f{program, "rgba"};
        auto light_pos = gl::UniformVec3f{program, "lightPos"};
        auto light_color = gl::UniformVec3f{program, "lightColor"};
        auto view_pos = gl::UniformVec3f{program, "viewPos"};

        auto in_position = gl::Attribute{program, "location"};
        auto in_normal = gl::Attribute{program, "in_normal"};

        return App_static_glstate {
            .program = std::move(program),

            .projMat = std::move(projMat),
            .viewMat = std::move(viewMat),
            .modelMat = std::move(modelMat),
            .rgba = std::move(rgba),
            .light_pos = std::move(light_pos),
            .light_color = std::move(light_color),
            .view_pos = std::move(view_pos),

            .location = std::move(in_position),
            .in_normal = std::move(in_normal),

            .cylinder = gen_cylinder_mesh(in_position, in_normal, 24),
            .sphere = gen_sphere_mesh(in_position, in_normal),
        };
    }

    struct Line {
        gl::Array_buffer vbo;
        gl::Vertex_array vao;
        osim::Line data;

        Line(gl::Attribute& in_attr, osim::Line const& _data) :
            data{_data} {
            Vec3 points[2] = {
                {_data.p1.x, _data.p1.y, _data.p1.z},
                {_data.p2.x, _data.p2.y, _data.p2.z},
            };
            gl::BindVertexArray(vao);
            {
                gl::BindBuffer(vbo);
                gl::BufferData(vbo, sizeof(points), points, GL_STATIC_DRAW);
                gl::VertexAttributePointer(in_attr, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
                gl::EnableVertexAttribArray(in_attr);
            }
            gl::BindVertexArray();
        }
    };

    std::tuple<Mesh_point, Mesh_point, Mesh_point> to_mesh_points(osim::Triangle const& t) {
        glm::vec3 normal = (t.p2 - t.p1) * (t.p3 - t.p1);
        Vec3 normal_vec3 = Vec3{normal.x, normal.y, normal.z};

        return {
            Mesh_point{Vec3{t.p1.x, t.p1.y, t.p1.z}, normal_vec3},
            Mesh_point{Vec3{t.p2.x, t.p2.y, t.p2.z}, normal_vec3},
            Mesh_point{Vec3{t.p3.x, t.p3.y, t.p3.z}, normal_vec3}
        };
    }

    // TODO: this is hacked together
    Triangle_mesh make_mesh(gl::Attribute& in_attr, gl::Attribute& in_normal, osim::Mesh const& data) {
        std::vector<Mesh_point> triangles;
        triangles.reserve(3*data.triangles.size());
        for (osim::Triangle const& t : data.triangles) {
            auto [p1, p2, p3] = to_mesh_points(t);
            triangles.emplace_back(p1);
            triangles.emplace_back(p2);
            triangles.emplace_back(p3);
        }
        return Triangle_mesh{in_attr, in_normal, triangles};
    }

    struct Osim_mesh {
        osim::Mesh data;
        Triangle_mesh mesh;

        Osim_mesh(gl::Attribute& in_attr, gl::Attribute& in_normal, osim::Mesh _data) :
            data{std::move(_data)},
            mesh{make_mesh(in_attr, in_normal, data)} {
        }
    };

    struct ModelState {
        std::vector<osim::Cylinder> cylinders;
        std::vector<Line> lines;
        std::vector<osim::Sphere> spheres;
        std::vector<Osim_mesh> meshes;
    };

    ModelState load_model(App_static_glstate& gls, std::string_view path) {
        ModelState rv;
        for (osim::Geometry const& g : osim::geometry_in(path)) {
            std::visit(overloaded {
                [&](osim::Cylinder const& c) {
                    rv.cylinders.push_back(c);
                },
                [&](osim::Line const& l) {
                    rv.lines.emplace_back(gls.location, l);
                },
                [&](osim::Sphere const& s) {
                    rv.spheres.push_back(s);
                },
                [&](osim::Mesh const& m) {
                    rv.meshes.emplace_back(gls.location, gls.in_normal, m);
                }
            }, g);
        }
        return rv;
    }

    struct ScreenDims {
        int w = 0;
        int h = 0;

        ScreenDims(std::pair<int, int> p) :
            w{p.first}, h{p.second} {
        }
    };

    std::string slurp_file(std::string const& path) {
        auto fs = std::ifstream{path};
        auto ss = std::stringstream{};

        if (not fs) {
            throw std::runtime_error{path + ": error opening path"};
        }
        ss << fs.rdbuf();
        return ss.str();
    }

    void show(ui::State& s, std::string file) {
        // OpenGL
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_ALPHA_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // ImGUI
        auto imgui_ctx = ig::Context{};
        auto imgui_sdl2_ctx = ig::SDL2_Context{s.window, s.gl};
        auto imgui_ogl3_ctx = ig::OpenGL3_Context{glsl_version};
        ImGui::StyleColorsLight();
        ImGuiIO& io = ImGui::GetIO();


        // Unchanging OpenGL state (which programs are used, uniforms, etc.)
        App_static_glstate gls = initialize();


        // Mutable runtime state
        ModelState ms = load_model(gls, file);

        bool wireframe_mode = false;
        ScreenDims window_dims = sdl::window_size(s.window);
        float radius = 1.0f;
        float wheel_sensitivity = 0.9f;
        float line_width = 0.002f;

        float fov = 120.0f;

        bool dragging = false;
        float theta = 0.0f;
        float phi = 0.0f;
        float sensitivity = 1.0f;

        bool panning = false;
        glm::vec3 pan = {0.0f, 0.0f, 0.0f};

        // initial pan position is the average center of *some of the* geometry
        // in the scene, which is found in an extremely dumb way.
        {
            glm::vec3 middle = {0.0f, 0.0f, 0.0f};
            unsigned n = 0;
            auto update_middle = [&](glm::vec3 const& v) {
                middle = glm::vec3{
                    (n*middle.x - v.x)/(n+1),
                    (n*middle.y - v.y)/(n+1),
                    (n*middle.z - v.z)/(n+1)
                };
                ++n;
            };

            for (auto const& l : ms.lines) {
                update_middle(l.data.p1);
                update_middle(l.data.p2);
            }

            for (auto const& p : ms.spheres) {
                glm::vec3 translation = {p.transform[3][0], p.transform[3][1], p.transform[3][2]};
                update_middle(translation);
            }

            pan = middle;
        }

        auto light_pos = glm::vec3{1.0f, 1.0f, 0.0f};
        float light_color[3] = {0.98f, 0.95f, 0.95f};
        bool show_light = false;
        bool show_unit_cylinder = false;
        bool gamma_correction = false;

        while (true) {
            if (gamma_correction) {
                glEnable(GL_FRAMEBUFFER_SRGB);
            } else {
                glDisable(GL_FRAMEBUFFER_SRGB);
            }

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glPolygonMode(GL_FRONT_AND_BACK, wireframe_mode ? GL_LINE : GL_FILL);

            glEnableClientState(GL_VERTEX_ARRAY);
            gl::UseProgram(gls.program);

            // set *invariant* uniforms
            auto rot_theta = glm::rotate(glm::identity<glm::mat4>(), -theta, glm::vec3{0.0f, 1.0f, 0.0f});
            auto theta_vec = glm::normalize(glm::vec3{sin(theta), 0.0f, cos(theta)});
            auto phi_axis = glm::cross(theta_vec, glm::vec3{0.0, 1.0f, 0.0f});
            auto rot_phi = glm::rotate(glm::identity<glm::mat4>(), -phi, phi_axis);
            auto pan_translate = glm::translate(glm::identity<glm::mat4>(), pan);
            float aspect_ratio = static_cast<float>(window_dims.w)/static_cast<float>(window_dims.h);
            {
                auto proj_matrix = glm::perspective(fov, aspect_ratio, 0.1f, 100.0f);
                // camera: at a fixed position pointing at a fixed origin. The "camera" works by translating +
                // rotating all objects around that origin. Rotation is expressed as polar coordinates. Camera
                // panning is represented as a translation vector.
                auto camera_pos = glm::vec3(0.0f, 0.0f, radius);
                glglm::Uniform(gls.projMat, proj_matrix);
                glglm::Uniform(gls.viewMat,
                               glm::lookAt(camera_pos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3{0.0f, 1.0f, 0.0f}) * rot_theta * rot_phi * pan_translate);
                glglm::Uniform(gls.light_pos, light_pos);
                glglm::Uniform(gls.light_color, glm::vec3(light_color[0], light_color[1], light_color[2]));
                glglm::Uniform(gls.view_pos, glm::vec3{radius * sin(theta) * cos(phi), radius * sin(phi), radius * cos(theta) * cos(phi)});
            }

            for (auto const& c : ms.cylinders) {
                gl::BindVertexArray(gls.cylinder.vao);
                glglm::Uniform(gls.rgba, c.rgba);

                // simbody defines a cylinder's top+bottom as +Y/-Y
                auto hacky_cylinder_correction = glm::rotate(glm::identity<glm::mat4>(), static_cast<float>(M_PI/2.0f), glm::vec3{1.0f, 0.0f, 0.0f});

                auto scaler = glm::scale(c.transform, c.scale);
                glglm::Uniform(gls.modelMat, scaler * hacky_cylinder_correction);
                glDrawArrays(GL_TRIANGLES, 0, gls.cylinder.num_verts);
                gl::BindVertexArray();
            }

            for (auto const& c : ms.spheres) {
                gl::BindVertexArray(gls.sphere.vao);
                glglm::Uniform(gls.rgba, c.rgba);
                auto scaler = glm::scale(c.transform, glm::vec3{c.radius, c.radius, c.radius});
                glglm::Uniform(gls.modelMat, scaler);
                glDrawArrays(GL_TRIANGLES, 0, gls.sphere.num_verts);
                gl::BindVertexArray();
            }

            for (auto& l : ms.lines) {
                gl::BindVertexArray(gls.cylinder.vao);
                //gl::BindVertexArray(l.vao);

                // color
                glglm::Uniform(gls.rgba, l.data.rgba);

                glm::vec3 p1_to_p2 = l.data.p2 - l.data.p1;
                glm::vec3 c1_to_c2 = glm::vec3{0.0f, 0.0f, 2.0f};
                auto rotation =
                        glm::rotate(glm::identity<glm::mat4>(),
                                    glm::acos(glm::dot(glm::normalize(c1_to_c2), glm::normalize(p1_to_p2))),
                                    glm::cross(glm::normalize(c1_to_c2), glm::normalize(p1_to_p2)));
                float scale = glm::length(p1_to_p2)/glm::length(c1_to_c2);
                auto scale_xform = glm::scale(glm::identity<glm::mat4>(), glm::vec3{line_width, line_width, scale});
                auto translation = glm::translate(glm::identity<glm::mat4>(), l.data.p1 + p1_to_p2/2.0f);

                glglm::Uniform(gls.modelMat, translation * rotation * scale_xform);
                glDrawArrays(GL_TRIANGLES, 0, gls.cylinder.num_verts);
                //glDrawArrays(GL_LINES, 0, 2);
                gl::BindVertexArray();
            }

            for (auto& m : ms.meshes) {
                gl::BindVertexArray(m.mesh.vao);
                glglm::Uniform(gls.rgba, m.data.rgba);
                auto scaler = glm::scale(m.data.transform, m.data.scale);
                glglm::Uniform(gls.modelMat, scaler);
                glDrawArrays(GL_TRIANGLES, 0, m.mesh.num_verts);
                gl::BindVertexArray();
            }

            // draw lamp
            if (show_light) {
                gl::BindVertexArray(gls.sphere.vao);
                glglm::Uniform(gls.rgba, glm::vec4{1.0f, 1.0f, 0.0f, 0.3f});
                glglm::Uniform(gls.modelMat, glm::scale(glm::translate(glm::identity<glm::mat4>(), light_pos), {0.05, 0.05, 0.05}));
                glDrawArrays(GL_TRIANGLES, 0 , gls.sphere.num_verts);
                gl::BindVertexArray();
            }

            if (show_unit_cylinder) {
                gl::BindVertexArray(gls.cylinder.vao);
                glglm::Uniform(gls.rgba, glm::vec4{0.9f, 0.9f, 0.9f, 1.0f});
                glglm::Uniform(gls.modelMat, glm::identity<glm::mat4>());
                glDrawArrays(GL_TRIANGLES, 0 , gls.cylinder.num_verts);
                gl::BindVertexArray();
            }

            gl::UseProgram();
            glDisableClientState(GL_VERTEX_ARRAY);

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL2_NewFrame(s.window);

            ImGui::NewFrame();
            bool b = true;
            //ImGui::ShowDemoWindow(&b);
            ImGui::Begin("Scene", &b, ImGuiWindowFlags_MenuBar);

            {
                std::stringstream fps;
                fps << "Fps: " << io.Framerate;
                ImGui::Text(fps.str().c_str());
            }
            ImGui::NewLine();

            ImGui::Text("Camera Position:");

            ImGui::NewLine();

            if (ImGui::Button("Front")) {
                // assumes models tend to point upwards in Y and forwards in +X
                theta = M_PI/2.0f;
                phi = 0.0f;
            }
            ImGui::SameLine();
            if (ImGui::Button("Back")) {
                // assumes models tend to point upwards in Y and forwards in +X
                theta = 3.0f * (M_PI/2.0f);
                phi = 0.0f;
            }

            ImGui::SameLine();
            ImGui::Text("|");
            ImGui::SameLine();

            if (ImGui::Button("Left")) {
                // assumes models tend to point upwards in Y and forwards in +X
                // (so sidewards is theta == 0 or PI)
                theta = M_PI;
                phi = 0.0f;
            }
            ImGui::SameLine();
            if (ImGui::Button("Right")) {
                // assumes models tend to point upwards in Y and forwards in +X
                // (so sidewards is theta == 0 or PI)
                theta = 0.0f;
                phi = 0.0f;
            }

            ImGui::SameLine();
            ImGui::Text("|");
            ImGui::SameLine();

            if (ImGui::Button("Top")) {
                theta = 0.0f;
                phi = M_PI/2.0f;
            }
            ImGui::SameLine();
            if (ImGui::Button("Bottom")) {
                theta = 0.0f;
                phi = 3.0f*(M_PI/2.0f);
            }

            ImGui::NewLine();

            ImGui::SliderFloat("radius", &radius, 0.0f, 10.0f);
            ImGui::SliderFloat("theta", &theta, 0.0f, 2*M_PI);
            ImGui::SliderFloat("phi", &phi, 0.0f, 2*M_PI);
            ImGui::NewLine();
            ImGui::SliderFloat("pan_x", &pan.x, -100.0f, 100.0f);
            ImGui::SliderFloat("pan_y", &pan.y, -100.0f, 100.0f);
            ImGui::SliderFloat("pan_z", &pan.z, -100.0f, 100.0f);

            ImGui::NewLine();
            ImGui::Text("Lighting:");
            ImGui::SliderFloat("light_x", &light_pos.x, -30.0f, 30.0f);
            ImGui::SliderFloat("light_y", &light_pos.y, -30.0f, 30.0f);
            ImGui::SliderFloat("light_z", &light_pos.z, -30.0f, 30.0f);
            ImGui::ColorEdit3("light_color", light_color);
            ImGui::Checkbox("show_light", &show_light);
            ImGui::Checkbox("show_unit_cylinder", &show_unit_cylinder);
            ImGui::SliderFloat("line_width", &line_width, 0.0f, 0.01f);
            ImGui::Checkbox("gamma_correction", &gamma_correction);

            ImGui::NewLine();
            ImGui::Text("Interaction:");
            if (dragging) {
                ImGui::Text("rotating");
            }
            if (panning) {
                ImGui::Text("panning");
            }


            ImGui::End();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // draw
            SDL_GL_SwapWindow(s.window);

            // event loop
            SDL_Event e;
            while (SDL_PollEvent(&e)) {
                ImGui_ImplSDL2_ProcessEvent(&e);
                if (e.type == SDL_QUIT) {
                    return;
                } else if (e.type == SDL_KEYDOWN) {
                    switch (e.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            return;  // quit visualizer
                        case SDLK_w:
                            wireframe_mode = not wireframe_mode;
                            break;
                    }
                } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                    switch (e.button.button) {
                        case SDL_BUTTON_LEFT:
                            dragging = true;
                            break;
                        case SDL_BUTTON_RIGHT:
                            panning = true;
                            break;
                    }
                } else if (e.type == SDL_MOUSEBUTTONUP) {
                    switch (e.button.button) {
                        case SDL_BUTTON_LEFT:
                            dragging = false;
                            break;
                        case SDL_BUTTON_RIGHT:
                            panning = false;
                            break;
                    }
                } else if (e.type == SDL_MOUSEMOTION) {
                    if (io.WantCaptureMouse) {
                        // if ImGUI wants to capture the mouse, then the mouse
                        // is probably interacting with an ImGUI panel and,
                        // therefore, the dragging/panning shouldn't be handled
                        continue;
                    }

                    if (abs(e.motion.xrel) > 200 or abs(e.motion.yrel) > 200) {
                        // probably a frameskip or the mouse was forcibly teleported
                        // because it hit the edge of the screen
                        continue;
                    }

                    if (dragging) {
                        // alter camera position while dragging
                        float dx = -static_cast<float>(e.motion.xrel)/static_cast<float>(window_dims.w);
                        float dy = static_cast<float>(e.motion.yrel)/static_cast<float>(window_dims.h);
                        theta += 2.0f * static_cast<float>(M_PI) * sensitivity * dx;
                        phi += 2.0f * static_cast<float>(M_PI) * sensitivity * dy;
                    }

                    if (panning) {
                        float dx = static_cast<float>(e.motion.xrel)/static_cast<float>(window_dims.w);
                        float dy = -static_cast<float>(e.motion.yrel)/static_cast<float>(window_dims.h);

                        // how much panning is done depends on how far the camera is from the
                        // origin (easy, with polar coordinates) *and* the FoV of the camera.
                        float x_amt = dx * aspect_ratio * (2.0f * tan(fov/2.0f) * radius);
                        float y_amt = dy * (1.0f/aspect_ratio) * (2.0f * tan(fov/2.0f) * radius);

                        // this assumes the scene is not rotated, so we need to rotate these
                        // axes to match the scene's rotation
                        glm::vec4 default_panning_axis = {x_amt, y_amt, 0.0f, 1.0f};
                        auto rot_theta = glm::rotate(glm::identity<glm::mat4>(), theta, glm::vec3{0.0f, 1.0f, 0.0f});
                        auto theta_vec = glm::normalize(glm::vec3{sin(theta), 0.0f, cos(theta)});
                        auto phi_axis = glm::cross(theta_vec, glm::vec3{0.0, 1.0f, 0.0f});
                        auto rot_phi = glm::rotate(glm::identity<glm::mat4>(), phi, phi_axis);

                        glm::vec4 panning_axes = rot_phi * rot_theta * default_panning_axis;
                        pan.x += panning_axes.x;
                        pan.y += panning_axes.y;
                        pan.z += panning_axes.z;
                    }

                    if (dragging or panning) {
                        // wrap mouse if it hits edges
                        constexpr int edge_width = 5;
                        if (e.motion.x + edge_width > window_dims.w) {
                            SDL_WarpMouseInWindow(s.window, edge_width, e.motion.y);
                        }
                        if (e.motion.x - edge_width < 0) {
                            SDL_WarpMouseInWindow(s.window, window_dims.w - edge_width, e.motion.y);
                        }
                        if (e.motion.y + edge_width > window_dims.h) {
                            SDL_WarpMouseInWindow(s.window, e.motion.x, edge_width);
                        }
                        if (e.motion.y - edge_width < 0) {
                            SDL_WarpMouseInWindow(s.window, e.motion.x, window_dims.h - edge_width);
                        }
                    }
                } else if (e.type == SDL_WINDOWEVENT) {
                    window_dims = sdl::window_size(s.window);
                    glViewport(0, 0, window_dims.w, window_dims.h);
                } else if (e.type == SDL_MOUSEWHEEL) {
                    if (e.wheel.y > 0 and radius >= 0.1f) {
                        radius *= wheel_sensitivity;
                    }

                    if (e.wheel.y <= 0 and radius < 100.0f) {
                        radius /= wheel_sensitivity;
                    }
                }
            }
        }
    }
}

int main(int argc, char** argv) {
    auto ui = ui::State{};

    if (argc <= 1) {
        examples::imgui::show(ui, rajagopal_model_path);
    } else {
        for (int i = 1; i < argc; ++i) {
            examples::imgui::show(ui, argv[i]);
        }
    }
};