from Herwig7_i.Herwig7Control import herwig_version

## Set the tune metadata based on version number
evgenConfig.tune = "H"+herwig_version()+"-Default"

## Enable POWHEG LHEF reading in Herwig7
include("Herwig7_i/Herwig7_LHEF.py")
evgenConfig.generators += ["Powheg"]
