# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from JetToolHelpers.HelperConfig import VarToolCfg, HistoInputCfg
from AthenaConfiguration.ComponentFactory import CompFactory
from PathResolver import PathResolver

def update_nested_dict(originalDict, inputDict):
    for key, value in inputDict.items():
        if key not in originalDict:
            originalDict[key] = value
        elif isinstance(value, dict):
            if isinstance(originalDict[key], dict):
                update_nested_dict(originalDict[key],value)
            else: 
                raise TypeError(f'Key {key} has different types in original and input dictionaries')
        else:
            originalDict[key] = value
    return originalDict


def smearingStep(flags, **configDict):
    """ Configuration of the Smearing step. """

    # TODO: Replace this defaultDict with a default YAML file
    defaultDict = dict(
        HistoReaderMC = dict(
            histName = "JER_Nominal_MC16_AntiKt4EMTopo",
            inputFile = PathResolver.FindCalibFile("JetCalibTools/CalibArea-00-04-82/CalibrationFactors/JER_Nominal_Apr2019.root"),
            InterpType = "OnlyX",
            varX = dict(Name = 'pt', Scale = 1e-3, Type = "float", isJetVar = True),
        ),
        HistoReaderData = dict(
            histName = "JER_Nominal_data_AntiKt4EMTopo",
            inputFile = PathResolver.FindCalibFile("JetCalibTools/CalibArea-00-04-82/CalibrationFactors/JER_Nominal_Apr2019.root"),
            InterpType = "OnlyX",
            varX = dict(Name = 'pt', Scale = 1e-3, Type = "float", isJetVar = True),
        ),
        JSCStartingScale = "JetGSCScaleMomentum",
        JSCOutScale = "JetSmearedMomentum",
        SmearType = "FourVec",
    )

    # HistoReaderMC and HistoReaderData can be specified simultaneously using a single "HistoReader" YAML block.
    # This should contain the common configuration for both and histNameMC/histNameData entries for the histogram names.
    # This cannot be used simultaneously with a HistoReaderMC or HistoReaderData block
    if "HistoReader" in configDict:
        if "HistoReaderMC" in configDict:
            raise ValueError("Invalid YAML - both HistoReader and HistoReaderMC included")
        if "HistoReaderData" in configDict:
            raise ValueError("Invalid YAML - both HistoReader and HistoReaderData included")
        
        # Build the HistoReaderMC and HistoReaderData blocks from the HistoReader block
        histoReader = configDict.pop("HistoReader")
        histNameMC = histoReader.pop("histNameMC")
        histNameData = histoReader.pop("histNameData")
        configDict["HistoReaderMC"] = dict(histoReader)
        configDict["HistoReaderData"] = dict(histoReader)
        configDict["HistoReaderMC"]["histName"] = histNameMC
        configDict["HistoReaderData"]["histName"] = histNameData

    defaultDict = update_nested_dict(defaultDict, configDict)

    histToolMC = HistoInputCfg(flags, "HistToolMC", **defaultDict["HistoReaderMC"])
    histToolData = HistoInputCfg(flags, "HistToolData", **defaultDict["HistoReaderData"])
    defaultDict["HistoReaderMC"] = histToolMC
    defaultDict["HistoReaderData"] = histToolData

    smearStep = CompFactory.SmearingCalibStep("SmearingCalibStep", **defaultDict)

    return smearStep

def puresidualStep(flags, **configDict):

    # ----------------
    # defining default values from AntiKt4EMPFlow_MC23a_PreRecR22_Phase2_CalibConfig_ResPU_EtaJES_GSC_240306_InSitu.config
    # this is temporary so that the function can be invoked without argument during testing phase.
    #  in production we'll expect all config is really passed through the configDic argument
    defaultDic = dict(
        CorrectionDesc = "MC20 residual mu and NPV pileup corrections for R=0.4 AntiKt4EMPFlowJets. October 22.",
        RhoKey= "Kt4EMPFlowNeutEventShape",
        IsData = False,
        DoJetArea=True,
        DefaultMuRef=0,
        DefaultNPVRef=1,
        ApplyNPVBeamspotCorrection=False,                                          
        AbsEtaBins = [0.000, 0.100, 0.200, 0.300, 0.400, 0.500, 0.600, 0.700, 0.800, 0.900, 1.000, 1.100, 1.200, 1.300, 1.400, 1.500, 1.600, 1.700, 1.800, 1.900, 2.000, 2.100, 2.200, 2.300, 2.400, 2.500, 2.600, 2.700, 2.800, 2.900, 3.000, 3.100, 3.200, 3.300, 3.400, 3.500, 3.600, 3.800, 4.000, 4.200, 4.500, 4.900],
        
        MuTerm =[-0.008, -0.059, 0.046, -0.095, 0.096, -0.048, -0.015, -0.016, 0.040, -0.097, 0.065, -0.120, 0.058, -0.110, 0.084, 0.023, -0.106, -0.041, -0.123, 0.096, -0.353, 0.290, -0.352, 0.344, -0.558, 0.185, -0.508, -0.234, -0.234, -0.034, -0.089, 0.293, 0.200, 0.147, -0.070, -0.180, -0.269, -0.147, -0.128, -0.085, 0.076, 0.304],
    
        NPVTerm=[0.024, 0.233, -0.196, 0.285, -0.250, 0.126, -0.029, 0.023, -0.144, 0.205, 0.002, 0.113, 0.080, 0.004, -0.060, 0.026, 0.013, 0.041, 0.165, -0.051, 0.445, -0.343, 0.373, -0.503, 0.810, -0.042, 0.741, 0.405, 0.405, 0.266, -0.084, -0.490, -0.471, -0.340, 0.036, 0.324, 0.443, 0.235, 0.207, 0.163, -0.159, -0.616],
                                                 
    )

    defaultDic.update(configDict) # overwrite the default in case something was passed
    
    return CompFactory.Pileup1DResidualCalibStep("PUResid", **defaultDic)


def gscStep(flags, **configDict):

    # Example of old calibration:
    # PTResponseRequirementOff: true # Response>1 requirement
    # nTrkwTrk4PFlow: true
    # GSCFactorsFile: JetCalibTools/CalibrationFactors/AntiKt4EMPFlow_MC23a_PreRecR22_Phase2_GSC_calibrationFactors_240306.root

    # # Order of GSC correction is chargedFraction->Tile0->EM3->nTrk->trackWIDTH->PunchThrough
    # # Use the GSCDepth flag to control the last correction applied
    # Acceptable values for the GSC Depth flag are: "chargedFraction" (only for PFlow), "Tile0", "EM3", "nTrk", "trackWIDTH", "PunchThrough", or "Full" (equivalent to "PunchThrough")
    # GSCDepth: trackWIDTH
    # PunchThroughEtaBins: 0.0 1.3 1.9

    defaultFileGSC = configDict.pop('fileGSC',PathResolver.FindCalibFile('JetCalibTools/CalibArea-00-04-83/CalibrationFactors/AntiKt4EMPFlow_MC23a_PreRecR22_Phase2_GSC_calibrationFactors_240306.root'))

    defaultDict = dict(
        vartool1 = dict(Name="pt", Scale=1e-3, isJetVar=True, Type="float"),
        vartool2 = dict(Name="EM3", Scale=1, isJetVar=False, Type="float"),
        # Check this default is right, is histTool not the same as histTool_EM3[0] but with y=abseta instead of EM3? Is that right?
        histTool = dict(varX = "pt", varY = "abseta", histName="AntiKt4EMPFlow_EM3_interpolation_resp_eta_0", inputFile=defaultFileGSC),
        histTool_EM3 = [dict(varX = "pt", varY = dict(Name="EM3", isJetVar=False), histName=f"AntiKt4EMPFlow_EM3_interpolation_resp_eta_{j}", inputFile=defaultFileGSC) for j in range(35)],
        histTool_CharFrac = [dict(varX = dict(Name="pt"), varY = dict(Name="ChargedFraction", isJetVar=False), histName = f"AntiKt4EMPFlow_chargedFraction_interpolation_resp_eta_{j}", inputFile=defaultFileGSC) for j in range(25)],
        histTool_Tile0 = [dict(varX = "pt", varY = dict(Name="Tile0", isJetVar=False), histName=f"AntiKt4EMPFlow_Tile0_interpolation_resp_eta_{j}", inputFile=defaultFileGSC) for j in range(18)],
        histTool_nTrk=[dict(varX = "pt", varY = dict(Name="nTrk", isJetVar=False, Type="int",), histName=f"AntiKt4EMPFlow_nTrk_interpolation_resp_eta_{j}", inputFile=defaultFileGSC) for j in range(25)],
        histTool_trackWIDTH=[dict(varX = "pt", varY = dict(Name="trackWIDTH", isJetVar=False), histName=f"AntiKt4EMPFlow_trackWIDTH_interpolation_resp_eta_{j}", inputFile=defaultFileGSC) for j in range(25)],
    )

    # Build the config dictionary
    for key in configDict:
        if key in ['vartool1','vartool2']:
            defaultDict[key] = key
        elif key=='histTool':
            defaultDict[key].update(configDict[key])
        elif key not in ['histTool_EM3', 'histTool_CharFrac', 'histTool_Tile0', 'histTool_nTrk', 'histTool_trackWIDTH']:
            raise ValueError(f'Key {key} not recognised in GSC block.')
        elif isinstance(configDict[key],list):
            # in this case the full list of sub-tools has been specified in the YAML
            # and the defaultDict will be fully overwritten (ie. all parameters must be specified)
            for subDict in configDict[key]:
                subDict.setdefault('inputFile',defaultFileGSC)
            defaultDict[key] = configDict[key]
        else:
            # Functionality to build the arrays of histogram reader from a single block in the YAML file:
            baseDict = dict(configDict[key])
            N_hist = baseDict.pop('N_hist')
            histNameBase = baseDict.pop('histNameBase')
            inputFile = baseDict.pop('inputFile', defaultFileGSC)
            varX = baseDict.pop('varX',defaultDict[key][0]['varX'])
            varY = baseDict.pop('varY', defaultDict[key][0]['varY'])
            toolArray = [dict(varX = varX, varY = varY, histName=f'{histNameBase}_{j}', inputFile = inputFile) for j in range(N_hist)]
            defaultDict[key] = toolArray

    var1tool = VarToolCfg(flags, defaultDict["vartool1"], Tname="VarTool")
    var2tool = VarToolCfg(flags, defaultDict["vartool2"], Tname="VarTool")
    defaultDict["vartool1"] = var1tool
    defaultDict["vartool2"] = var2tool

    histTool = HistoInputCfg(flags, Tname="EM3_0", **defaultDict["histTool"])
    defaultDict["histTool"] = histTool

    histEM3Array = [HistoInputCfg(flags, Tname=f"EM3_{j}", **histEM3Config) for j, histEM3Config in enumerate(defaultDict["histTool_EM3"])]
    defaultDict["histTool_EM3"] = histEM3Array

    histCharFracArray = [HistoInputCfg(flags, Tname=f"ChargedFraction_{j}", **histCharConfig) for j, histCharConfig in enumerate(defaultDict["histTool_CharFrac"])]
    defaultDict["histTool_CharFrac"] = histCharFracArray

    histTile0Array = [HistoInputCfg(flags, Tname=f"Tile0_{j}", **histTile0Config) for j, histTile0Config in enumerate(defaultDict["histTool_Tile0"])]
    defaultDict["histTool_Tile0"] = histTile0Array

    histnTrkArray = [HistoInputCfg(flags, Tname=f"nTrk_{j}", **histTrkConfig) for j, histTrkConfig in enumerate(defaultDict["histTool_nTrk"])] 
    defaultDict["histTool_nTrk"] = histnTrkArray

    histTrackWIDTHArray = [HistoInputCfg(flags, Tname=f"trackWIDTH_{j}", **histTrackWIDTHConfig) for j, histTrackWIDTHConfig in enumerate(defaultDict["histTool_trackWIDTH"])]
    defaultDict["histTool_trackWIDTH"] = histTrackWIDTHArray
    
    GSCstep = CompFactory.GSCCalibStep("gsccalibstep", **defaultDict)

    return GSCstep

def etajesStep(flags, **configDic):

    pVars = configDic.pop("ParametrizedVars")

    jesstep = CompFactory.EtaMassJESCalibStep("EtaMassJESCalib",
                                              VarToolE= VarToolCfg(flags,  var=pVars['varE']),
                                              VarToolEta= VarToolCfg(flags, var=pVars["varEta"]),
                                              **configDic
                                              )
    return jesstep

def insituStep(flags, **configDic):
    defaultFileInsitu = configDic.pop('fileInsitu',[PathResolver.FindCalibFile('/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/JetCalibTools/CalibArea-00-04-82/InsituCalibration/InsituCalibration_80ifb_1516_Nov_2018_4PF_Consolidated.root'),PathResolver.FindCalibFile('/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/JetCalibTools/CalibArea-00-04-82/InsituCalibration/InsituCalibration_80ifb_17_Nov_2018_4PF_Consolidated.root')])
    defaultHistoReaderEtaInterDict_vec, defaultHistoReaderAbsDict_vec = [], []
    for infile in defaultFileInsitu:
        d = {}
        d['inputFile'] = infile
        for key in configDic['histEtaInterCalib']:
            d[key] = configDic['histEtaInterCalib'][key]
        defaultHistoReaderEtaInterDict_vec.append(d)
    for infile in defaultFileInsitu:
        dd = {}
        dd['inputFile'] = infile
        for key in configDic['histAbsCalib']:
            dd[key] = configDic['histAbsCalib'][key]
        defaultHistoReaderAbsDict_vec.append(dd)
    defaultDict = dict(
        vartool1 = VarToolCfg(flags, var=configDic.get('histEtaInterCalib',{}).get('varX',{}).get('Name','pt'), Tname="VarTool", Scale=configDic.get('histEtaInterCalib',{}).get('varX',{}).get('Scale',1.0)),
        vartool2 = VarToolCfg(flags, var=configDic.get('histEtaInterCalib',{}).get('varY',{}).get('Name','eta'), Tname="VarTool", Scale=configDic.get('histEtaInterCalib',{}).get('varY',{}).get('Scale',1.0)),
        RunNumbers = configDic.pop('InsituRunBins',[251102, 314199, 999999]),
        HistoReaderEtaInter = [HistoInputCfg(flags, "HistToolEtaInter"+str(j), **defaultHistoReaderEtaInterDict_vec[j]) for j in range(len(defaultHistoReaderEtaInterDict_vec))],
        HistoReaderAbs = [HistoInputCfg(flags, "HistToolAbs"+str(j), **defaultHistoReaderAbsDict_vec[j]) for j in range(len(defaultHistoReaderAbsDict_vec))],
        CalibrateMC = configDic.pop('CalibrateMC', False),
        isMC = True,
        InSituStartingScale = "JetGSCScaleMomentum",
        InSituOutScale = "JetInsituScaleMomentum"
    )
    insituStep = CompFactory.InSituCalibStep("insitucalibstep", **defaultDict)
    return insituStep

#####################
    
calibStepDic = dict(
    JetArea = None,
    Residual = puresidualStep,
    EtaJES = etajesStep,
    GSC = gscStep,
    InSitu = insituStep,
    Smear = smearingStep,
)


def calibSeqToToolList(flags, calibseq, **configDict):
    """returns a list of instantiated tools for each of the calibration steps encoded in `calibseq`
    tools are instantiated by calling functions declared in the calibStepDic dictionnary.
    """
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()

    steps = calibseq.split("_")

    tools = []
    for s in steps:
        calibFunc = calibStepDic.get(s,None)
        if calibFunc is None:
            continue # during testing phase. It should be an ERROR when in "prod"

        calibConfig = configDict.get(s,{})

        if calibConfig != {}:
            print("!! ",s," passing explicit dict ",calibConfig)
        
        tools += [ calibFunc(flags, **calibConfig) ]

    return tools


def calibToolFromConfigFile(flags, calibSeq, configFile):
    from yaml import safe_load
    configDic = safe_load(open(configFile))

    calibTool = CompFactory.JetCalibTool("jetcalib",CalibSteps=calibSeqToToolList(flags, calibSeq, **configDic))
    return calibTool
