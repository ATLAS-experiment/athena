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

#  Data structures
#  ----------------
#
# alg_ida: dictionary str:int keys are AlgNames: class/instance name.
#
# G: A digraph providing  parent child relations. Nodes are int alg ids
#
# read_handles: dictionary str: (str: str).
#  Outer dictionary key: AlgTool class name
#  Inner dictionary key: standardised read handle name used by config file
#  value: python name of the read handle.
#
# write_handles: dictionary str:str key: AlgTool class name.
# Value: python name of the write handle. Wa allow only one write handle
#    For GlobalSim connections.
#
# alg_tools: dictionary {str : int} key = alg full name, int = alg_id
#
# input_slots: dict{int: dict{int, str}} outter dict key:parent int id
#     inner dict key: child int id innner dict value: generic slot str eg 'in9'
       
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from AthenaCommon.Logging import logging
logger = logging.getLogger(__name__)
from AthenaCommon import Constants

from GlobalSimulation.Digraph import Digraph
from  GlobalSimulation.graphAlgs import Topological

from PathResolver import PathResolver

import xml.etree.ElementTree as ET
import os
from collections import defaultdict

# The following DataHandle look up tables will be removed in
# future developments.

#The entry to the outer dictiones is the name
# of a GlobalSim AlgTool
#
# for read handles, the value is itself a dictionary with the key being
# the giving the name referred to by the configuratioh file, and the
# value of the inner dictionary begin the python name of the read handle.
# This mecahnism removes the previous existing limits of the number
# of child AlgTools a parent AlgTool may have.

read_handles = {
    'eFexCvtrAlgTool': {'in0': 'eFexEMRoIKey'},
    'gFexRhoCvtrAlgTool': {'in0': 'gFexJetRoIKey'},
    'Egamma1BDTAlgTool': {'in0': 'LArNeighborhoodTOBContainerKey'},
    'GlobalCellTowerAlgTool': {'in0': 'GlobalLArCellsKey'},
    'GlobalJet1AlgTool': {'in0': 'GlobalCellTowersKey'},
    'GlobalMETAlgTool': {'in0': 'GlobalCellTowersKey', #NOTE MET has two inputs (unlike existing algorithms)
                         'in1': 'GlobalJet1JetsKey'},
    'eEmMultAlgTool': {'in0': 'eEmTOBs'},
    'eEmEg1BDTMultAlgTool': {'in0': 'eEmEg1BDTTOBContainerKey'},
    'CommonMultAlgTool': {'in0': 'CommonTOBsKey'},
    }

write_handles = {
    'eFexCvtrAlgTool': 'eEmTOBContainerKey',
    'gFexRhoCvtrAlgTool': 'gFexRhoTOBContainerKey',
    'Egamma1BDTAlgTool': 'eEmEg1BDTTOBContainerKey',
    'GlobalCellTowerAlgTool': 'GlobalCellTowersKey',
    'GlobalJet1AlgTool': 'GlobalJet1JetsKey',
    'GlobalMETAlgTool': 'GlobalMETKey',
}

def GlobalSimulationAlgCfg(flags,
                           dump=False,
                           fn=None,
                           algName='GlobalSimTestAlg',
                           OutputLevel=Constants.INFO):

    logger.setLevel(OutputLevel)
    cfg = ComponentAccumulator()

    fn = os.environ.get('GS_CFG_FILE', None)
    if fn is not None:
        if not os.path.exists(fn):
            raise RuntimeError ('specified cfg file ' +  fn + ' does not exist')
    else:
        def_fn = "GlobalSimulation/globalSim_AllChainsCfg.xml"
        logger.info('environment variable GS_CFG_FILE not set ' +
                    'looking for default config file'+  def_fn)
        fn = PathResolver.FindCalibFile(def_fn)
        if not fn:
            logger.info ('could not find default cfg file ' + def_fn +
                         'giving up')
            raise RuntimeError ('default cfg file ' +  def_fn + ' not found')
 
    logger.info('GlobalSim local config, cfg file: ' + fn)

    def str_id(toolEl):
        """ obtain a string id for each AlgTool"""
        
        a_class = toolEl.attrib['class']
        a_name = toolEl.attrib['name']
        return  '/'.join((a_class, a_name))

    def classname_from_fullname(fullname):
        return fullname.split('/')[0]


    def configure_algtool(toolEl):
        """
        Set the AlgTool properties from configure file information.
        Datahandles are not processed here.
        """
        
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
                        raise AssertionError('Algorithm duplicated in ' + fn)
                    alg_ids[f_name] = alg_ind
                    alg_tools[alg_ind] = (configure_algtool(toolEl), toolType)
                    alg_ind += 1
        return alg_ids, alg_tools, alg_ind

    def fill_input_slots(root, alg_ids):
        """
        Create a dictionary
        {par_alg_id:int ||  {input_slot:str ||  child_alg_id:int}}

        Where is a generic name for the input location, eg "in0", and
        is used by the config file. The actual location is
        currently obtained using the read_handles dictionary at the top
        of this file.
        """

        input_slots = defaultdict(dict)
        
        for toolEl in root.iter('AlgTool'):
            par_full_name = str_id(toolEl)
            par_id = alg_ids[par_full_name]
            
            for childEl in toolEl.iter('child'):
                child_full_name = str_id(childEl)
                child_id = alg_ids[child_full_name]
                slot = childEl.attrib.get('slot', None)
                if slot is None:
                    msg = ['No slot information for ',
                           par_full_name,
                           ' child ',
                           child_full_name]
                    raise AssertionError(' '.join(msg))

                input_slots[par_id][child_id] = slot

        return input_slots
                
            
    def make_digraph(alg_ids, V):
        """
        Construct an Algtool Digraph.

        Obtain parent child relations from the config XML file. 

        The graph knows only about
        the AlgTool insances's indices, and so works from a
        dictionary that associates the AlgoTool name (string) to its

        ineger index.
        """

        
        logger.debug('make_digraph alg_ids: ', alg_ids)
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
                            raise AssertionError('child ' + f_c_name +
                                                 ' not in ' + fn)

                        G.addEdge(par_id, alg_ids[f_c_name])

        R = G.reverse()
        roots = [n for n in range(R.V) if not R.adj(n) and n != 0]
        return G, roots

    def set_SGout_locations(tools):
        """
        Set the StoreGate locations to be written to. As the
        same Algorithm may have > 1 instance, ensure that the
        write locations differ.
       """
        # set the Storegate location each tool writes to.
        
        out_index = 0
        for indx, (tool, tooltype)  in tools.items():
            class_name = tool.__class__.__name__
            handle = write_handles.get(class_name, None)

            if handle is not None:
                setattr(tool, handle, 'GlobalSim_'+str(out_index))
                out_index += 1
 

    def set_SGin_locations(tools, alg_ids, input_slots, G):
        """"

        Set locations read from by each Algorithm according to the
        call graph G.
        
        NOTE: currently we assume a tool has one output location
        and one input location, which allows only "narrow chains".
        This will be extended to allow multiple children in the near future.
        
        alg_ids is a str:int map
        tools is a int : (tool, toolType) map
        """

        for nid in range(1, G.V):
            parent = tools[nid][0]
            child_ids =  G.adj(nid)
            if len(child_ids) == 0:
                continue

            for child_id  in  child_ids:
                slot = input_slots[nid][child_id] # eg 'in0'
                child_tool = tools[child_id][0] # tools.values: (tool, tooltype)
                w_handle_name = write_handles[child_tool.__class__.__name__]
                read_handle = read_handles[parent.__class__.__name__][slot]
                read_from = getattr(child_tool, w_handle_name)
                setattr(parent, read_handle, read_from)

               
    # parse the config XML file
    
    tree = ET.parse(fn)
    root = tree.getroot()

    # extract tool information from the XML file
    # alg_ids: str : int
    # alg_tools: int : (tool, toolType), toolType is a string
    # V number of vertices (including unused root vertex = 0
    alg_ids,  alg_tools, V= fill_alg_ids(root)
    input_slots = fill_input_slots(root, alg_ids)
    G, roots = make_digraph(alg_ids, V)
    logger.debug('call graph ' + str(G))

    topological = Topological(G, roots=roots)
    if not topological.isDAG(): raise AssertionError(
            'Call graph is not a DAG')
    
    index_order = topological.order()
    set_SGout_locations(tools=alg_tools)
    set_SGin_locations(tools=alg_tools, alg_ids = alg_ids,
                       input_slots=input_slots, G=G)

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

    tools = [alg_tools[i][0] for i in index_order]
    msg = ['GlobalSim tool IO dump:']
    for tool in tools:
        
        tname =  tool.__class__.__name__ + '/' + tool.name

        logger.debug('GS tool name ' + tname)
        logger.debug('GS r_handle str(tool) ' + str(tool))

        handle_name = read_handles.get(tool.__class__.__name__, None)
        if handle_name is None:
            logger.debug('GS r_handle not in table')
        else:
                  
            logger.debug('GS r_handle from table: ', handle_name)
            logger.debug('GS r_handle loc from tool: ' + tname + ' ' +
                         str(getattr(tool, handle_name['in0'])))
        
        handle_name = write_handles.get(tool.__class__.__name__, None)
        if handle_name is None:
            logger.debug('GS w_handle not in table')
        else:
            logger.debug('GS w_handle from table: ' + handle_name)
            logger.debug('GS w_handle loc from tool: ' + tname + ' ' +
                         str(getattr(tool, handle_name)))


    alg = CompFactory.GlobalSim.GlobalSimulationAlg(algName)
    alg.globalsim_algs = orderedTOBWriters
    alg.TIPwriters = orderedTIPWriters
    alg.OutputLevel = OutputLevel
    alg.enableDumps = dump


    cfg.addEventAlgo(alg)
    return cfg
