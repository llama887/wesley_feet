#ifndef ENTITY_H
#define ENTITY_H

#include "Map.h"
#include "glm/glm.hpp"
#include "ShaderProgram.h"
enum EntityType { PLATFORM, PLAYER, ENEMY, BULLET  };
enum AIType     { WALKER, GUARD, ASTEROID };
enum AIState    { WALKING, IDLE, ATTACKING };


enum AnimationDirection { LEFT, RIGHT, UP, DOWN };

class Entity
{
private:
    bool m_is_active = true;
    
    int m_walking[4][4]; // 4x4 array for walking animations

    
    EntityType m_entity_type;
    AIType     m_ai_type;
    AIState    m_ai_state;
    // ————— TRANSFORMATIONS ————— //
    glm::vec3 m_movement;
    glm::vec3 m_position;
    glm::vec3 m_scale;
    glm::vec3 m_velocity;
    glm::vec3 m_acceleration;

    glm::mat4 m_model_matrix;

    float     m_speed,
              m_jumping_power;
    
    bool m_is_jumping;

    // ————— TEXTURES ————— //
    GLuint    m_texture_id;

    // ————— ANIMATION ————— //
    int m_animation_cols;
    int m_animation_frames,
        m_animation_index,
        m_animation_rows;

    int* m_animation_indices = nullptr;
    float m_animation_time = 0.0f;

    float m_width = 1.0f,
          m_height = 1.0f;
    // ————— COLLISIONS ————— //
    bool m_collided_top    = false;
    bool m_collided_bottom = false;
    bool m_collided_left   = false;
    bool m_collided_right  = false;

    const float max_acceleration = 0.5f;
    glm::vec3 asteroid_velocity = glm::vec3(rand() % 2 - 1, rand() % 2 - 1, 0.0f);
    const float asteroid_speed_limiter = 0.2;
    const float asteroid_x_limit = 7.5f / 2;
    const float asteroid_y_limit = 10.0f / 2;
    const float player_top_speed = 5.0f;
public:
    void acclerate_towards(glm::vec3 target);
    // ————— STATIC VARIABLES ————— //
    static constexpr int SECONDS_PER_FRAME = 4;

    // ————— METHODS ————— //
    Entity();
    Entity(GLuint texture_id, float speed, glm::vec3 acceleration, float jump_power, int walking[4][4], float animation_time,
        int animation_frames, int animation_index, int animation_cols,
        int animation_rows, float width, float height, EntityType EntityType);
    Entity(GLuint texture_id, float speed, float width, float height, EntityType EntityType); // Simpler constructor
    Entity(GLuint texture_id, float speed, float width, float height, EntityType EntityType, AIType AIType, AIState AIState); // AI constructor
    // Copy constructor
    Entity(const Entity& other)
        : m_is_active(other.m_is_active),
        m_entity_type(other.m_entity_type),
        m_ai_type(other.m_ai_type),
        m_ai_state(other.m_ai_state),
        m_movement(other.m_movement),
        m_position(other.m_position),
        m_scale(other.m_scale),
        m_velocity(other.m_velocity),
        m_acceleration(other.m_acceleration),
        m_model_matrix(other.m_model_matrix),
        m_speed(other.m_speed),
        m_jumping_power(other.m_jumping_power),
        m_is_jumping(other.m_is_jumping),
        m_texture_id(other.m_texture_id),
        m_animation_cols(other.m_animation_cols),
        m_animation_frames(other.m_animation_frames),
        m_animation_index(other.m_animation_index),
        m_animation_rows(other.m_animation_rows),
        m_animation_time(other.m_animation_time),
        m_width(other.m_width),
        m_height(other.m_height),
        m_collided_top(other.m_collided_top),
        m_collided_bottom(other.m_collided_bottom),
        m_collided_left(other.m_collided_left),
        m_collided_right(other.m_collided_right),
        max_acceleration(other.max_acceleration)
    {
        if (other.m_animation_indices) {
            m_animation_indices = new int[other.m_animation_frames];
            std::copy(other.m_animation_indices, other.m_animation_indices + other.m_animation_frames, m_animation_indices);
        }
    }

    // Copy assignment operator
    Entity& operator=(const Entity& other) {
        if (this == &other) {
            return *this;
        }

        m_is_active = other.m_is_active;
        m_entity_type = other.m_entity_type;
        m_ai_type = other.m_ai_type;
        m_ai_state = other.m_ai_state;
        m_movement = other.m_movement;
        m_position = other.m_position;
        m_scale = other.m_scale;
        m_velocity = other.m_velocity;
        m_acceleration = other.m_acceleration;
        m_model_matrix = other.m_model_matrix;
        m_speed = other.m_speed;
        m_jumping_power = other.m_jumping_power;
        m_is_jumping = other.m_is_jumping;
        m_texture_id = other.m_texture_id;
        m_animation_cols = other.m_animation_cols;
        m_animation_frames = other.m_animation_frames;
        m_animation_index = other.m_animation_index;
        m_animation_rows = other.m_animation_rows;
        m_animation_time = other.m_animation_time;
        m_width = other.m_width;
        m_height = other.m_height;
        m_collided_top = other.m_collided_top;
        m_collided_bottom = other.m_collided_bottom;
        m_collided_left = other.m_collided_left;
        m_collided_right = other.m_collided_right;

        if (m_animation_indices) {
            delete[] m_animation_indices;
        }
        if (other.m_animation_indices) {
            m_animation_indices = new int[other.m_animation_frames];
            std::copy(other.m_animation_indices, other.m_animation_indices + other.m_animation_frames, m_animation_indices);
        }
        else {
            m_animation_indices = nullptr;
        }
        return *this;
    }

    // Move constructor
    Entity(Entity&& other) noexcept
        : m_is_active(std::move(other.m_is_active)),
        m_entity_type(std::move(other.m_entity_type)),
        m_ai_type(std::move(other.m_ai_type)),
        m_ai_state(std::move(other.m_ai_state)),
        m_movement(std::move(other.m_movement)),
        m_position(std::move(other.m_position)),
        m_scale(std::move(other.m_scale)),
        m_velocity(std::move(other.m_velocity)),
        m_acceleration(std::move(other.m_acceleration)),
        m_model_matrix(std::move(other.m_model_matrix)),
        m_speed(std::move(other.m_speed)),
        m_jumping_power(std::move(other.m_jumping_power)),
        m_is_jumping(std::move(other.m_is_jumping)),
        m_texture_id(std::move(other.m_texture_id)),
        m_animation_cols(std::move(other.m_animation_cols)),
        m_animation_frames(std::move(other.m_animation_frames)),
        m_animation_index(std::move(other.m_animation_index)),
        m_animation_rows(std::move(other.m_animation_rows)),
        m_animation_time(std::move(other.m_animation_time)),
        m_width(std::move(other.m_width)),
        m_height(std::move(other.m_height)),
        m_collided_top(std::move(other.m_collided_top)),
        m_collided_bottom(std::move(other.m_collided_bottom)),
        m_collided_left(std::move(other.m_collided_left)),
        m_collided_right(std::move(other.m_collided_right)),
        max_acceleration(std::move(other.max_acceleration)),
        m_animation_indices(other.m_animation_indices) // Take ownership of the pointer
    {
        other.m_animation_indices = nullptr; // Prevent the moved-from object from deleting the pointer
    }

    // Move assignment operator
    Entity& operator=(Entity&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        m_is_active = std::move(other.m_is_active);
        m_entity_type = std::move(other.m_entity_type);
        m_ai_type = std::move(other.m_ai_type);
        m_ai_state = std::move(other.m_ai_state);
        m_movement = std::move(other.m_movement);
        m_position = std::move(other.m_position);
        m_scale = std::move(other.m_scale);
        m_velocity = std::move(other.m_velocity);
        m_acceleration = std::move(other.m_acceleration);
        m_model_matrix = std::move(other.m_model_matrix);
        m_speed = std::move(other.m_speed);
        m_jumping_power = std::move(other.m_jumping_power);
        m_is_jumping = std::move(other.m_is_jumping);
        m_texture_id = std::move(other.m_texture_id);
        m_animation_cols = std::move(other.m_animation_cols);
        m_animation_frames = std::move(other.m_animation_frames);
        m_animation_index = std::move(other.m_animation_index);
        m_animation_rows = std::move(other.m_animation_rows);
        m_animation_time = std::move(other.m_animation_time);
        m_width = std::move(other.m_width);
        m_height = std::move(other.m_height);
        m_collided_top = std::move(other.m_collided_top);
        m_collided_bottom = std::move(other.m_collided_bottom);
        m_collided_left = std::move(other.m_collided_left);
        m_collided_right = std::move(other.m_collided_right);

        if (m_animation_indices) {
            delete[] m_animation_indices;
        }

        m_animation_indices = other.m_animation_indices;
        other.m_animation_indices = nullptr;

        return *this;
    }

    // Destructor
    ~Entity() {
        delete[] m_animation_indices;
    }


    void draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index);
    bool const check_collision(Entity* other) const;
    
    void const check_collision_y(Entity* collidable_entities, int collidable_entity_count);
    void const check_collision_x(Entity* collidable_entities, int collidable_entity_count);
    
    // Overloading our methods to check for only the map
    void const check_collision_y(Map *map);
    void const check_collision_x(Map *map);
    
    void update(float delta_time, Entity *player, Entity *collidable_entities, int collidable_entity_count, Map *map);
    void render(ShaderProgram* program);

    void ai_activate(Entity *player);
    void ai_walk();
    void ai_float(Entity* player);
    void ai_guard(Entity *player);
    
    void normalise_movement() { m_movement = glm::normalize(m_movement); }

    void face_left() { m_animation_indices = m_walking[LEFT]; }
    void face_right() { m_animation_indices = m_walking[RIGHT]; }
    void face_up() { m_animation_indices = m_walking[UP]; }
    void face_down() { m_animation_indices = m_walking[DOWN]; }

    void limit_acceleration() { m_acceleration = glm::normalize(m_acceleration) * max_acceleration; };
    void move_left() {
        m_acceleration.x = -1.0f; face_left(); limit_acceleration(); }
    void move_right() {
        m_acceleration.x = 1.0f;  face_right(); limit_acceleration(); }
    void move_up() {
        m_acceleration.y = 1.0f;  face_up(); limit_acceleration(); }
    void move_down() { m_acceleration.y = -1.0f; face_down(); limit_acceleration(); }
    
    void const jump() { m_is_jumping = true; }

    // ————— GETTERS ————— //
    EntityType const get_entity_type()    const { return m_entity_type;   };
    AIType     const get_ai_type()        const { return m_ai_type;       };
    AIState    const get_ai_state()       const { return m_ai_state;      };
    float const get_jumping_power() const { return m_jumping_power; }
    glm::vec3 const get_position()     const { return m_position; }
    glm::vec3 const get_velocity()     const { return m_velocity; }
    glm::vec3 const get_acceleration() const { return m_acceleration; }
    glm::vec3 const get_movement()     const { return m_movement; }
    glm::vec3 const get_scale()        const { return m_scale; }
    GLuint    const get_texture_id()   const { return m_texture_id; }
    float     const get_speed()        const { return m_speed; }
    bool      const get_collided_top() const { return m_collided_top; }
    bool      const get_collided_bottom() const { return m_collided_bottom; }
    bool      const get_collided_right() const { return m_collided_right; }
    bool      const get_collided_left() const { return m_collided_left; }
    
    void activate()   { m_is_active = true;  };
    void deactivate() { m_is_active = false; };
    // ————— SETTERS ————— //
    void const set_entity_type(EntityType new_entity_type)  { m_entity_type = new_entity_type;};
    void const set_ai_type(AIType new_ai_type){ m_ai_type = new_ai_type;};
    void const set_ai_state(AIState new_state){ m_ai_state = new_state;};
    void const set_position(glm::vec3 new_position) { m_position = new_position; }
    void const set_velocity(glm::vec3 new_velocity) { m_velocity = new_velocity; }
    void const set_acceleration(glm::vec3 new_acceleration) { m_acceleration = new_acceleration; }
    void const set_movement(glm::vec3 new_movement) { m_movement = new_movement; }
    void const set_scale(glm::vec3 new_scale) { m_scale = new_scale; }
    void const set_texture_id(GLuint new_texture_id) { m_texture_id = new_texture_id; }
    void const set_speed(float new_speed) { m_speed = new_speed; }
    void const set_animation_cols(int new_cols) { m_animation_cols = new_cols; }
    void const set_animation_rows(int new_rows) { m_animation_rows = new_rows; }
    void const set_animation_frames(int new_frames) { m_animation_frames = new_frames; }
    void const set_animation_index(int new_index) { m_animation_index = new_index; }
    void const set_animation_time(float new_time) { m_animation_time = new_time; }
    void const set_jumping_power(float new_jumping_power) { m_jumping_power = new_jumping_power;}
    void const set_width(float new_width) {m_width = new_width; }
    void const set_height(float new_height) {m_height = new_height; }

    // Setter for m_walking
    void set_walking(int walking[4][4])
    {
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                m_walking[i][j] = walking[i][j];
            }
        }
    }
};

#endif // ENTITY_H
