# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

"""
Some useful decorators.
"""

from typing import Any, Callable
import functools
import inspect
import logging
import sys

log = logging.getLogger(__name__)

TFunc = Callable[..., Any]

def _reraise_exception(new_exc, exc_info=None):
    if exc_info is None:
        exc_info = sys.exc_info()
    _exc_class, _exc, tb = exc_info
    raise new_exc.__class__ (new_exc, tb)
    

def forking(func):
    """
    This decorator implements the forking patterns, i.e. it runs the function
    in a forked process.
    see:
     http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/511474
    """
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        import os
        import pickle

        # create a pipe which will be shared between parent and child
        pread, pwrite = os.pipe()

        # do fork
        pid = os.fork()

        ## parent ##
        if pid > 0:
            os.close(pwrite)
            with os.fdopen(pread, 'rb') as f:
                status, result = pickle.load(f)
            os.waitpid(pid, 0)
            if status == 0:
                return result
            else:
                remote_exc = result[0]
                _reraise_exception(remote_exc)

        ## child ##
        else:
            os.close(pread)
            try:
                result = func(*args, **kwargs)
                status = 0
            except (Exception, KeyboardInterrupt) as exc:
                import traceback
                exc_string = traceback.format_exc(limit=10)
                for l in exc_string.splitlines():
                    print ("[%d]"%os.getpid(),l.rstrip())
                sys.stdout.flush()
                result = exc, exc_string
                status = 1
            with os.fdopen(pwrite, 'wb') as f:
                try:
                    pickle.dump((status,result), f, pickle.HIGHEST_PROTOCOL)
                except pickle.PicklingError as exc:
                    pickle.dump((2,exc), f, pickle.HIGHEST_PROTOCOL)
            os._exit(0)
        return result

    return wrapper


def deprecate(
    reason: str, *, warn_once_per_call: bool = True, print_context: bool = False
) -> Callable[[TFunc], TFunc]:
    """Decorator to indicate that a function should not be used

    Parameters
    ----------
    reason : str
        A string explaining why the function should not be used
    warn_once_per_call : bool
        If True, only warn once per call location, default True
    print_context: bool
        If True, print the calling context as part of the warning, default False
    """

    # Record when we've warned about a function+filename+line combination
    warncache: set[tuple[int, str, int]] = set()

    def deprecate_decorator(f):
        @functools.wraps(f)
        def call_deprecated(*args, **kwargs):
            """Call a deprecated function"""
            # Figure out where we're calling from. The stack indices are as follows
            # 0: this inspect.stack call
            # 1: the actual call site we want to mark
            frame_info = inspect.stack()[1]
            cache_value = id(f), frame_info.filename, frame_info.lineno
            if not warn_once_per_call or cache_value not in warncache:
                if warn_once_per_call:
                    warncache.add(cache_value)
                log.warning(f"Calling deprecated function '{f.__qualname__}'")
                log.warning(
                    f"in function {frame_info.function}, file {frame_info.filename}, line {frame_info.lineno}"
                )
                if print_context:
                    for idx, line in enumerate(frame_info.code_context):
                        log.warning(f"{frame_info.lineno + idx - frame_info.index}\t{line}")
                log.warning(reason)
            return f(*args, **kwargs)
        return call_deprecated
    return deprecate_decorator
