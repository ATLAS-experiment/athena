# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

## @brief Module with standard reconstruction transform options and substeps

import logging
msg = logging.getLogger(__name__)

import PyJobTransforms.trfArgClasses as trfArgClasses

from PyJobTransforms.trfExe import athenaExecutor, dummyExecutor, DQMergeExecutor, reductionFrameworkExecutor, reductionFrameworkExecutorNTUP
from PyJobTransforms.trfArgs import addPrimaryDPDArguments, addExtraDPDTypes, addReductionArguments


def addCommonRecTrfArgs(parser):
    parser.defineArgGroup('Common Reco', 'Common Reconstruction Options')
    parser.add_argument('--autoConfiguration', group='Common Reco', type=trfArgClasses.argFactory(trfArgClasses.argList), 
                        help='Autoconfiguration settings (whitespace or comma separated)', nargs='+', metavar='AUTOCONFKEY')
    parser.add_argument('--trigStream', group='Common Reco', type=trfArgClasses.argFactory(trfArgClasses.argList), 
                        help='Trigger stream setting')
    parser.add_argument('--topOptions', group='Common Reco', type=trfArgClasses.argFactory(trfArgClasses.argSubstep), 
                        nargs="+", help='Alternative top options file for reconstruction (can be substep specific)', 
                        metavar="substep:TOPOPTIONS")
    parser.add_argument('--valid', group='Common Reco', type=trfArgClasses.argFactory(trfArgClasses.argBool), 
                        help='Enable decorations for AOD that allow for enhanced physics validation', metavar='BOOL')


def addStandardRecoFiles(parser):
    parser.defineArgGroup('Reco Files', 'Reconstruction file options')
    parser.add_argument('--inputBSFile', nargs='+', 
                        type=trfArgClasses.argFactory(trfArgClasses.argBSFile, io='input'),
                        help='Input bytestream file', group='Reco Files')
    parser.add_argument('--inputDRAW_ZMUMUFile', nargs='+', 
                        type=trfArgClasses.argFactory(trfArgClasses.argBSFile, io='input'),
                        help='Input skimmed Z->mumu bytestream', group='Reco Files')
    parser.add_argument('--inputDRAW_ZEEFile', nargs='+', 
                        type=trfArgClasses.argFactory(trfArgClasses.argBSFile, io='input'),
                        help='Input skimmed Z->ee bytestream', group='Reco Files')
    parser.add_argument('--inputDRAW_EMUFile', nargs='+', 
                        type=trfArgClasses.argFactory(trfArgClasses.argBSFile, io='input'),
                        help='Input skimmed e+mu bytestream', group='Reco Files')
    parser.add_argument('--outputBSFile', 
                        type=trfArgClasses.argFactory(trfArgClasses.argBSFile, io='output'),
                        help='Output bytestream file', group='Reco Files')
    parser.add_argument('--inputRDOFile', nargs='+', 
                        type=trfArgClasses.argFactory(trfArgClasses.argRDOFile, io='input'),
                        help='Input RDO file', group='Reco Files')
    parser.add_argument('--inputESDFile', nargs='+', 
                        type=trfArgClasses.argFactory(trfArgClasses.argPOOLFile, io='input'),
                        help='Input ESD file', group='Reco Files')
    parser.add_argument('--outputESDFile', 
                        type=trfArgClasses.argFactory(trfArgClasses.argPOOLFile, io='output'),
                        help='Output ESD file', group='Reco Files')
    parser.add_argument('--outputRDO_TRIGFile', 
                        type=trfArgClasses.argFactory(trfArgClasses.argRDOFile, io='output'),
                        help='Output RDO_TRIG file', group='Reco Files')
    parser.add_argument('--inputRDO_TRIGFile', 
                        type=trfArgClasses.argFactory(trfArgClasses.argRDOFile, io='input'),
                        help='Input RDO_TRIG file', group='Reco Files')
    parser.add_argument('--inputAODFile', nargs='+', 
                        type=trfArgClasses.argFactory(trfArgClasses.argPOOLFile, io='input'),
                        help='Input AOD file', group='Reco Files')
    parser.add_argument('--outputAODFile', 
                        type=trfArgClasses.argFactory(trfArgClasses.argPOOLFile, io='output'),
                        help='Output AOD file', group='Reco Files')
    parser.add_argument('--outputAOD_RPRFile', 
                        type=trfArgClasses.argFactory(trfArgClasses.argPOOLFile, io='output'),
                        help='Output AOD (reprocessed) file', group='Reco Files')
    parser.add_argument('--outputAOD_SKIMFile', 
                        type=trfArgClasses.argFactory(trfArgClasses.argPOOLFile, io='output'),
                        help='Output skimmed AOD file', group='Reco Files')
    parser.add_argument('--outputHISTFile', 
                        type=trfArgClasses.argFactory(trfArgClasses.argHISTFile, io='output'), 
                        help='Output DQ monitoring file', group='Reco Files')
    parser.add_argument('--outputHIST_AODFile', 
                        type=trfArgClasses.argFactory(trfArgClasses.argHISTFile, io='output', countable=False), 
                        help='Output DQ monitoring file', group='Reco Files')
    parser.add_argument('--outputTAGFile', 
                        type=trfArgClasses.argFactory(trfArgClasses.argFile, io='output'), 
                        help='Output TAG file', group='Reco Files')
    parser.add_argument('--inputEVNTFile', nargs='+', 
                        type=trfArgClasses.argFactory(trfArgClasses.argPOOLFile, io='input'),
                        help='Input EVNT file for NTUP_TRUTH making', group='Reco Files')
    parser.add_argument('--outputTXT_JIVEXMLTGZFile',
                        type = trfArgClasses.argFactory(trfArgClasses.argFile, io = 'output'),
                        help = 'Output JiveXML.tgz file', group = 'Reco Files')
    parser.add_argument('--outputDAOD_TLAFile', nargs='+',
                        type=trfArgClasses.argFactory(trfArgClasses.argPOOLFile, io='output'),
                        help='Output DAOD_TLA file', group='Reco Files')
    parser.add_argument('--outputDAOD_TLAFTAGPEBFile', nargs='+',
                        type=trfArgClasses.argFactory(trfArgClasses.argPOOLFile, io='output'),
                        help='Output DAOD_TLAFTAGPEB file', group='Reco Files')
    parser.add_argument('--outputDAOD_TLAEGAMPEBFile', nargs='+',
                        type=trfArgClasses.argFactory(trfArgClasses.argPOOLFile, io='output'),
                        help='Output DAOD_TLAEGAMPEB file', group='Reco Files')

## @brief Add reconstruction substeps to a set object
#  @note This is done in a separate function so that other transforms (full chain ones)
#  can import these steps easily
def addRecoSubsteps(executorSet):
    executorSet.add(athenaExecutor(name = 'RDOtoBS', skeletonFile = 'RecJobTransforms/skeleton.RDOtoBS_tf.py',
                                   substep = 'r2b', inData = ['RDO'], outData = ['BS']))
    executorSet.add(athenaExecutor(name = 'RDOtoRDOTrigger', skeletonFile = 'RecJobTransforms/skeleton.RDOtoRDOtrigger.py',
                                   skeletonCA = 'RecJobTransforms.RDOtoRDO_TRIG_Skeleton',
                                   substep = 'r2t', inData = ['RDO'], outData = ['RDO_TRIG']))
    executorSet.add(athenaExecutor(name = 'RAWtoALL', skeletonFile = 'RecJobTransforms/skeleton.RAWtoALL_tf.py',
                                   skeletonCA = 'RecJobTransforms.RAWtoALL_Skeleton',
                                   substep = 'r2a', inData = ['BS', 'RDO', 'DRAW_ZMUMU', 'DRAW_ZEE', 'DRAW_EMU', 'DRAW_RPVLL'], 
                                   outData = ['ESD', 'AOD', 'HIST_R2A', 'TXT_JIVEXMLTGZ'],))
    executorSet.add(athenaExecutor(name = 'RAWtoDAODTLA', skeletonCA = 'RecJobTransforms.RAWtoDAOD_TLA_Skeleton',
                                   substep = 'r2tla', inData = ['BS'], outData = ['DAOD_TLA'], ))
    executorSet.add(athenaExecutor(name = 'RAWtoDAODTLAFTAGPEB', skeletonCA = 'RecJobTransforms.RAWtoDAOD_TLA_Skeleton',
                                   substep = 'r2TLAFTAGPEB', inData = ['BS'], outData = ['DAOD_TLAFTAGPEB'], ))
    executorSet.add(athenaExecutor(name = 'RAWtoDAODTLAEGAMPEB', skeletonCA = 'RecJobTransforms.RAWtoDAOD_TLA_Skeleton',
                                   substep = 'r2TLAEGAMPEB', inData = ['BS'], outData = ['DAOD_TLAEGAMPEB'], ))
    executorSet.add(athenaExecutor(name = 'RAWtoESD', skeletonFile = 'RecJobTransforms/skeleton.RAWtoESD_tf.py',
                                   substep = 'r2e', inData = [], outData = [],))
    executorSet.add(athenaExecutor(name = 'ESDtoAOD', skeletonFile = 'RecJobTransforms/skeleton.ESDtoAOD_tf.py',
                                   substep = 'e2a', inData = ['ESD'], outData = ['AOD', 'HIST_AOD_INT']))
    executorSet.add(DQMergeExecutor(name = 'DQHistogramMerge', inData = [('HIST_ESD_INT', 'HIST_AOD_INT'), 'HIST_R2A', 'HIST_AOD'], outData = ['HIST']))
    executorSet.add(athenaExecutor(name = 'ESDtoDPD', skeletonFile = 'PATJobTransforms/skeleton.ESDtoDPD_tf.py',
                                   substep = 'e2d', inData = ['ESD'], outData = []))
    executorSet.add(athenaExecutor(name = 'AODtoDPD', skeletonFile = 'PATJobTransforms/skeleton.AODtoDPD_tf.py',
                                   substep = 'a2d', inData = ['AOD', 'EVNT'], outData = []))
    executorSet.add(athenaExecutor(name = 'AODtoAOD', skeletonFile = 'RecJobTransforms/skeleton.AODtoAOD_tf.py',
                                   substep = 'a2a', inData = ['AOD'], outData = ['AOD_RPR'],
                                   perfMonFile = None))
    executorSet.add(dummyExecutor(name = 'AODtoTAG',
                                   inData = ['AOD', 'AOD_RPR'], outData = ['TAG'],))
    executorSet.add(athenaExecutor(name = 'AODtoHIST', skeletonFile = 'RecJobTransforms/skeleton.AODtoHIST_tf.py', skeletonCA = 'RecJobTransforms.AODtoHIST_Skeleton',
                                   substep = 'a2h', inData = ['AOD'], outData = ['HIST_AOD'],))
    executorSet.add(reductionFrameworkExecutor(name = 'AODtoDAOD', skeletonFile = 'PATJobTransforms/skeleton.AODtoDAOD_tf.py',
                                   substep = 'a2da', inData = ['AOD'], outData = ['DAOD']))
    executorSet.add(reductionFrameworkExecutorNTUP(name = 'NTUPtoRED', skeletonFile = 'PATJobTransforms/skeleton.NTUPtoRED_tf.py',
                                   substep = 'n2n', inData = ['NTUP_COMMON'], outData = ['DNTUP']))
    executorSet.add(reductionFrameworkExecutor(name = 'EVNTtoDAOD', skeletonFile = 'PATJobTransforms/skeleton.AODtoDAOD_tf.py',
                                   substep = 'ev2da', inData = ['EVNT'], outData = ['DAOD']))
    executorSet.add(reductionFrameworkExecutor(name = 'DAODtoD2AOD', skeletonFile = 'PATJobTransforms/skeleton.DAODtoD2AOD_tf.py',
                                   substep = 'da2d2a', inData = ['DAOD_PHYS'], outData = ['D2AOD']))



## @brief The standard suite of reconstruction specific arguments
#  @param trf The transform to which these arguments should be added
def addAllRecoArgs(trf):
    addCommonRecTrfArgs(trf.parser)
    addStandardRecoFiles(trf.parser)
    addPrimaryDPDArguments(trf.parser, transform = trf)
    addExtraDPDTypes(trf.parser, transform = trf)
    addReductionArguments(trf.parser, transform = trf)
