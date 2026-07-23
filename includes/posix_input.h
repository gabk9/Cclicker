#if !defined(POSIX_INPUT_H) && defined(__linux__)
#define POSIX_INPUT_H

#include <fcntl.h>
#include <unistd.h>
#include <linux/uinput.h>

void destroy_virtual_mouse(int fd);
int create_virtual_mouse(const char *name);
void emit(int fd, int type, int code, int value);

#endif
