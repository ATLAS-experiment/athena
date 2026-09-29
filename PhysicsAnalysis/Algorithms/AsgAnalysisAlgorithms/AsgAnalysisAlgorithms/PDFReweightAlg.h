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

#include <memory>
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

      /// \brief deleter for the LHAPDF::PDF objects, defined where the type is complete
      struct PDFDeleter
      {
        void operator() (const LHAPDF::PDF *pdf) const;
      };
      using PDFPtr = std::unique_ptr<const LHAPDF::PDF, PDFDeleter>;

      SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoKey{
          this, "EventInfoKey", "EventInfo", "EventInfo container to dump"};

      SG::ReadHandleKey<xAOD::TruthEventContainer> m_TruthEventKey{
          this, "TruthEvents", "TruthEvents", "TruthEvent container to read"};

      PDFPtr m_p0; //!
      std::vector<PDFPtr> m_p1_vars;
      Gaudi::Property<std::string> m_inPDF {this, "inPDFName", ""};
      Gaudi::Property<std::vector<std::string>> m_outPDF {this, "outPDFName", {}};
      Gaudi::Property<std::string> m_additionalPdfPath {this, "additionalPdfPath", ""};
      std::vector<SG::WriteDecorHandleKey<xAOD::EventInfo>> m_reweightKeys;
  };

}
#endif