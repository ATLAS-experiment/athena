# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

import xml.etree.ElementTree as ET

from AthenaCommon.Logging import logging
logger = logging.getLogger(__name__)
from AthenaCommon.Constants import DEBUG

def GlobalSimulationAlgCfg(flags,
                           fn,
                           dump,
                           algName = 'GlobalSimTestAlg',
                           OutputLevel=DEBUG):

    logger.setLevel(OutputLevel)
    logger.info('GlobalSim local config, cfg file:'+ fn)

    cfg = ComponentAccumulator()

   
    tree = ET.parse(fn)
    root = tree.getroot()

    def fillTools(label='TOBWriters'):
        tools = []
        for writer in root.iter(label):
            for toolEl in writer.iter('AlgTool'):
                prop_names = []
                factory = getattr(CompFactory.GlobalSim,
                                  toolEl.find('class').text)
                tool =  factory(toolEl.find('name').text)
                for prop in toolEl.iter('property'):
                    name = prop.attrib['name']
                    value = prop.attrib['value']
                    setattr(tool, name, value)
                    prop_names.append(name)
                    
                tools.append(tool)
        return tools

    
    alg = CompFactory.GlobalSim.GlobalSimulationAlg(algName)
    alg.globalsim_algs = fillTools('TOBWriters')
    alg.TIPwriters = fillTools('TIPWriters')
    alg.OutputLevel = OutputLevel
    alg.enableDumps = dump
    
    cfg.addEventAlgo(alg)
    return cfg
