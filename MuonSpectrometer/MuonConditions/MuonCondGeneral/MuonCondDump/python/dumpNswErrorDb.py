# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
import logging
import json
from itertools import product
def setupArgParser():
    from argparse import ArgumentParser

    parser = ArgumentParser()
    parser.add_argument("--outFilePreFix", help="Output JSON file",
                                     default="NswUncerts")
    parser.add_argument("--scaleError", help="Scale the uncertainties of the MM",type=float, default = 1. )
    parser.add_argument("--addErrorMMClusterTime", help="add fixed error to the uncertainties of the MM",type=float, default = 0.)
    return parser

if __name__ == "__main__":
    args = setupArgParser().parse_args()

    dbEntriesMMASide = []
    dbEntriesMMCSide = []
    
    for stationName, stationEta, stationPhi, multilayer, gasGap, author in product([ "MMS", "MML"], [-2,-1,1,2], range(1,9), [1,2],range(1,5), [0,65,66]):

        #CTP for MMG
        if author == 66:
            #----------Side A
            if stationEta > 0: 
                if abs(stationEta) == 1: 
                    constant = args.scaleError * 0.163
                    linear   = args.scaleError * 0.0
                    quad     = args.scaleError * 1.312
                elif abs(stationEta) == 2: 
                    constant = args.scaleError * 0.113
                    linear   = args.scaleError * 0.0
                    quad     = args.scaleError * 1.258
                    #constant = args.scaleError * 0.0475
                    #linear   = args.scaleError * 0.0
                    #quad     = args.scaleError *  1.796

            #----------Side C
            else: 

                if abs(stationEta) == 1: 
                    constant = args.scaleError * 0.127
                    linear   = args.scaleError * 0.0
                    quad     = args.scaleError * 1.192
                elif abs(stationEta) == 2 and stationName == "MML":
                    constant = args.scaleError * (-0.496)
                    linear   = args.scaleError * 0.0
                    quad     = args.scaleError * 4.086
                elif abs(stationEta) == 2 and stationName == "MMS":
                    constant = args.scaleError * 0.319
                    linear   = args.scaleError * 0.0
                    quad     = args.scaleError * 0.117

        else:
           #Keep original values for centroid
           constant = args.scaleError * 0.1796
           linear   = args.scaleError * 0.0
           quad     = args.scaleError * 1.9821


        minStrip = 1
        maxStrip = (5 if abs(stationEta)==1 else 3)*1024
        
        calibEntry = dict({"station": stationName, "eta":stationEta, "phi": stationPhi, "multilayer": multilayer, "gasGap": gasGap, "minStrip": minStrip,"maxStrip": maxStrip, "modelName":"tanThetaPolynomial" ,"modelPars": [constant, linear, quad], "clusterAuthor": author})
        logging.debug(calibEntry)
        if(stationEta<0):
            dbEntriesMMCSide.append(calibEntry)
        else:
            dbEntriesMMASide.append(calibEntry)


#    for stationName, stationEta, stationPhi, multilayer, gasGap, author in product([ "MMS", "MML"], [-2,-1,1,2], range(1,9), [1,2],range(1,5), [66]):
#        constant = args.scaleError* 0.0615 + args.addErrorMMClusterTime
#        linear =  args.scaleError * 0
#        quad =  args.scaleError * 0.275
#
#        minStrip = 1
#        maxStrip = (5 if abs(stationEta)==1 else 3)*1024
#        
#        calibEntry = dict({"station": stationName, "eta":stationEta, "phi": stationPhi, "multilayer": multilayer, "gasGap": gasGap, "minStrip": minStrip,"maxStrip": maxStrip, "modelName":"tanThetaPolynomial" ,"modelPars": [constant, linear, quad], "clusterAuthor": author})
#        logging.debug(calibEntry)
#        if(stationEta<0):
#            dbEntriesMMCSide.append(calibEntry)
#        else:
#            dbEntriesMMCSide.append(calibEntry)
    
    
    dbEntriessTGCASide = []
    dbEntriessTGCCSide = []
    maxStripStgc = dict({"QL1": 408 ,"QL2": 366,"QL3": 353,"QS1": 406, "QS2": 365, "QS3": 307})
    for stationName, stationEta, stationPhi, multilayer, gasGap in product([ "STL", "STS"], [-3, -2,-1,1,2,3], range(1,9), [1,2],range(1,5)):
        chamberKey = f"Q{stationName[2]}{abs(stationEta)}"
        minStrip = 1
        maxStrip = maxStripStgc[chamberKey]

        if abs(stationEta) == 1:
            const  = args.scaleError * 0.261
            linear = args.scaleError * 0
            quad   = args.scaleError * (-0.1949)
        elif abs(stationEta) == 2:
            const  = args.scaleError * 0.228
            linear = args.scaleError * 0
            quad   = args.scaleError * (0.0252)
        elif abs(stationEta) == 3:
            const  = args.scaleError * 0.3158
            linear = args.scaleError * 0
            quad   = args.scaleError * (-0.1223)
        
        author = 3 # centroid
        calibEntry = dict({"station": stationName, "eta":stationEta, "phi": stationPhi, "multilayer": multilayer, "gasGap": gasGap, "minStrip": minStrip,"maxStrip": maxStrip, "modelName":"tanThetaPolynomial" ,"modelPars": [const, linear, quad], "clusterAuthor": author})
        logging.debug(calibEntry)
        if(stationEta<0):
            dbEntriessTGCCSide.append(calibEntry)
        else:
            dbEntriessTGCASide.append(calibEntry)
        
#        author = 4 #caruana 
#        const = args.scaleError * 0.175
#        linear = args.scaleError * 0
#        quad = args.scaleError * 0.201
#        calibEntry = dict({"station": stationName, "eta":stationEta, "phi": stationPhi, "multilayer": multilayer, "gasGap": gasGap, "minStrip": minStrip,"maxStrip": maxStrip, "modelName": "tanThetaPolynomial" ,"modelPars": [const, linear, quad], "clusterAuthor": author})
#        logging.debug(calibEntry)
#        if(stationEta<0):
#            dbEntriessTGCCSide.append(calibEntry)
#        else:
#            dbEntriessTGCASide.append(calibEntry)


    #Global json database for quick tests on reco
    outFileName = f"{args.outFilePreFix}_scale_x{args.scaleError}_mmConstOffset_{args.addErrorMMClusterTime}.json"
    with open(outFileName,'w') as outStream:
        json.dump(dbEntriesMMASide + dbEntriesMMCSide  +dbEntriessTGCASide + dbEntriessTGCCSide, outStream)
    
    #Individual databases to be converted to cool
#    outFileName = f"{args.outFilePreFix}_MMonly_ASide_scale_x{args.scaleError}_mmConstOffset_{args.addErrorMMClusterTime}.json"
#    with open(outFileName,'w') as outStream:
#        json.dump(dbEntriesMMASide, outStream)
#    
#    outFileName = f"{args.outFilePreFix}_MMonly_CSide_scale_x{args.scaleError}_mmConstOffset_{args.addErrorMMClusterTime}.json"
#    with open(outFileName,'w') as outStream:
#        json.dump(dbEntriesMMCSide, outStream)
#    
#    outFileName = f"{args.outFilePreFix}_sTGConly_ASide_scale_x{args.scaleError}.json"
#    with open(outFileName,'w') as outStream:
#        json.dump(dbEntriessTGCASide, outStream)
#    
#    outFileName = f"{args.outFilePreFix}_sTGConly_CSide_scale_x{args.scaleError}.json"
#    with open(outFileName,'w') as outStream:
#        json.dump(dbEntriessTGCCSide, outStream)
