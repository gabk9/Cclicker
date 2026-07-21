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
    double target_cps = NAN;

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

            target_cps = val;

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

    if (!isnan(delay_sec) && !isnan(target_cps)) {
        fprintf(stderr,
            "%s: %s and %s flags cannot be used at the same time\n",
            PROJ_NAME, DELAY_FLAG, CPS_FLAG);

        return INVALID_ARG;
    }

    if (isnan(delay_sec) && isnan(target_cps)) {
        fprintf(stderr,
            "%s: either %s or %s must be specified\n",
            PROJ_NAME, DELAY_FLAG, CPS_FLAG);

        return INVALID_ARG;
    }

    if (isnan(delay_sec))
        delay_sec = 1.0 / target_cps;

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

    const char *platform;
#ifdef __linux__
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
#else
    platform = "windows";
#endif

    long total = 0;

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

        total++;

        sleepS(remaining < delay_sec ? remaining : delay_sec);
    }

#ifdef _WIN32
    double actual_cps = (double)total / elapsed;

    long theor_total = (long)(duration_sec / delay_sec);
    double theor_cps = (double)theor_total / duration_sec;

    long lost_clicks = theor_total - total;
    double lost_cps = theor_cps - actual_cps;

    double efficiency = (double)total / theor_total * 100.0;
    double cps_accuracy = isnan(target_cps)
        ? (actual_cps / theor_cps) * 100.0
        : (actual_cps / target_cps) * 100.0;

    printf("\n");
    printf("=========================================\n");
    printf(" %s (%s)\n", PROJ_NAME, platform);
    printf("=========================================\n");

    printf("\nConfiguration\n");
    printf("-------------\n");
    printf(" Duration     : %.3f s\n", duration_sec);
    printf(" Delay        : %.6f s\n", delay_sec);

    if (!isnan(target_cps))
        printf(" Target CPS   : %.2f\n", target_cps);
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
#endif

    return 0;
}
