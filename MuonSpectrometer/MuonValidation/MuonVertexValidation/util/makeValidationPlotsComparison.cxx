/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MSVtxPlotComparison.h"

int main(int argc, char *argv[]){
    // For each dataset to compare, input the directory path to the Histogram.root file created by `makeValidationPlots`, followed by the dataset label in the comparison plots.
    // The last argument specifies the output directory (including a trailing "/").
    // Example:
    // makeValidationPlotsComparison <path to dataset1 Histograms.root> <dataset1 label> <path to dataset2 Histograms.root> <dataset2 label> <path to output directory>
    //
    // When only two input datasets are provided, the comparisons plots also show a ratio plot.

    std::vector<std::string> datapaths;
    std::vector<std::string> labels;

    for (int i=1; i<argc-1; ++i){
        if (i%2 == 1) datapaths.push_back(argv[i]);
        else labels.push_back(argv[i]);
    }

    MSVtxPlotComparison plotter(datapaths, labels, argv[argc-1]);
    plotter.makeComparison();

    return 0;
}
