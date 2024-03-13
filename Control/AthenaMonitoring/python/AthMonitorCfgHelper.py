#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

'''@file AthMonitorCfgHelper.py
@author C. D. Burton
@author P. Onyisi
@date 2019-01-25
@brief Helper classes for Run 3 monitoring algorithm configuration
'''

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

class AthMonitorCfgHelper(object):
    '''
    This class is for the Run 3-style configuration framework. It is intended to be instantiated once
    per group of related monitoring algorithms.
    '''
    def __init__(self, flags, monName):
        '''
        Create the configuration helper. Needs the job flags and the name of the set of
        monitoring algorithms.

        Arguments:
        flags -- the configuration flag object
        monName -- the name you want to assign the family of algorithms
        '''
        
        AthSequencer=CompFactory.AthSequencer
        self.flags = flags
        self.monName = monName
        self.monSeq = AthSequencer('AthMonSeq_' + monName)
        self.monSeq.StopOverride=True
        self.resobj = ComponentAccumulator()
        self.resobj.addSequence(self.monSeq)
        if self.flags.DQ.useTrigger:
            from .TriggerInterface import TrigDecisionToolCfg
            self.resobj.merge(TrigDecisionToolCfg(flags))

    def addAlgorithm(self, algClassOrObj, name = None, addFilterTools = [], *args, **kwargs):
        '''
        Instantiate/add a monitoring algorithm

        Arguments:
        algClassOrObj -- the Configurable class object of the algorithm to create, or an instance
                         of the algorithm Configurable. The former is recommended.  In the former case,
                         the name argument is required.
        name -- the name of the algorithm to create. Required when passing a Configurable class object
                as algClassOrObj.  No effect if a Configurable instance is passed.
        *args, **kwargs -- additional arguments will be forwarded to the Configurable constructor if
                           a Configurable class object is passed. No effect if a Configurable instance
                           is passed.

        Returns:
        algObj -- an algorithm Configurable object
        '''
        from inspect import isclass
        if isclass(algClassOrObj):
            if name is None:
                raise TypeError('addAlgorithm with a class argument requires a name for the algorithm')
            algObj = algClassOrObj(name, *args, **kwargs)
        else:
            algObj = algClassOrObj

        # configure these properties; users really should have no reason to override them
        algObj.Environment = self.flags.DQ.Environment
        algObj.DataType = self.flags.DQ.DataType.value
        if self.flags.DQ.useTrigger:
            algObj.TrigDecisionTool = self.resobj.getPublicTool("TrigDecisionTool")

        if self.flags.DQ.enableLumiAccess:
            algObj.EnableLumi = True
            from LumiBlockComps.LuminosityCondAlgConfig import LuminosityCondAlgCfg
            self.resobj.merge (LuminosityCondAlgCfg (self.flags))
            if not self.flags.Input.isMC:
                from LumiBlockComps.LBDurationCondAlgConfig import LBDurationCondAlgCfg
                from LumiBlockComps.TrigLiveFractionCondAlgConfig import TrigLiveFractionCondAlgCfg
                self.resobj.merge (LBDurationCondAlgCfg (self.flags))
                self.resobj.merge (TrigLiveFractionCondAlgCfg (self.flags))
        else:
            algObj.EnableLumi = False

        # add some optional filters?
        for obj in addFilterTools:
            # accept either ComponentAccumulators or tools (trusting that user has already merged CA)
            if isinstance(obj, ComponentAccumulator):
                filter = self.resobj.popToolsAndMerge(obj)
            elif hasattr(obj, 'getGaudiType') and obj.getGaudiType() == 'AlgTool':
                filter = obj
            else:
                raise ValueError(f'Object {obj} passed to addFilterTools is not a ComponentAccumulator or an AlgTool')
            algObj.FilterTools += [filter]


        self.resobj.addEventAlgo(algObj, sequenceName=self.monSeq.name)
        return algObj

    def addGroup(self, alg, name, topPath='', defaultDuration='run'):
        '''Add a group to an algorithm

        Technically, adding a GenericMonitoringTool instance. The name given here can be
        used to retrieve the group from within the algorithm when calling the fill()
        function. Note this is *not* the same thing as the Monitored::Group class. To
        avoid replication of code, this calls the more general case, getArray with an 1D
        array of length 1.

        Arguments:
        alg -- algorithm Configurable object (e.g. one returned from addAlgorithm)
        name -- name of the group
        topPath -- directory name in the output ROOT file under which histograms will be
                   produced
        defaultDuration -- default time between histogram reset for all histograms in
                           group; can be overridden for each specific histogram

        Returns:
        tool -- a GenericMonitoringTool Configurable object. This can be used to define
                histograms associated with that group (using defineHistogram).
        '''
        if alg is not None and name == alg.getName():
            raise ValueError(f"Cannot have a monitoring group with the same name as the parent algorithm ({name})")
        array = self.addArray([1],alg,name,topPath=topPath,defaultDuration=defaultDuration)
        return array[0]

    def addArray(self, dimensions, alg, baseName, topPath='', defaultDuration='run'):
        '''Add many groups to an algorithm

        Arguments:
        dimensions -- list holding the size in each dimension [n1,n2,n3,n4,...]
        alg -- algorithm Configurable object
        baseName -- base name of the group. postfixes are added by GMT Array initialize
        topPath -- directory name in the output ROOT file under which histograms will be
                   produced
        duration -- default time between histogram reset for all histograms in group

        Returns:
        tool -- a GenericMonitoringToolArray object. This is used to define histograms
                associated with each group in the array.
        '''
        # Generate the n-dimensional array
        from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringArray
        array = GenericMonitoringArray(self.flags, baseName, dimensions)

        acc = getDQTHistSvc(self.flags)
        self.resobj.merge(acc)

        pathToSet = self.flags.DQ.FileKey+('/%s' % topPath if topPath else '')
        if self.flags.Output.HISTFileName:
            pathToSet = '/' + pathToSet
        array.broadcast('HistPath',pathToSet)
        array.broadcast('UseCache',True)
        convention = 'ONLINE' if self.flags.Common.isOnline else 'OFFLINE'
        array.broadcast('convention', convention)
        array.broadcast('defaultDuration',defaultDuration)
        if alg is not None:
            alg.GMTools += array.toolList()
        return array

    def result(self):
        '''
        Finalize the creation of the set of monitoring algorithms.

        Returns:
        resobj -- a ComponentAccumulator 
        '''
        return self.resobj


def getDQTHistSvc(flags):
    '''
    This function creates a THistSvc - used for the new-style job configuration
    
    Returns:
    (result, histsvc) -- a tuple of (ComponentAccumulator, THistSvc Configurable object)
    '''
    result = ComponentAccumulator()
    histsvc = CompFactory.THistSvc()
    if flags.Output.HISTFileName:
        histsvc.Output += ["%s DATAFILE='%s' OPT='RECREATE'" % (flags.DQ.FileKey, 
                                                                flags.Output.HISTFileName)]
    result.addService(histsvc)
    return result
