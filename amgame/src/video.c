#include <game.h>
#include<klib.h>
#define SIDE 16
extern int w, h;
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

void splash()
{
    init_screen();
    // for (int x = 0; x * SIDE <= w; x++) {
    //     for (int y = 0; y * SIDE <= h; y++) {
    //         if ((x & 1) ^ (y & 1)) {
    //             draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, 0xffffff);  // white
    //         }
    //     }
    // }
    for (int x = 0; x <= w;x=x+10){
        for (int y = 0; y < h;y=y+10){
            draw_tile(x, y, 10, 10, 0xff0000);
        }
    }
}

void draw_bdr(int bdr_w, int bdr_h)
{
    int beg_x = (w - bdr_w) / 2;
    int beg_y = (h - bdr_h) / 2;
    draw_tile(beg_x, beg_y, bdr_w, 1, 0xff0000);
    draw_tile(beg_x, beg_y, 1, bdr_h, 0xff0000);
    draw_tile(beg_x + bdr_w, beg_y, 1, bdr_h, 0xff0000);
    draw_tile(beg_x, beg_y + bdr_h, bdr_w, 1, 0xff0000);
}