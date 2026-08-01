
#ifdef __linux__
    #define _POSIX_C_SOURCE 200809L
    #include <time.h>
    #include "posix_input.h"
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
#endif

#include <math.h>
#include "utils.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const b_info *get_button(m_button btn) {
    static const b_info buttons[] = {

    #ifdef _WIN32
        { .down = MOUSEEVENTF_LEFTDOWN,   .up = MOUSEEVENTF_LEFTUP   },
        { .down = MOUSEEVENTF_RIGHTDOWN,  .up = MOUSEEVENTF_RIGHTUP  },
        { .down = MOUSEEVENTF_MIDDLEDOWN, .up = MOUSEEVENTF_MIDDLEUP },
    #endif

    #ifdef __linux__
        { .code = BTN_LEFT   },
        { .code = BTN_RIGHT  },
        { .code = BTN_MIDDLE },
    #endif

    #ifdef __APPLE__
        {
            .down = kCGEventLeftMouseDown,
            .up   = kCGEventLeftMouseUp,
            .code = kCGMouseButtonLeft 
        },

        { 
            .down = kCGEventRightMouseDown,
            .up   = kCGEventRightMouseUp,
            .code = kCGMouseButtonRight 
        },

        {
            .down = kCGEventOtherMouseDown,
            .up   = kCGEventOtherMouseUp,
            .code = (CGMouseButton)2 
        },
    #endif
    };

    if (btn < LEFT_BTN || btn > MID_BTN)
        return NULL;

    return &buttons[btn - 1];
}

int is_running_in_wsl(void) {
    if (getenv("WSL_DISTRO_NAME") || getenv("WSL_INTEROP") ||
        getenv("WSLENV")) {
        return 1;
    }

    FILE *f = fopen("/proc/sys/kernel/osrelease", "r");
    if (f) {
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), f)) {
            if (strstr(buffer, "microsoft") ||strstr(buffer, "Microsoft") ||
                strstr(buffer, "wsl")) {
                fclose(f);
                return 1;
            }
        }
        fclose(f);
    }
    return 0;
}

void get_platform(char *buff, size_t size) {
#ifdef __linux__

    if (is_running_in_wsl()) {
        snprintf(buff, size, "linux/wsl");
        return;
    }

    switch (get_display_server()) {
        case DISPLAY_X11:
            snprintf(buff, size, "linux/x11");
            break;
        case DISPLAY_WAYLAND: 
            snprintf(buff, size, "linux/wayland");
            break;
        default:
            snprintf(buff, size, "linux");
    }
#elif defined(__APPLE__)
    snprintf(buff, size, "darwin (macOS)");
#elif defined(_WIN32)
    snprintf(buff, size, "windows");
#endif
}

void print_report(long total, double elapsed, m_info info) {

    char platform[16];
    get_platform(platform, sizeof(platform));

    double actual_cps = (double)total / elapsed;

    long theor_total = (long)(info.duration_s / info.delay_s);
    double theor_cps = (double)theor_total / info.duration_s;

    long lost_clicks = theor_total - total;
    double lost_cps = theor_cps - actual_cps;

    double efficiency = (double)total / theor_total * 100.0;
    double cps_accuracy = isnan(info.cps)
        ? (actual_cps / theor_cps) * 100.0
        : (actual_cps / info.cps) * 100.0;

    printf("\n");
    printf("=========================================\n");
    printf(" %s (%s)\n", PROJ_NAME, platform);
    printf("=========================================\n");

    printf("\nConfiguration\n");
    printf("-------------\n");
    printf(" Duration     : %.3f s\n", info.duration_s);
    printf(" Delay        : %.6f s\n", info.delay_s);

    if (!isnan(info.cps))
        printf(" Target CPS   : %.2f\n", info.cps);
    else
        printf(" Target CPS   : %.2f (from delay)\n", theor_cps);

    printf("\nResults\n");
    printf("-------\n");
    printf(" Runtime      : %.3f s\n", elapsed);
    printf(" Clicks       : %ld\n", total);
    printf(" Actual CPS   : %.2f\n", actual_cps);

    printf("\nAnalysis\n");
    printf("--------\n");
    printf(" Maximum      : %ld clicks (%.2f CPS)\n", theor_total, theor_cps);
    printf(" Difference   : %ld clicks (%.2f CPS)\n", lost_clicks, lost_cps);
    printf(" Accuracy     : %.2f%%\n", cps_accuracy);
    printf(" Efficiency   : %.2f%%\n", efficiency);
}

int manage_argv(char **argv, int argc, m_info *info) {
    int btn_set = 0;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-' || argv[i][1] != '-') {
            fprintf(stderr, "%s: invalid argument: '%s'\n",
                PROJ_NAME, argv[i]);

            return INVALID_ARG;
        }

        if (is_arg(argv[i] + 2, DURATION_FLAG)) {
            double val = get_arg_value(argv[i]);

            if (isnan(val) || val == (double)INVALID_ARG_VALUE) {
                fprintf(stderr, "%s: invalid value for '%s' flag\n",
                    PROJ_NAME, DURATION_FLAG);

                return INVALID_ARG;
            }

            if (val <= 0.0 || val > MAX_DURATION) {
                fprintf(stderr, "%s: invalid duration amount\n",
                    PROJ_NAME);

                return INVALID_ARG;
            }

            info->duration_s = val;
        } else if (is_arg(argv[i] + 2, DELAY_FLAG)) {
            double val = get_arg_value(argv[i]);

            if (isnan(val) || val == (double)INVALID_ARG_VALUE) {
                fprintf(stderr, "%s: invalid value for %s flag\n",
                    PROJ_NAME, DELAY_FLAG);

                return INVALID_ARG;
            }

            if (val <= 0.0) {
                fprintf(stderr, "%s: invalid delay amount\n",
                    PROJ_NAME);

                return INVALID_ARG;
            }

            info->delay_s = val;
        } else if (is_arg(argv[i] + 2, CPS_FLAG)) {
            double val = get_arg_value(argv[i]);

            if (isnan(val) || val == (double)INVALID_ARG_VALUE) {
                fprintf(stderr, "%s: invalid value for %s flag\n",
                    PROJ_NAME, CPS_FLAG);

                return INVALID_ARG;
            }

            if (val <= 0.0) {
                fprintf(stderr, "%s: invalid cps amount\n",
                    PROJ_NAME);

                return INVALID_ARG;
            }

            info->cps = val;
        } else if (is_arg(argv[i] + 2, BUTTON_FLAG)) {
            double val = get_arg_value(argv[i]);

            if (isnan(val) || val == (double)INVALID_ARG_VALUE) {
                fprintf(stderr, "%s: invalid value for %s flag\n",
                    PROJ_NAME, BUTTON_FLAG);

                return INVALID_ARG;
            }

            if (val < LEFT_BTN || val > MID_BTN) {
                fprintf(stderr, "%s: button value out of range. must be: >= %d and <= %d\n",
                    PROJ_NAME, LEFT_BTN, MID_BTN);

                return INVALID_ARG;
            }

            if (val != trunc(val)) {
                fprintf(stderr, "%s: button value must be an integer\n",
                    PROJ_NAME);

                return INVALID_ARG;
            }

            const b_info *b = get_button((m_button)val);

            if (!b)
                return INVALID_ARG;

            info->button = *b;

            btn_set = 1;
        } else {
            argv[i][strcspn(argv[i], "=")] = '\0';

            fprintf(stderr, "%s: unknown flag: '%s'\n",
                PROJ_NAME, argv[i]);

            return INVALID_ARG;
        }

        if (!btn_set) {
            const b_info *b = get_button(LEFT_BTN);
            info->button = *b;
        }
    }

    return EXIT_SUCCESS;
}

void sleepS(double sec) {
    if (isnan(sec) || sec < 0.0)
        return;

#ifdef _WIN32
    HANDLE timer;
    LARGE_INTEGER ft;

    ft.QuadPart = -(LONGLONG)(sec * 1e7);
    timer = CreateWaitableTimer(NULL, TRUE, NULL);

    if (timer != NULL) {
        SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
        WaitForSingleObject(timer, INFINITE);
        CloseHandle(timer);
    }
#else

    double frac = sec - (time_t)sec;

    struct timespec req = {
        .tv_sec = (time_t)sec,
        .tv_nsec = (long)(frac * 1e9)
    };

    if (req.tv_nsec >= 1000000000L) {
        req.tv_sec++;
        req.tv_nsec -= 1000000000L;
    }

    while (nanosleep(&req, &req) == -1 && errno == EINTR);

#endif
}

double get_arg_value(const char *arg) {
    const char *val = strchr(arg, '=');

    if (!val)
        return NAN;

    char *endptr = NULL;

    errno = 0;

    double result = strtod(val + 1, &endptr);


    if (endptr == val + 1 || *endptr != '\0')
        return (double)INVALID_ARG_VALUE;

    if (errno == ERANGE)
        return NAN;

    return result;
}

int is_arg(const char *src, const char *arg) {
    char cpy[64];
    strncpy(cpy, src, sizeof(cpy) - 1);

    cpy[sizeof(cpy) - 1] = '\0';

    char *eq = strchr(cpy, '=');
    if (eq) *eq = '\0';

    return strcmp(cpy, arg) == 0;
}

#if !defined(_WIN32) && !defined(__APPLE__)

display get_display_server(void) {
    const char *session = getenv("XDG_SESSION_TYPE");

    if (session) {
        if (strcmp(session, "wayland") == 0)
            return DISPLAY_WAYLAND;

        if (strcmp(session, "x11") == 0)
            return DISPLAY_X11;
    }

    if (getenv("WAYLAND_DISPLAY"))
        return DISPLAY_WAYLAND;

    if (getenv("DISPLAY"))
        return DISPLAY_X11;

    return DISPLAY_NONE;
}

#endif