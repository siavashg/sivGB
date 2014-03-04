#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG 1
#ifdef DEBUG
#define print_debug(...) \
    do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): ", __FILE__, __LINE__, \
                            __func__); \
                    fprintf(stderr, __VA_ARGS__); } while(0)
#else
#define print_debug(...) do { } while(0)
#endif

#endif // DEBUG_H
