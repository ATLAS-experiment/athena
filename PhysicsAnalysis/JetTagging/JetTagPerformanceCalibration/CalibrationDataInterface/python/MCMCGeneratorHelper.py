# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.Enums import LHCPeriod

MC20_Generator_dict = {
    "default": "default",
    "Powheg+Pythia8": "default",
    "aMcAtNlo+Pythia8": "410464",
    "Herwig713": "411233",
    "Herwig721": "600666",
    "Herwig723": "600666",
    "aMcAtNlo+Herwig7": "412116",
    "Sherpa221": "410250",
    "Sherpa2210": "700122",
    "Sherpa2211": "700122",
    "Sherpa2212": "700660",
    "Sherpa2214": "700660",
}

MC23_Generator_dict = {
    "default": "default",
    "Powheg+Pythia8": "default",
    "Herwig721": "601414",
    "Herwig723": "601414",
    "Sherpa2211": "700660",
    "Sherpa2212": "700660",
    "Sherpa2214": "700660",
}

def MCMC_generator_map(generatorDict):
    generator = None
    if 'Powheg' in generatorDict and 'Pythia8' in generatorDict:
        generator = 'Powheg+Pythia8'
    elif 'aMcAtNlo' in generatorDict and 'Pythia8' in generatorDict:
        generator = 'aMcAtNlo+Pythia8'
    elif 'aMcAtNlo' in generatorDict and 'Herwig7' in generatorDict:
        generator = 'aMcAtNlo+Herwig7'
    elif 'Herwig7' in generatorDict:
        generator = 'Herwig'+generatorDict['Herwig7'].replace('.', '')
    elif 'Sherpa' in generatorDict:
        generator = 'Sherpa'+generatorDict['Sherpa'].replace('.', '')
    else:
        generator = str(generatorDict)
    return generator

def MCMC_dsid_map(geometry, generatorDict={}, selfDefineGenerator=None):
    """use metadata(generatorDict) or self set generator (selfDefineGenerator)
       to get the generator setting for MCMC efficiency map"""

    if geometry is LHCPeriod.Run2:
        mc_dict = MC20_Generator_dict
    elif geometry is LHCPeriod.Run3:
        mc_dict = MC23_Generator_dict
    else:
        raise ValueError("No CDI MCMC map avaialble for " +  geometry)

    dsid = None
    generator = None

    if selfDefineGenerator is not None and selfDefineGenerator != "autoconfig":
        generator = selfDefineGenerator
    else:
        generator = MCMC_generator_map(generatorDict)
    if generator in mc_dict:
        dsid = mc_dict[generator]

    if dsid is None:
        raise ValueError("No CDI MCMC map avaialble for generator: " + generator )
    else:
        return dsid
