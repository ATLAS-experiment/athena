#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from graphAlgs import Topological
from dot import dot

from AthenaCommon.Logging import logging
logger = logging.getLogger(__name__)
from AthenaCommon.Constants import VERBOSE
logger.setLevel(VERBOSE)


def algdata_from_menu(flags, do_dot=False, root_names=[]):
    """algdata_from_menu uses a graph created from an L1Menu to created
    an execution oredered sequence of AlgData objects. AlgData have
    configuration information that is used to configure a GlobalSim
    L1TopoAlgorithm AlgTool"""
    
    from l1MenuGraph import l1MenuGraph

    G, alg_data_list = l1MenuGraph(flags)
    if do_dot:
        dot(G, "menu_graph.dot")


    algname2sn = {}
    for ad in alg_data_list:
        algname2sn[ad.name] = ad.sn

    assert len(algname2sn) == len(alg_data_list)

    if do_dot:
        txt_l = ['%s %d' % (k, v) for k, v in algname2sn.items()]
        txt = '\n'.join(txt_l)
        txt += '\n\n'
        txt_l = ['%d %s' % (v, k) for k, v in algname2sn.items()]
        txt += '\n'.join(txt_l)
        with open('algname2sn.txt', 'w') as fh:
            fh.write(txt)
            
    # allow running of sub graphs.
    # root_names contains the names of the roots of the subgraphs.
    if root_names:
        roots = [algname2sn[name]
                 for name in root_names if name in algname2sn]
        if not roots:
            logger.error("no requested root nodes present in menu")
        elif len(roots) < len(root_names):
            logger.debug(
                'requested ' + str(len(root_names)),
                'found  ' + str(len(roots)) +  'in menu')
                                           
    else:
        # if no nodes are named as root nodes, use all
        # nodes except the node with sn = 0 for now.
        # this is because there is currently no need to tie together
        # the different components, and leaving them unjoined to node 0
        # facilitates the visualisation of the graph structure.i
        roots = [i for i in range(1, G.V)]

        

    # find the executrion order of the AlgTools in terms of graph node
    # identifiers (ints)
    topo_m = Topological(G, roots)
    
    logger.debug('G is a DAG:' + str(topo_m.isDAG()))
    order_sn = topo_m.order()
    logger.debug('root nodes:' +  str(roots))
    logger.debug('Topological order [' +  str(len(order_sn)) + ']')
    logger.debug(str(order_sn))
    if do_dot:
        dot(G, "G_ordered.dot", order_sn)

    # convert ordered sequence from graph node identifiers to AlgData objects
    sn2algData = {}
    for ad in alg_data_list:
        sn2algData[ad.sn] = ad

    assert len(sn2algData) == len(alg_data_list)

    ordered_algs = ['%d %s %s' % (i,
                                  sn2algData[i].name,
                                  sn2algData[i].klass,
                                  ) for i in  order_sn]
    for e in ordered_algs:
        logger.debug(str(e))
        
    ordered_algData = [sn2algData[sn] for sn in order_sn]
    logger.verbose('ordered_algData:[' + str(len(ordered_algData))+']:')
    for ad in ordered_algData:
        logger.verbose(str(ad))

    return ordered_algData

if __name__ == '__main__':
    root_names = [#'SC111-CjJ40abpETA26',
        'Mult_cTAU20M',
        'Mult_cTAU30M',
        'Mult_cTAU35M',
        'Mult_cTAUSPARE2',
    ]
    
    algdata_from_menu(root_names=root_names, do_dot=True)
