#ifndef CCLICKER_h
#define CCLICKER_h

#include "types.h"

extern long total_clicks;
extern double elapsed;

typedef struct Cclicker Cclicker;

Cclicker *cclicker_create(void);
void cclicker_start(Cclicker *cc);
void cclicker_destroy(Cclicker *cc);
void cclicker_set_cps(Cclicker *cc, double cps);
void cclicker_set_delay_s(Cclicker *cc, double delay_s);
void cclicker_set_button(Cclicker *cc, m_button button);
void cclicker_set_duration_s(Cclicker *cc, double duration_s);

#endif