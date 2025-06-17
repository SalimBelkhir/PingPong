#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#define screenWidth 640 
#define screenHeight 480
#define MAX_LINE 20 
#define MAX_COL 20 

typedef struct Player{
    Vector2 position ;
    Vector2 size ;
    int life ;
}Player;

typedef struct Ball {
    Vector2 position ;
    Vector2 speed ;
    int radius ;
    bool active ;
}Ball;

typedef struct Brick{
    Rectangle rect ;
    bool active ;
}Brick;

typedef enum GameScreen{
    MENU,
    LEVEL_SELECT,
    SOUND_CONFIG,
    GAMEPLAY
}GameScreen;

static GameScreen currentscreen = MENU ;
static Brick bricks[MAX_LINE][MAX_COL] ;
static int brickCount = 0 ;
static int selectedLevel = 2 ;
static bool SoundOn = true ;

static int lifeCount = 3 ;
static bool pause = false ;
static bool GameOver = false ;

static Player player1 = {0} ;
static Player player2 = {0} ;
static Ball ball = {0} ;

static Music sound ;
static Music menuSound ;
static Sound gameover ;


//prototype

static void InitGame(void);         // Initialize game
static void UpdateGame(void);       // Update game (one frame)
static void DrawGame(void);         // Draw game (one frame)
static void UnloadGame(void);       // Unload game
static void UpdateDrawFrame(void);  // Update and Draw (one frame)
 // Initialize audio device
void DrawMenu(void);
void DrawLevelSelect(void);
void DrawSoundConfig(void);

int main(void){
    InitAudioDevice();
    Sound winSfx = LoadSound("./cute-level-up-3-189853.mp3") ;
    Music MainMenuMusic = LoadMusicStream("./retro-wave-style-track-59892.mp3");
    Music PlayMusic = LoadMusicStream("./beep-boop-64194.mp3");
    Sound shot = LoadSound("./retro-game-shot-152052.mp3");

    sound = PlayMusic ;
    menuSound = MainMenuMusic;
    gameover = winSfx ;

    InitWindow(screenWidth,screenHeight,"pingpong");
    InitGame();

    #if defined(PLATFORM_WEB)
        emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
    #else
        SetTargetFPS(60); 
        while (!WindowShouldClose()) 
        {
            UpdateDrawFrame();
        }
    #endif

    UnloadGame();
    CloseWindow();
    return 0 ;
}

//Module Functions Definitions (local)

void InitBricks(int level){
    int j ;
    int mid = 4 ;
    
    switch(level) {
        case 1 : 
            for(int i = 0 ;i<=8 ;i++){
                for(j=0 ; j<=8 ; j++){
                    bricks[i][j].active=false ;
                }
            }
            break ;
        case 2 :
            for(int i = 0 ;i<=8 ;i++){
                for(j=0 ; j<=8 ; j++){
                    bricks[i][j].active=false ;
                }
            }
            for(int i = 0 ; i<6 ; i++){
                j = ((i+1)%2) ? 1 : 2 ;
                bricks[i][j].rect = (Rectangle){
                screenWidth/2 -100 + 45*j , 30*i +40 +screenHeight/4,30,15};
                bricks[i][j].active = true ;                
            }
            break ;
        case 3 :
            for(int i = 0 ;i<8 ;i++){
                for(j=0 ; j<8 ; j++){
                    bricks[i][j].active=false ;
                }
            }
            for(int i=0;i<=mid ; i++){
                bricks[i][mid-i].rect = (Rectangle){
                screenWidth/2 -200 + 45*(mid-i) , 30*i +40 +screenHeight/4,30,15};
                bricks[i][mid-i].active = true ;  
                bricks[i][mid+i].rect = (Rectangle){
                screenWidth/2 -200 + 45*(mid+i) , 30*i +40 +screenHeight/4,30,15};
                bricks[i][mid+i].active = true ;  
            }
            for(int i=mid+1 ; i<=2*mid;i++){
                bricks[i][mid -(2*mid-i)].rect = (Rectangle){
                screenWidth/2 -200 + 45*(mid -(2*mid-i) ), 30*i +40 +screenHeight/4,30,15};
                bricks[i][mid -(2*mid-i)].active = true ; 
                
                bricks[i][mid +(2*mid-i)].rect = (Rectangle){
                screenWidth/2 -200 + 45*(mid +(2*mid-i)) , 30*i +40 +screenHeight/4,30,15};
                bricks[i][mid +(2*mid-i)].active = true ;
            }

        break;
        case 4 :
            for(int i = 0 ;i<=8 ;i++){
                for(j=0 ; j<=8 ; j++){
                    bricks[i][j].active=false ;
                }
            }
            for(int i = 0 ;i<=2*mid ; i++){
                bricks[i][0].rect =(Rectangle){screenWidth/2 -200 +45*i , (30*i +40 +screenHeight/4),30,15};
                bricks[i][0].active = true ;
            }
            break;
        case 5 :
            for(int i = 0 ;i<=8 ;i++){
                for(j=0 ; j<=8 ; j++){
                    bricks[i][j].active=false ;
                }
            }
            for(int i = 0 ;i<=2*mid +2 ; i++){
                bricks[0][i].rect =(Rectangle){screenWidth/2 -25 , (45*i -70 +screenHeight/4),30,15};
                bricks[0][i].active = true ;
            }
            break;
            
    }
    
    

}

static void InitGame(void){
    PlayMusicStream(menuSound);

    player1.position = (Vector2){ screenWidth/8 , screenHeight/2} ;
    player1.size = (Vector2){20,100} ;
    player1.life = lifeCount ;

    player2.position = (Vector2){screenWidth*7/8 , screenHeight/2} ;
    player2.size = (Vector2){20,100} ;
    player2.life = lifeCount ;

    ball.position = (Vector2){player1.position.x + player1.size.x/2 + ball.radius,player1.position.y} ;
    ball.speed = (Vector2){0,0} ;
    ball.radius = 10;
    ball.active = false;  
    
    InitBricks(selectedLevel);
  
}

void UpdateGame(void){
    
    UpdateMusicStream(sound);
    if(!GameOver){ 
        
        if (IsKeyPressed(KEY_P)) pause = !pause ;
        
        if(!pause){
            
            if(IsKeyDown(KEY_S)) player1.position.y += 5 ;
            if((IsKeyDown(KEY_W))) player1.position.y -= 5 ;
            if((player1.position.y + player1.size.y/2) >= screenHeight) player1.position.y = screenHeight - player1.size.y/2 ;
            if((player1.position.y - player1.size.y/2) <= 0) player1.position.y = player1.size.y/2 ;

            if(IsKeyDown(KEY_DOWN)) player2.position.y += 5 ;
            if(IsKeyDown(KEY_UP)) player2.position.y -= 5 ;
            if((player2.position.y + player2.size.y/2)>= screenHeight) player2.position.y = screenHeight - player2.size.y/2 ;
            if((player2.position.y - player2.size.y/2) <= 0) player2.position.y = player2.size.y/2 ;

            if(!ball.active){
                if (ball.speed.x >= 0) {
                // Ball will go right, attach to player 1
                ball.position = (Vector2){player1.position.x + player1.size.x/2 + ball.radius, player1.position.y};
                } else {
                // Ball will go left, attach to player 2
                ball.position = (Vector2){player2.position.x - player2.size.x/2 - ball.radius, player2.position.y};
                }
                if(IsKeyPressed(KEY_SPACE)){
                    ball.active = true ;
                    ball.speed = (Vector2){7,1};
                }
            }

            if(ball.active){
                ball.position.x += ball.speed.x;
                ball.position.y += ball.speed.y;
            } else {
                //ball.position = (Vector2){ player1.position.x + player1.size.x/2 + ball.radius, player1.position.y };    
            }

            if(ball.position.y + ball.radius >= screenHeight || ball.position.y - ball.radius <= 0){
                 ball.speed.y *= -1;
            }

            if(ball.position.x +ball.radius >= screenWidth ){
                player2.life--;
                ball.active =false ;
                PlaySound(gameover);
                ball.position = (Vector2){player1.position.x + player1.size.x/2 + ball.radius,player1.position.y};
                
            }
            if(ball.position.x -ball.radius <= 0){
                player1.life--;
                ball.active = false ;
                PlaySound(gameover);
                ball.position = (Vector2){player2.position.x - player2.size.x/2 - ball.radius,player2.position.y};
                
            }

            if(IsKeyPressed(KEY_A)){
               currentscreen = MENU ;
               StopMusicStream(sound);
               GameOver = false ;
               ball.active = false ;
               return ;
            } 
            
            if (CheckCollisionCircleRec(ball.position, ball.radius,
            (Rectangle){player1.position.x - player1.size.x/2, player1.position.y - player1.size.y/2, player1.size.x, player1.size.y})) {
                if (ball.speed.x < 0) { // Ball moving left
                    ball.speed.x *= -1;
                    float offset = (ball.position.y - player1.position.y) / (player1.size.y / 2);
                    ball.speed.y = 5 * offset;
                }
            }
        
            else if (CheckCollisionCircleRec(ball.position, ball.radius,
            (Rectangle){player2.position.x - player2.size.x/2, player2.position.y - player2.size.y/2, player2.size.x, player2.size.y})) {
                if (ball.speed.x > 0) { 
                ball.speed.x *= -1;
                float offset = (ball.position.y - player2.position.y) / (player2.size.y / 2);
                ball.speed.y = 5 * offset;
            }
        }    
            for (int i = 0; i < MAX_LINE; i++) {
                for(int j = 0; j < MAX_COL; j++) {
                    //check collision with bricks
                    if (bricks[i][j].active &&
                        CheckCollisionCircleRec(ball.position, ball.radius, bricks[i][j].rect)) {
                    //printf("Collision with brick at (%d, %d)\n", i, j);
                    bricks[i][j].active = false;
                    ball.speed.x *= -1;
                    Vector2 bricks_pos = (Vector2){bricks[i][j].rect.x,bricks[i][j].rect.y} ;
                    float off = (ball.position.y - bricks_pos.y)/(bricks_pos.y /2);
                    ball.speed.y = 5*off ;
                    }
                }
            }

            unsigned int t = (unsigned int)(GetTime()*1000) ;
            const float dir = (((int)t%2000) >= 1000) ? 1.0f : -1.0f ;

            if(selectedLevel == 4){
                for(int i = 0 ; i<=8;i++){
                    float scale = sinf((t/1000.0f + i*0.1f) * 3.14159f); // -1 to 1
                    bricks[i][0].rect.y =200*scale +screenWidth/4 + 70;
                    
                }
            }
            if(selectedLevel == 5){
                for (int i=0;i<=10;i++){
                    float s = sinf((t/1000.0f +i*0.1f)*PI);
                    if((i%2)==0){
                        bricks[0][i].rect.x = (200)*s + screenWidth/2 ;
                    }
                    else{
                        bricks[0][i].rect.x = -(200)*s + screenWidth/2;
                    }
                }
            }
        
        UpdateMusicStream(menuSound);
        
        // Check for game over
        if(player1.life <= 0 || player2.life <= 0){
            GameOver = true;
        }
        }
    }else {
        if(IsKeyPressed(KEY_ENTER)){
                InitGame();
                GameOver = false;
                ball.active=true ;
                ball.speed = (Vector2){5,1} ;
        }
        if (IsKeyPressed(KEY_A)) currentscreen = MENU ;
        //Play Sound 
        PlaySound(gameover) ;


    }
}

void DrawGame(void){
    BeginDrawing();
    ClearBackground(RAYWHITE);

    if(!GameOver){
        DrawRectangle(player1.position.x - player1.size.x/2, player1.position.y - player1.size.y/2,player1.size.x,player1.size.y,BLACK);
        DrawRectangle(player2.position.x - player2.size.x/2 , player2.position.y - player2.size.y/2 , player2.size.x ,player2.size.y,BLACK);
        DrawCircleV(ball.position, ball.radius,BLACK) ;
        for(int i =0 ; i < MAX_LINE;i++){
            for(int j=0 ; j<MAX_COL ; j++){
                if(bricks[i][j].active){
                DrawRectangleRec(bricks[i][j].rect,DARKGRAY) ; 
            }
            }
        }
        for (int i = 0; i < player1.life; i++) DrawRectangle(20 + 40*i, screenHeight - 30, 35, 10, LIGHTGRAY);
        for (int i = 0; i < player2.life; i++) DrawRectangle(screenWidth - 40*i -60 , screenHeight - 30, 35, 10, LIGHTGRAY);

        if(pause){
            DrawText("PAUSED",screenWidth/2 - MeasureText("PAUSED",20)/2,screenHeight/2 - 20,20,GRAY);
        }else {
            //...
        }       


    } else {
        DrawText("GAME OVER", screenWidth/2 - MeasureText("GAME OVER", 40)/2, screenHeight/2 - 40, 40, GRAY);
        if(player1.life<=0){
            DrawText("Player 2 wins!",screenWidth/2 - MeasureText("Player 2 wins!",20)/2,screenHeight/2+100 ,20,GRAY);
        } else {
            DrawText("Player 1 wins!", screenWidth/2 - MeasureText("Player 1 wins!", 20)/2, screenHeight/2 + 100, 20, GRAY);
        }
        DrawText("Press ENTER to restart", screenWidth/2 - MeasureText("Press ENTER to restart", 20)/2, screenHeight/2 + 10, 20, GRAY);
    }
    EndDrawing();
}

void DrawMenu(void) {
    
    BeginDrawing() ;
    ClearBackground(RAYWHITE);
    DrawText("PingPong", screenWidth/2-MeasureText("PingPong",40)/2, 80, 40, BLACK);
    StopMusicStream(sound);
    if (!IsMusicStreamPlaying(menuSound)) PlayMusicStream(menuSound);
    UpdateMusicStream(menuSound);
    //Rectangle playBtn = {screenWidth/2 -100 , 180 , 200 ,40};
    Rectangle levelBtn = {screenWidth/2-100, 180, 200, 40};
    Rectangle soundBtn = {screenWidth/2-100, 240, 200, 40};

    //DrawRectangleRec(playBtn, LIGHTGRAY);
    //DrawText("Play", playBtn.x + 70, playBtn.y + 10, 20, BLACK);

    DrawRectangleRec(levelBtn, LIGHTGRAY);
    DrawText("Select Level", levelBtn.x + 35, levelBtn.y + 10, 20, BLACK);

    DrawRectangleRec(soundBtn, LIGHTGRAY);
    DrawText("Sound Config", soundBtn.x + 30, soundBtn.y + 10, 20, BLACK);

    if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mouse = GetMousePosition();
        if(CheckCollisionPointRec(mouse,levelBtn)){
            currentscreen = LEVEL_SELECT ;
            StopMusicStream(sound);
    if (!IsMusicStreamPlaying(menuSound)) PlayMusicStream(menuSound);
    UpdateMusicStream(menuSound);
        }else if(CheckCollisionPointRec(mouse,soundBtn)){
            currentscreen = SOUND_CONFIG ;
        }
    }
    DrawText("Press P to pause",10,screenHeight-70,20,GRAY);
    DrawText("Press A to return to main menu",10,screenHeight-50,20,GRAY);
    DrawText("Press Esc to quite",10,screenHeight-30,20,GRAY);
    EndDrawing();
}

void DrawLevelSelect(void){
    if (!IsMusicStreamPlaying(menuSound)) PlayMusicStream(menuSound);
    UpdateMusicStream(menuSound);
    StopMusicStream(sound); 
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawText("Select Level",screenWidth/2 -MeasureText("Select Level",30)/2 , 80,30 , BLACK);
    for (int i = 1; i <= 5; i++) {
        Rectangle btn = {screenWidth/2-100, 120 + i*50, 200, 40};
        DrawRectangleRec(btn, (selectedLevel == i) ? ORANGE : LIGHTGRAY);
        char label[16];
        sprintf(label, "Level %d", i);
        DrawText(label, btn.x + 70, btn.y + 10, 20, BLACK);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mouse = GetMousePosition();
            if (CheckCollisionPointRec(mouse, btn)) {
                selectedLevel = i;
                InitBricks(selectedLevel);
                currentscreen = GAMEPLAY;
                    StopMusicStream(menuSound); // Stop menu music
                    PlayMusicStream(sound);
            }
        }
    }
  
    if(IsKeyPressed(KEY_A)){
        currentscreen = MENU ;
        StopMusicStream(sound);
    }
    EndDrawing();
}


void DrawSoundConfig(void) {
    if (!IsMusicStreamPlaying(menuSound)) PlayMusicStream(menuSound);
    UpdateMusicStream(menuSound);
    StopMusicStream(sound); 

    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawText("Sound Configuration", screenWidth/2-MeasureText("Sound Configuration",30)/2, 80, 30, BLACK);
    Rectangle soundBtn = {screenWidth/2-100, 200, 200, 40};
    DrawRectangleRec(soundBtn, LIGHTGRAY);
    DrawText(SoundOn ? "Sound: ON" : "Sound: OFF", soundBtn.x + 50, soundBtn.y + 10, 20, BLACK);
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mouse = GetMousePosition();
        if (CheckCollisionPointRec(mouse, soundBtn)) {
            SoundOn = !SoundOn;
        }
    }
 
    if(IsKeyPressed(KEY_A)){
        currentscreen = MENU ;
        StopMusicStream(sound);
    }
    EndDrawing();
}


void UnloadGame(void){
    // Unload game resources here if needed
    UnloadMusicStream(sound) ;
    UnloadMusicStream(menuSound);
    CloseAudioDevice();
}

void UpdateDrawFrame(void){
    switch (currentscreen) {
        case MENU: DrawMenu(); break;
        case LEVEL_SELECT: DrawLevelSelect(); break;
        case SOUND_CONFIG: DrawSoundConfig(); break;
        case GAMEPLAY:
            UpdateGame();
            DrawGame();
            break;
    }
}




