# Configure Herwig7 to read input events from an LHEF file

from Herwig7_i.Herwig7_iConf import Herwig7 
from Herwig7_i.Herwig7ConfigLHEF import Hw7ConfigLHEF

genSeq += Herwig7()
Herwig7Config = Hw7ConfigLHEF(genSeq, runArgs)

# Set Herwig7 for evgen
evgenConfig.generators += ["Herwig7"]

hasInput = hasattr(runArgs, "inputGeneratorFile")
if hasInput:
    include("EvgenProdTools/mult_lhe_input.py")
