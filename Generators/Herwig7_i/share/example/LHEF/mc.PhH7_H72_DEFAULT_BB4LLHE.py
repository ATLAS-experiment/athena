# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

## Initialise Herwig7 for LHEF showering
include("Herwig7_i/Herwig7_LHEF.py")

## Provide config information
evgenConfig.generators += ["Powheg", "Herwig7"]
evgenConfig.tune        = "H7.2-Default"
evgenConfig.description = "PowhegBox/Herwig7 BB4L LHEF"
evgenConfig.keywords    = ["SM","top"]
evgenConfig.contact     = ["Katharina Voß (katharina.voss@cern.ch)"]

## Configure Herwig7
Herwig7Config.me_pdf_commands(order="NLO", name="NNPDF30_nlo_as_0118")
Herwig7Config.lhef_powhegbox_commands(lhe_filename=runArgs.inputGeneratorFile,me_pdf_order="NLO", usespin=True, usepwhglhereader=True, pwg_reader="BB4LPowheg", pwg_reader_lib="libpowhegHerwigBB4L.so")
Herwig7Config.add_commands("""

set /Herwig/EventHandlers/LHEReader:WeightWarnings 0
set /Herwig/EventHandlers/LHEReader:CacheFileName cache.tmp
set /Herwig/EventHandlers/LHEReader:MaxScan 3

###################################################################
# Technical parameters for this run and setup the event generator #
###################################################################
cd /Herwig/Generators

# theGenerator represents the Les Houches Handler generator
#set /Herwig/Generators/EventGenerator:RandomNumberGenerator /Herwig/Random
set /Herwig/Generators/EventGenerator:StandardModelParameters /Herwig/Model


#########################################################    
## Option to use FullShowerVeto  #    
##########################################################   
cd /Herwig/Shower
create Herwig::bb4lFullShowerVeto bb4lFullShowerVeto {PwgReaderLib}
insert ShowerHandler:FullShowerVetoes 0 bb4lFullShowerVeto
set bb4lFullShowerVeto:Type Decay
#reshowering the event if it does not pass the veto
set bb4lFullShowerVeto:Behaviour Shower
set ShowerHandler:MaxTry 1000

#################################################
#Set up the LHAPDF and set the PDFs             #
#################################################
# Don't try and find PDF index out from the LH file ...
set /Herwig/EventHandlers/LHEReader:InitPDFs 0
#create ThePEG::LHAPDF myPDFset ThePEGLHAPDF.so
# Instead set them explicitly here:
set /Herwig/EventHandlers/LHEReader:PDFA /Herwig/Partons/Hard{PDFOrder}PDF
set /Herwig/EventHandlers/LHEReader:PDFB /Herwig/Partons/Hard{PDFOrder}PDF

######################################################### 
# Set particles properties                              # 
######################################################### 

#Top (and tbar)
set /Herwig/Particles/t:Width 1.32*GeV
set /Herwig/Particles/tbar:Width 1.32
set /Herwig/Particles/t:Width_generator:Initialize Yes 
set /Herwig/Particles/t:Mass_generator:Initialize Yes # mass is set in physics_parameter_commands -> but is it ok to Initialize here?
set /Herwig/Decays/Top:Initialize Yes

#bottom (and bbar)
set /Herwig/Particles/b:NominalMass 4.95
set /Herwig/Particles/bbar:NominalMass 4.95
set /Herwig/Particles/b:ConstituentMass 4.95
set /Herwig/Particles/bbar:ConstituentMass 4.95

#Wp (and Wm)
set /Herwig/Particles/W+:Width_generator:Initialize Yes
set /Herwig/Particles/W+:Mass_generator:Initialize Yes

cd /Herwig/Generators
create ThePEG::powhegAnalysis powhegAnalysis libpowhegHerwigBB4L.so
insert /Herwig/Generators/EventGenerator:AnalysisHandlers 0 powhegAnalysis

set powhegAnalysis:Units GeV_mm
set powhegAnalysis:RunNumber 0001

""".format(PwgReaderLib = "libpowhegHerwigBB4L.so",
           PDFOrder="NLO") )

## run generator
Herwig7Config.run()
