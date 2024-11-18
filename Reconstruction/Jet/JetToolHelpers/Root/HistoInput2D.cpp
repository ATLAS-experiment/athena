/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <iostream>

#include "JetToolHelpers/HistoInput2D.h"

namespace JetHelper {
HistoInput2D::HistoInput2D(const std::string& name)
    : HistoInputBase{name}
{ }

StatusCode HistoInput2D::initialize()
{
    // First deal with the input variables
    // Make sure we haven't already configured the input variables

    ATH_CHECK( m_varTool1.retrieve() );
    ATH_CHECK( m_varTool2.retrieve() );

    if (m_varTool1.empty() || m_varTool2.empty())
    {
        ATH_MSG_ERROR("Failed to create input variable(s)");        
        return StatusCode::FAILURE;
    }

    // Now deal with the histogram
    // Make sure we haven't already retrieved the histogram
    if (m_hist != nullptr)
    {
        ATH_MSG_ERROR("The histogram already exists");       
        return StatusCode::FAILURE;
    }

    if (!readHistoFromFile())
    {
        ATH_MSG_ERROR("Failed while reading histogram from root file");	
	return StatusCode::FAILURE;
    }

    if (!m_hist)
    {
        ATH_MSG_ERROR("Histogram pointer is empty after reading from file");
        return StatusCode::FAILURE;
    }

    if (m_hist->GetDimension() != 2)
    {
        ATH_MSG_ERROR("Read the specified histogram, but it has a dimension of " << m_hist->GetDimension() << " instead of the expected 2");        
        return StatusCode::FAILURE;
    }

    // Determine the histogram interpolation strategy
    if (m_interpStr == "")
    {
        ATH_MSG_FATAL("No histogram interpolation type was specified. Aborting.");
        return StatusCode::FAILURE;
    }
    else if (m_interpStr == "Full")
        m_interpNum = InterpType::Full;
    else if (m_interpStr == "None")
        m_interpNum = InterpType::None;
    else if (m_interpStr == "OnlyX")
        m_interpNum = InterpType::OnlyX;
    else if (m_interpStr == "OnlyY")
        m_interpNum = InterpType::OnlyY;
    else
    {
        ATH_MSG_FATAL("Unrecognized interpolation type: " << m_interpStr << " --> options are None/Full/OnlyY/OnlyX");
        return StatusCode::FAILURE;
    }

    // Pre-cache the histogram file in 1D projections if relevant (depends on m_interpType)
    if (m_interpNum == InterpType::OnlyX || m_interpNum == InterpType::OnlyY)
    {
        ATH_CHECK(cacheProjections());
    }

    // TODO
    // We have both, set the dynamic range of the input variable according to histogram range
    // Low edge of first bin (index 1, as index 0 is underflow)
    // High edge of last bin (index N, as index N+1 is overflow)
    //m_inVar1.SetDynamicRange(m_hist->GetXaxis()->GetBinLowEdge(1),m_hist->GetXaxis()->GetBinUpEdge(m_hist->GetNbinsX()));
    //m_inVar2.SetDynamicRange(m_hist->GetYaxis()->GetBinLowEdge(1),m_hist->GetYaxis()->GetBinUpEdge(m_hist->GetNbinsY()));

    return StatusCode::SUCCESS;
}

float HistoInput2D::getValue(const xAOD::Jet& jet, const JetContext& event) const
{
    float varValue1 {m_varTool1->getValue(jet,event)};

    varValue1 = enforceAxisRange(*m_hist->GetXaxis(),varValue1);
    
    float varValue2 {m_varTool2->getValue(jet,event)};

    varValue2 = enforceAxisRange(*m_hist->GetYaxis(),varValue2);
    
    switch (m_interpNum)
    {
        case InterpType::OnlyX:
            // Determine the y-bin and use the cached projection to interpolate x
            return m_cachedProj.at(m_hist->GetYaxis()->FindBin(varValue2))->Interpolate(varValue1);
        case InterpType::OnlyY:
            // Determine the x-bin and use the cached projection to interpolate y
            return m_cachedProj.at(m_hist->GetXaxis()->FindBin(varValue1))->Interpolate(varValue2);
        case InterpType::Full:
            // Full interpolation using default HistoInputBase reading function
            return readFromHisto(varValue1,varValue2);
        case InterpType::None:
            // No interpolation at all
            return m_hist->GetBinContent(m_hist->GetXaxis()->FindBin(varValue1),m_hist->GetYaxis()->FindBin(varValue2));
        default:
            // Should never get here due to previous checks
            ATH_MSG_ERROR("Unsupported interpolation type");
            return 0;
    }
}

StatusCode HistoInput2D::cacheProjections()
{
    // Project histogram from 2D to 1D
    // Intentionally include underflow and overflow bins
    // This keeps the same indexing scheme as root
    // Avoids confusion and problems later at cost of a small amount of RAM

    //TH2* localHist = dynamic_cast<TH2*>(fullHistogram);
    TH2* localHist = dynamic_cast<TH2*>(m_hist.get());
    if (!localHist)
    {
        ATH_MSG_FATAL("Failed to convert histogram to a TH2, please check inputs.");
        return StatusCode::FAILURE;
    }
    switch (m_interpNum)
    {
        case InterpType::OnlyX:
            for (Long64_t binY = 0; binY < localHist->GetNbinsY()+1; ++binY)
            {
                // Single bin of Y, interpolate across X
                m_cachedProj.emplace_back(localHist->ProjectionX(Form("projx_%lld",binY),binY,binY));
            }
            break;
        case InterpType::OnlyY:
            for (Long64_t binX = 0; binX < localHist->GetNbinsX()+1; ++binX)
            {
                // Single bin of X, interpolate across Y
                m_cachedProj.emplace_back(localHist->ProjectionY(Form("projy_%lld",binX),binX,binX));
            }
            break;
        default:
            ATH_MSG_FATAL("The interpolation type is not supported for caching");
            return StatusCode::FAILURE;
    }

    // Ensure that ROOT doesn't try to take posession
    for (auto& hist : m_cachedProj)
    {
        hist->SetDirectory(nullptr);
    }

    // All done
    return StatusCode::SUCCESS;
}

bool HistoInput2D::runUnitTests() const
{
    // TODO
    return false;
}

} // namespace JetHelper
