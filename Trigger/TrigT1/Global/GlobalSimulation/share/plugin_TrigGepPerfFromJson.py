from GlobalSimulation.GlobalSimJsonConfig import GlobalSimJsonCfg
from TrigValTools.TrigValSteering.Common import find_file_in_path

def setup(flags):
    if flags.Concurrency.NumThreads==0:
        flags.Concurrency.NumThreads=1

    flags.DQ.doMonitoring = False
    flags.IOVDb.GlobalTag = "OFLCOND-MC23-SDR-RUN3-11-02"

#jsonpath = find_file_in_path("L0GlobalTestMenuv1_Khoo.json", 'DATAPATH')
jsonpath = find_file_in_path("GlobalSimulation/L0GlobalTestMenuv1_TrigGepPerf.json", 'DATAPATH')

force_TOBs = [
    'OfflineCellsTower',
    'Eratio',
    'GEPMETBasicClusters',
    'GEPMETEtaSKClusters',
    'GEPMETCellsTower',
    'GEPMETTopoTower',
    'GEPMETTCTower',
    'ConeBasicClustersJets',
    'ModAntikTBasicClustersJets',
    # 'WTAConeBasicClustersJets',
    # 'WTAConeEtaSKClustersJets',
    # 'WTAConeCellTowerJets',
]

ignore_prereqs = [
]

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
offline_cfg = ComponentAccumulator()
offline_cfg.addSequence(CompFactory.AthSequencer('TrigGepPerf_OfflineReco'))

from CaloRec.CaloTopoClusterConfig import CaloTopoClusterCfg
offline_cfg.merge(CaloTopoClusterCfg(
        flags,
        cellsname='AllCalo',
        clustersname='CaloCalTopoClusters',
        clustersnapname='CaloTopoClusters',
        cellthresholds=(4,2,0)
    ), sequenceName='TrigGepPerf_OfflineReco')

offline_cfg.merge(CaloTopoClusterCfg(
        flags,
        cellsname='AllCalo',
        clustersname='CaloCalTopoClusters422',
        clustersnapname='CaloTopoClusters422',
        cellthresholds=(4,2,2)
    ), sequenceName='TrigGepPerf_OfflineReco')

# PU suppressed clusters
from JetRecConfig.JetDefinition import JetInputConstitSeq
from JetRecConfig.JetRecConfig import getInputAlgs
from JetRecConfig.StandardJetConstits import stdConstitDic
from ROOT import xAODType

for clusters in [
    'CaloTopoClusters',
    'CaloTopoClusters422',
]:
    for puseq in [
        ['SK'],
        ['Vor'],
        ['Vor','SK']
    ]:
        label = clusters + ''.join(puseq)
        if label not in stdConstitDic:
            stdConstitDic[label] = JetInputConstitSeq(
                name = label,
                objtype = xAODType.CaloCluster,
                modifiers = puseq,
                inputname = clusters,
                outputname = label,
            )
            for alg in getInputAlgs(stdConstitDic[label], flags):
                offline_cfg.addEventAlgo(alg, sequenceName='TrigGepPerf_OfflineReco')

from JetRecConfig.StandardSmallRJets import AntiKt4EMTopo, AntiKt4Truth, AntiKt4EMPFlow
from JetRecConfig.StandardLargeRJets import (
    AntiKt10Truth, AntiKt10TruthSoftDrop,
    AntiKt10UFOCSSK, AntiKt10UFOCSSKSoftDrop
)
from JetRecConfig.JetRecConfig import JetRecCfg
AntiKt1EMTopo = AntiKt4EMTopo.clone(radius=0.1,modifiers=[])
AntiKt2EMTopo = AntiKt4EMTopo.clone(radius=0.2,modifiers=[])
AntiKt3EMTopo = AntiKt4EMTopo.clone(radius=0.3,modifiers=[])
AntiKt10EMTopo = AntiKt4EMTopo.clone(radius=1.0,modifiers=[])
offline_jets = [
        AntiKt1EMTopo, AntiKt2EMTopo, AntiKt3EMTopo, 
        AntiKt4EMTopo, AntiKt10EMTopo,
        AntiKt4Truth, AntiKt4EMPFlow,
        AntiKt10Truth, AntiKt10TruthSoftDrop,
        AntiKt10UFOCSSK, AntiKt10UFOCSSKSoftDrop,
    ]
for j in offline_jets:
    if j.fullname() not in flags.Input.Collections:
        print(f'Scheduling reco of {j.fullname()}')
        offline_cfg.merge(JetRecCfg(flags,j), sequenceName='TrigGepPerf_OfflineReco')

offline_cfg.printConfig()
cfg.merge(offline_cfg)


triggepperf_cfg = GlobalSimJsonCfg(flags, jsonpath, force_TOBs, ignore_prereqs)

# Hack because TrigGepPerf doesn't provide handles to set this
# and at this stage I prefer not to touch their code.
# Depending on the job/input, the name of the available jFEX
# jets may not match the default (L1_jFexSRJetRoISim)
jFexSRname = "L1_jFexSRJetRoI_ReSim"
for alg in triggepperf_cfg.getEventAlgos():
    if hasattr(alg,'jFexSRJetRoIs'):
        alg.jFexSRJetRoIs = jFexSRname

cfg.merge( triggepperf_cfg )

