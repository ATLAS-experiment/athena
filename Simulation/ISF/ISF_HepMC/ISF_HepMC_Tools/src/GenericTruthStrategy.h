/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_TOOLS_GENERICTRUTHSTRATEGY_H
#define ISF_TOOLS_GENERICTRUTHSTRATEGY_H 1

// stl includes
#include <set>
#include <vector>

// Athena includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "AtlasDetDescr/AtlasRegion.h"

// ISF includes
#include "ISF_HepMC_Interfaces/ITruthStrategy.h"

namespace ISF {
  typedef std::vector<int>              VertexTypesVector;
  typedef std::set<int>                 VertexTypesSet;
  typedef std::vector<int>              PDGCodesVector;
  typedef std::set<int>                 PDGCodesSet;

  /** @class GenericTruthStrategy

      A multi-purpose implementation of an ISF TruthStrategy.

      @author Elmar.Ritsch -at- cern.ch
  */
  class GenericTruthStrategy final : public extends<AthAlgTool, ITruthStrategy> {

  public:
    /** Constructor with parameters */
    GenericTruthStrategy( const std::string& t, const std::string& n, const IInterface* p );

    /** Destructor */
    ~GenericTruthStrategy() = default;

    // Athena algtool's Hooks
    virtual StatusCode  initialize() override final;

    /** true if the ITruthStrategy implementation applies to the given ITruthIncident */
    virtual bool pass( ITruthIncident& incident) const override final;

    virtual bool appliesToRegion(unsigned short geoID) const override final;
  private:
    // provide either a pT or Ekin cut for the parent and child particles respectively.
    // if none are given for either type, it will not use pT or Ekin cuts
    // (the Pt2 variables get squared in the initialize() method)

    /** parent kinetic energy / transverse momentum cuts
        (pT is stored as pT^2 which allows for faster comparisons) */
    bool m_useParentPt{true};         //!< use pT or Ekin cuts?
    double m_parentPt2{-1.};
    Gaudi::Property<double> m_parentPt{this, "ParentMinPt", -1.};           //!< parent particle
    Gaudi::Property<double> m_parentEkin{this, "ParentMinEkin", -1.};          //!< parent particle

    /** child particle kinetic energy / transverse momentum cuts
        (pT is stored as pT^2 which allows for faster comparisons) */
    bool m_useChildPt{true};          //!< use pT or Ekin cuts?
    double m_childPt2{-1.};
    Gaudi::Property<double> m_childPt{this, "ChildMinPt", -1.};            //!< pT momentum cut
    Gaudi::Property<double> m_childEkin{this, "ChildMinEkin", -1.};           //!< Ekin cut
    Gaudi::Property<bool> m_allowChildrenOrParentPass{this, "AllowChildrenOrParentPassKineticCuts", false}; //!< pass cuts if parent did not
    // if set to true, kinetic cuts are passed even if only child particles pass them
    // (used for special cases such as de-excitation)

    /** vertex type (physics code) checks */
    Gaudi::Property<VertexTypesVector> m_vertexTypesVector{this, "VertexTypes", 0};  //!< Python property
    VertexTypesSet m_vertexTypes{};        //!< optimized for search
    bool m_doVertexRangeCheck{false};
    Gaudi::Property<int> m_vertexTypeRangeLow{this, "VertexTypeRangeLow", 0};
    Gaudi::Property<int> m_vertexTypeRangeHigh{this, "VertexTypeRangeHigh", 0};
    unsigned m_vertexTypeRangeLength{0};

    /** PDG code checks */
    Gaudi::Property<PDGCodesVector> m_parentPdgCodesVector{this, "ParentPDGCodes", 0};  //!< Python property
    PDGCodesSet m_parentPdgCodes{};        //!< optimized for search

    IntegerArrayProperty            m_regionListProperty{this, "Regions", {}};
  };

}


#endif //> !ISF_TOOLS_GENERICTRUTHSTRATEGY_H
