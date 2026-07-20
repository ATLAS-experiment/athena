# SM Higgs H->inclusive BR, taken from YR4 mH=125 GeV
# https://gitlab.cern.ch/LHCHIGGSXS/LHCHXSWG1/YR4/-/blob/master/Higgs_XSBR_YR4.xlsx?ref_type=heads
# this is needed as the Pythia defaults can be out-dated.  
genSeq.Pythia8.Commands += [ '25:onMode = off',
                             '25:oneChannel = 1 0.00000   100 3 -3',   # H->ss, no recommendations
                             '25:addChannel = 1 0.02891   100 4 -4',   # H->cc, 
                             '25:addChannel = 1 0.5824    100 5 -5',   # H->bb,
                             '25:addChannel = 1 0.00000   100 6 -6',   # H->tt, zero.
                             '25:addChannel = 1 0.0002176 100 13 -13', # H->mumu, 
                             '25:addChannel = 1 0.06272   100 15 -15', # H->tautau
                             '25:addChannel = 1 0.08187   100 21 21',  # H->gg
                             '25:addChannel = 1 0.002270  100 22 22',  # H->yy
                             '25:addChannel = 1 0.001533  100 22 23',  # H->Zy
                             '25:addChannel = 1 0.02619   100 23 23',  # H->ZZ
                             '25:addChannel = 1 0.2137    100 24 -24', # H->WW
]