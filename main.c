#define _GNU_SOURCE

#include <math.h>
#include <stdio.h>
#include "includes/utils.h"

#ifndef _WIN32
    #include <time.h>
    #include <string.h>
#endif

int main(int argc, char **argv) {

    if (argc == 1) {
        fprintf(stderr, "%s: missing parameters\n", PROJ_NAME);
        return INVALID_ARG;
    }

    double delay_sec = NAN;
    double duration_sec = NAN;

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

            duration_sec = val;
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

            delay_sec = val;
        } else {
            argv[i][strcspn(argv[i], "=")] = '\0';

            fprintf(stderr, "%s: unknown flag: '%s'\n",
                PROJ_NAME, argv[i]);

            return INVALID_ARG;
        }
    }

    if (isnan(duration_sec)) {
        fprintf(stderr, "%s: %s flag out of use\n",
            PROJ_NAME, DURATION_FLAG);

        return INVALID_ARG;
    }

    if (isnan(delay_sec)) {
        fprintf(stderr, "%s: %s flag out of use\n",
            PROJ_NAME, DELAY_FLAG);

        return INVALID_ARG;
    }

    if (delay_sec > duration_sec) {
        fprintf(stderr, "%s: the delay must be less than the duration\n",
            PROJ_NAME);

        return INVALID_DELAY;
    }

#ifdef _WIN32
    LARGE_INTEGER qpc_freq, qpc_start, qpc_now;
    QueryPerformanceFrequency(&qpc_freq);
    QueryPerformanceCounter(&qpc_start);
#else
    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);
#endif

    double elapsed = 0.0;

#ifdef __linux__
    const char *platform;
    switch (get_display_server()) {
        case DISPLAY_X11:
            platform = "linux/x11";
            break;
        case DISPLAY_WAYLAND: 
            platform = "linux/wayland";
            break;
        default:
            printf("%s: display protocol not recognized\n", PROJ_NAME);
            return DISPLAY_ERR;
    }
#elif defined(__APPLE__)
    const char *platform;
    platform = "darwin (MacOS)";
#endif

    for (;;) {
    #ifdef _WIN32
        QueryPerformanceCounter(&qpc_now);
        elapsed = (qpc_now.QuadPart - qpc_start.QuadPart) / (double)qpc_freq.QuadPart;
    #else
        clock_gettime(CLOCK_MONOTONIC, &now);
        elapsed = (now.tv_sec - start.tv_sec) + (now.tv_nsec - start.tv_nsec) / 1e9;
    #endif

        double remaining = duration_sec - elapsed;

        if (remaining <= 0.0)
            break;

    #ifdef _WIN32
        INPUT input[2] = {
            {.type = INPUT_MOUSE, .mi.dwFlags = MOUSEEVENTF_LEFTDOWN},
            {.type = INPUT_MOUSE, .mi.dwFlags = MOUSEEVENTF_LEFTUP}
        };

        SendInput(ARRAYSIZE(input), input, sizeof(*input));
    #else
        printf("Doing something for %.2lf seconds while delaying %.2f seconds on %s\n",
            duration_sec, delay_sec, platform);
    #endif

        sleepS(remaining < delay_sec ? remaining : delay_sec);
    }

#ifdef _WIN32
    printf("%s: clicked with the left mouse button %ld times\n", PROJ_NAME, (long int)trunc(duration_sec / delay_sec));
#endif

    return 0;
}
