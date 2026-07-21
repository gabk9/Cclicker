#ifndef UTILS_H
#define UTILS_H

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

#define CPS_FLAG "cps"
#define DELAY_FLAG "delay"
#define DURATION_FLAG "duration"

#define PROJ_NAME "Cclicker"

#define MAX_DURATION (86400.0 * 365.0) // 1 year


enum errors {
    // other functions
    INVALID_ARG_VALUE = -1,

    // main()
    INVALID_ARG,
    DISPLAY_ERR,
    INVALID_DELAY
};

#if !defined(_WIN32) && !defined(__APPLE__)

typedef enum display_server {
    DISPLAY_NONE,
    DISPLAY_X11,
    DISPLAY_WAYLAND
} display;

display get_display_server(void);
#endif


void sleepS(double sec);
double get_arg_value(const char *arg);
int is_arg(const char *src, const char *arg);


#endif
