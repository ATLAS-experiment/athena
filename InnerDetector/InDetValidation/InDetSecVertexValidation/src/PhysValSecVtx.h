// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
#ifndef INDETSECVERTEXVALIDATION_PHYSVALSECVTX_H
#define INDETSECVERTEXVALIDATION_PHYSVALSECVTX_H

// STL includes
#include <memory>
#include <string>
#include <vector>

#include "AthenaMonitoring/ManagedMonitorToolBase.h"
#include "xAODEventInfo/EventInfo.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODTracking/VertexContainer.h"
#include "src/SecVtxValidationPlots.h"

class PhysValSecVtx
  : public ManagedMonitorToolBase
{ 

  public: 
    PhysValSecVtx(const std::string& type,
	              const std::string& name, 
	              const IInterface* parent);
    virtual ~PhysValSecVtx() = default;

    virtual StatusCode initialize() override;
    virtual StatusCode bookHistograms() override;
    virtual StatusCode fillHistograms(const EventContext& ctx) override;
    virtual StatusCode procHistograms() override;

  private: 
    SG::ReadHandleKey< xAOD::EventInfo > m_eventInfoKey {this, "EventInfo", "EventInfo", 
	"Event info key"};
    
    SG::ReadHandleKey< xAOD::VertexContainer > m_vertexContainerKey {this, "VertexContainerKey", 
    "VrtSecInclusive_SecondaryVertices", "Key of input vertices"};

    //Histograms
    std::unique_ptr<SecVtxValidationPlots> m_secVtxValidationPlots;

    template<typename external_collection_t>
      StatusCode bookCollection(external_collection_t*);
 
}; 

template<typename external_collection_t>
  StatusCode PhysValSecVtx::bookCollection(external_collection_t* plot_collection)
  {
    std::vector<HistData> hists = plot_collection->retrieveBookedHistograms();
    for (auto& [histo, directory] : hists) {
      ATH_MSG_DEBUG ("Initializing " << histo << " " << histo->GetName() << " " << directory << "...");
      ATH_CHECK(regHist(histo, directory, all));
    }
    plot_collection->initialize();
    return StatusCode::SUCCESS;
  }

#endif //> !INDETSECVERTEXVALIDATION_PHYSVALSECVTX_H
