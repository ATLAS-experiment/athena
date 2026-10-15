/* 
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS Collaboration
*/

#ifndef ASSOCIATIONUTILS_OVERLAPTOOLFE_H
#define ASSOCIATIONUTILS_OVERLAPTOOLFE_H

#include "AsgTools/PropertyWrapper.h"

#include "AssociationUtils/IOverlapTool.h"
#include "AssociationUtils/BaseOverlapTool.h"
#include "AssociationUtils/FEMatcher.h"

#include "AsgDataHandles/ReadHandleKey.h"

#include "xAODMissingET/MissingETAssociationMap.h"

namespace ADD_CMODE(ORUtils) {
  /// @class FEOverlapTool
  /// @brief An overlap finder that utilizes flow element energy sharing.
  ///
  /// This class will remove _all_ objects that fit the criteria.
  ///
  /// @author Laura Miller <laura.miller@cern.ch>

  class FEOverlapTool : public virtual IOverlapTool,
                        public BaseOverlapTool
  {
    /// Create proper constructor for Athena
    ASG_TOOL_CLASS(FEOverlapTool, IOverlapTool)

    public:

      /// Standalone constructor
      FEOverlapTool(const std::string& name);

      /// @brief Identify overlaps using flow element energy sharing.
      /// Flags objects in cont1 which are found to overlap with objects in cont2.
      virtual StatusCode findOverlaps(columnar::Particle1Range cont1, 
                                      columnar::Particle2Range cont2, 
                                      columnar::EventContextId eventContext) const override;
      using IOverlapTool::findOverlaps;

    protected:
      
      // Initialize the tool
      virtual StatusCode initializeDerived() override;

    private:

      SG::ReadHandleKey<xAOD::MissingETAssociationMap> m_feMapKey{
        this, "FEAssociationMap", "FEAssociationMap", "Flow element association map"
      };

      template<columnar::ContainerIdConcept CI1,columnar::RegularContainerIdConcept CI2>
      StatusCode internalFindOverlaps(columnar::ObjectRange<CI1> testCont, columnar::ObjectRange<CI2> refCont) const;


      
      std::unique_ptr<FEMatcher> m_FEMatcher;

  }; // class FEOverlapTool

} // namespace ADD_CMODE(ORUtils)


#endif

