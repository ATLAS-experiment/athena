#!/usr/bin/env python

import subprocess
import argparse
import sys

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--inputFiles', type=str, nargs='+')
    args = parser.parse_args()

    if len(sys.argv) < 2:
        parser.print_help()
        sys.exit(1)

    processOutput = subprocess.Popen(["xAODChecker"]+args.inputFiles,universal_newlines=True,stdout=subprocess.PIPE,stderr=subprocess.DEVNULL)
    problemCounts = {}
    linePair = []
    lines = [line for line in processOutput.stdout]
    for line in lines:
        if "ERROR" not in line:
            continue
        if len(linePair)==2:
            errorMessage = linePair[0][1].replace("\n","")
            splitErrorMessage = errorMessage.split()
            errorType = splitErrorMessage[0]
            if (errorType == '252'):
                errorMessage = ' '.join([splitErrorMessage[1],'(X) !=',splitErrorMessage[4],' (Y)'])
            else:
                errorMessage = ' '.join([splitErrorMessage[i] for i in range(1,len(splitErrorMessage))])
            name = linePair[1][1].split()[5]
            if name not in problemCounts.keys():
                problemCounts[name] = {errorMessage:1}
            else:
                if errorMessage not in problemCounts[name].keys():
                    problemCounts[name][errorMessage] = 1
                else:
                    problemCounts[name][errorMessage] += 1
            linePair.clear()
        if len(linePair)!=2:
            linePair.append(line.split("TFileChecker.cxx:"))
            continue
    print("=" * 80)
    print("Number of impacted events and error message type for each container:")
    print("-" * 80)
    for typeName in problemCounts.keys():
        print(typeName.replace("\"",""))
        for errorMessage in problemCounts[typeName].keys():
            print("\t",problemCounts[typeName][errorMessage],'\t\t',errorMessage)
    print("=" * 80)

    if len(problemCounts)>0:
        return 1
    else:
        return 0

if __name__ == "__main__":
    ret = main()
    sys.exit(ret)
