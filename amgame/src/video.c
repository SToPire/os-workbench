#include <game.h>

#define SIDE 16
static int w, h;
extern int x, y, vx, vy, FPS;
static void init()
{
    _DEV_VIDEO_INFO_t info = {0};
    _io_read(_DEV_VIDEO, _DEVREG_VIDEO_INFO, &info, sizeof(info));
    w = info.width;
    h = info.height;
}

static void draw_tile(int x, int y, int w, int h, uint32_t color)
{
    uint32_t pixels[w * h];  // careful! stack is limited!
    _DEV_VIDEO_FBCTRL_t event = {
        .x = x,
        .y = y,
        .w = w,
        .h = h,
        .sync = 1,
        .pixels = pixels,
    };
    for (int i = 0; i < w * h; i++) {
        pixels[i] = color;
    }
    _io_write(_DEV_VIDEO, _DEVREG_VIDEO_FBCTRL, &event, sizeof(event));
}

void splash()
{
    init();
    for (int x = 0; x * SIDE <= w; x++) {
        for (int y = 0; y * SIDE <= h; y++) {
            if ((x & 1) ^ (y & 1)) {
                draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, 0xffffff);  // white
            }
        }
    }
}

void screen_update()
{
    init();
    draw_tile(0, 0, w, y, 0xffffff);
    draw_tile(x + w / 2, y + h / 2, 1, 1, 0xff00ff);
}

void game_progress()
{
    x += vx / FPS;
    y += vy / FPS;
    if (x == w / 2) vx = -vx;
}