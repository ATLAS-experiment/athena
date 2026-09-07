#!/usr/bin/env python

# Script to run JetCalibTestAlg
#
# Example useage:
# JetCalibTestAlgConfig.py \
#   --configFile=JetCalibTools/calibConfigExample.yaml \
#   --evtMax 5 \
#   --filesInput /eos/atlas/atlascerngroupdisk/perf-jets/Hackathon/mc23_13p6TeV.830187.H7EG_H72NNPDF30NLO_jetjet_Lund_JZ1.deriv.DAOD_PHYS.e8551_s4159_r15224_p6266/DAOD_PHYS.40788428._000097.pool.root.1
#   --debugAlg
#
# --debugAlg prints the per-jet pT DEBUG messages from JetCalibTestAlg;
# use --level DEBUG instead to get DEBUG output from every component.

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from JetCalibTools.JetCalibStepsConfig import calibToolFromConfigFile

def JetCalibTestAlgCfg(flags, name="JetCalibTestAlg",
                       configFile="JetCalibTools/calibConfigExample.yaml", **kwargs):
    acc = ComponentAccumulator()

    # Configure the jet calibration tool from a YAML config file
    jetCalibTool = calibToolFromConfigFile(
        flags,
        configFile = configFile,
        name       = "JetCalibTool",
        calibSeqKey='Default',
    )

    alg = CompFactory.JetCalibTestAlg(
        name,
        JetCalibTool    = jetCalibTool,
        JetContainerKey = "AntiKt4EMPFlowJets",
        **kwargs
    )
    acc.addEventAlgo(alg)
    return acc

if __name__=='__main__':

    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO
    log.setLevel(INFO)

    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()
    # Defaults, overridable on the command line with --filesInput and --evtMax
    defaultInputFile = '/eos/atlas/atlascerngroupdisk/perf-jets/Hackathon/mc23_13p6TeV.830187.H7EG_H72NNPDF30NLO_jetjet_Lund_JZ1.deriv.DAOD_PHYS.e8551_s4159_r15224_p6266/DAOD_PHYS.40788428._000097.pool.root.1'
    flags.Input.Files = [defaultInputFile]
    flags.Exec.MaxEvents = 10

    # --configFile command-line option to specify YAML config file
    parser = flags.getArgumentParser()
    parser.add_argument("--configFile", default="JetCalibTools/calibConfigExample.yaml",
                        help="jet calibration YAML config (PathResolver or local path)")
    parser.add_argument("--debugAlg", action="store_true",
                        help="print DEBUG messages from JetCalibTestAlg only, "
                             "leaving all other components at the global output level")
    args = flags.fillFromArgs(parser=parser)
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    algName = "JetCalibTestAlg"
    alg = JetCalibTestAlgCfg(flags, name=algName, configFile=args.configFile)
    cfg.merge(alg)

    if args.debugAlg:
        msgSvc = cfg.getService("MessageSvc")
        msgSvc.setDebug = list(msgSvc.setDebug) + [algName]

    cfg.run(flags.Exec.MaxEvents)
