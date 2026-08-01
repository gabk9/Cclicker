#ifndef UTILS_H
#define UTILS_H

#ifdef _WIN32
    #include <windows.h>
#elif defined(__linux__) || defined(__APPLE__)
    #include <unistd.h>
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
    INVALID_ARG,
    DISPLAY_ERR,
    INVALID_DELAY
};

typedef enum mouse_button {
    LEFT_BTN = 1,
    RIGHT_BTN,
    SCR_BTN,
    BRB_BTN,
    BRF_BTN
} m_button;

typedef struct mouse_info {
    double delay_s;
    double duration_s;
    double cps;
    m_button button;
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
void get_platform(char *buff, size_t size);
int is_arg(const char *src, const char *arg);
int manage_argv(char **argv, int argc, m_info *info);
void print_report(long total, double elapsed, m_info info);


#endif
