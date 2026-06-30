/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina

#ifndef EVENT_SELECTOR_OBJECTKINEMATICSELECTORALG_H
#define EVENT_SELECTOR_OBJECTKINEMATICSELECTORALG_H

// Algorithm includes
#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/PropertyWrapper.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysHandleArray.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SelectionHelpers/SysWriteSelectionHandle.h>

// Framework includes
#include <xAODBase/IParticleContainer.h>
#include <xAODBase/IParticle.h>
#include <xAODMissingET/MissingETContainer.h>
#include <xAODEventInfo/EventInfo.h>

#include <EventSelectionAlgorithms/SignEnums.h>

namespace CP {

  /// \brief a generic algorithm to cut on a kinematic quantity computed from
  /// one or more objects, e.g. dR(jet[0], jet[1]) or m(el[0]+mu[0]).
  ///
  /// Operands are described by three parallel, ordered lists over the
  /// *particle* operands (collections / selections / indices) plus an
  /// operand-kind list that interleaves "PARTICLE" and "MET" entries. For a
  /// particle operand the objects passing its selection are pT-ordered and the
  /// requested index is picked; an out-of-range index aborts the job (the user
  /// is responsible for guaranteeing the required multiplicity upstream).

  class ObjectKinematicSelectorAlg final : public EL::AnaAlgorithm {

    public:
      ObjectKinematicSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);
      virtual StatusCode initialize() override;
      virtual StatusCode execute(const EventContext& ctx) override;

    private:

      /// \brief the kinematic variable to compute (dR, dPhi, dEta, m, pt, e, eta, phi)
      Gaudi::Property<std::string> m_variable {this, "variable", "SetMe", "kinematic variable to compute"};

      /// \brief the comparison sign (GT, LT, etc)
      Gaudi::Property<std::string> m_sign {this, "sign", "SetMe", "comparison sign to use"};

      /// \brief the reference value against which to compare (in MeV / unitless)
      Gaudi::Property<float> m_refValue {this, "refValue", 0., "reference value to compare against"};

      /// \brief per-operand kind, in expression order: "PARTICLE" or "MET"
      Gaudi::Property<std::vector<std::string>> m_operandKinds {this, "operandKinds", {}, "per-operand kind (PARTICLE/MET), in order"};

      /// \brief per particle-operand pT-ordered index (parallel to the particle handles)
      Gaudi::Property<std::vector<int>> m_indices {this, "indices", {}, "per particle-operand pT-ordered index"};

      /// \brief the MET term to use for a MET operand
      Gaudi::Property<std::string> m_metTerm {this, "metTerm", "Final", "the MET term to use"};

      /// \brief the operator version of the comparison (>, <, etc)
      SignEnum::ComparisonOperator m_signEnum{};

      /// \brief the systematics list
      CP::SysListHandle m_systematicsList {this};

      /// \brief the particle-operand input containers (one per PARTICLE operand, in order)
      CP::SysHandleArray<CP::SysReadHandle<xAOD::IParticleContainer>> m_particlesHandles {
        this, "collections", {}, "the particle containers, one per PARTICLE operand"
      };

      /// \brief the per particle-operand selection keys (parallel to the particle handles)
      Gaudi::Property<std::vector<std::string>> m_selectionKeys {
        this, "selections", {}, "the selections, one per PARTICLE operand"
      };

      /// \brief the per particle-operand selection handles, built from m_selectionKeys
      std::vector<CP::SysReadSelectionHandle> m_selections;

      /// \brief the MET container (used iff any operand is MET)
      CP::SysReadHandle<xAOD::MissingETContainer> m_metHandle {
        this, "met", "", "the MET container to use for a MET operand"
      };

      /// \brief the event info handle
      CP::SysReadHandle<xAOD::EventInfo> m_eventInfoHandle {
        this, "eventInfo", "EventInfo", "the EventInfo container to read selection decisions from"
      };

      /// \brief the preselection
      CP::SysReadSelectionHandle m_preselection {
        this, "eventPreselection", "SetMe", "name of the preselection to check before applying this one"
      };

      /// \brief the output selection decoration
      CP::SysWriteSelectionHandle m_decoration {
        this, "decorationName", "SetMe", "decoration name for the EXPR selector"
      };

  }; // class
} // namespace CP

#endif // EVENT_SELECTOR_OBJECTKINEMATICSELECTORALG_H
