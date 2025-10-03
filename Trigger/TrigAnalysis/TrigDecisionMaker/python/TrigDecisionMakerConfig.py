# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import Format


def Run1Run2DecisionMakerCfg(flags):
    """Configures HLTNavigation(tool) -> xAODNavigation and TrigDec::TrigDecision -> xAOD::TrigDecision """
    acc = ComponentAccumulator()
    doL1=flags.Trigger.L1.doCTP
    doL2=flags.Trigger.decodeHLT
    doEF=flags.Trigger.decodeHLT
    doHLT=flags.Trigger.decodeHLT


    if 'HLT' not in flags.Trigger.availableRecoMetadata:
        doL2=False
        doEF=False
        doHLT=False
        
    if 'L1' not in flags.Trigger.availableRecoMetadata:
        doL1=False

    if flags.Trigger.EDMVersion == 1:  # Run-1 has L2 and EF result
        doHLT = False
    else:
        doL2 = False
        doEF = False

    L1ResultKey = "" if flags.Input.Format is Format.BS else "Lvl1Result"
    
    decMaker = CompFactory.TrigDec.TrigDecisionMaker( 'TrigDecMaker', 
                                                      doL1 = doL1,
                                                      doL2 = doL2,
                                                      doEF = doEF,
                                                      doHLT = doHLT,
                                                      L1ResultKey = L1ResultKey)
    acc.addEventAlgo(decMaker)


    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
    acc.merge(TrigDecisionToolCfg(flags))

    from TrigConfxAOD.TrigConfxAODConfig import getxAODConfigSvc
    cnvTool = CompFactory.xAODMaker.TrigDecisionCnvTool('TrigDecisionCnvTool', 
                                                        TrigConfigSvc = acc.getPrimaryAndMerge( getxAODConfigSvc( flags )) )


    decCnv = CompFactory.xAODMaker.TrigDecisionCnvAlg(CnvTool = cnvTool)    
    if "EventInfo#EventInfo" not in flags.Input.TypedCollections:
        decCnv.EventInfoKey=""

    acc.addEventAlgo(decCnv)
    if doHLT or doEF:
        # TrigNavigationCnvAlg runs on the original HLTResult in the bytestream as it used
        # to be in serial athena, i.e. it does not run on the modified HLTResult created by
        # TrigBSExtraction. See also ATLASRECTS-6453.
        from SGComps.AddressRemappingConfig import InputRenameCfg

        # Even for Run-1, we only convert the EF result:
        aodKey = "HLTResult_HLT" if doHLT else "HLTResult_EF"

        acc.merge(InputRenameCfg("HLT::HLTResult", aodKey, aodKey+"_BS"))
        acc.addEventAlgo( CompFactory.xAODMaker.TrigNavigationCnvAlg('TrigNavigationCnvAlg',
                                                                    AODKey = aodKey,
                                                                    xAODKey = "TrigNavigation") )

    return acc

def Run3DecisionMakerCfg(flags):
    acc = ComponentAccumulator()
    tdm = CompFactory.TrigDec.TrigDecisionMakerMT()
    tdm.doL1 = flags.Trigger.L1.doCTP
    tdm.doHLT = flags.Trigger.decodeHLT
    tdm.UseEDMxAOD = flags.Trigger.CTP.UseEDMxAOD
    if flags.Input.Format is not Format.BS:
        # Construct trigger bits from HLTNav_summary instead of reading from BS
        tdm.BitsMakerTool = CompFactory.TriggerBitsMakerTool()
    acc.addEventAlgo( tdm )

    # Validate that the output of the TrigDecisionMakerMT is sensible.
    if flags.Trigger.DecisionMakerValidation.Execute:
        tdmv = CompFactory.TrigDec.TrigDecisionMakerValidator()
        tdmv.doL1 = flags.Trigger.L1.doCTP
        tdmv.doHLT = flags.Trigger.decodeHLT
        tdmv.samplingFrequency = 1
        tdmv.errorOnFailure = flags.Trigger.DecisionMakerValidation.ErrorMode
        tdmv.EDMVersion = flags.Trigger.EDMVersion
        from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg, getRun3NavigationContainerFromInput
        tdmv.TrigDecisionTool = acc.getPrimaryAndMerge(TrigDecisionToolCfg(flags))
        tdmv.NavigationKey = getRun3NavigationContainerFromInput(flags)
        acc.addEventAlgo( tdmv )

    return acc
