# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
import logging
def setupArgParser():
    from argparse import ArgumentParser

    parser = ArgumentParser()
    parser.add_argument("--inFile", help="Input file to be translated",
                                    default="TGC_Digitization_timejitter.dat")
    parser.add_argument("--outFile", help="Output JSON file",
                                     default="TGC_Digitization_timejitter.json")
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
    jitterFields = []
    with open (resolvedInFile, 'r') as inStream:
        for line in inStream:
            tokens = [x.strip() for x in line.split(" ") if len(x.strip())]
            
            angle = tokens[0]
            nBins = tokens[1]
            timeJitters= tokens[2:]
            jitterEntry ="\n".join([  "   {",
                                    "     \"angle\"   : {angle},".format(angle = angle),
                                    "     \"nBins\"   : {nBins},".format(nBins = nBins),
                                    "     \"values\"  : [{timeJitters}]".format(timeJitters = ",".join(timeJitters)),
                                    "   }"])
            logging.debug(jitterEntry)
            jitterFields+=[jitterEntry]

    with open(args.outFile, 'w') as outStream:
        outStream.write("[\n")
        for num, jitter in enumerate(jitterFields, 1):
            outStream.write(jitter)
            if num != len(jitterFields): outStream.write(",")
            outStream.write("\n")
        outStream.write("]\n")
