# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from TrigValTools.TrigValSteering import ExecStep
from AthenaConfiguration.TestDefaults import defaultConditionsTags

def prepare_acts_rdo2aod(pipeline : str):
    _args = [
        'flags.Trigger.triggerMenuSetup=\'MC_pp_run4_v1\'',
        'flags.Trigger.AODEDMSet=\'AODFULL\'',
        'flags.Tracking.doPixelDigitalClustering=True',
        'from TrkConfig.TrkConfigFlags import TrackingComponent',
        'flags.Tracking.recoChain = [TrackingComponent.ActsChain]'
    ]

    if pipeline == "C230":
        _args.append('from ActsConfig.ActsConfigFlags import SeedingStrategy')
        _args.append('flags.Acts.SeedingStrategy=SeedingStrategy.Gbts2')

    preExec = ';'.join(_args)

    rdo2aod = ExecStep.ExecStep()
    rdo2aod.type = 'Reco_tf'
    rdo2aod.input = 'ttbar_pu200_Run4'
    rdo2aod.threads = 8
    rdo2aod.max_events = 100
    rdo2aod.args = '--outputAODFile=AOD.pool.root --steering "doRDO_TRIG"'
    rdo2aod.args += ' --CA "all:True"'
    rdo2aod.args += ' --perfmon fullmonmt'
    rdo2aod.args += ' --preExec "all:{:s};"'.format(preExec)
    rdo2aod.args += ' --preInclude "RAWtoALL:ActsConfig.ActsCIFlags.actsWorkflowFlags"'
    rdo2aod.args += ' --conditionsTag f"default:{defaultConditionsTags.RUN4_MC}"'
    rdo2aod.args += ' --ignorePatterns "Propagation.+reached.+the.+step.+count.+limit,Propagation.+failed:.+PropagatorError:..+Propagation.+reached.+the.+configured.+maximum.+number.+of.+steps.+with.+the.+initial.+parameters"'
    rdo2aod.timeout = 5400 # default = 3600 s
    rdo2aod.flags = ['Trigger.useActsTracking=True',
                     'Acts.useCache=True',
                     'Acts.doAmbiguityResolution=False',
                     'Acts.GsfRefitActs=True',
                     'Acts.GsfDirectNavigation=True',
                     'Tracking.doITkFastTracking=True',
                     'Trigger.doRuntimeNaviVal=True',
                     'ITk.doTruth=False',
                     'Tracking.doTruth=False',
                     f'IOVDb.GlobalTag=\'{defaultConditionsTags.RUN4_MC}\'',
                     ]

    return rdo2aod
