# 
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

def tefficiency2th(inputs):
    rv = []
    for group in inputs:
        rv.extend(histo.CreateHistogram() for histo in group[1])
    return rv
