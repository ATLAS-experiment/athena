# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Make chain dicts for testing jet hypo config modules"""

from TriggerMenuMT.HLT.Menu.Physics_pp_run3_v1 import (
    SingleJetGroup,
    MultiJetGroup)

from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp
from TriggerMenuMT.HLT.Config.Utility.DictFromChainName import dictFromChainName

from TrigHLTJetHypo.hypoConfigBuilder import hypotool_from_chaindict
from TrigHLTJetHypo.HypoToolAnalyser import HypoToolAnalyser

import unittest

from AthenaCommon.Logging import logging
from AthenaCommon.Constants import DEBUG
logger = logging.getLogger( __name__)

logger.setLevel(DEBUG)

from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()
flags.Input.Files = []

flags.lock()

class HypoToolStructure(unittest.TestCase):
    def test_structure(self):
        testData = [
            {'prop':
             ChainProp(name='HLT_j420_subresjesgscIS_ftf_L1J100',
                       l1SeedThresholds=['FSNOSEED'],
                       groups=SingleJetGroup),
             'connections': dict(((0, [1]),
                                  (1, [2]),
                                  (2, [3, 5]),
                                  (3, [4]),
                                  (5, [6, 7]))),
             'values': []
             },
            
            {'prop':
             ChainProp(name='HLT_j260f_L1J75_31ETA49',
                       l1SeedThresholds=['FSNOSEED'],
                       groups=SingleJetGroup),
             'connections': dict(((0, [1]),
                                  (1, [2]),
                                  (2, [3, 5]),
                                  (3, [4]),
                                  (5, [6, 7]))),
             'values': []
             },
            
            
            {'prop':
             ChainProp(name='HLT_j80_j60_L1J15',
                       l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup),
             'connections': dict(((0, [1]),
                                  (1, [2]),
                                  (2, [3, 5, 8]),
                                  (3, [4]),
                                  (5, [6, 7]),
                                  (8, [9, 10]))),
             'values': []
             },
            
        {
            'prop': ChainProp(name='HLT_2j80_3j60_L1J15',
                              l1SeedThresholds=['FSNOSEED']*2, groups=MultiJetGroup),
            'connections': dict(((0, [1]), (1, [2]),
                                 (2, [3, 5, 8]),
                                 (3, [4]),
                                 (5, [6, 7]),
                                 (8, [9,10]))),
             'values': []
        },

        {
            'prop': ChainProp(name='HLT_j0_HT1000_L1J20',
                              l1SeedThresholds=['FSNOSEED'],
                              groups=SingleJetGroup),
            
            'connections': dict(((0, [1]),
                                 (1, [2]),
                                 (2, [3, 5, 7]),
                                 (3, [4]),
                                 (5, [6]),
                                 (7, [8]),
                                 (8, [9, 10]))),
             'values': []
        },

        {
            'prop':  ChainProp(
                name='HLT_j70_1j50a_j0_DIJET70j12etXX1000djmassXXdjdphi200XX400djdeta_L1MJJ-500-NFF',
                l1SeedThresholds=['FSNOSEED']*3,
                groups=MultiJetGroup),
            
            'connections': dict(((0, [1]),
                                 (1, [2, 11]),
                                 (2, [3, 5, 8]),
                                 (3, [4]),
                                 (5, [6, 7]),
                                 (8, [9, 10]),
                                 (11, [12, 14, 18]),
                                 (12, [13]),
                                 (14, [15, 16, 17]),
                                 (18, [19, 20]),)),
             'values': []
        },

            {
                'prop': ChainProp(name='HLT_10j40_L1J15',
                                  l1SeedThresholds=['FSNOSEED'],
                                  groups=MultiJetGroup),
         
                'connections': dict(((0, [1]),
                                     (1, [2]),
                                     (2, [3, 5]),
                                     (3, [4]),
                                     (5, [6, 7]))),
             'values': [{'nodeid': 5,
                         'multiplicity': 10}]
            },

            {
                'prop': ChainProp(name='HLT_j0_FBDJSHARED_L1J20',
                                  l1SeedThresholds=['FSNOSEED'],
                                  groups=SingleJetGroup),
        
                'connections': dict(((0, [1]),
                                     (1, [2, 11]),
                                     (2, [3, 5, 8]),
                                     (3, [4]),
                                     (5, [6, 7]),
                                     (8, [9, 10]),
                                     (11, [12, 14, 17, 20]),
                                     (12, [13]),
                                     (14, [15, 16]),
                                     (17, [18, 19]),
                                     (20, [21, 22]))),
             'values': []
            },
            
            {
                'prop': ChainProp(name='HLT_j40_j0_HT50XX10etXX0eta320_L1J20',
                                  l1SeedThresholds=['FSNOSEED']*2,
                                  groups=MultiJetGroup),
        
                'connections': dict(((0, [1]),
                                     (1, [2, 8]),
                                     (2, [3, 5]),
                                     (3, [4]),
                                     (5, [6, 7]),
                                     (8, [9, 11, 13]),
                                     (9, [10]),
                                     (11, [12]),
                                     (13, [14]),
                                     (14, [15, 16]))),
             'values': []
            },
            
            {
                'prop': ChainProp(name='HLT_j0_FBDJNOSHARED10etXX20etXX34massXX50fbet_L1J20',
                                  l1SeedThresholds=['FSNOSEED'],
                                  groups=SingleJetGroup),
                
                'connections': dict(((0, [1]),
                                     (1, [2]),
                                     (2, [3, 5, 8, 11, 14, 17]),
                                     (3, [4]),
                                     (5, [6, 7]),
                                     (8, [9, 10]),
                                     (11, [12, 13]),
                                     (14, [15, 16]),
                                     (17, [18, 19]))),
                'values': []
            },


            {
                'prop': ChainProp(name='HLT_j45_pf_ftf_preselj20_L1J15',
                                  l1SeedThresholds=['FSNOSEED'],
                                  groups=SingleJetGroup),
        
                'connections': dict(((0, [1]),
                                     (1, [2]),
                                     (2, [3, 5]),
                                     (3, [4]),
                                     (5, [6, 7]))),
             'values': []
            },
            
            {
                'prop': ChainProp(name='HLT_j85_ftf_MASK300ceta210XX300nphi10_L1J20',
                                  l1SeedThresholds=['FSNOSEED'],
                                  groups=SingleJetGroup),
                
                
                'connections': dict(((0, [1]),
                                     (1, [2, 8]),
                                     (2, [3, 5]),
                                     (3, [4]),
                                     (5, [6, 7]),
                                     (8, [9]),
                                     (9, [10, 11]))),
             'values': []
            },

    
            {
                'prop': ChainProp(name='HLT_j0_DIJET80j12etXX0j12eta240XX700djmass_L1J20',
                                  l1SeedThresholds=['FSNOSEED'],
                                  groups=SingleJetGroup),

        
        
                'connections': dict(((0, [1]),
                                     (1, [2]),
                                     (2, [3, 5, 7]),
                                     (3, [4]),
                                     (5, [6]),
                                     (7, [8, 9]))),

                'values': [{'nodeid': 6, 'min': '700000.0', 'max': 'inf'},
                           
                           {'nodeid': 7, 'multiplicity': 2},
                           {'nodeid': 8, 'min': '80000.0', 'max': 'inf'},
                           {'nodeid': 9, 'min': '0.0', 'max': '2.4'},
                           ]

            }, 
            
        
        ]
        
        for td in testData:
            chain_dict =chain_dict = dictFromChainName(flags, td['prop'])
            # set chain part indices (kludge)
            cpi = 0
            for cp in chain_dict['chainParts']:
                cp['chainPartIndex'] = cpi
                cpi += 1
            chain_name = chain_dict['chainName']
            tool = hypotool_from_chaindict(chain_dict)
           
            analyser = HypoToolAnalyser(tool)

            node_table = []
            for k in sorted(analyser.node_table.keys()):
                node_table.append ('%d: %s' % (k, analyser.node_table[k]))

            node_table = '\n'.join(node_table)
                                   
            self.assertTrue(analyser.connections == td['connections'],
                            'fail for case %s expected %s; saw %s \n %s' %(
                                chain_name,
                                str(td['connections']),
                                str(analyser.connections),
                                node_table)) 

            # check selected attributes of inner (nested) tools.

            for ref_values in td['values']:

                # find the inner (nested) subtool
                # from the tool list in the analyser
                
                nodeid = ref_values['nodeid']
                del ref_values['nodeid']
                subtool = analyser.node_table[nodeid]

                # check selected attributes of the subtool
                
                for attrname, testval in ref_values.items():
                    toolval = getattr(subtool, attrname)
                    self.assertEqual(toolval, testval,
                                    msg='%s:  node: %d expected %s saw  %s' % (
                                        chain_name,
                                        nodeid,
                                        str(testval),
                                        str(toolval)))
                    

if __name__ == '__main__':

    unittest.main()



