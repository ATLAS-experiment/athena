# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.Enums import LHCPeriod

DL1dv01_MC20_Generator_dict = {
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

DL1dv01_MC23_Generator_dict = {
    "default": "default",
    "Powheg+Pythia8": "default",
    "Herwig721": "601414",
    "Herwig723": "601414",
    "Sherpa2211": "700660",
    "Sherpa2212": "700660",
    "Sherpa2214": "700660",
}

GN2v01_MC20_Generator_dict = {
    "default": "default",
    "Pythia8": "default",
    "Herwig713": "411233",
    "Herwig721": "600666",
    "Herwig723": "600666",
    "Sherpa2210": "700660",
    "Sherpa2211": "700660",
    "Sherpa2212": "700660",
    "Sherpa2214": "700660",
    "Sherpa2216": "700660",
}

GN2v01_MC23_Generator_dict = {
    "default": "default",
    "Pythia8": "default",
    "Herwig721": "601414",
    "Herwig723": "601414",
    "Sherpa2212": "700808",
    "Sherpa2214": "700808",
    "Sherpa2216": "700808",
}
dict_tagger_generator = {
        ('DL1dv01', LHCPeriod.Run2): DL1dv01_MC20_Generator_dict,
        ('DL1dv01', LHCPeriod.Run3): DL1dv01_MC23_Generator_dict,
        ('GN2v01', LHCPeriod.Run2): GN2v01_MC20_Generator_dict,
        ('GN2v01', LHCPeriod.Run3): GN2v01_MC23_Generator_dict,
        }
def MCMC_generator_map(generatorDict, tagger='GN2v01'):
    generator = None
    if tagger == 'DL1dv01':
        if 'Powheg' in generatorDict and 'Pythia8' in generatorDict:
            generator = 'Powheg+Pythia8'
        elif ('aMcAtNlo' in generatorDict or 'MadGraph' in generatorDict) and 'Pythia8' in generatorDict:
            generator = 'aMcAtNlo+Pythia8'
        elif 'aMcAtNlo' in generatorDict and 'Herwig7' in generatorDict:
            generator = 'aMcAtNlo+Herwig7'
        elif 'Herwig7' in generatorDict:
            generator = 'Herwig'+generatorDict['Herwig7'].replace('.', '')
    else:
        if 'Herwig7' in generatorDict:
            generator = 'Herwig'+generatorDict['Herwig7'].replace('.', '')[:3]
        elif 'Pythia8' in generatorDict:
            generator = 'Pythia8'
    if 'Sherpa' in generatorDict:
        sherpa_versions = ['Sherpa2210', 'Sherpa2211', 'Sherpa2212', 'Sherpa2214', 'Sherpa2216']
        generator = 'Sherpa'+generatorDict['Sherpa'].replace('.', '')
        if any(generator.startswith(version) for version in sherpa_versions):
            generator = generator[:10]
        elif generator.startswith('Sherpa221'):
            generator = 'Sherpa221'
    if generator is None:
        generator = str(generatorDict)
    return generator

def MCMC_dsid_map(geometry, generatorDict={}, selfDefineGenerator=None, tagger='GN2v01'):
    """use metadata(generatorDict) or self set generator (selfDefineGenerator)
       to get the generator setting for MCMC efficiency map"""

    if (tagger, geometry) not in dict_tagger_generator:
        raise ValueError("No CDI MCMC map avaialble for " + tagger + " in " + str(geometry))
    mc_dict = dict_tagger_generator[tagger, geometry]

    dsid = None
    generator = None

    if selfDefineGenerator is not None and selfDefineGenerator != "autoconfig":
        generator = selfDefineGenerator
    else:
        generator = MCMC_generator_map(generatorDict, tagger)
    if generator in mc_dict:
        dsid = mc_dict[generator]

    if dsid is None:
        raise ValueError("No CDI MCMC map avaialble for generator: " + generator + " with " + tagger + " in " + str(geometry))
    else:
        return dsid
