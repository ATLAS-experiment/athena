#
#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#

'''@file MissingEtDAQConfig.py
@contact atlas-cp-jetetmiss-jsv-conveners@cern.ch
@brief Main CA-based python configuration for MissingEtDQA
'''

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def PhysValMETCfg(flags, **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("EnableLumi", False)
    kwargs.setdefault("DetailLevel", 10)
    kwargs.setdefault("DoTruth", flags.Input.isMC)

    from METUtilities.METMakerConfig import METMakerToolCfg
    # for EMTopo jets no NNJvt is calculated so we need to fall back to Jvt (re-calculated in MissingEtDQA::PhysValMET as "NewJvt")
    METMakerTopo = acc.popToolsAndMerge(METMakerToolCfg(flags,
                                                        jetCollection="AntiKt4EMTopoJets",
                                                        name="METMaker_AntiKt4Topo",
                                                        JetSelection="Loose",
                                                        DoPFlow=False))
    METMakerPFlow = acc.popToolsAndMerge(METMakerToolCfg(flags,
                                                         jetCollection="AntiKt4EMPFlowJets",
                                                         name="METMaker_AntiKt4PFlow",
                                                         JetSelection="Loose",
                                                         DoPFlow=True))

    kwargs.setdefault("METMakerTopo", METMakerTopo )
    kwargs.setdefault("METMakerPFlow", METMakerPFlow )

    from METUtilities.METMakerConfig import METMuonSelectionToolCfg,METElectronSelectionToolCfg,METPhotonSelectionToolCfg,METTauSelectionToolCfg
    kwargs.setdefault("MuonSelectionTool", acc.popToolsAndMerge(METMuonSelectionToolCfg(flags)))
    kwargs.setdefault("ElectronLHSelectionTool", acc.popToolsAndMerge(METElectronSelectionToolCfg(flags)))
    kwargs.setdefault("PhotonIsEMSelectionTool", acc.popToolsAndMerge(METPhotonSelectionToolCfg(flags)))
    kwargs.setdefault("TauSelectionTool", acc.popToolsAndMerge(METTauSelectionToolCfg(flags)))

    from AthenaConfiguration.AutoConfigFlags import GetFileMD
    metadata = GetFileMD(flags.Input.Files)
    isDAOD_PHYSVAL=False
    for class_name, name in metadata['metadata_items'].items():
        if name == 'EventStreamInfo':
            if "DAOD_PHYSVAL" in class_name :
                print ("Running on DAOD_PHYSVAL - will not add TTVA decorations.")
                isDAOD_PHYSVAL=True
            break
    kwargs.setdefault("InputIsDAOD", isDAOD_PHYSVAL)

    kwargs.setdefault("DoMETRefPlots", "xAOD::MissingETContainer#MET_Reference_AntiKt4EMTopo" in flags.Input.TypedCollections)

    tool = CompFactory.MissingEtDQA.PhysValMET(**kwargs)
    acc.setPrivateTools(tool)
    return acc
