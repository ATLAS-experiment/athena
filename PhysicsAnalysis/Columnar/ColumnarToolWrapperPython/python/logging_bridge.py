# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Giordon Stark

"""Bridge C++ message output to Python logging module.

Installs a callback that routes C++ IMessagePrinter output to a Python
logger named "atlas", integrating with standard logging configuration
(handlers, formatters, levels).

The bridge is installed automatically on package import with a default
StreamHandler and INFO level, so C++ messages are visible out of the box.
To suppress them::

    import logging
    logging.getLogger("atlas").setLevel(logging.WARNING)

To see DEBUG/VERBOSE messages::

    import logging
    logging.getLogger("atlas").setLevel(logging.DEBUG)

To customize the format, replace the default handler::

    import logging, sys
    logger = logging.getLogger("atlas")
    logger.handlers.clear()
    handler = logging.StreamHandler(sys.stderr)
    handler.setFormatter(logging.Formatter("[%(tool)s] %(levelname)s %(message)s"))
    logger.addHandler(handler)

Tool context is passed via the `extra` dict, so formatters can reference
`%(tool)s` to display the tool name.

In Athena/AthAnalysis builds (non-standalone), IMessagePrinter is not available
and the C++ bridge cannot be installed. The logger is still created and usable
for Python-side logging, but C++ messages will not be routed through it.
"""

import logging
import sys
import atexit

from ColumnarToolWrapperPython.python_tool_handle import set_python_printer, MsgLevel


# Mapping from C++ MSG::Level to Python logging levels.
# C++ levels (0-6) map to Python levels preserving severity ordering.
_LEVEL_MAP = {
    MsgLevel.NIL: logging.NOTSET,
    MsgLevel.VERBOSE: logging.DEBUG - 5,
    MsgLevel.DEBUG: logging.DEBUG,
    MsgLevel.INFO: logging.INFO,
    MsgLevel.WARNING: logging.WARNING,
    MsgLevel.ERROR: logging.ERROR,
    MsgLevel.FATAL: logging.CRITICAL,
}


# Format approximating the C++ default MessagePrinter output:
# "ToolSvc.myTool           INFO    message text"
_DEFAULT_FORMAT = "%(tool)-25s%(levelname)-8s%(message)s"


def install_printer(name="atlas"):
    """Install a Python logger as the C++ message printer.

    Routes all C++ IMessagePrinter messages (from tools, ASG framework, etc.)
    to a Python logger with the given name. Called automatically by the package
    on import with a default StreamHandler and INFO level, matching the C++
    default stdout printer behaviour.

    In Athena/AthAnalysis (non-standalone) builds, IMessagePrinter is not
    available. The logger is still created and configured, but C++ messages
    will not be routed through it.

    Tool name is passed via the `extra` dict, so formatters can use `%(tool)s`.

    Parameters
    ----------
    name:
        Logger name (default: "atlas").

    Returns
    -------
    logging.Logger
        The installed logger. Configure with handlers and setLevel as needed.

    Notes
    -----
    Only one printer can be active at a time. Calling install_printer again
    replaces the previous one. Call remove_printer() to restore default
    stdout printing.

    Cleanup is automatically registered via atexit, so you don't need to
    call remove_printer() explicitly at interpreter shutdown.
    """
    logger = logging.getLogger(name)

    # Add a default handler if none exists yet, so messages are visible
    # out of the box without any user configuration.
    if not logger.handlers:
        handler = logging.StreamHandler(sys.stderr)
        handler.setFormatter(logging.Formatter(_DEFAULT_FORMAT))
        logger.addHandler(handler)

    # Set level to INFO to match the C++ default MsgStream threshold, but
    # only if the caller has not already configured a level (NOTSET means
    # no explicit level was set yet).
    if logger.level == logging.NOTSET:
        logger.setLevel(logging.INFO)

    # Don't propagate to the root logger — avoids double printing when
    # the user has logging.basicConfig() or other root handlers configured.
    logger.propagate = False

    def printer_callback(level_int, tool_name, text):
        """Callback invoked by C++ for each message."""
        # Look up Python logging level for the C++ message level (int key)
        py_level = _LEVEL_MAP.get(level_int, logging.INFO)
        # Pass tool name via extra dict for formatter access
        logger.log(py_level, text.rstrip(), extra={"tool": tool_name})

    try:
        set_python_printer(printer_callback)
        # Ensure cleanup at interpreter shutdown
        atexit.register(remove_printer)
    except RuntimeError:
        # Non-standalone (Athena/AthAnalysis) build: IMessagePrinter is not
        # available. The logger is still usable for Python-side logging.
        logger.debug(
            "C++ message bridge unavailable (not a standalone build); "
            "C++ tool messages will not be routed through Python logging."
        )

    return logger


def remove_printer():
    """Remove the Python printer and restore default stdout printing."""
    set_python_printer()  # Call with no arguments to reset
