#define _GNU_SOURCE

#include "types.h"
#include "utils.h"
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include "cclicker.h"

#ifdef _WIN32
    #include <stdlib.h>
    #include <windows.h>
#elif defined(__linux__) || defined(__APPLE__)
    #include <time.h>
    #include "posix_input.h"
#endif

long total_clicks = 0;
double elapsed = 0.0;

struct Cclicker {
    double duration_s;
    double delay_s;
    m_button btn;
};

Cclicker *cclicker_create(void) {
    static Cclicker cc = {
        .duration_s = NAN,
        .delay_s = NAN,
        .btn = LEFT_BTN
    };

    return &cc;
}

void cclicker_set_cps(Cclicker *cc, double cps) {
    cc->delay_s = 1.0 / cps;
}

void cclicker_set_delay_s(Cclicker *cc, double delay_s) {
    cc->delay_s = delay_s;
}

void cclicker_set_duration_s(Cclicker *cc, double duration_s) {
    cc->duration_s = duration_s;
}

void cclicker_set_button(Cclicker *cc, m_button button) {
    cc->btn = button;
}

void cclicker_start(Cclicker *cc) {
    if (isnan(cc->delay_s)) {
        fprintf(stderr, "Cclicker: the delay/cps was not set\n");
        return;
    }

    if (isnan(cc->duration_s)) {
        fprintf(stderr, "Cclicker: the duration was not set\n");
        return;
    }

    const b_info *info;
    info = get_button(cc->btn);

#ifdef _WIN32
    LARGE_INTEGER qpc_freq, qpc_start, qpc_now;
    QueryPerformanceFrequency(&qpc_freq);
    QueryPerformanceCounter(&qpc_start);
#elif defined(__linux__) || defined(__APPLE__)
    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);
#endif

#ifdef __linux__
    int fd = create_virtual_mouse("Cclicker_virtual_mouse", info->code);
#elif defined(__APPLE__)
    CGEventRef current = CGEventCreate(NULL);
    CGPoint pos = CGEventGetLocation(current);
    CFRelease(current);
#endif

    for (;;) {
    #ifdef _WIN32
        QueryPerformanceCounter(&qpc_now);
        elapsed = (qpc_now.QuadPart - qpc_start.QuadPart) / (double)qpc_freq.QuadPart;
    #elif defined(__linux__) || defined(__APPLE__)
        clock_gettime(CLOCK_MONOTONIC, &now);
        elapsed = (now.tv_sec - start.tv_sec) + (now.tv_nsec - start.tv_nsec) / 1e9;
    #endif

        double remaining = cc->duration_s - elapsed;

        if (remaining <= 0.0)
            break;

    #ifdef _WIN32
        INPUT input[] = {
            {.type = INPUT_MOUSE, .mi.dwFlags = info->down},
            {.type = INPUT_MOUSE, .mi.dwFlags = info->up}
        };

        SendInput(ARRAYSIZE(input), input, sizeof(*input));
    #elif __linux__
        emit(fd, EV_KEY, info->code, 1);
        emit(fd, EV_SYN, SYN_REPORT, 0);

        emit(fd, EV_KEY, info->code, 0);
        emit(fd, EV_SYN, SYN_REPORT, 0);    
    #elif __APPLE__
        emit_mouse(info->down, pos, info->btn);
        emit_mouse(info->up, pos, info->btn);
    #endif

        total_clicks++;

        sleepS(remaining < cc->delay_s ? remaining : cc->delay_s);
    }

#ifdef __linux__
    destroy_virtual_mouse(fd);
#endif
}
