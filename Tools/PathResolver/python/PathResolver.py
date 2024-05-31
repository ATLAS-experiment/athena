# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""
Python bindings to the C++ PathResolver. Usage:
>>>  from PathResolver import PathResolver
>>>  PathResolver.FindCalibFile("blah")

In general, it is preferred to use the Python-implementation from AthenaCommon.Utils.unixtools.
"""

import ROOT
FindCalibFile = ROOT.PathResolver.FindCalibFile
FindCalibDirectory = ROOT.PathResolver.FindCalibDirectory
SetOutputLevel = ROOT.PathResolver.SetOutputLevel
