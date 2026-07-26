/*
 *
 */
#ifndef LVM_COMMON_H
#define LVM_COMMON_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <uv.h>

#include <lvm/config.h>

#ifdef LVM_DEBUG
#   include <assert.h>
#   define LVM_ASSERT(expr) assert(expr)
#else
#   define LVM_ASSERT(expr) ((void)0)
#endif

#ifdef __cplusplus
#   define LVM_EXTERN_C extern "C"
#   define LVM_BEGIN_DECLS extern "C" {
#   define LVM_END_DECLS }
#else
#   define LVM_EXTERN_C
#   define LVM_BEGIN_DECLS
#   define LVM_END_DECLS
#endif

#if defined(__GNUC__)
#   define LVM_FORCE_INLINE __inline __attribute__((__always_inline__))
#   define LVM_LIKELY(expr) __builtin_expect((expr), 1)
#   define LVM_UNLIKELY(expr) __builtin_expect((expr), 0)
#   define LVM_EXPORT
#   define LVM_IMPORT
#elif defined(_MSC_VER)
#   define LVM_FORCE_INLINE __forceinline
#   define LVM_LIKELY(expr) (expr)
#   define LVM_UNLIKELY(expr) (expr)
#   define LVM_EXPORT __declspec(dllexport)
#   define LVM_IMPORT __declspec(dllimport)
#else
#   error unknown compiler.
#endif

#ifdef LUA_BUILD_AS_DLL
#   ifdef LVM_CORE
#       define LVM_API LVM_EXPORT
#   else
#       define LVM_API LVM_IMPORT
#   endif
#else
#   define LVM_API
#endif

#define LVM_CON(a, b) LVM_CON_(a, b)
#define LVM_CON_(a, b) a##b
#define LVM_CON3(a, b, c) LVM_CON3_(a, b, c)
#define LVM_CON3_(a, b, c) a##b##c
#define LVM_STRIZE(x) LVM_STRIZE_(x)
#define LVM_STRIZE_(x) #x

#define LVM_PMOVB(p, offset) \
    ((intptr_t)(((uint8_t*)(p)) + (offset)))
#define LVM_OFFSETOF(struct_type, member_name) \
    ((intptr_t)(&(((struct_type*)0)->member_name)))
#define LVM_MEMBEROF(p, struct_type, member_name) \
    ((struct_type*)LVM_PMOVB((p), -LVM_OFFSETOF(struct_type, member_name)))

#define LVM_VERSION_STR \
    LVM_STRIZE(LVM_VERSION_MAJOR) "." \
    LVM_STRIZE(LVM_VERSION_MINOR) "." \
    LVM_STRIZE(LVM_VERSION_PATCH)

LVM_BEGIN_DECLS

typedef struct lvm_kernel lvm_kernel_t;

typedef struct {
    int major;
    int minor;
    int patch;
} lvm_version_t;

LVM_API const lvm_version_t* lvm_version(void);

LVM_END_DECLS

#endif /* LVM_COMMON_H */
