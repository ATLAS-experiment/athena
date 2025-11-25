/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Michael Holzbock


#ifndef ASG_ANALYSIS_ALGORITHMS__METADATA_HIST_ALG_H
#define ASG_ANALYSIS_ALGORITHMS__METADATA_HIST_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/PropertyWrapper.h>

namespace CP
{
  /// \brief Dump metadata information into a histogram

  class MetadataHistAlg final : public EL::AnaAlgorithm
  {
    /// \brief standard constructor
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   out of memory II
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    virtual ::StatusCode initialize () override;
    virtual ::StatusCode execute () override;
    virtual ::StatusCode finalize () override;

    /// \brief the name of the histogram to use
  private:
    Gaudi::Property<std::string> m_histogramName {this, "histogramName", "metadata", "the name of the output histogram"};

    /// \brief the data type of the sample to store
  private:
    Gaudi::Property<std::string> m_dataType {this, "dataType", "", "dataType"};

    /// \brief the campaign of the sample to store
  private:
    Gaudi::Property<std::string> m_campaign {this, "campaign", "", "campaign"};

    /// \brief the mc channel number of the sample to store
  private:
    Gaudi::Property<std::string> m_mcChannelNumber {this, "mcChannelNumber", "", "mcChannelNumber"};

    /// \brief the e-tag of the sample to store
  private:
    Gaudi::Property<std::string> m_etag {this, "etag", "", "etag"};


    /// \brief whether the next event will be the first event
  private:
    bool m_firstEvent = true;
  };
}

#endif
