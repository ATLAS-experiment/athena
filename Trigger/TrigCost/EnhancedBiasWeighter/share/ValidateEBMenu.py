#!/usr/bin/env python
#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

import json

from AthenaCommon.Logging import logging
log = logging.getLogger('ValidateEBMenu.py')


def getL1Category(chainName):
    return chainName.replace("HLT_noalg_L1", "L1_").replace("_noPS", "")


if __name__=='__main__':
    from argparse import ArgumentParser
    parser = ArgumentParser()
    parser.add_argument('--hltPsk', type=str, help='HLTPrescale.json file')
    parser.add_argument('--l1Psk', type=str, help='HLTPrescale.json file')
    args = parser.parse_args()

    ebChains = [
        "HLT_eb_low_L1RD2_FILLED", "HLT_eb_medium_L1RD2_FILLED",
        "HLT_noalg_L1PhysicsHigh_noPS", "HLT_noalg_L1PhysicsVeryHigh_noPS",
        "HLT_noalg_L1RD3_FILLED", "HLT_noalg_L1RD3_EMPTY", 
        "HLT_noalg_L1EMPTY_noPS", "HLT_noalg_L1FIRSTEMPTY_noPS",
        "HLT_noalg_L1UNPAIRED_ISO_noPS", "HLT_noalg_L1UNPAIRED_NONISO_noPS"
    ]

    if not args.hltPsk or not args.l1Psk:
        log.error("Prescale files are required!")
        exit(-1)

    with open(args.hltPsk) as f:
        HLTPrescales = json.load(f)["prescales"]

    with open(args.l1Psk) as f:
        L1Prescales = json.load(f)["cutValues"]

    from TriggerMenuMT.HLT.Menu.L1Seeds import getEBnoL1PSSeed
    from TriggerMenuMT.HLT.CalibCosmicMon.EnhancedBiasChainConfiguration import l1seeds

    alreadyProcessed = {}

    for chainName in ebChains:
        log.info(f"Processing chain {chainName}...")
        hltPrescale = HLTPrescales[chainName]
        if not hltPrescale:
            log.error(f"Chain {chainName} not found in the HLT Prescales file")
            continue

        if not hltPrescale["enabled"]:
            log.error(f"Chain {chainName} disabled in the HLT Prescales file")
            continue

        if "noalg" not in chainName: # randomly seeded chains
            if "medium" in chainName:
                currentSeeds = l1seeds["medium"]
            else:
                currentSeeds = l1seeds["low"]

            for seedName in currentSeeds:
                if seedName in alreadyProcessed:
                    log.error(f"Seed {seedName} is already part of eb chain {alreadyProcessed[seedName]}")
                else:
                    alreadyProcessed[seedName] = chainName

                l1Prescale = L1Prescales[seedName]
                if not l1Prescale:
                    log.error(f"Seed {seedName} of chain {chainName} not found in the L1 Prescales file")
                    continue

                if not l1Prescale["enabled"]:
                    log.error(f"Seed {seedName} of chain {chainName} is disabled in the L1 Prescales file")
                    continue

        elif "_noPS" in chainName:
            currentSeeds = getEBnoL1PSSeed([], getL1Category(chainName))
            for seedName in currentSeeds:
                if seedName in alreadyProcessed:
                    log.error(f"Seed {seedName} is already part of eb chain {alreadyProcessed[seedName]}")
                else:
                    alreadyProcessed[seedName] = chainName
                    
                l1Prescale = L1Prescales[seedName]
                if not l1Prescale:
                    log.error(f"Seed {seedName} of chain {chainName} not found in the L1 Prescales file")
                    continue

                if not l1Prescale["enabled"]:
                    log.error(f"Seed {seedName} of chain {chainName} is disabled in the L1 Prescales file")
                    continue

                if l1Prescale["cut"] != 1:
                    log.error(f"Seed {seedName} of chain {chainName} is prescaled in the L1 Prescales file (it is in no prescale category)")
                    continue
        else:
            # randoms
            seedName = getL1Category(chainName)
            l1Prescale = L1Prescales[seedName]
            if not l1Prescale:
                log.error(f"Seed {seedName} of chain {chainName} not found in the L1 Prescales file")
                continue

            if not l1Prescale["enabled"]:
                log.error(f"Seed {seedName} of chain {chainName} is disabled in the L1 Prescales file")
                continue