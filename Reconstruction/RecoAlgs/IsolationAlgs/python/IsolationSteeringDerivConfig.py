# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

__doc__ = """
          Instantiate the pflow isolation for egamma in derivation
          """

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def IsolationSteeringDerivCfg(flags, name = 'IsolationSteeringDeriv', inType = 'EMPFlow'):

    mlog = logging.getLogger(name)
    mlog.info('Starting Isolation steering')

    acc = ComponentAccumulator()

    suff = 'CSSK' if inType.find('CSSK') >= 0 else ''
    from IsolationAlgs.IsoDensityConfig import NFlowInputAlgCfg
    acc.merge(NFlowInputAlgCfg(flags,InputType = inType))
    # Prepare densities (not needed in HI, because no pu subtraction)
    if not flags.HeavyIon.Egamma.doSubtractedClusters:
        from IsolationAlgs.IsoDensityConfig import DensityForIsoAlgCfg
        acc.merge(DensityForIsoAlgCfg(flags,name='CentralDensityFor'+suff+'NFlowIso'))
        acc.merge(DensityForIsoAlgCfg(flags,name='ForwardDensityFor'+suff+'NFlowIso'))

    # Prepare CaloIsolationTool
    kwargs = dict()
    margs = dict()
    margs['FlowElementsInConeTool'] = CompFactory.xAOD.FlowElementsInConeTool(
        name=suff+'FlowElementsInConeTool')
    if len(suff) > 0:
        kwargs['CustomConfigurationNameEl'] = suff
        kwargs['CustomConfigurationNamePh'] = suff
        margs['EFlowEDCentralContainer'] = 'NeutralParticle'+suff+'FlowIsoCentralEventShape'
        margs['EFlowEDForwardContainer'] = 'NeutralParticle'+suff+'FlowIsoForwardEventShape'
        margs['FlowElementsInConeTool'].PFlowKey=suff+'NeutralParticleFlowObjects'
    from IsolationAlgs.IsoToolsConfig import EGammaCaloIsolationToolCfg
    kwargs['PFlowIsolationTool'] = acc.popToolsAndMerge(EGammaCaloIsolationToolCfg(flags,**margs))

    # Prepare IsolationBuilder
    from xAODPrimitives.xAODIso import xAODIso as isoPar
    isoType  = [ [ isoPar.neflowisol20, isoPar.neflowisol30, isoPar.neflowisol40 ] ]
    isoCor   = [ [ isoPar.coreCone ] ]
    if not flags.HeavyIon.Egamma.doSubtractedClusters:
        isoCor[0].append(isoPar.pileupCorrection)
    isoExCor = [ [ isoPar.coreConeSC ] ]
    kwargs['ElIsoTypes'] = isoType
    kwargs['ElCorTypes'] = isoCor
    kwargs['ElCorTypesExtra'] = isoExCor
    kwargs['PhIsoTypes'] = isoType
    kwargs['PhCorTypes'] = isoCor
    kwargs['PhCorTypesExtra'] = isoExCor

    kwargs['name'] = suff+'PFlowIsolationBuilder'
    
    acc.addEventAlgo(CompFactory.IsolationBuilder(**kwargs))

    mlog.info("PFlow isolation configured")

    return acc

def LRTElectronIsolationSteeringDerivCfg(flags, name = 'LRTElectronCaloIsolationSteeringDeriv'):

    mlog = logging.getLogger(name)
    mlog.info('Starting LRT electron calo Isolation steering')

    acc = ComponentAccumulator()

    # Need to add density inputs for mc20 (containers present in MC21 AODs)
    from IsolationAlgs.IsoDensityConfig import (
        NFlowInputAlgCfg, DensityForIsoAlgCfg)
    acc.merge(NFlowInputAlgCfg(flags,InputType = "EMPFlow"))
    acc.merge(DensityForIsoAlgCfg(flags,name='CentralDensityForNFlowIso'))
    acc.merge(DensityForIsoAlgCfg(flags,name='ForwardDensityForNFlowIso'))

    # Prepare CaloIsolationTool
    kwargs = dict()
    the_pflowElementsTool = CompFactory.xAOD.FlowElementsInConeTool(
            name='FlowElementsInConeTool')

    from IsolationAlgs.IsoToolsConfig import EGammaCaloIsolationToolCfg
    cisoTool = acc.popToolsAndMerge(EGammaCaloIsolationToolCfg(flags,
                                                               FlowElementsInConeTool = the_pflowElementsTool))

    # Prepare IsolationBuilder
    from xAODPrimitives.xAODIso import xAODIso as isoPar
    isoType  = [ [ isoPar.topoetcone20, isoPar.topoetcone30, isoPar.topoetcone40 ],
                 [ isoPar.neflowisol20, isoPar.neflowisol30, isoPar.neflowisol40 ] ]
    isoCor   = [ [ isoPar.core57cells, isoPar.ptCorrection, isoPar.pileupCorrection ],
                 [ isoPar.coreCone, isoPar.pileupCorrection ] ]
    isoExCor = [ [ ], [ isoPar.coreConeSC ] ]

    acc.addEventAlgo(CompFactory.IsolationBuilder(**kwargs,
                                                  name = 'LRTElectronCaloIsolationBuilder',
                                                  ElectronCollectionContainerName = 'LRT'+flags.Egamma.Keys.Output.Electrons,
                                                  ElIsoTypes = isoType,
                                                  ElCorTypes = isoCor,
                                                  ElCorTypesExtra = isoExCor,
                                                  CaloTopoIsolationTool = cisoTool,
                                                  PFlowIsolationTool = cisoTool))

    mlog.info("LRTElectron calo isolation configured")

    return acc

def LRTMuonIsolationSteeringDerivCfg(flags, name = 'LRTMuonCaloIsolationSteeringDeriv'):

    mlog = logging.getLogger(name)
    mlog.info('Starting LRT muon calo Isolation steering')

    acc = ComponentAccumulator()

    # Need to add density inputs for mc20 (containers present in MC21 AODs)
    from IsolationAlgs.IsoDensityConfig import (
        NFlowInputAlgCfg, DensityForIsoAlgCfg)
    acc.merge(NFlowInputAlgCfg(flags,InputType = "EMPFlow"))
    acc.merge(DensityForIsoAlgCfg(flags,name='CentralDensityForNFlowIso'))
    acc.merge(DensityForIsoAlgCfg(flags,name='ForwardDensityForNFlowIso'))

    # Prepare CaloIsolationTool
    the_pflowElementsTool = CompFactory.xAOD.FlowElementsInConeTool(
            name='FlowElementsInConeTool')
    
    from IsolationAlgs.IsoToolsConfig import MuonCaloIsolationToolCfg
    cisoTool = acc.popToolsAndMerge(MuonCaloIsolationToolCfg(flags,
                                    FlowElementsInConeTool =  the_pflowElementsTool ))
    # Prepare IsolationBuilder
    from xAODPrimitives.xAODIso import xAODIso as isoPar
    misoType  = [ [ isoPar.topoetcone20, isoPar.topoetcone30, isoPar.topoetcone40 ],
                  [ isoPar.neflowisol20, isoPar.neflowisol30, isoPar.neflowisol40 ] ]
    misoCor   = [ [ isoPar.coreCone, isoPar.pileupCorrection ],
                  [ isoPar.coreCone, isoPar.pileupCorrection ] ]
    misoExCor = [ [ ], [ ] ]

    acc.addEventAlgo(CompFactory.IsolationBuilder(name= "LRTMuonCaloIsolationBuilder",
                                                  MuonCollectionContainerName = "MuonsLRT",
                                                  MuCorTypesExtra = misoExCor,
                                                  MuCorTypes = misoCor,
                                                  CaloTopoIsolationTool = cisoTool,
                                                  PFlowIsolationTool = cisoTool,
                                                  MuIsoTypes = misoType ))

    mlog.info("MuonLRT calo isolation configured")

    return acc

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.ComponentAccumulator import printProperties
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RDO_RUN2
    flags.Output.doWriteESD = True  # To test the ESD parts
    flags.Output.doWriteAOD = True  # To test the AOD parts
    flags.lock()

    mlog = logging.getLogger("isolationConfigTest")
    mlog.info("Configuring isolation: ")

    acc = MainServicesCfg(flags)
    acc.merge(IsolationSteeringDerivCfg(flags))
    acc.merge(LRTElectronIsolationSteeringDerivCfg(flags))
    acc.merge(LRTMuonIsolationSteeringDerivCfg(flags))
    acc.printConfig(withDetails=True,
                    printDefaults=True)
    printProperties(mlog,
                    acc.getEventAlgo('PFlowIsolationBuilder'),
                    nestLevel=1,
                    printDefaults=True)
    printProperties(mlog,
                    acc.getEventAlgo('LRTElectronCaloIsolationBuilder'),
                    nestLevel=1,
                    printDefaults=True)
    printProperties(mlog,
                    acc.getEventAlgo('LRTMuonCaloIsolationBuilder'),
                    nestLevel=1,
                    printDefaults=True)

    with open("isolationconfig.pkl", "wb") as f:
        acc.store(f)
