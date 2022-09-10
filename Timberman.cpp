// TODO: Crear mas logs para que no tenga ese lag al cortar el arbol muy rapido
// TODO: Intenar arreglar la ralentizacion de la abeja y las nubes

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

#include <stdio.h>
#include <time.h>
#include <stdlib.h>


#ifdef _DEBUG

#define ASSERT(x) if(!(x)) __debugbreak()

#else

#define ASSERT(x) 

#endif

#define GRAPHICS_PATH   "../../assets/graphics/"
#define FONTS_PATH      "../../assets/fonts/"
#define SOUNDS_PATH     "../../assets/sounds/"

#define FPS 60
#define FRAME_TARGET_TIME (1000.0f / FPS)

#define SCREEN_WIDTH 768
#define SCREEN_HEIGHT 760

// #define NUM_BRANCHES 6  (falla)
#define NUM_BRANCHES 5

#define IDLE_FRAMES 2
#define ATTACK_FRAMES 3
#define TOTAL_FRAMES (IDLE_FRAMES + ATTACK_FRAMES)

bool audio = false;

bool restart_anim = false;

typedef struct sprite
{
    SDL_Texture* texture;
    SDL_Rect rect;
    SDL_Color color;
} Sprite;

struct game_state
{
    // Track whether the game is running
    bool running;
    bool paused;
    
    bool player_dead;
    
    int score;
    
    // Control the player input
    bool acceptInput;
    bool return_pressed;
    
    bool right_arrow;
    bool left_arrow;
} state = {0};


// Where is the player/branch?
// Left or Right
enum side { LEFT, RIGTH, NONE };

enum Player_State {   
    IDLE, 
    ATTACKING, 
} player_state = IDLE;


SDL_Color color_white = { 0xFF, 0xFF, 0xFF, 0xFF };
SDL_Color color_black = { 0x00, 0x00, 0x00, 0xFF };
SDL_Color color_gray  = { 0xAA, 0xAA, 0xAA, 0xFF };
SDL_Color color_red   = { 0xFF, 0x00, 0x00, 0xFF };


// Declaracion de funciones

bool init();
bool load_media();
Sprite load_sprite(const char* path, SDL_Renderer *renderer);
bool load_player_animations(Sprite* player_sprite);

void render_sprite(Sprite* sprite);
void draw_player(float dt, enum Player_State *anim_state);
bool draw_text(SDL_Texture** texture, SDL_Rect *rect, const char* text, SDL_Color text_color, TTF_Font* fuente);
void set_message(const char* text);

void updateBranches(int seed);

void shutdown_game();
void show_error_window(const char* titulo, const char* msg);


// Variables para controlar el tiempo
float current_time = 0;
float last_frame_time = 0;

// Time bar
float timeBarStartWidth = 400;
int timeBarHeight = 50;

SDL_Rect time_bar = {
    SCREEN_WIDTH / 2 - (int)(timeBarStartWidth / 2), 
    SCREEN_HEIGHT - timeBarHeight - 10, 
    (int)timeBarStartWidth, timeBarHeight
};

float timeRemaining = 6.0f;

// Cantidad de pixeles por segundo que se tiene que achicar la barra de tiempo.
float timeBarWidthPerSecond = timeBarStartWidth / timeRemaining;

int tree_width = SCREEN_WIDTH / 4;
int piso_height = 100;

SDL_Window* window     = NULL;
SDL_Renderer* renderer = NULL;
SDL_Event evento;

// Globally used fonts
TTF_Font* font_50 = NULL;
TTF_Font* font_40 = NULL;


// Text stuff
#define MSG_MAX_LEN 128

char message_text[MSG_MAX_LEN] = "Enter para empezar!";
SDL_Texture* message_text_texture = NULL;
SDL_Rect message_text_rect = { 0 };

char score_text[50];
SDL_Texture* score_text_texture = NULL;
SDL_Rect score_text_rect = { 0 };

SDL_Texture* nube_text_texture = NULL;

SDL_Texture* dt_texture = NULL;
char dt_text[50];


// Sprites

SDL_Texture *player_animations[TOTAL_FRAMES];

Sprite player;
SDL_Rect player_clip;

Sprite spriteRIP;

Sprite tree;
Sprite piso;
Sprite pasto;
Sprite bee;

Sprite background;
Sprite nubes[3];

Sprite spriteLog;
Sprite branches[NUM_BRANCHES];

int PLAYER_POSITION_LEFT;
int PLAYER_POSITION_RIGHT;

side player_side;

bool logActive = false;
float logSpeedX = 800;
float logSpeedY = -1000;

// Cosas de la ramas
side branchPositions[NUM_BRANCHES];

// Cosas de Audio
Mix_Music* sword_sound;
Mix_Music* dead_sound;
// TODO: Deberia haber un audio para la abeja
// Mix_Music* bee_sound;

// Are the clouds currently on screen?
bool cloud1Active = false;
bool cloud2Active = false;
bool cloud3Active = false;

// Velocidad de las nubes
float cloud1Speed = 0.0f;
float cloud2Speed = 0.0f;
float cloud3Speed = 0.0f;

// Si la abeja se esta moviendo o no
bool beeActive = false;

// Que tan rapido puede volar la abeja
// Almacena la velocidad, en pixeles por segundo.
float beeSpeed = 0.0f;


int main(int argc, char* argv[])
{
    state.running = init();
    
    ASSERT(state.running);
    
    SDL_RaiseWindow(window);
    
    while (state.running)
    {
        while (SDL_PollEvent(&evento))
        {
            switch (evento.type)
            {
                case SDL_QUIT:
                {
                    state.running = false;
                } break;
                
                case SDL_KEYDOWN:
                {
                    if (evento.key.keysym.sym == SDLK_RETURN)
                    {
                        state.return_pressed = true;
                    }
                    else if (evento.key.keysym.sym == SDLK_ESCAPE)
                    {
                        state.running = false;
                    }
                    else if (evento.key.keysym.sym == SDLK_RIGHT)
                    {
                        state.right_arrow = true;
                    }
                    else if (evento.key.keysym.sym == SDLK_LEFT)
                    {
                        state.left_arrow = true;
                    }
                } break;
                
                case SDL_KEYUP:
                {
                    if (evento.key.keysym.sym == SDLK_RETURN)
                    {
                        if (state.return_pressed)
                        {
                            state.paused = false;
                            
                            // Reset the time and the score   
                            state.score = 0;
                            timeRemaining = 6.0f;
                            
                            // Make all the branches disappear
                            for (int i = 1; i < NUM_BRANCHES; i++)
                            {
                                branchPositions[i] = NONE;
                            }
                            
                            // Make sure the gravestone is hidden
                            spriteRIP.rect.x = -1000;
                            
                            // Move the player into position
                            player.rect.x = PLAYER_POSITION_LEFT;
                            player.rect.y = piso.rect.y - player.rect.h;
                            
                            // The player starts on the left
                            player_side = LEFT;
                            player_state = IDLE;
                            
                            state.acceptInput = true;
                            
                            state.return_pressed = false;
                            state.left_arrow = false;
                            state.right_arrow = false;
                        }
                    }
                    
                    if (!state.paused)
                    {
                        // Listen for key presses again 
                        state.acceptInput = true;
                    }
                    
                } break;
            }
        }
        
        // Wrap the player controls to make sure we are accepting input
        if (state.acceptInput)
        {
            // First handle pressing the right cursor key
            if (state.right_arrow)
            {
                // NOTE: Prueba de reiniciar la animacion
                if (player_state == ATTACKING) restart_anim = true;
                
                
                // Make sure the player is on the right
                player_side = RIGTH;
                player_state = ATTACKING;
                state.score++;
                
                // Add to the amount of time remaining
                timeRemaining += (2 / state.score) + .15f;
                if (timeRemaining >= 6.0f) timeRemaining = 6.0f;
                
                player.rect.x = PLAYER_POSITION_RIGHT;
                
                // update the branches
                updateBranches(state.score);
                
                // set the log flying to the left
                spriteLog.rect.x = tree.rect.x;
                spriteLog.rect.y = player.rect.y;
                
                logSpeedX = -2000;
                logActive = true;
                
                state.right_arrow = false;
                state.acceptInput = false;
                
                if (audio) {
                    // Play a chop sound
                    if (Mix_PlayMusic(sword_sound, 1) == -1) {
                        fprintf(stderr, "Mix_PlayMusic: %s\n", Mix_GetError());
                        // No hay sonido, pero no es un error critico...
                    }
                }
            }
            
            // Handle the left cursor key
            if (state.left_arrow)
            {
                // NOTE: Prueba de reiniciar la animacion
                if (player_state == ATTACKING) restart_anim = true;
                
                // Make sure the player is on the left
                player_side = LEFT;
                player_state = ATTACKING;
                
                state.score++;
                
                // Add to the amount of time remaining
                timeRemaining += (2 / state.score) + .15f;
                if (timeRemaining >= 6.0f) timeRemaining = 6.0f;
                
                player.rect.x = PLAYER_POSITION_LEFT;
                
                // Update the branches
                updateBranches(state.score);
                
                // set the log flying to the left
                spriteLog.rect.x = tree.rect.x;
                spriteLog.rect.y = player.rect.y;
                
                logSpeedX = 2000;
                logActive = true;
                
                state.left_arrow = false;
                state.acceptInput = false;
                
                if (audio) {
                    //Play a chop sound
                    if (Mix_PlayMusic(sword_sound, 1) == -1) {
                        fprintf(stderr, "Mix_PlayMusic: %s\n", Mix_GetError());
                        // No hay sonido, pero no es un error critico...
                    }
                }
                
            }
        }
        
        /*
        //  Actualiza la escena
        */
        
        current_time = SDL_GetTicks();
        
        // Dormir el proceso hasta el target time (Bloquear a cierta cantidad de FPS)
        int time_to_wait = FRAME_TARGET_TIME - (current_time - last_frame_time);
        if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME)
        {
            SDL_Delay(time_to_wait);
        }
        
        // El valor de dt es una fraccion de 1 que representa cuanto tiempo paso desde el frame anterior
        float dt = (current_time - last_frame_time) / 1000.0f;
        last_frame_time = current_time;
        
        if (!state.paused)
        {
            // Subtract from the amount of time remaining
            // First we subtracted the amount of time the player has left by however long the previous frame took to execute 
            timeRemaining -= dt;
            
            // size up the time bar
            // This produces exactly the correct width, relative to how long the player has left.
            time_bar.w = (int)(timeBarWidthPerSecond * timeRemaining);
            
            
            if (timeRemaining <= 0.0f)
            {
                // Pause the game   
                state.paused = true;
                state.acceptInput = false;
                
                //Reposition the text based on its new size   
                message_text_rect.x = SCREEN_WIDTH / 2 - message_text_rect.w / 2 + 100;
                
                // Cambia el mensaje que se le muestra al jugador
                set_message("Se Termino el Tiempo!!");
                
                timeRemaining = 0.0f;
            }
            
            
            if (!beeActive)
            {
                // How fast is the bee
                srand((int)time(0));
                beeSpeed = (rand() % 100) + 100.0f;
                
                // How high is the bee   
                srand((int)time(0) * 10);
                
                int height = (rand() % (SCREEN_HEIGHT - bee.rect.w));
                
                bee.rect.x = 1000;
                bee.rect.y = height;
                
                beeActive = true;
            }
            else
            {
                // Move the bee
                bee.rect.x -= (int)(beeSpeed * dt);
                
                // Has the bee reached the right hand edge of the screen? 
                // whether the bee has disappeared off the left-hand side of the screen.
                if (bee.rect.x < -bee.rect.w)
                {
                    beeActive = false;
                    
                    // a new bee will be set flying at a new random height and a new random speed.
                }
            }
            
            // Manejo de las nubes
            // Cloud 1
            if (!cloud1Active)
            {
                // How fast is the cloud
                srand((int)time(0) * 10);
                cloud1Speed = (rand() % 30) + 100.0f;
                
                // Que tan alto aparece la nube
                srand((int)time(0) * 10);
                nubes[0].rect.y = (rand() % 150);
                nubes[0].rect.x = -200;
                
                cloud1Active = true;
            }
            else
            {
                nubes[0].rect.x += (int)(cloud1Speed * dt);
                
                // Si la nube pasa del lado derecho de la pantalla
                if (nubes[0].rect.x > SCREEN_WIDTH)
                {
                    // Lo prepara para que sea otra nube en el sig frame
                    cloud1Active = false;
                }
            }
            
            // Cloud 2
            if (!cloud2Active)
            {
                // How fast is the cloud
                srand((int)time(0) * 20);
                cloud2Speed = (rand() % 50) + 150.0f;
                
                // Que tan alto aparece la nube
                srand((int)time(0) * 20);
                nubes[1].rect.y = (rand() % 350) - 250;
                nubes[1].rect.x = -200;
                
                cloud2Active = true;
            }
            else
            {
                nubes[1].rect.x += (int)(cloud2Speed * dt);
                
                // Si la nube pasa del lado derecho de la pantalla
                if (nubes[1].rect.x > SCREEN_WIDTH)
                {
                    // Lo prepara para que sea otra nube en el sig frame
                    cloud2Active = false;
                }
            }
            
            // Cloud 3
            if (!cloud3Active)
            {
                // How fast is the cloud
                srand((int)time(0) + 50);
                cloud3Speed = (rand() % 30) + 100.0f;
                
                // Que tan alto aparece la nube
                srand((int)time(0) * 20);
                nubes[2].rect.y = (rand() % 450) - 150;
                nubes[2].rect.x = -200;
                
                cloud3Active = true;
            }
            else
            {
                nubes[2].rect.x += (int)(cloud3Speed * dt);
                
                // Si la nube pasa del lado derecho de la pantalla
                if (nubes[2].rect.x > SCREEN_WIDTH)
                {
                    // Lo prepara para que sea otra nube en el sig frame
                    cloud3Active = false;
                }
            }
            
            // Update the score text
            snprintf(score_text, 50, "Score = %d", state.score);
            
            
            // Update the branch sprites
            for (int i = 0; i < NUM_BRANCHES; i++)
            {
                //float height = i * 150;
                int height = i * player.rect.h;
                branches[i].rect.y = height;
                
                if (branchPositions[i] == LEFT)
                {
                    // Move the sprite to the left side
                    branches[i].rect.x = tree.rect.x - branches[i].rect.w;
                }
                else if (branchPositions[i] == RIGTH)
                {
                    // Move the sprite to the right side
                    branches[i].rect.x = tree.rect.x + tree.rect.w;
                }
                else
                {
                    // Hide the branch
                    branches[i].rect.x = 3000;
                }
                
            }
            
            // Handle a flying log
            if (logActive)
            {
                spriteLog.rect.x += (int)(logSpeedX * dt);
                spriteLog.rect.y += (int)(logSpeedY * dt);
                
                // Has the log reached the right hand edge?
                if (spriteLog.rect.x < -100 || spriteLog.rect.x > SCREEN_WIDTH)
                {
                    // Set it up ready to be a whole new log next frame
                    logActive = false;
                    spriteLog.rect.x = tree.rect.x;
                    spriteLog.rect.y = player.rect.y;
                }
                
            }
            
            // Si el jugador es aplastado por una rama
            if (branchPositions[NUM_BRANCHES - 1] == player_side)
            {
                // death
                state.paused = true;
                state.acceptInput = false;
                
                // Draw the gravestone
                spriteRIP.rect.x = player.rect.x + player.rect.w / 2;
                spriteRIP.rect.y = player.rect.y + player.rect.h / 2 - spriteRIP.rect.h;
                
                // hide the player
                player.rect.x = -2000;
                
                // Change the text of the message
                set_message("Aplastado!!");
                
                state.player_dead = true;
                
                if (audio) {
                    // Play the dead sound
                    if (Mix_PlayMusic(dead_sound, 1) == -1) {
                        fprintf(stderr, "Mix_PlayMusic: %s\n", Mix_GetError());
                    }
                }
            }
        }
        
        
        // Animar que la tumba caiga
        if (state.player_dead && (spriteRIP.rect.y + spriteRIP.rect.h) < piso.rect.y)
        {
            int tumba_vel_y = 500;
            int tumba_vel_x = 0;
            
            if (player_side == LEFT) tumba_vel_x = -100;
            if (player_side == RIGTH) tumba_vel_x = 100;
            
            spriteRIP.rect.y += (int)(tumba_vel_y * dt);
            spriteRIP.rect.x += (int)(tumba_vel_x * dt);
        }
        
        /*
        ****************************************
        Draw the scene
        ****************************************
        */
        
        // Clear everything from the last frame     
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderClear(renderer);
        
        // Draw our game scene here
        
        // Dibuja el fondo
        render_sprite(&background);
        
        // Draw the "clouds"
        for (int i = 0; i < 3; i++)
        {
            render_sprite(&nubes[i]);
        }
        
        // Dibuja el "piso"
        render_sprite(&pasto);
        
        // Draw the branches
        for (int i = 0; i < NUM_BRANCHES; i++)
        {
            // No tienen textura por ahora
            render_sprite(&branches[i]);
        }
        
        
        // Draw the "tree"
        render_sprite(&tree);
        
        // Draw the player animated
        draw_player(dt, &player_state);
        
        // Draw the flying log
        render_sprite(&spriteLog);
        
        // Remarca el log
#ifdef _DEBUG
        SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF);
        SDL_RenderDrawRect(renderer, &spriteLog.rect);
#endif
        
        
        // Draw the gravestone 
        render_sprite(&spriteRIP);
        
        
        // Draw the "bee"
        render_sprite(&bee);
        
        
        // Draw score text
        score_text_rect.x = 20;
        score_text_rect.y = 20;
        draw_text(&score_text_texture, &score_text_rect, score_text, color_red, font_40);
        
        // Draw the time bar
        SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
        SDL_RenderFillRect(renderer, &time_bar);
        
        
        if (state.paused)
        {
            message_text_rect.x = SCREEN_WIDTH / 2 - message_text_rect.w / 2;
            message_text_rect.y = SCREEN_HEIGHT / 2 - message_text_rect.h / 2;
            
            draw_text(&message_text_texture, &message_text_rect, message_text, color_red, font_50);
        }
        
        // Show everything we just drew
        SDL_RenderPresent(renderer);
    }
    
    
    shutdown_game();
    
    return 0;
}


bool init()
{
    bool success = false;
    
    //SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK);
    
    window = SDL_CreateWindow("Timberman!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        
    if (!window || !renderer)
    {
        show_error_window("Error al inicializar SDL", 
                          "No se pudo inicializar la ventana o el renderer.");
    }
    
    //Initialize PNG loading
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        
        ASSERT(false);
        
        return false;
    }
    
    
    // Initialize SDL_ttf
    if (TTF_Init() == -1)
    {
        printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    
    
    // Inicializa SDL_Mixer
    //int flags = MIX_INIT_MP3 | MIX_INIT_OGG;
    int flags = MIX_INIT_OGG;
    int initted = Mix_Init (flags);
    if ((initted & flags) != flags)
    {
        fprintf(stderr, "Error al inicializar SDL_Mix: %s\n", Mix_GetError());
        
    #ifdef _DEBUG
        fprintf(stderr, "%s\n", SDL_GetError());
        ASSERT(false);
            
    #else
        audio = false;
    #endif
    }
    else {
        audio = true;
    }
    
    
    // Inicializacion del estado del juego
    
    // El juego inicia en pausa
    state.paused = true;
    state.running = false;
    
    
    state.player_dead = false;
    state.score = 0;
    
    // Control the player input
    state.acceptInput = false;
    state.return_pressed = false;
    
    state.right_arrow = false;
    state.left_arrow = false;
    
    
    success = load_media();
    
    
    return success;
}

bool load_media()
{
    bool success = true;
    
    // Inicializa el score text
    snprintf(score_text, 50, "Score = %d", state.score);
    
    // Inicializa el arbol (sin textura por ahora)
    tree = { NULL, 
        { SCREEN_WIDTH / 2 - tree_width / 2, 0, tree_width, SCREEN_HEIGHT - piso_height }, 
        { 0x68, 0x4E, 0x1D, 0xFF }  };
    
    piso = { 
        NULL, 
        { 0, SCREEN_HEIGHT - piso_height, SCREEN_WIDTH, piso_height }, 
        { 0x82, 0xD8, 0x00, 0xFF} 
    };
    
    
    // Prepare the player
    player.rect.w = 200;
    player.rect.h = 140;
    player.rect.x = tree.rect.x - player.rect.w;
    player.rect.y = piso.rect.y - player.rect.h;
    
    
    if (load_player_animations(&player))
    {
        player.texture = NULL;
        
        // Recorta el espacio transparente del jugador para que calze con el arbol (Temporal)
        SDL_QueryTexture(player_animations[2], NULL, NULL, &player_clip.w, &player_clip.h);
        //player_clip.x += 100;
        player_clip.w -= 100;
    }
    else
    {
        show_error_window("Error al cargar los assets",
                          "Error al cargar las animaciones, deberian estar en assets/graphics/character");
        
        success = false;
    }
    
    // The player starts on the left
    player_side = LEFT;
    
    
    // Prepare the gravestone
    spriteRIP = { 
        {NULL}, 
        {-1000, 560, player.rect.w / 2, player.rect.h}, 
        { 0xAA, 0xAA, 0xAA, 0xFF }
    };
    
    
    // Some other useful log related variables 
    logSpeedX = 800;
    logSpeedY = -1000;
    
    
    background = load_sprite(GRAPHICS_PATH "background/sky01.png", renderer);
    nubes[0]   = load_sprite(GRAPHICS_PATH "nubes/cloud4.png", renderer);
    nubes[0].rect.x = -nubes[0].rect.w;
    //nubes[0].rect.x = 0;
    
    nubes[1] = load_sprite(GRAPHICS_PATH "nubes/cloud5.png", renderer);
    nubes[1].rect.x = -nubes[1].rect.w;
    
    nubes[2] = load_sprite(GRAPHICS_PATH "nubes/cloud6.png", renderer);
    nubes[2].rect.x = -nubes[2].rect.w;
    
    
    pasto = { NULL,
        { 0, SCREEN_HEIGHT - piso_height - 100, SCREEN_WIDTH, piso_height + 100},
        { 0x82, 0xD8, 0x00, 0xFF}
    };
    
    bee = {
        NULL,
        {-1000, 0, 50, 50},
        { 0xFF, 0xE6, 0x03, 0xFF }
    };
    
    // Prepare the flying log 
    //("graphics/log.png");
    spriteLog = { 
        { NULL }, 
        { tree.rect.x, player.rect.y, tree.rect.w, player.rect.h }, 
        { 0x68, 0x4E, 0x1D, 0xFF }
    };
    
    
    // Line the player up with the tree
    PLAYER_POSITION_LEFT = tree.rect.x - player.rect.w;
    PLAYER_POSITION_RIGHT = tree.rect.x + tree.rect.w;
    
    
    // Prepare 6 branches
    // Cargar la textura de la rama (cuando tenga una)
    
    // Set the texture and rect for each branch sprite
    for (int i = 0; i < NUM_BRANCHES; i++)
    {
        branches[i].texture = NULL;
        branches[i].color = { 0x68, 0x4E, 0x1D, 0xFF };
        
        branches[i].rect.x = -2000;
        branches[i].rect.y = -2000;
        branches[i].rect.w = 150;
        branches[i].rect.h = 20;
    }
    
    
    // Carga las fuentes
    font_50 = TTF_OpenFont(FONTS_PATH "KOMIKAP_.ttf", 50);
    font_40 = TTF_OpenFont(FONTS_PATH "KOMIKAP_.ttf", 40);
    
    if (!font_50 || !font_40)
    {
        show_error_window("Error al cargar la fuente", "No se pudo cargar la fuente: assets/fonts/KOMIKAP_.ttf");
    }
    
    
#if 1
    // Carga los efectos de sonido 
    if (audio) {
        // Initialize the mixer API.
        // This must be called before using other functions in this library.
        int open_audio = Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 2048);
        if (open_audio == -1)
        {
            printf("SDL_Mix Error: %s\n", Mix_GetError());
            success = false;
            shutdown_game();

            show_error_window("Error al cargar audio", "Error al inicializar el dispositivo de audio.");
        }

        // Audio de corte
        sword_sound = Mix_LoadMUS(SOUNDS_PATH "sword_1.ogg");
        if (!sword_sound)
        {
            printf("SDL_Mix Error: %s\n", Mix_GetError());
            success = false;
            shutdown_game();

            show_error_window("Error al cargar efecto de sonido", "Error al cargar el efecto de sonido de corte: assets/sounds/sword_1.ogg");

            // NOTE: Esto podria ser un error critico...
        }

        // Audio de muerte
        dead_sound = Mix_LoadMUS(SOUNDS_PATH "bone_crushing.wav");
        if (!dead_sound)
        {
            printf("SDL_Mix Error: %s\n", Mix_GetError());
            success = false;
            shutdown_game();

            show_error_window("Error al cargar efecto de sonido", "Error al cargar el efecto de sonido de muerte: assets/sounds/bone_crushing.wav");
        }
    }
#endif
    
    return success;
}


bool draw_text(SDL_Texture** texture, SDL_Rect *rect, const char* text, SDL_Color text_color, TTF_Font* fuente)
{
    SDL_Surface* text_surface = NULL;
    
    //Get rid of preexisting texture
    if (*texture != NULL) {
        SDL_DestroyTexture(*texture);
        *texture = NULL;
        rect->w = 0;
        rect->h = 0;
    }
    
    // Si no hay texto
    int w = 0, h = 0;
    TTF_SizeText(fuente, text, &w, &h);
    if (w == 0)
        text = " ";
    
    text_surface = TTF_RenderText_Blended(fuente, text, text_color);
    
    if (text_surface == NULL)
    {
        printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
    } 
    else
    {
        //Create texture from surface pixels
        *texture = SDL_CreateTextureFromSurface(renderer, text_surface);
        if (*texture == NULL)
        {
            printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
        }
        else {
            //Get image dimensions
            rect->w = text_surface->w;
            rect->h = text_surface->h;
        }
        
        //Get rid of old surface
        SDL_FreeSurface(text_surface);
    }
    
    //Render font texture to screen
    SDL_RenderCopy(renderer, *texture, NULL, rect);
    
    //Return success
    return *texture != NULL;
}



void updateBranches(int seed)
{
    // Move all the branches down one place
    for (int j = NUM_BRANCHES - 1; j > 0; j--)
    {
        branchPositions[j] = branchPositions[j - 1];
    }
    
    // Spawn ane branch at position 0
    // LEFT, RIGHT or NONE
    srand((int)time(0) + seed);
    int r = (rand() % 5);
    
    switch (r)
    {
        case 0:
        branchPositions[0] = LEFT;
        break;
        
        case 1:
        branchPositions[0] = RIGTH;
        break;
        
        default:
        branchPositions[0] = NONE;
        break;
    }
}

// NOTE: Esto se puede mejorar
void set_message(const char* text)
{
    int i = 0;
    for (; text[i] != '\0' && i < MSG_MAX_LEN; i++)
        message_text[i] = text[i];
    
    message_text[i] = '\0';
}


Sprite load_sprite (const char* path, SDL_Renderer *renderer)
{
    Sprite sprite;
    
    sprite.texture = IMG_LoadTexture(renderer, path);
    
    if (!sprite.texture)
    {
        printf("Unable to create texture from %s! SDL Error: %s\n", path, SDL_GetError());
        exit(EXIT_FAILURE);
    }
    
    sprite.rect.x = 0;
    sprite.rect.y = 0;
    
    SDL_QueryTexture(sprite.texture, NULL, NULL, &sprite.rect.w, &sprite.rect.h);
    
    return sprite;
}


void render_sprite(Sprite* sprite)
{
    if (sprite->texture != NULL)
    {
        // Renderizar como una imagen
        //SDL_RenderCopyEx();
        SDL_RenderCopy(renderer, sprite->texture, NULL, &sprite->rect);
        
        // Dibuja el contorno del sprite para depurar
#ifdef _DEBUG
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xE6, 0x03, 0xFF);
        SDL_RenderDrawRect(renderer, &sprite->rect);
#endif
        
    }
    else {
        // Renderizar como un rectangulo con color solido
        SDL_SetRenderDrawColor(renderer, sprite->color.r, sprite->color.g, sprite->color.b, sprite->color.a);
        SDL_RenderFillRect(renderer, &sprite->rect);
    }
}


void shutdown_game()
{
    // Termina SDL_Mix
    Mix_CloseAudio();
    Mix_Quit();
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow (window);
    SDL_Quit();
}


void show_error_window(const char* titulo, const char* msg)
{
    if (SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, titulo, msg, NULL))
    {
        fprintf(stderr, "Error al mostrar la ventana de error...\n");
        fprintf(stderr, "%s\n", SDL_GetError());
        fprintf(stderr, "Error: %s\n", msg);
    }
    
#ifdef _DEBUG
    ASSERT(false);
#else
    exit(EXIT_FAILURE);
#endif
}

bool load_player_animations(Sprite *player_sprite)
{
    // NOTE: Seria mas facil si las animacion estubiesen en un sprite sheet
    
    char path[512];
    
    for (int i = 1; i  <= IDLE_FRAMES; i++)
    {
        snprintf(path, 512, GRAPHICS_PATH "character/%s/%c%d.png", "idle", 'i', i);
        player_animations[i - 1] = IMG_LoadTexture(renderer, path);
        
        if (!player_animations[i - 1])
        {
            printf("Unable to load texture from %s! SDL Error: %s\n", path, SDL_GetError());
            ASSERT(false);
            exit(EXIT_FAILURE);
        }
    }
    
    for (int i = 1; i <= ATTACK_FRAMES; i++)
    {
        snprintf(path, 512, GRAPHICS_PATH "character/%s/%c%d.png", "attack", 'a', i);
        player_animations[(i- 1) + IDLE_FRAMES] = IMG_LoadTexture(renderer, path);
        
        if (!player_animations[(i- 1) + IDLE_FRAMES])
        {
            printf("Unable to load texture from %s! SDL Error: %s\n", path, SDL_GetError());
            ASSERT(false);
            
            exit(EXIT_FAILURE);
        }
    }
    
    return true;
}


// Animacion del jugador
// TODO: Deberia limpiar este codigo
void draw_player(float dt, enum Player_State *anim_state)
{
    // NOTE: Esto no deberia ser static
    static float passed_time = 0.0f;
    static int animation_frame = 0;
    static int idle_animation_frame = 0;
    static int attacking_animation_frame = IDLE_FRAMES;
    
    float idle_animation_time = .5f;
    float attacking_animation_time = .2f;
    float frame_time = 0; 
    
    
    passed_time += dt;
    
    
    if (*anim_state == IDLE)
    {
        attacking_animation_frame = IDLE_FRAMES;
        
        frame_time = idle_animation_time / IDLE_FRAMES;
        
        if (passed_time >= frame_time)
        {
            idle_animation_frame = (idle_animation_frame + 1) % IDLE_FRAMES;
            passed_time = 0.0f;
            animation_frame = idle_animation_frame;
        }
    }
    
    
    if (*anim_state == ATTACKING)
    {
        if (restart_anim)
        {
            attacking_animation_frame = IDLE_FRAMES;
            
            restart_anim = false;
        }
        
        frame_time = attacking_animation_time / ATTACK_FRAMES;
        
        if (passed_time >= frame_time)
        {
            if (attacking_animation_frame < TOTAL_FRAMES)
            {
                animation_frame = attacking_animation_frame++;
                passed_time = 0.0f;
                
            } else
            {
                attacking_animation_frame = IDLE_FRAMES;
                //player_state = IDLE;
                *anim_state = IDLE;
                animation_frame = 0;
                idle_animation_frame = 0;
            }
        }
        
    }
    
    if (player_side == RIGTH)
    {
        SDL_RenderCopyEx(renderer, player_animations[animation_frame], &player_clip, &player.rect, 0.0f, NULL, SDL_FLIP_HORIZONTAL);
    }
    
    if (player_side == LEFT)
    {
        SDL_RenderCopy(renderer, player_animations[animation_frame], &player_clip, &player.rect);
    }
}
