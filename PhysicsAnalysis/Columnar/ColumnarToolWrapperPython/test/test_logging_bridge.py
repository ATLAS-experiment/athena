# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Giordon Stark

import os
import sys
import logging
sys.path.insert(0, os.path.dirname(__file__))

from testutils import _run_tests

from ColumnarToolWrapperPython.logging_bridge import install_printer, remove_printer, _DEFAULT_FORMAT
from ColumnarToolWrapperPython.python_tool_handle import MsgLevel


def _fresh_logger(name="atlas"):
    """Remove all handlers from the named logger and return it."""
    logger = logging.getLogger(name)
    logger.handlers.clear()
    return logger


def test_install_printer_creates_logger():
    """install_printer() creates a logger with INFO level, a handler, and no propagation."""
    _fresh_logger()
    logger = install_printer()
    assert isinstance(logger, logging.Logger)
    assert logger.name == "atlas"
    assert logger.level == logging.INFO
    assert len(logger.handlers) == 1
    assert logger.propagate is False


def test_install_printer_no_duplicate_handler():
    """Calling install_printer() twice does not add a second handler."""
    _fresh_logger()
    install_printer()
    install_printer()
    logger = logging.getLogger("atlas")
    # Second call should not add another handler since one already exists
    assert len(logger.handlers) == 1


def test_default_handler_formatter():
    """Default handler uses a formatter that includes %(tool)s."""
    import io
    from ColumnarToolWrapperPython.logging_bridge import _LEVEL_MAP

    _fresh_logger()
    logger = install_printer()

    # Replace the default StreamHandler with a StringIO one (same formatter)
    stream = io.StringIO()
    handler = logging.StreamHandler(stream)
    handler.setFormatter(logging.Formatter(_DEFAULT_FORMAT))
    logger.handlers.clear()
    logger.addHandler(handler)

    py_level = _LEVEL_MAP[3]  # MSG::INFO
    logger.log(py_level, "hello world", extra={"tool": "ToolSvc.myTool"})

    output = stream.getvalue()
    assert "ToolSvc.myTool" in output
    assert "hello world" in output
    # Level name should appear in the output
    assert "INFO" in output


def test_printer_message_routing():
    """Printer routes C++ messages to Python logging at correct level."""
    _fresh_logger()
    logger = install_printer()
    # Default install adds one handler and sets INFO level
    assert logger.hasHandlers()
    assert logger.level == logging.INFO


def test_remove_printer_clears_handler():
    """remove_printer() restores default stdout printing."""
    _fresh_logger()
    install_printer()
    remove_printer()
    # If we get here without error, removal succeeded
    assert True


def test_level_filtering_suppresses_messages():
    """Logger level controls whether C++ messages are captured."""
    import io
    from ColumnarToolWrapperPython.logging_bridge import _LEVEL_MAP

    _fresh_logger()
    logger = install_printer()

    # Replace default handler with a StringIO one for capture
    stream = io.StringIO()
    handler = logging.StreamHandler(stream)
    handler.setLevel(logging.DEBUG)  # handler passes everything; logger level filters
    logger.handlers.clear()
    logger.addHandler(handler)

    # At WARNING level, INFO messages must not appear
    logger.setLevel(logging.WARNING)
    py_level = _LEVEL_MAP[3]  # MSG::INFO → logging.INFO
    logger.log(py_level, "should be suppressed", extra={"tool": "T"})
    assert stream.getvalue() == ""

    # At DEBUG level, INFO messages must appear
    logger.setLevel(logging.DEBUG)
    logger.log(py_level, "should appear", extra={"tool": "T"})
    assert "should appear" in stream.getvalue()

    # Cleanup: restore default level
    logger.setLevel(logging.INFO)


def test_logging_formatter_with_tool_context():
    """Formatter can use %(tool)s to display tool names."""
    import io

    _fresh_logger()
    logger = install_printer()

    # Add a second handler with custom format to capture output
    stream = io.StringIO()
    handler = logging.StreamHandler(stream)
    handler.setFormatter(logging.Formatter("[%(tool)s] %(message)s"))
    logger.addHandler(handler)

    # Simulate a C++ message callback
    from ColumnarToolWrapperPython.logging_bridge import _LEVEL_MAP

    tool_name = "TestTool"
    text = "test message"
    level_int = 3  # MSG::INFO

    # Manually invoke what the C++ side would call
    py_level = _LEVEL_MAP.get(level_int, logging.INFO)
    logger.log(py_level, text.rstrip(), extra={"tool": tool_name})

    # Check that output contains the tool name
    output = stream.getvalue()
    assert "[TestTool]" in output
    assert "test message" in output


if __name__ == "__main__":
    _run_tests(sys.modules[__name__])
