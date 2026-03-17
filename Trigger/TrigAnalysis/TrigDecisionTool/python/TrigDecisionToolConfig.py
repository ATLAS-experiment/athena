#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaCommon.Logging import logging


@AccumulatorCache
def TrigDecisionToolCfg(flags):
    '''
    @brief Configures and returns the TrigDecisionTool (TDT) for use in Athena-MT. 
    The TDT uses a Tool interface such that it can be used in either Athena or AnalysisBase releases.
    The manages an MT-safe internal state, hence we should only have one instance of it configured in any job.
    It is thus one of the few places where it OK to use a PublicTool in Athena-MT.

    Obtain the configured public tool instance by calling getPrimary()
    on the ComponentAccumulator returned by this function, or by calling
    getPublicTool('TrigDecisionTool') or getPrimaryAndMerge()
    on any downstream merged ComponentAccumulator.


    When running in AnalysisBase, the tdt.TrigConfigTool='TrigConf::xAODConfigTool' should be used 
    in place of the TrigConf::xAODConfigSvc
    '''
    msg = logging.getLogger('TrigDecisionToolCfg')
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.Enums import Format, Project
    acc = ComponentAccumulator()

    from TrigConfxAOD.TrigConfxAODConfig import getxAODConfigSvc
    cfgsvc = acc.getPrimaryAndMerge(getxAODConfigSvc(flags))

    tdt = CompFactory.Trig.TrigDecisionTool('TrigDecisionTool')
    tdt.TrigConfigSvc = cfgsvc
    use_run3_format = flags.Trigger.EDMVersion >= 3 # or flags.Trigger.doEDMVersionConversion (to be added when this is working)
    tdt.NavigationFormat = 'TrigComposite' if use_run3_format else 'TriggerElement'
    tdt.HLTSummary = getRun3NavigationContainerFromInput(flags)

    if flags.Input.Format is Format.BS and flags.Trigger.EDMVersion in [1, 2]:
        tdt.UseAODDecision = True

    if flags.Common.Project is not Project.AthAnalysis:
        # Full Athena
        # This pre-loads libraries required to read the run 2 trigger navigation
        from TrigEDMConfig.TriggerEDM import EDMLibraries
        nav = CompFactory.HLT.Navigation('Navigation')
        nav.Dlls = [e for e in  EDMLibraries if 'TPCnv' not in e]
        tdt.Navigation = nav
        acc.addPublicTool(nav)

    acc.addPublicTool(tdt, primary=True)

    msg.info('Configuring the TrigDecisionTool and xAODConfigSvc to use ConfigSource: %s, Run3NavigationFormat: %s, Run3NavigationSummaryCollection: %s',
        'InFileMetadata' if flags.Trigger.triggerConfig == 'INFILE' else 'ConditionsAndDetStore',
        str(use_run3_format),
        tdt.HLTSummary)

    return acc


# List of all possible keys of the Run 3 navigation summary collection
# in order of verbosity. Want to take the most verbose which is available.
possible_keys = [
    'HLTNav_Summary', # Produced initially online (only the final nodes, all other nodes spread out over many many collections created by trigger framework algs)
    'HLTNav_Summary_OnlineSlimmed', # Produced online, all nodes in one container. Accessible during RAWtoALL, good for T0 monitoring.
    'HLTNav_Summary_ESDSlimmed', # Produced offline in jobs writing ESD. Minimal slimming, good for T0-style monitoring. Equivalent to AOD at AODFULL level.
    'HLTNav_Summary_AODSlimmed', # Produced offline in jobs writing AOD. Minimal slimming in AODFULL mode, good for T0-style monitoring. Slimming applied in AODSLIM mode, good for analysis use, final-features accessible.
    'HLTNav_Summary_DAODSlimmed', # Chain level slimming and IParticle feature-compacting for DAOD. Good for analysis use, final-features four vectors accessible.
    'HLTNav_R2ToR3Summary' # Output of Run 2 to Run 3 navigation conversion procedure. Somewhat equivalent to AODFULL level. Designed to be further reduced to DAODSlimmed level before analysis use.
    ]

@AccumulatorCache
def getRun3NavigationContainerFromInput(flags):
    # What to return if we cannot look in the file
    from AthenaConfiguration.Enums import LHCPeriod
    if getattr(flags.Trigger, "doOnlineNavigationCompactification", True) is False:
        default_key = 'HLTNav_Summary' # This can only be for Run 3(+), and is a very niche case of exporting un-compacted navigation collections for advanced debugging
    elif getattr(flags.Trigger, "doEDMVersionConversion", False) is True and getattr(flags.GeoModel, "Run", LHCPeriod.Run3) <= LHCPeriod.Run2:
        default_key = 'HLTNav_R2ToR3Summary' # This can only be for Run 2 navigation conversion.
    else:
        default_key = 'HLTNav_Summary_OnlineSlimmed' # This is the default expected key for the navigation container within the serialised HLT payload inside Run 3(+) RAW files.
    to_return = default_key
    
    for key in possible_keys:
        if key in flags.Input.Collections:
            to_return = key
            break

    msg = logging.getLogger('getRun3NavigationContainerFromInput')
    msg.info('Returning %s as the Run 3 trigger navigation collection to read in this job.', to_return)

    # Double check 'possible_keys' is kept up to date
    if to_return not in possible_keys:
        msg.error('Must add %s to the "possible_keys" array!', to_return)

    return to_return


if __name__ == '__main__':
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    import sys

    flags = initConfigFlags()
    if '--RAWRUN2' in sys.argv:
        flags.Input.Files = defaultTestFiles.RAW_RUN2
    else:
        flags.Input.Files = defaultTestFiles.AOD_RUN2_DATA
        #TODO expand the test scope Run3 AODs and RAWs

    flags.lock()
    acc = TrigDecisionToolCfg(flags)
    acc.printConfig(withDetails=True, summariseProps=True, prefix='UnitTest')
    acc.wasMerged()
    # TODO possibly add EDM printing alg using the TDT

