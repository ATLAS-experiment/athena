# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.Enums import FlagEnum
class Campaign(FlagEnum):
    Unknown = ''
    MC16a = 'mc16a'
    MC16d = 'mc16d'
    MC16e = 'mc16e'
    MC20a = 'mc20a'
    MC20d = 'mc20d'
    MC20e = 'mc20e'
    MC21a = 'mc21a'
    MC23a = 'mc23a'
    MC23c = 'mc23c'
    MC23d = 'mc23d'
    MC23e = 'mc23e'
    PhaseII = 'phaseII'

# Campaign run numbers (only latest campaigns that do not have metadata present)
campaign_runs = {
    284500: Campaign.MC20a,
    300000: Campaign.MC20d,
    310000: Campaign.MC20e,
    410000: Campaign.MC21a,
}

# Function to get the campaign
def getMCCampaign(files):
    # Auto-configure from file
    from AthenaConfiguration.AutoConfigFlags import GetFileMD
    metadata = GetFileMD(files)
    mc_campaign = Campaign(metadata.get('mc_campaign', ''))
    project_name = metadata.get('project_name', '')
    run_numbers = metadata.get('runNumbers', [])

    if mc_campaign is not Campaign.Unknown:
        return mc_campaign

    if run_numbers:
        mc_campaign = campaign_runs.get(run_numbers[0], Campaign.Unknown)

    # MC-equivalent projects for data
    if 'data24' in project_name:
        return Campaign.MC23e
    elif 'data23' in project_name:
        return Campaign.MC23c
    elif 'data22' in project_name:
        return Campaign.MC21a
    elif 'data18' in project_name:
        return Campaign.MC20e
    elif 'data17' in project_name:
        return Campaign.MC20d
    elif 'data16' in project_name or 'data15' in project_name:
        return Campaign.MC20a

    return mc_campaign
