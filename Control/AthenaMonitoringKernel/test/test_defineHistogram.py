#!/usr/bin/env python
#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

import unittest
import json

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaMonitoringKernel.GenericMonitoringTool import defineHistogram, defineTree

class Test( unittest.TestCase ):
   def test_1D( self ):
      flags = initConfigFlags()
      check = defineHistogram(flags, 'var', 'TH1F', 'EXPERT', 'title', '', 10, 0.0, 10.0)
      true = '{"alias": "var", "allvars": ["var"], "convention": "", "merge": "", "path": "EXPERT", "title": "title", "treeDef": "", "type": "TH1F", "weight": "", "cutMask": "", "xarray": [], "xbins": 10, "xlabels": [], "xmax": 10.0, "xmin": 0.0, "xvar": "var", "yarray": [], "ybins": 0.0, "ylabels": [], "ymax": 0.0, "ymin": 0.0, "yvar": "", "zbins": 0.0, "zlabels": [], "zmax": 0.0, "zmin": 0.0, "zvar": "", "Sumw2": false, "kLBNHistoryDepth": 0, "kAddBinsDynamically": false, "kRebinAxes": false, "kCanRebin": false, "kVec": false, "kVecUO": false, "kCumulative": false, "kLive": false, "kAlwaysCreate": false}'
      self.assertEqual(json.loads(check), json.loads(true))

   def test_1D_all_kwargs( self ):
      flags = initConfigFlags()
      check = defineHistogram(flags=flags, varname='var', weight='myweight')
      true = '{"alias": "var", "allvars": ["var"], "convention": "", "merge": "", "path": "", "title": "var", "treeDef": "", "type": "TH1F", "weight": "myweight", "cutMask": "", "xarray": [], "xbins": 100, "xlabels": [], "xmax": 1, "xmin": 0, "xvar": "var", "yarray": [], "ybins": 0.0, "ylabels": [], "ymax": 0.0, "ymin": 0.0, "yvar": "", "zbins": 0.0, "zlabels": [], "zmax": 0.0, "zmin": 0.0, "zvar": "", "Sumw2": false, "kLBNHistoryDepth": 0, "kAddBinsDynamically": false, "kRebinAxes": false, "kCanRebin": false, "kVec": false, "kVecUO": false, "kCumulative": false, "kLive": false, "kAlwaysCreate": false}'
      self.assertEqual(json.loads(check), json.loads(true))

   def test_1D_varname( self ):
      flags = initConfigFlags()
      check = defineHistogram(flags, varname='var', weight='myweight')
      true = '{"alias": "var", "allvars": ["var"], "convention": "", "merge": "", "path": "", "title": "var", "treeDef": "", "type": "TH1F", "weight": "myweight", "cutMask": "", "xarray": [], "xbins": 100, "xlabels": [], "xmax": 1, "xmin": 0, "xvar": "var", "yarray": [], "ybins": 0.0, "ylabels": [], "ymax": 0.0, "ymin": 0.0, "yvar": "", "zbins": 0.0, "zlabels": [], "zmax": 0.0, "zmin": 0.0, "zvar": "", "Sumw2": false, "kLBNHistoryDepth": 0, "kAddBinsDynamically": false, "kRebinAxes": false, "kCanRebin": false, "kVec": false, "kVecUO": false, "kCumulative": false, "kLive": false, "kAlwaysCreate": false}'
      self.assertEqual(json.loads(check), json.loads(true))

   def test_1D_weight( self ):
      flags = initConfigFlags()
      check = defineHistogram(flags, 'var', weight='myweight')
      true = '{"alias": "var", "allvars": ["var"], "convention": "", "merge": "", "path": "", "title": "var", "treeDef": "", "type": "TH1F", "weight": "myweight", "cutMask": "", "xarray": [], "xbins": 100, "xlabels": [], "xmax": 1, "xmin": 0, "xvar": "var", "yarray": [], "ybins": 0.0, "ylabels": [], "ymax": 0.0, "ymin": 0.0, "yvar": "", "zbins": 0.0, "zlabels": [], "zmax": 0.0, "zmin": 0.0, "zvar": "", "Sumw2": false, "kLBNHistoryDepth": 0, "kAddBinsDynamically": false, "kRebinAxes": false, "kCanRebin": false, "kVec": false, "kVecUO": false, "kCumulative": false, "kLive": false, "kAlwaysCreate": false}'
      self.assertEqual(json.loads(check), json.loads(true))

   def test_1D_cutmask( self ):
      flags = initConfigFlags()
      check = defineHistogram(flags, 'var', cutmask='mycutmask')
      true = '{"alias": "var", "allvars": ["var"], "convention": "", "merge": "", "path": "", "title": "var", "treeDef": "", "type": "TH1F", "weight": "", "cutMask": "mycutmask", "xarray": [], "xbins": 100, "xlabels": [], "xmax": 1, "xmin": 0, "xvar": "var", "yarray": [], "ybins": 0.0, "ylabels": [], "ymax": 0.0, "ymin": 0.0, "yvar": "", "zbins": 0.0, "zlabels": [], "zmax": 0.0, "zmin": 0.0, "zvar": "", "Sumw2": false, "kLBNHistoryDepth": 0, "kAddBinsDynamically": false, "kRebinAxes": false, "kCanRebin": false, "kVec": false, "kVecUO": false, "kCumulative": false, "kLive": false, "kAlwaysCreate": false}'
      self.assertEqual(json.loads(check), json.loads(true))

   def test_1D_array( self ):
      flags = initConfigFlags()
      check = defineHistogram(flags, 'var', xbins=[0, 1, 2, 4, 8])
      true = '{"alias": "var", "allvars": ["var"], "convention": "", "merge": "", "path": "", "title": "var", "treeDef": "", "type": "TH1F", "weight": "", "cutMask": "", "xarray": [0, 1, 2, 4, 8], "xbins": 4, "xlabels": [], "xmax": 1, "xmin": 0, "xvar": "var", "yarray": [], "ybins": 0.0, "ylabels": [], "ymax": 0.0, "ymin": 0.0, "yvar": "", "zbins": 0.0, "zlabels": [], "zmax": 0.0, "zmin": 0.0, "zvar": "", "Sumw2": false, "kLBNHistoryDepth": 0, "kAddBinsDynamically": false, "kRebinAxes": false, "kCanRebin": false, "kVec": false, "kVecUO": false, "kCumulative": false, "kLive": false, "kAlwaysCreate": false}'
      self.assertEqual(json.loads(check), json.loads(true))

   def test_1D_title( self ):
      flags = initConfigFlags()
      check = defineHistogram(flags, 'var', title='mytitle')
      true = '{"alias": "var", "allvars": ["var"], "convention": "", "merge": "", "path": "", "title": "mytitle", "treeDef": "", "type": "TH1F", "weight": "", "cutMask": "", "xarray": [], "xbins": 100, "xlabels": [], "xmax": 1, "xmin": 0, "xvar": "var", "yarray": [], "ybins": 0.0, "ylabels": [], "ymax": 0.0, "ymin": 0.0, "yvar": "", "zbins": 0.0, "zlabels": [], "zmax": 0.0, "zmin": 0.0, "zvar": "", "Sumw2": false, "kLBNHistoryDepth": 0, "kAddBinsDynamically": false, "kRebinAxes": false, "kCanRebin": false, "kVec": false, "kVecUO": false, "kCumulative": false, "kLive": false, "kAlwaysCreate": false}'
      self.assertEqual(json.loads(check), json.loads(true))

   def test_1D_labelsX( self ):
      flags = initConfigFlags()
      check = defineHistogram(flags, 'var', xlabels=["bin0", "bin1"])
      true = '{"alias": "var", "allvars": ["var"], "convention": "", "merge": "", "path": "", "title": "var", "treeDef": "", "type": "TH1F", "weight": "", "cutMask": "", "xarray": [], "xbins": 2, "xlabels": ["bin0", "bin1"], "xmax": 1, "xmin": 0, "xvar": "var", "yarray": [], "ybins": 0.0, "ylabels": [], "ymax": 0.0, "ymin": 0.0, "yvar": "", "zbins": 0.0, "zlabels": [], "zmax": 0.0, "zmin": 0.0, "zvar": "", "Sumw2": false, "kLBNHistoryDepth": 0, "kAddBinsDynamically": false, "kRebinAxes": false, "kCanRebin": false, "kVec": false, "kVecUO": false, "kCumulative": false, "kLive": false, "kAlwaysCreate": false}'
      self.assertEqual(json.loads(check), json.loads(true))

   def test_1D_labelsY( self ):
      flags = initConfigFlags()
      check = defineHistogram(flags, 'var', ylabels=["bin0", "bin1"])
      true = '{"alias": "var", "allvars": ["var"], "convention": "", "merge": "", "path": "", "title": "var", "treeDef": "", "type": "TH1F", "weight": "", "cutMask": "", "xarray": [], "xbins": 100, "xlabels": [], "xmax": 1, "xmin": 0, "xvar": "var", "yarray": [], "ybins": 2, "ylabels": ["bin0", "bin1"], "ymax": 1.0, "ymin": 0.0, "yvar": "", "zbins": 0.0, "zlabels": [], "zmax": 0.0, "zmin": 0.0, "zvar": "", "Sumw2": false, "kLBNHistoryDepth": 0, "kAddBinsDynamically": false, "kRebinAxes": false, "kCanRebin": false, "kVec": false, "kVecUO": false, "kCumulative": false, "kLive": false, "kAlwaysCreate": false}'
      self.assertEqual(json.loads(check), json.loads(true))

   def test_2D( self ):
      flags = initConfigFlags()
      check = defineHistogram(flags, 'varX,varY', type='TH2F', xbins=10, xmin=0.0, xmax=10.0, ybins=40, ymin=0.0, ymax=20.0)
      true = '{"alias": "varY_vs_varX", "allvars": ["varX", "varY"], "convention": "", "merge": "", "path": "", "title": "varX,varY", "treeDef": "", "type": "TH2F", "weight": "", "cutMask": "", "xarray": [], "xbins": 10, "xlabels": [], "xmax": 10.0, "xmin": 0.0, "xvar": "varX", "yarray": [], "ybins": 40, "ylabels": [], "ymax": 20.0, "ymin": 0.0, "yvar": "varY", "zbins": 0.0, "zlabels": [], "zmax": 0.0, "zmin": 0.0, "zvar": "", "Sumw2": false, "kLBNHistoryDepth": 0, "kAddBinsDynamically": false, "kRebinAxes": false, "kCanRebin": false, "kVec": false, "kVecUO": false, "kCumulative": false, "kLive": false, "kAlwaysCreate": false}'
      self.assertEqual(json.loads(check), json.loads(true))

   def test_2D_space( self ):
      flags = initConfigFlags()
      check = defineHistogram(flags, ' varX , varY ', type='TH2F', xbins=10, xmin=0.0, xmax=10.0, ybins=40, ymin=0.0, ymax=20.0)
      true = '{"alias": "varY_vs_varX", "allvars": ["varX", "varY"], "convention": "", "merge": "", "path": "", "title": " varX , varY ", "treeDef": "", "type": "TH2F", "weight": "", "cutMask": "", "xarray": [], "xbins": 10, "xlabels": [], "xmax": 10.0, "xmin": 0.0, "xvar": "varX", "yarray": [], "ybins": 40, "ylabels": [], "ymax": 20.0, "ymin": 0.0, "yvar": "varY", "zbins": 0, "zlabels": [], "zmax": 0.0, "zmin": 0.0, "zvar": "", "Sumw2": false, "kLBNHistoryDepth": 0, "kAddBinsDynamically": false, "kRebinAxes": false, "kCanRebin": false, "kVec": false, "kVecUO": false, "kCumulative": false, "kLive": false, "kAlwaysCreate": false}'
      self.assertEqual(json.loads(check), json.loads(true))

   def test_2D_array( self ):
      flags = initConfigFlags()
      check = defineHistogram(flags, 'varX,varY', 'TH2F', xbins=[0,1,2], ybins=[1,2,3,7])
      true = '{"alias": "varY_vs_varX", "allvars": ["varX", "varY"], "convention": "", "merge": "", "path": "", "title": "varX,varY", "treeDef": "", "type": "TH2F", "weight": "", "cutMask": "", "xarray": [0, 1, 2], "xbins": 2, "xlabels": [], "xmax": 1, "xmin": 0, "xvar": "varX", "yarray": [1, 2, 3, 7], "ybins": 3, "ylabels": [], "ymax": 0.0, "ymin": 0.0, "yvar": "varY", "zbins": 0.0, "zlabels": [], "zmax": 0.0, "zmin": 0.0, "zvar": "", "Sumw2": false, "kLBNHistoryDepth": 0, "kAddBinsDynamically": false, "kRebinAxes": false, "kCanRebin": false, "kVec": false, "kVecUO": false, "kCumulative": false, "kLive": false, "kAlwaysCreate": false}'
      self.assertEqual(json.loads(check), json.loads(true))

   def test_2D_labelsXY( self ):
      flags = initConfigFlags()
      check = defineHistogram(flags, 'varX,varY', 'TH2F', xlabels=["bin0", "bin1"], ylabels=["bin0", "bin1", "bin2"])
      true = '{"alias": "varY_vs_varX", "allvars": ["varX", "varY"], "convention": "", "merge": "", "path": "", "title": "varX,varY", "treeDef": "", "type": "TH2F", "weight": "", "cutMask": "", "xarray": [], "xbins": 2, "xlabels": ["bin0", "bin1"], "xmax": 1, "xmin": 0, "xvar": "varX", "yarray": [], "ybins": 3, "ylabels": ["bin0", "bin1", "bin2"], "ymax": 1.0, "ymin": 0.0, "yvar": "varY", "zbins": 0.0, "zlabels": [], "zmax": 0.0, "zmin": 0.0, "zvar": "", "Sumw2": false, "kLBNHistoryDepth": 0, "kAddBinsDynamically": false, "kRebinAxes": false, "kCanRebin": false, "kVec": false, "kVecUO": false, "kCumulative": false, "kLive": false, "kAlwaysCreate": false}'
      self.assertEqual(json.loads(check), json.loads(true))

   def test_3D( self ):
      flags = initConfigFlags()
      check = defineHistogram(flags, 'varX,varY,varZ', 'TProfile2D',
         xbins=10, xmin=0.0, xmax=10.0, ybins=40, ymin=0.0, ymax=20.0, zmin=-1.0, zmax=1.0)
      true = '{"alias": "varZ_vs_varY_vs_varX", "allvars": ["varX", "varY", "varZ"], "convention": "", "merge": "", "path": "", "title": "varX,varY,varZ", "treeDef": "", "type": "TProfile2D", "weight": "", "cutMask": "", "xarray": [], "xbins": 10, "xlabels": [], "xmax": 10.0, "xmin": 0.0, "xvar": "varX", "yarray": [], "ybins": 40, "ylabels": [], "ymax": 20.0, "ymin": 0.0, "yvar": "varY", "zbins": 0.0, "zlabels": [], "zmax": 1.0, "zmin": -1.0, "zvar": "varZ", "Sumw2": false, "kLBNHistoryDepth": 0, "kAddBinsDynamically": false, "kRebinAxes": false, "kCanRebin": false, "kVec": false, "kVecUO": false, "kCumulative": false, "kLive": false, "kAlwaysCreate": false}'
      self.assertEqual(json.loads(check), json.loads(true))

   def test_efficiency( self ):
      flags = initConfigFlags()
      check = defineHistogram(flags, 'var,pass', type='TEfficiency')
      true = '{"alias": "pass_vs_var", "allvars": ["var", "pass"], "convention": "", "merge": "", "path": "", "title": "var,pass", "treeDef": "", "type": "TEfficiency", "weight": "", "cutMask": "", "xarray": [], "xbins": 100, "xlabels": [], "xmax": 1, "xmin": 0, "xvar": "var", "yarray": [], "ybins": 0.0, "ylabels": [], "ymax": 0.0, "ymin": 0.0, "yvar": "pass", "zbins": 0.0, "zlabels": [], "zmax": 0.0, "zmin": 0.0, "zvar": "", "Sumw2": false, "kLBNHistoryDepth": 0, "kAddBinsDynamically": false, "kRebinAxes": false, "kCanRebin": false, "kVec": false, "kVecUO": false, "kCumulative": false, "kLive": false, "kAlwaysCreate": false}'
      self.assertEqual(json.loads(check), json.loads(true))

   def test_tree( self ):
      flags = initConfigFlags()
      check = defineTree(flags, 'var,pass', treedef='var/F:pass/I')
      true = '{"alias": "pass_vs_var", "allvars": ["var", "pass"], "convention": "", "merge": "", "path": "", "title": "var,pass", "treeDef": "var/F:pass/I", "type": "TTree", "weight": "", "cutMask": "", "xarray": [], "xbins": 100, "xlabels": [], "xmax": 1, "xmin": 0, "xvar": "var", "yarray": [], "ybins": 0.0, "ylabels": [], "ymax": 0.0, "ymin": 0.0, "yvar": "pass", "zbins": 0, "zlabels": [], "zmax": 0.0, "zmin": 0.0, "zvar": "", "Sumw2": false, "kLBNHistoryDepth": 0, "kAddBinsDynamically": false, "kRebinAxes": false, "kCanRebin": false, "kVec": false, "kVecUO": false, "kCumulative": false, "kLive": false, "kAlwaysCreate": false}'
      self.assertEqual(json.loads(check), json.loads(true))

   def test_offlineNamingConvention( self ):
      flags = initConfigFlags()
      check = defineHistogram(flags, 'var', path='EXPERT', convention='OFFLINE:lowStat')
      true = '{"alias": "var", "allvars": ["var"], "convention": "OFFLINE:lowStat", "merge": "", "path": "EXPERT", "title": "var", "treeDef": "", "type": "TH1F", "weight": "", "cutMask": "", "xarray": [], "xbins": 100, "xlabels": [], "xmax": 1, "xmin": 0, "xvar": "var", "yarray": [], "ybins": 0.0, "ylabels": [], "ymax": 0.0, "ymin": 0.0, "yvar": "", "zbins": 0.0, "zlabels": [], "zmax": 0.0, "zmin": 0.0, "zvar": "", "Sumw2": false, "kLBNHistoryDepth": 0, "kAddBinsDynamically": false, "kRebinAxes": false, "kCanRebin": false, "kVec": false, "kVecUO": false, "kCumulative": false, "kLive": false, "kAlwaysCreate": false}'
      self.assertEqual(json.loads(check), json.loads(true))

   def test_merge( self ):
      flags = initConfigFlags()
      check = defineHistogram(flags, 'var', 'TH1F', 'EXPERT', 'title', '', 10, 0.0, 10.0, merge='weightedAverage')
      true = '{"alias": "var", "allvars": ["var"], "convention": "", "merge": "weightedAverage", "path": "EXPERT", "title": "title", "treeDef": "", "type": "TH1F", "weight": "", "cutMask": "", "xarray": [], "xbins": 10, "xlabels": [], "xmax": 10.0, "xmin": 0.0, "xvar": "var", "yarray": [], "ybins": 0.0, "ylabels": [], "ymax": 0.0, "ymin": 0.0, "yvar": "", "zbins": 0, "zlabels": [], "zmax": 0.0, "zmin": 0.0, "zvar": "", "Sumw2": false, "kLBNHistoryDepth": 0, "kAddBinsDynamically": false, "kRebinAxes": false, "kCanRebin": false, "kVec": false, "kVecUO": false, "kCumulative": false, "kLive": false, "kAlwaysCreate": false}'
      self.assertEqual(json.loads(check), json.loads(true))

   def test_optString( self ):
      flags = initConfigFlags()
      check = defineHistogram(flags, 'var', opt='Sumw2,kLBNHistoryDepth=9')
      true = '{"alias": "var", "allvars": ["var"], "convention": "", "merge": "", "path": "", "title": "var", "treeDef": "", "type": "TH1F", "weight": "", "cutMask": "", "xarray": [], "xbins": 100, "xlabels": [], "xmax": 1, "xmin": 0, "xvar": "var", "yarray": [], "ybins": 0.0, "ylabels": [], "ymax": 0.0, "ymin": 0.0, "yvar": "", "zbins": 0.0, "zlabels": [], "zmax": 0.0, "zmin": 0.0, "zvar": "", "Sumw2": true, "kLBNHistoryDepth": 9, "kAddBinsDynamically": false, "kRebinAxes": false, "kCanRebin": false, "kVec": false, "kVecUO": false, "kCumulative": false, "kLive": 0, "kAlwaysCreate": false}'
      self.assertEqual(json.loads(check), json.loads(true))

      check = defineHistogram(flags, 'var', opt='Sumw2 kLBNHistoryDepth=9')
      self.assertEqual(json.loads(check), json.loads(true))

      with self.assertRaises(AssertionError):
         defineHistogram(flags, 'var', opt='Sumw2,kLBNHistoryDepth=False')
      with self.assertRaises(ValueError):
         defineHistogram(flags, 'var', opt='Sumw2,kLBNHistoryDepth=xxx')
      with self.assertRaises(AssertionError):
         defineHistogram(flags, 'var', opt='Sumw2=1,kLBNHistoryDepth=1')
      with self.assertRaises(AssertionError):
         defineHistogram(flags, 'var', opt='xxx=0')

   def test_optDict( self ):
      flags = initConfigFlags()
      check = defineHistogram(flags, 'var', opt={'Sumw2':True,'kLBNHistoryDepth':9})
      true = '{"alias": "var", "allvars": ["var"], "convention": "", "merge": "", "path": "", "title": "var", "treeDef": "", "type": "TH1F", "weight": "", "cutMask": "", "xarray": [], "xbins": 100, "xlabels": [], "xmax": 1, "xmin": 0, "xvar": "var", "yarray": [], "ybins": 0.0, "ylabels": [], "ymax": 0.0, "ymin": 0.0, "yvar": "", "zbins": 0.0, "zlabels": [], "zmax": 0.0, "zmin": 0.0, "zvar": "", "Sumw2": true, "kLBNHistoryDepth": 9, "kAddBinsDynamically": false, "kRebinAxes": false, "kCanRebin": false, "kVec": false, "kVecUO": false, "kCumulative": false, "kLive": 0, "kAlwaysCreate": false}'
      self.assertEqual(json.loads(check), json.loads(true))
      with self.assertRaises(AssertionError): # wrong type for kLBNHistoryDepth (expected int, given bool)
         defineHistogram(flags, 'var', opt={'Sumw2':True,'kLBNHistoryDepth':False})
      with self.assertRaises(AssertionError): # wrong type for kLBNHistoryDepth (expected int, given string)
         defineHistogram(flags, 'var', opt={'Sumw2':True,'kLBNHistoryDepth':'xxx'})
      with self.assertRaises(AssertionError): # wrong type for Sumw2 (expected bool, given int)
         print(defineHistogram(flags, 'var', opt={'Sumw2':1,'kLBNHistoryDepth':9}))
      with self.assertRaises(AssertionError): # incorrect key
         defineHistogram(flags, 'var', opt={'xxx':1})

   def test_live( self ):
      flags = initConfigFlags()
      flags.Common.isOnline = True
      check = defineHistogram(flags, 'var', path='EXPERT', opt='kLive=1')
      true = '{"alias": "var", "allvars": ["var"], "convention": "", "merge": "", "path": "EXPERT", "title": "var", "treeDef": "", "type": "TH1F", "weight": "", "cutMask": "", "xarray": [], "xbins": 100, "xlabels": [], "xmax": 1.0, "xmin": 0.0, "xvar": "var", "yarray": [], "ybins": 0.0, "ylabels": [], "ymax": 0.0, "ymin": 0.0, "yvar": "", "zbins": 0.0, "zlabels": [], "zmax": 0.0, "zmin": 0.0, "zvar": "", "Sumw2": false, "kLBNHistoryDepth": 0, "kAddBinsDynamically": false, "kRebinAxes": false, "kCanRebin": false, "kVec": false, "kVecUO": false, "kCumulative": false, "kLive": 1, "kAlwaysCreate": false}'
      self.assertEqual(json.loads(check), json.loads(true))

   def test_LBN( self ):
      flags = initConfigFlags()
      flags.Common.isOnline = True
      check = defineHistogram(flags, 'var', path='EXPERT', opt='kLBNHistoryDepth=1')
      self.assertEqual(json.loads(check)['kAlwaysCreate'], True)

   def test_badAlias( self ):
      flags = initConfigFlags()
      check = defineHistogram(flags, 'var;alias;more')
      self.assertFalse(check)

   def test_invalidAlias( self ):
      flags = initConfigFlags()
      flags.Common.isOnline = True
      check = defineHistogram(flags, 'var;myhist(', path='EXPERT')
      self.assertIs(check, '')
      check = defineHistogram(flags, 'var;myhist', path='EXPERT')
      self.assertNotEqual(check, '')

      flags.Common.isOnline = False
      check = defineHistogram(flags, 'var;my/hist')
      self.assertIs(check, '')
      check = defineHistogram(flags, 'var;myhist(')
      self.assertNotEqual(check, '')

   def test_enforceType( self ):
      flags = initConfigFlags()
      flags.Common.isOnline = True
      check = defineTree(flags, 'var,pass', treedef='var/F:pass/I')
      self.assertIs(check, '')

   def test_enforceMergeTypesTest( self ):
      flags = initConfigFlags()
      with self.assertRaises(AssertionError):
         defineHistogram(flags, 'var,pass', type='TEfficiency', merge='weightedAverage')
      with self.assertRaises(AssertionError):
         defineHistogram(flags, 'var,pass', type='TTree', merge='weightedAverage')
     
   def test_enforceNoOfflineLive( self ):
      flags = initConfigFlags()
      with self.assertRaises(AssertionError):
         defineHistogram(flags, 'var', opt='kLive=1')

if __name__ == '__main__':
   unittest.main()
