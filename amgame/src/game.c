#include <game.h>
#include <klib.h>

int FPS = 30;
int x = 0, y = 0, vx = 30, vy = 0;
int prex, prey;
int w, h;

int bdr_w, bdr_h;
int beg_x, beg_y;
int bias;
int Gear;

struct carPosition {
    int prex;
    int prey;
    int x;
    int y;
} carPositions[5];

int speed[6] = {0, 1, 2, 3, 4, 5};

// Operating system is a C program!
int main(const char* args)
{
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
    beg_x = (w - bdr_w) / 2;
    beg_y = (h - bdr_h) / 2;

    draw_bdr(bdr_w, bdr_h);
    draw_line(bdr_w, bdr_h);
    draw_car(beg_x + bdr_w / 4 - 5, beg_y + bdr_h - 25, 0xff0000);
    carPositions[0].prex = carPositions[0].x;
    carPositions[0].prey = carPositions[0].y;
}


void kbd_event(int keycode)
{
    if (keycode == _KEY_ESCAPE) _halt(0);
    if (keycode == _KEY_W || (keycode ^ 0x8000) == _KEY_W) if(Gear != 100) Gear += 1;
    if (keycode == _KEY_S || (keycode ^ 0x8000) == _KEY_S) if (Gear != 0) Gear -= 1;
    if (keycode == _KEY_D || (keycode ^ 0x8000) == _KEY_D) if(carPositions[0].x != beg_x+bdr_w-15) carPositions[0].x += speed[Gear/20];
    if (keycode == _KEY_A || (keycode ^ 0x8000) == _KEY_A) if(carPositions[0].x != beg_x + 1) carPositions[0].x -= speed[Gear/20];
}

void screen_update()
{
    int unit_length = bdr_h / 10;
    draw_tile(beg_x + bdr_w / 4, beg_y, 1, bdr_h+unit_length, 0x000000);
    draw_tile(beg_x + bdr_w / 2, beg_y, 1, bdr_h + unit_length, 0x000000);
    draw_tile(beg_x + 3 * bdr_w / 4, beg_y, 1, bdr_h + unit_length, 0x000000);

    for (int i = 1; i <= 9;i+=2){
        draw_tile(beg_x + bdr_w / 4, beg_y + (bias + unit_length * i) % bdr_h, 1, unit_length, 0xffffff);
        draw_tile(beg_x + bdr_w / 4, beg_y + bdr_h, 1, unit_length, 0x000000);
        draw_tile(beg_x + bdr_w / 2, beg_y + (bias + unit_length * i) % bdr_h, 1, unit_length, 0xffffff);
        draw_tile(beg_x + bdr_w / 2, beg_y + bdr_h, 1, unit_length, 0x000000);
        draw_tile(beg_x + 3 * bdr_w / 4, beg_y + (bias + unit_length * i) % bdr_h, 1, unit_length, 0xffffff);
        draw_tile(beg_x + 3 * bdr_w / 4, beg_y + bdr_h, 1, unit_length, 0x000000);
    }

    draw_car(carPositions[0].prex, carPositions[0].prey, 0x000000);
    draw_car(carPositions[0].x, carPositions[0].y, 0xff0000);

}

void game_progress()
{
    bias += speed[Gear/20];
}
