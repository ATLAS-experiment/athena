# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
from AthenaCommon.CFElements import isSequence
from AthenaCommon.Configurable import ConfigurableAlgTool
from GaudiKernel.GaudiHandles import PrivateToolHandle, PrivateToolHandleArray

import fnmatch

msg = logging.getLogger('PropSetterProxy')


class PropSetterProxy(object):
   __compPaths = {}
   __scannedCA = None

   def __init__(self, ca, path):      
      self.__path = path      
      self.__findComponents( ca )
      
   def __setattr__(self, name, value):
      if name.startswith("_PropSetterProxy"):
         return super(PropSetterProxy, self).__setattr__(name, value)

      if name != "OutputLevel":
         msg.error("foreach_component is a debugging feature and should not be used in production jobs, remove it before committing to the repository, proceeding to set the properties now" )

      matches = 0
      for component_path, component in PropSetterProxy.__compPaths.items():
         if fnmatch.fnmatch( component_path, self.__path ):
            matches += 1
            if name in component._descriptors:
               try:
                  setattr( component, name, value )
                  msg.info( "Set property %s to %s for component %s because it matched '%s'",
                            name, value, component_path, self.__path )
               except Exception as ex:
                  msg.warning( "Failed to set property %s to value %s for component %s: %s",
                               name, value, component_path, ex )
            else:
               msg.warning( "Property %s does not exist for component %s",
                            name, component_path )

      if not matches:
         msg.warning("No components found matching '%s'", self.__path)


   def __findComponents(self, ca):
       if ca is not PropSetterProxy.__scannedCA:
           PropSetterProxy.__scannedCA = ca
           PropSetterProxy.__compPaths = {}
           def __add(path, comp):
               if comp.getName() == "":
                   return
               PropSetterProxy.__compPaths[ path ] = comp


           for svc in ca._services:
               PropSetterProxy.__compPaths['SvcMgr/'+svc.getFullJobOptName()] = svc
           for t in ca._publicTools:
               PropSetterProxy.__compPaths['ToolSvc/'+t.getFullJobOptName()] = t
           for t in ca._conditionsAlgs:
               PropSetterProxy.__compPaths[t.getFullJobOptName()] = t
           if ca._privateTools:
               for t in ca._privateTools:
                   PropSetterProxy.__compPaths[t.getFullJobOptName()] = t

           def __nestAlg(startpath, comp): # it actually dives inside the algorithms and (sub) tools               
               if comp.getName() == "":
                   return
               for name, value in comp._descriptors.items():
                   if isinstance(value.cpp_type, (ConfigurableAlgTool, PrivateToolHandle)):
                       __add( startpath+"/"+name+"/"+value.getFullJobOptName(), value )
                       __nestAlg( startpath+"/"+name+"/"+value.getName(), value )
                   if isinstance( value.cpp_type, PrivateToolHandleArray):
                       for toolIndex,t in enumerate(value):
                           __add( startpath+"/"+name+"/"+t.getFullJobOptName(), t )
                           __nestAlg( startpath+"/"+name+"/"+t.getName(), value[toolIndex] )
                           
               
           def __nestSeq( startpath, seq ):
               for c in seq.Members:
                   if isSequence(c):
                       __nestSeq( startpath+"/"+c.getName(), c )                       
                   else: # the algorithm or tool
                       __add( startpath+"/"+c.getFullJobOptName(),  c )
                       __nestAlg( startpath+"/"+c.getFullJobOptName(), c )

           __nestSeq("", ca._sequence)
            
            



