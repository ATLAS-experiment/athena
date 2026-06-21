# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.Enums import LHCPeriod

def chainDefinitions_TrigBtagEmulation(flags,
                                       toBeEmulatedTriggers: list) -> list:
    ### determine trigger thresholds from to be emulated chain names
    chainDefinitions = {}
    for chain in toBeEmulatedTriggers:
        if flags.GeoModel.Run is LHCPeriod.Run3:
            from TriggerMenuMT.HLT.Config.Utility.DictFromChainName import dictFromChainName
            from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp
            chainDict = dictFromChainName(flags, chain)
            chainParts = ['L1item:' + chainDict['L1item']]
            for chainPart in chainDict['chainParts']:
                # Gaudi::Property has limited support for nested structures, put everything in a long string and parse in C++
                # and update TrigBtagEmulationChain::parseChainDefinition(...) accordingly
                partDefinition = ''
                partDefinition += 'L1threshold:' + chainPart['L1threshold']
                partDefinition += '|name:' + chainPart['chainPartName']
                partDefinition += '|multiplicity:' + chainPart['multiplicity']
                partDefinition += '|threshold:' + chainPart['threshold']
                partDefinition += '|etaRange:' + chainPart['etaRange']
                partDefinition += '|jvt:' + (chainPart['jvt'] if chainPart['jvt'] else '-99999')
                partDefinition += '|tagger:' + chainPart['bTag']
                partDefinition += '|jetpresel:' + chainPart['trkpresel']
                partDefinition += '|dijetmass:' + (chainPart['hypoScenario'][len('DJMASS'):] if ('DJMASS' in chainPart['hypoScenario']) else 'None')
                partDefinition += '|isPFlow:' + ('True' if (chainPart['constitType'] == 'pf') else 'False')
                partDefinition += '|isShared:' + ('True' if ('SHARED' in chainPart['chainPartName']) else 'False')
                partDefinition += '|GSCthreshold:-99999'
                chainParts.append(partDefinition)
            chainName = chain.name if isinstance(chain, ChainProp) else chain
            chainDefinitions[chainName] = chainParts
        elif flags.GeoModel.Run is LHCPeriod.Run2:
            from ROOT.ChainNameParser import HLTChainInfo
            chainInfo = HLTChainInfo(chain)
            chainParts = ['L1item:' + chainInfo.l1Item()]
            for legInfo in chainInfo:
                eta = '0eta320'
                tagger = ''
                gscthreshold = '-99999'
                for part in legInfo.legParts:
                    if part.startswith('b'):
                        tagger = part
                    elif 'eta' in part:
                        eta = part
                    elif part.startswith('gsc'):
                        gscthreshold = str(part)[3:]
                partDefinition = ''
                partDefinition += 'L1threshold:' + chainInfo.l1Item()
                partDefinition += '|name:' + legInfo.legName()
                partDefinition += '|multiplicity:' + str(legInfo.multiplicity)
                partDefinition += '|threshold:' + str(legInfo.threshold)
                partDefinition += '|etaRange:' + eta
                partDefinition += '|jvt:-99999'
                partDefinition += '|tagger:' + tagger
                partDefinition += '|jetpresel:nopresel'
                partDefinition += '|dijetmass:None' # TODO: add invm chain support
                partDefinition += '|isPFlow:False'
                partDefinition += '|isShared:False'
                partDefinition += '|GSCthreshold:' + gscthreshold
                chainParts.append(partDefinition)
            chainDefinitions[chain] = chainParts

    return chainDefinitions

def TrigBtagEmulation_kwargs(flags, toBeEmulatedTriggers: list, **kwargs):
    ### determine trigger thresholds from to be emulated chain names
    chainDefinitions = chainDefinitions_TrigBtagEmulation(flags, toBeEmulatedTriggers)
    kwargs.setdefault('EmulatedChainDefinitions', chainDefinitions)

    kwargs.setdefault('LHCPeriod', 3 if flags.GeoModel.Run is LHCPeriod.Run3 else 2)

    if flags.GeoModel.Run is LHCPeriod.Run3:
        from TrigBjetHypo.TrigBjetBtagHypoTool import bTaggingWP
        from TrigBjetHypo.TrigBjetBtagHypoTool import bbTaggingWP
        working_points = { "newTagger" : 1.234 }
        working_points.update(bTaggingWP)
        working_points.update(bbTaggingWP)

        kwargs.setdefault('FTD_Remapping', 
                          {"DL1d20210519r22_pu" : "DL1dEMUL_pu",
                           "DL1d20210519r22_pc" : "DL1dEMUL_pc",
                           "DL1d20210519r22_pb" : "DL1dEMUL_pb"})
    elif flags.GeoModel.Run is LHCPeriod.Run2:
        # Run2 taggers from https://twiki.cern.ch/twiki/bin/view/Atlas/OnlineBTaggingBenchmarks
        working_points = { 
            "mv2c2040": 0.75,
            "mv2c2050": 0.50,
            "mv2c2060": -0.0224729,
            "mv2c2070": -0.509032,
            "mv2c2077": -0.764668,
            "mv2c2085": -0.938441,
        }
        working_points.update({
            "mv2c1040": 0.978,
            "mv2c1050": 0.948,
            "mv2c1060": 0.846,
            "mv2c1070": 0.580,
            "mv2c1077": 0.162,
            "mv2c1085": -0.494
        })

    kwargs.setdefault('WorkingPoints', working_points)

    return kwargs
