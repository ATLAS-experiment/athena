# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

"""Module with dummy Configurable classes for testing"""

from GaudiConfig2._configurables import makeConfigurableClass
from GaudiKernel.GaudiHandles import PrivateToolHandleArray,PrivateToolHandle


dummyService = makeConfigurableClass(
  "dummyService",
    __module__ = __name__,
  __cpp_type__ = "dummyService",
  __component_type__ = "Service",
  properties = {
    "AString": ("std::string", ""),
    "AList": ("std::vector<std::string>", [], "my list", "OrderedSet<std::string>"),
    "OneTool": ("PrivateToolHandle", PrivateToolHandle()),
    "SomeTools": ("PrivateToolHandleArray", PrivateToolHandleArray([])),
  }
)

dummyTool = makeConfigurableClass(
  "dummyTool",
  __module__ = __name__,
  __cpp_type__ = "dummyTool",
  __component_type__ = "AlgTool",
  properties = {
    "BString": ("std::string", ""),
    "BList": ("std::vector<std::string>", [], "my list", "OrderedSet<std::string>"),
  }
)
