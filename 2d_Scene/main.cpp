/**
* Author: Franklin
* Assignment: Simple 2D Scene
* Date due: 2024-06-15, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>

int counter = 0;
int counter_limit = 50000;

enum AppStatus { RUNNING, TERMINATED };
GLuint load_texture(const char* filepath);

struct Sprite {
    const char* sprite_file_path;
    glm::vec3 initial_scale = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 initial_position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 movement = glm::vec3(0.0f, 0.0f, 0.0f);
    GLuint texture_id;
    glm::mat4 model_matrix = glm::mat4(1.0f);
    Sprite(const char* sprite_file_path, 
        glm::vec3 initial_scale, 
        glm::vec3 initial_position)
        : initial_scale(initial_scale), 
        initial_position(initial_position){
        this->sprite_file_path = sprite_file_path;
        texture_id = load_texture(sprite_file_path);
    }
};

constexpr float WINDOW_SIZE_MULT = 2.0f;

constexpr int WINDOW_WIDTH = static_cast<int>(640 * WINDOW_SIZE_MULT),
WINDOW_HEIGHT = static_cast<int>(480 * WINDOW_SIZE_MULT);

constexpr float BG_RED = 0.9765625f,
BG_GREEN = 0.97265625f,
BG_BLUE = 0.9609375f,
BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;


constexpr char KANO_SPRITE_FILEPATH[] = "feet.jpg",
MAHIRU_SPRITE_FILEPATH[] = "face.jpg";

constexpr float MINIMUM_COLLISION_DISTANCE = 1.0f;
constexpr glm::vec3 INIT_SCALE = glm::vec3(1.0f * 3, 856.0f / 1093.0f * 3, 0.0f),
INIT_POS_KANO = glm::vec3(2.0f, 0.0f, 0.0f),
INIT_POS_MAHIRU = glm::vec3(-2.0f, 0.0f, 0.0f);

SDL_Window* g_display_window;

AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program = ShaderProgram();
glm::mat4 g_view_matrix, g_kano_matrix, g_projection_matrix, g_trans_matrix, g_mahiru_matrix;

float g_previous_ticks = 0.0f;

GLuint g_kano_texture_id;
GLuint g_mahiro_texture_id;
// constructor

glm::vec3 g_kano_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_kano_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 g_mahiru_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_mahiru_movement = glm::vec3(0.0f, 0.0f, 0.0f);
//constructor

float g_kano_speed = 1.0f;  // move 1 unit per second

void initialise();
void process_input();
void update();
void render();
void shutdown();

constexpr GLint NUMBER_OF_TEXTURES = 1;  // to be generated, that is
constexpr GLint LEVEL_OF_DETAIL = 0;  // base image level; Level n is the nth mipmap reduction image
constexpr GLint TEXTURE_BORDER = 0;  // this value MUST be zero

GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);

    return textureID;
}

char weasel_face_file_path[] = "face.jpg";
Sprite weasel_face(weasel_face_file_path,
    glm::vec3(1.0f * 2, 856.0f / 1093.0f * 2, 0.0f),
    glm::vec3(0.0f, 0.0f, 0.0f));
// file path
// initial_scale
// initial_position

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("CENSORED",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);


    if (g_display_window == nullptr)
    {
        shutdown();
    }
#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_kano_matrix = glm::mat4(1.0f);
    g_mahiru_matrix = glm::mat4(1.0f);
    g_mahiru_matrix = glm::translate(g_mahiru_matrix, glm::vec3(1.0f, 1.0f, 0.0f));
    g_mahiru_position += g_mahiru_movement;

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    g_kano_texture_id = load_texture(KANO_SPRITE_FILEPATH);
    g_mahiro_texture_id = load_texture(MAHIRU_SPRITE_FILEPATH);

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    g_kano_movement = glm::vec3(0.0f);

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            // End game
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_app_status = TERMINATED;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
            {
            case SDLK_LEFT:
                // Move the player left
                break;

            case SDLK_RIGHT:
                // Move the player right
                g_kano_movement.x = 1.0f;
                break;

            case SDLK_q:
                // Quit the game with a keystroke
                g_app_status = TERMINATED;
                break;

            default:
                break;
            }

        default:
            break;
        }
    }


    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT])
    {
        g_kano_movement.x = -1.0f;
    }
    else if (key_state[SDL_SCANCODE_RIGHT])
    {
        g_kano_movement.x = 1.0f;
    }

    if (key_state[SDL_SCANCODE_UP])
    {
        g_kano_movement.y = 1.0f;
    }
    else if (key_state[SDL_SCANCODE_DOWN])
    {
        g_kano_movement.y = -1.0f;
    }

    // This makes sure that the player can't "cheat" their way into moving
    // faster
    if (glm::length(g_kano_movement) > 1.0f)
    {
        g_kano_movement = glm::normalize(g_kano_movement);
    }
}
float rot_angle = 0;
float size = 0;
void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - g_previous_ticks; // the delta time is the difference from the last frame
    g_previous_ticks = ticks;

    g_kano_position += g_kano_movement * g_kano_speed * delta_time;
    counter++;
    if (counter > counter_limit && true) {
        g_kano_matrix = glm::mat4(1.0f);
        g_mahiru_matrix = glm::mat4(1.0f);
        g_kano_matrix = glm::translate(g_kano_matrix, glm::vec3(-0.4f, 0.65f, 0.0f));
        g_mahiru_matrix = glm::translate(g_kano_matrix, glm::vec3(0.0f, 856.0f / 1093.0f * 3 - 0.45, 0.0f));
        //g_mahiru_matrix = glm::translate(g_kano_matrix, g_kano_position);
        g_kano_matrix = glm::scale(g_kano_matrix, INIT_SCALE);
        g_mahiru_matrix = glm::scale(g_mahiru_matrix, INIT_SCALE);
    }
    else {
        // Add direction * units per second * elapsed time

        g_kano_matrix = glm::mat4(1.0f);
    
        rot_angle++;
        size++;
        g_kano_matrix = glm::translate(g_kano_matrix, glm::vec3((INIT_POS_MAHIRU.x + 200 * cos(rot_angle)) * delta_time, (INIT_POS_MAHIRU.y + 200 * sin(rot_angle)) * delta_time, 0.0f));
        glm::vec3 scale = glm::vec3((INIT_POS_MAHIRU.x + 2000*sin(size/100000)) * delta_time, (INIT_POS_MAHIRU.y + 2000*sin(size/100000)) * delta_time, INIT_POS_MAHIRU.z);
        g_kano_matrix = glm::translate(g_kano_matrix, g_kano_position);
        g_kano_matrix = glm::rotate(g_kano_matrix, glm::radians(rot_angle / 10.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        g_kano_matrix = glm::translate(g_kano_matrix, glm::vec3(5 * delta_time, 0.0f, 0.0f)); // this ordering is intentional

        g_mahiru_matrix = glm::mat4(1.0f);
        g_mahiru_matrix = glm::rotate(g_kano_matrix, glm::radians(rot_angle/10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec3 scale1 = glm::vec3((scale.x + 1000) * delta_time, (scale.y + 1000) * delta_time, scale.z);
        g_mahiru_matrix = glm::translate(g_mahiru_matrix, scale1);


        g_kano_matrix = glm::scale(g_kano_matrix, scale);
        g_mahiru_matrix = glm::scale(g_mahiru_matrix, scale);

        weasel_face.model_matrix = glm::scale(weasel_face.model_matrix, weasel_face.initial_scale);

    }
    /*
    float x_distance = fabs(g_kano_position.x + INIT_POS_KANO.x - INIT_POS_MAHIRU.x) -
        ((INIT_SCALE.x + INIT_SCALE.x) / 2.0f);

    float y_distance = fabs(g_kano_position.y + INIT_POS_KANO.y - INIT_POS_MAHIRU.y) -
        ((INIT_SCALE.y + INIT_SCALE.y) / 2.0f);

    if (x_distance < 0.0f && y_distance < 0.0f)
    {
        std::cout << std::time(nullptr) << ": Collision.\n";
    }
    */
}

void draw_object(glm::mat4& object_model_matrix, GLuint& object_texture_id)
{
    g_shader_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Vertices
    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    // Textures
    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    // Bind texture
    draw_object(g_kano_matrix, g_kano_texture_id);
    draw_object(g_mahiru_matrix, g_mahiro_texture_id);
    draw_object(weasel_face.model_matrix, weasel_face.texture_id);

    // We disable two attribute arrays now
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() { SDL_Quit(); }


int main(int argc, char* argv[])
{
    initialise();

    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}