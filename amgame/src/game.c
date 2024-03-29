#include <game.h>
#include <klib.h>

#define MAXCAR 10

int FPS = 30;
int x = 0, y = 0, vx = 30, vy = 0;
int prex, prey;
int w, h;

int bdr_w, bdr_h;
int beg_x, beg_y;
int bias;
int Gear;

int GAME_OVER;

struct carPosition {
    int prex;
    int prey;
    int x;
    int y;
} carPositions[MAXCAR];

int speed[6] = {0, 1, 2, 3, 4, 5};
int new_car;
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
        screen_update(x, y);       // 重新绘制屏幕
        next_frame += 1000 / FPS;  // 计算下一帧的时间
    }

    return 0;
}

void global_initial()
{
    _ioe_init();
    init_screen();
    _DEV_TIMER_DATE_t curtime;
    get_timeofday(&curtime);
    srand(curtime.second + 60 * curtime.minute + 3600 * curtime.hour);
    w = screen_width();
    h = screen_height();
    bdr_w = 300, bdr_h = 180;
    beg_x = (w - bdr_w) / 2;
    beg_y = (h - bdr_h) / 2;

    draw_bdr(bdr_w, bdr_h);
    draw_line(bdr_w, bdr_h);
    draw_car(beg_x + bdr_w / 4 - 5, beg_y + bdr_h - 25, 0xff0000, 0);
    carPositions[0].prex = carPositions[0].x;
    carPositions[0].prey = carPositions[0].y;

    printf("Control:WSAD\n");
}

void kbd_event(int keycode)
{
    if (keycode == _KEY_ESCAPE) _halt(0);
    if (keycode == _KEY_W || (keycode ^ 0x8000) == _KEY_W)
        if (Gear != 100) Gear += 1;
    if (keycode == _KEY_S || (keycode ^ 0x8000) == _KEY_S)
        if (Gear != 0) Gear -= 1;
    if (keycode == _KEY_D || (keycode ^ 0x8000) == _KEY_D) {
        if (carPositions[0].x + speed[Gear / 20] <= beg_x + bdr_w - 15)
            carPositions[0].x += speed[Gear / 20];
        else
            GAME_OVER = 1;
    }
    if (keycode == _KEY_A || (keycode ^ 0x8000) == _KEY_A) {
        if (carPositions[0].x - speed[Gear / 20] >= beg_x + 1)
            carPositions[0].x -= speed[Gear / 20];
        else
            GAME_OVER = 1;
    }
}

int crash(int i, int j)
{
    if (carPositions[j].x >= carPositions[i].x && carPositions[j].x <= carPositions[i].x + 13 && carPositions[0].y >= carPositions[i].y && carPositions[0].y <= carPositions[i].y + 18)
        return 1;
    if (carPositions[j].x + 13 >= carPositions[i].x && carPositions[j].x + 13 <= carPositions[i].x + 13 && carPositions[0].y >= carPositions[i].y && carPositions[0].y <= carPositions[i].y + 18)
        return 1;
    if (carPositions[j].x >= carPositions[i].x && carPositions[j].x <= carPositions[i].x + 13 && carPositions[0].y + 18 >= carPositions[i].y && carPositions[0].y + 18 <= carPositions[i].y + 18)
        return 1;
    if (carPositions[j].x + 13 >= carPositions[i].x && carPositions[j].x + 13 <= carPositions[i].x + 13 && carPositions[0].y + 18 >= carPositions[i].y && carPositions[0].y + 18 <= carPositions[i].y + 18)
        return 1;
    return 0;
}

void screen_update()
{
    if (GAME_OVER) {
        printf("You die!\n");
        for (int i = 0; i < w; i += 10)
            for (int j = 0; j < h; j += 10)
                draw_tile(i, j, 10, 10, 0xff0000);
    } else {
        int unit_length = bdr_h / 10;
        draw_tile(beg_x + bdr_w / 4, beg_y, 1, bdr_h + unit_length, 0x000000);
        draw_tile(beg_x + bdr_w / 2, beg_y, 1, bdr_h + unit_length, 0x000000);
        draw_tile(beg_x + 3 * bdr_w / 4, beg_y, 1, bdr_h + unit_length, 0x000000);

        for (int i = 1; i <= 9; i += 2) {
            draw_tile(beg_x + bdr_w / 4, beg_y + (bias + unit_length * i) % bdr_h, 1, unit_length, 0xffffff);
            draw_tile(beg_x + bdr_w / 4, beg_y + bdr_h, 1, unit_length, 0x000000);
            draw_tile(beg_x + bdr_w / 2, beg_y + (bias + unit_length * i) % bdr_h, 1, unit_length, 0xffffff);
            draw_tile(beg_x + bdr_w / 2, beg_y + bdr_h, 1, unit_length, 0x000000);
            draw_tile(beg_x + 3 * bdr_w / 4, beg_y + (bias + unit_length * i) % bdr_h, 1, unit_length, 0xffffff);
            draw_tile(beg_x + 3 * bdr_w / 4, beg_y + bdr_h, 1, unit_length, 0x000000);
        }

        draw_car(carPositions[0].prex, carPositions[0].prey, 0x000000, 0);
        draw_car(carPositions[0].x, carPositions[0].y, 0xff0000, 0);

        if (new_car == 1) {
            new_car = 0;
            for (int i = 1; i < MAXCAR; i++) {
                if (carPositions[i].x == 0) {
                    carPositions[i].x = beg_x + rand() % (bdr_w - 15) + 1;
                    carPositions[i].y = beg_y + 1;
                    draw_car(carPositions[i].x, carPositions[i].y, 0x0000ff, i);
                    break;
                }
            }
        }

        for (int i = 1; i < MAXCAR; i++) {
            if (carPositions[i].x != 0) {
                draw_car(carPositions[i].prex, carPositions[i].prey, 0x000000, i);
                draw_car(carPositions[i].x, carPositions[i].y, 0x0000ff, i);
            }
        }
    }
}


void game_progress()
{
    bias += speed[Gear / 20];
    if (rand() % 10 == 0 && speed[Gear / 20] != 0) {
        new_car = 1;
    }
    for (int i = 1; i < MAXCAR; i++) {
        if (carPositions[i].x != 0) {
            if (crash(i,0)) {
                GAME_OVER = 1;
                break;
            }
            int new_x = carPositions[i].x  + 3 * (rand() % 3 - 1);
            if(new_x <=beg_x+bdr_w-1 && new_x >=beg_x && carPositions[i].y <=beg_y+bdr_h/2)
                carPositions[i].x = new_x;
            int new_y = carPositions[i].y + speed[Gear / 20] - 1;
            if (new_y <= beg_y + bdr_h - 1 && new_y >= beg_y)
                carPositions[i].y = new_y;
            else {
                draw_car(carPositions[i].x, carPositions[i].y, 0x000000, i);
                carPositions[i].x = 0;
                carPositions[i].y = 0;
                carPositions[i].prex = 0;
                carPositions[i].prey = 0;
        }
        }
    }
}
