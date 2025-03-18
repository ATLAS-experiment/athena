# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

def map_mc_campaign(MCCampaign):
    MCCampaign = MCCampaign.value
    if MCCampaign.lower().startswith("mc20"):
        return "MC20"
    elif MCCampaign.lower().startswith("mc21"):
        return "MC21"
    elif MCCampaign.lower().startswith("mc23"):
        return "MC23"
    else:
        return "MCUnknown"

def check_CDI_campaign(MCCampaign, CDIFile):
    """ Check if the right CDI file is use for a given MC Campaign """
    MCCampaign = map_mc_campaign(MCCampaign)
    if MCCampaign == "MCUnknown":
        raise ValueError("The MC Campaign is not recognized. Currently we only have CDI file for MC20, MC21 and MC23.")

    run3_campaigns = ["MC21", "MC23"]
    if MCCampaign == "MC20":
        if MCCampaign not in CDIFile and "nosf" not in CDIFile.lower():
            raise ValueError("Mismatch of MC Campaign and CDI file. CDI file %s being used for %s campaign" % (CDIFile, MCCampaign))
    elif not any (campaign in CDIFile for campaign in run3_campaigns) and "nosf" not in CDIFile.lower():
        raise ValueError("Mismatch of MC Campaign and CDI file. CDI file %s being used for %s campaign" % (CDIFile, MCCampaign))

