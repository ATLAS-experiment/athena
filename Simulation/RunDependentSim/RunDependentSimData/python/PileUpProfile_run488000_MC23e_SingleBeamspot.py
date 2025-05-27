# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Equivalent of PileUpProfile_run470000_MC23e_SingleBeamspot.py, but
# for pp reference run 

def setupProfile(flags, scaleTaskLength=1):

  def _evts(x):
    return int(scaleTaskLength * x)

  return [
    {'run':488000, 'lb':1, 'starttimestamp':1730250060, 'evts':_evts(2), 'mu':0.5, 'step':3},
    {'run':488000, 'lb':2, 'starttimestamp':1730250120, 'evts':_evts(26), 'mu':1.5, 'step':3},
    {'run':488000, 'lb':3, 'starttimestamp':1730250180, 'evts':_evts(120), 'mu':2.5, 'step':3},
    {'run':488000, 'lb':4, 'starttimestamp':1730250240, 'evts':_evts(243), 'mu':3.5, 'step':3},
    {'run':488000, 'lb':5, 'starttimestamp':1730250300, 'evts':_evts(446), 'mu':4.5, 'step':3},
    {'run':488000, 'lb':6, 'starttimestamp':1730250360, 'evts':_evts(179), 'mu':5.5, 'step':3},
    {'run':488000, 'lb':7, 'starttimestamp':1730250420, 'evts':_evts(3), 'mu':6.5, 'step':3},
    {'run':488000, 'lb':8, 'starttimestamp':1730250480, 'evts':_evts(1), 'mu':7.5, 'step':3}
  ]
