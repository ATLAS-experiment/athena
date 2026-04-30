# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

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
