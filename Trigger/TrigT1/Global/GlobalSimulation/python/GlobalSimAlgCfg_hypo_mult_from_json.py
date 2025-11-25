# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Set up the Algoririthm GlobalSimulation with tools to read
# eFex RoIs, and to run a number of isnstances of eEmMultAlgTool
#

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from AthenaCommon.Logging import logging
logger = logging.getLogger(__name__)
from AthenaCommon.Constants import DEBUG

from eEmTipWriterParams_from_json import eEmTipWriterParams_from_json

def GlobalSimulationAlgCfg(flags,
                           name="GlobalSimHypoMult",
                           OutputLevel=DEBUG,
                           dump=False):

    logger.setLevel(OutputLevel)

    cfg = ComponentAccumulator()


    param_dicts = eEmTipWriterParams_from_json(OutputLevel)
    tipwriter_tools = []
    itool = 0
    for params in param_dicts:
        tool = CompFactory.GlobalSim.eEmMultAlgTool(
            'eEmMultAlgTool_'+str(itool))
        
        itool += 1

        tool.TIPposition = params['startbit']
        tool.n_multbits = params['nbits']        
        tool.et_low = str(params['etmin'])
        tool.eta_low = str(params['etaMin'])
        tool.eta_high = str(params['etaMax'])
        tool.rhad = str(params['rhad'])
        tool.rhad_op = str(params['rhad_op'])
        tool.reta = str(params['rhad'])
        tool.reta_op = str(params['rhad_op'])
        tool.wstot = str(params['rhad'])
        tool.wstot_op = str(params['rhad_op'])
        tool.menu_name = str(params['name'])
        

        tool.OutputLevel = OutputLevel

        tipwriter_tools.append(tool)

    tool1 =  CompFactory.GlobalSim.eFexCvtrAlgTool('eFexCvtrAlgTool')
    tool1.OutputLevel = OutputLevel
    tool1.eFexEMRoIKey = 'L1_eEMxRoI'

    alg = CompFactory.GlobalSim.GlobalSimulationAlg(name + 'Alg')
    alg.globalsim_algs = [tool1]
    alg.TIPwriters = tipwriter_tools
    alg.enableDumps = dump
    alg.OutputLevel = OutputLevel
       
    cfg.addEventAlgo(alg)
    
    return cfg
