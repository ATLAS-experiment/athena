## Configure Pythia8 to read input events from an LHEF file
hasInput = hasattr(runArgs,"inputGeneratorFile")
inputGeneratorFile = getattr(runArgs, "inputGeneratorFile", "")
if hasInput:
   include ('EvgenProdTools/mult_lhe_input.py')

useCompressedLHE = (getattr(runArgs, "avoidExtracting", False)
                    and inputGeneratorFile.endswith(".events.gz"))

assert hasattr(genSeq, "Pythia8")
#genSeq.Pythia8.LHEFile = runArgs.inputGeneratorFile
genSeq.Pythia8.LHEFile = "events.lhe.gz" if useCompressedLHE else "events.lhe"
genSeq.Pythia8.CollisionEnergy = int(runArgs.ecmEnergy)
