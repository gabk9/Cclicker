#ifndef MSVC_COMPAT_H
#define MSVC_COMPAT_H

#ifdef _MSC_VER
    #include <BaseTsd.h>
    typedef SSIZE_T ssize_t;

    #ifndef __attribute__
        #define __attribute__(x)
    #endif
#endif

#endif
