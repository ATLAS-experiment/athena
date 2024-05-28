# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
##
# @file PyUtils/python/moduleExists.py
# @author sss
# @date Oct 2019
# @brief Helper to test for the existence of a module.
#
from functools import cache
import importlib.util


@cache
def moduleExists (modName):
    """Test for the existence of a module without actually importing it.

We could just do
  try:
    import modName
  except ImportError:
    ...
except that that has the potential to hide other errors."""

    return importlib.util.find_spec (modName) is not None
