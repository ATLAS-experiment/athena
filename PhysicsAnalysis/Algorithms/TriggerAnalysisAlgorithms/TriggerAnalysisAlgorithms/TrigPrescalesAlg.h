/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak


#ifndef TRIGGER_ANALYSIS_ALGORITHMS__TRIG_PRESCALES_ALG_H
#define TRIGGER_ANALYSIS_ALGORITHMS__TRIG_PRESCALES_ALG_H

#include <functional>

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgAnalysisInterfaces/IPileupReweightingTool.h>
#include <AsgTools/PropertyWrapper.h>

namespace CP
{
  /// \brief the decoration value to use if there is no valid
  /// trigger prescale information
  constexpr float invalidTriggerPrescale () {return -1;}



  /// \brief an algorithm for retrieving trigger prescales

  class TrigPrescalesAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute () override;


    /// \brief the pile-up reweighting tool
  private:
    ToolHandle<IPileupReweightingTool> m_pileupReweightingTool {this, "pileupReweightingTool", "PileupReweightingTool", "the pileup reweighting tool to be used"};

    /// \brief list of prescaled triggers or trigger chains
  private:
    Gaudi::Property<std::vector<std::string>> m_trigList {this, "triggers", {}, "trigger list"};
  
    /// \brief list of all triggers or trigger chains
  private:
    Gaudi::Property<std::vector<std::string>> m_trigListAll {this, "triggersAll", {}, "all trigger list"};

    /// \brief list of all triggers or trigger chains
  private:
    Gaudi::Property<std::string> m_trigFormula {this, "triggersFormula", "", "produce prescale based on formula instead of per trigger, e.g. (trigA||trigB)"};

    /// \brief list of helper functions to compute the prescales
  private:
    std::vector<std::function<float(const xAOD::EventInfo *, const std::string &)>> m_prescaleFunctions;

    /// \brief the decoration for trigger prescales
  private:
    Gaudi::Property<std::string> m_prescaleDecoration {this, "prescaleDecoration", "", "decoration to store prescales"};

    /// \brief whether to prescale MC instead of unprescale dat 
  private:
    Gaudi::Property<bool> m_prescaleMC {this, "prescaleMC", false, "whether to do prescaling of MC instead of unprescaling of data"};

    /// \brief the accessors for \ref m_prescaleDecoration and \ref m_trigList combination
  private:
    std::vector<SG::AuxElement::Decorator<float>> m_prescaleAccessors;
  };
}

#endif
