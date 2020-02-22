#include <game.h>
#include <klib.h>

int FPS = 30;
int x = 0, y = 0, vx = 30, vy = 0;
int prex, prey;
int w, h;

int bdr_w, bdr_h;
// Operating system is a C program!
int main(const char* args)
{
    // _ioe_init();
    // puts("mainargs = \"");
    // puts(args); // make run mainargs=xxx
    // puts("\"\n");

    // splash();
    // puts("Press any key to see its key code...\n");
    // while (1) {
    //     print_key();
    // }
    global_initial();

    int next_frame = 0;
    int key;
    while (1) {
        while (uptime() < next_frame)
            ;  // 等待一帧的到来
        while ((key = read_key()) != _KEY_NONE) {
            kbd_event(key);  // 处理键盘事件
        }
        game_progress();           // 处理一帧游戏逻辑，更新物体的位置等
        screen_update(x,y);           // 重新绘制屏幕
        next_frame += 1000 / FPS;  // 计算下一帧的时间
    }

    return 0;
}

void global_initial()
{
    _ioe_init();
    init_screen();
    w = screen_width();
    h = screen_height();
    bdr_w = 300, bdr_h = 180;
    draw_bdr(bdr_w, bdr_h);
}

void kbd_event(int keycode)
{
#define KEYNAME(key) \
    [_KEY_##key] = #key,
    static const char* key_names[] = {
        _KEYS(KEYNAME)};
    if(!(keycode & 0x8000)){
        if (keycode == _KEY_ESCAPE) _halt(0);
        printf("You have just pressed key: %s %d\n", key_names[keycode], keycode);
    }
}

void screen_update()
{
    // draw_tile(prex + w / 2, prey + h / 2, 1, 1, 0x000000);
    // draw_tile(x + w / 2, y + h / 2, 1, 1, 0xff00ff);
}

void game_progress()
{
    // prex = x;
    // prey = y;
    // x += vx / FPS;
    // y += vy / FPS;
    // if (x == w / 2) vx = -vx;
}
