#include <game.h>
#include <klib.h>
#define SIDE 16
extern int w, h;
extern struct carPosition {
    int prex;
    int prey;
    int x;
    int y;
} carPositions[5];
void init_screen()
{
    _DEV_VIDEO_INFO_t info = {0};
    _io_read(_DEV_VIDEO, _DEVREG_VIDEO_INFO, &info, sizeof(info));
    w = info.width;
    h = info.height;
}

void draw_tile(int x, int y, int w, int h, uint32_t color)
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

void draw_bdr(int bdr_w, int bdr_h)
{
    uint32_t bdr_color = 0xffff00;
    int beg_x = (w - bdr_w) / 2;
    int beg_y = (h - bdr_h) / 2;
    draw_tile(beg_x, beg_y, 1, bdr_h + 10, bdr_color);
    draw_tile(beg_x - 2, beg_y, 1, bdr_h + 10, bdr_color);
    draw_tile(beg_x + bdr_w, beg_y, 1, bdr_h + 10, bdr_color);
    draw_tile(beg_x + bdr_w + 2, beg_y, 1, bdr_h + 10, bdr_color);
}

void draw_line(int bdr_w, int bdr_h)
{
    uint32_t line_color = 0xffffff;
    int beg_x = (w - bdr_w) / 2;
    int beg_y = (h - bdr_h) / 2;
    int unit_length = bdr_h / 10;
    for (int i = 1; i <= 9; i += 2) {
        draw_tile(beg_x + bdr_w / 4, beg_y + unit_length * i, 1, unit_length, line_color);
        draw_tile(beg_x + bdr_w / 2, beg_y + unit_length * i, 1, unit_length, line_color);
        draw_tile(beg_x + 3 * bdr_w / 4, beg_y + unit_length * i, 1, unit_length, line_color);
    }
}

void draw_car(int car_x, int car_y, uint32_t color, int i)
{
    draw_tile(car_x + 2, car_y, 10, 20, color);
    draw_tile(car_x, car_y + 4, 2, 2, color);
    draw_tile(car_x, car_y + 15, 2, 2, color);
    draw_tile(car_x + 12, car_y + 4, 2, 2, color);
    draw_tile(car_x + 12, car_y + 15, 2, 2, color);
    if (color != 0x000000) {
        carPositions[i].prex = carPositions[i].x;
        carPositions[i].prey = carPositions[i].y;
        carPositions[i].x = car_x;
        carPositions[i].y = car_y;
    }
}