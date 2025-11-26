# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from Campaigns.Utils import getDataYear
from PyUtils.Logging import logging
msg = logging.getLogger('BJetTriggerByYearContent')
msg.setLevel(logging.INFO)

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
    elif flags.Trigger.EDMVersion == 3:
        triggerContent = [
            "HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_bJets",
            "HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_BTagging",
        ]

        year = getDataYear(flags)
        msg.debug(f'Configured Run 3 b-jet trigger content for {year}')

        btagstrs = []
        btaggers = {
            2022: ['DL1d20211216'],
            2023: ['GN120220813'],
            2024: ['GN220240122'],
            2025: ['GN220240122'],
            2026: ['GN220240122'],
        }[year]
        for btagger in btaggers:
            btagstrs.append('.'.join([f'{btagger}_{p}' for p in ['pb','pc','pu']]))
        jetstrs = btagstrs + ['pt', 'eta', 'phi', 'm']
        btagvars = '.'.join(btagstrs)
        jetvars = '.'.join(jetstrs)
        triggerContent.append(f"HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_BTaggingAux.{btagvars}")
        triggerContent.append(f"HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_bJetsAux.{jetvars}")
        return triggerContent

    elif flags.Trigger.EDMVersion >= 4:
        triggerContent = ["HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_bJets"]

        year = getDataYear(flags)
        msg.debug(f'Configured Run 4 b-jet trigger content for {year}')

        jetstrs = ['pt', 'eta', 'phi', 'm']
        btaggers = {
            2030: ['GN220240122'], # Some day we'll have something amazing here
        }[year]
        for btagger in btaggers:
            jetstrs.append('.'.join([f'{btagger}_{p}' for p in ['pb','pc','pu']]))
        jetvars = '.'.join(jetstrs)
        triggerContent.append(f"HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_bJetsAux.{jetvars}")
        return triggerContent

    elif flags.Trigger.EDMVersion == -1:
        # Allow for undefined trigger content -- no RDOtoRDOTrigger run
        msg.debug('Received EDMVersion=-1: no trigger info available. Returning empty b-jet trigger content')
        return []

    raise ValueError(f"Unsupported EDM version {flags.Trigger.EDMVersion} determined")
