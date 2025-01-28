# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from GaudiKernel.GaudiHandles import GaudiHandle
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon import Constants
from AthenaCommon.Logging import logging
import collections


def loadDefaultComps(allcomps):
    """Attempts to load all default components (those that are not actually configured)"""
    loadedSome = False
    def __load(comp, prop):
        descr = comp._descriptors[prop]
        if descr.default.getType() == "":
            return
        try:
            childComp = CompFactory.getComp(descr.default.getType())(descr.default.getName())
            comp._properties[prop] = childComp
            nonlocal loadedSome
            loadedSome = True
        except Exception:
            msg=logging.getLogger('loadDefaultComps')
            msg.warning("Default component %s can not be loaded", descr.default.typeAndName )
            pass

    for comp in allcomps:
        for propName,value in comp._descriptors.items():
            if propName in comp._properties: continue
            if isinstance(value.default, GaudiHandle):
                __load(comp, propName )
                        
    if loadedSome:
        loadDefaultComps(allcomps)

def exposeHandles(allcomps):
    """Sets all handle keys explicitly"""
    def __getDefault(d):
        if isinstance(d, collections.abc.Sequence):
            return [el  for el in d]
        else:
            return d.Path

    for comp in allcomps:
        for propName, value in comp._descriptors.items():
            if propName in comp._properties: continue
            if "HandleKey" in value.cpp_type:
                comp._properties[propName] = __getDefault(value.default)

def setupLoggingLevels(flags, ca):
    """Read the Exec.*MessageComponents flags and modify OutputLevel of component(s).
    
    The specification of components uses the Python `fnmatch` library and resembles UNIX paths.
    An event algorithm MyAlgo/MyInstance has the following path:
        MasterSeq/AthAllAlgSeq/AthAlgSeq/MyAlgo/MyInstance
    A private tool MyTool of name ToolInstance used by that algorithm:
        MasterSeq/AthAllAlgSeq/AthAlgSeq/MyAlgo/MyInstance/MyTool/ToolInstance
    A public tool:
        ToolSvc/MyTool/ToolInstance

    The path specification can take the following forms:
        '*/ToolInstance'      : all tools that have matching instance name
        '*/MyTool/*'          : all instances of type MyTool
        '*/MyAlgo/MyInstance' : specific algorithm instance
        '*/MyAlgo/*'          : all instances of the specific algorithm class
        '*/AthAlgSeq/*'       : all algorithms of the given sequence
        'ToolSvc/My*/*'       : all public tools with instance name starting with "My"

    The modifications to the OutputLevel are applied in the order ERROR to VERBOSE, i.e.
    it is possible to set higher verbosities with more specific selections.

    Each setting can be either a string or a list of strings. If the component path contains
    no '/' it is assumed to be a plain component name. In this case, the OutputLevel is set
    using the property MessageSvc.setDebug or equivalent. This works also for converters, which
    do not have any properties.
    """

    def __tolist(d):
        return ([d] if d else []) if isinstance(d, str) else d

    for flag_val, level in ((flags.Exec.ErrorMessageComponents, 'ERROR'),
                            (flags.Exec.WarningMessageComponents, 'WARNING'),
                            (flags.Exec.InfoMessageComponents, 'INFO'),
                            (flags.Exec.DebugMessageComponents, 'DEBUG'),
                            (flags.Exec.VerboseMessageComponents, 'VERBOSE')):
        for c in __tolist(flag_val):
            if "/" in c:
                ca.foreach_component(c).OutputLevel = getattr(Constants, level)

            # a plain component name is given
            else:
                msgSvc = ca.getService("MessageSvc")  # should exist by the time this code runs
                getattr(msgSvc, f'set{level.title()}').append(c)  # e.g. setDebug property
