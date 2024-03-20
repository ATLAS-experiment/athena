/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef MET_ANALYSIS_ALGORITHMS__MET_SIGNIFICANCE_ALG_H
#define MET_ANALYSIS_ALGORITHMS__MET_SIGNIFICANCE_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <SystematicsHandles/SysCopyHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteHandle.h>
#include <METInterface/IMETSignificance.h>
#include <xAODMissingET/MissingETContainer.h>
#include <AsgTools/PropertyWrapper.h>

namespace CP
{
  /// \brief an algorithm for calling \ref IMETSignificanceTool

  class MetSignificanceAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute () override;
    


    /// \brief the smearing tool
  private:
    ToolHandle<IMETSignificance> m_significanceTool {this, "significanceTool", "METMaker", "the significance tool we apply"};

    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// \brief the met collection we run on
  private:
    SysCopyHandle<xAOD::MissingETContainer> m_metHandle {
      this, "met", "MissingET_%SYS%", "the met collection we run on"};

    /// \brief the key for the final met term
  private:
    Gaudi::Property<std::string> m_totalMETName {this, "totalMETName", "Final", "the key for the final met term"};

    /// \brief the key for the jets term
  private:
    Gaudi::Property<std::string> m_jetTermName {this, "jetTermName", "RefJet", "the key for the jets term"};

    /// \brief the key for the soft term
  private:
    Gaudi::Property<std::string> m_softTermName {this, "softTermName", "PVSoftTrk", "the key for the soft term"};

    /// \brief the decoration for the significance
  private:
    Gaudi::Property<std::string> m_significanceDecoration {this, "significanceDecoration", "significance", "the decoration to use for the significance"};

    /// \brief the accessor for \ref m_selectionDecoration
  private:
    std::unique_ptr<const SG::AuxElement::Accessor<float> > m_significanceAccessor;
  };
}

#endif
