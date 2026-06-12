/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):



// local include
#include "CalibratedEgammaProvider.h"

#include "xAODBase/IParticleHelpers.h"

#include "Gaudi/Interfaces/IOptionsSvc.h"

namespace CP {

CalibratedEgammaProvider::CalibratedEgammaProvider( const std::string& name, ISvcLocator* svcLoc )
  : AthAlgorithm( name, svcLoc ) { }

StatusCode CalibratedEgammaProvider::initialize() {
  ATH_MSG_INFO( "Initialising..." );
  ATH_CHECK(m_evtInfoKey.initialize());
  ATH_CHECK(m_inputKey.initialize());
  ATH_CHECK(m_outputKey.initialize());
  if(m_tool.empty()) { //set up a default tool with the es2012c calibration
      m_tool.setTypeAndName("CP::EgammaCalibrationAndSmearingTool/AutoConfiguredEgammaCalibTool");
      ServiceHandle<Gaudi::Interfaces::IOptionsSvc> josvc("JobOptionsSvc",name());
      josvc->set("ToolSvc.AutoConfiguredEgammaCalibTool.esModel", "es2012c");
  }
   ATH_CHECK(m_tool.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode CalibratedEgammaProvider::execute(const EventContext& ctx) {

   SG::ReadHandle<xAOD::EventInfo> event_info{m_evtInfoKey,ctx};
   if (!event_info.isValid()) {
      ATH_MSG_FATAL("Failed to retrieve the event info "<<m_evtInfoKey.fullKey());
   }

    SG::ReadHandle<xAOD::EgammaContainer> readHandle{m_inputKey, ctx};
    if (!readHandle.isValid()) {
          ATH_MSG_FATAL("No Egamma container found");
          return StatusCode::FAILURE;
    }
    const xAOD::EgammaContainer* egamma{readHandle.cptr()};
    const xAOD::ElectronContainer* electrons = dynamic_cast<const xAOD::ElectronContainer*>(egamma);
    const xAOD::PhotonContainer* photons = dynamic_cast<const xAOD::PhotonContainer*>(egamma);
    xAOD::EgammaContainer* outcontainer{nullptr};
    if (electrons) {
      xAOD::ShallowCopyResult_t<xAOD::ElectronContainer> output = xAOD::shallowCopy(*electrons, ctx);
      if (!output.first || !output.second) {
            ATH_MSG_FATAL("Creation of shallow copy failed");
            return StatusCode::FAILURE;
      }
      outcontainer = output.first.get();
      SG::WriteHandle<xAOD::EgammaContainer> writeHandle{m_outputKey, ctx};
      ATH_CHECK(writeHandle.recordNonConst(std::move(output.first), std::move(output.second)));

    } else if (photons) {
         xAOD::ShallowCopyResult_t<xAOD::PhotonContainer> output = xAOD::shallowCopy(*photons, ctx);
         if (!output.first || !output.second) {
               ATH_MSG_FATAL("Creation of shallow copy failed");
               return StatusCode::FAILURE;
         }
         outcontainer = output.first.get();
         SG::WriteHandle<xAOD::EgammaContainer> writeHandle{m_outputKey, ctx};
         ATH_CHECK(writeHandle.recordNonConst(std::move(output.first), std::move(output.second)));
    } else {
         ATH_MSG_FATAL("Unknown Egamma container "<<m_inputKey.fullKey());
         return StatusCode::FAILURE;
    }
    if (!setOriginalObjectLink(*egamma, *outcontainer)) {
          ATH_MSG_ERROR("Failed to add original object links to shallow copy of " << m_inputKey);
          return StatusCode::FAILURE;
    }

      for(xAOD::Egamma* iParticle : *outcontainer) {
            ATH_MSG_VERBOSE(" Old pt=" << iParticle->pt());
            if(m_tool->applyCorrection(*iParticle).code()==CorrectionCode::Error) return StatusCode::FAILURE;
            ATH_MSG_VERBOSE(" New pt=" << iParticle->pt());
      }
      return StatusCode::SUCCESS;
}

}
