include('SFGen_i/Pythia8_Base_Common.py')

# Pythia shower configuration flags in the single dissociation case

if not (genSeq.SFGenConfig.diff == 'sda' or genSeq.SFGenConfig.diff == 'sdb'):
    raise Exception("SD Pythia8 shower configuration can only be used with diff='sda' or 'sdb'")

unresolvedHadron = -1
if genSeq.SFGenConfig.diff=='sda':
    unresolvedHadron = 2
elif genSeq.SFGenConfig.diff=='sdb':
    unresolvedHadron = 1

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
    "BeamRemnants:unresolvedHadron = {}".format(unresolvedHadron),
]
