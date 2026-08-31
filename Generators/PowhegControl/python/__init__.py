# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# flake8: noqa

__all__ = ['PowhegControl']

import importlib


def __getattr__(name):
    if name == 'PowhegControl':
        module = importlib.import_module('.powheg_control', __package__)
        return getattr(module, name)
    raise AttributeError(f"module {__name__!r} has no attribute {name!r}")
