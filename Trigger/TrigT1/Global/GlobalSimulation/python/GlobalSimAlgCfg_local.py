# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration


#
# Configure a GlobalSim Algorithm from an XML file
#
# Here, configuration means load the Algorithm with configured
# AlgTools.
#
# The configuration XML file specifies each AlgTool's immediate
# data provider. This python module uses this information to build
# A directed acyclic graph (DAG), to assign data handle keys in such a way
# that the DAG is implemented.
#
# The GlobalSim Algorithm has two types of AlgTools: TOBwriters and
# TIPwriters. TOBWriters read in and write out various TOB types
#
# TIPwriters read in TOBS, and are used to update the TIP word, which is a
# bitset which is  sent to the CTP.
#
# For now, we assume we can run all the TIPwriter tools after we
# have run all the TOBwriter tools.
#

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from AthenaCommon.Logging import logging
logger = logging.getLogger(__name__)
from AthenaCommon.Constants import DEBUG

from GlobalSimulation.Digraph import Digraph
from  GlobalSimulation.graphAlgs import Topological

import xml.etree.ElementTree as ET
import os

def GlobalSimulationAlgCfg(flags,
                           dump,
                           algName = 'GlobalSimTestAlg',
                           OutputLevel=DEBUG):

    logger.setLevel(OutputLevel)
    cfg = ComponentAccumulator()
    
    fn = os.environ.get('GS_CFG_FILE', None)
    if fn is None:
        logger.error('Please set export environment variable GS_CFG_FILE'\
                     'with the name of a GloblSim config xml file')


    logger.info('GlobalSim local config, cfg file:' + fn)

   
    def str_id(toolEl):
        """ obtain a string id for each AlgTool"""
        
        a_class = toolEl.attrib['class']
        a_name = toolEl.attrib['name']
        return  '/'.join((a_class, a_name))


    def configure_algtool(toolEl):
        a_class = toolEl.attrib['class']
        a_name = toolEl.attrib['name']
        prop_names = []
        factory = getattr(CompFactory.GlobalSim, a_class)
        tool =  factory(a_name)

        type_factories = {'int': int,
                          'float': float,
                          'str': str}
        
        for prop in toolEl.iter('property'):
            name = prop.attrib['name']
            value = prop.attrib['value']
            ptype = prop.attrib.get("type", None)
            if ptype is not None:
                value = type_factories[ptype](value)
            setattr(tool, name, value)
            prop_names.append(name)

        logger.debug('configure_algtool: ' + str(tool))
        return tool

    
    def fill_alg_ids(root):
        """
        Assign an index to each AlgTool instance specified by
        the configuration file.

        Return this information in a dictionary.
        """

        alg_ids = {}
        alg_ind = 1

        alg_tools = {}

        for toolType in  ('TOBWriters', 'TIPWriters'):
            for writerEl in root.iter(toolType): 
                for toolEl in writerEl.iter('AlgTool'):
                    f_name = str_id(toolEl)                    
                    if f_name in alg_ids:
                        AssertionError('Algorithm duplicated in ' + fn)
                    alg_ids[f_name] = alg_ind
                    alg_tools[alg_ind] = (configure_algtool(toolEl), toolType)
                    alg_ind += 1
        return alg_ids, alg_tools, alg_ind


    def make_digraph(alg_ids, V):
        """
        Construct an Algtool Digraph.

        Obtain parent child relations from the config XML file. 

        The graph knows only about
        the AlgTool insances's indices, and so works from a
        dictionary that associates the AlgoTool name (string) to its

        ineger index.
        """

        
        logger.debug('make_digraph: ', alg_ids)
        logger.debug('make_digraph:  V ' + str(V))

        # Create an empty DAG
        G = Digraph(V)


        for toolType in  ('TOBWriters', 'TIPWriters'):

            for writerEl in root.iter(toolType): 
                for toolEl in writerEl.iter('AlgTool'):
                    f_name = str_id(toolEl)
                    logger.debug('make_digraph:  toolType ' + toolType +
                                 ' ' + f_name)


                    par_id = alg_ids[f_name]
                
                    for childEl in toolEl.iter('child'):
                        f_c_name = str_id(childEl)
                        if f_c_name not in alg_ids:
                            AssertionError('child ' + f_c_name +
                                           ' not in ' + fn)

                        G.addEdge(par_id, alg_ids[f_c_name])

        R = G.reverse()
        roots = [n for n in range(R.V) if not R.adj(n) and n != 0]
        return G, roots




    # parse the config XML file
    
    tree = ET.parse(fn)
    root = tree.getroot()

    # extract tool information from the XML file
    # alg_ids: str : int
    # alg_tools: int : (tool, toolType), toolType is a string
    # V number of vertices (including unused root vertex = 0
    alg_ids,  alg_tools, V= fill_alg_ids(root)

    G, roots = make_digraph(alg_ids, V)
    topological = Topological(G, roots=roots)
    assert topological.isDAG()
    index_order = topological.order()

    logger.debug('DAG: ' + str(G))
    logger.debug('order: ' + str(index_order))


    toolType = 'TOBWriters'
    orderedTOBWriters = [alg_tools[i][0] for i in index_order
                         if alg_tools[i][1] == toolType]

    msg = [str(tool) for tool in orderedTOBWriters]
    logger.debug(toolType + ': ' + '\n'.join(msg))
    
    toolType = 'TIPWriters'
    orderedTIPWriters = [alg_tools[i][0] for i in index_order
                         if alg_tools[i][1] == toolType]

    msg = [str(tool) for tool in orderedTIPWriters]
    logger.debug(toolType + ': ' + '\n'.join(msg))
  
    alg = CompFactory.GlobalSim.GlobalSimulationAlg(algName)
    alg.globalsim_algs = orderedTOBWriters
    alg.TIPwriters = orderedTIPWriters
    alg.OutputLevel = OutputLevel
    alg.enableDumps = dump

    
    from TrigCaloRec.TrigCaloRecConfig import hltCaloCellSeedlessMakerCfg
    cfg.merge(hltCaloCellSeedlessMakerCfg(flags, roisKey=''))

    cfg.addEventAlgo(alg)
    return cfg
