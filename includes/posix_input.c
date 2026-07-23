#ifdef __linux__

#include <string.h>
#include "posix_input.h"
#include <linux/uinput.h>

void emit(int fd, int type, int code, int value) {
    struct input_event event = {0};

    event.type = type;
    event.code = code;
    event.value = value;

    write(fd, &event, sizeof(event));
}

int create_virtual_mouse(const char *name) {
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_EVBIT, EV_SYN);

    ioctl(fd, UI_SET_KEYBIT, BTN_LEFT);

    struct uinput_setup usetup = {0};

    strcpy(usetup.name, name);
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor  = 0x1234;
    usetup.id.product = 0x5678;

    ioctl(fd, UI_DEV_SETUP, &usetup);
    ioctl(fd, UI_DEV_CREATE);

    return fd;
}

void destroy_virtual_mouse(int fd) {
    ioctl(fd, UI_DEV_DESTROY);
    close(fd);
}

#endif
