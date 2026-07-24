#define _GNU_SOURCE

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "includes/utils.h"
#include "includes/posix_input.h"

#ifndef _WIN32
    #include <time.h>
#endif

#ifdef __APPLE__
    #error "this project currently does not support MacOS"
#endif

int main(int argc, char **argv) {

    if (argc == 1) {
        fprintf(stderr, "%s: missing parameters\n", PROJ_NAME);
        return INVALID_ARG;
    }

    double delay_sec = NAN;
    double duration_sec = NAN;
    double target_cps = NAN;

    int argv_error = manage_argv(argv, argc, &delay_sec, &duration_sec, &target_cps);

    if (argv_error)
        exit(argv_error);

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

    int fd = create_virtual_mouse(PROJ_NAME"_virtual_mouse");
#endif

    double elapsed = 0.0;
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
        INPUT input[] = {
            {.type = INPUT_MOUSE, .mi.dwFlags = MOUSEEVENTF_LEFTDOWN},
            {.type = INPUT_MOUSE, .mi.dwFlags = MOUSEEVENTF_LEFTUP}
        };

        SendInput(ARRAYSIZE(input), input, sizeof(*input));
    #elif __linux__

        emit(fd, EV_KEY, BTN_LEFT, 1);
        emit(fd, EV_SYN, SYN_REPORT, 0);

        emit(fd, EV_KEY, BTN_LEFT, 0);
        emit(fd, EV_SYN, SYN_REPORT, 0);    
    #endif

        total++;

        sleepS(remaining < delay_sec ? remaining : delay_sec);
    }

#ifdef __linux__
    destroy_virtual_mouse(fd);
#endif

    const double info[] = {
        elapsed,
        duration_sec,
        delay_sec,
        target_cps
    };

    char platform[16];
    get_platform(platform, sizeof(platform));

    print_report(platform, total, info);

    return 0;
}
