#if !defined(POSIX_INPUT_H) && !defined _WIN32
#define POSIX_INPUT_H


    #ifdef __linux__

    #include <fcntl.h>
    #include <unistd.h>
    #include <linux/uinput.h>

    void destroy_virtual_mouse(int fd);
    int create_virtual_mouse(const char *name);
    void emit(int fd, int type, int code, int value);


    #elif __APPLE__

    #include <ApplicationServices/ApplicationServices.h>

    void emit_mouse(CGEventType type, CGPoint mouse_pos, CGMouseButton button) {

    #endif


#endif
