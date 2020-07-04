#include <common.h>
#include<devices.h>

void vfs_init()
{
    device_t* sd = dev->lookup("sda");
    char buf[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    sd->ops->write(sd, 0, (void*)buf, sizeof(buf));
}

MODULE_DEF(vfs) = {
    .init = vfs_init,
};
