# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from JetRecConfig.StandardSmallRJets import AntiKt4EMPFlow, AntiKt4LCTopo, AntiKt4EMTopo, AntiKt4Truth
from JetRecConfig.StandardLargeRJets import AntiKt10LCTopo_noVR, AntiKt10UFOCSSKSoftDrop_trigger
from JetRecConfig.JetRecConfig import JetRecCfg

def addTruthPileupJetsToOutputCfg(flags, toAOD=True, toESD=True):
    result = ComponentAccumulator()

    jetdefs = ["InTimeAntiKt4TruthJets", "OutOfTimeAntiKt4TruthJets"]

    jetList = []
    for jetdef in jetdefs:
        if f"xAOD::JetContainer#{jetdef}" in flags.Input.TypedCollections:
            jetList += [ f"xAOD::JetContainer#{jetdef}" ,
                         f"xAOD::AuxContainerBase!#{jetdef}Aux.-PseudoJet.-constituentLinks.-constituentWeights"]

    from OutputStreamAthenaPool.OutputStreamConfig import addToESD, addToAOD
    if toESD:
        result.merge(addToESD(flags, jetList))
    if toAOD:
        result.merge(addToAOD(flags, jetList))

    return result


def addJetsToOutputCfg(flags,jetdefs, toAOD=True, toESD=True):
    """Write out the jet containers as defined by jetdefs (a list of JetDefinition).
    
    In Run3 we don't write out jets in AOD : this function is left for convenience and testing purpose.
    """
    result = ComponentAccumulator()

    #--------------------------------------------------------------
    # Build output container list.
    #--------------------------------------------------------------
    jetList = []

    for jetdef in jetdefs:
        jetList += [ f"xAOD::JetContainer#{jetdef.fullname()}" ,
                     f"xAOD::JetAuxContainer#{jetdef.fullname()}Aux.-PseudoJet"]
        # Not sure if this trigger special AuxContainer is obsolete in Run3 ?
        # if trigger:
        #   auxprefix = "Trig"
        #   jetAODList += [ f"xAOD::Jet{auxprefix}AuxContainer#{jetdef.fullname()}Aux." ,

    # Store event shapes when jets are being stored to output
    jetList += ["xAOD::EventShape#Kt4EMPFlowEventShape",
                "xAOD::EventShapeAuxInfo#Kt4EMPFlowEventShapeAux.",
                "xAOD::EventShape#Kt4EMPFlowNeutEventShape",
                "xAOD::EventShapeAuxInfo#Kt4EMPFlowNeutEventShapeAux.",
                "xAOD::EventShape#Kt4EMPFlowPUSBEventShape",
                "xAOD::EventShapeAuxInfo#Kt4EMPFlowPUSBEventShapeAux.",
                "xAOD::EventShape#Kt4EMTopoOriginEventShape",
                "xAOD::EventShapeAuxInfo#Kt4EMTopoOriginEventShapeAux.",
                "xAOD::EventShape#Kt4LCTopoOriginEventShape",
                "xAOD::EventShapeAuxInfo#Kt4LCTopoOriginEventShapeAux."]

    from OutputStreamAthenaPool.OutputStreamConfig import addToESD, addToAOD
    if toESD:
        result.merge(addToESD(flags, jetList))
    if toAOD:
        result.merge(addToAOD(flags, jetList))

    return result

def JetRecoSteeringCfg(flags):
    result = ComponentAccumulator()
    
    # the Standard list of jets to run :
    jetdefs = [AntiKt4EMTopo, AntiKt4EMPFlow, AntiKt4LCTopo, AntiKt4Truth, AntiKt10LCTopo_noVR, AntiKt10UFOCSSKSoftDrop_trigger]

    # We're in Reco job : propagate this info to the runIII jet config
    # (see JetConfigFlags.py for motivations on this way of doing)
    
    #--------------------------------------------------------------
    # Create the jet algs from the jet definitions
    #--------------------------------------------------------------
    for jd in jetdefs:
        result.merge(JetRecCfg(flags, jd))

    if flags.Output.doWriteAOD and flags.Jet.WriteToAOD:
        result.merge(addJetsToOutputCfg(flags, jetdefs, toAOD=True, toESD=False))
    if flags.Output.doWriteESD:
        jetdefs.remove(AntiKt10UFOCSSKSoftDrop_trigger)
        result.merge(addJetsToOutputCfg(flags, jetdefs, toAOD=False, toESD=True))

    return result

def JetRecoSteeringTest(flags=None):

    if flags is None:
        from AthenaConfiguration.AllConfigFlags import initConfigFlags
        flags = initConfigFlags()

        from AthenaConfiguration.TestDefaults import defaultTestFiles, defaultConditionsTags
        flags.Input.Files = defaultTestFiles.AOD_RUN3_MC
        flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_MC

        # We have to set the production step, which PFFlow muon linking uses for autoconfiguration.
        from AthenaConfiguration.Enums import ProductionStep
        flags.Common.ProductionStep=ProductionStep.Derivation

        flags.fillFromArgs()
        flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    # Setup calorimeter geometry, which is needed for jet reconstruction
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    acc.merge(LArGMCfg(flags))

    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge(TileGMCfg(flags))

    acc.merge(JetRecoSteeringCfg(flags))

    # We also need to build links between the newly
    # created jet constituents (GlobalFE)
    # and electrons,photons,muons and taus
    from eflowRec.PFCfg import PFGlobalFlowElementLinkingCfg
    acc.merge(PFGlobalFlowElementLinkingCfg(flags))

    acc.run()

if __name__=="__main__":
    JetRecoSteeringTest()