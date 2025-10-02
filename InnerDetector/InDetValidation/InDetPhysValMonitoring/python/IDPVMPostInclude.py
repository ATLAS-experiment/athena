#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# Convenience functions to schedule algorithms to produce IDPVM output
# during reconstruction.
#  USAGE:
#    --preExec "from InDetPhysValMonitoring.IDPVMPostInclude import setIDPVMFlags;setIDPVMFlags(flags,idpvm_output_file='idpvm.root');"
#    --postInclude 'InDetPhysValMonitoring.IDPVMPostInclude.addIPVM','InDetPhysValMonitoring.IDPVMPostInclude.removePRDFromAOD' \
# "removePRDFromAOD" will remove the MeasurementsAux content from the AOD, which significantly
# reduces the AOD for ITk.

def setIDPVMFlags(flags, idpvm_output_file:str='idpvm.root') :
    flags.PhysVal.OutputFileName = idpvm_output_file

    # Set default truthMinPt depending on Run config
                                
    flags.PhysVal.IDPVM.doExpertOutput   = True
    flags.PhysVal.IDPVM.doValidateTightPrimaryTracks = True
    flags.PhysVal.IDPVM.doHitLevelPlots = True
    flags.PhysVal.IDPVM.runDecoration = False
    flags.PhysVal.IDPVM.doTechnicalEfficiency = True

    # force the vertex for hgg case
    if flags.PhysVal.IDPVM.hardScatterStrategy == 3:
        flags.PhysVal.IDPVM.PrimaryVertexContainer = 'HggPrimaryVertices'

    flags.PhysVal.doExample = False

    if flags.PhysVal.IDPVM.doTechnicalEfficiency :
        flags.Tracking.writeExtendedSi_PRDInfo=True

def addIPVM(flags, cfg) :

    if flags.PhysVal.IDPVM.doPRW:    
        from AthenaConfiguration.ComponentFactory import CompFactory
        cfg.addService(CompFactory.CP.SystematicsSvc("SystematicsSvc"))
        from AsgAnalysisAlgorithms.PileupReweightingAlgConfig import PileupReweightingAlgCfg
        cfg.merge(PileupReweightingAlgCfg(flags))

    from InDetPhysValMonitoring.InDetPhysValDecorationConfig import AddDecoratorCfg
    cfg.merge(AddDecoratorCfg(flags))

    from InDetPhysValMonitoring.InDetPhysValMonitoringConfig import InDetPhysValMonitoringCfg
    cfg.merge(InDetPhysValMonitoringCfg(flags))

def removePRDFromAOD(flags, cfg) :
    '''
    The ExtendedSi_PRDInfo is needed by IDPVM for the technical efficiency, but increases the
    AOD size  significantly, to avoid that remove it from the output item list.
    '''
    import re
    from OutputStreamAthenaPool.OutputStreamConfig import outputStreamName
    StreamAOD = cfg.getEventAlgo(outputStreamName("AOD"))

    new_item_list=[]
    pat=re.compile('.*MeasurementsAux\\..*')
    for elm in StreamAOD.ItemList :
        m = pat.match(elm)
        if m is None :
            new_item_list.append(elm)
        else :
            print('DEBUG remove %s' % elm)
    StreamAOD.ItemList = new_item_list
    print (StreamAOD)
    pass
