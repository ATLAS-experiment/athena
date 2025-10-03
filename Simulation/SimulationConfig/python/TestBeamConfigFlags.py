# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags

#todo? add in the explanatory text from previous implementation

def createTestBeamConfigFlags():
    tbcf = AthConfigFlags()
    tbcf.addFlag("TestBeam.Eta", 'NONE')
    tbcf.addFlag("TestBeam.Theta", 'NONE')
    tbcf.addFlag("TestBeam.Phi", 'NONE')
    tbcf.addFlag("TestBeam.Y", 'NONE')
    tbcf.addFlag("TestBeam.Z", 'NONE')
    tbcf.addFlag("TestBeam.Layout", 'tb_Tile2000_2003_5B')
    # Test Beam beam properties
    tbcf.addFlag("TestBeam.BeamPID", 11)
    tbcf.addFlag("TestBeam.BeamEnergy", 100000) # Just use Beam.Energy?
    tbcf.addFlag("TestBeam.Xbeam", -27500)
    tbcf.addFlag("TestBeam.Ybeam", [-20,20])
    tbcf.addFlag("TestBeam.Zbeam", [-15,15])
    tbcf.addFlag("TestBeam.Tbeam", -27500)

    return tbcf


def testBeamRunArgsToFlags(runArgs, flags):
    if hasattr(runArgs,"Eta") and ( hasattr(runArgs,"Theta") or hasattr(runArgs,"Z") ):
        raise RuntimeError("Eta cannot be specified at the same time as Theta and Z.")

    if hasattr(runArgs,"Eta"):
        flags.TestBeam.Eta = runArgs.Eta

    if hasattr(runArgs,"Theta") or hasattr(runArgs,"Z"):
        if hasattr(runArgs,"Theta"):
            flags.TestBeam.Theta = runArgs.Theta
        else:
            # Z specified on the commmand-line without Theta
            flags.TestBeam.Theta=90

        if hasattr(runArgs,"Z"):
            flags.TestBeam.Z = runArgs.Z
        else:
            # Theta specified on the commmand-line without Z
            flags.TestBeam.Z = 2550.0

    if hasattr(runArgs,"Phi"):
        flags.TestBeam.Phi=runArgs.Phi

    if hasattr(runArgs,"Y"):
        flags.TestBeam.Y=runArgs.Y
