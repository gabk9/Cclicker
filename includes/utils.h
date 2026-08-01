#ifndef UTILS_H
#define UTILS_H

#ifdef _WIN32
    #include <windows.h>
#elif defined(__linux__)
    #include <unistd.h>
#elif defined(__APPLE__)
    #include <unistd.h>
    #include <ApplicationServices/ApplicationServices.h>
#endif

#define CPS_FLAG "cps"
#define DELAY_FLAG "delay"
#define BUTTON_FLAG "button"
#define DURATION_FLAG "duration"

#define PROJ_NAME "Cclicker"

#define MAX_DURATION (86400.0 * 365.0) // 1 year

enum errors {
    // other functions
    INVALID_ARG_VALUE = -1,

    // main()
    INVALID_ARG = 1,
    DISPLAY_ERR,
    INVALID_DELAY
};

typedef enum mouse_button {
    LEFT_BTN = 1,
    RIGHT_BTN,
    MID_BTN,
} m_button;

typedef struct button_info {
#ifdef _WIN32
    DWORD down;
    DWORD up;
#elif __linux__
    int code;
#elif __APPLE__
    CGEventType down;
    CGEventType up;
    CGMouseButton btn;
#endif
} b_info;

typedef struct mouse_info {
    double delay_s;
    double duration_s;
    double cps;
    b_info button_values;
    m_button button_rep;
} m_info;


#if !defined(_WIN32) && !defined(__APPLE__)

typedef enum display_server {
    DISPLAY_NONE,
    DISPLAY_X11,
    DISPLAY_WAYLAND
} display;

display get_display_server(void);
#endif


void sleepS(double sec);
int is_running_in_wsl(void);
double get_arg_value(const char *arg);
const b_info *get_button(m_button btn);
void get_platform(char *buff, size_t size);
int is_arg(const char *src, const char *arg);
int manage_argv(char **argv, int argc, m_info *info);
void print_report(long total, double elapsed, m_info info);


#endif
