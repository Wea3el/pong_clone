#define GL_GLEXT_PROTOTYPES 1
#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

// —— NEW STUFF —— //
#include <ctime>   //
#include "cmath"   //
// ——————————————— //

const int WINDOW_WIDTH  = 640,
          WINDOW_HEIGHT = 480;

const float BG_RED     = 0.1922f,
            BG_BLUE    = 0.549f,
            BG_GREEN   = 0.9059f,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X      = 0,
          VIEWPORT_Y      = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char  V_SHADER_PATH[]          = "shaders/vertex_textured.glsl",
            F_SHADER_PATH[]          = "shaders/fragment_textured.glsl",
            PLAYER_SPRITE_FILEPATH[] = "assets/rectangle.png",
            WIN1_SPRITE_FILEPATH[] = "assets/winner-1.png",
WIN2_SPRITE_FILEPATH[] = "assets/winner-2.png",
BALL_SPRITE_FILEPATH[] = "assets/ballkirby.png";


const float MILLISECONDS_IN_SECOND     = 1000.0;
const float MINIMUM_COLLISION_DISTANCE = 1.0f;
const float ANGLE = glm::radians(90.0f);\
const float ROT_SPEED = 200.0f;

const int   NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL    = 0;
const GLint TEXTURE_BORDER     = 0;

SDL_Window* g_display_window;
bool  g_game_is_running = true;
float g_previous_ticks  = 0.0f;

ShaderProgram g_shader_program;
glm::mat4     g_view_matrix,
              g_paddle1_matrix,
              g_projection_matrix,
              g_paddle2_matrix,
              g_ball_matrix,
              g_player1_win_matrix,
              g_player2_win_matrix;

GLuint g_paddle_texture_id,
       g_other_texture_id,
       g_player1_win_id,
       g_player2_win_id,
       g_ball_texture_id;

const glm::vec3 PADDLE_INIT_SCAL = glm::vec3(0.8f,0.2f,0.0f);
const glm::vec3 PADDLE1_INIT_POS = glm::vec3(-4.8f,0.0f,1.0f);
glm::vec3 paddle1_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 paddle1_movement = glm::vec3(0.0f, 0.0f, 0.0f);
bool paddle1_at_top = false;
bool paddle1_at_bot = false;

const glm::vec3 PADDLE2_INIT_POS = glm::vec3(4.8f,0.0f,1.0f);
glm::vec3 paddle2_position  = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 paddle2_movement  = glm::vec3(0.0f, 0.0f, 0.0f);
bool allow_movement = true;
bool paddle2_at_top = false;
bool paddle2_at_bot = false;
bool go_up = true;

glm::vec3 ball_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 ball_movement = glm::vec3(0.0f, 0.0f, 0.0f);
const glm::vec3 BALL_INIT_SCAL = glm::vec3(0.3f, 0.3f,0.0f);
bool beginning = true;
bool left = false,
    right = false,
    up = false,
    down = false;
float x_speed = 0.0f;
float y_speed = 0.0f;
bool bool_rand_up = false;

bool start = false;
float g_player_speed = 4.0f;
float ball_speed = 2.0f;
bool player1_wins = false;
bool player2_wins = false;
float g_rot_angle = 0.0f;


GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }
    
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(image);
    
    return textureID;
}


void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Hello, Collisions!",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_paddle1_matrix = glm::mat4(1.0f);
    
    g_paddle2_matrix = glm::mat4(1.0f);
    
    g_ball_matrix = glm::mat4(1.0f);
    
    g_player1_win_matrix = glm::mat4(1.0f);
    g_player1_win_matrix =  glm::scale(g_player1_win_matrix, glm::vec3(3.0f));
    
    g_player2_win_matrix = glm::mat4(1.0f);
    g_player2_win_matrix =  glm::scale(g_player2_win_matrix, glm::vec3(3.0f));
//    g_player1_win_matrix = glm::translate(g_player1_win_matrix, glm::vec3(8.0f));
//    g_player2_win_matrix = glm::translate(g_player2_win_matrix, glm::vec3(8.0f));
    
//    g_paddle2_matrix = glm::translate(g_paddle2_matrix, glm::vec3(1.0f, 1.0f, 0.0f));
//
//    paddle2_position    += paddle2_movement;
    
    g_view_matrix       = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_paddle_texture_id = load_texture(PLAYER_SPRITE_FILEPATH);
    g_ball_texture_id = load_texture(BALL_SPRITE_FILEPATH);
    g_player1_win_id = load_texture(WIN1_SPRITE_FILEPATH);
    g_player2_win_id = load_texture(WIN2_SPRITE_FILEPATH);
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    
    glUseProgram(g_shader_program.get_program_id());
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void process_input()
{
    paddle1_movement = glm::vec3(0.0f);
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_game_is_running = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                    case SDLK_q:
                        g_game_is_running = false;
                        break;
                    
                    case SDLK_m:
                        start = true;
                        break;
                        
                    default:
                        break;
                }
                
            default:
                break;
        }
    }
    
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);

    if(!player1_wins && !player2_wins){
        if (key_state[SDL_SCANCODE_UP] and !paddle1_at_top)
        {
            paddle1_movement.y = 1.0f;
        }
        else if (key_state[SDL_SCANCODE_DOWN] and !paddle1_at_bot)
        {
            paddle1_movement.y = -1.0f;
        }
        
        if (glm::length(paddle1_movement) > 1.0f)
        {
            paddle1_movement = glm::normalize(paddle1_movement);
        }
        
        if(paddle1_position.y > 3.0f){
            paddle1_at_top = true;
        }
        else{
            paddle1_at_top = false;
        }
        if(paddle1_position.y < -2.9f){
            paddle1_at_bot = true;
        }
        else{
            paddle1_at_bot = false;
        }
    }
    
}

void process_input_2()
{
    paddle2_movement = glm::vec3(0.0f);
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_game_is_running = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                    case SDLK_q:
                        g_game_is_running = false;
                        break;
                    
                    case SDLK_t:
                        allow_movement = false;
                        break;
                        
                    
                        
                    default:
                        break;
                }
                
            default:
                break;
        }
    }
    
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);

    if(!player1_wins && !player2_wins){
        if (key_state[SDL_SCANCODE_W] and !paddle2_at_top)
        {
            paddle2_movement.y = 1.0f;
        }
        else if (key_state[SDL_SCANCODE_S] and !paddle2_at_bot)
        {
            paddle2_movement.y = -1.0f;
        }
        if(key_state[SDL_SCANCODE_T]){
            allow_movement = !allow_movement;
        }
        
        if (glm::length(paddle2_movement) > 1.0f)
        {
            paddle2_movement = glm::normalize(paddle2_movement);
        }
        
        if(paddle2_position.y > 3.0f){
            paddle2_at_top = true;
        }
        else{
            paddle2_at_top = false;
        }
        if(paddle2_position.y < -2.9f){
            paddle2_at_bot = true;
        }
        else{
            paddle2_at_bot = false;
        }
    }
    

}

void paddle2_up_down(){
    if(!player1_wins && !player2_wins){
        if(go_up){
            paddle2_movement.y = 1.0f;
        }
        else{
            paddle2_movement.y = -1.0f;
        }
        if(paddle2_position.y >3.0f){
            go_up = false;
        }
        else if(paddle2_position.y < -2.9f){
            go_up = true;
        }
    }
    
    
}

bool check_collision_1(glm::vec3 &position_a, glm::vec3 &position_b, const glm::vec3 &init_pos)  //
{                                                                   //
    // —————————————————  Distance Formula ————————————————————————
    
    float x_distance = fabs(position_a.x - init_pos.x) - ((BALL_INIT_SCAL.x*0.45 + PADDLE_INIT_SCAL.x * 0.45) / 2.0f);
    float y_distance = fabs(position_a.y - position_b.y) - ((BALL_INIT_SCAL.y*2.5f + PADDLE_INIT_SCAL.y*2.5f) / 2.0f);

    if (x_distance < 0 && y_distance <= 0){
        return true;
    }
    else{
        return false;
    }//
    // ———————————————————————————————————————————————————————————————— //
}


bool rand_up_or_down(){
    double rand_up = ((double)rand()) / double(RAND_MAX)+0.5;
    return rand_up >=1;
}
void ball_direction(){
    ball_movement = glm::vec3(0.0f);
    
    
    
    if(check_collision_1(ball_position, paddle1_position, PADDLE1_INIT_POS)){
        right = false;
        left = true;
        x_speed = ((float)rand()) / ((float)RAND_MAX) + 1.0;
        y_speed = ((float)rand()) / ((float)RAND_MAX)/2.0 + 0.5;
        
        bool_rand_up = rand_up_or_down();
        if(bool_rand_up){
            up = true;
            down = false;
        }else{
            up = false;
            down = true;
        }

    }
    else if(check_collision_1(ball_position, paddle2_position, PADDLE2_INIT_POS)){
        right = true;
        left = false;
        x_speed = ((float)rand()) / ((float)RAND_MAX) + 1.0;
        y_speed = ((float)rand()) / ((float)RAND_MAX)/2.0 + 0.5;
        bool_rand_up = rand_up_or_down();
        if(bool_rand_up){
            up = true;
            down = false;
        }else{
            up = false;
            down = true;
        }
        


    }
    if(ball_position.y > 3.7f){
        
        up = false;
        down = true;
    }else if(ball_position.y < -3.5f){
        up = true;
        down = false;
    }
    
    if(left){
        ball_movement.x = x_speed;
    }
    else if(right){
        ball_movement.x = -x_speed;
    }else{
        ball_movement.x = 1;
    }
    if(up){
        ball_movement.y = y_speed;
    }else if(down){
        ball_movement.y = -y_speed;
    }else{
        ball_movement.y = 0.0f;
    }
    if(ball_position.x < -5.0){
        std::cout << "player 2 wins\n";
        player2_wins = true;
        
    }
    else if(ball_position.x > 5.0){
        std::cout << "player 1 wins\n";
        player1_wins = true;
    }
    
    
    
    
}
// ————————————————————————— NEW STUFF ———————————————————————————— //


void update()
{
    float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    g_paddle1_matrix     =glm::mat4(1.0f);
    g_paddle1_matrix = glm::translate(g_paddle1_matrix, PADDLE1_INIT_POS);
    paddle1_position += paddle1_movement * g_player_speed * delta_time;
    
    g_paddle1_matrix     = glm::translate(g_paddle1_matrix, paddle1_position);
    g_paddle1_matrix = glm:: rotate(g_paddle1_matrix, ANGLE, glm::vec3(0.0f,0.0f,1.0f));
    g_paddle1_matrix = glm::scale(g_paddle1_matrix, PADDLE_INIT_SCAL);
    
    g_paddle2_matrix     = glm::mat4(1.0f);
    g_paddle2_matrix = glm::translate(g_paddle2_matrix, PADDLE2_INIT_POS);
    paddle2_position += paddle2_movement * g_player_speed * delta_time;
    
    g_paddle2_matrix     = glm::translate(g_paddle2_matrix, paddle2_position);
    g_paddle2_matrix = glm:: rotate(g_paddle2_matrix, ANGLE, glm::vec3(0.0f,0.0f,1.0f));
    g_paddle2_matrix = glm::scale(g_paddle2_matrix, PADDLE_INIT_SCAL);

    
    g_ball_matrix = glm::mat4(1.0f);
    ball_position += ball_movement * ball_speed * delta_time;
    g_ball_matrix = glm::translate(g_ball_matrix, ball_position);
    g_ball_matrix = glm::scale(g_ball_matrix, BALL_INIT_SCAL);
    
    g_rot_angle += ROT_SPEED *delta_time;
    g_ball_matrix = glm::rotate(g_ball_matrix, glm::radians(g_rot_angle), glm::vec3(0.0f,0.0f,1.0f));
    // —————————————————————————————————————————————————————————//
}


void draw_object(glm::mat4 &object_model_matrix, GLuint &object_texture_id)
{
    g_shader_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void render_paddle()
{
    glClear(GL_COLOR_BUFFER_BIT);
    
    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f
    };

    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    };
    
    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    draw_object(g_paddle1_matrix, g_paddle_texture_id);
    draw_object(g_paddle2_matrix, g_paddle_texture_id);
    draw_object(g_ball_matrix, g_ball_texture_id);
    if(player1_wins){
        draw_object(g_player1_win_matrix, g_player1_win_id);
    }else if(player2_wins){
        draw_object(g_player2_win_matrix, g_player2_win_id);
    }
    
    
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    SDL_GL_SwapWindow(g_display_window);
}



void shutdown() { SDL_Quit(); }


int main(int argc, char* argv[])
{
    initialise();
    
    while (g_game_is_running)
    {
        
        process_input();
        if(allow_movement){
            process_input_2();
        }else{
            paddle2_up_down();
        }
        if(start && !player1_wins && !player2_wins){
            ball_direction();
        }
        update();
        render_paddle();
    }
        
    
    shutdown();
    return 0;
}
