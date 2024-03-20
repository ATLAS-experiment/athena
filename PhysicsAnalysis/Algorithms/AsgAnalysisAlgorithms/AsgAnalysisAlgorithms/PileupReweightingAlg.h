/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef ASG_ANALYSIS_ALGORITHMS__PILEUP_REWEIGHTING_ALG_H
#define ASG_ANALYSIS_ALGORITHMS__PILEUP_REWEIGHTING_ALG_H

#include <xAODEventInfo/EventInfo.h>
#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgAnalysisInterfaces/IPileupReweightingTool.h>
#include <SelectionHelpers/OutOfValidityHelper.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <AsgTools/PropertyWrapper.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/ReadHandle.h>

namespace CP
{
  /// \brief an algorithm for calling \ref IPileupReweightingTool

  class PileupReweightingAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute () override;
    


    /// \brief the smearing tool
  private:
    ToolHandle<IPileupReweightingTool> m_pileupReweightingTool {this, "pileupReweightingTool", "PileupReweightingTool", "the pileup reweighting tool we apply"};

    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// \brief the decoration for the pileup weight
  private:
    CP::SysWriteDecorHandle<float> m_weightDecorator{
        this, "pileupWeightDecoration", "PileupWeight_%SYS%", "the decoration for the pileup weight"};

    /// \brief the name of the event info object
  private:
    CP::SysReadHandle<xAOD::EventInfo> m_eventInfoHandle{
        this, "eventInfo", "EventInfo", "the input EventInfo object"};

    /// \brief the name of the original event info (this should usually be the same as eventiNfoHandle and EventInfo)
  private:
    SG::ReadHandleKey<xAOD::EventInfo> m_baseEventInfoName {this, "baseEventInfo", "EventInfo",       "The name of the original event info. The non-systematic dependent decorations will be applied to this "
      "object so it should be at least a base of the shallow copies read in by the 'eventInfo' handle. "
      "The default (and strongly recommended behaviour) is to leave all of these pointed at the central 'EventInfo' object!"};

    /// \brief the decoration for the corrected and scaled average interactions per crossing
  private:
    Gaudi::Property<std::string> m_correctedScaledAverageMuDecoration {this, "correctedScaledAverageMuDecoration", "", "the decoration for the corrected and scaled average interactions per crossing"};

    /// \brief the decoration for the corrected actual interactions per crossing
  private:
    Gaudi::Property<std::string> m_correctedActualMuDecoration {this, "correctedActualMuDecoration", "", "the decoration for the corrected actual interactions per crossing"};

    /// \brief the decoration for the corrected and scaled actual interactions per crossing
  private:
    Gaudi::Property<std::string> m_correctedScaledActualMuDecoration {this, "correctedScaledActualMuDecoration", "", "the decoration for the corrected and scaled actual interactions per crossing"};

    /// \brief the accessor for \ref m_correctedScaledAverageMuDecoration
  private:
    std::unique_ptr<const SG::AuxElement::Decorator<float>> m_correctedScaledAverageMuDecorator;

    /// \brief the accessor for \ref m_correctedActualMuDecoration
  private:
    std::unique_ptr<const SG::AuxElement::Decorator<float>> m_correctedActualMuDecorator;

    /// \brief the accessor for \ref m_correctedScaledAverageMuDecoration
  private:
    std::unique_ptr<const SG::AuxElement::Decorator<float>> m_correctedScaledActualMuDecorator;

    /// \brief the helper for OutOfValidity results
  private:
    OutOfValidityHelper m_outOfValidity {this};
  };
}

#endif
