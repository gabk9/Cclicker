#ifndef _WIN32
    #define _POSIX_C_SOURCE 200809L
    #include <time.h>
#endif

#include <math.h>
#include "utils.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

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