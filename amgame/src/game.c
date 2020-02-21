#include <game.h>
#include <klib.h>

#define FPS 30

void kbd_event(int keycode);

// Operating system is a C program!
int main(const char* args)
{
    _ioe_init();

    // puts("mainargs = \"");
    // puts(args); // make run mainargs=xxx
    // puts("\"\n");

    // splash();
    // puts("Press any key to see its key code...\n");
    // while (1) {
    //     print_key();
    // }

    int next_frame = 0;
    int key;
    while (1) {
        while (uptime() < next_frame)
            ;  // 等待一帧的到来
        while ((key = read_key()) != _KEY_NONE) {
            kbd_event(key);  // 处理键盘事件
        }
        //game_progress();           // 处理一帧游戏逻辑，更新物体的位置等
        //screen_update();           // 重新绘制屏幕
        next_frame += 1000 / FPS;  // 计算下一帧的时间
    }

    return 0;
}

void kbd_event(int keycode)
{
#define KEYNAME(key) \
    [_KEY_##key] = #key,
    static const char* key_names[] = {
        _KEYS(KEYNAME)};
    printf("You have just pressed key: %s\n", key_names[keycode]);
}