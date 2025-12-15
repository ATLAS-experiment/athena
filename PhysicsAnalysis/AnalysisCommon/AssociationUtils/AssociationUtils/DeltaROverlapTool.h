/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASSOCIATIONUTILS_OVERLAPTOOLDR_H
#define ASSOCIATIONUTILS_OVERLAPTOOLDR_H

// Framework includes
#include "AsgTools/PropertyWrapper.h"

// Local includes
#include "AssociationUtils/IOverlapTool.h"
#include "AssociationUtils/BaseOverlapTool.h"
#include "AssociationUtils/DeltaRMatcher.h"

namespace ORUtils
{

  /// @class DeltaROverlapTool
  /// @brief A simple overlap finder that uses a dR match.
  ///
  /// This class will remove _all_ objects that fit the criteria.
  ///
  /// @author Steve Farrell <Steven.Farrell@cern.ch>
  ///
  class DeltaROverlapTool : public virtual IOverlapTool,
                            public BaseOverlapTool
  {

      /// Create proper constructor for Athena
      ASG_TOOL_CLASS(DeltaROverlapTool, IOverlapTool)

    public:

      /// Standalone constructor
      DeltaROverlapTool(const std::string& name);

      /// @brief Identify overlaps with simple dR check.
      /// Flags all objects in cont1 which are found to overlap
      /// with any object in cont2 within the configured dR window.
      virtual StatusCode
      findOverlaps(columnar::Particle1Range cont1,
                   columnar::Particle2Range cont2,
                   columnar::EventContextId eventContext) const override;
      using IOverlapTool::findOverlaps;

    protected:

      /// Initialize the tool
      virtual StatusCode initializeDerived() override;

    private:

      /// @name Configurable properties
      /// @{

      /// Delta-R cone for flagging objects as overlap.
      float m_dR;
      /// Calculate delta-R using rapidity
      bool m_useRapidity;

      /// In default configuration, the first container is the one that gets
      /// tested for object rejection. If this switch is true, it will instead
      /// be the second container which gets flagged. This is useful for
      /// modifying the behavior of a master tool in which the argument order
      /// is fixed.
      bool m_swapContainerPrecedence;

      Gaudi::Property<unsigned> m_objectType1{this, "ObjectType1", 0,
        "The xAOD::Type::ObjectType enum value for the first particle type"};
      Gaudi::Property<unsigned> m_objectType2{this, "ObjectType2", 0,
        "The xAOD::Type::ObjectType enum value for the second particle type"};

      /// @}

      /// @name Utilities
      /// @{

      /// Delta-R matcher
      std::unique_ptr<DeltaRMatcher> m_dRMatcher;

      /// @}

      template<columnar::ContainerIdConcept CI1,columnar::RegularContainerIdConcept CI2,typename CM>
      StatusCode internalFindOverlaps(columnar::ObjectRange<CI1,CM> testCont, columnar::ObjectRange<CI2,CM> refCont) const;

  }; // class DeltaROverlapTool

} // namespace ORUtils

#endif
