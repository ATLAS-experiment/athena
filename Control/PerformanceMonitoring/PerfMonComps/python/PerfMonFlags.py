# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# @file: PerfMonFlags.py
# @purpose: a container of flags for Performance Monitoring
# @author: Sebastien Binet <binet@cern.ch>

"""  A container of flags for Performance Monitoring   

"""
#
#
__author__  = 'Sebastien Binet'
__version__ = "$Revision: 1.17 $"
__doc__     = "A container of flags for Performance Monitoring"

from AthenaCommon.JobProperties import JobProperty, JobPropertyContainer
from AthenaCommon.JobProperties import jobproperties

#
class doMonitoringMT(JobProperty):
    """Flag to active the MT-safe monitoring framework and service(s)
       It of course deactives serial monitoring
    """
    statusOn     = False
    allowedTypes = ['bool']
    StoredValue  = False
    def _do_action(seld):
        # Setup PerfMonMTSvc
        from AthenaCommon.AppMgr import ServiceMgr as svcMgr
        if not hasattr(svcMgr, 'PerfMonMTSvc'):
            from PerfMonComps.MTJobOptCfg import PerfMonMTSvc
            svcMgr += PerfMonMTSvc("PerfMonMTSvc")
        return

    def _undo_action(self):
        # Uninstall the service and the algorithm
        from AthenaCommon.AppMgr import ServiceMgr as svcMgr

        if hasattr(svcMgr, 'PerfMonMTSvc'):
            del svcMgr.PerfMonMTSvc
        return
#
class doFastMonMT(JobProperty):
    """ Flag to active fast MT-safe monitoring framework and service(s)
        It also activates the doMonitoringMT flag
    """
    statusOn     = False
    allowedTypes = ['bool']
    StoredValue  = False
    def _do_action(self):
        jobproperties.PerfMonFlags.doMonitoringMT = True
        return

#
class doFullMonMT(JobProperty):
    """ Flag to activate full MT-safe monitoring framework and service(s)
        It also activate the doMonitoringMT flag
        Note that due to locks this functionality might negatively impact
        concurrency
    """
    statusOn     = False
    allowedTypes = ['bool']
    StoredValue  = False
    def  _do_action(self):
        jobproperties.PerfMonFlags.doMonitoringMT = True
        return

#
class OutputJSON(JobProperty):
    """ Flag to override the default output file name of the perfmonmt json
    """
    statusOn     = True
    allowedTypes = ['str']
    StoredValue  = "perfmonmt.json"

# Defines the container for the performance monitoring flags  
class PerfMonFlags(JobPropertyContainer):
    """ The global performance monitoring flag/job property container.
    """
    pass

# add the perfmon flags container to the top container 
jobproperties.add_Container(PerfMonFlags)


# We want always the following flags in the container  
list_jobproperties = [
    doMonitoringMT,
    doFastMonMT,
    doFullMonMT,
    OutputJSON,
    ]

for i in list_jobproperties:
    jobproperties.PerfMonFlags.add_JobProperty(i)

## module clean-up
del list_jobproperties

## -- helper function to decode athena-command-line options
def _decode_pmon_opts(opts, dry_run=False):
    """helper function to decode athena-command-line options.

    @param opts is a list of options which can enable or disable a few
           jobproperties.PerfMonFlags fields

    @param dry_run to only decode but not set the options

    one activates a perfmon flag by prepending '+' in front of the option name
    (or nothing prepended, '+' being the implied default)
    and de-activates it by prepending '-'.
    """
    pmf = jobproperties.PerfMonFlags
    dispatch = {
        'perfmonmt':  pmf.doMonitoringMT,
        'fastmonmt':  pmf.doFastMonMT,
        'fullmonmt':  pmf.doFullMonMT,
        }

    for opt in opts:
        flag_name = opt[:]
        val = True
        if opt.startswith('-'):
            val = False
            flag_name = flag_name[1:]
        elif opt.startswith('+'):
            val = True
            flag_name = flag_name[1:]
        if flag_name not in dispatch:
            raise ValueError(
                '[%s] is not a valid PerfMonFlag (allowed: %r)' %
                (flag_name, list(dispatch.keys()))
                )
                
        if not dry_run:
            dispatch[flag_name].set_Value(val)
