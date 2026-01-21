/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TH3.h"
#include "JetToolHelpers/HistoInput3D.h"

namespace JetHelper {
HistoInput3D::HistoInput3D(const std::string& name)
    : HistoInputBase{name}
{ }

StatusCode HistoInput3D::initialize()
{
    // First deal with the input variables
    // Make sure we haven't already configured the input variables

    ATH_CHECK( m_varTool1.retrieve() );
    ATH_CHECK( m_varTool2.retrieve() );
    ATH_CHECK( m_varTool3.retrieve() );

    if (m_varTool1.empty() || m_varTool2.empty() || m_varTool3.empty())
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

    if (m_hist->GetDimension() != 3)
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
    else if (m_interpStr == "OnlyZ")
        m_interpNum = InterpType::OnlyZ;
    else
    {
        ATH_MSG_FATAL("Unrecognized interpolation type: " << m_interpStr << " --> options are None/Full/OnlyY/OnlyX");
        return StatusCode::FAILURE;
    }

    // Pre-cache the histogram file in 1D projections if relevant (depends on m_interpType)
    if (m_interpNum == InterpType::OnlyX || m_interpNum == InterpType::OnlyY || m_interpNum == InterpType::OnlyZ)
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

float HistoInput3D::getValue(const xAOD::Jet& jet, const JetContext& event) const
{
    float varValue1 {m_varTool1->getValue(jet,event)};

    varValue1 = enforceAxisRange(*m_hist->GetXaxis(),varValue1);
    
    float varValue2 {m_varTool2->getValue(jet,event)};

    varValue2 = enforceAxisRange(*m_hist->GetYaxis(),varValue2);

    float varValue3 {m_varTool3->getValue(jet,event)};

    varValue3 = enforceAxisRange(*m_hist->GetZaxis(),varValue3);
    
    switch (m_interpNum)
    {
        case InterpType::OnlyX:
            // Determine the y- and z-bin and use the cached projection to interpolate x
            return m_cachedProj2.at(m_hist->GetYaxis()->FindBin(varValue2)).at(m_hist->GetZaxis()->FindBin(varValue3))->Interpolate(varValue1);
        case InterpType::OnlyY:
            // Determine the x- and zbin and use the cached projection to interpolate y
            return m_cachedProj2.at(m_hist->GetXaxis()->FindBin(varValue1)).at(m_hist->GetZaxis()->FindBin(varValue3))->Interpolate(varValue2);
        case InterpType::OnlyZ:
            // Determine the x- and y-bin and use the cached projection to interpolate z
            return m_cachedProj2.at(m_hist->GetXaxis()->FindBin(varValue1)).at(m_hist->GetYaxis()->FindBin(varValue2))->Interpolate(varValue3);
        case InterpType::Full:
            // Full interpolation using default HistoInputBase reading function
            return readFromHisto(varValue1,varValue2,varValue3);
        case InterpType::None:
            // No interpolation at all
            return m_hist->GetBinContent(m_hist->GetXaxis()->FindBin(varValue1),m_hist->GetYaxis()->FindBin(varValue2),m_hist->GetZaxis()->FindBin(varValue3));
        default:
            // Should never get here due to previous checks
            ATH_MSG_ERROR("Unsupported interpolation type");
            return 0;
    }
}

StatusCode HistoInput3D::cacheProjections()
{
    // Project histogram from 2D to 1D
    // Intentionally include underflow and overflow bins
    // This keeps the same indexing scheme as root
    // Avoids confusion and problems later at cost of a small amount of RAM

    //TH2* localHist = dynamic_cast<TH2*>(fullHistogram);
    TH3* localHist = dynamic_cast<TH3*>(m_hist.get());
    if (!localHist)
    {
        ATH_MSG_FATAL("Failed to convert histogram to a TH3, please check inputs.");
        return StatusCode::FAILURE;
    }
    switch (m_interpNum)
    {
        case InterpType::OnlyX:
            for (Long64_t binY = 0; binY <= localHist->GetNbinsY()+1; ++binY)
            {
    		for (Long64_t binZ = 0; binZ <= localHist->GetNbinsZ()+1; ++binZ)
    		{
        		// Single bin of Y and Z, interpolate across X
        		m_cachedProj.emplace_back(localHist->ProjectionX(Form("projx_%lld_%lld",binY,binZ),binY,binY,binZ,binZ));
        	}
		m_cachedProj2.emplace_back(std::move(m_cachedProj));
		m_cachedProj.clear();
            }
            break;
        case InterpType::OnlyY:
            for (Long64_t binX = 0; binX <= localHist->GetNbinsX()+1; ++binX)
            {
    		for (Long64_t binZ = 0; binZ <= localHist->GetNbinsZ()+1; ++binZ)
	    	{
        		// Single bin of X and Z, interpolate across Y
  		      	m_cachedProj.emplace_back(localHist->ProjectionY(Form("projy_%lld_%lld",binX,binZ),binX,binX,binZ,binZ));
        	}
		m_cachedProj2.emplace_back(std::move(m_cachedProj));
		m_cachedProj.clear();
            }
            break;
        case InterpType::OnlyZ:
            for (Long64_t binX = 0; binX <= localHist->GetNbinsX()+1; ++binX)
            {
    		for (Long64_t binY = 0; binY <= localHist->GetNbinsY()+1; ++binY)
    		{ 
    	    		// Single bin of X and Y, interpolate across Z
    	    		m_cachedProj.emplace_back(localHist->ProjectionZ(Form("projz_%lld_%lld",binX,binY),binX,binX,binY,binY));
    		} 
		m_cachedProj2.emplace_back(std::move(m_cachedProj));
		m_cachedProj.clear(); 
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

bool HistoInput3D::runUnitTests() const
{
    // TODO
    return false;
}

} // namespace JetHelper
