#ifndef __INJECT__H__
#define __INJECT__H__

#include <stdio.h>

#define TYPE_TO_DECLARE_0(...) void 
#define TYPE_TO_DECLARE_1(t1) t1 _a
#define TYPE_TO_DECLARE_2(t1, t2) TYPE_TO_DECLARE_1(t1), t2 _b
#define TYPE_TO_DECLARE_3(t1, t2, t3) TYPE_TO_DECLARE_2(t1, t2), t3 _c
#define TYPE_TO_DECLARE_4(t1, t2, t3, t4) TYPE_TO_DECLARE_3(t1, t2, t3), t4 _d
#define TYPE_TO_DECLARE_5(t1, t2, t3, t4, t5) TYPE_TO_DECLARE_4(t1, t2, t3, t4), t5 _e
#define TYPE_TO_DECLARE_6(t1, t2, t3, t4, t5, t6) TYPE_TO_DECLARE_5(t1, t2, t3, t4, t5), t6 _f
#define TYPE_TO_DECLARE(nr, ...) TYPE_TO_DECLARE_##nr(__VA_ARGS__)


#define TYPE_TO_ARGS_0(...) 
#define TYPE_TO_ARGS_1(t1) _a
#define TYPE_TO_ARGS_2(t1, t2) TYPE_TO_ARGS_1(t1), _b
#define TYPE_TO_ARGS_3(t1, t2, t3) TYPE_TO_ARGS_2(t1, t2), _c
#define TYPE_TO_ARGS_4(t1, t2, t3, t4) TYPE_TO_ARGS_3(t1, t2, t3), _d
#define TYPE_TO_ARGS_5(t1, t2, t3, t4, t5) TYPE_TO_ARGS_4(t1, t2, t3, t4), _e
#define TYPE_TO_ARGS_6(t1, t2, t3, t4, t5, t6) TYPE_TO_ARGS_5(t1, t2, t3, t4, t5), _f
#define TYPE_TO_ARGS(nr, ...) TYPE_TO_ARGS_##nr(__VA_ARGS__)


#define PRINT_ARG(t, a) print(#t, (&(a)))
#define PRINT_ARGS_0(...)
#define PRINT_ARGS_1(sep, t1) {PRINT_ARG(t1, _a);}
#define PRINT_ARGS_2(sep, t1, t2) {PRINT_ARGS_1(sep, t1); mprintf(sep); PRINT_ARG(t2, _b);}
#define PRINT_ARGS_3(sep, t1, t2, t3) {PRINT_ARGS_2(sep, t1, t2); mprintf(sep); PRINT_ARG(t3, _c);}
#define PRINT_ARGS_4(sep, t1, t2, t3, t4) {PRINT_ARGS_3(sep, t1, t2, t3); mprintf(sep); PRINT_ARG(t4, _d);}
#define PRINT_ARGS_5(sep, t1, t2, t3, t4, t5) {PRINT_ARGS_4(sep, t1, t2, t3, t4); mprintf(sep); PRINT_ARG(t5, _e);}
#define PRINT_ARGS_6(sep, t1, t2, t3, t4, t5, t6) {PRINT_ARGS_5(sep, t1, t2, t3, t4, t5); mprintf(sep); PRINT_ARG(t6, _f);}
#define PRINT_ARGS(nr, sep, ...) PRINT_ARGS_##nr(sep, __VA_ARGS__)


#define MONITOR(name, nr, ...) { \
    mprintf("[monitor] "#name"("); \
    PRINT_ARGS(nr, ", ", __VA_ARGS__); \
    mprintf(")"); \
}

#define inject(before, type, name, nr, ...) \
type name(TYPE_TO_DECLARE(nr, __VA_ARGS__)) { \
    static type (*old_##name)(__VA_ARGS__) = NULL; \
    if(!old_##name) { \
        old_##name = (type(*)(__VA_ARGS__))load_sym(#name); \
    } \
    if(before) MONITOR(name, nr, __VA_ARGS__); \
    type res = 0; \
    res = old_##name(TYPE_TO_ARGS(nr, __VA_ARGS__)); \
    if(!before) MONITOR(name, nr, __VA_ARGS__); \
    mprintf(" = "); \
    PRINT_ARG(type, res); \
    mprintf("\n"); \
    return res; \
}

#define inject_no_ret(before, name, nr, ...) \
void name(TYPE_TO_DECLARE(nr, __VA_ARGS__)) { \
    static void (*old_##name)(__VA_ARGS__) = NULL; \
    if(!old_##name) { \
        old_##name = (void(*)(__VA_ARGS__))load_sym(#name); \
    } \
    if(before) { \
        MONITOR(name, nr, __VA_ARGS__); \
        mprintf("\n"); \
    } \
    old_##name(TYPE_TO_ARGS(nr, __VA_ARGS__)); \
    if(!before) { \
        MONITOR(name, nr, __VA_ARGS__); \
        mprintf("\n"); \
    } \
}



FILE *get_output_fd();
void close_output_fd(FILE*);
void mprintf(const char*, ...);
void *load_sym(const char*);
typedef int fd_t;

int fork();
int execl(const char *path, const char *arg, ...);
int execle(const char *path, const char *arg, ...);
int execlp(const char *file, const char *arg, ...);
int system(const char *command);
fd_t open(const char *pathname, int flags, ...);

#endif
