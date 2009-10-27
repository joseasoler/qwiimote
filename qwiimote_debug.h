#ifndef QWIIMOTE_DEBUG_H
#define QWIIMOTE_DEBUG_H

/* #define QT_NO_DEBUG_OUTPUT   */
/* #define QT_NO_WARNING_OUTPUT */

    #ifndef QT_NO_DEBUG_OUTPUT
        #define QW_ASSERT(boolean) Q_ASSERT(boolean)
    #else
        #define QW_ASSERT(boolean)
    #endif

#endif // QWIIMOTE_DEBUG_H
