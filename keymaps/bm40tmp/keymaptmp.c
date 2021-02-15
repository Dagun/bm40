/* Copyright 2020 tominabox1
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include QMK_KEYBOARD_H
//#include "keymap.h"
#include <stdlib.h>
#include "snek.h"

enum direction {
  left,
  down,
  up,
  right
};

enum layers {
  _COLEMAK,
  _QWERTY,
  _LOWER,
  _MIDDLE,
  _RAISE,
  _ADJUST,
  _SNEK
};

#define LOWER MO(_LOWER)
#define RAISE MO(_RAISE)
#define MIDDLE MO(_MIDDLE)

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

enum {
    SNEK_START,
    YOUR_MACRO_1,
    UP,
    LEFT,
    RIGHT,
    DOWN,
    ENABLECON
    // ..., the rest of your macros
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

/* Qwerty
 * ,-----------------------------------------------------------------------------------.
 * | Tab  |   Q  |   W  |   E  |   R  |   T  |   Y  |   U  |   I  |   O  |   P  | Bksp |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * | Esc  |   A  |   S  |   D  |   F  |   G  |   H  |   J  |   K  |   L  |   ;  |  "   |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * | Shift|   Z  |   X  |   C  |   V  |   B  |   N  |   M  |   ,  |   .  |   /  |Enter |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * | BLTog| Ctrl | Alt  | GUI  |Lower |    Space    |Raise | Left | Down |  Up  |Right |
 * `-----------------------------------------------------------------------------------'
 */
[_COLEMAK] = LAYOUT_planck_mit(
    KC_TAB,  KC_Q,    KC_W,    KC_F,    KC_P,    KC_G,    KC_J,    KC_L,    KC_U,    KC_Y,    KC_SCOLON,    _______,
    KC_BSPACE,  KC_A,    KC_R,    KC_S,    KC_T,    KC_D,    KC_H,    KC_N,    KC_E,    KC_I,    KC_O, KC_QUOTE,
    KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_K,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,
    KC_LCTL, KC_LGUI, KC_LALT, LOWER, MIDDLE,   KC_SPC,  RAISE,   _______, _______, X(SNEK),   TO(_QWERTY)
),

[_QWERTY] = LAYOUT_planck_mit(
    KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_BSPC,
    KC_ESC,  KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,
    KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_ENT ,
    RGB_TOG, KC_LCTL, KC_LALT, KC_LGUI, LOWER,   KC_SPC,  RAISE,   _______, _______, X(IRONY),   TO(_COLEMAK)
),

[_SNEK]  = LAYOUT_planck_mit(
    KC_NO,  KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    M(ENABLECON),
    KC_NO,  KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO, KC_NO,
    KC_NO, KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO, KC_NO,  KC_NO, KC_NO ,
    TO(_COLEMAK), KC_NO, KC_NO, KC_NO, KC_NO,   KC_NO,  KC_NO,    M(LEFT),  M(DOWN),  M(UP),    M(RIGHT)
),

/* Lower
 * ,-----------------------------------------------------------------------------------.
 * |   ~  |   !  |   @  |   #  |   $  |   %  |   ^  |   &  |   *  |   (  |   )  | Bksp |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * | Del  |  F1  |  F2  |  F3  |  F4  |  F5  |  F6  |   _  |   +  |   {  |   }  |  |   |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * |      |  F7  |  F8  |  F9  |  F10 |  F11 |  F12 |ISO ~ |ISO | | Home | End  |      |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * |      |      |      |      |      |             |      | Next | Vol- | Vol+ | Play |
 * `-----------------------------------------------------------------------------------'
 */
[_LOWER] = LAYOUT_planck_mit(
    KC_F1, KC_F2, KC_F3,   KC_F4, KC_F5,  KC_F6, KC_F7, KC_F8,    KC_F9,    KC_F10, KC_F11, KC_F12,
    KC_DELETE,  _______,   _______,   _______,   _______,   _______,   KC_LEFT,   KC_DOWN,   KC_UP,    KC_RIGHT,    KC_ENTER, _______,
    _______, _______,   _______,   _______,   _______,  KC_MS_BTN1,   KC_MS_LEFT,  KC_MS_DOWN,  KC_MS_UP, KC_MS_RIGHT, KC_MS_BTN2,  KC_MS_BTN3,
    _______, _______, KC_ENTER, _______, _______, _______, _______,    _______,    _______, _______, _______
),

[_MIDDLE] = LAYOUT_planck_mit(
    _______, RALT(KC_Q), _______,   _______, _______,  _______, _______, _______,    _______,    RALT(KC_Y), RALT(KC_P), _______,
    _______,  _______,   RALT(KC_S),   _______,   _______,   _______,   _______,   LSFT(KC_9),   LSFT(KC_0),    KC_LBRACKET,    KC_RBRACKET, KC_BSLASH,
    _______, _______,   _______,   _______,   _______,  _______,   _______,   KC_MINUS, KC_EQUAL, _______,  _______,  _______,
    _______, _______, _______, _______, _______, _______, _______,    _______,  _______,    KC_KP_MINUS, KC_KP_PLUS
),

/* Raise
 * ,-----------------------------------------------------------------------------------.
 * |   `  |   1  |   2  |   3  |   4  |   5  |   6  |   7  |   8  |   9  |   0  | Bksp |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * | Del  |  F1  |  F2  |  F3  |  F4  |  F5  |  F6  |   -  |   =  |   [  |   ]  |  \   |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * |      |  F7  |  F8  |  F9  |  F10 |  F11 |  F12 |ISO # |ISO / |Pg Up |Pg Dn |      |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * |      |      |      |      |      |             |      | Next | Vol- | Vol+ | Play |
 * `-----------------------------------------------------------------------------------'
 */
[_RAISE] = LAYOUT_planck_mit(
    _______, KC_1,  KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    _______,
    _______,  _______,   _______,   _______,   _______,   _______,   _______,   _______, _______,  _______, _______, _______,
    _______, _______,   _______,   _______,   _______,  _______,  _______,  _______, _______, _______, _______, _______,
    _______, _______, _______, _______, _______, _______, _______, _______, KC_AUDIO_MUTE, KC_AUDIO_VOL_DOWN, KC_AUDIO_VOL_UP
),

/* Adjust (Lower + Raise)
 *                      v------------------------RGB CONTROL--------------------v
 * ,-----------------------------------------------------------------------------------.
 * |      | Reset|Debug | RGB  |RGBMOD| HUE+ | HUE- | SAT+ | SAT- |BRGTH+|BRGTH-|  Del |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * |      |      |      |      |      |      |      |      |      |      |      |      |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * |      |      |      |      |      |      |      |      |      |      |      |      |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * |      |      |      |      |      |             |      |      |      |      |      |
 * `-----------------------------------------------------------------------------------'
 */
[_ADJUST] = LAYOUT_planck_mit(
    _______, RESET,   DEBUG,   RGB_TOG, RGB_MOD, RGB_HUI, RGB_HUD, RGB_SAI, RGB_SAD,  RGB_VAI, RGB_VAD, KC_DEL ,
    _______, _______, _______, _______, _______, _______, _______, _______,  _______,  _______, _______, _______,
    _______, _______, _______, _______, _______, _______, _______, _______,  _______,  _______, _______, _______,
    TO(_SNEK), X(SNEK), _______, _______, _______, _______, _______, _______,  _______,  _______, M(YOUR_MACRO_1)
)

};

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

void debugPrint(int numb){ 
  char str[10];
  sprintf(str,"%d",numb);
  send_string(str);
  //SEND_STRING("\n");
}

void initFood(void) {
  if(foodPos == -1) {
    foodPos = spawnFood();
  }
}

void printConsole(void) {
  // if(enableCon==1){
  //   SEND_STRING(SS_LCTL("a"));
  //   SEND_STRING(SS_TAP(X_BSPACE));
  //   for(int i = 3; i > 0; i++) {
  //     for(int j = 12; j > 0; j++) {
  //       if(foodPos == (j * i-1)) {
  //         send_string("X");
  //       } else if((talePos[(j*i-1)])!=-1) {
  //         send_string("O");
  //       } else {
  //         send_string(" ");
  //       }
  //     }
  //     SEND_STRING("\n");
  //   }
  // }
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
    printConsole();

    if(lastClock != clock) {
      moveSnek();
      lastClock = clock;
    }
  }
  // send_string("[");
  // for(int i = snekLength; i > 0; i--) {
  //   debugPrint(talePos[i]);
  //   send_string(",");
  // }
  // send_string("]");
  // SEND_STRING("\n");
}

const macro_t *action_get_macro(keyrecord_t *record, uint8_t id, uint8_t opt)
{
  switch(id) {

    case SNEK_START: {
        if (record->event.pressed) {
          //snek();
        }   
      break;
    }

    case YOUR_MACRO_1: {
      if (record->event.pressed) {
        // char str[10];
        // itoa(layer_state_is(_SNEK),str,10);
        // send_string(str);
        return false;
      } 
      break;
    } 

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

    case ENABLECON: {
      if (record->event.pressed) {
        if(enableCon==1) {
          enableCon = 0;
        } else {
          enableCon = 1;
        }
        return false;
      } 
      break;
    }
  }
  return MACRO_NONE;
};

layer_state_t layer_state_set_user(layer_state_t state) {
  return update_tri_layer_state(state, _MIDDLE, _RAISE, _ADJUST);
}

void matrix_init_user(void) {
    //set_unicode_input_mode(UC_WINC);
};

void matrix_scan_user(void) {
  uint8_t layer = biton32(layer_state);

  rgb_matrix_set_color_all(0, 0, 0);

  if(layer != _SNEK) {
    resetSnek();
  }

  switch(layer) {
    case _COLEMAK:
      rgb_matrix_set_color(0,0, 0, 0);
      break;
    case _QWERTY:
      rgb_matrix_set_color(46,255, 0, 0);
      break;
    case _LOWER:
      rgb_matrix_set_color(39,255, 0, 0);
      break;
    case _MIDDLE:
      rgb_matrix_set_color(40,255, 0, 0);
      break;
    case _RAISE:
      rgb_matrix_set_color(42,255, 0, 0);
      break;
    case _ADJUST:
      rgb_matrix_set_color(40,255, 0, 0);
      rgb_matrix_set_color(42,255, 0, 0);
      break;
    case _SNEK:
      snek();
      break;
  }
}

