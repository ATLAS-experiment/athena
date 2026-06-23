/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETTAGDERIVATIONUTILS_METADATAALG_H
#define JETTAGDERIVATIONUTILS_METADATAALG_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/IIncidentListener.h"
#include "PATInterfaces/SystematicSet.h"
#include "PMGAnalysisInterfaces/IPMGTruthWeightTool.h"
#include "xAODCutFlow/CutBookkeeper.h"
#include "HDF5Utils/IH5GroupSvc.h"

#include "CutBookkeeperUtils/OriginalAodCounts.h"

#include <set>
#include <string>

namespace ftag {

  class MetadataAlg final :
    public AthAlgorithm,
    public IIncidentListener
  {
  public:
    MetadataAlg(const std::string& name, ISvcLocator* pSvcLocator);

    StatusCode initialize() override;
    StatusCode execute(const EventContext& ctx) override;
    StatusCode finalize() override;

    // hook to call this alg on each new input file
    void handle(const Incident&) override;

  private:
    ServiceHandle< StoreGateSvc > m_inputMetaStore;

    ToolHandle<PMGTools::IPMGTruthWeightTool> m_truthWeightTool {
      this, "truthWeightTool", "PMGTools::PMGTruthWeightTool",
      "the truth weight tool"};
    ServiceHandle<IH5GroupSvc> m_output_svc {
      this, "h5Output", "", "output file service"};
    Gaudi::Property<std::string> m_json_output {
      this, "jsonOutput", "", "json output file"
    };
    ServiceHandle<IH5GroupSvc> m_hist_output_svc {
      this, "h5OutputHists", "",
      "output service for histogram output"};
    Gaudi::Property<bool> m_enable_systematics {
      this, "enableSystematics", true,
      "include systematic variations (false = nominal only)"};
    Gaudi::Property<std::set<std::string>> m_allowed_streams {
      this, "allowedStreams",
      {"StreamAOD", "StreamEVGEN", "StreamEVNT",
       "StreamDAOD_PHYS", "StreamDAOD_PHYSLITE"},
      "CutBookkeeper inputStream values accepted as AllExecutedEvents"
    };
    std::unordered_map<size_t, OriginalAodCounts> m_weights;

    bool isGoodBook(const xAOD::CutBookkeeper& cbk) const;

  };

} // end namespace ftag

#endif
