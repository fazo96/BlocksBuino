#include <SPI.h>
#include <Gamebuino.h>

Gamebuino gb = Gamebuino();
extern const byte PROGMEM logo[];

#define NOT_INIT 0
#define PLAYING 1
#define GAME_OVER 2
#define MENU 3
byte gameStatus = 0;

const byte game_force_level = 0;
byte game_level = 1; // 1,2,3,4,5,6,7,8,9 Levels
byte game_menu_level = game_level;
#define GAME_LEVEL_MAX 9
const byte PROGMEM game_levels[GAME_LEVEL_MAX] = { 800, 750, 700, 650, 600, 500, 400, 300, 200 };
byte game_score = 0;
byte game_lines = 0;
unsigned long game_delai = game_levels[game_level];
unsigned long game_prevTime = 0;
//END game variables

//animation variables
unsigned short game_animation_default_counter = 60;
short game_animation_counter = game_animation_default_counter;
short game_animation_color = 0;
const unsigned int game_animation_delai = 1000;
unsigned long game_animation_delai_prevTime = millis();
//END animation variables

//blocks variables
#define BLOCKS_MAX_Y 16
#define BLOCKS_MAX_X 10
const short block_w = 4;
const short block_draw_w = block_w - 1;
const short block_h = 4;
const short block_draw_h = block_h - 1;

boolean blocks_activation[BLOCKS_MAX_Y][BLOCKS_MAX_X];
//END blocks variables

//player variables
boolean player_new_blocks = true;
short player_nb_lines_completions = 0;
boolean lines_completions[BLOCKS_MAX_Y];
enum type_blocks {
  BLOCKS_LINE = 1,
  BLOCKS_CUBE = 2,
  BLOCKS_T = 3,
  BLOCKS_L = 4,
  BLOCKS_L_REVERT = 5,
  BLOCKS_S = 6,
  BLOCKS_S_REVERT = 7
};
short player_blocks_type = BLOCKS_LINE;
short player_blocks_current_type = BLOCKS_LINE;
short player_blocks_rotation = 1; //1,2,3,4
short player_blocks[4][2]; //rotation point y,x
//END player variables

//field variables 84x48
const int field_x = 26; // 16
const int field_y = LCDHEIGHT + 1;
const int field_w = 32;// 10 BLOCKS MAX // 42
const int field_h = -(LCDHEIGHT) - 1;// - 4);// 12 BLOCKS MAX
//END field variables

void setup(){
  gb.begin();
  gb.pickRandomSeed();
  gb.titleScreen(logo);
  gb.battery.show = false;
}

//the loop routine runs over and over again forever
void loop(){
  if (gameStatus == NOT_INIT) {
    initGame();
    gameStatus = PLAYING;
  }
  if(gb.update()){
    if (gameStatus == PLAYING) {
      if(gb.buttons.pressed(BTN_C)){
        gb.sound.playCancel();
        gameStatus = MENU;
      }
      Play();
    }
    else if (gameStatus == MENU) {
      GameMenu();
    }
    else if (gameStatus == GAME_OVER) {
      GameOver();
    }
  }
}

void initGame() {
  player_new_blocks = true;

  int i = 0;
  int j = 0;
  while (i < BLOCKS_MAX_Y) {
    j = 0;
    while (j < BLOCKS_MAX_X) {
      blocks_activation[i][j] = false;
      j++;
    }
    i++;
  }

  for (i =0; i < BLOCKS_MAX_Y;i++) {
    lines_completions[i] = false;
  }
  player_nb_lines_completions = 0;

  player_blocks_type = random(1, 8);
  player_blocks_rotation = 1;

  game_lines = 0;
  game_score = 0;
  if (game_force_level > 0) {
    game_level = game_force_level;
  }
  else if (game_menu_level != 1) {
    game_level = game_menu_level;
  }
  else {
    game_level = 1;
  }
  game_menu_level = game_level;
  game_delai = game_levels[game_level - 1];
}

void Play() {
  if (player_nb_lines_completions > 0) {
    unsigned long game_animation_currentTime = millis() - game_animation_delai_prevTime;
    if (game_animation_currentTime >= game_animation_delai) {
      //END of animation
      UpdateBlocks();//remove completed blocks

      DrawScore();
      DrawField();
      DrawBlocks();

      //update animation prevtime
      game_animation_delai_prevTime = millis();
      game_animation_counter = game_animation_default_counter;
    }
    else {
      //display animation
      DrawScore();
      DrawField();
      DrawAnimationBlocks();
    }
  }
  else {
    //GAME
    MovePlayerBlocks();

    DrawScore();
    DrawField();
    DrawPlayerBlocks();
    DrawBlocks();

    //update animation prevtime
    game_animation_delai_prevTime = millis();
  }

  if (gameStatus == GAME_OVER) {
    PlaySoundFxGameOver();
  }
}

// Show main menu
void GameMenu() {
  gb.display.cursorX = 5;
  gb.display.cursorY = 1;
  gb.display.print(F("-CHOOSE GAME LEVEL-"));

  gb.display.fillTriangle(30, 10, 25, 15, 35, 15);
  gb.display.fillTriangle(30, 28, 25, 23, 35, 23);

  gb.display.cursorX = 0;
  gb.display.cursorY = 17;
  gb.display.print("LEVEL: " + String(game_menu_level));

  gb.display.cursorX = 0;
  gb.display.cursorY = 40;
  gb.display.print(F("\x15:accept \x16:cancel"));

  if(gb.buttons.pressed(BTN_UP)){
    if ((game_menu_level + 1) <= GAME_LEVEL_MAX) {
      game_menu_level++;
    }
  }
  if(gb.buttons.pressed(BTN_DOWN)){
    if ((game_menu_level - 1) >= 1) {
      game_menu_level--;
    }
  }
  if(gb.buttons.pressed(BTN_A)){
    gb.sound.playOK();

    game_level = game_menu_level;
    initGame();
    gameStatus = PLAYING;
  }
  if(gb.buttons.pressed(BTN_B)){
    gb.sound.playOK();

    game_menu_level = game_level;
    gameStatus = PLAYING;
  }
  if(gb.buttons.pressed(BTN_C)){
    gb.sound.playOK();
    gb.titleScreen(logo);
    gb.battery.show = false;
  }
}

void GameOver() {
  gb.display.cursorX = 22;
  gb.display.cursorY = 1;
  gb.display.print(F("!GAME OVER!"));

  gb.display.cursorX = 0;
  gb.display.cursorY = 10;
  gb.display.print("Level: [" + String(game_level) + "]");

  gb.display.cursorX = 0;
  gb.display.cursorY = 20;
  gb.display.print("Lines: "+String(game_lines));

  gb.display.cursorX = 0;
  gb.display.cursorY = 30;
  gb.display.print("Score: "+String(game_score));

  gb.display.cursorX = 0;
  gb.display.cursorY = 40;
  gb.display.print(F("\x16:accept"));

  initGame();

  if(gb.buttons.pressed(BTN_B) || gb.buttons.pressed(BTN_A)){
    gb.sound.playOK();
    gameStatus = MENU;
  } else if(gb.buttons.pressed(BTN_C)){
    gameStatus = MENU;
    gb.titleScreen(logo);
    gb.battery.show = false;
  }
}

void MovePlayerBlocks() {
  boolean action = false;
  if (gb.buttons.repeat(BTN_RIGHT, 3)) {
    if (!CheckBlocksCollision(1, 0)) {
      MoveXBlocks(1);
      action = true;
    }
  }
  if (gb.buttons.repeat(BTN_LEFT, 3)) {
    if (!CheckBlocksCollision(-1, 0)) {
      MoveXBlocks(-1);
      action = true;
    }
  }
  if (gb.buttons.repeat(BTN_DOWN, 1)) {
    if (player_blocks[0][0] > 0 && !CheckBlocksCollision(0, -1)) {
      MoveYBlocks(-1);
      action = true;
    }
  } else if(gb.buttons.repeat(BTN_UP,10)){
    while(player_blocks[0][0] > 0 && !CheckBlocksCollision(0, -1)) {
      MoveYBlocks(-1);
      action = true;
    }
  }
  if (gb.buttons.pressed(BTN_A)) {
    RotateBlocks();
  }
  unsigned long game_currentTime = millis();
  if (!action && (game_currentTime - game_prevTime) >= game_delai) {
    //check collision
    if (!player_new_blocks && CheckBlocksCollision(0, -1)) {
      if (player_blocks[0][0] >= (BLOCKS_MAX_Y - 1)) {
        gameStatus = GAME_OVER;//END of GAME
      }
      else {
        PlaySoundFxPieceDrop();

        blocks_activation[player_blocks[0][0]][player_blocks[0][1]] = true;
        blocks_activation[player_blocks[1][0]][player_blocks[1][1]] = true;
        blocks_activation[player_blocks[2][0]][player_blocks[2][1]] = true;
        blocks_activation[player_blocks[3][0]][player_blocks[3][1]] = true;
        player_new_blocks = true;

        CheckLinesCompletion();
      }
    }
    else {
      MoveYBlocks(-1);
    }

    //update prevtime
    game_prevTime = game_currentTime;
  }

  //launch new player block
  if (player_nb_lines_completions == 0 && player_new_blocks) {
    NewPlayerBlocks();

    player_new_blocks = false;
  }
}

void MoveYBlocks(int value) {
  player_blocks[0][0] += value;
  player_blocks[1][0] += value;
  player_blocks[2][0] += value;
  player_blocks[3][0] += value;
}

void MoveXBlocks(int value) {
  player_blocks[0][1] += value;
  player_blocks[1][1] += value;
  player_blocks[2][1] += value;
  player_blocks[3][1] += value;
}

void RotateBlocks() {
  int current_rotation = ((player_blocks_rotation + 1) > 4) ? 1 : (player_blocks_rotation + 1);

  switch(player_blocks_current_type) {
    case BLOCKS_LINE:
      BlocksRotation_Type1(current_rotation);//line
      break;
    case BLOCKS_T:
      BlocksRotation_Type3(current_rotation);//T
      break;
    case BLOCKS_L:
      BlocksRotation_Type4(current_rotation);//L
      break;
    case BLOCKS_L_REVERT:
      BlocksRotation_Type5(current_rotation);//L(revert)
      break;
    case BLOCKS_S:
      BlocksRotation_Type6(current_rotation);//S
      break;
    case BLOCKS_S_REVERT:
      BlocksRotation_Type7(current_rotation);//S(revert)
      break;
  }
}

void NewPlayerBlocks() {
  player_blocks_rotation = 1;//reset to initial value

  switch(player_blocks_type) {
  case BLOCKS_LINE:
    //line
    player_blocks[0][0] = BLOCKS_MAX_Y + 2;//axe
    player_blocks[0][1] = BLOCKS_MAX_X / 2;//axe

    player_blocks[1][0] = BLOCKS_MAX_Y + 3;
    player_blocks[1][1] = BLOCKS_MAX_X / 2;

    player_blocks[2][0] = BLOCKS_MAX_Y + 1;
    player_blocks[2][1] = BLOCKS_MAX_X / 2;

    player_blocks[3][0] = BLOCKS_MAX_Y;
    player_blocks[3][1] = BLOCKS_MAX_X / 2;
    break;
  case BLOCKS_CUBE:
    //cube
    player_blocks[0][0] = BLOCKS_MAX_Y;//axe
    player_blocks[0][1] = (BLOCKS_MAX_X / 2) - 1;//axe
    player_blocks[1][0] = BLOCKS_MAX_Y;
    player_blocks[1][1] = (BLOCKS_MAX_X / 2);
    player_blocks[2][0] = BLOCKS_MAX_Y + 1;
    player_blocks[2][1] = (BLOCKS_MAX_X / 2) - 1;
    player_blocks[3][0] = BLOCKS_MAX_Y + 1;
    player_blocks[3][1] = (BLOCKS_MAX_X / 2);
    break;
  case BLOCKS_T:
    //T
    player_blocks[0][0] = BLOCKS_MAX_Y;//axe
    player_blocks[0][1] = BLOCKS_MAX_X / 2;//axe
    player_blocks[1][0] = BLOCKS_MAX_Y;
    player_blocks[1][1] = (BLOCKS_MAX_X / 2) - 1;
    player_blocks[2][0] = BLOCKS_MAX_Y + 1;
    player_blocks[2][1] = BLOCKS_MAX_X / 2;
    player_blocks[3][0] = BLOCKS_MAX_Y - 1;
    player_blocks[3][1] = BLOCKS_MAX_X / 2;
    break;
  case BLOCKS_L:
    //L
    player_blocks[0][0] = BLOCKS_MAX_Y + 1;//axe
    player_blocks[0][1] = (BLOCKS_MAX_X / 2);//axe
    player_blocks[1][0] = BLOCKS_MAX_Y;
    player_blocks[1][1] = (BLOCKS_MAX_X / 2);
    player_blocks[2][0] = BLOCKS_MAX_Y + 2;
    player_blocks[2][1] = (BLOCKS_MAX_X / 2);
    player_blocks[3][0] = BLOCKS_MAX_Y + 2;
    player_blocks[3][1] = (BLOCKS_MAX_X / 2) - 1;
    break;
  case BLOCKS_L_REVERT:
    //L(revert)
    player_blocks[0][0] = BLOCKS_MAX_Y + 1;//axe
    player_blocks[0][1] = BLOCKS_MAX_X / 2;//axe
    player_blocks[1][0] = BLOCKS_MAX_Y;
    player_blocks[1][1] = BLOCKS_MAX_X / 2;
    player_blocks[2][0] = BLOCKS_MAX_Y + 2;
    player_blocks[2][1] = BLOCKS_MAX_X / 2;
    player_blocks[3][0] = BLOCKS_MAX_Y + 2;
    player_blocks[3][1] = (BLOCKS_MAX_X / 2) + 1;
    break;
  case BLOCKS_S:
    //S
    player_blocks[0][0] = BLOCKS_MAX_Y;//axe
    player_blocks[0][1] = (BLOCKS_MAX_X / 2) - 1;//axe
    player_blocks[1][0] = BLOCKS_MAX_Y + 1;
    player_blocks[1][1] = (BLOCKS_MAX_X / 2) - 1;
    player_blocks[2][0] = BLOCKS_MAX_Y;
    player_blocks[2][1] = (BLOCKS_MAX_X / 2);
    player_blocks[3][0] = BLOCKS_MAX_Y - 1;
    player_blocks[3][1] = (BLOCKS_MAX_X / 2);
    break;
  case BLOCKS_S_REVERT:
    //S(revert)
    player_blocks[0][0] = BLOCKS_MAX_Y;//axe
    player_blocks[0][1] = BLOCKS_MAX_X / 2;//axe
    player_blocks[1][0] = BLOCKS_MAX_Y + 1;
    player_blocks[1][1] = BLOCKS_MAX_X / 2;
    player_blocks[2][0] = BLOCKS_MAX_Y;
    player_blocks[2][1] = (BLOCKS_MAX_X / 2) - 1;
    player_blocks[3][0] = BLOCKS_MAX_Y - 1;
    player_blocks[3][1] = (BLOCKS_MAX_X / 2) - 1;
    break;
  default:
    //cube
    player_blocks[0][0] = BLOCKS_MAX_Y;//axe
    player_blocks[0][1] = BLOCKS_MAX_X / 2;//axe
    player_blocks[1][0] = BLOCKS_MAX_Y;
    player_blocks[1][1] = (BLOCKS_MAX_X / 2) + 1;
    player_blocks[2][0] = BLOCKS_MAX_Y + 1;
    player_blocks[2][1] = BLOCKS_MAX_X / 2;
    player_blocks[3][0] = BLOCKS_MAX_Y + 1;
    player_blocks[3][1] = (BLOCKS_MAX_X / 2) + 1;
    break;
  }

  player_blocks_current_type = player_blocks_type;
  player_blocks_type = random(1, 8);
}

boolean CheckBlocksCollision(int x_value, int y_value) {
  //check move right collision
  if (x_value > 0 && ((player_blocks[0][1] + x_value) >= BLOCKS_MAX_X || (player_blocks[1][1] + x_value) >= BLOCKS_MAX_X
    || (player_blocks[2][1] + x_value) >= BLOCKS_MAX_X || (player_blocks[3][1] + x_value) >= BLOCKS_MAX_X)) {
    return true;
  }
  //check move left collision
  if (x_value < 0 && ((player_blocks[0][1] + x_value) < 0 || (player_blocks[1][1] + x_value) < 0
    || (player_blocks[2][1] + x_value) < 0 || (player_blocks[3][1] + x_value) < 0)) {
    return true;
  }

  //check move down collision
  if (y_value < 0 && ((player_blocks[0][0] + y_value) < 0 || (player_blocks[1][0] + y_value) < 0
    || (player_blocks[2][0] + y_value) < 0  || (player_blocks[3][0] + y_value) < 0)) {
    return true;
  }

  //check collision with other blocks
  return (player_blocks[0][0] < BLOCKS_MAX_Y && (blocks_activation[player_blocks[0][0] + y_value][player_blocks[0][1] + x_value])) || (player_blocks[1][0] < BLOCKS_MAX_Y && blocks_activation[player_blocks[1][0] + y_value][player_blocks[1][1] + x_value])
    || (player_blocks[2][0] < BLOCKS_MAX_Y && blocks_activation[player_blocks[2][0] + y_value][player_blocks[2][1] + x_value]) || (player_blocks[3][0] < BLOCKS_MAX_Y && blocks_activation[player_blocks[3][0] + y_value][player_blocks[3][1] + x_value]);
}

void UpdateBlocks() {
  int i = 0;
  int j = 0;
  int x = 0;
  int y = 0;
  int new_index = 0;
  boolean increment = false;

  //UPDATE activations blocks
  while (i < BLOCKS_MAX_Y) {
    j = 0;

    increment = false;

    while (j < BLOCKS_MAX_X) {
      if (!lines_completions[i]) {
        blocks_activation[new_index][j] = blocks_activation[i][j];
        increment = true;
      }

      j++;
    }

    if (increment) {
      new_index++;
    }

    i++;
  }

  i = new_index;
  while (i < BLOCKS_MAX_Y) {
    j = 0;
    while (j < BLOCKS_MAX_X) {
      blocks_activation[i][j] = false;
      j++;
    }
    i++;
  }

  //reset
  for (i =0; i < BLOCKS_MAX_Y;i++) {
    lines_completions[i] = false;
  }

  UpdateGameScore();

  player_nb_lines_completions = 0;
}

void UpdateGameScore() {
  game_lines += player_nb_lines_completions;
  game_score += (player_nb_lines_completions > 1) ? (player_nb_lines_completions * (game_level + 1)) : 1;

  if (game_lines >= (game_level * 10)) {
    //increase game_level
    if ((game_level + 1) <= GAME_LEVEL_MAX) {
      game_level++;
    }
    game_delai = game_levels[game_level - 1];
  }
}

void CheckLinesCompletion() {
  int i = 0;
  int j = 0;
  int x = 0;
  int y = 0;
  int completion = false;

  while (i < BLOCKS_MAX_Y) {
    j = 0;

    completion = true;

    while (j < BLOCKS_MAX_X) {
      if (!blocks_activation[i][j]) {
        completion = false;
        break;
      }
      j++;
    }

    if(completion) {
      player_nb_lines_completions++;
    }

    lines_completions[i] = completion;
    i++;
  }

  if (player_nb_lines_completions > 0) {
    PlaySoundFxLineCompleted();
  }
}

int GetXcoordonnee(int x) {
  return (field_x + ((block_draw_w * x) + 1));
}

int GetYcoordonnee(int y) {
  return ((field_y - 1) - (block_draw_h * (y + 1)));
}

void DrawPlayerBlocks() {
  if (player_blocks[0][0] < BLOCKS_MAX_Y) {
    gb.display.drawRect(GetXcoordonnee(player_blocks[0][1]), GetYcoordonnee(player_blocks[0][0]), block_draw_w, block_draw_h);
  }
  if (player_blocks[1][0] < BLOCKS_MAX_Y) {
    gb.display.drawRect(GetXcoordonnee(player_blocks[1][1]), GetYcoordonnee(player_blocks[1][0]), block_draw_w, block_draw_h);
  }
  if (player_blocks[2][0] < BLOCKS_MAX_Y) {
    gb.display.drawRect(GetXcoordonnee(player_blocks[2][1]), GetYcoordonnee(player_blocks[2][0]), block_draw_w, block_draw_h);
  }
  if (player_blocks[3][0] < BLOCKS_MAX_Y) {
    gb.display.drawRect(GetXcoordonnee(player_blocks[3][1]), GetYcoordonnee(player_blocks[3][0]), block_draw_w, block_draw_h);
  }
}

void DrawScore() {
  DrawNextBlocks();

  gb.display.cursorX = 62;
  gb.display.cursorY = 5;
  gb.display.print(String("Lvl.")+""+String(game_level));

  gb.display.cursorX = 62;
  gb.display.cursorY = 15;
  gb.display.print(F("LINES"));
  gb.display.cursorX = 62;
  gb.display.cursorY = 22;
  gb.display.println(GetScoreString(game_lines, 3));
}

String GetScoreString(int score, int sizeBuffer) {
  char buf[sizeBuffer];
  if (sizeBuffer == 3) {
    sprintf(buf, "%03i", score);
  }
  else if (sizeBuffer == 5) {
    sprintf(buf, "%05i", score);
  }
  else {
    return String(0);
  }

  int d1, d2, d3, d4, d5;//5 digits MAX score 99999

  d1 = buf[0] - '0';
  d2 = buf[1] - '0';
  d3 = buf[2] - '0';
  d4 = buf[3] - '0';
  d5 = buf[4] - '0';

  if (sizeBuffer == 3) {
    return String(d1)+String(d2)+String(d3);
  }
  else if (sizeBuffer == 5) {
    return String(d1)+String(d2)+String(d3)+String(d4)+String(d5);
  }
}

void DrawBlocks() {
  int i = 0;
  int j = 0;
  int x = 0;
  int y = 0;

  while (i < BLOCKS_MAX_Y) {
    j = 0;
    y = GetYcoordonnee(i);
    while (j < BLOCKS_MAX_X) {
      x = GetXcoordonnee(j);
      if (blocks_activation[i][j]) {
        //gb.display.drawRect(x, y, block_draw_w, block_draw_h);
        gb.display.fillRect(x, y, block_draw_w, block_draw_h);
      }
      j++;
    }
    i++;
  }
}

void DrawAnimationBlocks() {
  int i = 0;
  int j = 0;
  int x = 0;
  int y = 0;

  if ((game_animation_counter % 2) == 0) {
    game_animation_color = !(boolean)game_animation_color;
  }

  while (i < BLOCKS_MAX_Y) {
    j = 0;
    y = GetYcoordonnee(i);

    if (!lines_completions[i]) {
      while (j < BLOCKS_MAX_X) {
        x = GetXcoordonnee(j);
        if (blocks_activation[i][j]) {
          gb.display.fillRect(x, y, block_draw_w, block_draw_h);
        }
        j++;
      }
     }
     else {
       while (j < BLOCKS_MAX_X) {
        x = GetXcoordonnee(j);

        if (game_animation_color == 0) {
          gb.display.drawRect(x, y, block_draw_w, block_draw_h);
        }
        else {
          gb.display.fillRect(x, y, block_draw_w, block_draw_h);
        }
        j++;
      }
    }
    i++;
  }

  game_animation_counter--;
}

void DrawNextBlocks() {
  int x = -2;
  int y = 10;

  gb.display.cursorX = 1;
  gb.display.cursorY = 5;
  gb.display.println(String("SCORE"));
  gb.display.cursorX = 1;
  gb.display.cursorY = 13;
  gb.display.println(GetScoreString(game_score, 5));

  gb.display.cursorX = 1;
  gb.display.cursorY = (LCDHEIGHT / 2) + y - 13;
  gb.display.print("NEXT");

  switch(player_blocks_type) {
    case BLOCKS_LINE:
      //line
      gb.display.drawRect((field_x / 2) + x, (LCDHEIGHT / 2) + y, block_draw_w, block_draw_h);
      gb.display.drawRect((field_x / 2) + x, (LCDHEIGHT / 2) + y - block_draw_h, block_draw_w, block_draw_h);
      gb.display.drawRect((field_x / 2) + x, (LCDHEIGHT / 2) + y + block_draw_h, block_draw_w, block_draw_h);
      gb.display.drawRect((field_x / 2) + x, (LCDHEIGHT / 2) + y + (block_draw_h * 2), block_draw_w, block_draw_h);
      break;
    case BLOCKS_CUBE:
      //cube
      gb.display.drawRect((field_x / 2) + x, (LCDHEIGHT / 2) + y, block_draw_w, block_draw_h);
      gb.display.drawRect((field_x / 2) + x, (LCDHEIGHT / 2) + y + block_draw_h, block_draw_w, block_draw_h);
      gb.display.drawRect((field_x / 2) + x + block_draw_w, (LCDHEIGHT / 2) + y, block_draw_w, block_draw_h);
      gb.display.drawRect((field_x / 2) + x + block_draw_w, (LCDHEIGHT / 2) + y + block_draw_h, block_draw_w, block_draw_h);
      break;
    case BLOCKS_T:
      //T
      gb.display.drawRect((field_x / 2) + x, (LCDHEIGHT / 2) + y, block_draw_w, block_draw_h);
      gb.display.drawRect((field_x / 2) + x - block_draw_w, (LCDHEIGHT / 2) + y, block_draw_w, block_draw_h);
      gb.display.drawRect((field_x / 2) + x, (LCDHEIGHT / 2) + y + block_draw_h, block_draw_w, block_draw_h);
      gb.display.drawRect((field_x / 2) + x, (LCDHEIGHT / 2) + y - block_draw_h, block_draw_w, block_draw_h);
      break;
    case BLOCKS_L:
      //L
      gb.display.drawRect((field_x / 2) + x, (LCDHEIGHT / 2) + y, block_draw_w, block_draw_h);
      gb.display.drawRect((field_x / 2) + x, (LCDHEIGHT / 2) + y + block_draw_h, block_draw_w, block_draw_h);
      gb.display.drawRect((field_x / 2) + x, (LCDHEIGHT / 2) + y - block_draw_h, block_draw_w, block_draw_h);
      gb.display.drawRect((field_x / 2) + x - block_draw_w, (LCDHEIGHT / 2) + y - block_draw_h, block_draw_w, block_draw_h);
      break;
    case BLOCKS_L_REVERT:
      //L(revert)
      gb.display.drawRect((field_x / 2) + x, (LCDHEIGHT / 2) + y, block_draw_w, block_draw_h);
      gb.display.drawRect((field_x / 2) + x, (LCDHEIGHT / 2) + y + block_draw_h, block_draw_w, block_draw_h);
      gb.display.drawRect((field_x / 2) + x, (LCDHEIGHT / 2) + y - block_draw_h, block_draw_w, block_draw_h);
      gb.display.drawRect((field_x / 2) + x + block_draw_w, (LCDHEIGHT / 2) + y - block_draw_h, block_draw_w, block_draw_h);
      break;
    case BLOCKS_S:
      //S
      gb.display.drawRect((field_x / 2) + x, (LCDHEIGHT / 2) + y, block_draw_w, block_draw_h);
      gb.display.drawRect((field_x / 2) + x, (LCDHEIGHT / 2) + y - block_draw_h, block_draw_w, block_draw_h);
      gb.display.drawRect((field_x / 2) + x + block_draw_w, (LCDHEIGHT / 2) + y, block_draw_w, block_draw_h);
      gb.display.drawRect((field_x / 2) + x + block_draw_w, (LCDHEIGHT / 2) + y + block_draw_h, block_draw_w, block_draw_h);
      break;
    case BLOCKS_S_REVERT:
      //S(revert)
      gb.display.drawRect((field_x / 2) + x, (LCDHEIGHT / 2) + y, block_draw_w, block_draw_h);
      gb.display.drawRect((field_x / 2) + x, (LCDHEIGHT / 2) + y + block_draw_h, block_draw_w, block_draw_h);
      gb.display.drawRect((field_x / 2) + x + block_draw_w, (LCDHEIGHT / 2) + y, block_draw_w, block_draw_h);
      gb.display.drawRect((field_x / 2) + x + block_draw_w, (LCDHEIGHT / 2) + y - block_draw_h, block_draw_w, block_draw_h);
      break;
    default:
      //cube
      gb.display.drawRect((field_x / 2) + x, (LCDHEIGHT / 2) + y, block_draw_w, block_draw_h);
      gb.display.drawRect((field_x / 2) + x, (LCDHEIGHT / 2) + y + block_draw_h, block_draw_w, block_draw_h);
      gb.display.drawRect((field_x / 2) + x + block_draw_w, (LCDHEIGHT / 2) + y, block_draw_w, block_draw_h);
      gb.display.drawRect((field_x / 2) + x + block_draw_w, (LCDHEIGHT / 2) + y + block_draw_h, block_draw_w, block_draw_h);
      break;
  }
}

void DrawField() {
  gb.display.drawRect(field_x, field_y, field_w, field_h);
  gb.display.drawRect(field_x - 2, field_y, field_w + 4, field_h);
  //gb.display.drawLine(field_x, 0,field_x, LCDHEIGHT);
  //gb.display.drawLine(field_x2, 0, field_x2, LCDHEIGHT);
}

void ShowDebug(String message) {
  //print debug line
  gb.display.cursorX = 1;
  gb.display.cursorY = 1;
  gb.display.print("[ " + message + " ]");
}
