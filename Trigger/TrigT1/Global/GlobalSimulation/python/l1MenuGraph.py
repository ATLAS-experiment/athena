#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# The code in this module is used to examine the contents of a Menu
# and create a list of AlgData objects. These objects contain the information
# required to initialise a GlobalSim AlgTool used to run a GlobalL1TopoSim
# Algorithm.



from .getMenu import getMenu
from .Digraph import Digraph
from .dot import dot

from AthenaCommon.Logging import logging
logger = logging.getLogger(__name__)
from AthenaCommon.Constants import VERBOSE
logger.setLevel(VERBOSE)

import pickle
import os
import pprint

class AlgData():

    def __init__(self):
        self.name = None
        self.klass = None
        self.l1AlgType = None # TOPO, MUTOPO, R2TOPO, MULTTOPO

        # globalL1AlgType must appear in router table in toolFromAlgData
        self.globalL1AlgType = None # INPUT, COUNT, SORT, DECISION

        self.inputs = []
        self.outputs = []

        self.input_sns = []
        self.sn = None

        self.numResultBits = None
        
        # only for some COUNT algorithms
        self.threshold = None
        self.threshold_flavour = None
        
    def __str__(self):

        s = ['Algdata:']
        for k, v in self.__dict__.items():
            s.append(k + ': ' + str(v))

        return '\n'.join(s)

def l1MenuGraph(flags, do_dot=False, verbose=False):
    """Find the L1Topo call graph. For each node, provide a AlgData
    object. These objects carry information needed to configure
    Athena components."""

    L1Menu = None
    if os.path.exists('L1Menu.pkl'):
        with open('L1Menu.pkl', 'rb') as fh:
            L1Menu = pickle.load(fh)
            logger.info('L1Menu  from .pkl file')
    else:
        L1Menu = getMenu(flags)
        with open('L1Menu.pkl', 'wb') as fh:
            pickle.dump(L1Menu, fh)
        logger.info('L1Menu  not from .pkl file')

    logger.debug('menu object')
    logger.debug(L1Menu.printSummary())

    alg_data_list = []

    sn = 1  # node number. 0 is reserved for root
    for algType in ("TOPO", "MULTTOPO"):
        algs = L1Menu.topoAlgorithms(algType) # map: algName: alg details

        for alg_key in algs.keys():
            keyed_algs = algs[alg_key]
            for name, value in keyed_algs.items():
                
                ad = AlgData()
                ad.name = name
                ad.klass = value['klass']
                ad.l1AlgType = algType
                if algType == 'MULTTOPO':
                    ad.globalL1AlgType = 'COUNT'
                    try:
                        threshold_name = value['threshold']
                        ad.threshold = L1Menu.thresholds()[threshold_name]['value']
                        ad.threshold_flavour = value['flavour']
                    except KeyError:
                        logger.info('No threshold data for COUNT alg '+name)

                inputs = value['input']
                if isinstance(inputs, list):
                    ad.inputs = inputs
                else:
                    ad.inputs = [inputs]

                outputs = value['output']
                if isinstance(outputs, list):
                    ad.outputs = outputs
                else:
                    ad.outputs = [outputs]

                ad.sn = sn
                ad.menu = value

                sn += 1
                alg_data_list.append(ad)


    # make lists with no duplicates in a manner that avoids
    # non-fatal but unpleasant changes in odering if set() is naively used.
    all_inputs = []
    [all_inputs.extend(ad.inputs) for ad in alg_data_list]
    all_inputs = {}.fromkeys(all_inputs)
    all_inputs = all_inputs.keys()
    
    all_outputs = []
    [all_outputs.extend(ad.outputs) for ad in alg_data_list]
    all_outputs = {}.fromkeys(all_outputs)
    all_outputs = all_outputs.keys()
 

    missing_inputs = []
    for k in all_inputs:
        if k not in all_outputs:
            missing_inputs.append(k)

    logger.debug('input algs to be added ['+ str(len(missing_inputs))+']')
    for io in missing_inputs:
        logger.debug('input alg: ' + io)
        ad = AlgData()
        # the class string must match the name with which the
        # class appears in the AlRegister
        # 
        klass = 'TopoEventInputAlg' 
        ad.name = klass + '_' + io
        ad.klass =  klass
        ad.outputs = [io]
        ad.globalL1AlgType = 'INPUT'
        ad.sn = sn
        sn += 1
        alg_data_list.append(ad)

    # identify the SORT and DECISION algorithms
    for ad in alg_data_list:
        if ad.globalL1AlgType is not None: continue
        isSortAlg = ad.inputs[0] in missing_inputs
        if isSortAlg:
            if len(ad.inputs) != 1:
                raise RuntimeError('AlgData with > 1 inputs '
                                   'at least 1 from input alg')

            ad.globalL1AlgType = 'SORT'
        else:
            for inp in ad.inputs:
                if inp in missing_inputs:
                    raise RuntimeError('AlgData with >1 inputs'
                                       'at least 1 from input alg')
                
            ad.globalL1AlgType = 'DECISION'

        
    alg2inputs = {}
    output2alg = {}
    for ad in alg_data_list:
        algname = ad.name
        assert algname not in alg2inputs

        alg2inputs[algname] = ad.inputs

        for outp in ad.outputs:
            assert outp not in output2alg
            output2alg[outp] = algname
        
    logger.verbose('\nalg2inputs')
    logger.verbose(pprint.pformat(alg2inputs, indent=4))
    
    logger.verbose('\noutput2alg')
    logger.verbose(pprint.pformat(output2alg, indent=4))


    # ensure alg names are unique
    alg_names = [ad.name for ad in alg_data_list]
    assert len(alg_names) == len(set(alg_names))

    # ensure output names are not shared by more than one algorithm
    outputs = []
    [outputs.extend(ad.outputs) for ad in alg_data_list]
    assert len(outputs) == len(set(outputs))

    # algorithm C  is the child of algorithm P if C has an output
    # which is an input to P

    output2sn = {}
    for a in alg_data_list:
        for o in a.outputs:
            output2sn[o] = a.sn

    # from input names, fill in input sns:
    for ad in alg_data_list:
        ad.input_sns = [output2sn[inp] for inp in ad.inputs]

       
    logger.verbose('\nalg data:')
    for ad in alg_data_list:
        logger.verbose(pprint.pformat(str(ad), indent=4))

    adj_table = {}
    V = sn  # no. of vertices. 0 reserved for root. sn has already been bumped
    for v in range(V):  # need V locations
        adj_table[v] = []

    for a in alg_data_list:
        for i in a.inputs:
            if i in output2sn:
                adj_table[a.sn].append(output2sn[i])

    G = Digraph(V)
    for v in adj_table:
        for w in adj_table[v]:
            G.addEdge(v, w)

    if do_dot:
        dot(G, 'G.dot')

    return G, alg_data_list

if __name__ == '__main__':
    l1MenuGraph(do_dot=True, verbose=True)
