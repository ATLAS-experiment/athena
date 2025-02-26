# 
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

def tefficiency2th(inputs):
    rv = []
    for group in inputs:
        rv.extend(histo.CreateHistogram() for histo in group[1])
    return rv
def relative_difference(inputs):
    assert len(inputs) == 1, 'Only a single match group per output is expected'
    matches, histos = inputs[0]
    assert len(histos) == 2, 'Exactly two inputs are required'
    a, b = histos
    result = a.Clone()
    result.Add(b, -1)
    divisor = a.Clone()
    divisor.Add(b)
    result.Divide(divisor)
    result.GetYaxis().SetTitle("(Front - End) / (Front + End)")
    result.SetTitle((result.GetTitle()+", Relative Difference"))
    return [result]