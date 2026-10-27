/* Temporary VP1 workaround for zero GLX_MAX_PBUFFER_PIXELS reports.
 * Derived from the pixel-only shim tested with VP1 over SSH/XQuartz.
 * This synthetic cap is a workaround, not a statement of driver capacity.
 */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <GL/glx.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef void *(*lookup_fn)(void *, const char *);
typedef __GLXextFuncPtr (*resolver_fn)(const GLubyte *);
typedef int (*attribute_fn)(Display *, GLXFBConfig, int, int *);
static _Atomic(attribute_fn) attribute_real;
static _Atomic(resolver_fn) resolver_real, resolver_arb_real;
static atomic_flag reported = ATOMIC_FLAG_INIT;

static lookup_fn original_dlsym(void)
{
#if defined(__aarch64__)
    const char *version = "GLIBC_2.17";
#elif defined(__x86_64__)
    const char *version = "GLIBC_2.2.5";
#else
#error "This workaround supports Linux glibc on aarch64 and x86_64 only"
#endif
    lookup_fn fn = (lookup_fn)dlvsym(RTLD_NEXT, "dlsym", version);
    if (!fn) {
        static const char message[] = "vp1 GLX workaround: cannot resolve glibc dlsym\n";
        (void)write(STDERR_FILENO, message, sizeof(message) - 1);
        _exit(127);
    }
    return fn;
}

int glXGetFBConfigAttrib(Display *display, GLXFBConfig config, int attribute, int *value)
{
    attribute_fn fn = atomic_load(&attribute_real);
    if (!fn) {
        fn = (attribute_fn)original_dlsym()(RTLD_NEXT, "glXGetFBConfigAttrib");
        if (!fn) return GLX_BAD_ATTRIBUTE;
        atomic_store(&attribute_real, fn);
    }
    int result = fn(display, config, attribute, value);
    if (result == Success && attribute == GLX_MAX_PBUFFER_PIXELS && value && *value == 0) {
        *value = 4096 * 4096;
        if (!atomic_flag_test_and_set(&reported))
            fprintf(stderr, "[vp1-glx-workaround] GLX_MAX_PBUFFER_PIXELS: 0 -> %d\n", *value);
    }
    return result;
}

static __GLXextFuncPtr resolve(resolver_fn fn, const GLubyte *name)
{
    if (!fn) return NULL;
    __GLXextFuncPtr result = fn(name);
    if (result && name && strcmp((const char *)name, "glXGetFBConfigAttrib") == 0) {
        if (result != (__GLXextFuncPtr)glXGetFBConfigAttrib)
            atomic_store(&attribute_real, (attribute_fn)result);
        return (__GLXextFuncPtr)glXGetFBConfigAttrib;
    }
    return result;
}

static __GLXextFuncPtr shimGetProcAddress(const GLubyte *name)
{
    resolver_fn fn = atomic_load(&resolver_real);
    if (!fn) {
        fn = (resolver_fn)original_dlsym()(RTLD_NEXT, "glXGetProcAddress");
        atomic_store(&resolver_real, fn);
    }
    return resolve(fn, name);
}

static __GLXextFuncPtr shimGetProcAddressARB(const GLubyte *name)
{
    resolver_fn fn = atomic_load(&resolver_arb_real);
    if (!fn) {
        fn = (resolver_fn)original_dlsym()(RTLD_NEXT, "glXGetProcAddressARB");
        atomic_store(&resolver_arb_real, fn);
    }
    return resolve(fn, name);
}

/* Keep resolver wrappers private: exporting the GLX resolver names causes
 * dlsym(RTLD_DEFAULT, ...) and driver-internal lookups to find this shim,
 * caching our own wrapper as the real resolver and recursing indefinitely.
 * Coin resolves GLX entry points using a private dlopen handle. Intercept
 * those resolver lookups as well as ordinary linked GLX calls. */
void *dlsym(void *handle, const char *name)
{
    void *result = original_dlsym()(handle, name);
    if (!result) return result;
    if (strcmp(name, "glXGetProcAddress") == 0) {
        attribute_fn attr = (attribute_fn)original_dlsym()(handle, "glXGetFBConfigAttrib");
        if (attr && attr != glXGetFBConfigAttrib) atomic_store(&attribute_real, attr);
        atomic_store(&resolver_real, (resolver_fn)result);
        return (void *)shimGetProcAddress;
    }
    if (strcmp(name, "glXGetProcAddressARB") == 0) {
        attribute_fn attr = (attribute_fn)original_dlsym()(handle, "glXGetFBConfigAttrib");
        if (attr && attr != glXGetFBConfigAttrib) atomic_store(&attribute_real, attr);
        atomic_store(&resolver_arb_real, (resolver_fn)result);
        return (void *)shimGetProcAddressARB;
    }
    return result;
}
