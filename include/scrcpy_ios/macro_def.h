/**
 * NOTE: there two rules to import this file:
 * 1. IT IS OK to import this file in any "*.cpp" files.
 * 2. TRY NOT TO import this file in any "*.h/hpp" header files. If you have to do it,  please
 * import the "macro_undef.h" file at the end of the header file to avoid bringing these macro
 * definitions to other files.
 */
// clang-format off
#ifndef ISCRCPY_MACRO_DEF_H
#define ISCRCPY_MACRO_DEF_H

#include <cassert>    // assert

// DEBUG 
#ifdef NDEBUG
#define ISCRCPY_DEBUG 0
#else
#define ISCRCPY_DEBUG 1
#endif

// LOG 
#define ISCRCPY_LOG_LEVEL 2
#if ISCRCPY_LOG_LEVEL >= 0
#define ISCRCPY_LOG_E(fmt, ...) \
  fprintf(stderr, "[ERROR] %s:%d:%s(): " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define ISCRCPY_LOG_E(fmt, ...)
#endif
#if ISCRCPY_LOG_LEVEL >= 1
#define ISCRCPY_LOG_I(fmt, ...) \
  printf("[INFO] %s:%d:%s(): " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define ISCRCPY_LOG_I(fmt, ...)
#endif
#if ISCRCPY_LOG_LEVEL >= 2
#define ISCRCPY_LOG_D(fmt, ...) \
  printf("[DEBUG] %s:%d:%s(): " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define ISCRCPY_LOG_D(fmt, ...)
#endif
#if ISCRCPY_LOG_LEVEL >= 3
#define ISCRCPY_LOG_V(fmt, ...) \
  printf("[VERBOSE] %s:%d:%s(): " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define ISCRCPY_LOG_V(fmt, ...)
#endif

// MEMORY
#define ISCRCPY_MEM_ALIGN(v, a) (((v) + (a)-1) & ~((a)-1))

// ASSERT
#define ISCRCPY_ASSERT(exp, fmt, ...) if (!(exp)) { ISCRCPY_LOG_E(fmt, ##__VA_ARGS__); } assert(exp)

#define ISCRCPY_DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete;      \
  void operator=(const TypeName&) = delete

#endif // ISCRCPY_MACRO_DEF_H
// clang-format on