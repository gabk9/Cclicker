#define _GNU_SOURCE

#include <math.h>
#include <stdio.h>
#include "includes/utils.h"
#include "includes/posix_input.h"

#if !defined(__APPLE__) && !defined(__linux__) && !defined(_WIN32)
    #error "Error: platform not supported or not recognized"
#endif

#ifdef _WIN32
    #include <io.h>

    #define isatty _isatty
    #define STDIN_FILENO 0
#elif defined(__linux__) || defined(__APPLE__)
    #include <time.h>
#endif

#if defined(__APPLE__) && !defined(CCLICKER_HAS_APPLE_HEADERS)
    #warning "this project currently does not support macOS in this build"
#endif

int main(int argc, char **argv) {

    if (!isatty(STDIN_FILENO)) {
        puts(PROJ_NAME": interactive terminal input is not supported");
        return INVALID_ARG;
    }

    if (argc <= 1) {
        fprintf(stderr, "%s: missing parameters\n", PROJ_NAME);
        return INVALID_ARG;
    }

    m_info info = {
        .delay_s = NAN,
        .duration_s = NAN,
        .cps = NAN,
    };

    int argv_error = manage_argv(argv, argc, &info);

    if (argv_error)
        return argv_error;

    if (isnan(info.duration_s)) {
        fprintf(stderr, "%s: %s flag out of use\n",
            PROJ_NAME, DURATION_FLAG);

        return INVALID_ARG;
    }

    if (!isnan(info.delay_s) && !isnan(info.cps)) {
        fprintf(stderr,
            "%s: %s and %s flags cannot be used at the same time\n",
            PROJ_NAME, DELAY_FLAG, CPS_FLAG);

        return INVALID_ARG;
    }

    if (isnan(info.delay_s) && isnan(info.cps)) {
        fprintf(stderr,
            "%s: either %s or %s must be specified\n",
            PROJ_NAME, DELAY_FLAG, CPS_FLAG);

        return INVALID_ARG;
    }

    if (isnan(info.delay_s))
        info.delay_s = 1.0 / info.cps;

    if (info.delay_s > info.duration_s) {
        fprintf(stderr, "%s: the delay must be less than the duration\n",
            PROJ_NAME);

        return INVALID_DELAY;
    }

#ifdef _WIN32
    LARGE_INTEGER qpc_freq, qpc_start, qpc_now;
    QueryPerformanceFrequency(&qpc_freq);
    QueryPerformanceCounter(&qpc_start);
#elif defined(__linux__) || defined(__APPLE__)
    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);
#endif

#ifdef __linux__
    int fd = create_virtual_mouse(PROJ_NAME"_virtual_mouse", info.button.code);
#elif defined(__APPLE__)
    CGEventRef current = CGEventCreate(NULL);
    CGPoint pos = CGEventGetLocation(current);
    CFRelease(current);
#endif

    double elapsed = 0.0;
    long total = 0;

    for (;;) {
    #ifdef _WIN32
        QueryPerformanceCounter(&qpc_now);
        elapsed = (qpc_now.QuadPart - qpc_start.QuadPart) / (double)qpc_freq.QuadPart;
    #elif defined(__linux__) || defined(__APPLE__)
        clock_gettime(CLOCK_MONOTONIC, &now);
        elapsed = (now.tv_sec - start.tv_sec) + (now.tv_nsec - start.tv_nsec) / 1e9;
    #endif

        double remaining = info.duration_s - elapsed;

        if (remaining <= 0.0)
            break;

    #ifdef _WIN32
        INPUT input[] = {
            {.type = INPUT_MOUSE, .mi.dwFlags = info.button.down},
            {.type = INPUT_MOUSE, .mi.dwFlags = info.button.up}
        };

        SendInput(ARRAYSIZE(input), input, sizeof(*input));
    #elif __linux__
        emit(fd, EV_KEY, info.button.code, 1);
        emit(fd, EV_SYN, SYN_REPORT, 0);

        emit(fd, EV_KEY, info.button.code, 0);
        emit(fd, EV_SYN, SYN_REPORT, 0);    
    #elif __APPLE__
        emit_mouse(info.button.down, pos, info.button.btn);
        emit_mouse(info.button.up, pos, info.button.btn);
    #endif

        total++;

        sleepS(remaining < info.delay_s ? remaining : info.delay_s);
    }

#ifdef __linux__
    destroy_virtual_mouse(fd);
#endif

    print_report(total, elapsed, info);

    return 0;
}
