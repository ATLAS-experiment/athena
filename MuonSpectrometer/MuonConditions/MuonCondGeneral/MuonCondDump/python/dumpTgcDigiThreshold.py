# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
import logging
def setupArgParser():
    from argparse import ArgumentParser

    parser = ArgumentParser()
    parser.add_argument("--inFile", help="Input file to be translated",
                                    default="TGC_Digitization_energyThreshold.dat")
    parser.add_argument("--outFile", help="Output JSON file",
                                     default="TGC_Digitization_energyThreshold.json")
    return parser

if __name__ == "__main__":
    args = setupArgParser().parse_args()
    from ROOT import PathResolver
    resolver = PathResolver()

    resolvedInFile = resolver.find_file(args.inFile, "DATAPATH")
    if not resolvedInFile or len(resolvedInFile) == 0:
        logging.error("Failed to find file {fileName}".format(fileName = args.inFile))
        exit(1)
    ### translate the station name indices into the string staiton name
    stationNameDict = {41:"T1F", 42:"T1E", 43:"T2F", 44:"T2E", 45:"T3F", 46:"T3E", 47:"T4F", 48:"T4E"}
    threshChambers = []
    with open (resolvedInFile, 'r') as inStream:
        for line in inStream:            
            line = line[0 : line.find("#")]
            tokens = [x.strip() for x in line.split(" ") if len(x.strip())]
            stationName = stationNameDict[int(tokens[0])]
            stationEta = tokens[1][tokens[1].find("+")+1: ]
            stationPhi = tokens[2]
            gasGap = tokens[3]
            isStrip = tokens[4]
            threshold = tokens[5]
            threshEntry ="\n".join([  "   {",
                                    "     \"station\"  : \"{name}\",".format(name = stationName),
                                    "     \"eta\"      : {eta},".format(eta = stationEta),
                                    "     \"phi\"      : {phi},".format(phi = stationPhi),
                                    "     \"gasGap\"   : {gap},".format(gap = gasGap),
                                    "     \"isStrip\"  : {strip},".format(strip = isStrip),
                                    "     \"threshold\": {threshold}".format(threshold=threshold),                                    
                                    "   }"])
            logging.debug(threshEntry)
            threshChambers+=[threshEntry]
    with open(args.outFile, 'w') as outStream:
        outStream.write("[\n")
        for num, thresh in enumerate(threshChambers, 1):
            outStream.write(thresh)
            if num != len(threshChambers): outStream.write(",")
            outStream.write("\n")
        outStream.write("]\n")
