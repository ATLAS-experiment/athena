/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Yi Yu <yiyu@cern.ch>, Xinzhe Liu, Thomas Strebler <thomas.strebler@cern.ch>

#include "AsgAnalysisAlgorithms/PDFReweightAlg.h"
#include <LHAPDF/LHAPDF.h>
#include "LHAPDF/Reweighting.h"

#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>

#include <xAODTruth/TruthEvent.h>

namespace CP
{ 
  StatusCode PDFReweightAlg::initialize()
  { 

    ATH_CHECK(m_EventInfoKey.initialize());
    ATH_CHECK(m_TruthEventKey.initialize());

    m_p0 = LHAPDF::mkPDF(m_inPDF);
    for (const auto& pdfstring : m_outPDF) {
      m_p1_vars.push_back(LHAPDF::mkPDF(pdfstring));
    }

    for(auto temp_name : m_outPDF){
      if (auto pos = temp_name.find('/'); pos != std::string::npos)
        temp_name[pos] = '_';

      m_reweightKeys.emplace_back(m_EventInfoKey, "PDFReweightSF_"+temp_name);

      ATH_CHECK(m_reweightKeys.back().initialize());
    }

    if(m_additionalPdfPath != "")
      LHAPDF::pathsAppend(m_additionalPdfPath);

    return StatusCode::SUCCESS;
  }

  StatusCode PDFReweightAlg::execute(const EventContext &ctx) const
  {

    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey, ctx);
    ATH_CHECK(eventInfo.isValid());

    SG::ReadHandle<xAOD::TruthEventContainer> TruthEventContainer(m_TruthEventKey, ctx);
    ATH_CHECK(TruthEventContainer.isValid());

    if(TruthEventContainer->size() != 1){
      ATH_MSG_ERROR("ERROR. Multiple Truth Events Detected");
      return StatusCode::FAILURE;
    }

    const xAOD::TruthEvent *truthEvent = TruthEventContainer->at(0);

    static const SG::AuxElement::ConstAccessor<int> accPDGID1("PDGID1");
    static const SG::AuxElement::ConstAccessor<int> accPDGID2("PDGID2");
    static const SG::AuxElement::ConstAccessor<float> accX1("X1");
    static const SG::AuxElement::ConstAccessor<float> accX2("X2");
    static const SG::AuxElement::ConstAccessor<float> accQ("Q");

    int pdgid1 = accPDGID1(*truthEvent);
    int pdgid2 = accPDGID2(*truthEvent);
    float X1 = accX1(*truthEvent);
    float X2 = accX2(*truthEvent);
    float Q = accQ(*truthEvent);

    for (size_t i = 0; i<m_outPDF.size(); ++i) {

      float reweight = LHAPDF::weightxxQ(pdgid1, pdgid2, X1, X2, Q,
                        static_cast<const LHAPDF::PDF*>(m_p0),
                        static_cast<const LHAPDF::PDF*>(m_p1_vars[i])); // reweight is the scale factor around 1

      SG::WriteDecorHandle<xAOD::EventInfo, float> handle(m_reweightKeys[i], ctx);
      handle(*eventInfo) = reweight;
    }

    return StatusCode::SUCCESS;
    
  }

}
