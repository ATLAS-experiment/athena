# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

"""Simple fallback logging for PowhegControl when Athena is unavailable."""

try:
    from AthenaCommon import Logging as _AthenaLogging
    logging = _AthenaLogging.logging
    getLogger = logging.getLogger
except ImportError:
    import logging as _logging

    if not _logging.root.handlers:
        _handler = _logging.StreamHandler()
        _handler.setFormatter(_logging.Formatter('[%(levelname)s] %(name)s: %(message)s'))
        _logging.root.addHandler(_handler)
        _logging.root.setLevel(_logging.INFO)

    logging = _logging
    getLogger = _logging.getLogger

__all__ = ['logging', 'getLogger']
