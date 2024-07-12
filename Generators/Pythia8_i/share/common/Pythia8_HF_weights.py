# Warnings:
# please look at Pythia8_ShowerWeights.py for relevant warnings
#
# Pythia8 reweighting for (a subset of) hadronization parameters are only available in release 3.11 and later. 
# For the time being, following from rFactb, all other parameters are varied by 10%: must be fixed from validation studies

if "ShowerWeightNames" in genSeq.Pythia8.__slots__.keys():
      print ("Initalizing Shower HF Weights from Pythia8_HF_Weights.py")
      print ("Please notice that this functionality is present from 8.311 onward, but no checks are implemented in this respect.")
      genSeq.Pythia8.Commands += [
                            'UncertaintyBands:doVariations = on',
                            #'StringFlav:mesonCvector = 1.3,'     #no parameter for having such variation
                            'VariationFrag:List = {\
                             frag:aLund=0.61 frag:aLund=0.61,\
                             frag:aLund=0.75 frag:aLund=0.75,\
                             frag:bLund=0.88 frag:bLund=0.88,\
                             frag:bLund=1.1 frag:bLund=1.1,\
                             frag:rFactC=1.9 frag:rFactC=1.9,\
                             frag:rFactC=1.7 frag:rFactC=1.7,\
                             frag:rFactC=2.1 frag:rFactC=2.1,\
                             frag:rFactB=1.05 frag:rFactB=1.05,\
                             frag:rFactB=1.01 frag:rFactB=1.01,\
                             frag:rFactB=1.03 frag:rFactB=1.03,\
                             frag:rFactB=1.07 frag:rFactB=1.07,\
                             frag:rFactB=1.09 frag:rFactB=1.09,\
                             frag:sigma=0.302 frag:ptSigma=0.302,\
                             frag:sigma=0.368 frag:ptSigma=0.368,\
                             frag:xi=0.073 frag:xi=0.073,\
                             frag:xi=0.089 frag:xi=0.089,\
                             frag:rho=0.197 frag:rho=0.197,\
                             frag:rho=0.237 frag:rho=0.237,\
                             frag:x=0.815 frag:x=0.815,\
                             frag:x=1.015 frag:x=1.015,\
                             frag:y=0.0255 frag:y=0.0255,\
                             frag:y=0.0295 frag:y=0.0295\
                             }'] 

      genSeq.Pythia8.ShowerWeightNames += [ 
                             #"StringFlav:mesonCvector",  #  no parameter for having such variation
                             "frag:aLund=0.61",
                             "frag:aLund=0.75",
                             "frag:bLund=0.88",
                             "frag:bLund=1.1",
                             "frag:rFactC=1.9",
                             "frag:rFactC=1.7",
                             "frag:rFactC=2.1",
                             "frag:rFactB=1.05",
                             "frag:rFactB=1.01",
                             "frag:rFactB=1.03",
                             "frag:rFactB=1.07",
                             "frag:rFactB=1.09",
                             "frag:sigma=0.302",
                             "frag:sigma=0.368",
                             "frag:xi=0.073",
                             "frag:xi=0.089",
                             "frag:rho=0.197",
                             "frag:rho=0.237",
                             "frag:x=0.815",
                             "frag:x=1.015",
                             "frag:y=0.0255",
                             "frag:y=0.0295"
                             ]


