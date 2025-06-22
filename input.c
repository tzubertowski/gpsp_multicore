/* gameplaySP
 *
 * Copyright (C) 2006 Exophase <exophase@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "common.h"

bool libretro_supports_bitmasks    = false;
bool libretro_supports_ff_override = false;
bool libretro_ff_enabled           = false;
bool libretro_ff_enabled_prev      = false;
#ifdef SF2000
bool mappingYXtoLR                 = false;

// SPEED CONTROL: Select+R and Select+L speed toggles
static u8 speed_mode_fast = 0;    // 0=normal, 1=fast, 2=fast+frameskip
static u8 speed_mode_slow = 0;    // 0=normal, 1=0.7x, 2=0.5x
static bool select_r_pressed_prev = false;
static bool select_l_pressed_prev = false;
#endif

unsigned turbo_period      = TURBO_PERIOD_MIN;
unsigned turbo_pulse_width = TURBO_PULSE_WIDTH_MIN;
unsigned turbo_a_counter   = 0;
unsigned turbo_b_counter   = 0;

static u32 old_key = 0;
static retro_input_state_t input_state_cb;

void retro_set_input_state(retro_input_state_t cb) { input_state_cb = cb; }

extern void set_fastforward_override(bool fastforward);

static void trigger_key(u32 key)
{
  u32 p1_cnt = read_ioreg(REG_P1CNT);

  if((p1_cnt >> 14) & 0x01)
  {
    u32 key_intersection = (p1_cnt & key) & 0x3FF;

    if(p1_cnt >> 15)
    {
      if(key_intersection == (p1_cnt & 0x3FF))
      {
        flag_interrupt(IRQ_KEYPAD);
        check_and_raise_interrupts();
      }
    }
    else
    {
      if(key_intersection)
      {
        flag_interrupt(IRQ_KEYPAD);
        check_and_raise_interrupts();
      }
    }
  }
}

u32 update_input(void)
{
   unsigned i;
   uint32_t new_key = 0;
   bool turbo_a     = false;
   bool turbo_b     = false;

   if (!input_state_cb)
      return 0;

   if (libretro_supports_bitmasks)
   {
      int16_t ret = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_MASK);

      for (i = 0; i < sizeof(btn_map) / sizeof(map); i++)
         new_key |= (ret & (1 << btn_map[i].retropad)) ? btn_map[i].gba : 0;

      libretro_ff_enabled = libretro_supports_ff_override &&
            (ret & (1 << RETRO_DEVICE_ID_JOYPAD_R2));

      #ifdef SF2000
      if (mappingYXtoLR) 
      {
        if (ret & (1 << RETRO_DEVICE_ID_JOYPAD_X))
            new_key = BUTTON_R;
        if (ret & (1 << RETRO_DEVICE_ID_JOYPAD_Y))
            new_key = BUTTON_L;
      }
      else
      {
      #endif
        turbo_a = (ret & (1 << RETRO_DEVICE_ID_JOYPAD_X));
        turbo_b = (ret & (1 << RETRO_DEVICE_ID_JOYPAD_Y));
      #if defined SF2000
      }
      #endif
   }
   else
   {
      for (i = 0; i < sizeof(btn_map) / sizeof(map); i++)
         new_key |= input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, btn_map[i].retropad) ? btn_map[i].gba : 0;

       libretro_ff_enabled = libretro_supports_ff_override &&
            input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R2);

      #ifdef SF2000
      if (mappingYXtoLR) 
      {
        if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X))
            new_key = BUTTON_R;
        if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y))
            new_key = BUTTON_L;
      }
      else
      {
      #endif
        turbo_a = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X);
        turbo_b = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y);
      #if defined SF2000
      }
      #endif
   }

   /* Handle turbo buttons */
   if (turbo_a)
   {
      new_key |= (turbo_a_counter < turbo_pulse_width) ?
            BUTTON_A : 0;

      turbo_a_counter++;
      if (turbo_a_counter >= turbo_period)
         turbo_a_counter = 0;
   }
   else
      turbo_a_counter = 0;

   if (turbo_b)
   {
      new_key |= (turbo_b_counter < turbo_pulse_width) ?
            BUTTON_B : 0;

      turbo_b_counter++;
      if (turbo_b_counter >= turbo_period)
         turbo_b_counter = 0;
   }
   else
      turbo_b_counter = 0;

#ifdef SF2000
   // SPEED CONTROL: Handle Select+R and Select+L combinations
   bool select_pressed = (new_key & BUTTON_SELECT) != 0;
   bool r_pressed = (new_key & BUTTON_R) != 0;
   bool l_pressed = (new_key & BUTTON_L) != 0;
   
   bool select_r_combo = select_pressed && r_pressed;
   bool select_l_combo = select_pressed && l_pressed;
   
   // Select+R: Toggle fast speed modes (normal -> fast -> fast+frameskip)
   if (select_r_combo && !select_r_pressed_prev) {
     speed_mode_fast = (speed_mode_fast + 1) % 3;
     // Reset slow mode when using fast mode
     if (speed_mode_fast != 0) {
       speed_mode_slow = 0;
     }
   }
   select_r_pressed_prev = select_r_combo;
   
   // Select+L: Toggle slow speed modes (normal -> 0.7x -> 0.5x)
   if (select_l_combo && !select_l_pressed_prev) {
     speed_mode_slow = (speed_mode_slow + 1) % 3;
     // Reset fast mode when using slow mode  
     if (speed_mode_slow != 0) {
       speed_mode_fast = 0;
     }
   }
   select_l_pressed_prev = select_l_combo;
   
   // Clear Select+L/R from the input when used for speed control
   if (select_r_combo || select_l_combo) {
     new_key &= ~(BUTTON_SELECT | BUTTON_R | BUTTON_L);
   }
#endif

   // GBP keypad detection hack (only at game startup!)
   if (serial_mode == SERIAL_MODE_GBP) {
     // During the startup screen (aproximate)
     if (frame_counter > 20 && frame_counter < 100) {
       // Emulate 4 keypad buttons pressed (which is impossible).
       new_key = (frame_counter % 3) ? 0x3FF : 0x30F;
     }
   }

   if ((new_key | old_key) != old_key)
      trigger_key(new_key);

   old_key = new_key;
   write_ioreg(REG_P1, (~old_key) & 0x3FF);

   /* Handle fast forward button */
   if (libretro_ff_enabled != libretro_ff_enabled_prev)
   {
      set_fastforward_override(libretro_ff_enabled);
      libretro_ff_enabled_prev = libretro_ff_enabled;
   }

   return 0;
}

bool input_check_savestate(const u8 *src)
{
  const u8 *p = bson_find_key(src, "input");
  return (p && bson_contains_key(p, "prevkey", BSON_TYPE_INT32));
}

bool input_read_savestate(const u8 *src)
{
  const u8 *p = bson_find_key(src, "input");
  if (p)
    return bson_read_int32(p, "prevkey", &old_key);
  return false;
}

unsigned input_write_savestate(u8 *dst)
{
  u8 *wbptr1, *startp = dst;
  bson_start_document(dst, "input", wbptr1);
  bson_write_int32(dst, "prevkey", old_key);
  bson_finish_document(dst, wbptr1);
  return (unsigned int)(dst - startp);
}

#ifdef SF2000
// SPEED CONTROL: Get current speed multiplier for timing control
float get_speed_multiplier(void) {
  if (speed_mode_fast == 1) {
    return 2.0f; // Fast mode (2x speed)
  } else if (speed_mode_fast == 2) {
    return 3.0f; // Fast mode with frameskip (3x speed)
  } else if (speed_mode_slow == 1) {
    return 0.7f; // Slow mode (0.7x speed)
  } else if (speed_mode_slow == 2) {
    return 0.5f; // Slow mode (0.5x speed)
  }
  return 1.0f; // Normal speed
}

// SPEED CONTROL: Check if frameskip should be enabled
bool get_speed_frameskip_enabled(void) {
  return (speed_mode_fast == 2); // Only enable frameskip in fast mode 2
}

// SPEED CONTROL: Get current speed mode for UI/OSD display
void get_speed_mode_info(char *buffer, size_t size) {
  if (speed_mode_fast == 1) {
    snprintf(buffer, size, "SPEED: 2x");
  } else if (speed_mode_fast == 2) {
    snprintf(buffer, size, "SPEED: 3x+SKIP");
  } else if (speed_mode_slow == 1) {
    snprintf(buffer, size, "SPEED: 0.7x");
  } else if (speed_mode_slow == 2) {
    snprintf(buffer, size, "SPEED: 0.5x");
  } else {
    snprintf(buffer, size, "SPEED: 1x");
  }
}
#endif


