/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Yi Yu <yiyu@cern.ch>, Xinzhe Liu, Thomas Strebler <thomas.strebler@cern.ch>

#ifndef ASG_ANALYSIS_ALGORITHMS__PDFREWEIGHT__ALG_H
#define ASG_ANALYSIS_ALGORITHMS__PDFREWEIGHT__ALG_H

#include <AnaAlgorithm/AnaReentrantAlgorithm.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>
#include <AsgTools/PropertyWrapper.h>


#include <xAODEventInfo/EventInfo.h>
#include "xAODTruth/TruthEventContainer.h"

#include <vector>

namespace LHAPDF{
  class PDF;
}


namespace CP
{
  /// \brief An algorithm for the PDF reweighting
  class PDFReweightAlg : public EL::AnaReentrantAlgorithm
  {
    public:
        using EL::AnaReentrantAlgorithm::AnaReentrantAlgorithm;
        virtual StatusCode initialize() final;
        virtual StatusCode execute(const EventContext &ctx) const final; 

    private:
      
      SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoKey{
          this, "EventInfoKey", "EventInfo", "EventInfo container to dump"};

      SG::ReadHandleKey<xAOD::TruthEventContainer> m_TruthEventKey{
          this, "TruthEvents", "TruthEvents", "TruthEvent container to read"};

      LHAPDF::PDF* m_p0=nullptr; //!
      std::vector<LHAPDF::PDF*> m_p1_vars;
      Gaudi::Property<std::string> m_inPDF {this, "inPDFName", ""};
      Gaudi::Property<std::vector<std::string>> m_outPDF {this, "outPDFName", {}};
      Gaudi::Property<std::string> m_additionalPdfPath {this, "additionalPdfPath", ""};
      std::vector<SG::WriteDecorHandleKey<xAOD::EventInfo>> m_reweightKeys;
  };

}
#endif