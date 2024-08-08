#include "LevelA.h"
#include "Utility.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

constexpr char SPRITESHEET_FILEPATH[] = "assets/bunny_wes.png",
           ENEMY_FILEPATH[]       = "assets/mohan.jpg";
constexpr char test_wall[] = "assets/platform.jpg";


unsigned int LEVELA_DATA[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

LevelA::~LevelA()
{
    delete [] m_game_state.enemies;
    delete    m_game_state.player;
    delete    m_game_state.map;
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void LevelA::initialise()
{
    m_game_state.next_scene_id = -1;
    
    GLuint map_texture_id = Utility::load_texture("assets/tileset.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELA_DATA, map_texture_id, 1.0f, 4, 1);
    
    GLuint player_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);
    
    m_game_state.player = new Entity(
        player_texture_id,         // texture id
        5.0f,                      // speed
        1.0f,                      // width
        1.0f,                      // height
        PLAYER
    );
    // Entity(GLuint texture_id, float speed, float width, float height, EntityType EntityType);
        
    m_game_state.player->set_position(glm::vec3(5.0f, 1.0f, 0.0f));

    GLuint wall_texture_id = Utility::load_texture(test_wall);
    m_game_state.left_wall = new Entity(wall_texture_id, 0.0f, 1.0f, 10.0f, WALL);
    m_game_state.left_wall->set_position(m_game_state.player->get_position() + glm::vec3(-4.0f, 0.0f, 0.0f));
    m_game_state.top_wall = new Entity(wall_texture_id, 0.0f, 10.0f, 1.0f, WALL);
    m_game_state.top_wall->set_position(m_game_state.player->get_position() + glm::vec3(0.0f, 3.0f, 0.0f));
    m_game_state.right_wall = new Entity(wall_texture_id, 0.0f, 1.0f, 10.0f, WALL);
    m_game_state.right_wall->set_position(m_game_state.player->get_position() + glm::vec3(4.0f, 0.0f, 0.0f));
    m_game_state.bottom_wall = new Entity(wall_texture_id, 0.0f, 10.0f, 1.0f, WALL);
    m_game_state.bottom_wall->set_position(m_game_state.player->get_position() + glm::vec3(0.0f, -3.0f, 0.0f));
    m_game_state.left_wall->update(0.0f, m_game_state.player, m_game_state.player, 0.0, m_game_state.map);
    m_game_state.top_wall->update(0.0f, m_game_state.player, m_game_state.player, 0.0, m_game_state.map);
    m_game_state.right_wall->update(0.0f, m_game_state.player, m_game_state.player, 0.0, m_game_state.map);
    m_game_state.bottom_wall->update(0.0f, m_game_state.player, m_game_state.player, 0.0, m_game_state.map);
    m_game_state.walls = new Entity[4];
    m_game_state.walls[0] = *m_game_state.left_wall;
    m_game_state.walls[1] = *m_game_state.top_wall;
    m_game_state.walls[2] = *m_game_state.right_wall;
    m_game_state.walls[3] = *m_game_state.bottom_wall;
    /**
    Enemies' stuff */
    GLuint enemy_texture_id = Utility::load_texture(ENEMY_FILEPATH);

    m_game_state.enemies = new Entity[ENEMY_COUNT];

    for (int i = 0; i < ENEMY_COUNT; i++){
        m_game_state.enemies[i] = Entity(enemy_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, ASTEROID, IDLE);
    }
    m_game_state.enemies[0].set_position(glm::vec3(5.0f, -0.5f, 0.0f));

    /*
    Screen Edge Colliders
    */

    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    m_game_state.bgm = Mix_LoadMUS("assets/dooblydoo.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(0.0f);
    
    m_game_state.jump_sfx = Mix_LoadWAV("assets/bounce.wav");
}

void LevelA::update(float delta_time)
{
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_COUNT, m_game_state.map);
    m_game_state.enemies->update(delta_time, m_game_state.player, m_game_state.walls, 4, m_game_state.map);
    /*
    m_game_state.left_wall->update(delta_time, m_game_state.player, m_game_state.player, 0.0, m_game_state.map);
    m_game_state.top_wall->update(delta_time, m_game_state.player, m_game_state.player, 0.0, m_game_state.map);
    m_game_state.right_wall->update(delta_time, m_game_state.player, m_game_state.player, 0.0, m_game_state.map);
    m_game_state.bottom_wall->update(delta_time, m_game_state.player, m_game_state.player, 0.0, m_game_state.map);
    */
    // if (m_game_state.player->get_position().y < -10.0f) m_game_state.next_scene_id = 1;
}

void LevelA::render(ShaderProgram *program)
{
    m_game_state.map->render(program);
    m_game_state.player->render(program);
    m_game_state.enemies->render(program);
    m_game_state.left_wall->render(program);
    m_game_state.top_wall->render(program);
    m_game_state.right_wall->render(program);
    m_game_state.bottom_wall->render(program);
}
