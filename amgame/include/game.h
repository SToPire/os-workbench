#include <am.h>
#include <amdev.h>

void global_initial();
void kbd_event(int keycode);
void game_progress();
void screen_update();

void init_screen();
void draw_tile(int x, int y, int w, int h, uint32_t color);

void print_key();
static inline void puts(const char *s) {
  for (; *s; s++) _putc(*s);
}
