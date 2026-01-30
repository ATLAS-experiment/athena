# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from Campaigns.Utils import getDataYear
import logging
msg = logging.getLogger('BJetTriggerByYearContent')
msg.setLevel(logging.INFO)

run3_year_tagger_map = {
    2022: 'bdl1d',
    2023: 'bgn1',
    2024: 'bgn2',
    2025: 'bgn2',
    2026: 'bgn2',
    2030: 'bgn2', # Some day we'll have something amazing here
}

run3_tagger_deco_map = {
    "bdl1r":  ["DL1r"],
    "bdl1d":  ["DL1d20211216"],
    "bgn1" :  ["GN120220813"],
    "bgn2" :  ["GN220240122"],
}

def getDecoByTrigName(trigName):
    decoration_list = []
    for tagger, deco in run3_tagger_deco_map.items():
        if tagger in trigName:
            decoration_list += deco
    if len(decoration_list) == 0:
        raise ValueError(f"Could not find decorations for trigger name {trigName}, avaialable sub-strings: {list(run3_tagger_deco_map.keys())}")
    # deduplicate
    return list(set(decoration_list))

def getBJetTriggerContent(flags):
    if flags.Trigger.EDMVersion == 2:
        triggerContent = [
            "HLT_xAOD__BTaggingContainer_HLTBjetFex",
            "HLT_xAOD__BTaggingContainer_HLTBjetFexAux.MV2c00_discriminant.MV2c10_discriminant.MV2c20_discriminant.BTagBtagToJetAssociator",
        ]
        jetCollections = {
            2016: [
                # Jet collections needed for HLT jet matching
                "HLT_xAOD__JetContainer_a4tcemsubjesFS",
                "HLT_xAOD__JetContainer_a4tcemsubjesFSAux.pt.eta.phi.m",
                # B-jet collections needed for HLT jet matching
                "HLT_xAOD__JetContainer_EFJet", # 2015
                "HLT_xAOD__JetContainer_EFJetAux.pt.eta.phi.m",
                "HLT_xAOD__JetContainer_SplitJet",
                "HLT_xAOD__JetContainer_SplitJetAux.pt.eta.phi.m",
            ],
            2017: [
                # Jet collections needed for HLT jet matching
                "HLT_xAOD__JetContainer_a4tcemsubjesISFS",
                "HLT_xAOD__JetContainer_a4tcemsubjesISFSAux.pt.eta.phi.m",
                # B-jet collections needed for HLT jet matching
                "HLT_xAOD__JetContainer_SplitJet", # For Btag->Split->GSC matching in 2017 and 2018, and low-pt 2b2j in 2018
                "HLT_xAOD__JetContainer_SplitJetAux.pt.eta.phi.m",
                "HLT_xAOD__JetContainer_GSCJet",
                "HLT_xAOD__JetContainer_GSCJetAux.pt.eta.phi.m",
            ],
        }
        jetCollections[2015] = jetCollections[2016]
        jetCollections[2018] = jetCollections[2017]

        year = getDataYear(flags)
        msg.debug(f'Configured b-jet trigger content for {year}')

        triggerContent += jetCollections[year]
        return triggerContent
    elif flags.Trigger.EDMVersion >= 3: # currently Run 3 and 4 shares the same code block
        year = getDataYear(flags)
        msg.debug(f'Configured Run 3 / Run 4 b-jet trigger content for {year}')

        ftagstrs = []
        ftaggers = run3_tagger_deco_map.get(run3_year_tagger_map[year])
        for ftagger in ftaggers:
            ftagstrs.append('.'.join([f'{ftagger}_{p}' for p in ['pb','pc','pu']]))
        jetstrs = ftagstrs + ['pt', 'eta', 'phi', 'm']
        jetvars = '.'.join(jetstrs)
        triggerContent = [
            "HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_bJets",
            f"HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_bJetsAux.{jetvars}",
        ]
        return triggerContent

    elif flags.Trigger.EDMVersion == -1:
        # Allow for undefined trigger content -- no RDOtoRDOTrigger run
        msg.debug('Received EDMVersion=-1: no trigger info available. Returning empty b-jet trigger content')
        return []

    raise ValueError(f"Unsupported EDM version {flags.Trigger.EDMVersion} determined")
