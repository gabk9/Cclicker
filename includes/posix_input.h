#if !defined(POSIX_INPUT_H) && !defined(_WIN32)
#define POSIX_INPUT_H


#ifdef __linux__
    #include <fcntl.h>
    #include <unistd.h>
    #include <linux/uinput.h>

    void destroy_virtual_mouse(int fd);
    void emit(int fd, int type, int code, int value);
    int create_virtual_mouse(const char *name, int code);
#elif defined(__APPLE__)
    #include <ApplicationServices/ApplicationServices.h>

    void emit_mouse(CGEventType type, CGPoint mouse_pos, CGMouseButton button);
#endif


#endif
