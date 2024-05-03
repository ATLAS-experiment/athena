#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from ROOT import xAOD, TFile
import os
import csv
from argparse import ArgumentParser
import numpy as np
from numpy import array
from InDetMeasurementUtilities.CSV_InDetImporter import getCSVFilename
class CSVDumper:

    def __init__(self, inputAOD, outputDir, dict_variables_types, treename="CollectionTree", nEvents=-1):

        xAOD.Init()    # Setting up ROOT tools
        self.tree            = xAOD.MakeTransientTree( TFile(inputAOD), treename )
        self.n_entries       = self.tree.GetEntriesFast()
        self.outputDir       = outputDir

        if nEvents < 0 or nEvents > self.n_entries:   # Getting number of events to run
            self.nEvents = self.n_entries
        else:
            self.nEvents = nEvents
        print(f"Running on {self.nEvents} events")

        os.system("mkdir -p "+self.outputDir)   # Creating directory to store files

        # Container and Variables with their types that should be stored in csv format. Should be something like:
        #{
        #    "ContainerName": { "var1": "int",
        #                       "var2": "float",
        #                     }
        #}
        self.dict_variables_types = dict_variables_types

    def WriteCSV(self, filename, dictionary):

        with open(filename, "w") as out:
            writer = csv.writer(out)
            writer.writerow(dictionary.keys())
            writer.writerows( zip(*dictionary.values()) )
        print(f"New file saved: {filename}")

    def ArrayFloat3_to_CppArray(self, ArrayFloat3): # Check ArrayFloat3 in https://gitlab.cern.ch/atlas/athena/-/blob/master/Event/xAOD/xAODInDetMeasurement/xAODInDetMeasurement/versions/SpacePoint_v1.h
        try:
            arr = ArrayFloat3.data()
        except Exception:
            arr = ArrayFloat3    
        arr.reshape((3,))
        return list(arr)


    def ProcessEvent(self, evt):

        # Getting aux variables based on the implementation of https://gitlab.cern.ch/atlas/athena/-/commit/fc5baf9fd2bb28c56f115fc2107a3ff159f1945d
        print("--> Event "+str(evt))
        self.tree.GetEntry(evt)
        EventNumber = self.tree.EventInfo.mcEventNumber()

        # Check whether the container exists
        for container in self.dict_variables_types.keys():
            dict_lists = {}
            dict_container = self.dict_variables_types[container]
            try:
                tp  = getattr(self.tree, container)
            except Exception:
                print(".. Missing ", container)
                continue

            for var,fmt in dict_container.items():
                try:
                    sp  = tp.getConstDataSpan[ fmt ]( var) 
                except Exception:
                    # This is for the case arrays are either empty or have some trouble when accessed via getConstDataSpan. Used in excepction as makes the code slower
                    print("getConstDataSpan failed for variable ",var,fmt)
                    sp  = [ getattr(element, var)() for element in tp ]

                try:
                    # Convert list of std::vector<double> to the list of lists
                    list_of_lists = [list(std_vector) for std_vector in sp] 
                    # Find the length of the longest vector
                    max_len = max(len(ll) for ll in list_of_lists)
                    # extend lists to the length of max_len
                    for ll in list_of_lists:
                        ll.extend([np.nan] * (max_len - len(ll)))
                except Exception:
                    list_of_lists = sp  # the sp is not iterable        

                # Convert the list of lists to a NumPy array
                sp = np.array(list_of_lists)

                if "ArrayFloat3" in fmt and len(sp) > 0:  # Needs extra coding when dealing with xAOD::ArrayFloat3 instead of standard C++ array
                    sp = array( list( map(self.ArrayFloat3_to_CppArray, sp) ) )

                if ("unsigned char" in fmt or "uint8" in fmt) and len(sp) > 0:
                    sp = sp.view(np.int32)

                if sp.ndim == 1:
                    dict_lists[var] = sp
                else:
                    # Then we have an array. Each element of the array will be a column in the csv file. [We want things flat]
                    for column in range(sp.shape[1]):
                        dict_lists[var+f"_at{column}"] = sp.T[column]
            
            self.WriteCSV( filename=getCSVFilename(self.outputDir, container, EventNumber), dictionary=dict_lists )


    def Run(self):

        for evt in range(self.nEvents):
            self.ProcessEvent(evt)


if __name__ == "__main__":


    parser = ArgumentParser()
    parser.add_argument('--inputAOD', type=str, default="", help="Input AOD.root file")
    parser.add_argument('--outputDir', type=str, default="", help="Output directory")
    parser.add_argument('--treename', type=str, default="CollectionTree")
    parser.add_argument('--nEvents', type=int, default=-1, help="Number of events")
    parser.add_argument('--CSV_DictFormats', type=str, default="InDetMeasurementUtilities.CSV_DictFormats", 
                        help="Name of the python file (ex. local CSV_DictFormats) with variable list. Default: InDetMeasurementUtilities.CSV_DictFormats" )
    parser.add_argument('--renames', type=str, default="", help="Names of collections other than default eg. InDetTrackParticles=NewColl,ITkPixelClusters=NewClusters,...")

    args = parser.parse_args()

    import importlib
    module = importlib.import_module(args.CSV_DictFormats)
    CSV_DictFormats = module.CSV_DictFormats

    if args.inputAOD == "":
        raise Exception("No inputAOD was provided!")

    if args.outputDir == "":
        raise Exception("No outputDir was provided!")

    if args.renames != "":
        for n in args.renames.split(","):
            old,new = n.split("=")
            CSV_DictFormats[new] = CSV_DictFormats[old]
            del CSV_DictFormats[old]
            print("Instead of", old, "will use", new)

    Dumper = CSVDumper(inputAOD=args.inputAOD,
                       outputDir=args.outputDir,
                       dict_variables_types=CSV_DictFormats,
                       treename=args.treename,
                       nEvents=args.nEvents)
    Dumper.Run()
