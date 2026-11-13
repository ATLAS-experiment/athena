# Copyright (C) 2026 CERN for the benefit of the ATLAS collaboration
"""Exercise real ELF interposition without requiring an X server."""
import os
from pathlib import Path
import platform
import shutil
import subprocess
import tempfile
import unittest


class GLXShimTest(unittest.TestCase):
    @unittest.skipUnless(platform.system() == "Linux" and
                         platform.machine() in ("aarch64", "x86_64"),
                         "Linux glibc shim")
    def test_resolver_lookup_does_not_recurse(self):
        compiler = shutil.which(os.environ.get("CC", "cc"))
        if not compiler:
            self.skipTest("C compiler unavailable")
        source = Path(__file__).resolve().parents[1] / "share/offscreen-workaround/vp1-glx-pixel-limit.c"
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            mock = directory / "mock.c"
            mock.write_text(r'''#include <GL/glx.h>
#include <string.h>
#include <assert.h>
static int mode, calls;
static Bool requests[2];
void configure(int value) { mode = value; calls = 0; }
int count(void) { return calls; }
Bool requested(int index) { return requests[index]; }
GLXContext glXCreateContext(Display *d, XVisualInfo *v, GLXContext share, Bool direct) {
    assert(d == (Display *)1 && v == (XVisualInfo *)2 && share == (GLXContext)3);
    assert(calls < 2);
    requests[calls++] = direct;
    return mode == 0 || (mode == 1 && direct) ? (GLXContext)4 : 0;
}
int glXGetFBConfigAttrib(Display *d, GLXFBConfig c, int a, int *v) {
    (void)d; (void)c;
    *v = a == GLX_MAX_PBUFFER_WIDTH ? 2048 : 0;
    return a < 0 ? GLX_BAD_ATTRIBUTE : Success;
}
__GLXextFuncPtr glXGetProcAddress(const GLubyte *name) {
    if (!strcmp((const char *)name, "glXCreateContext"))
        return (__GLXextFuncPtr)glXCreateContext;
    return strcmp((const char *)name, "glXGetFBConfigAttrib") ? 0 :
        (__GLXextFuncPtr)glXGetFBConfigAttrib;
}
__GLXextFuncPtr glXGetProcAddressARB(const GLubyte *name) {
    return glXGetProcAddress(name);
}
''')
            harness = directory / "test.c"
            harness.write_text(r'''#define _GNU_SOURCE
#include <GL/glx.h>
#include <assert.h>
#include <dlfcn.h>
int main(int argc, char **argv) {
    (void)argc;
    void *handle = dlopen(argv[1], RTLD_NOW | RTLD_GLOBAL);
    assert(handle);
    void *handles[] = {handle, RTLD_DEFAULT, handle, RTLD_DEFAULT};
    const char *names[] = {"glXGetProcAddress", "glXGetProcAddressARB"};
    for (int i = 0; i < 4; ++i) for (int j = 0; j < 2; ++j) {
        __GLXextFuncPtr (*resolve)(const GLubyte *) = dlsym(handles[i], names[j]);
        assert(resolve);
        int (*query)(Display *, GLXFBConfig, int, int *) =
            (void *)resolve((const GLubyte *)"glXGetFBConfigAttrib");
        assert(query);
        int value;
        assert(query(0, 0, GLX_MAX_PBUFFER_PIXELS, &value) == Success);
        assert(value == 16777216);
        assert(query(0, 0, GLX_MAX_PBUFFER_WIDTH, &value) == Success);
        assert(value == 2048);
        assert(query(0, 0, -1, &value) == GLX_BAD_ATTRIBUTE && value == 0);
        assert(!resolve((const GLubyte *)"missing"));
    }
    void (*configure)(int) = dlsym(handle, "configure");
    int (*count)(void) = dlsym(handle, "count");
    Bool (*requested)(int) = dlsym(handle, "requested");
    assert(configure && count && requested);
    /* Cover ordinary ELF interposition, explicit dlsym and both GLX resolvers. */
    for (int route = 0; route < 5; ++route) {
        GLXContext (*create)(Display *, XVisualInfo *, GLXContext, Bool);
        if (route == 0) create = glXCreateContext;
        else if (route < 3) create = dlsym(route == 1 ? handle : RTLD_DEFAULT, "glXCreateContext");
        else {
            __GLXextFuncPtr (*resolve)(const GLubyte *) = dlsym(handle, names[route - 3]);
            create = (void *)resolve((const GLubyte *)"glXCreateContext");
        }
        assert(create);
        for (int mode = 0; mode < 3; ++mode) for (int direct = 0; direct < 2; ++direct) {
            configure(mode);
            GLXContext context = create((Display *)1, (XVisualInfo *)2, (GLXContext)3, direct);
            assert(context == (mode < 2 ? (GLXContext)4 : 0));
            int retry = !direct && mode != 0;
            assert(count() == 1 + retry && requested(0) == direct);
            if (retry) assert(requested(1) == True);
        }
    }
    return 0;
}
''')
            shim = directory / "shim.so"
            library = directory / "mock.so"
            executable = directory / "test"
            for command in (
                [compiler, "-std=c11", "-shared", "-fPIC", "-O2", "-Wall", "-Wextra",
                 "-Werror", "-o", str(shim), str(source), "-ldl"],
                [compiler, "-shared", "-fPIC", "-o", str(library), str(mock)],
                [compiler, "-o", str(executable), str(harness), str(library), "-ldl"],
            ):
                subprocess.run(command, check=True, timeout=60)
            environment = dict(os.environ, LD_PRELOAD=str(shim))
            subprocess.run([str(executable), str(library)], env=environment,
                           check=True, timeout=10)


if __name__ == "__main__":
    unittest.main()
