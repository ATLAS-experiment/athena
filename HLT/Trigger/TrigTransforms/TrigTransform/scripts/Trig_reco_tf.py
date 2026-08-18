#! /usr/bin/env python

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

## Trig_reco_tf.py
## - based on PyJobTransforms/Reco_tf.py
## Documentation on the twiki https://twiki.cern.ch/twiki/bin/viewauth/Atlas/TriggerTransform

import sys
import time

from PyJobTransforms.transform import transform
from PyJobTransforms.trfExe import athenaExecutor, DQMergeExecutor
from PyJobTransforms.trfArgs import addAthenaArguments, addDetectorArguments
from PyJobTransforms.trfDecorators import stdTrfExceptionHandler, sigUsrStackTrace
from RecJobTransforms.recTransformUtils import addCommonRecTrfArgs, addStandardRecoFiles

import PyJobTransforms.trfArgClasses as trfArgClasses

from TrigTransform.trigRecoExe import trigRecoExecutor
from TrigTransform.trigCostExe import trigCostExecutor
from TrigTransform.trigRateExe import trigRateExecutor

# Setup core logging here
from PyJobTransforms.trfLogger import msg
msg.info('logging set in %s', sys.argv[0])

@stdTrfExceptionHandler
@sigUsrStackTrace
def main():

    msg.info('This is %s', sys.argv[0])

    trf = getTransform()
    trf.parseCmdLineArgs(sys.argv[1:])
    trf.execute()
    trf.generateReport()

    msg.info("%s stopped at %s, trf exit code %d", sys.argv[0], time.asctime(), trf.exitCode)
    sys.exit(trf.exitCode)

def getTransform():
    executorSet = set()

    # BSRDOtoRAW is the HLT step of the trigger transform
    executorSet.add(trigRecoExecutor(name = 'BSRDOtoRAW',
                                     skeletonCA = 'TriggerJobOpts.runHLT',
                                     substep = 'b2r', tryDropAndReload = False,
                                     inData = ['BS_RDO', 'RDO'], outData = ['BS', 'DRAW_TRIGCOST', 'HIST_HLTMON', 'HIST_DEBUGSTREAMMON'],
                                     perfMonFile = 'ntuple_BSRDOtoRAW.pmon.gz'))

    # RAWtoCOST is the COST step for trigger transform
    # runs in athena and will succeed if input BS file has costmon enabled
    executorSet.add(trigCostExecutor(name = 'DRAWCOSTtoNTUPCOST',
                                     exe = 'RunTrigCostAnalysis.py',
                                     inData = ['DRAW_TRIGCOST'], outData = ['NTUP_TRIGCOST']))

    # AODtoNTUPRATE is the RATE step for trigger transform
    # runs in athena from an input AOD file
    executorSet.add(trigRateExecutor(name = 'AODtoNTUPRATE',
                                     exe = 'RatesAnalysisFullMenu.py',
                                     inData = ['AOD'], outData = ['NTUP_TRIGRATE']))

    # RAWtoALL, DQHistogramMerge are the reconstruction substeps for trigger transform
    # shortened list from addRecoSubsteps in RecJobTransforms.recTransformUtils
    executorSet.add(athenaExecutor(name = 'RAWtoALL',
                                   skeletonCA = 'RecJobTransforms.RAWtoALL_Skeleton',
                                   substep = 'r2a', inData = ['BS', 'RDO'],
                                   outData = ['ESD', 'AOD', 'HIST_R2A'],
                                   perfMonFile = 'ntuple_RAWtoALL.pmon.gz'))
    executorSet.add(DQMergeExecutor(name = 'DQHistogramMerge', inData = ['HIST_R2A'], outData = ['HIST']))

    # Other reco steps - not currently used in trigger reprocessings
    # if remove can also remove outputNTUP_TRIGFile

    trf = transform(executor = executorSet, description = 'Trigger transform to run HLT, followed by'
                    ' general purpose ATLAS reconstruction transform. Input to HLT is inputBS_RDOFile'
                    ' with outputs of RDO, ESD or AOD. For more details see:'
                    ' https://twiki.cern.ch/twiki/bin/viewauth/Atlas/TriggerTransform or for reco_tf, see:'
                    ' https://twiki.cern.ch/twiki/bin/viewauth/Atlas/RecoTf')

    # Add arguments
    # shortened list from RecJobTransforms.Reco_tf
    addAthenaArguments(trf.parser)
    addDetectorArguments(trf.parser)
    # shortened list from addAllRecoArgs in RecJobTransforms.recTransformUtils
    addCommonRecTrfArgs(trf.parser)
    addStandardRecoFiles(trf.parser)

    # Now add specific trigger transform arguments
    # Putting this last makes them appear last in the help so easier to find
    addTriggerArgs(trf.parser)
    addTrigCostRateArgs(trf.parser)
    addTriggerDBArgs(trf.parser)
    addDebugArgs(trf.parser)

    return trf


def addTriggerArgs(parser):
    # Use arggroup to get these arguments in their own sub-section (of --help)
    parser.defineArgGroup('Trigger', 'Specific options related to the trigger configuration used for reprocessing')

    # Arguments specific for trigger transform
    # writeBS used in literal arguments when running HLT step in athena (not athenaHLT/EF)
    parser.add_argument('--writeBS', type=trfArgClasses.argFactory(trfArgClasses.argBool, runarg=True),
                          help='Needed if running BSRDO to BS step in athena (default: True)', group='Trigger', default=trfArgClasses.argBool(True, runarg=True))
    # input BS file for the HLT step (name just to be unique identifier)
    parser.add_argument('--inputBS_RDOFile', nargs='+',
                        type=trfArgClasses.argFactory(trfArgClasses.argBSFile, io='input', runarg=True, type='bs'),
                        help='Input bytestream file', group='Trigger')
    # without an outputBSFile name specified then any further steps will know to use tmp.BS
    parser.add_argument('--outputBSFile', nargs='+',
                        type=trfArgClasses.argFactory(trfArgClasses.argBSFile, io='output', runarg=True, type='bs'),
                        help='Output bytestream file', group='Trigger')
    # select output stream in  BS file
    ## athenaHLT/EF writes All streams into one file, but this can't be proceesed by standard reco if it contains events in only PEB streams
    ## by defualt selects the Main stream, as likely the most needed option, but can ber reverted to All or any other stream chosen
    parser.add_argument('--streamSelection', nargs='+', type=trfArgClasses.argFactory(trfArgClasses.argList, runarg=True),
                        help='select output streams in produced BS file (default: \"Main\"). Specify \"All\" to disable splitting (standard reco will fail on any events with only PEB data)', group='Trigger', default=trfArgClasses.argList("Main", runarg=True))
    # HLT out histogram file, if defined renames expert-monitoring file that is produced automatically
    parser.add_argument('--outputHIST_HLTMONFile', nargs='+',
                        type=trfArgClasses.argFactory(trfArgClasses.argHISTFile, io='output', runarg=True, countable=False),
                        help='Output HLTMON file', group='Trigger')
    # Trigger Configuration String as used in reco Steps
    parser.add_argument('--triggerConfig', nargs='+', metavar='substep=TRIGGERCONFIG',
                        type=trfArgClasses.argFactory(trfArgClasses.argSubstep, runarg=True, separator='='),
                        help='Trigger Configuration String. '
                        'N.B. This argument uses EQUALS (=) to separate the substep name from the value.', group='Trigger')
    # precommand
    parser.add_argument('--precommand', nargs='+', type=trfArgClasses.argFactory(trfArgClasses.argList, runarg=True),
                        help='precommand for trigger step ("-c")', group='Trigger')
    # postcommand
    parser.add_argument('--postcommand', nargs='+', type=trfArgClasses.argFactory(trfArgClasses.argList, runarg=True),
                        help='postcommand for trigger step ("-C")', group='Trigger')

    # trigger executable
    parser.add_argument('--trigExe', type=trfArgClasses.argFactory(trfArgClasses.argString, runarg=True),
                        default=trfArgClasses.argString("athenaEF.py"),
                        help='Executable to run in the trigger step', group='Trigger')

    # For prodsys to make sure uses inputBS_RDOFile rather than inputBSFile when running the b2r step
    parser.add_argument('--prodSysBSRDO', type=trfArgClasses.argFactory(trfArgClasses.argBool, runarg=True),
                        help='For prodsys to make sure uses inputBS_RDOFile rather than inputBSFile when running the b2r step', group='Trigger')


def addTrigCostRateArgs(parser):
    # Use arggroup to get these arguments in their own sub-section (of --help)
    parser.defineArgGroup('TrigCost', 'Specific options related to the trigger cost and rates steps in trigger reprocessings')

    # without a outputDRAW_TRIGCOSTFile name specified then it will not be possible to run any further COST analysis if the BS is slimmed to a specific stream
    parser.add_argument('--outputDRAW_TRIGCOSTFile', nargs='+',
                        type=trfArgClasses.argFactory(trfArgClasses.argBSFile, io='output', runarg=True),
                        help='Output bytestream file of CostMonitoring stream', group='TrigCost')
    # input BS file for the TRIGCOST step (name just to be unique identifier for prodSys)
    parser.add_argument('--inputDRAW_TRIGCOSTFile', nargs='+',
                        type=trfArgClasses.argFactory(trfArgClasses.argBSFile, io='input', runarg=True),
                        help='Input bytestream file of CostMonitoring stream', group='TrigCost')
    # NTUP_COST is used for COST monitoring - used in the reco release
    parser.add_argument('--outputNTUP_TRIGCOSTFile', nargs='+',
                        type=trfArgClasses.argFactory(trfArgClasses.argHISTFile, io='output', runarg=True, countable=False),
                        help='D3PD output NTUP_TRIGCOST file', group='TrigCost')
    # NTUP_RATE is used for COST monitoring - used in the reco release
    parser.add_argument('--outputNTUP_TRIGRATEFile', nargs='+',
                        type=trfArgClasses.argFactory(trfArgClasses.argHISTFile, io='output', runarg=True, countable=False),
                        help='D3PD output NTUP_TRIGRATE file', group='TrigCost')

    # Additional cost arguments for cost processing step
    parser.add_argument('--costopts', nargs='+',
                        type=trfArgClasses.argFactory(trfArgClasses.argSubstepList, splitter=' ', runarg=False),
                        help='Extra options to pass to cost processing.', group='TrigCost')

    # Additional rate arguments for rates analysis step
    parser.add_argument('--rateopts', nargs='+',
                        type=trfArgClasses.argFactory(trfArgClasses.argSubstepList, splitter=' ', runarg=False),
                        help='Extra options to pass to rates analysis.', group='TrigCost')

def addTriggerDBArgs(parser):
    # Use arggroup to get these arguments in their own sub-section (of --help)
    parser.defineArgGroup('TriggerDB', 'Specific options related to the trigger DB')

    parser.add_argument('--useDB', type=trfArgClasses.argFactory(trfArgClasses.argBool, runarg=True),
                        help='read from DB', group='TriggerDB')
    parser.add_argument('--DBserver', type=trfArgClasses.argFactory(trfArgClasses.argString, runarg=True),
                        help='DB name', group='TriggerDB')
    parser.add_argument('--DBsmkey', type=trfArgClasses.argFactory(trfArgClasses.argString, runarg=True),
                        help='DB SMK', group='TriggerDB')
    parser.add_argument('--DBhltpskey', type=trfArgClasses.argFactory(trfArgClasses.argString, runarg=True),
                        help='DB hltpskey', group='TriggerDB')
    parser.add_argument('--DBl1pskey', type=trfArgClasses.argFactory(trfArgClasses.argString, runarg=True),
                        help='DB l1pskey', group='TriggerDB')


def addDebugArgs(parser):
    # Use arggroup to get these arguments in their own sub-section (of --help)
    parser.defineArgGroup('Debug', 'Specific options related to the trigger debug recovery')

    parser.add_argument('--outputHIST_DEBUGSTREAMMONFile', nargs='+',
                        type=trfArgClasses.argFactory(trfArgClasses.argHISTFile, io='output', runarg=True, countable=False),
                        help='Output DEBUGSTREAMMON file', group='Debug')


if __name__ == '__main__':
    main()
