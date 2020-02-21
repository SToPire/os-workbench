#include <am.h>
#include <amdev.h>

void kbd_event(int keycode);
void game_progress();
void screen_update();

void splash();
void print_key();
static inline void puts(const char *s) {
  for (; *s; s++) _putc(*s);
}
