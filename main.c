#define _GNU_SOURCE

#include <math.h>
#include <stdio.h>
#include "includes/utils.h"
#include "includes/cclicker.h"

#if !defined(__APPLE__) && !defined(__linux__) && !defined(_WIN32)
    #error "Error: platform not supported or not recognized"
#endif

#ifdef _WIN32
    #include <io.h>

    #define isatty _isatty
    #define STDIN_FILENO 0
#endif

#if defined(__APPLE__) && !defined(CCLICKER_HAS_APPLE_HEADERS)
    #warning "this project currently does not support macOS in this build"
#endif

int main(int argc, char **argv) {

    if (!isatty(STDIN_FILENO)) {
        fprintf(stderr, "Cclicker: interactive terminal input is not supported");
        return INVALID_ARG;
    }

    if (argc <= 1) {
        fprintf(stderr, "Cclicker: missing parameters");
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
        fprintf(stderr, "Cclicker: %s flag out of use\n", DURATION_FLAG);

        return INVALID_ARG;
    }

    if (!isnan(info.delay_s) && !isnan(info.cps)) {
        fprintf(stderr, "Cclicker: %s and %s flags cannot be used at the same time\n",
            DELAY_FLAG, CPS_FLAG);

        return INVALID_ARG;
    }

    if (isnan(info.delay_s) && isnan(info.cps)) {
        fprintf(stderr, "Cclicker: either %s or %s must be specified\n",
            DELAY_FLAG, CPS_FLAG);

        return INVALID_ARG;
    }

    if (isnan(info.delay_s))
        info.delay_s = 1.0 / info.cps;

    if (info.delay_s > info.duration_s) {
        fprintf(stderr, "Cclicker: the delay must be less than the duration\n");

        return INVALID_DELAY;
    }

    Cclicker *cc = cclicker_create();

    cclicker_set_duration_s(cc, info.duration_s);
    cclicker_set_delay_s(cc, info.delay_s);
    cclicker_set_button(cc, info.button_rep);

    cclicker_start(cc);

    print_report(total_clicks, elapsed, info);

    return 0;
}
