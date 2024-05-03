# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

##
## @file AthenaCommon/python/AthenaCommonFlags.py
## @brief Python module to hold common flags to configure JobOptions
##

""" AthenaCommonFlags
    Python module to hold common flags to configure JobOptions.

    From the python prompt:
    >>> from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
    >>> print athenaCommonFlags.EvtMax()
    >>> athenaCommonFlags.EvtMax = 50
    >>> assert( athenaCommonFlags.EvtMax() == 50 )
    >>> athenaCommonFlags.print_JobProperties('tree&value')

"""

__author__ = "S.Binet, M.Gallas"
__version__= "$Revision: 1.11 $"
__doc__    = "AthenaCommonFlags"

__all__    = [ "athenaCommonFlags" ]

##-----------------------------------------------------------------------------
## Import

from AthenaCommon.JobProperties import JobProperty, JobPropertyContainer
from AthenaCommon.JobProperties import jobproperties

##-----------------------------------------------------------------------------
## 1st step: define JobProperty classes

class EvtMax(JobProperty):
    """Number of events to process or generate"""
    statusOn     = False
    allowedTypes = ['int']
    StoredValue  = 5

class SkipEvents(JobProperty):
    """Number of events to skip when reading an input POOL file. This should
    be given to the EventSelector service.
    """
    statusOn     = False
    allowedTypes = ['int']
    StoredValue  = 0

class FilesInput(JobProperty):
    """The list of input data files (if not empty override all the specific XYZInput) """
    statusOn     = True
    allowedTypes = ['list']
    StoredValue  = []

    def _do_action( self, *args, **kwds ):
        #first remove any blanks
        if "" in self.StoredValue: 
           self.StoredValue = list(filter(None,self.StoredValue))
        from AthenaCommon import AppMgr
        if hasattr(AppMgr.ServiceMgr,"EventSelector") and hasattr(AppMgr.ServiceMgr.EventSelector,"InputCollections"):
            AppMgr.ServiceMgr.EventSelector.InputCollections = self.StoredValue

        pass

class TreeName(JobProperty):
    """The name of the ttree in the input file"""
    statusOn     = True
    allowedType  = "str"
    StoredValue  = "CollectionTree"

    def _do_action( self, *args, **kwds ):
        from AthenaCommon import AppMgr
        if not hasattr(AppMgr.ServiceMgr,"EventSelector"): 
            #assume the TreeAccess mode then ..
            jobproperties.AthenaCommonFlags.AccessMode="TreeAccess"

        if not hasattr(AppMgr.ServiceMgr.EventSelector,"TupleName"):
            if self.StoredValue != "CollectionTree":
                raise ValueError("TreeName can only be CollectionTree if you are not using AccessMode=TreeAccess")
        else:
            AppMgr.ServiceMgr.EventSelector.TupleName = self.StoredValue
    

class AccessMode(JobProperty):
    """The type of read mechanism to use in this athena job"""
    statusOn     = True
    allowedType  = "str"
    allowedValues = ["TreeAccess","TreeAccessWithEventInfo","BranchAccess","ClassAccess","AthenaAccess","POOLAccess"]
    StoredValue  = "ClassAccess"

    def _do_action( self, *args, **kwds ):
        from AthenaCommon import AppMgr

        if self.StoredValue in ["ClassAccess","AthenaAccess","BranchAccess"]:
            if hasattr(AppMgr.ServiceMgr,"EventSelector"):
                if AppMgr.ServiceMgr.EventSelector.getType()!="Athena::xAODEventSelector": 
                    raise ValueError("Cannot switch to %s mode with existing EventSelector of type %s" % (self.StoredValue,AppMgr.ServiceMgr.EventSelector.getType()) )
            else:
                import AthenaRootComps.ReadAthenaxAODHybrid # noqa: F401
            if self.StoredValue=="ClassAccess": AppMgr.ServiceMgr.EventSelector.AccessMode = 1
            elif self.StoredValue=="BranchAccess": AppMgr.ServiceMgr.EventSelector.AccessMode = 0
            elif self.StoredValue=="AthenaAccess": AppMgr.ServiceMgr.EventSelector.AccessMode = 2
        elif self.StoredValue=="POOLAccess":
            if hasattr(AppMgr.ServiceMgr,"EventSelector"):
                if AppMgr.ServiceMgr.EventSelector.getType()!="EventSelectorAthenaPool": 
                    raise ValueError("Cannot switch to %s mode with existing EventSelector of type %s" % (self.StoredValue,AppMgr.ServiceMgr.EventSelector.getType()) )
            else:
                import AthenaPoolCnvSvc.ReadAthenaPool # noqa: F401
        elif self.StoredValue=="TreeAccess" or self.StoredValue=="TreeAccessWithEventInfo":
            if hasattr(AppMgr.ServiceMgr,"EventSelector"):
                if AppMgr.ServiceMgr.EventSelector.getType()!="Athena::RootNtupleEventSelector": 
                    raise ValueError("Cannot switch to %s mode with existing EventSelector of type %s" % (self.StoredValue,AppMgr.ServiceMgr.EventSelector.getType()) )
            else:
                import AthenaRootComps.ReadAthenaRoot # noqa: F401
            AppMgr.ServiceMgr.EventSelector.TupleName = jobproperties.AthenaCommonFlags.TreeName()
            if self.StoredValue=="TreeAccessWithEventInfo":
                AppMgr.ServiceMgr.EventSelector.CreateEventInfo = True
                AppMgr.ServiceMgr.AthenaEventLoopMgr.DoLiteLoop = False


class HistOutputs(JobProperty):
    """A list of outputs to be produced by THistSvc in the format <streamName>:<fileName>"""
    statusOn     = True
    allowedTypes = ['list']
    StoredValue   = []

    def _do_action( self, *args, **kwds ):
        from AthenaCommon import CfgMgr
        from AthenaCommon import AppMgr
        if not hasattr(AppMgr.ServiceMgr,"THistSvc"):
            AppMgr.ServiceMgr += CfgMgr.THistSvc()

        for output in self.StoredValue:
            if ":" not in output:
                self.StoredValue.remove(output)
                raise ValueError("HistOutputs: %s must be in format '<streamName>:<fileName>'" % output)

            streamName = output.split(":",1)[0]
            fileName = output.split(":",1)[1]

            #check the THistSvc has the stream, if not we create it
            outputs = AppMgr.ServiceMgr.THistSvc.Output
            foundStream=False
            for hsOutput in outputs:
                hsStreamName = hsOutput.split(" ",1)[0]
                if hsStreamName==streamName: 
                    foundStream=True
                    break
            if not foundStream:
                AppMgr.ServiceMgr.THistSvc.Output += ["%s DATAFILE='%s' OPT='RECREATE'" % (streamName,fileName)]
    

class AllowIgnoreConfigError(JobProperty):
    """Allow an algorithm to ignore return error code from upstream algorithm
    and tools.
    """
    statusOn     = True
    allowedTypes = ['bool']
    StoredValue  = True

class isOnline(JobProperty):
    """ Set to True when running online
    """
    statusOn     = True
    allowedTypes = ['bool']
    StoredValue  = False


##-----------------------------------------------------------------------------
## 2nd step
## Definition of the AthenaCommon flag container
class AthenaCommonFlags(JobPropertyContainer):
    """Container for the common flags
    """
    pass

##-----------------------------------------------------------------------------
## 3rd step
## adding the container to the general top-level container
jobproperties.add_Container(AthenaCommonFlags)

##-----------------------------------------------------------------------------
## 4th step
## adding athena common flags to the AthenaCommonFlags container
jobproperties.AthenaCommonFlags.add_JobProperty(EvtMax)
jobproperties.AthenaCommonFlags.add_JobProperty(SkipEvents)
jobproperties.AthenaCommonFlags.add_JobProperty(FilesInput )
jobproperties.AthenaCommonFlags.add_JobProperty(TreeName)
jobproperties.AthenaCommonFlags.add_JobProperty(AccessMode)
jobproperties.AthenaCommonFlags.add_JobProperty(HistOutputs)
jobproperties.AthenaCommonFlags.add_JobProperty(AllowIgnoreConfigError)
jobproperties.AthenaCommonFlags.add_JobProperty(isOnline)

##-----------------------------------------------------------------------------
## 5th step
## short-cut for lazy people
## carefull: do not select AthenaCommonFlags as a short name as well. 
## otherwise problems with pickle
## Note: you still have to import it:
## >>> from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
athenaCommonFlags = jobproperties.AthenaCommonFlags
