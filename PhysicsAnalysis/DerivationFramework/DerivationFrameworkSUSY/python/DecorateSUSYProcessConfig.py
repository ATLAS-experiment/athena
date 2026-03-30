# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

#==============================================================================
# SUSY signal process augmentation
# Component accumulator version
#==============================================================================

def IsSUSYSignalRun3(flags):
    """Identify SUSY signal sample"""
    if not flags.Input.isMC:
        return False
    # with MC16, there are no dedicated SUSY DSID blocks anymore but blocks for
    # each generator: see https://twiki.cern.ch/twiki/bin/view/AtlasProtected/PmgMcSoftware#DSID_blocks
    if flags.Input.MCChannelNumber >= 500000:
        # there does not seem to be an elegant way to check whether its a SUSY sample
        # or not, thus run the mark all MadGraph samples as SUSY for the moment
        # to non-SUSY MG samples this adds only an empty decoration and does not break anything
        isSUSY = flags.Input.MCChannelNumber < 600000
        print("DecorateSUSYProcess: fileinfo.mc_channel_number",
              flags.Input.MCChannelNumber, "is SUSY (aka is MG):", isSUSY)
    # for pre-MC16 samples use the old way
    else:
        import os
        if not os.access('/cvmfs/atlas.cern.ch/repo/sw/Generators/MC15JobOptions/latest/share/Blocks.list', os.R_OK):
            # Do it the old-fashioned way
            # https://svnweb.cern.ch/trac/atlasoff/browser/Generators/MC15JobOptions/trunk/share/Blocks.list
            isSUSY = (370000 <= flags.Input.MCChannelNumber < 405000) or (406000 <= flags.Input.MCChannelNumber < 410000) \
                or (436000 <= flags.Input.MCChannelNumber < 439000) or (448000 <= flags.Input.MCChannelNumber < 450000)
        else:
            # Automatic detection based on cvmfs
            isSUSY = False
            blocks = open('/cvmfs/atlas.cern.ch/repo/sw/Generators/MC15JobOptions/latest/share/Blocks.list', 'r')
            for l in blocks.readlines():
                if 'SUSY' not in l:
                    continue
                myrange = l.split()[0].replace('DSID', '').replace('xxx', '000', 1).replace('xxx', '999', 1)
                low = int(myrange.split('-')[0])
                high = int(myrange.split('-')[1]) if '-' in myrange else int(myrange.replace('000', '999'))
                if low <= flags.Input.MCChannelNumber and flags.Input.MCChannelNumber <= high:
                    isSUSY = True
                    break
        print("DecorateSUSYProcess: fileinfo.mc_channel_number",
              flags.Input.MCChannelNumber, "is SUSY:", isSUSY)
    return isSUSY

# Configure SUSY signal tagger


def SUSYSignalTaggerCfg(flags, derivationName):
    """Configure SUSY signal tagger"""
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.ComponentFactory import CompFactory
    acc = ComponentAccumulator()
    if not IsSUSYSignalRun3(flags):
        print("SUSYSignalTaggerCfg WARNING: Trying to decorate, but sample is not SUSY signal?")
    acc.addEventAlgo(CompFactory.DerivationFramework.SUSYSignalTagger(name=derivationName + "SignalTagger",
                                                                       EventInfoName="EventInfo",
                                                                       MCCollectionName="TruthParticles"))
    return acc
