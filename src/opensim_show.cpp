#include <SDL2/SDL.h>
#include "opensim_wrapper.hpp"

#include <SDL_ttf.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <string>
#include <exception>
#include <optional>
#include <vector>
#include <sstream>
#include <array>
#include <chrono>
#include <algorithm>
#include <iostream>


using std::string_literals::operator""s;


// helper type for the visitor #4
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

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
            if (SDL_Init(SDL_INIT_VIDEO)  != 0) {
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

        if (GLenum error = glGetError(); error != GL_NO_ERROR) {
            throw std::runtime_error{"glUseProgram() failed: "s + to_string(gluErrorString(error)) };
        }
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
        return std::nullopt;  // no errors
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
            if (handle == static_cast<GLuint>(-1)) {
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

namespace ui {
    sdl::Window init_gl_window(sdl::Context&) {
#if __APPLE__
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#else
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
#endif
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        // vsync SDL_GL_SetSwapInterval(0);

        //SDL_SetRelativeMouseMode(SDL_TRUE);
        //SDL_CaptureMouse(SDL_TRUE);

        return sdl::Window{
                "Some window",
                SDL_WINDOWPOS_CENTERED,
                SDL_WINDOWPOS_CENTERED,
                1024,
                1024,
                SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS,
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

namespace examples::fractal {
    const char vert_shader_src[] = R"(
        #version 140

        in vec2 LVertexPos2D;
        out vec2 VertPos;

        void main() {
           gl_Position = vec4( LVertexPos2D.x, LVertexPos2D.y, 0, 1 );
           VertPos = gl_Position.xy;
        }
    )";

    const char frag_shader_src[] = R"(
        #version 140

        uniform float x_rescale;
        uniform float x_offset;
        uniform float y_rescale;
        uniform float y_offset;
        uniform int num_iterations;

        in vec2 VertPos;
        out vec4 LFragment;

        void main() {
           float x0 = x_rescale*VertPos.x + x_offset;
           float y0 = y_rescale*VertPos.y + y_offset;
           float x = 0.0;
           float y = 0.0;
           float x2 = 0.0;
           float y2 = 0.0;

           int iter = 0;
           while (iter < num_iterations && x2+y2 <= 4.0) {
             y = 2*x*y + y0;
             x = x2-y2 + x0;
             x2 = x*x;
             y2 = y*y;
             iter++;
           }

           float brightness = iter == num_iterations ? 0.0 : float(iter)/float(num_iterations);

           LFragment = vec4(brightness, brightness, brightness, 1.0);
        }
    )";

    struct GLState {
        gl::Program program;
        gl::Vertex_shader vert_shader;
        gl::Fragment_shader frag_shader;

        gl::Array_buffer vbo;
        gl::Element_array_buffer ibo;
        gl::Vertex_array vao;

        gl::Attribute attrib_LVertexPos2D;
        gl::Uniform1f x_rescale;
        gl::Uniform1f x_offset;
        gl::Uniform1f y_rescale;
        gl::Uniform1f y_offset;
        gl::Uniform1i num_iterations;
    };

    GLState glinit() {
        auto program = gl::Program{};
        auto vert_shader = gl::Vertex_shader::Compile(vert_shader_src);
        gl::AttachShader(program, vert_shader);
        auto frag_shader = gl::Fragment_shader::Compile(frag_shader_src);
        gl::AttachShader(program, frag_shader);

        gl::LinkProgram(program);

        auto attrib_LVertexPos2D = gl::Attribute{program, "LVertexPos2D"};

        glClearColor(0.5f, 0.0f, 0.0f, 1.0f);

        GLfloat vbo_data[] = {
            -1.0, -1.0,  // bottom-left
            +1.0, -1.0,  // bottom-right
            +1.0, +1.0,  // top-right
            -1.0, +1.0,  // top-left
        };
        auto vbo = gl::Array_buffer{};
        gl::BindBuffer(vbo);
        gl::BufferData(vbo, sizeof(vbo_data), vbo_data, GL_STATIC_DRAW);

        GLuint ibo_data[] = { 0, 1, 2, 3 };
        auto ibo = gl::Element_array_buffer{};
        gl::BindBuffer(ibo);
        gl::BufferData(ibo, sizeof(ibo_data), ibo_data, GL_STATIC_DRAW);

        // set up vao
        auto vao = gl::Vertex_array{};
        gl::BindVertexArray(vao);

        // vao: vertex positions
        gl::BindBuffer(vbo);
        glEnableVertexAttribArray(vbo);
        glVertexAttribPointer(vbo, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

        // vao: LVertexPos2D
        glEnableVertexAttribArray(attrib_LVertexPos2D);
        gl::BindBuffer(vbo);
        glVertexAttribPointer(attrib_LVertexPos2D, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), nullptr);


        // vao: index buffer
        gl::BindBuffer(ibo);

        gl::BindVertexArray();

        auto x_rescale = gl::Uniform1f{program, "x_rescale"};
        auto x_offset = gl::Uniform1f{program, "x_offset"};
        auto y_rescale = gl::Uniform1f{program, "y_rescale"};
        auto y_offset = gl::Uniform1f{program, "y_offset"};
        auto num_iterations = gl::Uniform1i{program, "num_iterations"};

        return GLState{
            .program = std::move(program),
            .vert_shader = std::move(vert_shader),
            .frag_shader = std::move(frag_shader),

            .vbo = std::move(vbo),
            .ibo = std::move(ibo),
            .vao = std::move(vao),

            .attrib_LVertexPos2D = attrib_LVertexPos2D,
            .x_rescale = x_rescale,
            .x_offset = x_offset,
            .y_rescale = y_rescale,
            .y_offset = y_offset,
            .num_iterations = num_iterations,
        };
    }

    void show(ui::State& s) {
        auto p = glinit();

        SDL_Event e;
        for (;;) {
            glClear(GL_COLOR_BUFFER_BIT);
            glEnableClientState(GL_VERTEX_ARRAY);
            gl::UseProgram(p.program);
            gl::Uniform(p.x_rescale, 3.5/2.0);
            gl::Uniform(p.x_offset, 3.5/2.0 - 2.5);
            gl::Uniform(p.y_rescale, 2.0/2.0);
            gl::Uniform(p.y_offset, -2.0/2.0 + 1.0);
            gl::Uniform(p.num_iterations, 32);
            gl::BindVertexArray(p.vao);
            glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, nullptr);
            gl::BindVertexArray();
            gl::UseProgram();
            glDisableClientState(GL_VERTEX_ARRAY);

            SDL_GL_SwapWindow(s.window);

            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    return;
                } else if (e.type == SDL_KEYDOWN) {
                    switch (e.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            return;
                    }
                } else if (e.type = SDL_WINDOWEVENT) {
                    auto [w, h] = sdl::window_size(s.window);
                    glViewport(0, 0, w, h);
                }
            }
        }
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
        o << "mesh:" << std::endl;
        return o;
    }
    std::ostream& operator<<(std::ostream& o, osim::Geometry const& g) {
        std::visit([&](auto concrete) { o << concrete; }, g);
        return o;
    }
}

namespace examples::cube {
    static const char vertex_shader_src[] = R"(
        #version 430

        uniform mat4 projMat;
        uniform mat4 viewMat;
        uniform mat4 modelMat;

        in vec3 in_position;
        in vec2 in_uv;

        out vec2 uv;

        void main() {
            gl_Position = projMat * viewMat * modelMat * vec4(in_position, 1.0);
            uv = in_uv;
        }
    )";

    static const char frag_shader_src[] = R"(
        #version 430

        in vec2 uv;
        out vec4 color;

        uniform sampler2D texture1;
        uniform sampler2D texture2;
        uniform float mix_amt;

        void main() {
            vec2 uv_xflipped = vec2(1.0 - uv.x, uv.y);
            uv_xflipped *= 2.0;  // repeat
            color = mix(texture(texture1, uv), texture(texture2, uv_xflipped), mix_amt);
        }
    )";

    struct GLState {
        gl::Program program;

        gl::UniformMatrix4fv projMat;
        gl::UniformMatrix4fv viewMat;
        gl::UniformMatrix4fv modelMat;
        gl::Uniform1i texture1_sampler;
        gl::Texture_2d texture1;
        gl::Uniform1i texture2_sampler;
        gl::Texture_2d texture2;
        gl::Uniform1f mix_amt;

        gl::Vertex_array vao;
        GLsizei vao_num_triangles;
        gl::Array_buffer vertBuffer;
    };

    GLState initialize() {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        auto program = gl::Program{};
        auto vertex_shader = gl::Vertex_shader::Compile(vertex_shader_src);
        gl::AttachShader(program, vertex_shader);
        auto frag_shader = gl::Fragment_shader::Compile(frag_shader_src);
        gl::AttachShader(program, frag_shader);

        gl::LinkProgram(program);

        auto projMat = gl::UniformMatrix4fv{program, "projMat"};
        auto viewMat = gl::UniformMatrix4fv{program, "viewMat"};
        auto modelMat = gl::UniformMatrix4fv{program, "modelMat"};
        auto texture1_sampler = gl::Uniform1i{program, "texture1"};
        auto texture2_sampler = gl::Uniform1i{program, "texture2"};
        auto mix_amt = gl::Uniform1f{program, "mix_amt"};

        auto in_position = gl::Attribute{program, "in_position"};
        auto in_uv = gl::Attribute{program, "in_uv"};

        auto vbo = gl::Array_buffer{};

        struct Vec2 { float x, y; };
        struct Vec3 { float x, y, z; };
        struct Vert { Vec3 pos; Vec2 tex; };

        auto cubeVerts = std::array<Vert, 36>{{
               {{-0.5f, -0.5f, -0.5f},  {0.0f, 0.0f}},
               {{ 0.5f, -0.5f, -0.5f},  {1.0f, 0.0f}},
               {{ 0.5f,  0.5f, -0.5f},  {1.0f, 1.0f}},
               {{ 0.5f,  0.5f, -0.5f},  {1.0f, 1.0f}},
               {{-0.5f,  0.5f, -0.5f},  {0.0f, 1.0f}},
               {{-0.5f, -0.5f, -0.5f},  {0.0f, 0.0f}},

               {{-0.5f, -0.5f,  0.5f},  {0.0f, 0.0f}},
               {{ 0.5f, -0.5f,  0.5f},  {1.0f, 0.0f}},
               {{ 0.5f,  0.5f,  0.5f},  {1.0f, 1.0f}},
               {{ 0.5f,  0.5f,  0.5f},  {1.0f, 1.0f}},
               {{-0.5f,  0.5f,  0.5f},  {0.0f, 1.0f}},
               {{-0.5f, -0.5f,  0.5f},  {0.0f, 0.0f}},

               {{-0.5f,  0.5f,  0.5f},  {1.0f, 0.0f}},
               {{-0.5f,  0.5f, -0.5f},  {1.0f, 1.0f}},
               {{-0.5f, -0.5f, -0.5f},  {0.0f, 1.0f}},
               {{-0.5f, -0.5f, -0.5f},  {0.0f, 1.0f}},
               {{-0.5f, -0.5f,  0.5f},  {0.0f, 0.0f}},
               {{-0.5f,  0.5f,  0.5f},  {1.0f, 0.0f}},

               {{ 0.5f,  0.5f,  0.5f},  {1.0f, 0.0f}},
               {{ 0.5f,  0.5f, -0.5f},  {1.0f, 1.0f}},
               {{ 0.5f, -0.5f, -0.5f},  {0.0f, 1.0f}},
               {{ 0.5f, -0.5f, -0.5f},  {0.0f, 1.0f}},
               {{ 0.5f, -0.5f,  0.5f},  {0.0f, 0.0f}},
               {{ 0.5f,  0.5f,  0.5f},  {1.0f, 0.0f}},

               {{-0.5f, -0.5f, -0.5f},  {0.0f, 1.0f}},
               {{ 0.5f, -0.5f, -0.5f},  {1.0f, 1.0f}},
               {{ 0.5f, -0.5f,  0.5f},  {1.0f, 0.0f}},
               {{ 0.5f, -0.5f,  0.5f},  {1.0f, 0.0f}},
               {{-0.5f, -0.5f,  0.5f},  {0.0f, 0.0f}},
               {{-0.5f, -0.5f, -0.5f},  {0.0f, 1.0f}},

               {{-0.5f,  0.5f, -0.5f},  {0.0f, 1.0f}},
               {{ 0.5f,  0.5f, -0.5f},  {1.0f, 1.0f}},
               {{ 0.5f,  0.5f,  0.5f},  {1.0f, 0.0f}},
               {{ 0.5f,  0.5f,  0.5f},  {1.0f, 0.0f}},
               {{-0.5f,  0.5f,  0.5f},  {0.0f, 0.0f}},
               {{-0.5f,  0.5f, -0.5f},  {0.0f, 1.0f}},
        }};

        // don't use an EBO because it makes UV mapping a PITA

        // set attributes to read the cube verts correctly
        auto vao = gl::Vertex_array{};
        gl::BindVertexArray(vao);
        {
            gl::BindBuffer(vbo);
            gl::BufferData(vbo, sizeof(Vert) * cubeVerts.size(), cubeVerts.data(), GL_STATIC_DRAW);

            gl::VertexAttributePointer(in_position, 3, GL_FLOAT, GL_FALSE, sizeof(Vert), 0);
            gl::EnableVertexAttribArray(in_position);

            gl::VertexAttributePointer(in_uv, 2, GL_FLOAT, GL_FALSE, sizeof(Vert), (GLvoid*)sizeof(Vec3));
            gl::EnableVertexAttribArray(in_uv);
        }
        gl::BindVertexArray();

        // box texture
        auto texture1 = gl::Texture_2d{};
        {
            auto img = stbi::Image{"../container.jpg"};
            gl::BindTexture(texture1);
            stbigl::TexImage2D(texture1, 0, img);
            gl::GenerateMipMap(texture1);
        }

        auto texture2 = gl::Texture_2d{};
        {
            auto img = stbi::Image{"../awesomeface.png"};
            gl::BindTexture(texture2);
            stbigl::TexImage2D(texture2, 0, img);
            gl::GenerateMipMap(texture2);
        }

        return GLState {
            .program = std::move(program),

            .projMat = std::move(projMat),
            .viewMat = std::move(viewMat),
            .modelMat = std::move(modelMat),
            .texture1_sampler = std::move(texture1_sampler),
            .texture1 = std::move(texture1),
            .texture2_sampler = std::move(texture2_sampler),
            .texture2 = std::move(texture2),
            .mix_amt = std::move(mix_amt),

            .vao = std::move(vao),
            .vao_num_triangles = cubeVerts.size(),
            .vertBuffer = std::move(vbo),
        };
    }

    struct ScreenDims {
        int w = 0;
        int h = 0;

        ScreenDims(std::pair<int, int> p) :
            w{p.first}, h{p.second} {
        }
    };

    void show(ui::State& s) {
        GLState gls = initialize();
        auto font = sdl::ttf::Font{"../FantasqueSansMono-Regular.ttf", 16};
        auto font_color = SDL_Color{ .r = 0x00, .g = 0x00, .b = 0x00, .a = 0xff };

        glEnable(GL_DEPTH_TEST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // when shrinking textures
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  // when magnifying textures

        bool wireframe_mode = false;
        ScreenDims window_dims = sdl::window_size(s.window);

        // camera: at a fixed position pointing at a fixed origin. The "camera" works by translating +
        // rotating all objects around that origin. Rotation is expressed as polar coordinates. Camera
        // panning is represented as a translation vector.
        float radius = 10.0f;
        float wheel_sensitivity = 1.0f;

        float fov = 120.0f;

        bool dragging = false;
        float theta = 0.0f;
        float phi = 0.0f;
        float sensitivity = 1.0f;

        bool panning = false;
        glm::vec3 pan = {0.0f, 0.0f, 0.0f};

        // top-level loop just constantly renders the state (above)
        while (true) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // render info messages
            {
                auto ss = std::stringstream{};
                ss << "radius = " << radius << std::endl;
                sdl::Surface surf = sdl::ttf::RenderText_Blended_Wrapped(font, ss.str().c_str(), font_color, 1000);
                sdl::Texture tex = sdl::CreateTextureFromSurface(s.renderer, surf);
                auto text_pos = SDL_Rect{.x = 16, .y = 16, .w = surf->w, .h = surf->h};
                SDL_RenderCopy(s.renderer, tex, nullptr, &text_pos);
            }

            glPolygonMode(GL_FRONT_AND_BACK, wireframe_mode ? GL_LINE : GL_FILL);

            glEnableClientState(GL_VERTEX_ARRAY);
            gl::UseProgram(gls.program);

            // set *invariant* uniforms
            auto rot_theta = glm::rotate(glm::identity<glm::mat4>(), -theta, glm::vec3{0.0f, 1.0f, 0.0f});
            auto theta_vec = glm::normalize(glm::vec3{sin(theta), 0.0f, cos(theta)});
            auto phi_axis = glm::cross(theta_vec, glm::vec3{0.0, 1.0f, 0.0f});
            auto rot_phi = glm::rotate(glm::identity<glm::mat4>(), -phi, phi_axis);
            auto pan_translate = glm::translate(glm::identity<glm::mat4>(), pan);
            {
                // projection viewport
                float aspect_ratio = static_cast<float>(window_dims.w)/static_cast<float>(window_dims.h);
                glglm::Uniform(gls.projMat, glm::perspective(fov, aspect_ratio, 0.1f, 100.0f));

                glm::mat4 view_matrix =
                        glm::lookAt(glm::vec3(0.0f, 0.0f, radius), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3{0.0f, 1.0f, 0.0f}) * rot_theta * rot_phi * pan_translate;
                glglm::Uniform(gls.viewMat, view_matrix);

                gl::Uniform(gls.texture1_sampler, 0);
                gl::Uniform(gls.texture2_sampler, 1);
                gl::Uniform(gls.mix_amt, 0.5);
            }

            glActiveTexture(GL_TEXTURE0);
            gl::BindTexture(gls.texture1);

            glActiveTexture(GL_TEXTURE1);
            gl::BindTexture(gls.texture2);

            // for SDL
            glActiveTexture(GL_TEXTURE0);

            // draw models (instanced from a single cube)
            gl::BindVertexArray(gls.vao);
            {
                glm::vec3 cubePositions[] = {
                    glm::vec3( 0.0f,  0.0f,  0.0f),
                    glm::vec3( 2.0f,  5.0f, -15.0f),
                    glm::vec3(-1.5f, -2.2f, -2.5f),
                    glm::vec3(-3.8f, -2.0f, -12.3f),
                    glm::vec3( 2.4f, -0.4f, -3.5f),
                    glm::vec3(-1.7f,  3.0f, -7.5f),
                    glm::vec3( 1.3f, -2.0f, -2.5f),
                    glm::vec3( 1.5f,  2.0f, -2.5f),
                    glm::vec3( 1.5f,  0.2f, -1.5f),
                    glm::vec3(-1.3f,  1.0f, -1.5f)
                };
                for (auto const& pos : cubePositions) {
                    auto translate = glm::translate(glm::identity<glm::mat4>(), pos);
                    glglm::Uniform(gls.modelMat, translate);
                    glDrawArrays(GL_TRIANGLES, 0, 36);
                }

            }
            gl::BindVertexArray();
            gl::BindTexture();

            gl::UseProgram();
            glDisableClientState(GL_VERTEX_ARRAY);

            // draw
            SDL_GL_SwapWindow(s.window);

            // event loop
            SDL_Event e;
            while (SDL_PollEvent(&e)) {
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

                        // this assumes the scene is not rotated, so we need to rotate these
                        // axes to match the scene's rotation
                        // TODO: cleanup
                        glm::vec4 default_panning_axis = {dx * 2.0f * M_PI, dy * 2.0f * M_PI, 0.0f, 1.0f};
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
                    if (e.wheel.y > 0 and (radius - 1.0f) > 1.0f) {
                        radius -= wheel_sensitivity;
                    }

                    if (e.wheel.y <= 0 and radius < 100.0f) {
                        radius += wheel_sensitivity;
                    }
                }
            }
        }
    }
}

namespace examples::geom {
    static const char vertex_shader_src[] = R"(
        #version 430

        uniform mat4 projMat;
        uniform mat4 viewMat;
        uniform mat4 modelMat;

        in vec3 location;

        void main() {
            gl_Position = projMat * viewMat * modelMat * vec4(location, 1.0);
        }
    )";

    static const char frag_shader_src[] = R"(
        #version 430

        uniform vec4 rgba;

        out vec4 color;

        void main() {
            color = rgba;
        }
    )";

    struct Vec3 {
        GLfloat x;
        GLfloat y;
        GLfloat z;
    };

    struct Mesh_point {
        Vec3 position;
        Vec3 normal;
    };

    // dumb alg. for producing a unit (radius = 1.0, height = 1.0) cylinder
    //
    // TODO: this is dumb because a cylinder can be EBO-ed quite easily, which
    //       would reduce the amount of vertices needed
    std::vector<Mesh_point> unit_cylinder_mesh(unsigned num_sides) {
        if (num_sides < 3) {
            throw std::runtime_error{"cannot create a cylinder with fewer than 3 sides"};
        }

        std::vector<Mesh_point> rv;
        rv.reserve(2*3*num_sides + 6*num_sides);

        float step_angle = (2*M_PI)/num_sides;
        float top_z = -0.5f;
        float bottom_z = +0.5f;

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

    struct Triangle_mesh {
        unsigned num_verts;
        gl::Array_buffer vbo;
        gl::Vertex_array vao;

        Triangle_mesh(gl::Attribute& in_attr, std::vector<Mesh_point> const& points) :
            num_verts(points.size()) {

            gl::BindVertexArray(vao);
            {
                gl::BindBuffer(vbo);
                gl::BufferData(vbo, sizeof(Mesh_point) * points.size(), points.data(), GL_STATIC_DRAW);
                gl::VertexAttributePointer(in_attr, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh_point), 0);
                gl::EnableVertexAttribArray(in_attr);
            }
            gl::BindVertexArray();
        }
    };

    Triangle_mesh make_cylinder(gl::Attribute& in_attr, unsigned num_sides) {
        auto points = unit_cylinder_mesh(num_sides);
        return Triangle_mesh{in_attr, points};
    }

    struct GLState {
        gl::Program program;

        gl::UniformMatrix4fv projMat;
        gl::UniformMatrix4fv viewMat;
        gl::UniformMatrix4fv modelMat;
        gl::UniformVec4f rgba;

        gl::Attribute location;

        Triangle_mesh cylinder;
    };

    GLState initialize() {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

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

        auto in_position = gl::Attribute{program, "location"};

        auto cylinder = make_cylinder(in_position, 24);

        return GLState {
            .program = std::move(program),

            .projMat = std::move(projMat),
            .viewMat = std::move(viewMat),
            .modelMat = std::move(modelMat),
            .rgba = std::move(rgba),

            .location = std::move(in_position),

            .cylinder = std::move(cylinder),
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

    struct ModelState {
        std::vector<osim::Cylinder> cylinders;
        std::vector<Line> lines;
        std::vector<osim::Sphere> spheres;
    };

    ModelState load_model(GLState& gls, char const* path) {
        ModelState rv;
        for (osim::Geometry const& g : osim::geometry_in(path)) {
            std::visit(overloaded {
                [&](osim::Cylinder const& c) {
                    rv.cylinders.push_back(c);
                },
                [&](osim::Line const& l) {
                    rv.lines.push_back(Line{ gls.location, l });
                },
                [&](osim::Sphere const& s) {
                    rv.spheres.push_back(s);
                },
                [&](osim::Mesh const& m) {
                    std::cerr << m << std::endl;
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

    void show(ui::State& s) {
        static char const* files[] = {
            "/home/adam/Desktop/osim-snippets/opensim-models/Models/Arm26/arm26.osim",
            "/home/adam/Desktop/osim-snippets/opensim-models/Models/BouncingBlock/bouncing_block.osim"
        };

        GLState gls = initialize();
        auto font = sdl::ttf::Font{"../FantasqueSansMono-Regular.ttf", 16};
        auto font_color = SDL_Color{ .r = 0x00, .g = 0x00, .b = 0x00, .a = 0xff };
        ModelState ms = load_model(gls, files[0]);

        glEnable(GL_DEPTH_TEST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // when shrinking textures
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  // when magnifying textures
        glPointSize(4.0f);

        bool wireframe_mode = false;
        ScreenDims window_dims = sdl::window_size(s.window);

        // camera: at a fixed position pointing at a fixed origin. The "camera" works by translating +
        // rotating all objects around that origin. Rotation is expressed as polar coordinates. Camera
        // panning is represented as a translation vector.
        float radius = 1.0f;
        float wheel_sensitivity = 0.9f;

        float fov = 120.0f;

        bool dragging = false;
        float theta = 0.0f;
        float phi = 0.0f;
        float sensitivity = 1.0f;

        bool panning = false;
        glm::vec3 pan = {0.0f, 0.0f, 0.0f};

        // top-level loop just constantly renders the state (above)
        while (true) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // render info messages
            {
                glActiveTexture(GL_TEXTURE0);  // important: used for rasterization
                auto ss = std::stringstream{};
                ss << "radius = " << radius << std::endl;
                sdl::Surface surf = sdl::ttf::RenderText_Blended_Wrapped(font, ss.str().c_str(), font_color, 1000);
                sdl::Texture tex = sdl::CreateTextureFromSurface(s.renderer, surf);
                auto text_pos = SDL_Rect{.x = 16, .y = 16, .w = surf->w, .h = surf->h};
                SDL_RenderCopy(s.renderer, tex, nullptr, &text_pos);
            }

            glPolygonMode(GL_FRONT_AND_BACK, wireframe_mode ? GL_LINE : GL_FILL);

            glEnableClientState(GL_VERTEX_ARRAY);
            gl::UseProgram(gls.program);

            // set *invariant* uniforms
            auto rot_theta = glm::rotate(glm::identity<glm::mat4>(), -theta, glm::vec3{0.0f, 1.0f, 0.0f});
            auto theta_vec = glm::normalize(glm::vec3{sin(theta), 0.0f, cos(theta)});
            auto phi_axis = glm::cross(theta_vec, glm::vec3{0.0, 1.0f, 0.0f});
            auto rot_phi = glm::rotate(glm::identity<glm::mat4>(), -phi, phi_axis);
            auto pan_translate = glm::translate(glm::identity<glm::mat4>(), pan);
            {
                // projection viewport
                float aspect_ratio = static_cast<float>(window_dims.w)/static_cast<float>(window_dims.h);
                glglm::Uniform(gls.projMat, glm::perspective(fov, aspect_ratio, 0.1f, 100.0f));

                glm::mat4 view_matrix =
                        glm::lookAt(glm::vec3(0.0f, 0.0f, radius), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3{0.0f, 1.0f, 0.0f}) * rot_theta * rot_phi * pan_translate;
                glglm::Uniform(gls.viewMat, view_matrix);
            }

            for (auto const& c : ms.cylinders) {
                gl::BindVertexArray(gls.cylinder.vao);
                glglm::Uniform(gls.rgba, c.rgba);
                auto hacky_cylinder_correction =
                        glm::rotate(glm::identity<glm::mat4>(), static_cast<float>(M_PI/2.0f), glm::vec3{-1.0f, 0.0f, 0.0f});
                glglm::Uniform(gls.modelMat, glm::scale(c.transform * hacky_cylinder_correction, c.scale));
                glDrawArrays(GL_TRIANGLES, 0, gls.cylinder.num_verts);
                gl::BindVertexArray();
            }

            for (auto const& c : ms.spheres) {
                gl::BindVertexArray(gls.cylinder.vao);
                glglm::Uniform(gls.rgba, c.rgba);
                auto hacky_cylinder_correction =
                        glm::rotate(glm::identity<glm::mat4>(), static_cast<float>(M_PI/2.0f), glm::vec3{-1.0f, 0.0f, 0.0f});
                auto scaler = glm::scale(c.transform * hacky_cylinder_correction, glm::vec3{c.radius, c.radius, c.radius});
                glglm::Uniform(gls.modelMat, scaler);
                glDrawArrays(GL_TRIANGLES, 0, gls.cylinder.num_verts);
                gl::BindVertexArray();
            }

            for (auto& l : ms.lines) {
                glLineWidth(5.0f);
                gl::BindVertexArray(l.vao);
                glglm::Uniform(gls.rgba, l.data.rgba);
                glglm::Uniform(gls.modelMat, glm::identity<glm::mat4>());
                glDrawArrays(GL_LINES, 0, 2);
                gl::BindVertexArray();
            }

            gl::UseProgram();
            glDisableClientState(GL_VERTEX_ARRAY);

            // draw
            SDL_GL_SwapWindow(s.window);

            // event loop
            SDL_Event e;
            while (SDL_PollEvent(&e)) {
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

                        // this assumes the scene is not rotated, so we need to rotate these
                        // axes to match the scene's rotation
                        // TODO: cleanup
                        glm::vec4 default_panning_axis = {dx * 2.0f * M_PI, dy * 2.0f * M_PI, 0.0f, 1.0f};
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

int main() {
    auto ui = ui::State{};
    //examples::fractal::show(s);
    //examples::cube::show(s);
    examples::geom::show(ui);
    return 0;
};
