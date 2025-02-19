/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
//   MuonContainerMergingAlg   
//   author Maria Mironova, Simone Pagan Griso, Kehang Bai
///////////////////////////////////////////////////////////////////

#ifndef ASG_ANALYSIS_ALGORITHMS__MUON_CONTAINER_MERGING_ALGORITHM__H
#define ASG_ANALYSIS_ALGORITHMS__MUON_CONTAINER_MERGING_ALGORITHM__H

#include <AnaAlgorithm/AnaReentrantAlgorithm.h>
#include <xAODMuon/MuonContainer.h>

#include <AsgTools/PropertyWrapper.h>
#include <AthContainers/ConstDataVector.h>
#include <AsgDataHandles/WriteHandleKey.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include "AsgDataHandles/ReadHandleKeyArray.h"

namespace CP
{
  /// \brief this algorithm allows to merge multiple xAOD::MuonCollection objects into a single (view) collection.

  class MuonContainerMergingAlg final : public EL::AnaReentrantAlgorithm
  {
    /// \brief the standard constructor
  public:
    MuonContainerMergingAlg (const std::string& name,
                                 ISvcLocator* pSvcLocator);
    virtual StatusCode initialize () override;
    virtual StatusCode execute (const EventContext& ctx) const  override;

  private:

      ///////////////////////////////////////////////////////////////////
      /** @brief Private data:                                       */
      ///////////////////////////////////////////////////////////////////

      /// handles for interacting with the event storage

      /** Input objects of type xAOD::MuonContainer to be merged */
      SG::ReadHandleKeyArray<xAOD::MuonContainer> m_inputMuonContainers{
          this, "InputMuonContainers",{}, "List of xAOD::MuonContainer objects to merge into a view container."
      };
       
      /** Output xAOD::MuonContainer object */
      SG::WriteHandleKey<xAOD::MuonContainer> m_outMuonLocationCopy{
          this, "OutputMuonLocationCopy", "MergedMuons", "Name of the muon container to write"
      };
      /** Output xAOD::MuonContainer for the case of a view container */
      SG::WriteHandleKey<ConstDataVector <xAOD::MuonContainer>> m_outMuonLocationView{
          this, "OutputMuonLocation", "MergedMuons", "Name of the view only muon container to write"
      };

      /// flag to create a view collection rather than building deep-copies (true by default)
      Gaudi::Property<bool>  m_createViewCollection{this, "CreateViewCollection", true};

  };
}

#endif
