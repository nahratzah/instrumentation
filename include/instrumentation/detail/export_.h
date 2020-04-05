#ifndef INSTRUMENTATION_DETAIL_EXPORT__H
#define INSTRUMENTATION_DETAIL_EXPORT__H

/*
 * Various macros to control symbol visibility in libraries.
 */
#if defined(WIN32)
# ifdef instrumentation_EXPORTS
#   define instrumentation_export_  __declspec(dllexport)
#   define instrumentation_local_   /* nothing */
# else
#   define instrumentation_export_  __declspec(dllimport)
#   define instrumentation_local_   /* nothing */
# endif
#elif defined(__GNUC__) || defined(__clang__)
# define instrumentation_export_    __attribute__ ((visibility ("default")))
# define instrumentation_local_     __attribute__ ((visibility ("hidden")))
#else
# define instrumentation_export_    /* nothing */
# define instrumentation_local_     /* nothing */
#endif

#endif /* INSTRUMENTATION_DETAIL_EXPORT__H */
