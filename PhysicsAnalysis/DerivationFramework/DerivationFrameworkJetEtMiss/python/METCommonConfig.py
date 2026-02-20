# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#********************************************************************
# METCommonConfig.py
# Configures DF MET content building and association tools
# Component accumulator version  
#********************************************************************



def METCommonCfg(ConfigFlags):
    """Configure MET for the derivation framework"""

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from METReconstruction.METAssociatorCfg import METAssociatorCfg

    acc = ComponentAccumulator()

    metDefs = ['AntiKt4EMTopo']
    if ConfigFlags.MET.DoPFlow:
        metDefs.append('AntiKt4EMPFlow')

    for metDef in metDefs:
        acc.merge(METAssociatorCfg(ConfigFlags, metDef))

    return acc

def METCustomVtxCfg(ConfigFlags, vxColl, jetColl, constituentColl):
    from METReconstruction.METAssocCfg import METAssocConfig, AssocConfig
    from METReconstruction.METAssociatorCfg import getAssocCA

    associators = [AssocConfig('CustomJet', jetColl+'Jets'),
                    AssocConfig('Muon'),
                    AssocConfig('Ele'),
                    AssocConfig('Gamma'),
                    AssocConfig('Tau'),
                    AssocConfig('Soft')]

    cfg = METAssocConfig(jetColl,
                         ConfigFlags,
                         buildconfigs = associators,
                         doPFlow = ('PFlow' in jetColl),
                         usePFOLinks = ConfigFlags.MET.UseFELinks,
                         modConstKey = constituentColl)

    for assoc in cfg.assoclist:
        assoc.PrimVxColl = vxColl

    return getAssocCA(cfg, METName='CustomJet')


def METLRTCfg(ConfigFlags, jetType):
    from METReconstruction.METAssocCfg import METAssocConfig, AssocConfig
    from METReconstruction.METAssociatorCfg import getAssocCA

    jetColl = {"AntiKt4LCTopo" : "LCJet",
               "AntiKt4EMTopo" : "EMJet",
               "AntiKt4EMPFlow" : "PFlowJet"}

    associators = [ AssocConfig(jetColl[jetType]),
                    AssocConfig('Muon'),
                    AssocConfig('MuonLRT'),
                    AssocConfig('Ele'),
                    AssocConfig('LRTEle'),
                    AssocConfig('Gamma'),
                    AssocConfig('Tau'),
                    AssocConfig('Soft')]

    modConstKey = ""
    modClusColls = {}
    if ConfigFlags.MET.UseTracks:
        modConstKey="OriginCorr"
        modClusColls={
            'LCOriginCorrClusters':'LCOriginTopoClusters',
            'EMOriginCorrClusters':'EMOriginTopoClusters'
            }
    usePFlow = ('PFlow' in jetType)
    cfg = METAssocConfig(jetType+"_LRT",
                         ConfigFlags,
                         buildconfigs = associators,
                         doPFlow = usePFlow,
                         usePFOLinks = (ConfigFlags.MET.UseFELinks if usePFlow else False),
                         modConstKey=("" if usePFlow else modConstKey),
                         modClusColls=({} if usePFlow else modClusColls) )

    return getAssocCA(cfg, METName=jetType+'_LRT')

def METRemappingCfg(ConfigFlags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator, CompFactory

    acc = ComponentAccumulator()
    acc.addEventAlgo(CompFactory.DerivationFramework.METRemappingAlg('AnalysisMETRemappingAlg'))

    return acc

def HadRecoilMETCfg(ConfigFlags):
    from METReconstruction.METAssocCfg import METAssocConfig, AssocConfig
    from METReconstruction.METAssociatorCfg import getAssocCA


    jettype='HadronicRecoil' # -> i.e. dont build assocs to jets at all
    assocname='AntiKt4EMPFlowHR'
    doPFlow=True
    doRecoil=True

    #only mu, e and soft assocs are needed
    associators = [AssocConfig(jettype),
                    AssocConfig('Muon'),
                    AssocConfig('Ele'),
                    AssocConfig('Soft')]

    cfg = METAssocConfig(assocname,
                         ConfigFlags,
                         buildconfigs = associators,
                         doPFlow = doPFlow,
                         doRecoil = doRecoil,
                         usePFOLinks = ConfigFlags.MET.UseFELinks)


    return getAssocCA(cfg, METName=assocname)
