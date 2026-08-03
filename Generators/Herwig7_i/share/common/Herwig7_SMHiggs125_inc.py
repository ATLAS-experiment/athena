# SM Higgs H->inclusive BR, taken from YR4 mH=125 GeV
# https://gitlab.cern.ch/LHCHIGGSXS/LHCHXSWG1/YR4/-/blob/master/Higgs_XSBR_YR4.xlsx?ref_type=heads
# this is needed as the HW7 defaults can be out-dated. 

Herwig7Config.add_commands("""
set /Herwig/Particles/h0/h0->b,bbar;:BranchingRatio      0.5824
set /Herwig/Particles/h0/h0->c,cbar;:BranchingRatio      0.02891
set /Herwig/Particles/h0/h0->t,tbar;:BranchingRatio      0.00000
set /Herwig/Particles/h0/h0->mu-,mu+;:BranchingRatio     0.0002176
set /Herwig/Particles/h0/h0->tau-,tau+;:BranchingRatio   0.06272
set /Herwig/Particles/h0/h0->g,g;:BranchingRatio         0.08187
set /Herwig/Particles/h0/h0->gamma,gamma;:BranchingRatio 0.002270
set /Herwig/Particles/h0/h0->Z0,Z0;:BranchingRatio       0.02619
set /Herwig/Particles/h0/h0->W+,W-;:BranchingRatio       0.2137
decaymode h0->Z0,gamma; 0.001533 1 /Herwig/Decays/Mambo
set /Herwig/Particles/h0/h0->Z0,gamma;:OnOff On
set /Herwig/Particles/h0/h0->Z0,gamma;:BranchingRatio    0.001533
decaymode h0->s,sbar; 0.000246 1 /Herwig/Decays/Hff
set /Herwig/Particles/h0/h0->s,sbar;:OnOff On
set /Herwig/Particles/h0/h0->s,sbar;:BranchingRatio      0.00000
""")