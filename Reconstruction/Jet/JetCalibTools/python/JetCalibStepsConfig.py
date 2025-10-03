# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaCommon import Logging
jcslog = Logging.logging.getLogger('JetCalibStepsConfig')

from JetToolHelpers.HelperConfig import VarToolCfg, HistoInputCfg
from AthenaConfiguration.ComponentFactory import CompFactory
from PathResolver import PathResolver

def smearingStep(flags, **configDict):
    """ Configuration of the Smearing step. """

    # HistoReaderMC and HistoReaderData can be specified simultaneously using a single "HistoReader" YAML block.
    # This should contain the common configuration for both and histNameMC/histNameData entries for the histogram names.
    # This cannot be used simultaneously with a HistoReaderMC or HistoReaderData block
    if "HistoReader" in configDict:
        if "HistoReaderMC" in configDict:
            raise JetCalibConfigError("Both HistoReader and HistoReaderMC blocks included in YAML config")
        if "HistoReaderData" in configDict:
            raise JetCalibConfigError("Both HistoReader and HistoReaderData blocks included in YAML config")
        
        # Build the HistoReaderMC and HistoReaderData blocks from the HistoReader block
        histoReader = configDict.pop("HistoReader")
        histNameMC = histoReader.pop("histNameMC")
        histNameData = histoReader.pop("histNameData")
        configDict["HistoReaderMC"] = dict(histoReader)
        configDict["HistoReaderData"] = dict(histoReader)
        configDict["HistoReaderMC"]["histName"] = histNameMC
        configDict["HistoReaderData"]["histName"] = histNameData

    configDict["HistoReaderMC"]["inputFile"] = PathResolver.FindCalibFile(configDict["HistoReaderMC"]["inputFile"])
    configDict["HistoReaderData"]["inputFile"] = PathResolver.FindCalibFile(configDict["HistoReaderData"]["inputFile"])

    histToolMC = HistoInputCfg(flags, "HistToolMC", **configDict["HistoReaderMC"])
    histToolData = HistoInputCfg(flags, "HistToolData", **configDict["HistoReaderData"])
    configDict["HistoReaderMC"] = histToolMC
    configDict["HistoReaderData"] = histToolData

    smearStep = CompFactory.SmearingCalibStep("SmearingCalibStep", **configDict)

    return smearStep

def puresidualStep(flags, **configDict):

    configDict.setdefault('IsData', not flags.Input.isMC)

    return CompFactory.Pileup1DResidualCalibStep("PUResid", **configDict)


def gscStep(flags, **configDict):

    defaultFileGSC = PathResolver.FindCalibFile(configDict.pop('fileGSC'))

    # These HistoInput2D defaults can't be set in the C++, so are set here:
    defaultHistTools = dict(
        histTool_EM3 = [dict(varX = "pt", varY = "EM3", histName=f"AntiKt4EMPFlow_EM3_interpolation_resp_eta_{j}", inputFile=defaultFileGSC) for j in range(35)],
        histTool_CharFrac = [dict(varX = "pt", varY = "ChargedFraction", histName = f"AntiKt4EMPFlow_chargedFraction_interpolation_resp_eta_{j}", inputFile=defaultFileGSC) for j in range(25)],
        histTool_Tile0 = [dict(varX = "pt", varY = "Tile0", histName=f"AntiKt4EMPFlow_Tile0_interpolation_resp_eta_{j}", inputFile=defaultFileGSC) for j in range(18)],
        histTool_nTrk=[dict(varX = "pt", varY = dict(Name="nTrk", Type="int",), histName=f"AntiKt4EMPFlow_nTrk_interpolation_resp_eta_{j}", inputFile=defaultFileGSC) for j in range(25)],
        histTool_trackWIDTH=[dict(varX = "pt", varY = "trackWIDTH", histName=f"AntiKt4EMPFlow_trackWIDTH_interpolation_resp_eta_{j}", inputFile=defaultFileGSC) for j in range(25)],
    )

    # Build the hist tools
    for key in ['histTool_EM3', 'histTool_CharFrac', 'histTool_Tile0', 'histTool_nTrk', 'histTool_trackWIDTH']:

        # Use defaultHistTools by default
        if key not in configDict:
            toolArray = defaultHistTools[key]

        # In this case the full list of sub-tools has been specified in the YAML
        # and the defaultDict will be fully overwritten (ie. all parameters must be specified)
        elif isinstance(configDict[key],list):
            toolArray = configDict[key]
            for subDict in toolArray:
                subDict.setdefault('inputFile',defaultFileGSC)

        # Functionality to build the arrays of histogram reader from a shorter block in the YAML file
        # Fall back on defaultHistTools for defaults
        else:
            baseDict = dict(configDict[key])
            N_hist = baseDict.pop('N_hist')
            histNameBase = baseDict.pop('histNameBase')
            inputFile = baseDict.pop('inputFile', defaultFileGSC)
            varX = baseDict.pop('varX',defaultHistTools[key][0]['varX'])
            varY = baseDict.pop('varY', defaultHistTools[key][0]['varY'])
            toolArray = [dict(varX = varX, varY = varY, histName=f'{histNameBase}_{j}', inputFile = inputFile) for j in range(N_hist)]

        # Convert array of properties to HistoInput tools
        configDict[key] = [HistoInputCfg(flags, Tname=f"{key.split('_')[1]}_{j}", **toolConfig) for j, toolConfig in enumerate(toolArray)]

    GSCstep = CompFactory.GSCCalibStep("gsccalibstep", **configDict)

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

    histEtaInterCalib = configDic.pop('histEtaInterCalib')
    histAbsCalib = configDic.pop('histAbsCalib')

    histoReaderEta_vec, histoReaderAbs_vec = [], []

    for infile in configDic.pop('fileInsitu'):
        histoReaderEta_vec.append(dict(inputFile = PathResolver.FindCalibFile(infile), **histEtaInterCalib))
        histoReaderAbs_vec.append(dict(inputFile = PathResolver.FindCalibFile(infile), **histAbsCalib))
    configDic['HistoReaderEtaInter'] = [HistoInputCfg(flags, "HistToolEtaInter"+str(j), **etaDic) for j, etaDic in enumerate(histoReaderEta_vec)]
    configDic['HistoReaderAbs'] = [HistoInputCfg(flags, "HistToolAbs"+str(j), **absDic) for j, absDic in enumerate(histoReaderAbs_vec)]

    configDic['vartool1'] = VarToolCfg(flags, var=histEtaInterCalib['varX'], Tname="VarTool")
    configDic['vartool2'] = VarToolCfg(flags, var=histEtaInterCalib['varY'], Tname="VarTool")
    configDic['isMC'] = flags.Input.isMC

    insituStep = CompFactory.InSituCalibStep("insitucalibstep", **configDic)

    return insituStep

#####################
    
calibStepDic = dict(
    JetArea = None,
    Residual = puresidualStep,
    EtaJES = etajesStep,
    GSC = gscStep,
    Insitu = insituStep,
    Smear = smearingStep,
)

def calibConfigToToolList(flags, **configDict):
    """
    Returns a list of instantiated tools for each of the calibration steps. 
    The order of the steps is determined by the InScale and OutScale properties given in the config.
    Tools are instantiated by calling functions declared in the calibStepDic dictionary.
    """

    toolDic = {}
    foundCS = False # check at least one of the steps starts from constituent scale
    for step in configDict:

        configDict.get(step).pop('prereqs',{}) # removes the 'prereqs' entry not refined in steps       
        # expert option to skip a step
        if configDict.get(step).pop('noRun',False):
            jcslog.warning(f'Expert option: Skipping calib step {step}')
            continue

        # Skip Insitu for MC
        if step=="Insitu" and flags.Input.isMC and not configDict.get("Insitu").get("CalibrateMC",False):
            jcslog.info('Skipping Insitu for MC')
            continue

        calibFunc = calibStepDic.get(step,None)
        if calibFunc is None:
            raise NotImplementedError(f'Calibration step {step} is not found in calibStepDic')

        calibConfig = configDict.get(step)

        tool = calibFunc(flags, **calibConfig)

        toolDic[step] = tool
        if tool.InScale == "JetConstitScaleMomentum":
            foundCS = True
    
    if not foundCS:
        raise JetCalibConfigError('At least one step must have InScale = JetConstitScaleMomentum')

    ordered_tools = []
    ordered_step_names = []

    def findNextSteps(startScale = "JetConstitScaleMomentum"):
        ''' Recursively add tools to ordered_tools based on in/out scale '''
        for step in toolDic:
            if toolDic[step].InScale == startScale:
                ordered_tools.append(toolDic[step])
                ordered_step_names.append(step)
                findNextSteps(toolDic[step].OutScale)

    findNextSteps()

    # Check we've got all the steps
    for step in toolDic:
        if step not in ordered_step_names:
            raise JetCalibConfigError(f'Could not place calib step {step} - have you set InScale and OutScale correctly?')

    jcslog.info(f'Ordered jet calib steps: {"->".join(ordered_step_names)}')

    return ordered_tools

def calibToolFromConfigFile(flags, configFile, name = "jetcalib"):

    configDic = load_yaml_cfg(configFile)

    globalSettings = configDic.pop('Global',{})

    calibTool = CompFactory.JetCalibTool(name, CalibSteps=calibConfigToToolList(flags, **configDic), **globalSettings)
    return calibTool

def load_yaml_cfg(configFile):
    from yaml import safe_load

    path_configFile = PathResolver.FindCalibFile(configFile)
    configDic = safe_load(open(path_configFile))
    return configDic


class JetCalibConfigError(Exception):
    """ Exception raised for invalid jet calibration config """

    def __init__(self, message):
        super().__init__(message)
        self.message = message

    def __str__(self):
        return self.message
