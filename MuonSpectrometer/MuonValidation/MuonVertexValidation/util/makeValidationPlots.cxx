/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MSVtxPlotMaker.h"

int main(int argc, char *argv[]){
    // Produces the validation plots from the ntuple created by the MSVtxValidationAlg.
    // The path to the ntuple is the first argument, and the output directory (including a trailing "/") is the second argument. Optionally the tree name can be provided as the third argument.
    // Example: 
    // makeValidationPlots <path to ntuple> <output directory> <tree name>

    if (argc == 3){
        MSVtxPlotMaker plotter(argv[1], argv[2]);
        plotter.makePlots();
    }
    else if (argc == 4){
        MSVtxPlotMaker plotter(argv[1], argv[2], argv[3]);
        plotter.makePlots();
    }
    else{
        std::cout << "Invalid number of arguments. Please provide the input file path, the output directory, and optionally the tree name." << std::endl;
        return 1; 
    }

    return 0;
}
