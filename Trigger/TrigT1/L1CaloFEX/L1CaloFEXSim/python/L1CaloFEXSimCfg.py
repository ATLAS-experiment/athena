#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import logging

def ReadSCellFromPoolFileCfg(flags, key='SCell'):
    '''Configure reading SCell container from a Pool file like RDO or ESD'''
    acc = ComponentAccumulator()

    # Ensure SCell container is in the input file
    # TODO this needs to be uncommented once all MC files used in tests contain SCells
    # e.g. test_trig_mc_v1DevHI_build.py
    # assert key in flags.Input.Collections or not flags.Input.Collections, 'MC input file is required to contain SCell container'

    # Need geometry and conditions for the SCell converter from POOL
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    acc.merge(LArGMCfg(flags))

    return acc


def ReadSCellFromByteStreamCfg(flags, key='SCell', SCmask=True):
    acc=ComponentAccumulator()

    # Geometry, conditions and cabling setup
    from TileGeoModel.TileGMConfig import TileGMCfg
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    from LArCabling.LArCablingConfig import LArLATOMEMappingCfg
    from LArCabling.LArCablingConfig import LArOnOffIdMappingSCCfg
    from LArCellRec.LArRAWtoSuperCellConfig import LArRAWtoSuperCellCfg
    acc.merge(TileGMCfg(flags))
    acc.merge(LArGMCfg(flags))
    acc.merge(LArLATOMEMappingCfg(flags))
    acc.merge(LArOnOffIdMappingSCCfg(flags))

    # Conversion from ByteStream to LArRawSCContainer
    decoderTool = CompFactory.LArLATOMEDecoder('LArLATOMEDecoder', ProtectSourceId = True)
    decoderAlg = CompFactory.LArRawSCDataReadingAlg('LArRawSCDataReadingAlg', LATOMEDecoder=decoderTool)
    acc.addEventAlgo(decoderAlg)

    acc.merge(LArRAWtoSuperCellCfg(flags,mask=SCmask, SCellContainerOut=key) )

    return acc

def eFEXTOBEtToolCfg(flags):
    """
    Configure the eFEX TOB Et Tool which recalculates isolation variables
    The tool requires eTowers as inputs (add eTowerMaker algorithm)
    """
    acc = ComponentAccumulator()

    # had to comment this out for now, because it causes a clash with the eTowerMakerFromEfexTowers algorithm
    # if that gets scheduled
    #eTowerMakerAlg = CompFactory.LVL1.eTowerMakerFromSuperCells('eTowerMakerFromSuperCells')
    #acc.addEventAlgo(eTowerMakerAlg)

    eFEXTOBEtTool = CompFactory.LVL1.eFEXTOBEtTool
    acc.setPrivateTools(eFEXTOBEtTool())

    return acc

def TriggerTowersInputCfg(flags):
    '''Configuration to provide TriggerTowers as input to the Fex simulation'''
    from AthenaConfiguration.Enums import Format
    if flags.Input.Format is Format.POOL:
        # For POOL files produce TT with R2TTMaker
        from TrigT1CaloSim.TrigT1CaloSimRun2Config import Run2TriggerTowerMakerCfg
        return Run2TriggerTowerMakerCfg(flags)
    else:
        # For RAW decode TT from ByteStream
        from TrigT1CaloByteStream.LVL1CaloRun2ByteStreamConfig import LVL1CaloRun2ReadBSCfg
        return LVL1CaloRun2ReadBSCfg(flags)

def L1CaloFEXSimCfg(flags, eFexTowerInputs = ["L1_eFexDataTowers","L1_eFexEmulatedTowers"],deadMaterialCorrections=True, outputSuffix="", simulateAltTau=False):
    from AthenaConfiguration.Enums import Format

    if not simulateAltTau and flags.DQ.Environment == "tier0":
        simulateAltTau = True # require alt RoI at tier0 while validating new BDT alg against heuristic

    acc = ComponentAccumulator()

    log = logging.getLogger('L1CaloFEXSimCfg')


    # Configure SCell inputs
    sCellType = flags.Trigger.L1.L1CaloSuperCellContainerName
    if flags.Input.Format is Format.POOL:
        # Read SCell directly from input RDO file unless not necessary
        if 'L1_eFexEmulatedTowers' in eFexTowerInputs and "L1_eFexEmulatedTowers" not in flags.Input.Collections:
            acc.merge(ReadSCellFromPoolFileCfg(flags,sCellType))
        if flags.Input.isMC:
            # wont have eFexDataTowers available so remove that if it appears in input list
            eFexTowerInputs = [l for l in eFexTowerInputs if l != "L1_eFexDataTowers"]
    else:
        from AthenaConfiguration.Enums import LHCPeriod
        if flags.GeoModel.Run is LHCPeriod.Run2:
            # Run-2 data inputs, emulate SCells
            from TrigT1CaloFexPerf.EmulationConfig import emulateSC_Cfg
            acc.merge(emulateSC_Cfg(flags))
        else:
            # Run-3+ data inputs, decode SCells from ByteStream if needed
            if 'L1_eFexEmulatedTowers' in eFexTowerInputs and "L1_eFexEmulatedTowers" not in flags.Input.Collections:
                acc.merge(ReadSCellFromByteStreamCfg(flags,key=sCellType))

    # Need also TriggerTowers as input .. so reconstruct if not in input collections already
    if "xAODTriggerTowers" not in flags.Input.Collections:
        acc.merge(TriggerTowersInputCfg(flags))

    doV6Mapping=False # latome fex input mapping if different between v5 and v6 ... for now only switching to v6 in data
    if not flags.Input.isMC and len(flags.Input.RunNumbers)>0: # in HLT reprocessing jobs, the runNumbers list will be empty ... have to default to v5 for now for these jobs
        from LArConditionsCommon.LArRunFormat import getLArDTInfoForRun
        runinfo = getLArDTInfoForRun(flags.Input.RunNumbers[0], connstring="COOLONL_LAR/CONDBR2")
        doV6Mapping = (runinfo.FWversion()==6)
    
    if doV6Mapping and len(flags.Input.RunNumbers)>0:
        # add required dbOverride (don't do in athena HLT where we will rely on LAr/LATOME to have set it to the right thing)
        from IOVDbSvc.IOVDbSvcConfig import addOverride
        acc.merge( addOverride(flags,folder="/LAR/Identifier/LatomeMapping",tag="LARIdentifierLatomeMapping-fw6") )


    if flags.Trigger.L1.doeFex:
        if 'L1_eFexEmulatedTowers' in eFexTowerInputs and "L1_eFexEmulatedTowers" not in flags.Input.Collections:
            builderAlg = CompFactory.LVL1.eFexTowerBuilder("L1_eFexEmulatedTowers",UseLATOMEv6Mapping=doV6Mapping,
                                                           CaloCellContainerReadKey=sCellType,ApplyMasking=not flags.Input.isMC) # builds the emulated towers to use as secondary input to eTowerMaker - name has to match what it gets called in other places to avoid conflict
        if flags.Input.isMC: builderAlg.LArLatomeHeaderKey=""
        elif doV6Mapping or len(flags.Input.RunNumbers)==0:
            builderAlg.MappingFile='' # need to regenerate mapping on-the-fly for v6 or in athena hlt jobs
            # if regenerating mapping file and this is data, we will need the LATOME headers, otherwise don't use them
            from LArByteStream.LArRawSCDataReadingConfig import LArRawSCDataReadingCfg
            acc.merge(LArRawSCDataReadingCfg(flags))

        acc.addEventAlgo( builderAlg )

        if eFexTowerInputs==[]:
            # no input specified, so use the old eTowerMaker
            eFEXInputs = CompFactory.LVL1.eTowerMakerFromSuperCells('eTowerMakerFromSuperCells',
               eSuperCellTowerMapperTool = CompFactory.LVL1.eSuperCellTowerMapper('eSuperCellTowerMapper', SCell=sCellType))
        else:
            # if primary is DataTowers, check that caloInputs are enabled (if data towers not already available). If it isn't then skip this
            if (not flags.Trigger.L1.doCaloInputs) and eFexTowerInputs[0] == "L1_eFexDataTowers" and ("L1_eFexDataTowers" not in flags.Input.Collections):
                if len(eFexTowerInputs)==1:
                    log.fatal("Requested L1_eFexDataTowers but Trigger.L1.doCaloInputs is False, but not secondary collection given")
                    import sys
                    sys.exit(1)
                log.warning("Requested L1_eFexDataTowers but Trigger.L1.doCaloInputs is False, falling back to secondary")
                eFexTowerInputs[0] = eFexTowerInputs[1]
                eFexTowerInputs[1] = ""
            eFEXInputs = CompFactory.LVL1.eTowerMakerFromEfexTowers('eTowerMakerFromEfexTowers')
            eFEXInputs.InputTowers = eFexTowerInputs[0]
            eFEXInputs.SecondaryInputTowers = eFexTowerInputs[1] if len(eFexTowerInputs) > 1 else ""

        eFEX = CompFactory.LVL1.eFEXDriver('eFEXDriver')
        eFEX.eFEXSysSimTool = CompFactory.LVL1.eFEXSysSim('eFEXSysSimTool')
        eFEX.eFEXSysSimTool.eFEXSimTool = CompFactory.LVL1.eFEXSim('eFEXSimTool')
        eFEX.eFEXSysSimTool.eFEXSimTool.eFEXFPGATool = CompFactory.LVL1.eFEXFPGA('eFEXFPGATool')

        # read algoVersions from menu and configure the algo tools
        from TrigConfigSvc.TriggerConfigAccess import getL1MenuAccess
        L1_menu = getL1MenuAccess(flags)

        em_algoVersion = L1_menu.thresholdExtraInfo("eEM").get("algoVersion", 0)
        tau_algoVersion = L1_menu.thresholdExtraInfo("eTAU").get("algoVersion", 0)

        from PathResolver import PathResolver
        bdtConfigJsonPath = PathResolver.FindCalibFile("Run3L1CaloSimulation/L1CaloFEXSim/eTAU/" + ("bdt_config_v17.json" if tau_algoVersion==2 else "bdt_config_v16.json"))

        eFEX.eFEXSysSimTool.eFEXSimTool.eFEXFPGATool.eFEXegAlgoTool = CompFactory.LVL1.eFEXegAlgo('eFEXegAlgoTool',algoVersion=em_algoVersion,dmCorr=deadMaterialCorrections) # only dmCorrections in data for now
        eFEX.eFEXSysSimTool.eFEXSimTool.eFEXFPGATool.eFEXtauAlgoTool = CompFactory.LVL1.eFEXtauAlgo("eFEXtauAlgo") # heuristic algorithm
        eFEX.eFEXSysSimTool.eFEXSimTool.eFEXFPGATool.eFEXtauBDTAlgoTool = CompFactory.LVL1.eFEXtauBDTAlgo("eFEXtauBDTAlgo", BDTJsonConfigPath=bdtConfigJsonPath)
        # To dump supercells as a decorator to the tau TOB, set 
        #     eFEX.eFEXSysSimTool.eFEXSimTool.eFEXFPGATool.eFEXtauAlgoTool.DumpSuperCells = True
        #       and/or
        #     eFEX.eFEXSysSimTool.eFEXSimTool.eFEXFPGATool.eFEXtauBDTAlgoTool.DumpSuperCells = True




        # load noise cuts and dm corrections when running on data
        from IOVDbSvc.IOVDbSvcConfig import addFolders#, addFoldersSplitOnline

        acc.merge(addFolders(flags,"/TRIGGER/L1Calo/V1/Calibration/EfexNoiseCuts","TRIGGER_OFL" if flags.Input.isMC else "TRIGGER_ONL",className="CondAttrListCollection"))
        eFEXInputs.NoiseCutsKey = "/TRIGGER/L1Calo/V1/Calibration/EfexNoiseCuts"
        acc.merge(addFolders(flags,"/TRIGGER/L1Calo/V1/Calibration/EfexEnergyCalib","TRIGGER_OFL" if flags.Input.isMC else "TRIGGER_ONL",className="CondAttrListCollection")) # dmCorr from DB!
        eFEX.eFEXSysSimTool.eFEXSimTool.eFEXFPGATool.eFEXegAlgoTool.DMCorrectionsKey = "/TRIGGER/L1Calo/V1/Calibration/EfexEnergyCalib"

        acc.addEventAlgo(eFEXInputs)
        acc.addEventAlgo(eFEX)

        if simulateAltTau:
            eFEX.eFEXSysSimTool.Key_eFexAltTauOutputContainer="L1_eTauRoIAlt"
            eFEX.eFEXSysSimTool.Key_eFexAltTauxTOBOutputContainer="L1_eTauxRoIAlt"


    if flags.Trigger.L1.dojFex:
        
        if flags.Input.Format is not Format.POOL:
            from L1CaloFEXByteStream.L1CaloFEXByteStreamConfig import jFexInputByteStreamToolCfg
            inputjFexTool = acc.popToolsAndMerge(jFexInputByteStreamToolCfg(flags, 'jFexInputBSDecoderTool'))
            
            maybeMissingRobs = []
            decoderTools = []
            
            for module_id in inputjFexTool.ROBIDs:
                maybeMissingRobs.append(module_id)

            decoderTools += [inputjFexTool]
            decoderAlg = CompFactory.L1TriggerByteStreamDecoderAlg(name="L1TriggerByteStreamDecoder", DecoderTools=[inputjFexTool], MaybeMissingROBs=maybeMissingRobs)
            acc.addEventAlgo(decoderAlg)

        if "L1_jFexEmulatedTowers" not in flags.Input.Collections:
            from L1CaloFEXAlgos.FexEmulatedTowersConfig import jFexEmulatedTowersCfg
            acc.merge(jFexEmulatedTowersCfg(flags))
        
        from L1CaloFEXCond.L1CaloFEXCondConfig import jFexDBConfig
        acc.merge(jFexDBConfig(flags))      
        
        jFEXInputs = CompFactory.LVL1.jTowerMakerFromJfexTowers('jTowerMakerFromJfexTowers')
        jFEXInputs.IsMC = flags.Input.isMC
        jFEXInputs.jSuperCellTowerMapperTool = CompFactory.LVL1.jSuperCellTowerMapper('jSuperCellTowerMapper', SCell=sCellType)
        jFEXInputs.jSuperCellTowerMapperTool.SCellMasking = not flags.Input.isMC
        # need to set an IsMC property on a tool deep inside the toolstack:
        jFEX = CompFactory.LVL1.jFEXDriver('jFEXDriver',jFEXSysSimTool=CompFactory.LVL1.jFEXSysSim(
                                            'jFEXSysSimTool',jFEXSimTool=CompFactory.LVL1.jFEXSim(
                                              'LVL1::jFEXSim',jFEXFPGATool=CompFactory.LVL1.jFEXFPGA(
                                                'LVL1::jFEXFPGA',jFEXLargeRJetAlgoTool="", # disables jLJ algorithm - will produce empty container
                                                IjFEXFormTOBsTool=CompFactory.LVL1.jFEXFormTOBs(
                                                 'LVL1::jFEXFormTOBs',IsMC=flags.Input.isMC)))))
        acc.addEventAlgo(jFEXInputs)
        acc.addEventAlgo(jFEX)


    if flags.Trigger.L1.dogFex:

        if flags.Input.Format is not Format.POOL:
            from L1CaloFEXByteStream.L1CaloFEXByteStreamConfig import gFexInputByteStreamToolCfg
            inputgFexTool = acc.popToolsAndMerge(gFexInputByteStreamToolCfg(flags, 'gFexInputBSDecoderTool'))
            
            maybeMissingRobs = []
            decoderTools = []

            for module_id in inputgFexTool.ROBIDs:
                maybeMissingRobs.append(module_id)

            decoderTools += [inputgFexTool]
            decoderAlg = CompFactory.L1TriggerByteStreamDecoderAlg(name="L1TriggerByteStreamDecoder", DecoderTools=[inputgFexTool], MaybeMissingROBs=maybeMissingRobs)
            acc.addEventAlgo(decoderAlg)

        # only create emulated towers if not a POOL file or not EmulatedTowers collection
        if not Format.POOL or ("L1_gFexEmulatedTowers" not in flags.Input.Collections and sCellType in flags.Input.Collections):
            from L1CaloFEXAlgos.FexEmulatedTowersConfig import gFexEmulatedTowersCfg
            acc.merge(gFexEmulatedTowersCfg(flags,name="L1_gFexEmulatedTowers"))

        gFEXTowerSummer = CompFactory.LVL1.gFexTowerSummer('gFexTowerSummer')
        gFEXTowerSummer.gFexDataTowers = "L1_gFexEmulatedTowers" if flags.Input.isMC else "L1_gFexDataTowers"
        gFEXTowerSummer.gTowers200WriteKey = "L1_gFexEmulatedTowers200" if flags.Input.isMC else "L1_gFexDataTowers200"
        gFEXTowerSummer.gTowers50WriteKey = "L1_gFexEmulatedTowers50" if flags.Input.isMC else "L1_gFexDataTowers50"
        gFEXTowerSummer.gTowersEMWriteKey = ""
        gFEXTowerSummer.gTowersHADWriteKey = ""
        acc.addEventAlgo(gFEXTowerSummer)

        gFEXInputs = CompFactory.LVL1.gTowerMakerFromGfexTowers('gTowerMakerFromGfexTowers')
        gFEXInputs.InputDataTowers =  "L1_gFexEmulatedTowers200" if flags.Input.isMC else "L1_gFexDataTowers200"
        gFEXInputs.MyGTowers = "gTowerContainer"

        gFEXInputs50 = CompFactory.LVL1.gTowerMakerFromGfexTowers('gTowerMakerFromGfexTowers50')
        gFEXInputs50.InputDataTowers = "L1_gFexEmulatedTowers50" if flags.Input.isMC else "L1_gFexDataTowers50"
        gFEXInputs50.MyGTowers = "gTower50Container"

        from L1CaloFEXCond.L1CaloFEXCondConfig import gFexDBConfig
        acc.merge(gFexDBConfig(flags))

        gFEX = CompFactory.LVL1.gFEXDriver('gFEXDriver')    
        gFEX.gFEXSysSimTool = CompFactory.LVL1.gFEXSysSim('gFEXSysSimTool')
        acc.addEventAlgo(gFEXInputs)
        acc.addEventAlgo(gFEXInputs50)
        acc.addEventAlgo(gFEX)

    if flags.Trigger.doHLT:
        # This will be the case when the offline simulation is actually being run as part of MC
        # as opposed to running another pass of the simulation on either an MC or data file (e.g in DAOD)
        # Check the RoI EDM containers are registered in HLT outputs
        from TrigEDMConfig.TriggerEDM import recordable
        def check(key):
            assert key==recordable(key), f'recordable() check failed for {key}'
        if flags.Trigger.L1.doeFex:
            check(eFEX.eFEXSysSimTool.Key_eFexEMOutputContainer)
            check(eFEX.eFEXSysSimTool.Key_eFexTauOutputContainer)
            if (simulateAltTau):
                check(eFEX.eFEXSysSimTool.Key_eFexAltTauOutputContainer)
        if flags.Trigger.L1.dojFex:
            check(jFEX.jFEXSysSimTool.Key_jFexSRJetOutputContainer)
            check(jFEX.jFEXSysSimTool.Key_jFexLRJetOutputContainer)
            check(jFEX.jFEXSysSimTool.Key_jFexTauOutputContainer)
            check(jFEX.jFEXSysSimTool.Key_jFexSumETOutputContainer)
            check(jFEX.jFEXSysSimTool.Key_jFexMETOutputContainer)
            check(jFEX.jFEXSysSimTool.Key_jFexFwdElOutputContainer)
        if flags.Trigger.L1.dogFex:
            check(gFEX.gFEXSysSimTool.Key_gFexSRJetOutputContainer)
            check(gFEX.gFEXSysSimTool.Key_gFexLRJetOutputContainer)
            check(gFEX.gFEXSysSimTool.Key_gFexRhoOutputContainer)
            check(gFEX.gFEXSysSimTool.Key_gScalarEJwojOutputContainer)
            check(gFEX.gFEXSysSimTool.Key_gMETComponentsJwojOutputContainer)
            check(gFEX.gFEXSysSimTool.Key_gMHTComponentsJwojOutputContainer)
            check(gFEX.gFEXSysSimTool.Key_gMSTComponentsJwojOutputContainer)
            check(gFEX.gFEXSysSimTool.Key_gMETComponentsNoiseCutOutputContainer)
            check(gFEX.gFEXSysSimTool.Key_gMETComponentsRmsOutputContainer)
            check(gFEX.gFEXSysSimTool.Key_gScalarENoiseCutOutputContainer)
            check(gFEX.gFEXSysSimTool.Key_gScalarERmsOutputContainer)
    else:
        # Rename outputs for monitoring resimulation to avoid clash with standard SG keys
        def getSimHandle(key):
            """
            Add 'Sim' to the standard handle path and include user-specified suffix
            """
            key += outputSuffix
            if not key.endswith("Sim"): key += "Sim"
            return key

        if flags.Trigger.L1.doeFex:
            eFEX.eFEXSysSimTool.Key_eFexEMOutputContainer=getSimHandle("L1_eEMRoI")
            eFEX.eFEXSysSimTool.Key_eFexTauOutputContainer=getSimHandle("L1_eTauRoI")
            eFEX.eFEXSysSimTool.Key_eFexEMxTOBOutputContainer=getSimHandle("L1_eEMxRoI")
            eFEX.eFEXSysSimTool.Key_eFexTauxTOBOutputContainer=getSimHandle("L1_eTauxRoI")
            if simulateAltTau:
                eFEX.eFEXSysSimTool.Key_eFexAltTauOutputContainer=getSimHandle("L1_eTauRoIAlt")
                eFEX.eFEXSysSimTool.Key_eFexAltTauxTOBOutputContainer=getSimHandle("L1_eTauxRoIAlt")

        if flags.Trigger.L1.dojFex:
            jFEX.jFEXSysSimTool.Key_jFexSRJetOutputContainer=getSimHandle("L1_jFexSRJetRoI")
            jFEX.jFEXSysSimTool.Key_jFexLRJetOutputContainer=getSimHandle("L1_jFexLRJetRoI")
            jFEX.jFEXSysSimTool.Key_jFexTauOutputContainer=getSimHandle("L1_jFexTauRoI")
            jFEX.jFEXSysSimTool.Key_jFexSumETOutputContainer=getSimHandle("L1_jFexSumETRoI")
            jFEX.jFEXSysSimTool.Key_jFexMETOutputContainer=getSimHandle("L1_jFexMETRoI")
            jFEX.jFEXSysSimTool.Key_jFexFwdElOutputContainer=getSimHandle("L1_jFexFwdElRoI")
            jFEX.jFEXSysSimTool.Key_xTobOutKey_jJ=getSimHandle("L1_jFexSRJetxRoI")
            jFEX.jFEXSysSimTool.Key_xTobOutKey_jLJ=getSimHandle("L1_jFexLRJetxRoI")
            jFEX.jFEXSysSimTool.Key_xTobOutKey_jTau=getSimHandle("L1_jFexTauxRoI")
            jFEX.jFEXSysSimTool.Key_xTobOutKey_jEM=getSimHandle("L1_jFexFwdElxRoI")
        if flags.Trigger.L1.dogFex:
            gFEX.gFEXSysSimTool.Key_gFexSRJetOutputContainer=getSimHandle("L1_gFexSRJetRoI")
            gFEX.gFEXSysSimTool.Key_gFexLRJetOutputContainer=getSimHandle("L1_gFexLRJetRoI")
            gFEX.gFEXSysSimTool.Key_gFexRhoOutputContainer=getSimHandle("L1_gFexRhoRoI")
            gFEX.gFEXSysSimTool.Key_gScalarEJwojOutputContainer=getSimHandle("L1_gScalarEJwoj")
            gFEX.gFEXSysSimTool.Key_gMETComponentsJwojOutputContainer=getSimHandle("L1_gMETComponentsJwoj")
            gFEX.gFEXSysSimTool.Key_gMHTComponentsJwojOutputContainer=getSimHandle("L1_gMHTComponentsJwoj")
            gFEX.gFEXSysSimTool.Key_gMSTComponentsJwojOutputContainer=getSimHandle("L1_gMSTComponentsJwoj")
            gFEX.gFEXSysSimTool.Key_gMETComponentsNoiseCutOutputContainer=getSimHandle("L1_gMETComponentsNoiseCut")
            gFEX.gFEXSysSimTool.Key_gMETComponentsRmsOutputContainer=getSimHandle("L1_gMETComponentsRms")
            gFEX.gFEXSysSimTool.Key_gScalarENoiseCutOutputContainer=getSimHandle("L1_gScalarENoiseCut")
            gFEX.gFEXSysSimTool.Key_gScalarERmsOutputContainer=getSimHandle("L1_gScalarERms")

    return acc


if __name__ == '__main__':
    ##################################################
    # Add an argument parser
    ##################################################
    import argparse
    p = argparse.ArgumentParser()
    p.add_argument('-i', '--input',
                   metavar='KEY',
                   default='ttbar',
                   help='Key of the input from TrigValInputs to be used, default=%(default)s')
    p.add_argument('-e', '--execute',
                   action='store_true',
                   help='After building the configuration, also process a few events')
    p.add_argument('-n', '--nevents',
                   metavar='N',
                   type=int,
                   default=25,
                   help='Number of events to process if --execute is used, default=%(default)s')
    p.add_argument('-d', '--efexdebug',
                   action='store_true',
                   help='Activate DEBUG mode for eFEX driver .. this option is required by a unit test')

    args = p.parse_args()

    ##################################################
    # Configure all the flags
    ##################################################
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from TrigValTools.TrigValSteering import Input
    import os

    flags = initConfigFlags()
    flags.Common.isOnline = True
    flags.Input.Files = [args.input] if os.path.isfile(args.input) else Input.get_input(args.input).paths
    if not flags.Input.isMC:
        from AthenaConfiguration.TestDefaults import defaultGeometryTags
        flags.GeoModel.AtlasVersion = defaultGeometryTags.autoconfigure(flags)
        from AthenaConfiguration.Enums import LHCPeriod
        flags.IOVDb.GlobalTag = 'CONDBR2-HLTP-2023-01' if flags.GeoModel.Run is LHCPeriod.Run3 else 'CONDBR2-HLTP-2018-04'
    else:
        from AthenaConfiguration.TestDefaults import defaultConditionsTags
        flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_MC
    flags.Output.AODFileName = 'AOD.pool.root'
    flags.Exec.MaxEvents = args.nevents
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1
    flags.Scheduler.ShowDataDeps = True
    flags.Scheduler.CheckDependencies = True
    flags.Scheduler.ShowDataFlow = True
    flags.Trigger.EDMVersion = 3
    flags.Trigger.doLVL1 = True
    flags.Trigger.enableL1CaloPhase1 = True
    flags.Trigger.triggerConfig = 'FILE'

    # Enable only calo for this test
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(flags, ['LAr','Tile','MBTS'], toggle_geometry=True)

    flags.lock()

    ##################################################
    # Set up central services: Main + Input reading + L1Menu + Output writing
    ##################################################
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from AthenaConfiguration.Enums import Format
    if flags.Input.Format == Format.POOL:
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        acc.merge(PoolReadCfg(flags))
    else:
        from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg
        acc.merge(ByteStreamReadCfg(flags))

    from TrigConfigSvc.TrigConfigSvcCfg import L1ConfigSvcCfg, generateL1Menu, createL1PrescalesFileFromMenu
    generateL1Menu(flags)
    createL1PrescalesFileFromMenu(flags)
    acc.merge(L1ConfigSvcCfg(flags))

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    FexEDMList = [
        'xAOD::eFexEMRoIContainer#L1_eEMRoI','xAOD::eFexEMRoIAuxContainer#L1_eEMRoIAux.',
        'xAOD::eFexTauRoIContainer#L1_eTauRoI','xAOD::eFexTauRoIAuxContainer#L1_eTauRoIAux.',
        'xAOD::jFexTauRoIContainer#L1_jFexTauRoI','xAOD::jFexTauRoIAuxContainer#L1_jFexTauRoIAux.',
        'xAOD::jFexSRJetRoIContainer#L1_jFexSRJetRoI','xAOD::jFexSRJetRoIAuxContainer#L1_jFexSRJetRoIAux.',
        'xAOD::jFexLRJetRoIContainer#L1_jFexLRJetRoI','xAOD::jFexLRJetRoIAuxContainer#L1_jFexLRJetRoIAux.',
        'xAOD::jFexMETRoIContainer#L1_jFexMETRoI','xAOD::jFexMETRoIAuxContainer#L1_jFexMETRoIAux.',
        'xAOD::jFexSumETRoIContainer#L1_jFexSumETRoI','xAOD::jFexSumETRoIAuxContainer#L1_jFexSumETRoIAux.',
        'xAOD::gFexJetRoIContainer#L1_gFexSRJetRoI','xAOD::gFexJetRoIAuxContainer#L1_gFexSRJetRoIAux.',
        'xAOD::gFexJetRoIContainer#L1_gFexLRJetRoI','xAOD::gFexJetRoIAuxContainer#L1_gFexLRJetRoIAux.',
        'xAOD::gFexJetRoIContainer#L1_gFexRhoRoI','xAOD::gFexJetRoIAuxContainer#L1_gFexRhoRoIAux.',
        'xAOD::gFexGlobalRoIContainer#L1_gScalarEJwoj','xAOD::gFexGlobalRoIAuxContainer#L1_gScalarEJwojAux.',
        'xAOD::gFexGlobalRoIContainer#L1_gMETComponentsJwoj','xAOD::gFexGlobalRoIAuxContainer#L1_gMETComponentsJwojAux.',
        'xAOD::gFexGlobalRoIContainer#L1_gMHTComponentsJwoj','xAOD::gFexGlobalRoIAuxContainer#L1_gMHTComponentsJwojAux.',
        'xAOD::gFexGlobalRoIContainer#L1_gMSTComponentsJwoj','xAOD::gFexGlobalRoIAuxContainer#L1_gMSTComponentsJwojAux.',
        'xAOD::gFexGlobalRoIContainer#L1_gMETComponentsNoiseCut','xAOD::gFexGlobalRoIAuxContainer#L1_gMETComponentsNoiseCutAux.',
        'xAOD::gFexGlobalRoIContainer#L1_gMETComponentsRms','xAOD::gFexGlobalRoIAuxContainer#L1_gMETComponentsRmsAux.',
        'xAOD::gFexGlobalRoIContainer#L1_gScalarENoiseCut','xAOD::gFexGlobalRoIAuxContainer#L1_gScalarENoiseCutAux.',
        'xAOD::gFexGlobalRoIContainer#L1_gScalarERms','xAOD::gFexGlobalRoIAuxContainer#L1_gScalarERmsAux.',

    ]
    acc.merge(OutputStreamCfg(flags, 'AOD', ItemList=FexEDMList))

    ##################################################
    # The configuration fragment to be tested
    ##################################################
    acc.merge(L1CaloFEXSimCfg(flags))
    if args.efexdebug:
        from AthenaCommon.Constants import DEBUG
        acc.getEventAlgo("eFEXDriver").OutputLevel = DEBUG

    ##################################################
    # Save and optionally run the configuration
    ##################################################
    with open("L1Sim.pkl", "wb") as f:
        acc.store(f)
        f.close()

    if args.execute:
        sc = acc.run()
        if sc.isFailure():
            exit(1)
