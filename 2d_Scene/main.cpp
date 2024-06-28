﻿#define LOG(argument) std::cout << argument << '\n'
#define STB_IMAGE_IMPLEMENTATION
#define GL_SILENCE_DEPRECATION

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
#include <vector>
#include <ctime>
#include "cmath"
#include <cstdlib>

enum AppStatus { RUNNING, TERMINATED };

constexpr int WINDOW_WIDTH = 640 * 2,
WINDOW_HEIGHT = 480 * 2;

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

constexpr char SPRITESHEET_FILEPATH[] = "george_0.png",
FONTSHEET_FILEPATH[] = "font1.png";

constexpr GLint NUMBER_OF_TEXTURES = 1,
LEVEL_OF_DETAIL = 0,
TEXTURE_BORDER = 0;

/* -------------------------- NEW STUFF BELOW -------------------------- */
constexpr int SPRITESHEET_DIMENSIONS = 4;
constexpr int SECONDS_PER_FRAME = 4;
constexpr int LEFT = 0,
RIGHT = 1,
UP = 2,
DOWN = 3;

constexpr int FONTBANK_SIZE = 16;

int g_george_walking[SPRITESHEET_DIMENSIONS][SPRITESHEET_DIMENSIONS] =
{
    { 1, 5, 9,  13 }, // for George to move to the left,
    { 3, 7, 11, 15 }, // for George to move to the right,
    { 2, 6, 10, 14 }, // for George to move upwards,
    { 0, 4, 8,  12 }  // for George to move downwards
};

GLuint g_george_texture_id;
GLuint g_font_texture_id;

float g_ball_speed = 2.0f;  // move 1 unit per second

int* g_animation_indices = g_george_walking[RIGHT];
int g_animation_frames = SPRITESHEET_DIMENSIONS;
int g_animation_index = 0;

float g_animation_time = 0.0f;

void draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index,
    int rows, int cols);
void draw_text(ShaderProgram* shader_program, GLuint font_texture_id, std::string text,
    float font_size, float spacing, glm::vec3 position);
/* -------------------------- NEW STUFF ABOVE -------------------------- */

SDL_Window* g_display_window = nullptr;
AppStatus g_app_status = RUNNING;

ShaderProgram g_shader_program = ShaderProgram();

glm::mat4 g_view_matrix,
g_george_matrix,
g_projection_matrix;

float previous_ticks = 0.0f;

glm::vec3 g_george_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_george_movement = glm::vec3(static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
    static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 0.5f, 0.0f);

GLuint g_kano_texture_id, g_mahiru_texture_id, g_face1_texture_id, g_face2_texture_id;
constexpr char KANO_SPRITE_FILEPATH[] = "kano.png";
constexpr char MAHIRU_SPRITE_FILEPATH[] = "mahiru.png";
glm::mat4 g_kano_matrix, g_mahiru_matrix, g_face1_matrix, g_face2_matrix;
constexpr glm::vec3 ANIME_SCALE = glm::vec3(2.0f / 2, 4.2104f / 2, 0.0f);
glm::vec3 g_kano_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_kano_movement = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_mahiru_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_mahiru_movement = glm::vec3(0.0f, 0.0f, 0.0f);
constexpr char WESLEY_FILE_PATH[] = "face.jpg";
glm::vec3 face_scale = glm::vec3(1.0f * 0.5, 856.0f / 1093.0f * 0.5, 0.0f);
glm::vec3 g_face1_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_face1_movement = glm::vec3(-static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
    static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 0.5f, 0.0f);
glm::vec3 g_face2_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_face2_movement = glm::vec3(2*(static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 0.5),
    static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 0.5f, 0.0f);

float anime_speed = 5.0f;
bool is_player_controlled = true;
bool game_finished = false;
int balls = 1;

void initialise();
void process_input();
void update();
void render();
void shutdown();

GLuint load_texture(const char* filepath);
void draw_object(glm::mat4& object_model_matrix, GLuint& object_texture_id);


void draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index,
    int rows, int cols)
{
    // Step 1: Calculate the UV location of the indexed frame
    float u_coord = (float)(index % cols) / (float)cols;
    float v_coord = (float)(index / cols) / (float)rows;

    // Step 2: Calculate its UV size
    float width = 1.0f / (float)cols;
    float height = 1.0f / (float)rows;

    // Step 3: Just as we have done before, match the texture coordinates to the vertices
    float tex_coords[] =
    {
        u_coord, v_coord + height, u_coord + width, v_coord + height, u_coord + width,
        v_coord, u_coord, v_coord + height, u_coord + width, v_coord, u_coord, v_coord
    };

    float vertices[] =
    {
        -0.5, -0.5, 0.5, -0.5,  0.5, 0.5,
        -0.5, -0.5, 0.5,  0.5, -0.5, 0.5
    };

    // Step 4: And render
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0,
        vertices);
    glEnableVertexAttribArray(program->get_position_attribute());

    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0,
        tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}


void draw_text(ShaderProgram* shader_program, GLuint font_texture_id, std::string text,
    float font_size, float spacing, glm::vec3 position)
{
    // Scale the size of the fontbank in the UV-plane
    // We will use this for spacing and positioning
    float width = 1.0f / FONTBANK_SIZE;
    float height = 1.0f / FONTBANK_SIZE;

    // Instead of having a single pair of arrays, we'll have a series of pairs�one for
    // each character. Don't forget to include <vector>!
    std::vector<float> vertices;
    std::vector<float> texture_coordinates;

    // For every character...
    for (int i = 0; i < text.size(); i++) {
        // 1. Get their index in the spritesheet, as well as their offset (i.e. their
        //    position relative to the whole sentence)
        int spritesheet_index = (int)text[i];  // ascii value of character
        float offset = (font_size + spacing) * i;

        // 2. Using the spritesheet index, we can calculate our U- and V-coordinates
        float u_coordinate = (float)(spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
        float v_coordinate = (float)(spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;

        // 3. Inset the current pair in both vectors
        vertices.insert(vertices.end(), {
            offset + (-0.5f * font_size), 0.5f * font_size,
            offset + (-0.5f * font_size), -0.5f * font_size,
            offset + (0.5f * font_size), 0.5f * font_size,
            offset + (0.5f * font_size), -0.5f * font_size,
            offset + (0.5f * font_size), 0.5f * font_size,
            offset + (-0.5f * font_size), -0.5f * font_size,
            });

        texture_coordinates.insert(texture_coordinates.end(), {
            u_coordinate, v_coordinate,
            u_coordinate, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate + width, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate, v_coordinate + height,
            });
    }

    // 4. And render all of them using the pairs
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);

    shader_program->set_model_matrix(model_matrix);
    glUseProgram(shader_program->get_program_id());

    glVertexAttribPointer(shader_program->get_position_attribute(), 2, GL_FLOAT, false, 0,
        vertices.data());
    glEnableVertexAttribArray(shader_program->get_position_attribute());

    glVertexAttribPointer(shader_program->get_tex_coordinate_attribute(), 2, GL_FLOAT,
        false, 0, texture_coordinates.data());
    glEnableVertexAttribArray(shader_program->get_tex_coordinate_attribute());

    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(shader_program->get_position_attribute());
    glDisableVertexAttribArray(shader_program->get_tex_coordinate_attribute());
}


GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components,
        STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER,
        GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(image);

    return textureID;
}


void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Hello, Spritesheets!",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

    if (g_display_window == nullptr)
    {
        std::cerr << "Error: SDL window could not be created.\n";
        shutdown();
    }

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_george_matrix = glm::mat4(1.0f);
    g_kano_matrix = glm::mat4(1.0f);
    g_mahiru_matrix = glm::mat4(1.0f);
    g_face1_matrix = glm::mat4(1.0f);
    g_face2_matrix = glm::mat4(1.0f);
    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    g_george_texture_id = load_texture(SPRITESHEET_FILEPATH);
    g_kano_texture_id = load_texture(KANO_SPRITE_FILEPATH);
    g_mahiru_texture_id = load_texture(MAHIRU_SPRITE_FILEPATH);
    g_font_texture_id = load_texture(FONTSHEET_FILEPATH);
    g_face1_texture_id = load_texture(WESLEY_FILE_PATH);
    g_face2_texture_id = load_texture(WESLEY_FILE_PATH);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void process_input()
{
    // g_george_movement = glm::vec3(0.0f);
    g_mahiru_movement = glm::vec3(0.0f);
    g_kano_movement = glm::vec3(0.0f);

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_app_status = TERMINATED;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_t:
                is_player_controlled = !is_player_controlled;
                break;
            case SDLK_1:
                balls = 1;
                break;
            case SDLK_2:
                balls = 2;
                break;
            case SDLK_3:
                balls = 3;
                break;
            default:
                break;
            }

        default:
            break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_W])
    {
        g_mahiru_movement.y = 1.0f;
        //g_george_movement.x = -1.0f;
        //g_animation_indices = g_george_walking[LEFT];
    }
    else if (key_state[SDL_SCANCODE_S])
    {
        g_mahiru_movement.y = -1.0f;
        //g_george_movement.x = 1.0f;
        //g_animation_indices = g_george_walking[RIGHT];
    }

    if (key_state[SDL_SCANCODE_UP])
    {
        g_kano_movement.y = 1.0f;
        //g_george_movement.y = 1.0f;
        //g_animation_indices = g_george_walking[UP];
    }
    else if (key_state[SDL_SCANCODE_DOWN])
    {
        g_kano_movement.y = -1.0f;
        //g_george_movement.y = -1.0f;
        //g_animation_indices = g_george_walking[DOWN];
    }
}

int mahiru_score = 0;
int kano_score = 0;
bool mahiru_turn_george = false;
bool mahiru_turn_face1 = false;
bool mahiru_turn_face2 = false;

void update()
{
    /* DELTA TIME */
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - previous_ticks;
    previous_ticks = ticks;

    /* ANIMATION */
    if (glm::length(g_george_movement) != 0)
    {
        g_animation_time += delta_time;
        float frames_per_second = (float)1 / SECONDS_PER_FRAME;

        if (g_animation_time >= frames_per_second)
        {
            g_animation_time = 0.0f;
            g_animation_index++;

            if (g_animation_index >= g_animation_frames) g_animation_index = 0;
        }
    }

    /* GAME LOGIC */
    int damping = 10;
    float top = 3.5f;
    g_kano_position.x = 4.5f;
    g_mahiru_position.x = -g_kano_position.x;

    if (((g_mahiru_position.y + ANIME_SCALE.y / 2) >= top && g_mahiru_movement.y > 0)
        || ((g_mahiru_position.y - ANIME_SCALE.y / 2) <= -top && g_mahiru_movement.y < 0)) {
        g_mahiru_movement.y = 0;
    }
    if (!is_player_controlled) {
        g_kano_movement.y = sin(ticks);
    }
    if ((g_kano_position.y + ANIME_SCALE.y / 2 > top && g_kano_movement.y > 0)
        || (g_kano_position.y - ANIME_SCALE.y / 2 < -top && g_kano_movement.y < 0)) {
        g_kano_movement.y = 0;
    }
    if ((g_george_position.y > top && g_george_movement.y > 0)
        || (g_george_position.y < -top && g_george_movement.y < 0)) {
        g_george_movement.y = -g_george_movement.y;
    }
    if (balls > 1 && (g_face1_position.y > top && g_face1_movement.y > 0)
        || (g_face1_position.y < -top && g_face1_movement.y < 0)) {
        g_face1_movement.y = -g_face1_movement.y;
    }
    if (balls > 2 && (g_face2_position.y > top && g_face2_movement.y > 0)
        || (g_face2_position.y < -top && g_face2_movement.y < 0)) {
        g_face2_movement.y = -g_face2_movement.y;
    }
    if (!game_finished && g_george_position.x < g_kano_position.x && g_george_position.x > g_mahiru_position.x) {
        if (g_george_position.x > g_kano_position.x - ANIME_SCALE.x / 2 &&
            g_george_position.y >= g_kano_position.y - ANIME_SCALE.y / 2 &&
            g_george_position.y <= g_kano_position.y + ANIME_SCALE.y / 2 &&
            g_george_movement.x > 0) {
            g_george_movement.x = -g_george_movement.x + sin(rand()) / damping / damping;
            g_george_movement.y += g_kano_movement.y / damping + sin(rand()) / damping / damping;
            if (!mahiru_turn_george) {
                kano_score++;
                mahiru_turn_george = true;
            }
        }
        else if (g_george_position.x < g_mahiru_position.x + ANIME_SCALE.x / 2 &&
            g_george_position.y >= g_mahiru_position.y - ANIME_SCALE.y / 2 &&
            g_george_position.y <= g_mahiru_position.y + ANIME_SCALE.y / 2 &&
            g_george_movement.x < 0) {
            g_george_movement.x = -g_george_movement.x + sin(rand()) / damping / damping;
            g_george_movement.y += g_mahiru_movement.y / damping + sin(rand()) / damping / damping;
            if (mahiru_turn_george) {
                mahiru_score++;
                mahiru_turn_george = false;
            }
        }
        if (g_george_movement.x > 0 && g_george_movement.x < 1) {
            g_george_movement.x = 1;
        }
        else if (g_george_movement.x < 0 && g_george_movement.x > -1) {
            g_george_movement.x = -1;
        }
    }
    else {
        game_finished = true;
        return;
    }
    if (balls > 1) {
        if (!game_finished && g_face1_position.x < g_kano_position.x && g_face1_position.x > g_mahiru_position.x) {
            if (g_face1_position.x > g_kano_position.x - ANIME_SCALE.x / 2 &&
                g_face1_position.y >= g_kano_position.y - ANIME_SCALE.y / 2 &&
                g_face1_position.y <= g_kano_position.y + ANIME_SCALE.y / 2 &&
                g_face1_movement.x > 0) {
                g_face1_movement.x = -g_face1_movement.x + sin(rand()) / damping / damping;
                g_face1_movement.y += g_kano_movement.y / damping + sin(rand()) / damping / damping;
                if (!mahiru_turn_face1) {
                    kano_score++;
                    mahiru_turn_face1 = true;
                }
            }
            else if (g_face1_position.x < g_mahiru_position.x + ANIME_SCALE.x / 2 &&
                g_face1_position.y >= g_mahiru_position.y - ANIME_SCALE.y / 2 &&
                g_face1_position.y <= g_mahiru_position.y + ANIME_SCALE.y / 2 &&
                g_face1_movement.x < 0) {
                g_face1_movement.x = -g_face1_movement.x + sin(rand()) / damping / damping;
                g_face1_movement.y += g_mahiru_movement.y / damping + sin(rand()) / damping / damping;
                if (mahiru_turn_face1) {
                    mahiru_score++;
                    mahiru_turn_face1 = false;
                }
            }
            if (g_face1_movement.x > 0 && g_face1_movement.x < 1) {
                g_face1_movement.x = 1;
            }
            else if (g_face1_movement.x < 0 && g_face1_movement.x > -1) {
                g_face1_movement.x = -1;
            }
        }
        else {
            game_finished = true;
            return;
        }
    }
    if (balls > 2) {
        if (!game_finished && g_face2_position.x < g_kano_position.x && g_face2_position.x > g_mahiru_position.x) {
            if (g_face2_position.x > g_kano_position.x - ANIME_SCALE.x / 2 &&
                g_face2_position.y >= g_kano_position.y - ANIME_SCALE.y / 2 &&
                g_face2_position.y <= g_kano_position.y + ANIME_SCALE.y / 2 &&
                g_face2_movement.x > 0) {
                g_face2_movement.x = -g_face2_movement.x + sin(rand()) / damping / damping;
                g_face2_movement.y += g_kano_movement.y / damping + sin(rand()) / damping / damping;
                if (!mahiru_turn_face2) {
                    kano_score++;
                    mahiru_turn_face2 = true;
                }
            }
            else if (g_face2_position.x < g_mahiru_position.x + ANIME_SCALE.x / 2 &&
                g_face2_position.y >= g_mahiru_position.y - ANIME_SCALE.y / 2 &&
                g_face2_position.y <= g_mahiru_position.y + ANIME_SCALE.y / 2 &&
                g_face2_movement.x < 0) {
                g_face2_movement.x = -g_face2_movement.x + sin(rand()) / damping / damping;
                g_face2_movement.y += g_mahiru_movement.y / damping + sin(rand()) / damping / damping;
                if (mahiru_turn_face2) {
                    mahiru_score++;
                    mahiru_turn_face2 = false;
                }
            }
            if (g_face2_movement.x > 0 && g_face2_movement.x < 1) {
                g_face2_movement.x = 1;
            }
            else if (g_face2_movement.x < 0 && g_face2_movement.x > -1) {
                g_face2_movement.x = -1;
            }
        }
        else {
            game_finished = true;
            return;
        }
    }

    if (g_george_movement.x > 0) {
        g_animation_indices = g_george_walking[RIGHT];
    }
    else if (g_george_movement.x < 0) {
        g_animation_indices = g_george_walking[LEFT];
    }
    g_george_position += g_george_movement * g_ball_speed * delta_time;
    if (balls > 1) {
        g_face1_position += g_face1_movement * g_ball_speed * delta_time;
    } if (balls > 2) {
		g_face2_position += g_face2_movement * g_ball_speed * delta_time;
	}
    g_kano_position += g_kano_movement * anime_speed * delta_time;
    g_mahiru_position += g_mahiru_movement * anime_speed * delta_time;
    /* TRANSFORMATIONS */
    g_george_matrix = glm::mat4(1.0f);
    g_george_matrix = glm::translate(g_george_matrix, g_george_position);
    g_kano_matrix = glm::mat4(1.0f);
    g_kano_matrix = glm::translate(g_kano_matrix, g_kano_position);
    g_kano_matrix = glm::scale(g_kano_matrix, ANIME_SCALE);
    g_mahiru_matrix = glm::mat4(1.0f);
    g_mahiru_matrix = glm::translate(g_mahiru_matrix, g_mahiru_position);
    g_mahiru_matrix = glm::scale(g_mahiru_matrix, ANIME_SCALE);
    g_face1_matrix = glm::mat4(1.0f);
    g_face1_matrix = glm::translate(g_face1_matrix, g_face1_position);
    g_face1_matrix = glm::scale(g_face1_matrix, face_scale);
    g_face2_matrix = glm::mat4(1.0f);
    g_face2_matrix = glm::translate(g_face2_matrix, g_face2_position);
    g_face2_matrix = glm::scale(g_face2_matrix, face_scale);
}


void draw_object(glm::mat4& object_model_matrix, GLuint& object_texture_id)
{
    g_shader_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}


void render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    // Textures
    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false,
        0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT,
        false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    draw_object(g_kano_matrix, g_kano_texture_id);
    draw_object(g_mahiru_matrix, g_mahiru_texture_id);
    if (balls > 1) {
		draw_object(g_face1_matrix, g_face1_texture_id);
    } if (balls > 2) {
        draw_object(g_face2_matrix, g_face2_texture_id);
    }
    g_shader_program.set_model_matrix(g_george_matrix);
    draw_sprite_from_texture_atlas(&g_shader_program, g_george_texture_id,
        g_animation_indices[g_animation_index],
        SPRITESHEET_DIMENSIONS, SPRITESHEET_DIMENSIONS);
    if (game_finished) {
        draw_text(&g_shader_program, g_font_texture_id, "Game Finished", 0.5f, 0.05f,
            glm::vec3(-3.0f, 2.0f, 0.0f));
        if (mahiru_score > kano_score) {
            draw_text(&g_shader_program, g_font_texture_id, "Mahiru Won", 0.5f, 0.05f,
                glm::vec3(-2.5f, 1.5f, 0.0f));
        } else if (kano_score > mahiru_score) {
			draw_text(&g_shader_program, g_font_texture_id, "Kano Won", 0.5f, 0.05f,
				glm::vec3(-2.0f, 1.5f, 0.0f));
        }
        else {
            draw_text(&g_shader_program, g_font_texture_id, "Draw", 0.5f, 0.05f,
                glm::vec3(-1.0f, 1.5f, 0.0f));
        }
    }
    else {
        draw_text(&g_shader_program, g_font_texture_id, "Kano: " + std::to_string(kano_score), 0.5f, 0.05f,
            glm::vec3(-3.0f, 2.0f, 0.0f));
        draw_text(&g_shader_program, g_font_texture_id, "Mahiru: " + std::to_string(mahiru_score), 0.5f, 0.05f,
            glm::vec3(-3.0f, 1.5f, 0.0f));
    }

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