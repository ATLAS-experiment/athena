# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from Campaigns.Utils import Campaign, getMCCampaign
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType


def getLumicalcFiles(campaign):
    """ Returns the list of lumicalc files for a given campaign """
    ilumical_files = {
        Campaign.MC16a: [
            'GoodRunsLists/data15_13TeV/20170619/PHYS_StandardGRL_All_Good_25ns_276262-284484_OflLumi-13TeV-008.root',
            'GoodRunsLists/data16_13TeV/20180129/PHYS_StandardGRL_All_Good_25ns_297730-311481_OflLumi-13TeV-009.root'
        ],
        Campaign.MC16d: [
            'GoodRunsLists/data17_13TeV/20180619/physics_25ns_Triggerno17e33prim.lumicalc.OflLumi-13TeV-010.root'
        ],
        Campaign.MC16e: [
            'GoodRunsLists/data18_13TeV/20190318/ilumicalc_histograms_None_348885-364292_OflLumi-13TeV-010.root'
        ],
        Campaign.MC20a: [
            'GoodRunsLists/data15_13TeV/20170619/PHYS_StandardGRL_All_Good_25ns_276262-284484_OflLumi-13TeV-008.root',
            'GoodRunsLists/data16_13TeV/20180129/PHYS_StandardGRL_All_Good_25ns_297730-311481_OflLumi-13TeV-009.root'
        ],
        Campaign.MC20d: [
            'GoodRunsLists/data17_13TeV/20180619/physics_25ns_Triggerno17e33prim.lumicalc.OflLumi-13TeV-010.root'
        ],
        Campaign.MC20e: [
            'GoodRunsLists/data18_13TeV/20190318/ilumicalc_histograms_None_348885-364292_OflLumi-13TeV-010.root'
        ],
        Campaign.MC21a: [
            'GoodRunsLists/data22_13p6TeV/20230207/ilumicalc_histograms_None_431810-440613_OflLumi-Run3-003.root'
        ],
        Campaign.MC23a: [
            'GoodRunsLists/data22_13p6TeV/20230207/ilumicalc_histograms_None_431810-440613_OflLumi-Run3-003.root'
        ],
        Campaign.MC23c: [
            'GoodRunsLists/data23_13p6TeV/20230828/ilumicalc_histograms_None_451587-456749_OflLumi-Run3-003.root'
        ],
        Campaign.MC23d: [
            'GoodRunsLists/data23_13p6TeV/20230828/ilumicalc_histograms_None_451587-456749_OflLumi-Run3-003.root'
        ],
        Campaign.MC23e: [
            'GoodRunsLists/data24_13p6TeV/20241118/ilumicalc_histograms_None_473235-486706_OflLumi-Run3-005.root'
        ]
    }

    try:
        return ilumical_files[campaign]
    except KeyError:
        raise ValueError(f'Unsupported campaign {campaign} for lumicalc files')


def actualMuFiles(campaign):
    list = []

    if campaign in [Campaign.MC16d, Campaign.MC20d]:
        list.append(
            'GoodRunsLists/data17_13TeV/20180619/physics_25ns_Triggerno17e33prim.actualMu.OflLumi-13TeV-010.root'
        )
    elif campaign in [Campaign.MC16e, Campaign.MC20e]:
        list.append(
            'GoodRunsLists/data18_13TeV/20190318/physics_25ns_Triggerno17e33prim.actualMu.OflLumi-13TeV-010.root'
        )
    elif campaign in [Campaign.MC21a, Campaign.MC23a]:
        list.append(
            'GoodRunsLists/data22_13p6TeV/20230207/purw.actualMu.2022.root'
        )
    elif campaign in [Campaign.MC23c, Campaign.MC23d]:
        list.append(
            'GoodRunsLists/data23_13p6TeV/20230828/purw.actualMu.2023.root'
        )
    elif campaign in [Campaign.MC23e]:
        list.append(
            'GoodRunsLists/data24_13p6TeV/20241118/purw.actualMu.root'
        )

    if campaign in [Campaign.MC16d, Campaign.MC20d, Campaign.MC16e, Campaign.MC20e, Campaign.MC21a, Campaign.MC23a, Campaign.MC23c, Campaign.MC23d, Campaign.MC23e]:
        assert(len(list) == 1)
    else:
        assert(len(list) == 0)

    return list


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
    else:
        raise ValueError(f'Unsupported campaign {campaign}')

    assert(len(list) == 1)

    return list


def getConfigurationFiles(campaign=None, dsid=None, data_type=None, files=None, useDefaultConfig=False):
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

    configuration_files = actualMuFiles(campaign)
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
