/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <iostream>
#include <fstream>
#include <stdio.h>

#include "JetToolHelpers/HistoInputBase.h"

#include "TFile.h"

namespace JetHelper {

HistoInputBase::HistoInputBase(const std::string& name)
     : asg::AsgTool(name)
     , m_hist{nullptr}
 { }

bool HistoInputBase::readHistoFromFile()
{
    // Open the input file
    std::string local_path=static_cast<std::string> (m_fileName);
    std::string local_hist=static_cast<std::string> (m_histName);

    TFile inputFile(local_path.c_str(),"READ");
    if (inputFile.IsZombie())
    {
        ATH_MSG_WARNING("Failed to open the file to read: " << m_fileName); 
        inputFile.Close();
        return false;
    }

    // Get the input object
    TObject* inputObject = inputFile.Get(local_hist.c_str());
    if (!inputObject)
    {
        ATH_MSG_WARNING("Failed to retreive the requested histogram \"" << m_histName << "\" from the file: " << m_fileName);
        inputFile.Close();
        return false;
    }

    // Confirm that the input object is a histogram
    m_hist = std::unique_ptr<TH1>(dynamic_cast<TH1*>(inputObject));
    if (!m_hist)
    {
        ATH_MSG_WARNING("Failed to convert the retrieved input to a histogram \"" << m_histName << "\" from the file: " << m_fileName);
        inputFile.Close();
        return false;
    }

    // Successfully retrieved the histogram
    m_hist->SetDirectory(0);
    inputFile.Close();
    return true;
}

double HistoInputBase::enforceAxisRange(const TAxis& axis, const double inputValue) const
{
    // edgeOffset should be chosen to be above floating point precision, but negligible compared to the bin size
    // An offset of edgeOffset*binWidth is therefore irrelevant for physics as the values don't change fast (but avoids edge errors)
    static constexpr double edgeOffset {1.e-4};

    // Root histogram binning:
    //  bin 0 = underflow bin
    //  bin 1 = first actual bin
    //  bin N = last actual bin
    //  bin N+1 = overflow bin
    const int numBins {axis.GetNbins()};
    const int binIndex {axis.FindFixBin(inputValue)};

    if (binIndex < 1)
        // Return just inside the range of the first real bin
        return axis.GetBinLowEdge(1) + edgeOffset*axis.GetBinWidth(1);
    if (binIndex > numBins)
        // Return just inside the range of the last real bin
        // Don't use the upper edge as floating point can make it roll into the next bin (which is overflow)
        // Instead, use the last bin width to go slightly within the boundary
        return axis.GetBinLowEdge(numBins) + (1-edgeOffset)*axis.GetBinWidth(numBins);
    return inputValue;
}

double HistoInputBase::readFromHisto(const double X, const double Y, const double Z) const
{
    // TODO: extend this to have different reading strategies
    const int nDim {m_hist->GetDimension()};
    if (nDim == 1)
        return m_hist->Interpolate(X);
    else if (nDim == 2)
        return m_hist->Interpolate(X,Y);
    else if (nDim == 3)
        return m_hist->Interpolate(X,Y,Z);
    // Shouldn't reach here due to previous checks
    throw std::runtime_error("Unexpected number of dimensions of histogram: " + std::to_string(nDim));
}
} // namespace JetHelper
