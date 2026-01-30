/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_TOOLS_KEEPCHILDRENTRUTHSTRATEGY_H
#define ISF_TOOLS_KEEPCHILDRENTRUTHSTRATEGY_H 1

// stl includes
#include <set>
#include <vector>

// Athena includes
#include "AthenaBaseComps/AthAlgTool.h"

// ISF includes
#include "ISF_HepMC_Interfaces/ITruthStrategy.h"

namespace ISF {
  typedef std::vector<int>              VertexTypesVector;
  typedef std::set<int>                 VertexTypesSet;
  typedef std::vector<int>              PDGCodesVector;
  typedef std::set<int>                 PDGCodesSet;

  /** @class KeepChildrenTruthStrategy

      A modifier for the purposes of truth strategies defining cases
      in which we should keep all the children of an interaction.

      @author Zach.Marshall -at- cern.ch
  */
  class KeepChildrenTruthStrategy final : public extends<AthAlgTool, ITruthStrategy> {

  public:
    /** Constructor with parameters */
    KeepChildrenTruthStrategy( const std::string& t, const std::string& n, const IInterface* p );

    /** Destructor */
    ~KeepChildrenTruthStrategy() = default;

    // Athena algtool's Hooks
    virtual StatusCode  initialize() override final;

    /** true if the ITruthStrategy implementation applies to the given ITruthIncident */
    virtual bool pass( ITruthIncident& incident) const override final;

    /** true if the strategy applies to this region */
    virtual bool appliesToRegion(unsigned short) const override final;

  private:

    /** vertex type (physics code) checks */
    Gaudi::Property<VertexTypesVector> m_vertexTypesVector{this, "VertexTypes", 0};  //!< Python property
    VertexTypesSet m_vertexTypes{};        //!< optimized for search
    bool m_doVertexRangeCheck{false};
    Gaudi::Property<int> m_vertexTypeRangeLow{this, "VertexTypeRangeLow", 0};
    Gaudi::Property<int> m_vertexTypeRangeHigh{this, "VertexTypeRangeHigh", 0};
    unsigned m_vertexTypeRangeLength{0};
    Gaudi::Property<int>  m_passProcessCategory{this, "PassProcessCategory", 9};
    Gaudi::Property<bool> m_bsmParent{this, "BSMParent", false};             //!< Apply to BSM parents
    /** PDG code checks */
    Gaudi::Property<PDGCodesVector> m_parentPdgCodesVector{this, "ParentPDGCodes", 0};  //!< Python property
    PDGCodesSet m_parentPdgCodes{};        //!< optimized for search
  };

}

#endif //> !ISF_TOOLS_KEEPCHILDRENTRUTHSTRATEGY_H
