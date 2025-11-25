# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

def setupProfile(flags, scaleTaskLength=1):

  def _evts(x):
    return int(scaleTaskLength * x)

  return [
    {'run':311000, 'lb':1, 'starttstamp':1551000000, 'evts':_evts(10), 'mu':0.5, 'step':3},
    {'run':311000, 'lb':2, 'starttstamp':1551000060, 'evts':_evts(925), 'mu':1.5, 'step':3},
    {'run':311000, 'lb':3, 'starttstamp':1551000120, 'evts':_evts(1037), 'mu':2.5, 'step':3},
    {'run':311000, 'lb':4, 'starttstamp':1551000180, 'evts':_evts(19), 'mu':3.5, 'step':3},
    {'run':311000, 'lb':5, 'starttstamp':1551000240, 'evts':_evts(4), 'mu':4.5, 'step':3},
    {'run':311000, 'lb':6, 'starttstamp':1551000300, 'evts':_evts(2), 'mu':5.5, 'step':3},
    {'run':311000, 'lb':7, 'starttstamp':1551000360, 'evts':_evts(1), 'mu':6.5, 'step':3},
    {'run':311000, 'lb':8, 'starttstamp':1551000420, 'evts':_evts(1), 'mu':7.5, 'step':3},
    {'run':311000, 'lb':9, 'starttstamp':1551000480, 'evts':_evts(1), 'mu':8.5, 'step':3},
]
