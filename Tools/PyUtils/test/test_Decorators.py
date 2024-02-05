#! /usr/bin/env python

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# Simple test wrapper for PyUtils.Decorators
# Specifically for the deprecate decorator

from __future__ import annotations
import copy
import os

import unittest
from PyUtils.Decorators import deprecate, forking, log
import logging
import inspect
from contextlib import contextmanager

_warn_once_reason = "Test String for function warning once"


@deprecate(_warn_once_reason)
def warn_once() -> int:
    return 5


_warn_every_time_reason = "Test String for function warning every time"


@deprecate(_warn_every_time_reason, warn_once_per_call=False)
def warn_every_time() -> int:
    return 3


_warn_with_arg_reason = "Test String for function taking an argument"


@deprecate(_warn_with_arg_reason)
def warn_with_arg(x: str) -> str:
    return x + "_suff"


_warn_with_context_reason = (
    "Test String for function that warns with its calling context"
)


@deprecate(_warn_with_context_reason, print_context=True)
def warn_with_context() -> int:
    return 4


def _expected_depr_output(
    fname: str, reason: str, context: str | None = None, line_delta: int = -1
) -> list[str]:
    """Get the expected output of the deprecate warning

    This should be called from the same function as the deprecated function.

    Parameters
    ----------
    fname : str
        The name of the called deprecated function
    reason : str
        The reason given to the deprecation function
    context : str | None
        The calling context if it is expected in the output
    line_delta: int
        The difference in line numbers between this call and where the deprecated
        function was called
    """
    frame_info = inspect.stack()[1]
    lineno = frame_info.lineno + line_delta
    return (
        [
            f"Calling deprecated function '{fname}'",
            f"in function {frame_info.function}, file {frame_info.filename}, line {lineno}",
        ]
        + ([f"{lineno}\t{context}"] if context is not None else [])
        + [
            reason,
        ]
    )

class _CaptureHandler(logging.Handler):
    """Helper handler to capture logging messages"""

    def __init__(self, level: int):
        super().__init__(level=level)
        self.messages :list[str] = []
        # Set a basic formatter that just outputs the messages
        self.setFormatter(logging.Formatter())

    def emit(self, record: logging.LogRecord) -> None:
        self.messages.append(self.format(record))

@contextmanager
def _logger_capture(logger: logging.Logger, level: int = logging.WARNING):
    old_handlers = copy.copy(log.handlers)
    log.handlers = []
    handler = _CaptureHandler(level=level)
    log.addHandler(handler)
    try:
        yield handler.messages
    finally:
        log.removeHandler(handler)
        log.handlers = old_handlers

class DeprecateTestCase(unittest.TestCase):
    def test_warn_once(self) -> None:
        with _logger_capture(log) as testlog:
            self.assertEqual(warn_once(), 5)
        expected = _expected_depr_output("warn_once", _warn_once_reason)
        self.assertEqual(testlog, expected)
        # Now make sure it warns again for a new location, but make sure that it only
        # does it once in a loop
        for expect_warn in (True, False):
            with _logger_capture(logger=log) as testlog:
                self.assertEqual(warn_once(), 5)
            expected = _expected_depr_output("warn_once", _warn_once_reason)
            self.assertEqual(testlog, expected if expect_warn else [])

    def test_warn_every_time(self) -> None:
        for _ in range(3):
            with _logger_capture(logger=log) as testlog:
                self.assertEqual(warn_every_time(), 3)
            expected = _expected_depr_output("warn_every_time", _warn_every_time_reason)
            self.assertEqual(testlog, expected)

    def test_warn_with_arg(self) -> None:
        for arg in ("First", "Second", "Third"):
            with _logger_capture(logger=log) as testlog:
                self.assertEqual(warn_with_arg(arg), f"{arg}_suff")
            expected = _expected_depr_output("warn_with_arg", _warn_with_arg_reason)
            self.assertEqual(testlog, expected if arg == "First" else [])

    def test_warn_with_context(self) -> None:
        with _logger_capture(logger=log) as testlog:
            self.assertEqual(warn_with_context(), 4)
        expected = _expected_depr_output(
            "warn_with_context",
            _warn_with_context_reason,
            context="            self.assertEqual(warn_with_context(), 4)\n",
        )
        self.assertEqual(testlog, expected)


class Foo:
    pass

class ForkingTestCase(unittest.TestCase):
    def test_pid(self) -> None:
        @forking
        def f() -> int:
            return os.getpid()
        self.assertNotEqual(os.getpid(), f())

    def test_class(self) -> None:
        @forking
        def f() -> Foo:
            return Foo()
        self.assertIsInstance(f(), Foo)


if __name__ == "__main__":
    unittest.main()
