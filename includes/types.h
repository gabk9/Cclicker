#ifndef TYPES_H
#define TYPES_H

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

#endif
