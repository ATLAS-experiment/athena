# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
import logging
def setupArgParser():
    from argparse import ArgumentParser

    parser = ArgumentParser()
    parser.add_argument("--inFile", help="Input file to be translated",
                                    default="sTGC_Digitization_EffChamber.dat")
    parser.add_argument("--outFile", help="Output JSON file",
                                     default="sTGC_Digitization_EffChamber.json")
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
    efficiencies = []
    with open (resolvedInFile, 'r') as inStream:
        for i, line in enumerate(inStream):
            ## Skip the header line
            if i == 0: continue
            tokens = [x.strip() for x in line.split(" ") if len(x.strip())]
            
            stationName =  "STL" if tokens[0] == 1 else "STS"
            stationEta = tokens[1]
            ### Why does the file container station eta == 0?
            if stationEta == 0:
                continue
            stationPhi = tokens[2]
            multiLayer = int(tokens[3]) + 1
            gasGap = tokens[4]
            effi = tokens[5]
            effiEntry ="\n".join([  "   {",
                                    "     \"station\"    : \"{name}\",".format(name = stationName),
                                    "     \"eta\"        : {eta},".format(eta = stationEta),
                                    "     \"phi\"        : {phi},".format(phi = stationPhi),
                                    "     \"multiLayer\" : {phi},".format(phi = multiLayer),                                    
                                    "     \"gasGap\"     : {gap},".format(gap = gasGap),
                                    "     \"efficiency\": {effi}".format(effi=effi),                                    
                                    "   }"])
            logging.debug(effiEntry)
            efficiencies+=[effiEntry]

    with open(args.outFile, 'w') as outStream:
        outStream.write("[\n")
        for num, jitter in enumerate(efficiencies, 1):
            outStream.write(jitter)
            if num != len(efficiencies): outStream.write(",")
            outStream.write("\n")
        outStream.write("]\n")
