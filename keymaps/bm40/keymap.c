#include QMK_KEYBOARD_H

#define LOWER MO(_LOWER)
#define RAISE MO(_RAISE)
#define MIDDLE MO(_MIDDLE)

// snek code begin

enum direction {
  left,
  down,
  up,
  right
};

int qwerty = 0;

int snekLives = 3;
int clock = 0;
int lastClock = 0;
int currentDir = left;
int currentPos = 20;
int initRand = 0;
int foodPos = 12;
int snekLength = 1;
int talePos[46] = {-1};
int isDead = 0;
int timeWhenLoose = 0;
int enableCon = 0;
static uint16_t key_timer;

void initRandom(void) {
  if(initRand==0) {
    srand(timer_elapsed(key_timer));
    initRand = 1;
  }
}

void countClock(void) {
  if (timer_elapsed(key_timer) > 500) {
    clock = clock + 1;
    key_timer = timer_read();
  }
}

void updateSnek(void) {
  for(int i = snekLength; i > 0; i--) {
    rgb_matrix_set_color(talePos[i],255, 255, 255);
  }
}

void updateTale(void) {
  if(talePos[snekLength-1]==-1) {
    talePos[snekLength-1] = talePos[snekLength-2];
  }
  for(int i = snekLength; i > 0; i--) {
    talePos[i] = talePos[i-1];
  }
  talePos[0] = currentPos;
}

void updateFood(void) {
  rgb_matrix_set_color(foodPos,255, 0, 0);
}

int getRow(int a, int b) {
  return a/b;
}

int smallestRow(int a) {
  int c = getRow(a,12);
  return a-(c*12);
}

void resetSnek(void) {
  snekLives = 3;
  clock = 0;
  lastClock = 0;
  currentDir = left;
  currentPos = 20;
  initRand = 0;
  foodPos = -1;
  snekLength = 1;
  for(int i = 0; i < 46; i++) {
    talePos[i] = -1;
  }
  isDead = 0;
  timeWhenLoose = 0;
}

void moveSnek(void) {
  switch(currentDir) {
    case left:
      if(smallestRow(currentPos) - 1 != -1) {
        currentPos = currentPos - 1;
      } else {
        if(getRow(currentPos,12) != 3) {
          currentPos = currentPos + 11;
        } else {
          currentPos = currentPos + 10;
        }
      }
      break;
    case down:
      if(getRow(currentPos,12) + 1 < 3) {
        currentPos = currentPos + 12;
      } else {
        if(getRow(currentPos,12) + 1 < 4) { 
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
      if(getRow(currentPos,12) - 1 != -1) {
        if(getRow(currentPos,12)==3) {
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
      if(getRow(currentPos,12)!=3) {
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
  updateTale();
}

int getRandom(int a, int b) {
  int seed = rand();
  return (seed%(b - a + 1))+a;
}

int spawnFood(void) {
  return getRandom(0,46);
}

void die(void) {
  if(timeWhenLoose+1 < clock) {
    rgb_matrix_set_color_all(0, 0, 0);
    if(timeWhenLoose+2 < clock) {
      rgb_matrix_set_color_all(255, 0, 0);
      if(timeWhenLoose+3 < clock) {
        rgb_matrix_set_color_all(0, 0, 0);
        if(timeWhenLoose+4 < clock) {
          rgb_matrix_set_color_all(255, 0, 0);
          if(timeWhenLoose+5 < clock) {
            resetSnek();
          }
        }
      }
    }
  }
}

void collisionSnek(void) {
  if(currentPos == foodPos) {
    foodPos = spawnFood();
    snekLength = snekLength + 1;
  }
  for(int i = 1; i < snekLength; i++) {
    if(talePos[i]==currentPos) {
      isDead = 1;
      timeWhenLoose = clock;
    }
  }
}

void initFood(void) {
  if(foodPos == -1) {
    foodPos = spawnFood();
  }
}

void snek(void) {
  countClock();
  if(isDead==1) {
    die();
  } else {
    initRandom();
    initFood();
    collisionSnek();
    updateFood();
    updateSnek();

    if(lastClock != clock) {
      moveSnek();
      lastClock = clock;
    }
  }
}

// snek code end

enum layers {
  _COLEMAK,
  _QWERTY,
  _LOWER,
  _MIDDLE,
  _RAISE,
  _ADJUST,
  _SNEK
};

enum macros {
    UP,
    LEFT,
    RIGHT,
    DOWN
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

/* _COLEMAK
 * ,-----------------------------------------------------------------------------------.
 * | Tab  |   Q  |   W  |   F  |   P  |   G  |   J  |   L  |   U  |   Y  |   ;  |      |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * |BSpace|   A  |   R  |   S  |   T  |   D  |   H  |   N  |   E  |   I  |   O  |  "   |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * | Shift|   Z  |   X  |   C  |   V  |   B  |   K  |   M  |   ,  |   .  |   /  | Shift|
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * | Ctrl | GUI  | Alt  | LOWER|MIDDLE|    Space    |RAISE |      |      |XSNEK |QWERTY|
 * `-----------------------------------------------------------------------------------'
 */

[_COLEMAK] = LAYOUT_planck_mit(
    KC_TAB,    KC_Q,     KC_W,     KC_F,   KC_P,    KC_G,    KC_J,   KC_L,     KC_U,     KC_Y,     KC_SCOLON,  _______,
    KC_BSPACE, KC_A,     KC_R,     KC_S,   KC_T,    KC_D,    KC_H,   KC_N,     KC_E,     KC_I,     KC_O,       KC_QUOTE,
    KC_LSFT,   KC_Z,     KC_X,     KC_C,   KC_V,    KC_B,    KC_K,   KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,    KC_RSFT,
    KC_LCTL,   KC_LGUI,  KC_LALT,  MIDDLE, LOWER,      KC_SPC    ,  RAISE,  _______,  _______,  X(SNEK),  TO(_QWERTY)
),

/* _QWERTY
 * ,-----------------------------------------------------------------------------------.
 * | Tab  |   Q  |   W  |   E  |   R  |   T  |   Y  |   U  |   I  |   O  |   P  |BSpace|
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * |BSpace|   A  |   S  |   D  |   F  |   G  |   H  |   J  |   K  |   L  |   ;  |  "   |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * | Shift|   Z  |   X  |   C  |   V  |   B  |   N  |   M  |   ,  |   .  |   /  |Enter |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * | BLTog| Ctrl | Alt  | GUI  |Lower |    Space    |Raise |      |      |XIRONY|COLEMA|
 * `-----------------------------------------------------------------------------------'
 */

[_QWERTY] = LAYOUT_planck_mit(
    KC_TAB,   KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,      ,
    KC_BSPC,  KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN,   KC_QUOT,
    KC_LSFT,  KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH,   KC_RSFT ,
    KC_LCTL,  KC_LGUI, KC_LALT, MIDDLE,  LOWER,      KC_SPC    ,    RAISE,   _______, _______, X(IRONY),  TO(_COLEMAK)
),

/* _LOWER
 * ,-----------------------------------------------------------------------------------.
 * |  F1  |  F2  |  F3  |  F4  |  F5  |  F6  |  F7  |  F8  |  F9  | F10  | F11  | F12  |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * | Del  |      |      |      |      |      | LEFT | DOWN |  UP  |RIGHT |ENTER |      |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * |      |      |      |      |      | MB1  | MSL  | MSD  | MSU  | MSR  | MB2  | MB3  |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * |      |      |ENTER |      |      |             |      |      |      |      |      |
 * `-----------------------------------------------------------------------------------'
 */

[_LOWER] = LAYOUT_planck_mit(
    KC_F1,     KC_F2,    KC_F3,     KC_F4,     KC_F5,     KC_F6,      KC_F7,       KC_F8,       KC_F9,     KC_F10,      KC_F11,      KC_F12,
    KC_DELETE, _______,  _______,   _______,   _______,   _______,    KC_LEFT,     KC_DOWN,     KC_UP,     KC_RIGHT,    KC_ENTER,    _______,
    _______,   _______,  _______,   _______,   _______,   KC_MS_BTN1, KC_MS_LEFT,  KC_MS_DOWN,  KC_MS_UP,  KC_MS_RIGHT, KC_MS_BTN2,  KC_MS_BTN3,
    _______,   _______,  _______,   _______,   _______,         _______         ,    _______,     _______,     _______, KC_KP_MINUS, KC_KP_PLUS
),

/* _MIDDLE
 * ,-----------------------------------------------------------------------------------.
 * |  ESC |  Ä   |      |      |      |      |      |      |      |   Ü  |   Ö  |      |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * |      |      |      |   ß  |      |      |      |  (   |  )   |  [   |  ]   |  \   |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * |      |      |      |      |      |      |      |  -   |  =   |      |      |      |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * |      |      |      |      |      |             |      |      |      |NMINUS|NPLUS |
 * `-----------------------------------------------------------------------------------'
 */

[_MIDDLE] = LAYOUT_planck_mit(
    KC_ESCAPE,  RALT(KC_Q),  _______,     _______,    _______,   _______,   _______,   _______,      _______,     RALT(KC_Y),   RALT(KC_P),   _______,
    _______,    _______,     _______,     RALT(KC_S), _______,   _______,   _______,   LSFT(KC_9),   LSFT(KC_0),  KC_LBRACKET,  KC_RBRACKET,  KC_BSLASH,
    _______,    _______,     _______,     _______,    _______,   _______,   _______,   KC_MINUS,     KC_EQUAL,    _______,      _______,      _______,
    _______,    _______,     KC_ENTER,    _______,    _______,        _______      ,   _______,      _______,     _______,      _______,      _______
),

/* _RAISE
 * ,-----------------------------------------------------------------------------------.
 * | ESC  |   1  |   2  |   3  |   4  |   5  |   6  |   7  |   8  |   9  |   0  |      |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * |      |      |      |      |      |      |      |      |      |      |      |      |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * |      |      |      |      |      |      |      |      |      |      |      |      |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * |      |      |      |      |      |             |      |      | MUTE |AUDIO-|AUDIO+|
 * `-----------------------------------------------------------------------------------'
 */

[_RAISE] = LAYOUT_planck_mit(
    KC_ESCAPE,  KC_1,      KC_2,      KC_3,      KC_4,     KC_5,       KC_6,      KC_7,      KC_8,      KC_9,            KC_0,              _______,
    _______,  _______,   _______,   _______,   _______,   _______,   _______,   _______,   _______,   _______,         _______,           _______,
    _______,  _______,   _______,   _______,   _______,   _______,   _______,   _______,   _______,   _______,         _______,           _______,
    _______,  _______,   _______,   _______,   _______,       _______       ,   _______,   _______,   KC_AUDIO_MUTE,   KC_AUDIO_VOL_DOWN, KC_AUDIO_VOL_UP
),

/* _ADJUST (Lower + Raise)
 *                      v------------------------RGB CONTROL--------------------v
 * ,-----------------------------------------------------------------------------------.
 * |      | Reset|Debug | RGB  |RGBMOD| HUE+ | HUE- | SAT+ | SAT- |BRGTH+|BRGTH-|      |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * |      |      |      |      |      |      |      |      |      |      |      |      |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * |      |      |      |      |      |      |      |      |      |      |      |      |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * | SNEK |XSNEK |      |      |      |             |      |      |      |      |      |
 * `-----------------------------------------------------------------------------------'
 */

[_ADJUST] = LAYOUT_planck_mit(
    _______,   RESET,    DEBUG,   RGB_TOG, RGB_MOD, RGB_HUI, RGB_HUD, RGB_SAI,  RGB_SAD,  RGB_VAI, RGB_VAD, _______,
    _______,   _______,  _______, _______, _______, _______, _______, _______,  _______,  _______, _______, _______,
    _______,   _______,  _______, _______, _______, _______, _______, _______,  _______,  _______, _______, _______,
    TO(_SNEK), X(SNEK),  _______, _______, _______,     _______     , _______,  _______,  _______, _______, _______
),

/* _SNEK
 * ,-----------------------------------------------------------------------------------.
 * |      |      |      |      |      |      |      |      |      |      |      |      |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * |      |      |      |      |      |      |      |      |      |      |      |      |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * |      |      |      |      |      |      |      |      |      |      |      |      |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * |COLEMA|      |      |      |      |             |      | Left | Down |  Up  |Right |
 * `-----------------------------------------------------------------------------------'
 */

[_SNEK]  = LAYOUT_planck_mit(
    KC_NO,        KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,   KC_NO,
    KC_NO,        KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,   KC_NO,
    KC_NO,        KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,   KC_NO ,
    TO(_COLEMAK), KC_NO,    KC_NO,    KC_NO,    KC_NO,        KC_NO      ,    KC_NO,    M(LEFT),  M(DOWN),  M(UP),   M(RIGHT)
)

};

const macro_t *action_get_macro(keyrecord_t *record, uint8_t id, uint8_t opt);
void matrix_init_user(void);
void matrix_scan_user(void);
layer_state_t layer_state_set_user(layer_state_t state);

const macro_t *action_get_macro(keyrecord_t *record, uint8_t id, uint8_t opt)
{
  switch(id) {

    case UP: {
      if (record->event.pressed) {
        if(currentDir != down)
          currentDir = up;
      } 
      break;
    }

    case DOWN: {
      if (record->event.pressed) {
        if(currentDir != up)
          currentDir = down;
        return false;
      } 
      break;
    }

    case LEFT: {
      if (record->event.pressed) {
        if(currentDir != right)
          currentDir = left;
        return false;
      } 
      break;
    }

    case RIGHT: {
      if (record->event.pressed) {
        if(currentDir != left)
          currentDir = right;
        return false;
      } 
      break;
    }
  }
  return MACRO_NONE;
};

void matrix_scan_user(void) {
  uint8_t layer = biton32(layer_state);

  rgb_matrix_set_color_all(0, 0, 0);

  if(layer != _SNEK) {
    resetSnek();
  }

  switch(layer) {
    case _COLEMAK:
      qwerty = 0;
      rgb_matrix_set_color(0,0, 0, 0);
      break;
    case _QWERTY:
      qwerty = 1;
      break;
    case _LOWER:
      rgb_matrix_set_color(40,100, 50, 50);
      break;
    case _MIDDLE:
      rgb_matrix_set_color(39,100, 50, 50);
      break;
    case _RAISE:
      rgb_matrix_set_color(42,100, 50, 50);
      break;
    case _ADJUST:
      rgb_matrix_set_color(40,100, 50, 50);
      rgb_matrix_set_color(42,100, 50, 50);
      break;
    case _SNEK:
      snek();
      break;
  }

  if(qwerty) {
      rgb_matrix_set_color(46,100, 50, 50);
  }
}

layer_state_t layer_state_set_user(layer_state_t state) {
  return update_tri_layer_state(state, _LOWER, _RAISE, _ADJUST);
}