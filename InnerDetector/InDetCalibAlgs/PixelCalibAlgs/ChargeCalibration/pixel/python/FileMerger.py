# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# This function reads the layers that have been recalibrated (using the PixelCalibrationConfig python) and creates a merge file of all of them
# the "layers" aregument is an array i.e: ["Blayer","L1","L2","disk"] meaning that it will look for files like "calibration_Blayer.txt"... etc

import os.path

def MergeCalibFiles(layers):
    if(len(layers)==0): 
        print("MergeCalibFiles - ERROR no layer provided.")
        return 1
    mergedDict = {}
    for i in layers:
        mergedDict.update(readFile(("calibration_%s.txt" % i)))
        
    
    mergedDict = dict(sorted(mergedDict.items()))
    f = open("calibration_merged.txt", "w")
    for value in mergedDict.values():
        f.write(value)
    f.close()
    
    return 0
        
        
# Function to read the calibration_XXXXX.txt file of each layer
def readFile(name):
    mydict = {}
    if not os.path.exists(name):
        print("MergeCalibFiles - ERROR File name %22s not found" % name)
        return mydict
    
    with open(name) as fp:
        lines = fp.readlines()
        mod = -1
        for line in lines:
            # Module names for Pixel layers starts with D or L
            if line.startswith("L") or line.startswith("D"):
                line = line.rstrip("\n")
                splittedline = line.split(" ")
                
                if int(splittedline[1]) in mydict.keys():
                    print("MergeCalibFiles - ERROR Module key exists already - Contact pixel offline team")
                    exit(1)
                else:
                    mydict[int(splittedline[1])] = splittedline[0]+' '+splittedline[1]+"\n"
                    mod = int(splittedline[1])
            
            # Pixel frontends start with letter I  
            elif line.startswith("I"):
                mydict[mod] = mydict[mod] + line
            else:
                print("MergeCalibFiles - ERROR Line is not a module or a frontend - Contact pixel offline team")
                exit(1)
    
    return mydict        

# Just used for testing - Experts only                
if __name__ == "__main__":
    
    MergeCalibFiles(["Blayer","L1","L2","disk"])
    exit(0)    


