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
#include <ctype.h>
#include <time.h>

timer_type timer[4];

u32 frame_counter = 0;
u32 cpu_ticks = 0;
u32 execute_cycles = 0;
s32 video_count = 0;

u32 last_frame = 0;
u32 flush_ram_count = 0;
u32 gbc_update_count = 0;
u32 oam_update_count = 0;

char main_path[512];
char save_path[512];

// Custom splash screen variables
static bool splash_shown = false;
static u32 splash_timer = 0;
static bool first_rom_execution = false;

static u32 random_state = 0;

// Generate 16 random bits.
u16 rand_gen() {
  random_state = ((random_state * 1103515245) + 12345) & 0x7fffffff;
  return random_state;
}

// Add some random state to the initial seed.
void rand_seed(u32 data) {
  random_state ^= rand_gen() ^ data;
}

// Simple 8x8 bitmap font data (basic ASCII characters)
static const u8 font_8x8[95][8] = {
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // space
  {0x18,0x3C,0x3C,0x18,0x18,0x00,0x18,0x00}, // !
  {0x36,0x36,0x00,0x00,0x00,0x00,0x00,0x00}, // "
  {0x36,0x36,0x7F,0x36,0x7F,0x36,0x36,0x00}, // #
  {0x0C,0x3E,0x03,0x1E,0x30,0x1F,0x0C,0x00}, // $
  {0x00,0x63,0x33,0x18,0x0C,0x66,0x63,0x00}, // %
  {0x1C,0x36,0x1C,0x6E,0x3B,0x33,0x6E,0x00}, // &
  {0x06,0x06,0x03,0x00,0x00,0x00,0x00,0x00}, // '
  {0x18,0x0C,0x06,0x06,0x06,0x0C,0x18,0x00}, // (
  {0x06,0x0C,0x18,0x18,0x18,0x0C,0x06,0x00}, // )
  {0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00}, // *
  {0x00,0x0C,0x0C,0x3F,0x0C,0x0C,0x00,0x00}, // +
  {0x00,0x00,0x00,0x00,0x00,0x0C,0x06,0x00}, // ,
  {0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x00}, // -
  {0x00,0x00,0x00,0x00,0x00,0x0C,0x0C,0x00}, // .
  {0x60,0x30,0x18,0x0C,0x06,0x03,0x01,0x00}, // /
  {0x3E,0x63,0x73,0x7B,0x6F,0x67,0x3E,0x00}, // 0
  {0x0C,0x0E,0x0C,0x0C,0x0C,0x0C,0x3F,0x00}, // 1
  {0x1E,0x33,0x30,0x1C,0x06,0x33,0x3F,0x00}, // 2
  {0x1E,0x33,0x30,0x1C,0x30,0x33,0x1E,0x00}, // 3
  {0x38,0x3C,0x36,0x33,0x7F,0x30,0x78,0x00}, // 4
  {0x3F,0x03,0x1F,0x30,0x30,0x33,0x1E,0x00}, // 5
  {0x1C,0x06,0x03,0x1F,0x33,0x33,0x1E,0x00}, // 6
  {0x3F,0x33,0x30,0x18,0x0C,0x0C,0x0C,0x00}, // 7
  {0x1E,0x33,0x33,0x1E,0x33,0x33,0x1E,0x00}, // 8
  {0x1E,0x33,0x33,0x3E,0x30,0x18,0x0E,0x00}, // 9
  {0x00,0x0C,0x0C,0x00,0x00,0x0C,0x0C,0x00}, // :
  {0x00,0x0C,0x0C,0x00,0x00,0x0C,0x06,0x00}, // ;
  {0x18,0x0C,0x06,0x03,0x06,0x0C,0x18,0x00}, // <
  {0x00,0x00,0x3F,0x00,0x00,0x3F,0x00,0x00}, // =
  {0x06,0x0C,0x18,0x30,0x18,0x0C,0x06,0x00}, // >
  {0x1E,0x33,0x30,0x18,0x0C,0x00,0x0C,0x00}, // ?
  {0x3E,0x63,0x7B,0x7B,0x7B,0x03,0x1E,0x00}, // @
  {0x0C,0x1E,0x33,0x33,0x3F,0x33,0x33,0x00}, // A
  {0x3F,0x66,0x66,0x3E,0x66,0x66,0x3F,0x00}, // B
  {0x3C,0x66,0x03,0x03,0x03,0x66,0x3C,0x00}, // C
  {0x1F,0x36,0x66,0x66,0x66,0x36,0x1F,0x00}, // D
  {0x7F,0x46,0x16,0x1E,0x16,0x46,0x7F,0x00}, // E
  {0x7F,0x46,0x16,0x1E,0x16,0x06,0x0F,0x00}, // F
  {0x3C,0x66,0x03,0x03,0x73,0x66,0x7C,0x00}, // G
  {0x33,0x33,0x33,0x3F,0x33,0x33,0x33,0x00}, // H
  {0x1E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0x00}, // I
  {0x78,0x30,0x30,0x30,0x33,0x33,0x1E,0x00}, // J
  {0x67,0x66,0x36,0x1E,0x36,0x66,0x67,0x00}, // K
  {0x0F,0x06,0x06,0x06,0x46,0x66,0x7F,0x00}, // L
  {0x63,0x77,0x7F,0x7F,0x6B,0x63,0x63,0x00}, // M
  {0x63,0x67,0x6F,0x7B,0x73,0x63,0x63,0x00}, // N
  {0x1C,0x36,0x63,0x63,0x63,0x36,0x1C,0x00}, // O
  {0x3F,0x66,0x66,0x3E,0x06,0x06,0x0F,0x00}, // P
  {0x1E,0x33,0x33,0x33,0x3B,0x1E,0x38,0x00}, // Q
  {0x3F,0x66,0x66,0x3E,0x36,0x66,0x67,0x00}, // R
  {0x1E,0x33,0x07,0x0E,0x38,0x33,0x1E,0x00}, // S
  {0x3F,0x2D,0x0C,0x0C,0x0C,0x0C,0x1E,0x00}, // T
  {0x33,0x33,0x33,0x33,0x33,0x33,0x3F,0x00}, // U
  {0x33,0x33,0x33,0x33,0x33,0x1E,0x0C,0x00}, // V
  {0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00}, // W
  {0x63,0x63,0x36,0x1C,0x1C,0x36,0x63,0x00}, // X
  {0x33,0x33,0x33,0x1E,0x0C,0x0C,0x1E,0x00}, // Y
  {0x7F,0x63,0x31,0x18,0x4C,0x66,0x7F,0x00}, // Z
  {0x1E,0x06,0x06,0x06,0x06,0x06,0x1E,0x00}, // [
  {0x03,0x06,0x0C,0x18,0x30,0x60,0x40,0x00}, // backslash
  {0x1E,0x18,0x18,0x18,0x18,0x18,0x1E,0x00}, // ]
  {0x08,0x1C,0x36,0x63,0x00,0x00,0x00,0x00}, // ^
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF}, // _
  {0x0C,0x0C,0x18,0x00,0x00,0x00,0x00,0x00}, // `
  {0x00,0x00,0x1E,0x30,0x3E,0x33,0x6E,0x00}, // a
  {0x07,0x06,0x06,0x3E,0x66,0x66,0x3B,0x00}, // b
  {0x00,0x00,0x1E,0x33,0x03,0x33,0x1E,0x00}, // c
  {0x38,0x30,0x30,0x3e,0x33,0x33,0x6E,0x00}, // d
  {0x00,0x00,0x1E,0x33,0x3f,0x03,0x1E,0x00}, // e
  {0x1C,0x36,0x06,0x0f,0x06,0x06,0x0F,0x00}, // f
  {0x00,0x00,0x6E,0x33,0x33,0x3E,0x30,0x1F}, // g
  {0x07,0x06,0x36,0x6E,0x66,0x66,0x67,0x00}, // h
  {0x0C,0x00,0x0E,0x0C,0x0C,0x0C,0x1E,0x00}, // i
  {0x30,0x00,0x30,0x30,0x30,0x33,0x33,0x1E}, // j
  {0x07,0x06,0x66,0x36,0x1E,0x36,0x67,0x00}, // k
  {0x0E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0x00}, // l
  {0x00,0x00,0x33,0x7F,0x7F,0x6B,0x63,0x00}, // m
  {0x00,0x00,0x1F,0x33,0x33,0x33,0x33,0x00}, // n
  {0x00,0x00,0x1E,0x33,0x33,0x33,0x1E,0x00}, // o
  {0x00,0x00,0x3B,0x66,0x66,0x3E,0x06,0x0F}, // p
  {0x00,0x00,0x6E,0x33,0x33,0x3E,0x30,0x78}, // q
  {0x00,0x00,0x3B,0x6E,0x66,0x06,0x0F,0x00}, // r
  {0x00,0x00,0x3E,0x03,0x1E,0x30,0x1F,0x00}, // s
  {0x08,0x0C,0x3E,0x0C,0x0C,0x2C,0x18,0x00}, // t
  {0x00,0x00,0x33,0x33,0x33,0x33,0x6E,0x00}, // u
  {0x00,0x00,0x33,0x33,0x33,0x1E,0x0C,0x00}, // v
  {0x00,0x00,0x63,0x6B,0x7F,0x7F,0x36,0x00}, // w
  {0x00,0x00,0x63,0x36,0x1C,0x36,0x63,0x00}, // x
  {0x00,0x00,0x33,0x33,0x33,0x3E,0x30,0x1F}, // y
  {0x00,0x00,0x3F,0x19,0x0C,0x26,0x3F,0x00}, // z
};

// Simple function to draw text on the GBA screen buffer using bitmap font
static void draw_splash_text(u16 *screen_buffer, const char *text, int x, int y, u16 color) {
  int i, j, k;
  int len = strlen(text);
  
  // First render normally to a temporary buffer
  u16 temp_line[240]; // One line buffer
  
  for (j = 0; j < 8 && y + j < GBA_SCREEN_HEIGHT; j++) {
    // Clear temp line for this row
    memset(temp_line, 0, sizeof(temp_line));
    
    // Render all characters for this row
    for (i = 0; i < len && x + i * 8 < GBA_SCREEN_WIDTH; i++) {
      char c = text[i];
      if (c < 32 || c > 126) continue; // Skip non-printable chars
      
      const u8 *glyph = font_8x8[c - 32]; // ASCII offset
      u8 row = glyph[j];
      
      for (k = 0; k < 8 && x + i * 8 + k < GBA_SCREEN_WIDTH; k++) {
        if (row & (0x80 >> k)) { // Normal bit reading
          int px = x + i * 8 + k;
          if (px >= 0 && px < GBA_SCREEN_WIDTH) {
            temp_line[px] = color;
          }
        }
      }
    }
    
    // Now copy the line horizontally flipped to the real buffer
    int py = y + j;
    if (py >= 0 && py < GBA_SCREEN_HEIGHT) {
      for (k = 0; k < GBA_SCREEN_WIDTH; k++) {
        if (temp_line[k] == color) {  // Check if pixel was set to text color
          screen_buffer[py * GBA_SCREEN_WIDTH + (GBA_SCREEN_WIDTH - 1 - k)] = temp_line[k];
        }
      }
    }
  }
}

// Display custom splash screen
static void show_custom_splash() {
  extern u16* gba_screen_pixels;
  u16 bg_color = 0xF5BB;    // #FBB7DF background (RGB565: R31,G45,B27)
  u16 text_color = 0xFFFF;  // White text
  u16 accent_color = 0xFFFF; // White accent
  u16 red_color = 0xFFFF;   // White for UNSTABLE BUILD
  
  // Clear screen to black
  memset(gba_screen_pixels, 0, GBA_SCREEN_BUFFER_SIZE);
  
  // Fill with background color
  int i;
  for (i = 0; i < GBA_SCREEN_WIDTH * GBA_SCREEN_HEIGHT; i++) {
    gba_screen_pixels[i] = bg_color;
  }
  
  // Draw black border
  for (i = 0; i < GBA_SCREEN_WIDTH; i++) {
    gba_screen_pixels[i] = 0x0000; // Top border
    gba_screen_pixels[(GBA_SCREEN_HEIGHT - 1) * GBA_SCREEN_WIDTH + i] = 0x0000; // Bottom border
  }
  for (i = 0; i < GBA_SCREEN_HEIGHT; i++) {
    gba_screen_pixels[i * GBA_SCREEN_WIDTH] = 0x0000; // Left border
    gba_screen_pixels[i * GBA_SCREEN_WIDTH + (GBA_SCREEN_WIDTH - 1)] = 0x0000; // Right border
  }
  
  // Generate dynamic version string - use build date in DDMMYYYY format
  char version_str[32];
  char version_reversed[32];
  
  time_t now = time(NULL);
  if (now > 0 && now != (time_t)-1) {
    // System time is available
    struct tm *local_time = localtime(&now);
    if (local_time && local_time->tm_year > 70) {
      // Valid time (after 1970) - format as DDMMYYYY
      snprintf(version_str, sizeof(version_str), "Ver %02d%02d%04d",
               local_time->tm_mday,        // Day (01-31)
               local_time->tm_mon + 1,     // Month (01-12)
               local_time->tm_year + 1900); // Full year
    } else {
      // Use build date as fallback
      const char* build_date = __DATE__;  // "Jun 23 2025"
      
      int month = 0, day = 0, year = 0;
      char month_str[4];
      sscanf(build_date, "%3s %d %d", month_str, &day, &year);
      
      // Convert month string to number
      const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                              "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
      for (int i = 0; i < 12; i++) {
        if (strncmp(month_str, months[i], 3) == 0) {
          month = i + 1;
          break;
        }
      }
      
      snprintf(version_str, sizeof(version_str), "Ver %02d%02d%04d",
               day, month, year);
    }
  } else {
    // System time not available, use build date
    const char* build_date = __DATE__;  // "Jun 23 2025"
    
    int month = 0, day = 0, year = 0;
    char month_str[4];
    sscanf(build_date, "%3s %d %d", month_str, &day, &year);
    
    // Convert month string to number
    const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    for (int i = 0; i < 12; i++) {
      if (strncmp(month_str, months[i], 3) == 0) {
        month = i + 1;
        break;
      }
    }
    
    snprintf(version_str, sizeof(version_str), "Ver %02d%02d%04d",
             day, month, year);
  }
  
  // Reverse the string for mirrored display
  int len = strlen(version_str);
  for (i = 0; i < len; i++) {
    version_reversed[i] = version_str[len - 1 - i];
  }
  version_reversed[len] = '\0';
  
  // Draw splash text - using reversed strings for mirrored display
  draw_splash_text(gba_screen_pixels, "V HSAD PSPG", 90, 40, text_color);
  draw_splash_text(gba_screen_pixels, "YTSORP YB DOM", 80, 60, accent_color);
  draw_splash_text(gba_screen_pixels, "Ksx5vHkKfvb/gg.drocsid", 20, 100, text_color);
  draw_splash_text(gba_screen_pixels, version_reversed, 80, 120, text_color);
  
  // Clean design without diagonal lines
}

static unsigned update_timers(irq_type *irq_raised, unsigned completed_cycles)
{
   unsigned i, ret = 0;
   for (i = 0; i < 4; i++)
   {
      if(timer[i].status == TIMER_INACTIVE)
         continue;

      if(timer[i].status != TIMER_CASCADE)
      {
         timer[i].count -= completed_cycles;
         /* io_registers accessors range: REG_TM0D, REG_TM1D, REG_TM2D, REG_TM3D */
         write_ioreg(REG_TMXD(i), -(timer[i].count >> timer[i].prescale));
      }

      if(timer[i].count > 0)
         continue;

      /* irq_raised value range: IRQ_TIMER0, IRQ_TIMER1, IRQ_TIMER2, IRQ_TIMER3 */
      if(timer[i].irq)
         *irq_raised |= (IRQ_TIMER0 << i);

      if((i != 3) && (timer[i + 1].status == TIMER_CASCADE))
      {
         timer[i + 1].count--;
         write_ioreg(REG_TMXD(i + 1), -timer[i+1].count);
      }

      if(i < 2)
      {
         if(timer[i].direct_sound_channels & 0x01)
            ret += sound_timer(timer[i].frequency_step, 0);

         if(timer[i].direct_sound_channels & 0x02)
            ret += sound_timer(timer[i].frequency_step, 1);
      }

      timer[i].count += (timer[i].reload << timer[i].prescale);
   }
   return ret;
}

void init_main(void)
{
  u32 i;
  for(i = 0; i < 4; i++)
  {
    timer[i].status = TIMER_INACTIVE;
    timer[i].prescale = 0;
    timer[i].irq = 0;
    timer[i].reload = timer[i].count = 0x10000;
    timer[i].direct_sound_channels = TIMER_DS_CHANNEL_NONE;
    timer[i].frequency_step = 0;
  }

  timer[0].direct_sound_channels = TIMER_DS_CHANNEL_BOTH;
  timer[1].direct_sound_channels = TIMER_DS_CHANNEL_NONE;

  frame_counter = 0;
  cpu_ticks = 0;
  execute_cycles = 960;
  video_count = 960;
  
  // Initialize splash screen state
  splash_shown = false;
  splash_timer = 0;
  first_rom_execution = false;

#ifdef HAVE_DYNAREC
  init_dynarec_caches();
  init_emitter(gamepak_must_swap());
#endif
}

u32 function_cc update_gba(int remaining_cycles)
{
  u32 changed_pc = 0;
  u32 frame_complete = 0;
  irq_type irq_raised = IRQ_NONE;
  int dma_cycles;
  trace_update_gba(remaining_cycles);

  remaining_cycles = MAX(remaining_cycles, -64);

  // Check if we should show splash screen
  if (!splash_shown && reg[REG_PC] >= 0x08000000 && reg[REG_PC] < 0x0E000000) {
    // First time executing ROM code
    if (!first_rom_execution) {
      first_rom_execution = true;
      splash_timer = 0;
    }
    
    // Show splash for about 6 seconds (360 frames at 60fps)
    if (splash_timer < 360) {
      show_custom_splash();
      splash_timer++;
      
      // Force a frame completion to display the splash
      frame_complete = 0x80000000;
      frame_counter++;
      return execute_cycles | changed_pc | frame_complete;
    } else if (!splash_shown) {
      splash_shown = true;
      // Clear the screen after splash
      extern u16* gba_screen_pixels;
      memset(gba_screen_pixels, 0, GBA_SCREEN_BUFFER_SIZE);
    }
  }

  do
  {
    unsigned i;
    // Number of cycles we ask to run - cycles that we did not execute
    // (remaining_cycles can be negative and should be close to zero)
    unsigned completed_cycles = execute_cycles - remaining_cycles;
    cpu_ticks += completed_cycles;


    remaining_cycles = 0;

    // Timers can trigger DMA (usually sound) and consume cycles
    dma_cycles = update_timers(&irq_raised, completed_cycles);
    // Check for serial port IRQs as well.
    if (update_serial(completed_cycles))
      irq_raised |= IRQ_SERIAL;

    // Video count tracks the video cycles remaining until the next event
    video_count -= completed_cycles;

    // Ran out of cycles, move to the next video area
    if(video_count <= 0)
    {
      u32 vcount = read_ioreg(REG_VCOUNT);
      u32 dispstat = read_ioreg(REG_DISPSTAT);

      // Check if we are in hrefresh (0) or hblank (1)
      if ((dispstat & 0x02) == 0)
      {
        // Transition from hrefresh to hblank
        dispstat |= 0x02;
        video_count += (272);    // hblank duration, 272 cycles

        // Check if we are drawing (0) or we are in vblank (1)
        if ((dispstat & 0x01) == 0)
        {
          u32 i;

          // Render the scan line
          if(reg[OAM_UPDATED])
            oam_update_count++;

          update_scanline();

          // Trigger the HBlank DMAs if enabled
          for (i = 0; i < 4; i++)
          {
            if(dma[i].start_type == DMA_START_HBLANK)
              dma_transfer(i, &dma_cycles);
          }
        }

        // Trigger the hblank interrupt, if enabled in DISPSTAT
        if (dispstat & 0x10)
          irq_raised |= IRQ_HBLANK;
      }
      else
      {
        // Transition from hblank to the next scan line (vdraw or vblank)
        video_count += 960;
        dispstat &= ~0x02;
        vcount++;

        if(vcount == 160)
        {
          // Transition from vrefresh to vblank
          u32 i;
          dispstat |= 0x01;

          // Reinit affine transformation counters for the next frame
          video_reload_counters();

          // Trigger VBlank interrupt if enabled
          if (dispstat & 0x8)
            irq_raised |= IRQ_VBLANK;

          // Trigger the VBlank DMAs if enabled
          for (i = 0; i < 4; i++)
          {
            if(dma[i].start_type == DMA_START_VBLANK)
              dma_transfer(i, &dma_cycles);
          }
        }
        else if (vcount == 228)
        {
          // Transition from vblank to next screen
          vcount = 0;
          dispstat &= ~0x01;

          /* If there's no cheat hook, run on vblank! */
          if (cheat_master_hook == ~0U)
             process_cheats();

/*        printf("frame update (%x), %d instructions total, %d RAM flushes\n",
           reg[REG_PC], instruction_count - last_frame, flush_ram_count);
          last_frame = instruction_count;
*/
/*          printf("%d gbc audio updates\n", gbc_update_count);
          printf("%d oam updates\n", oam_update_count); */
          gbc_update_count = 0;
          oam_update_count = 0;
          flush_ram_count = 0;

          // Force audio generation. Need to flush samples for this frame.
          render_gbc_sound();

          // We completed a frame, tell the dynarec to exit to the main thread
          frame_complete = 0x80000000;
          frame_counter++;
        }

        // Vcount trigger (flag) and IRQ if enabled
        if(vcount == (dispstat >> 8))
        {
          dispstat |= 0x04;
          if(dispstat & 0x20)
            irq_raised |= IRQ_VCOUNT;
        }
        else
          dispstat &= ~0x04;

        write_ioreg(REG_VCOUNT, vcount);
      }
      write_ioreg(REG_DISPSTAT, dispstat);
    }

    // Flag any V/H blank interrupts, DMA IRQs, Vcount, etc.
    if (irq_raised)
      flag_interrupt(irq_raised);

    // Raise any pending interrupts. This changes the CPU mode.
    if (check_and_raise_interrupts())
      changed_pc = 0x40000000;

    // Figure out when we need to stop CPU execution. The next event is
    // a video event or a timer event, whatever happens first.
    execute_cycles = MAX(video_count, 0);
    
#ifdef SF2000
    // SF2000 optimization: batch more cycles together to reduce dynarec overhead
    // Only do this when we have a reasonable chunk and no critical events pending
    if (execute_cycles > 500 && execute_cycles < 2000) {
      execute_cycles = MIN(execute_cycles * 2, 4000);  // Double the execution chunk
    }
#endif
    {
      u32 cc = serial_next_event();
      execute_cycles = MIN(execute_cycles, cc);
    }

    // If we are paused due to a DMA, cap the number of cyles to that amount.
    if (reg[CPU_HALT_STATE] == CPU_DMA) {
      u32 dma_cyc = reg[REG_SLEEP_CYCLES];
      // The first iteration is marked by bit 31 set, just do nothing now.
      if (dma_cyc & 0x80000000)
        dma_cyc &= 0x7FFFFFFF;  // Start counting DMA cycles from now on.
      else
        dma_cyc -= MIN(dma_cyc, completed_cycles);  // Account DMA cycles.

      reg[REG_SLEEP_CYCLES] = dma_cyc;
      if (!dma_cyc)
        reg[CPU_HALT_STATE] = CPU_ACTIVE;   // DMA finished, resume execution.
      else
        execute_cycles = MIN(execute_cycles, dma_cyc);  // Continue sleeping.
    }

    for (i = 0; i < 4; i++)
    {
       if (timer[i].status == TIMER_PRESCALE &&
           timer[i].count < execute_cycles)
          execute_cycles = timer[i].count;
    }
  } while(reg[CPU_HALT_STATE] != CPU_ACTIVE && !frame_complete);

  // We voluntarily limit this. It is not accurate but it would be much harder.
  dma_cycles = MIN(64, dma_cycles);
  dma_cycles = MIN(execute_cycles, dma_cycles);

  return (execute_cycles - dma_cycles) | changed_pc | frame_complete;
}

void reset_gba(void)
{
  init_memory();
  init_main();
  init_cpu();
  reset_sound();
}

#ifdef TRACE_REGISTERS
void print_regs(void)
{
  printf("R0=%08x R1=%08x R2=%08x R3=%08x "
         "R4=%08x R5=%08x R6=%08x R7=%08x "
         "R8=%08x R9=%08x R10=%08x R11=%08x "
         "R12=%08x R13=%08x R14=%08x\n",
         reg[0], reg[1], reg[2], reg[3],
         reg[4], reg[5], reg[6], reg[7],
         reg[8], reg[9], reg[10], reg[11],
         reg[12], reg[13], reg[14]);
}
#endif

bool main_check_savestate(const u8 *src)
{
  int i;
  const u8 *p1 = bson_find_key(src, "emu");
  const u8 *p2 = bson_find_key(src, "timers");
  if (!p1 || !p2)
    return false;

  if (!bson_contains_key(p1, "cpu-ticks", BSON_TYPE_INT32) ||
      !bson_contains_key(p1, "exec-cycles", BSON_TYPE_INT32) ||
      !bson_contains_key(p1, "video-count", BSON_TYPE_INT32) ||
      !bson_contains_key(p1, "sleep-cycles", BSON_TYPE_INT32))
    return false;

  for (i = 0; i < 4; i++)
  {
    char tname[2] = {'0' + i, 0};
    const u8 *p = bson_find_key(p2, tname);
    if (!p)
      return false;

    if (!bson_contains_key(p, "count", BSON_TYPE_INT32) ||
        !bson_contains_key(p, "reload", BSON_TYPE_INT32) ||
        !bson_contains_key(p, "prescale", BSON_TYPE_INT32) ||
        !bson_contains_key(p, "freq-step", BSON_TYPE_INT32) ||
        !bson_contains_key(p, "dsc", BSON_TYPE_INT32) ||
        !bson_contains_key(p, "irq", BSON_TYPE_INT32) ||
        !bson_contains_key(p, "status", BSON_TYPE_INT32))
      return false;
  }

  return true;
}

bool main_read_savestate(const u8 *src)
{
  int i;
  const u8 *p1 = bson_find_key(src, "emu");
  const u8 *p2 = bson_find_key(src, "timers");
  if (!p1 || !p2)
    return false;

  if (!(bson_read_int32(p1, "cpu-ticks", &cpu_ticks) &&
         bson_read_int32(p1, "exec-cycles", &execute_cycles) &&
         bson_read_int32(p1, "video-count", (u32*)&video_count) &&
         bson_read_int32(p1, "sleep-cycles", &reg[REG_SLEEP_CYCLES])))
    return false;

  if (!bson_read_int32(p1, "frame-count", &frame_counter))
    frame_counter = 60 * 10;  // Use "fake" 10 seconds.

  for (i = 0; i < 4; i++)
  {
    char tname[2] = {'0' + i, 0};
    const u8 *p = bson_find_key(p2, tname);

    if (!(
      bson_read_int32(p, "count", (u32*)&timer[i].count) &&
      bson_read_int32(p, "reload", &timer[i].reload) &&
      bson_read_int32(p, "prescale", &timer[i].prescale) &&
      bson_read_int32(p, "freq-step", &timer[i].frequency_step) &&
      bson_read_int32(p, "dsc", &timer[i].direct_sound_channels) &&
      bson_read_int32(p, "irq", &timer[i].irq) &&
      bson_read_int32(p, "status", &timer[i].status)))
      return false;
  }

  return true;
}

unsigned main_write_savestate(u8* dst)
{
  int i;
  u8 *wbptr, *wbptr2, *startp = dst;
  bson_start_document(dst, "emu", wbptr);
  bson_write_int32(dst, "frame-count", frame_counter);
  bson_write_int32(dst, "cpu-ticks", cpu_ticks);
  bson_write_int32(dst, "exec-cycles", execute_cycles);
  bson_write_int32(dst, "video-count", video_count);
  bson_write_int32(dst, "sleep-cycles", reg[REG_SLEEP_CYCLES]);
  bson_finish_document(dst, wbptr);

  bson_start_document(dst, "timers", wbptr);
  for (i = 0; i < 4; i++)
  {
    char tname[2] = {'0' + i, 0};
    bson_start_document(dst, tname, wbptr2);
    bson_write_int32(dst, "count", timer[i].count);
    bson_write_int32(dst, "reload", timer[i].reload);
    bson_write_int32(dst, "prescale", timer[i].prescale);
    bson_write_int32(dst, "freq-step", timer[i].frequency_step);
    bson_write_int32(dst, "dsc", timer[i].direct_sound_channels);
    bson_write_int32(dst, "irq", timer[i].irq);
    bson_write_int32(dst, "status", timer[i].status);
    bson_finish_document(dst, wbptr2);
  }
  bson_finish_document(dst, wbptr);

  return (unsigned int)(dst - startp);
}