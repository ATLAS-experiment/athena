include('SFGen_i/Pythia8_Base_Common.py')

# Pythia shower configuration flags in the elastic case

if genSeq.SFGenConfig.diff != 'el':
    raise Exception("EL Pythia8 shower configuration can only be used with diff='el'")

genSeq.Pythia8.Commands += [
    "PartonLevel:MPI = off",
    "PartonLevel:Remnants = off",
    "Check:event = off",
    "BeamRemnants:primordialKT = off",
    "LesHouches:matchInOut = off"
    "PartonLevel:FSR = on",
    "SpaceShower:dipoleRecoil = on",
    "SpaceShower:pTmaxMatch = 2",
    "SpaceShower:QEDshowerByQ = off",
    "SpaceShower:pTdampMatch=1",
    "BeamRemnants:unresolvedHadron = 3",
]
