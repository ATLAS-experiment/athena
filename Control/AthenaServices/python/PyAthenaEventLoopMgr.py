# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

## @file PyAthenaEventLoopMgr.py
#  @brief Python facade of PyAthenaEventLoopMgr
#  @author Wim Lavrijsen (WLavrijsen@lbl.gov)

import GaudiPython.Bindings as PyGaudi


## @class _PyAthenaEventLoopMgrClass
#  @brief Python facade of PyAthenaEventLoopMgr.
class PyAthenaEventLoopMgr( PyGaudi.iService ):

   def __init__( self ):
      PyGaudi.iService.__init__( self, 'PyAthenaEventLoopMgr' )

   def _installServices( self, cppself ):
    # install the interfaces onto oneself; the order is (as per the
    # cpp side multiple inheritence:
    #   IEventSeek
    #   IService              => from here ...
    #     INamedInterface
    #     IInterface
    #   IProperty             => ... up to here from the iService base class
    #   IStateful
    #   IEventProcessor
    #
    # The expectation from the C++ side it that an IEventSeek is received. Note
    # that this code can not call into the application manager, as this is run
    # during initialize, making the IService PyAthenaEventLoopMgr unavailable.
    #
    # Note that this is all a poor man's way of not needing to have dictionaries
    # for all base classes of the C++ PyAthenaEventLoopMgr.

      import cppyy

    # need to set all the following through the __dict__ b/c of iPropert.__setattr__

    # the expect IEventSeek
      self.__dict__[ '_evtSeek' ] = cppyy.bind_object( cppself, cppyy.gbl.IEventSeek )

    # the IService needed for iService._isvc and likewise iProperty._ip
      self.__dict__[ '_isvc' ] = PyGaudi.InterfaceCast( cppyy.gbl.IService )( self._evtSeek )
      self.__dict__[ '_ip' ] = PyGaudi.InterfaceCast( cppyy.gbl.IProperty )( self._evtSeek )

    # IStateful and IEventProcessor
      self.__dict__[ '_state' ] = PyGaudi.InterfaceCast( cppyy.gbl.IStateful )( self._evtSeek )
      self.__dict__[ '_evtpro' ] = PyGaudi.InterfaceCast( cppyy.gbl.IEventProcessor )( self._evtSeek )

   def __getattr__( self, attr ):
    # note the lookup order: should be as per the C++ side
      for obj in [ self._evtSeek, self._state, self._evtpro ]:
         try:
            return getattr( obj, attr )
         except Exception:
            pass

    # let properties be tried last
      return super( PyAthenaEventLoopMgr, self ).__getattr__( attr )

   def executeAlgorithms( self, cppcontext ):

      from AthenaPython.PyAthena import py_svc
      from AthenaPython.PyAthenaComps import StatusCode
      appmgr = py_svc('ApplicationMgr',iface="IProperty")      
      algmgr = py_svc('ApplicationMgr',iface='IAlgManager')
     
      import cppyy
      ctx = cppyy.bind_object(cppcontext, "EventContext")

      try:
         for name in appmgr.getProperty("TopAlg").value():
            ialg=algmgr.algorithm(name).get()
            ialg.execState(ctx).reset()
            result = ialg.sysExecute(ctx)
            if result.isFailure():
               from AthenaCommon.Logging import log as msg
               msg.error( "Execution of algorithm %s failed", name )
               return result.getCode()
      except KeyboardInterrupt:
         from AthenaCommon.Logging import log as msg
         msg.critical( "event loop stopped by user interrupt" )
         return StatusCode.Failure

      return StatusCode.Success
