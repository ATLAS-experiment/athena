# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from Campaigns.Utils import Campaign, getMCCampaign
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType


def getYearsForCampaign(campaign):
    if campaign in [Campaign.MC16a, Campaign.MC20a]:
        return [2015, 2016]
    elif campaign in [Campaign.MC16d, Campaign.MC20d]:
        return [2017]
    elif campaign in [Campaign.MC16e, Campaign.MC20e]:
        return [2018]
    elif campaign in [Campaign.MC21a, Campaign.MC23a]:
        return [2022]
    elif campaign in [Campaign.MC23c, Campaign.MC23d]:
        return [2023]
    elif campaign in [Campaign.MC23e]:
        return [2024]
    elif campaign in [Campaign.MC23g]:
        return [2025]
    else:
        raise ValueError(f'Unsupported campaign {campaign}')


def getLumicalcDict():
    GRLDict={}
    ## RUN 3
    # 2025
    GRLDict['GRL2025'] = ['GoodRunsLists/data25_13p6TeV/20260129/ilumicalc_histograms_None_497924-509849_OflLumi-Run3-006.root']

    # 2024
    GRLDict['GRL2024'] = ['GoodRunsLists/data24_13p6TeV/20260127/ilumicalc_histograms_None_473235-486706_OflLumi-Run3-008.root']

    # 2023
    GRLDict['GRL2023'] = ['GoodRunsLists/data23_13p6TeV/20250321/ilumicalc_histograms_None_451587-456749_OflLumi-Run3-004.root']
    GRLDict['GRL2023_ignoreTRIG_JETCTPIN'] = ['GoodRunsLists/data23_13p6TeV/20250321/ilumicalc_histograms_None_451587-456749_OflLumi-Run3-004_ignoreTRIGJETCTPIN.root']

    # 2022
    GRLDict['GRL2022'] = ['GoodRunsLists/data22_13p6TeV/20250321/ilumicalc_histograms_None_431810-440613_OflLumi-Run3-004.root']
    GRLDict['GRL2022_ignore_TRIGLAR'] = ['GoodRunsLists/data22_13p6TeV/20250321/ilumicalc_histograms_None_430536-440613_OflLumi-Run3-004_ignore_TRIGLAR.root']

    ## RUN 2
    # 2018
    GRLDict['GRL2018_Triggerno17e33prim'] = ['GoodRunsLists/data18_13TeV/20190318/ilumicalc_histograms_None_348885-364292_OflLumi-13TeV-010.root']
    GRLDict['GRL2018_BjetHLT'] = ['GoodRunsLists/data18_13TeV/20200426/ilumicalc_histograms_None_348885-364292_OflLumi-13TeV-010-2.root']

    # 2017
    GRLDict['GRL2017_Triggerno17e33prim'] = ['GoodRunsLists/data17_13TeV/20180619/physics_25ns_Triggerno17e33prim.lumicalc.OflLumi-13TeV-010.root']
    GRLDict['GRL2017_BjetHLT_Normal2017'] = ['GoodRunsLists/data17_13TeV/20180619/physics_25ns_BjetHLT_Normal2017.lumicalc.OflLumi-13TeV-010.root']
    GRLDict['GRL2017_JetHLT_Normal2017'] = ['GoodRunsLists/data17_13TeV/20180619/physics_25ns_JetHLT_Normal2017.lumicalc.OflLumi-13TeV-010.root']

    # 2016
    GRLDict['GRL2016'] = ['GoodRunsLists/data16_13TeV/20180129/PHYS_StandardGRL_All_Good_25ns_297730-311481_OflLumi-13TeV-009.root']
    GRLDict['GRL2016_ignore_TOROID_STATUS'] = ['GoodRunsLists/data16_13TeV/20180129/PHYS_StandardGRL_All_Good_25ns_ignore_TOROID_STATUS_297730-311481_OflLumi-13TeV-009.root']
    GRLDict['GRL2016_BjetHLT'] = ['GoodRunsLists/data16_13TeV/20180129/PHYS_StandardGRL_All_Good_25ns_BjetHLT_297730-311481_OflLumi-13TeV-009.root']
    GRLDict['GRL2016_BjetHLT_Tight'] = ['GoodRunsLists/data16_13TeV/20180129/PHYS_StandardGRL_All_Good_25ns_BjetHLT_Tight_297730-311481_OflLumi-13TeV-009.root']

    # 2015
    GRLDict['GRL2015'] = ['GoodRunsLists/data15_13TeV/20170619/PHYS_StandardGRL_All_Good_25ns_276262-284484_OflLumi-13TeV-008.root']

    return GRLDict


def getActualMuDict():
    GRLDict={}
    ## RUN 3
    # 2025
    GRLDict['GRL2025'] = ['GoodRunsLists/data25_13p6TeV/20260129/purw.actualMu.root']

    # 2024
    GRLDict['GRL2024'] = ['GoodRunsLists/data24_13p6TeV/20260127/purw.actualMu.root']

    # 2023
    GRLDict['GRL2023'] = ['GoodRunsLists/data23_13p6TeV/20250321/purw.actualMu.root']
    GRLDict['GRL2023_ignoreTRIG_JETCTPIN'] = ['GoodRunsLists/data23_13p6TeV/20250321/purw.actualMu.ignoreTRIGJETCTPIN.root']

    # 2022
    GRLDict['GRL2022'] = ['GoodRunsLists/data22_13p6TeV/20250321/purw.actualMu.root']
    GRLDict['GRL2022_ignore_TRIGLAR'] = ['GoodRunsLists/data22_13p6TeV/20250321/purw.actualMu.ignore_TRIGLAR.root']

    ## RUN 2
    # 2018
    GRLDict['GRL2018_Triggerno17e33prim'] = ['GoodRunsLists/data18_13TeV/20190318/physics_25ns_Triggerno17e33prim.actualMu.OflLumi-13TeV-010.root']
    GRLDict['GRL2018_BjetHLT'] = ['GoodRunsLists/data18_13TeV/20200426/purw.actualMu.root']

    # 2017
    GRLDict['GRL2017_Triggerno17e33prim'] = ['GoodRunsLists/data17_13TeV/20180619/physics_25ns_Triggerno17e33prim.actualMu.OflLumi-13TeV-010.root']
    GRLDict['GRL2017_BjetHLT_Normal2017'] = ['GoodRunsLists/data17_13TeV/20180619/physics_25ns_BjetHLT_Normal2017.actualMu.OflLumi-13TeV-010.root']
    GRLDict['GRL2017_JetHLT_Normal2017'] = ['GoodRunsLists/data17_13TeV/20180619/physics_25ns_JetHLT_Normal2017.actualMu.OflLumi-13TeV-010.root']

    # 2016
    GRLDict['GRL2016'] = []
    GRLDict['GRL2016_ignore_TOROID_STATUS'] = []
    GRLDict['GRL2016_BjetHLT'] = []
    GRLDict['GRL2016_BjetHLT_Tight'] = []

    # 2015
    GRLDict['GRL2015'] = []

    return GRLDict


def getLumicalcFiles(campaign, GRLSuffixDict={}):
    """ Returns the list of lumicalc files for a given campaign """
    lumicalcDict = getLumicalcDict()
    data_years = getYearsForCampaign(campaign)

    file_list = []
    for data_year in data_years:
        GRLKey = 'GRL' + str(data_year)
        if data_year in GRLSuffixDict:
            GRLKey = GRLKey + '_' + GRLSuffixDict[data_year]
        elif data_year in [2017, 2018]:
            GRLKey = GRLKey + '_Triggerno17e33prim'
        file_list.extend(lumicalcDict[GRLKey])
    return file_list


def actualMuFiles(campaign, GRLSuffixDict={}):
    actualMuDict = getActualMuDict()
    data_years = getYearsForCampaign(campaign)

    file_list = []
    for data_year in data_years:
        GRLKey = 'GRL' + str(data_year)
        if data_year in GRLSuffixDict:
            GRLKey = GRLKey + '_' + GRLSuffixDict[data_year]
        elif data_year in [2017, 2018]:
            GRLKey = GRLKey + '_Triggerno17e33prim'
        file_list.extend(actualMuDict[GRLKey])

    if campaign in [Campaign.MC16d, Campaign.MC20d, Campaign.MC16e, Campaign.MC20e, Campaign.MC21a, Campaign.MC23a, Campaign.MC23c, Campaign.MC23d, Campaign.MC23e, Campaign.MC23g]:
        assert(len(file_list) == 1)
    else:
        assert(len(file_list) == 0)

    return file_list


def defaultConfigFiles(campaign):
    list = []

    if campaign in [Campaign.MC20a]:
        list.append(
            'PileupReweighting/mc20_common/mc20a.284500.physlite.prw.v1.root'
        )
    elif campaign in [Campaign.MC20d]:
        list.append(
            'PileupReweighting/mc20_common/mc20d.300000.physlite.prw.v1.root'
        )
    elif campaign in [Campaign.MC20e]:
        list.append(
            'PileupReweighting/mc20_common/mc20e.310000.physlite.prw.v1.root'
        )
    elif campaign in [Campaign.MC21a]:
        list.append(
            'PileupReweighting/mc21_common/mc21a.410000.physlite.prw.v1.root'
        )
    elif campaign in [Campaign.MC23a]:
        list.append(
            'PileupReweighting/mc23_common/mc23a.410000.physlite.prw.v2.root'
        )
    elif campaign in [Campaign.MC23c]:
        list.append(
            'PileupReweighting/mc23_common/mc23c.450000.physlite.prw.v1.root'
        )
    elif campaign in [Campaign.MC23d]:
        list.append(
            'PileupReweighting/mc23_common/mc23d.450000.physlite.prw.v1.root'
        )
    elif campaign in [Campaign.MC23e]:
        list.append(
            'PileupReweighting/mc23_common/mc23e.470000.physlite.prw.v1.root'
        )
    elif campaign in [Campaign.MC23g]:
        list.append(
            'PileupReweighting/mc23_common/mc23g.495000.physlite.prw.v1.root'
        )
    else:
        raise ValueError(f'Unsupported campaign {campaign}')

    assert(len(list) == 1)

    return list


def getConfigurationFiles(campaign=None, dsid=None, data_type=None, files=None, useDefaultConfig=False, GRLSuffixDict={}):
    # Attempt auto-configuration
    default_directory = 'dev/PileupReweighting/share'
    configuration_files = []

    if files is not None and (campaign is None or dsid is None or data_type is None):
        if campaign is None:
            campaign = getMCCampaign(files)

        if dsid is None or data_type is None:
            from AthenaConfiguration.AutoConfigFlags import GetFileMD
            metadata = GetFileMD(files)
            if dsid is None:
                dsid = str(metadata.get('mc_channel_number', 0))
            if data_type is None:
                simulation_flavour = GetFileMD(files).get('Simulator', '')
                if not simulation_flavour:
                    simulation_flavour = GetFileMD(files).get('SimulationFlavour', '')
                data_type = DataType.FullSim if (not simulation_flavour or 'FullG4' in simulation_flavour) else DataType.FastSim

    # data_type as in pileup analysis sequence: either 'data' or ('fullsim' or 'af3')
    if data_type is DataType.Data:
        raise ValueError('Data is not supported')

    if data_type is DataType.FullSim:
        simulation_type = 'FS'
    elif data_type is DataType.FastSim:
        simulation_type = 'AF3'
    else:
        raise ValueError(f'Invalid data_type {data_type}')

    configuration_files = actualMuFiles(campaign, GRLSuffixDict)
    if useDefaultConfig:
        configuration_files += defaultConfigFiles(campaign)
        return configuration_files

    config = f'{default_directory}/DSID{dsid[:3]}xxx/pileup_{campaign.value}_dsid{dsid}_{simulation_type}.root'
    from PathResolver import PathResolver
    if not PathResolver.FindCalibFile(config):
        return []
    else:
        configuration_files.append(config)
    return configuration_files
