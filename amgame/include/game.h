#include <am.h>
#include <amdev.h>

void global_initial();
void kbd_event(int keycode);
void game_progress();
void screen_update();

void init_screen();
void draw_tile(int x, int y, int w, int h, uint32_t color);

void draw_bdr(int bdr_w, int bdr_h);
void draw_line(int bdr_w, int bdr_h);
void draw_car(int car_x, int car_y, uint32_t color);

void print_key();
static inline void puts(const char *s) {
  for (; *s; s++) _putc(*s);
}
