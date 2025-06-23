# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

## @brief Module with PAT transform options and substeps

# Get the base logger for the transforms and extend it for us
from PyJobTransforms.trfLogger import msg
msg = msg.getChild(__name__)

import PyJobTransforms.trfArgClasses as trfArgClasses

from PyJobTransforms.trfArgs import getExtraDPDList
from PyJobTransforms.trfExe import  NTUPMergeExecutor, POOLMergeExecutor
    
def addPhysValidationMergeFiles(parser):
    # TODO: Better to somehow auto-import this from PhysicsAnalysis/PhysicsValidation/PhysValMonitoring
    # Use arggroup to get these arguments in their own sub-section (of --help)
    parser.defineArgGroup('PhysValMerge', 'Physics Validation merge job specific options')
    parser.add_argument('--inputNTUP_PHYSVALFile', 
                        type=trfArgClasses.argFactory(trfArgClasses.argNTUPFile, io='input'),
                        help='Input physics validation file', group='PhysValMerge', nargs='+')
    parser.add_argument('--outputNTUP_PHYSVAL_MRGFile',
                        type=trfArgClasses.argFactory(trfArgClasses.argNTUPFile, io='output'),
                        help='Output merged physics validation file', group='PhysValMerge')
    parser.add_argument('--skipPostProcessing', 
                        action='store_true', 
                        default = False,
                        help='If given, skip the post-processing step and just do the merging',
                        group='PhysValMerge')

def addNTUPMergeSubsteps(executorSet, skip_post_processing=False):
    # Ye olde NTUPs
    intermediateStep = 'NTUP_PHYSVAL_MRG0'
    try:
        if skip_post_processing:
            msg.info("User requested to SKIP post-processing ('--skipPostProcessing' [%s]), so we'll skip running post-processing.", skip_post_processing)
            out_data = ['NTUP_PHYSVAL_MRG']
        else:
            msg.info("We'll run merging and post-processing (currently implemented only for ID track monitoring).")
            out_data = [intermediateStep]

        executorSet.add(NTUPMergeExecutor(name='NTUPLEMergePHYSVAL', exe='hadd', inData=['NTUP_PHYSVAL'], outData=out_data, exeArgs=[]))

        if not skip_post_processing:
            executorSet.add(NTUPMergeExecutor(name='NTUPLEMergePHYSVALPostProc', exe='postProcessIDPVMHistos', inData=[intermediateStep], outData=['NTUP_PHYSVAL_MRG'], exeArgs=[]))


        # Extra Tier-0 NTUPs
        extraNTUPs = getExtraDPDList(NTUPOnly = True)
        for ntup in extraNTUPs:
            executorSet.add(NTUPMergeExecutor(name='NTUPLEMerge'+ntup.name.replace('_',''), exe ='hadd', inData=[ntup.name], outData=[ntup.name+'_MRG'], exeArgs=[]))
    except ImportError as e:
        msg.warning("Failed to get D3PD lists - probably D3PDs are broken in this release: {0}".format(e))


## @brief Import list of known DAODs from the derivation framework and 
def addDAODArguments(parser, mergerTrf=True):
    DAODTypes = knownDAODTypes()
    if mergerTrf:
        parser.defineArgGroup('Input DAOD', 'Input DAOD files to be merged')
        parser.defineArgGroup('Output DAOD', 'Output merged DAOD files')
        for DAOD in DAODTypes:
            parser.add_argument("--input" + DAOD + "File", nargs="+",
                                type=trfArgClasses.argFactory(trfArgClasses.argPOOLFile, io="input", type="AOD", subtype=DAOD),
                                help="Input DAOD file of " + DAOD + " derivation", group="Input DAOD")
            parser.add_argument("--output" + DAOD + "_MRGFile", 
                                type=trfArgClasses.argFactory(trfArgClasses.argPOOLFile, io="output", type="AOD", subtype=DAOD),
                                help="Output merged DAOD file of " + DAOD + " derivation", group="Output DAOD")
    else:
        parser.defineArgGroup('Output DAOD', 'Output derivation DAOD files')
        for DAOD in DAODTypes:
            parser.add_argument("--output" + DAOD + "File", 
                                type=trfArgClasses.argFactory(trfArgClasses.argPOOLFile, io="output", type="AOD", subtype=DAOD),
                                help="Output DAOD file of " + DAOD + " derivation", group="Output DAOD")


def addDAODMergerSubsteps(executorSet):
    DAODTypes = knownDAODTypes()
    for DAOD in DAODTypes:
        executorSet.add(POOLMergeExecutor(name = DAOD.removeprefix("DAOD_") + 'Merge', inData = [DAOD], outData = [DAOD+'_MRG']))

def knownDAODTypes():
    DAODTypes = []
    try:
        from DerivationFrameworkCore.DerivationFrameworkProdFlags import listAODtoDPD
        DAODTypes = [ name.lstrip("Stream") for name in listAODtoDPD ]
    except ImportError:
        msg.warning("Could not import DAOD subtypes from DerivationFramework.DerivationFrameworkCore")
    return DAODTypes
