/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Michael Holzbock


//
// includes
//

#include <regex>
#include <AsgAnalysisAlgorithms/MetadataHistAlg.h>
#include <TH1.h>

//
// method implementations
//

namespace CP
{

  StatusCode MetadataHistAlg ::
  initialize ()
  {
    if (m_histogramName.empty())
    {
      ANA_MSG_ERROR ("histogram name should not be empty");
      return StatusCode::FAILURE;
    }

    if (m_dataType.empty())
    {
      ANA_MSG_ERROR ("dataType should not be empty");
      return StatusCode::FAILURE;
    }

    if (m_campaign.empty())
    {
      ANA_MSG_ERROR ("campaign should not be empty");
      return StatusCode::FAILURE;
    }

    if (m_mcChannelNumber.empty())
    {
      ANA_MSG_ERROR ("mcChannelNumber should not be empty");
      return StatusCode::FAILURE;
    }

    if (m_etag.empty())
    {
      ANA_MSG_ERROR ("etag should not be empty");
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }



  StatusCode MetadataHistAlg ::
  execute ()
  {
    if (!m_firstEvent)
    {
      return StatusCode::SUCCESS;
    }

    m_firstEvent = false;

    ANA_CHECK (book (TH1F (m_histogramName.value().c_str(), "Sample metadata",  4, 0.0, 4.0)));
    TH1 *histogram = hist (m_histogramName);

    // Please keep the bin order unchanged
    histogram->GetXaxis()->SetBinLabel(1, m_dataType.value().c_str());
    histogram->GetXaxis()->SetBinLabel(2, m_campaign.value().c_str());
    histogram->GetXaxis()->SetBinLabel(3, m_mcChannelNumber.value().c_str());
    histogram->GetXaxis()->SetBinLabel(4, m_etag.value().c_str());

    return StatusCode::SUCCESS;
  }


  StatusCode MetadataHistAlg ::
  finalize ()
  {
    return StatusCode::SUCCESS;
  }
}
