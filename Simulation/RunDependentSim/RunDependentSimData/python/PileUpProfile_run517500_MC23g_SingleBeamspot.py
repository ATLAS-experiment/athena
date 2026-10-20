# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Pileup profile corresponding to 2026 low mu runs 517520 - 519157
# The high-mu LBs at the start of the runs were excluded via GRL
# No division for different beam spots.

def setupProfile(flags, scaleTaskLength=1):
    
  def _evts(x):
    return int(scaleTaskLength * x)
    
  return [
    {'run':517500, 'lb':1, 'starttstamp':1774864260, 'evts':_evts(1), 'mu':0.5, 'step':0},
    {'run':517500, 'lb':2, 'starttstamp':1774864320, 'evts':_evts(1), 'mu':1.5, 'step':0},
    {'run':517500, 'lb':3, 'starttstamp':1774864380, 'evts':_evts(998), 'mu':2.5, 'step':0},
    {'run':517500, 'lb':4, 'starttstamp':1774864440, 'evts':_evts(946), 'mu':3.5, 'step':0},
    {'run':517500, 'lb':5, 'starttstamp':1774864500, 'evts':_evts(46), 'mu':4.5, 'step':0},
    {'run':517500, 'lb':6, 'starttstamp':1774864560, 'evts':_evts(2), 'mu':5.5, 'step':0},
    {'run':517500, 'lb':7, 'starttstamp':1774864620, 'evts':_evts(1), 'mu':6.5, 'step':0},
    {'run':517500, 'lb':8, 'starttstamp':1774864680, 'evts':_evts(1), 'mu':7.5, 'step':0},
    {'run':517500, 'lb':9, 'starttstamp':1774864740, 'evts':_evts(1), 'mu':8.5, 'step':0},
    {'run':517500, 'lb':10, 'starttstamp':1774864800, 'evts':_evts(1), 'mu':9.5, 'step':0},
    {'run':517500, 'lb':11, 'starttstamp':1774864860, 'evts':_evts(1), 'mu':10.5, 'step':0},
    {'run':517500, 'lb':12, 'starttstamp':1774864920, 'evts':_evts(1), 'mu':11.5, 'step':0}
  ]
