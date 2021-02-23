#include QMK_KEYBOARD_H

#include "raw_hid.h"
#include "timer.h"

#define LOWER MO(_LOWER)
#define RAISE MO(_RAISE)
#define MIDDLE MO(_MIDDLE)
#define EXTRA MO(_EXTRA)

enum layers {
  _COLEMAK,
  _QWERTY,
  _MIDDLE,
  _LOWER,
  _RAISE,
  _EXTRA,
  _ADJUST,
  _LEFT,
  _RIGHT,
  _SNEK,
  _RANDOM
};

////////////////////
// LED CONTROLLER //
////////////////////

typedef struct {
  uint8_t red;
  uint8_t blue;
  uint8_t green;
} color;

color leds[47];
uint8_t ledsGlow[47];

void showRGB(void) {
  for(int i = 0; i<47; i++){
    if(ledsGlow[i] == 1) {
      rgb_matrix_set_color(i,100,50,50);
    } else {
      rgb_matrix_set_color(i,leds[i].red, leds[i].blue, leds[i].green);
    }
  }
}

void setAllRGB(int red, int green, int blue) {
  for(int i = 0; i<47; i++){
    leds[i].red = red;
    leds[i].blue = green;
    leds[i].green = blue;
  }
}

void setRGB(int index, int red, int blue, int green) {
  leds[index].red = red;
  leds[index].blue = blue;
  leds[index].green = green;
}

////////////////////
// Global values ///
////////////////////


enum direction {
  left,
  down,
  up,
  right,
  none
};

static uint16_t key_timer;
uint32_t clock = 0;
uint32_t lastClock = 0;
uint8_t currentDir = left;
uint8_t lastDir = left;
uint8_t currentPos = 20;

uint8_t resetGame = 0;

uint32_t pressedKeys = 0;


///////////////////////
// Global functions ///
///////////////////////

void countClock(uint16_t time) {
  if (timer_elapsed(key_timer) > time) {
    clock = clock + 1;
    key_timer = timer_read();
  }
}

uint32_t getClock(void) {
  return timer_elapsed(key_timer);
} 

int getRandom(int a, int b) {
  int seed = rand()*(1+pressedKeys);
  int random = (seed%(b - a + 1))+a;
  if(random < 0) {
    random = random * -1;
  }
  return random;
}

void send_return(void) {
  send_string("\n");
}

void send_int(int value) {
  char val[25];
  sprintf(val,"%d",value);
  send_string(val);
}

int getRow(int a) {
  return a/12;
}

int smallestRow(int a) {
  int c = getRow(a);
  return a-(c*12);
}

// further improvement?
void moveObject(void) {
  uint8_t layer = biton32(layer_state);
  if(layer == _SNEK) {
    if(currentDir != none) {
      lastDir = currentDir;
    }
  } else if(layer == _RANDOM){
    lastDir = currentDir;
  }
  switch(lastDir) {
    case none:

      break;
    case left:
      if(smallestRow(currentPos) - 1 != -1) {
        currentPos = currentPos - 1;
      } else {
        if(getRow(currentPos) != 3) {
          currentPos = currentPos + 11;
        } else {
          currentPos = currentPos + 10;
        }
      }
      break;
    case down:
      if(getRow(currentPos) + 1 < 3) {
        currentPos = currentPos + 12;
      } else {
        if(getRow(currentPos) + 1 < 4) { 
          if(smallestRow(currentPos)<6) { 
            currentPos = currentPos + 12;
          } else {
            currentPos = currentPos + 11;
          }
        } else { 
          if(smallestRow(currentPos)<6) { 
            currentPos = 0 + smallestRow(currentPos);
          } else { 
            currentPos = 1 + smallestRow(currentPos);
          }
        }
      }
      break;
    case up:
      if(getRow(currentPos) - 1 != -1) {
        if(getRow(currentPos)==3) {
          if(smallestRow(currentPos)>5) {
            currentPos = currentPos + 1;
          }
        }
        currentPos = currentPos - 12;
      } else {
        if(smallestRow(currentPos)<6) {
          currentPos = 36 + smallestRow(currentPos);
        } else {
          currentPos = 35 + smallestRow(currentPos);
        }
      }
      break;
    case right:
      if(getRow(currentPos)!=3) {
        if(smallestRow(currentPos) + 1 != 12) {
          currentPos = currentPos + 1;
        } else {
          currentPos = currentPos - 11;
        }
      } else {
        if(smallestRow(currentPos) + 1 != 11) {
          currentPos = currentPos + 1;
        } else {
          currentPos = currentPos - 10;
        }
      }
      
      break;
  }
}


////////////////////////////////
// Declaration of values SNEK //
////////////////////////////////

uint8_t snekLives = 3;
uint8_t foodPos = 12;
uint8_t snekLength = 2;
uint8_t talePos[46] = {-1};
uint8_t isDead = 0;
uint16_t snekSpeed = 400;

// further improvement of the die function? timer.h or something
uint16_t timeWhenLoose = 0;

////////////////
// SNEK CODE ///
////////////////

// improvement, buggy
void updateTale(void) {
  if(talePos[snekLength-1]==-1) {
    talePos[snekLength-1] = talePos[snekLength-2];
  }
  for(int i = snekLength; i > 0; i--) {
    talePos[i] = talePos[i-1];
  }
  talePos[0] = currentPos;
}

void resetSnek(void) {
  snekLives = 3;
  clock = 0;
  lastClock = 0;
  currentDir = left;
  currentPos = 20;
  foodPos = 47;
  snekLength = 1;
  for(int i = 0; i < 46; i++) {
    talePos[i] = 47;
  }
  isDead = 0;
  timeWhenLoose = 0;
}

void moveSnek(void) {
  moveObject();
  updateTale();
}

void die(void) {
  if(timeWhenLoose+1 < clock) {
    setAllRGB(255,0,0);
    if(timeWhenLoose+2 < clock) {
      setAllRGB(0,0,0);
      if(timeWhenLoose+3 < clock) {
        setAllRGB(255,0,0);
        if(timeWhenLoose+4 < clock) {
          resetSnek();
          resetGame = 1;
        }
      }
    }
  }
}

void collisionSnek(void) {
  if(currentPos == foodPos) {
    foodPos = 47;
    snekLength = snekLength + 1;
  }
  for(int i = 1; i < snekLength; i++) {
    if(talePos[i]==currentPos) {
      isDead = 1;
      timeWhenLoose = clock;
    }
  }
}

void spawnFood(void) {
  if(foodPos == 47) {
    foodPos = getRandom(0,46);
  }
  setRGB(foodPos,0,255,0);
}

void spawnSnake(void) {
  for(uint8_t i = snekLength; i > 0; i--) {
    setRGB(talePos[i],255,255,255);
  }
  setRGB(talePos[0],255,0,0);
}

void snek(void) {
  setAllRGB(0,0,0);
  countClock(snekSpeed);
  if(isDead) {
    die();
  } else {
    spawnFood();
    spawnSnake();
    collisionSnek();

    if(lastClock != clock) {
      moveSnek();
      lastClock = clock;
    }
  }
}

////////////////////////////////////////
// Declaration of values Minesweepers //
////////////////////////////////////////




///////////////////////
// Minesweepers code //
///////////////////////



///////////////////////////////////////
// Declaration of values random game //
///////////////////////////////////////

uint16_t lastPressedKeys = 0;
uint8_t board[36] = {0};
uint8_t revealed[36] = {0};
uint8_t flags[36] = {0};
const uint8_t mines = 5;
uint8_t remaining = mines;
uint8_t placed = 0;
uint8_t deadMine = 0;
uint8_t revealedNum = 0;
uint8_t initRandomGame = 0;
uint8_t placedFlags = 0;

//////////////////////
// Random Game Code //
//////////////////////

void flashMines(void) {
  if(clock%2==0) {
    for(int i = 0; i < 36; i++){
      if(board[i]==9){
        setRGB(i,255,0,0);
      }
    }
  } else {
    for(int i = 0; i < 36; i++){
      if(board[i]==9){
        setRGB(i,0,0,0);
      }
    }
  }
}

void flashFlags(void) {
  if(clock%2==0) {
    for(int i = 0; i < 36; i++){
      if(flags[i]==1){
        setRGB(i,255,255,0);
      }
    }
  } else {
    for(int i = 0; i < 36; i++){
      if(flags[i]==1){
        setRGB(i,0,0,0);
      }
    }
  }
}

void showRandomGame(void) {
  setAllRGB(0,0,0);
  setRGB(currentPos,255,255,255);

  setRGB(36,0,255,0); // 0

  setRGB(37,0,0,255); // 1
  setRGB(38,0,255,255); // 2
  setRGB(39,150,150,255); // 3
  setRGB(40,255,0,255); // 4

  setRGB(41,255,255,0); // flag

  setRGB(42,255,98,23); // 5
  setRGB(43,50,255,150); // 6
  setRGB(44,50,50,0); // 7
  setRGB(45,0,0,100); // 8

  setRGB(46,255,0,0); // 9

  for(int i = 0; i < 36; i++) {
    if(revealed[i]==1) {
      if(board[i] == 0) {
        setRGB(i,0,255,0);
      } else if(board[i] == 1) {
        setRGB(i,0,0,255); // 1
      } else if(board[i] == 2) {
        setRGB(i,0,255,255); // 2
      } else if(board[i] == 3) {
        setRGB(i,150,150,255); // 3
      } else if(board[i] == 4) {
        setRGB(i,255,0,255); // 4
      } else if(board[i] == 5) {
        setRGB(i,255,98,23); // 5
      } else if(board[i] == 6) {
        setRGB(i,50,255,150); // 6
      } else if(board[i] == 7) {
        setRGB(i,50,50,0); // 7
      } else if(board[i] == 8) {
        setRGB(i,0,0,100); // 8
      } else if(board[i] == 9) {
        setRGB(i,255,0,0);
      }
    }
    // if(flags[i]==1){
    //   setRGB(i,255,255,0);
    // }
  }
}

void revealAll(void){
  for(int i=0; i < 36; i++){
    revealed[i] = 1;
  }
}

void flashScreen(int r, int g, int b){
  setAllRGB(0,0,0);
  if(clock==1 || clock==3){
    setAllRGB(r,g,b);
  }
}

void reveal(int pos) {
  revealed[pos] = 1;
  if(board[pos]==9) {
      deadMine = 1;
      clock = 0;
      revealAll();
    } else {
      ++revealedNum;
    }
}

void checkWin(void) {
  if(revealedNum == 36 - mines) {
    deadMine = 2;
    clock = 0;
    revealAll();
  } else if(placedFlags == mines) {
    int won = true;
    for(int i = 0; i < 36; i++){
      if(flags[i]==1 && board[i]!=9){
        won = false;
      }
    }
    if(won){
      deadMine=2;
      clock = 0;
      revealAll();
    }
  }
}

void actionOneRandom(void) {
  if(flags[currentPos]!=1) {
    reveal(currentPos);
  }
  
}

void actionTwoRandom(void) {
  if(revealed[currentPos]==0){
    if(flags[currentPos]) {
      flags[currentPos] = 0;
      --placedFlags;
    } else {
      flags[currentPos] = 1;
      ++placedFlags;
    }
  }
}

void setMines(void) {
  while(placed<mines) {
    uint8_t pos = getRandom(0,35);
    if(board[pos]!=9) {
      board[pos]=9;
      placed = placed + 1;
    }
  }

  for(int i=0; i < 36; i++){
    if(board[i]!=9){
      uint8_t count = 0;
      if(i != 0 && i != 11 && i != 12 && i != 23 && i != 24 && i != 35) {
        if(i-13 >= 0) {
          if(board[i-13]==9)
            ++count;
        }
        if(i-12 >= 0) {
          if(board[i-12]==9)
            ++count;
        }
        if(i-11 >= 0) {
          if(board[i-11]==9)
            ++count;
        }
        if(i-1 >= 0) {
          if(board[i-1]==9)
            ++count;
        }
        if(i+1 <= 35) {
          if(board[i+1]==9)
            ++count;
        }
        if(i+11 <= 35) {
          if(board[i+11]==9)
            ++count;
        }
        if(i+12 <= 35) {
          if(board[i+12]==9)
            ++count;
        }
        if(i+13 <= 35) {
          if(board[i+13]==9)
            ++count;
        }
      } else {
        if(i == 0 || i == 24) {
          if(board[i+1]==9)
            ++count;
        } else if(i == 11 || i == 35) {
          if(board[i-1]==9)
            ++count;
        } else if(i == 12 || i == 23) {
          if(board[i-12]==9) 
            ++count;
          if(board[i+12]==9)
            ++count;
        }
      }
      
      board[i]=count;
    }
  }
}

void randomGame(void) {
  if(initRandomGame==0){
    initRandomGame = 1;
    deadMine = 0;
    placedFlags = 0;
    revealedNum = 0;
    placed = 0;
    for(int i = 0; i < 36; i++) {
      board[i] = 0;
      flags[i] = 0;
      revealed[i] = 0;
    }
    setMines();
  }

  
  countClock(snekSpeed);

  if(deadMine != 0){
    if(deadMine == 1) {
      flashScreen(255,0,0);
    } else if(deadMine == 2) {
      flashScreen(0,255,0);
    }
    if(clock>4) {
      showRandomGame();
      flashMines();
    }
      
  } else {
    if(lastPressedKeys != pressedKeys){
      lastPressedKeys = pressedKeys;
      moveObject();
      checkWin();
      showRandomGame();
    }
    flashFlags();
  }
}

///////////////////
// Keyboard Code //
///////////////////

enum macros {
    UP,
    LEFT,
    RIGHT,
    DOWN,
    TOCOLE,
    SNEKSPEEDM,
    SNEKSPEEDP,
    TOUCH,
    ACTION1,
    ACTION2
};

enum unicode_names {
    BANG,
    IRONY,
    SNEK
};

const uint32_t PROGMEM unicode_map[] = {
    [BANG]  = 0x203D,  // ‽
    [IRONY] = 0x2E2E,  // ⸮
    [SNEK]  = 0x1F40D, // 🐍
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

[_COLEMAK] = LAYOUT_planck_mit(
    KC_TAB,    KC_Q,     KC_W,     KC_F,   KC_P,    KC_G,    KC_J,   KC_L,     KC_U,     KC_Y,     KC_SCOLON,  _______,
    KC_BSPACE, KC_A,     KC_R,     KC_S,   KC_T,    KC_D,    KC_H,   KC_N,     KC_E,     KC_I,     KC_O,       KC_QUOTE,
    KC_LSFT,   KC_Z,     KC_X,     KC_C,   KC_V,    KC_B,    KC_K,   KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,    KC_RSFT,
    KC_LCTL,   KC_LGUI,  KC_LALT,  MIDDLE, LOWER,      KC_SPC    ,   RAISE,    EXTRA,    KC_LALT,  X(SNEK),    TO(_QWERTY)
),

[_QWERTY] = LAYOUT_planck_mit(
    KC_TAB,   KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,      _______,
    KC_BSPC,  KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN,   KC_QUOT,
    KC_LSFT,  KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH,   KC_RSFT ,
    KC_LCTL,  KC_LGUI, KC_LALT, MIDDLE,  LOWER,      KC_SPC    ,    RAISE,   EXTRA,   KC_LALT,  X(IRONY),   TO(_COLEMAK)
),

[_LOWER] = LAYOUT_planck_mit(
    KC_F1,     KC_F2,    KC_F3,     KC_F4,     KC_F5,     KC_F6,      KC_F7,       KC_F8,       KC_F9,     KC_F10,      KC_F11,      KC_F12,
    KC_DELETE, _______,  _______,   _______,   _______,   _______,    KC_LEFT,     KC_DOWN,     KC_UP,     KC_RIGHT,    KC_ENTER,    _______,
    _______,   _______,  _______,   _______,   _______,   KC_MS_BTN1, KC_MS_LEFT,  KC_MS_DOWN,  KC_MS_UP,  KC_MS_RIGHT, KC_MS_BTN2,  KC_MS_BTN3,
    _______,   _______,  _______,   _______,   _______,         _______         ,    _______,     _______,     _______, KC_KP_MINUS, KC_KP_PLUS
),

[_MIDDLE] = LAYOUT_planck_mit(
    KC_ESCAPE,  RALT(KC_Q),  _______,     _______,    _______,   _______,   _______,   _______,      _______,     RALT(KC_Y),   RALT(KC_P),   _______,
    _______,    _______,     _______,     RALT(KC_S), _______,   _______,   _______,   LSFT(KC_9),   LSFT(KC_0),  KC_LBRACKET,  KC_RBRACKET,  KC_BSLASH,
    _______,    _______,     _______,     _______,    _______,   _______,   _______,   KC_MINUS,     KC_EQUAL,    _______,      _______,      _______,
    _______,    _______,     KC_ENTER,    _______,    _______,        _______      ,   _______,      _______,     _______,      _______,      _______
),

[_RAISE] = LAYOUT_planck_mit(
    KC_ESCAPE,  KC_1,      KC_2,      KC_3,      KC_4,     KC_5,       KC_6,      KC_7,      KC_8,      KC_9,            KC_0,            _______,
    _______,  _______,   _______,   _______,   _______,   _______,   _______,   _______,   _______,   _______,         _______,           _______,
    _______,  _______,   _______,   _______,   _______,   _______,   _______,   _______,   _______,   _______,         _______,           _______,
    _______,  _______,   _______,   _______,   _______,       _______       ,   _______,   _______,   KC_AUDIO_MUTE,   KC_AUDIO_VOL_DOWN, KC_AUDIO_VOL_UP
),

[_EXTRA] = LAYOUT_planck_mit(
   _______,   _______,   _______,   _______,   _______,   _______,   _______,   _______,   _______,   _______,         _______,           _______,
    _______,  _______,   _______,   _______,   _______,   _______,   _______,   _______,   _______,   _______,         _______,           _______,
    _______,  _______,   _______,   _______,   _______,   _______,   _______,   _______,   _______,   _______,         _______,           _______,
    _______,  _______,   _______,   _______,   _______,       _______       ,   _______,   _______,   _______,         _______,           _______
),

[_ADJUST] = LAYOUT_planck_mit(
    _______,   RESET,        DEBUG,   RGB_TOG, RGB_MOD, RGB_HUI, RGB_HUD, RGB_SAI,  RGB_SAD,  RGB_VAI, RGB_VAD, _______,
    _______,   _______,      _______, _______, _______, _______, _______, _______,  _______,  _______, _______, _______,
    _______,   _______,      _______, _______, _______, _______, _______, _______,  _______,  _______, _______, _______,
    TO(_SNEK), TO(_RANDOM),  _______, _______, _______,     _______     , _______,  _______,  _______, _______, _______
),

[_LEFT] = LAYOUT_planck_mit(
    _______,   _______,  _______, _______, _______, _______, _______, _______,  _______,  _______, _______, _______,
    _______,   _______,  _______, _______, _______, _______, _______, _______,  _______,  _______, _______, _______,
    _______,   _______,  _______, _______, _______, _______, _______, _______,  _______,  _______, _______, _______,
    _______,   _______,  _______, _______, _______,     _______     , _______,  _______,  _______, _______, _______
),

[_RIGHT] = LAYOUT_planck_mit(
    _______,   _______,  _______, _______, _______, _______, _______, KC_0,  M(LEFT),  M(DOWN),  M(UP),   M(RIGHT),
    _______,   _______,  _______, _______, _______, _______, _______, _______,  _______,  _______, _______, _______,
    _______,   _______,  _______, _______, _______, _______, _______, _______,  _______,  _______, _______, _______,
    _______,   _______,  _______, _______, _______,     _______     , _______,  _______,  _______, _______, _______
),

[_SNEK]  = LAYOUT_planck_mit(
    KC_NO,        KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    _______,    M(SNEKSPEEDM),   M(SNEKSPEEDP),
    KC_NO,        KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    _______,    _______,        _______,
    KC_NO,        KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,        KC_NO ,
    M(TOCOLE), KC_NO,    KC_NO,    KC_NO,    KC_NO,        KC_NO      ,       KC_NO,    M(LEFT),  M(DOWN),  M(UP),        M(RIGHT)
),

[_RANDOM]  = LAYOUT_planck_mit(
    M(TOUCH),        M(TOUCH),    M(TOUCH),    M(TOUCH),    M(TOUCH),    M(TOUCH),    M(TOUCH),    M(TOUCH),    M(TOUCH),    M(TOUCH),    M(TOUCH),   M(TOUCH),
    M(TOUCH),        M(TOUCH),    M(TOUCH),    M(TOUCH),    M(TOUCH),    M(TOUCH),    M(TOUCH),    M(TOUCH),    M(TOUCH),    M(TOUCH),    M(TOUCH),   M(TOUCH),
    M(TOUCH),        M(TOUCH),    M(TOUCH),    M(TOUCH),    M(TOUCH),    M(TOUCH),    M(TOUCH),    M(TOUCH),    M(TOUCH),    M(TOUCH),    M(TOUCH),   M(TOUCH) ,
    M(TOCOLE), KC_NO,    KC_NO,    KC_NO,      M(ACTION2),        M(ACTION1)      ,   M(ACTION2),  M(LEFT),     M(DOWN),     M(UP),       M(RIGHT)
)

};

const macro_t *action_get_macro(keyrecord_t *record, uint8_t id, uint8_t opt)
{
  currentDir = none;

  switch(id) {
    case UP: {
      if (record->event.pressed) {
          currentDir = up;
          return false;
      }
    }

    case DOWN: {
      if (record->event.pressed) {
          currentDir = down;
        return false;
      } 
    }

    case LEFT: {
      if (record->event.pressed) {
          currentDir = left;
        return false;
      } 
    }

    case RIGHT: {
      if (record->event.pressed) {
          currentDir = right;
        return false;
      } 
    }

    case TOCOLE: {
      if(record->event.pressed) {
          resetGame = 1;
          initRandomGame = 0;
          layer_move(_COLEMAK);
        return false;
      }
    }

    case SNEKSPEEDP:
      if(record->event.pressed) {
        snekSpeed = snekSpeed + 25;
      }
      return false;
    case SNEKSPEEDM:
      if(record->event.pressed) {
        snekSpeed = snekSpeed - 25;
      } else {
        // released
      }
      return false; // skip further processing

    case TOUCH: {
      if(record->event.pressed){
        currentPos = (record->event.key.row*12)+record->event.key.col;
        currentDir=none;
        return false;
      }
    }

    case ACTION1: {
      if(record->event.pressed){
        actionOneRandom();
        return false;
      }
    }

    case ACTION2: {
      if(record->event.pressed){
        actionTwoRandom();
        return false;
      }
    }
  }
  return MACRO_NONE;
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  if(record->event.pressed){
    pressedKeys = pressedKeys + 1;
  }
  switch(keycode) {
    default:
      return true;
  }
}

void matrix_scan_user(void) {
  uint8_t layer = biton32(layer_state);

  if(layer != _SNEK && layer != _RANDOM) {
    resetSnek();
    if(resetGame==1){
      setAllRGB(0,0,0);
      resetGame = 0;
    }
  }

// one array of numbers instead of array of boolean
  ledsGlow[46] = 0;
  ledsGlow[40] = 0;
  ledsGlow[39] = 0;
  ledsGlow[42] = 0; 
  ledsGlow[43] = 0;

  switch(layer) {
    case _COLEMAK:
      break;
    case _QWERTY:
      ledsGlow[46] = 1;
      break;
    case _LOWER:
      ledsGlow[40] = 1;
      break;
    case _MIDDLE:
      ledsGlow[39] = 1;
      break;
    case _RAISE:
      ledsGlow[42] = 1;
      break;
    case _EXTRA:
      ledsGlow[43] = 1;
      break;
    case _ADJUST:
      ledsGlow[40] = 1;
      ledsGlow[42] = 1;
      break;
    case _LEFT:
      ledsGlow[39] = 1;
      ledsGlow[40] = 1;
      break;
    case _RIGHT:
      ledsGlow[43] = 1;
      ledsGlow[42] = 1;
      break;
    case _SNEK:
      snek();
      break;
    case _RANDOM:
      randomGame();
      break;
  }

  showRGB();
}

layer_state_t layer_state_set_user(layer_state_t state) {
  state = update_tri_layer_state(state, _LOWER, _MIDDLE, _LEFT);
  state = update_tri_layer_state(state, _LOWER, _RAISE, _ADJUST);
  state = update_tri_layer_state(state, _RAISE, _EXTRA, _RIGHT);
  return state;
}

void keyboard_post_init_user(void) {
  timer_init();
  srand(timer_elapsed(key_timer)*(1+pressedKeys));

  for(int i = 0; i<47; i++){
    leds[i].red = 0;
    leds[i].blue = 0;
    leds[i].green = 0;
    ledsGlow[i] = 0;
  }
}

//////////////
// HID CODE //
//////////////

void raw_hid_receive(uint8_t *data, uint8_t length) {
  if(data[0]==1) {
    for(int i = 0; i<47; i++) {
      setRGB(i, data[1], data[2], data[3]);
    }
  } else {
    setRGB(data[4], data[1], data[2], data[3]);
  }
  raw_hid_send(data,length);
}
