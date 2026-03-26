# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Giordon Stark

# Shared test utilities for ColumnarToolWrapperPython tests.
# Each test file imports these via sys.path manipulation:
#
#   import os, sys
#   sys.path.insert(0, os.path.dirname(__file__))
#   from testutils import _run_tests, approx, xfail, raises, unique_name

import contextlib
import inspect
import math
import sys

_tool_index = 0


def unique_name():
    """Return a unique tool instance name for each call."""
    global _tool_index
    _tool_index += 1
    return f"unique{_tool_index}"


def approx(expected, rel=1e-6):
    """Float comparison helper matching a value within relative tolerance."""
    class _A:
        def __eq__(self, o): return math.isclose(o, expected, rel_tol=rel)
        def __ne__(self, o): return not self.__eq__(o)
    return _A()


def xfail(reason=""):
    """Decorator: mark a test as expected-failure (silently passes on exception)."""
    def decorator(fn):
        def wrapper(*a, **k):
            try: fn(*a, **k)
            except Exception: pass
        wrapper.__name__ = fn.__name__
        return wrapper
    return decorator


@contextlib.contextmanager
def raises(exc_type):
    """Context manager: assert that the body raises exc_type."""
    try: yield
    except exc_type: return
    except Exception as e: raise AssertionError(f"Expected {exc_type.__name__}, got {type(e).__name__}") from e
    else: raise AssertionError(f"Expected {exc_type.__name__} but nothing raised")


def _run_tests(module):
    """Run all test_* functions in module with gtest-style output."""
    tests = [(n, f) for n, f in inspect.getmembers(module, inspect.isfunction) if n.startswith("test_")]
    suite = module.__name__
    print(f"[==========] Running {len(tests)} tests from {suite}.")
    failed = []
    for name, fn in tests:
        print(f"[ RUN      ] {suite}.{name}")
        try:
            fn()
            print(f"[       OK ] {suite}.{name}")
        except Exception as e:
            print(f"[  FAILED  ] {suite}.{name}: {e}")
            failed.append(name)
    print(f"[==========] {len(tests)} tests ran.")
    if failed:
        for name in failed:
            print(f"[  FAILED  ] {suite}.{name}")
        sys.exit(1)
    print(f"[  PASSED  ] {len(tests)} tests.")
