#!/usr/bin/env python
#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

import copy
import unittest
import json
import pickle

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaMonitoringKernel.GenericMonitoringTool import (GenericMonitoringTool,
                                                          GenericMonitoringArray)

# suppress error messages for these tests
from AthenaMonitoringKernel.GenericMonitoringTool import log
log.setLevel(50)

def histogramDictionary(histogramList):
    jsonList = [json.loads(histogram) for histogram in histogramList]
    return dict((h['alias'],h) for h in jsonList)

class TestGMT(unittest.TestCase):
    """Tests for GenericMonitoringTool"""

    def assertCheckName(self, tool, name):
        if callable(tool.name):
            tool_name = tool.name()
        else:
            tool_name = tool.name
        self.assertEqual(tool_name, name)

    def test_createGMT(self):
        flags = initConfigFlags()
        gmt = GenericMonitoringTool(flags)
        self.assertCheckName(gmt, 'GenericMonitoringTool')
        self.assertIsInstance(gmt, CompFactory.GenericMonitoringTool)

    def test_createGMT_withName(self):
        flags = initConfigFlags()
        gmt = GenericMonitoringTool(flags, 'gmt')
        self.assertCheckName(gmt, 'gmt')
        self.assertIsInstance(gmt, CompFactory.GenericMonitoringTool)

    def test_pickle(self):
        flags = initConfigFlags()
        gmt = GenericMonitoringTool(flags)
        pickle.dumps(gmt)

    def test_copy(self):
        flags = initConfigFlags()
        gmt = GenericMonitoringTool(flags)
        copy.deepcopy(gmt)  # required by PrivateToolHandle(Array)

    def test_defineHistogram(self):
        flags = initConfigFlags()
        gmt = GenericMonitoringTool(flags)
        gmt.defineHistogram('x')
        self.assertTrue(len(gmt.Histograms)>0)

    def test_defineTree(self):
        flags = initConfigFlags()
        gmt = GenericMonitoringTool(flags)
        gmt.defineTree('t', 'x/F')
        d = histogramDictionary(gmt.Histograms)
        self.assertEqual(d['t']['type'], 'TTree')

    def test_duration(self):
        flags = initConfigFlags()
        gmt = GenericMonitoringTool(flags, defaultDuration='run')
        gmt.convention = 'OFFLINE'
        gmt.defineHistogram('x;x_run')
        gmt.defineHistogram('x;x_lb', duration='lb')
        d = histogramDictionary(gmt.Histograms)
        self.assertEqual(d['x_run']['convention'], 'OFFLINE:run')
        self.assertEqual(d['x_lb']['convention'], 'OFFLINE:lb')

    # Tests for GenericMonitoringArray
    def test_toolVec(self):
        flags = initConfigFlags()
        gma = GenericMonitoringArray(flags, 'TestGMA', [2])
        self.assertIsInstance(gma, GenericMonitoringArray)

    def test_getitem(self):
        flags = initConfigFlags()
        gma = GenericMonitoringArray(flags, 'TestGMA', [2])
        gmt = gma[0]
        self.assertIsInstance(gmt, CompFactory.GenericMonitoringTool)

    def test_toolMap(self):
        flags = initConfigFlags()
        gma = GenericMonitoringArray(flags, 'TestGMA', [['x', 'y', 'z']])
        self.assertEqual(gma.Postfixes, ['_x', '_y', '_z'])

    def test_postfixes(self):
        flags = initConfigFlags()
        gmaVec = GenericMonitoringArray(flags, 'TestGMAVec', [2,2])
        gmaMap = GenericMonitoringArray(flags, 'TestGMAMap', [['a', 'b'],['c','d']])
        self.assertTrue(len(gmaVec.Tools)==4)
        self.assertTrue(len(gmaMap.Tools)==4)
        self.assertEqual(gmaVec.Postfixes, ['_0_0', '_0_1', '_1_0', '_1_1'])
        self.assertEqual(gmaMap.Postfixes, ['_a_c', '_a_d', '_b_c', '_b_d'])

    def test_broadcast(self):
        flags = initConfigFlags()
        gma = GenericMonitoringArray(flags, 'TestGMA', [2])
        gma.broadcast('HistPath', '/path/to/set/')
        self.assertEqual(gma[0].HistPath, '/path/to/set/')
        self.assertEqual(gma[1].HistPath, '/path/to/set/')

    def test_defineHistograms(self):
        flags = initConfigFlags()
        gmaVec = GenericMonitoringArray(flags, 'TestGMAVec', [2])
        gmaMap = GenericMonitoringArray(flags, 'TestGMAMap', [['a', 'b']])
        gmaVec.defineHistogram('x')
        gmaMap.defineHistogram('x')
        dVec = histogramDictionary(list(gmaVec[0].Histograms)+list(gmaVec[1].Histograms))
        dMap = histogramDictionary(list(gmaMap[0].Histograms)+list(gmaMap[1].Histograms))
        self.assertIn('x_0', dVec)
        self.assertIn('x_1', dVec)
        self.assertIn('x_a', dMap)
        self.assertIn('x_b', dMap)
        self.assertEqual(dVec['x_0']['xvar'], 'x')
        self.assertEqual(dVec['x_1']['xvar'], 'x')
        self.assertEqual(dMap['x_a']['xvar'], 'x')
        self.assertEqual(dMap['x_b']['xvar'], 'x')

    def test_titleTemplate(self):
        flags = initConfigFlags()
        gma = GenericMonitoringArray(flags, 'TestGMA', [['a', 'b']])
        gma.defineHistogram('x', title='Plot of x for {}')
        d = histogramDictionary(list(gma[0].Histograms)+list(gma[1].Histograms))
        self.assertEqual(d['x_a']['title'], 'Plot of x for a')
        self.assertEqual(d['x_b']['title'], 'Plot of x for b')
        with self.assertRaises(IndexError):
            gma.defineHistogram('y', title='{1}')

    def test_pathTemplate(self):
        flags = initConfigFlags()
        gma = GenericMonitoringArray(flags, 'TestGMA', [['a','b']])
        gma.defineHistogram('x', path='/xxx/group_{}/yyy/')
        d = histogramDictionary(list(gma[0].Histograms)+list(gma[1].Histograms))
        self.assertEqual(d['x_a']['path'], '/xxx/group_a/yyy/')
        self.assertEqual(d['x_b']['path'], '/xxx/group_b/yyy/')
        with self.assertRaises(IndexError):
            gma.defineHistogram('y', path='{1}')

    def test_aliasTemplate(self):
        flags = initConfigFlags()
        gma = GenericMonitoringArray(flags, 'TestGMA', [['a','b']])
        gma.defineHistogram('x;x-alias-{}-formatted')
        d = histogramDictionary(list(gma[0].Histograms)+list(gma[1].Histograms))
        self.assertEqual(d['x-alias-a-formatted']['alias'], 'x-alias-a-formatted')
        self.assertEqual(d['x-alias-b-formatted']['alias'], 'x-alias-b-formatted')
        with self.assertRaises(IndexError):
            gma.defineHistogram('y;y-alias-{1}-formatted')

    def test_forwardsHistogramArguments(self):
        flags = initConfigFlags()
        gma = GenericMonitoringArray(flags, 'TestGMA', [2])
        gma.defineHistogram('x,y', type='TH2D', weight='z')
        d = histogramDictionary(list(gma[0].Histograms)+list(gma[1].Histograms))
        self.assertEqual(d['y_vs_x_0']['xvar'], 'x')
        self.assertEqual(d['y_vs_x_1']['xvar'], 'x')
        self.assertEqual(d['y_vs_x_0']['yvar'], 'y')
        self.assertEqual(d['y_vs_x_1']['yvar'], 'y')
        self.assertEqual(d['y_vs_x_0']['weight'], 'z')
        self.assertEqual(d['y_vs_x_1']['weight'], 'z')


class TestCA(unittest.TestCase):
    """ComponentAccumulator related tests"""

    @classmethod
    def setUpClass(cls):
        from AthenaConfiguration import AtlasSemantics  # noqa: F401 (ATLAS merge semantics)

    def test_merge(self):
        flags = initConfigFlags()
        t1 = GenericMonitoringTool(flags, Histograms = ['x','y'])
        t2 = GenericMonitoringTool(flags, Histograms = ['z'])
        t2.merge(t1)
        self.assertEqual(list(t1.Histograms), ['x','y','z'])


if __name__ == '__main__':
   unittest.main()
