/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGLONGLIVEDPARTICLESHYPO_TRIGDISTRACKHYPOALG_H
#define TRIGLONGLIVEDPARTICLESHYPO_TRIGDISTRACKHYPOALG_H

#include <string>
#include <memory>

#include "Gaudi/Property.h"
#include "DecisionHandling/HypoBase.h"
#include "TrigDisappearingTrackHypoTool.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "CxxUtils/checker_macros.h"
#include "MVAUtils/BDT.h"

/**
 * @class TrigDisappearingTrackHypoAlg
 * @brief Implements Hypo selection on triggering disappearing tracks
 * @author Kunihiro Nagano <kunihiro.nagano@cern.ch> - KEK
 **/

class TrigDisappearingTrackHypoAlg : public ::HypoBase 
{ 
public: 

   TrigDisappearingTrackHypoAlg( const std::string& name, ISvcLocator* pSvcLocator );
   
   virtual StatusCode  initialize() override;
   virtual StatusCode  execute(const EventContext& context) const override;
   
private: 

   ToolHandleArray< TrigDisappearingTrackHypoTool >  m_hypoTools        {this, "HypoTools", {}, "Tools to perfrom selection"};
   SG::ReadHandleKey< xAOD::TrigCompositeContainer > m_DisTrkCandKey    {this, "DisTrkCand",  "HLT_DisTrkCand",   ""};
   SG::WriteHandleKey<xAOD::TrigCompositeContainer>  m_DisTrkBDTSelKey  {this, "DisTrkBDTSel","HLT_DisTrkBDTSel", ""};

   StatusCode createCandidates(const xAOD::TrigCompositeContainer*, xAOD::TrigCompositeContainer*) const;

   ToolHandle<GenericMonitoringTool> m_monTool{ this, "MonTool", "", "Monitoring tool" };

   // BDT selection methods
   float bdt_eval_pix4l_sct0 (float, float, float, float, float, float, float, float, int,   float, float, int) const;
   float bdt_eval_pix4l_sct1p(float, float, float, float, int,   float, float, float, float, int,   float, int) const;
   float bdt_eval_pix3l_sct0 (float, float, float, float, float, float, float, float, float, float, int,   int) const;
   float bdt_eval_pix3l_sct1p(float, float, float, float, float, float, int,   int,   float, int,   int,   float, float, float, float) const;
   inline float BDTinput(float) const;

   // MVAUtils BDT
   std::unique_ptr<MVAUtils::BDT> m_bdt[4];

}; 

#endif //> !TRIGLONGLIVEDPARTICLESHYPO_TRIGDISTRACKHYPOALG_H
