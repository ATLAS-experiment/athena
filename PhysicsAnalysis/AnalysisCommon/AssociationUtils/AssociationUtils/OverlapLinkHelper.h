/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASSOCIATIONUTILS_OVERLAPLINKHELPER_H
#define ASSOCIATIONUTILS_OVERLAPLINKHELPER_H

// Framework includes
#include "AsgMessaging/StatusCode.h"

// EDM includes
#include "xAODBase/IParticleContainer.h"

// Columnar includes
#include "ColumnarCore/ColumnAccessor.h"
#include "ColumnarCore/ColumnarTool.h"
#include "ColumnarCore/LinkColumn.h"
#include "ColumnarVariant/VariantLinkColumn.h"

// Local includes
#include "AssociationUtils/OverlapRemovalDefs.h"

namespace ORUtils
{

  /// @class OverlapLinkHelper
  /// @brief Helper class for setting links between overlapping objects.
  ///
  /// This utility class is used by the OverlapTools, though it could probably
  /// be used by a user as well. It can do two things at the moment:
  /// - Link one object to another
  /// - Retrieve a linked object
  ///
  /// For this simple implementation, use a single object link.
  /// I might later allow for an array of links to include all
  /// possible object overlaps.
  ///
  /// @author Steve Farrell <Steven.Farrell@cern.ch>
  ///
  template<columnar::ContainerIdConcept CI>
  class OverlapLinkHelper : public columnar::ColumnarTool<>
  {

    public:

      /// Constructor
      OverlapLinkHelper(const std::string& linkLabel);

      /// Decorate p1 with an overlap object link to p2
      template<columnar::ContainerIdConcept LT>
      StatusCode addObjectLink(columnar::ObjectId<CI> p1,
                               columnar::ObjectId<LT> p2) const;

      /// @brief Retrieve an overlap-linked particle.
      /// Returns null if no ElementLink decoration exists.
      template<columnar::ContainerIdConcept LT>
      columnar::OptObjectId<LT> getObjectLink(columnar::ObjectId<CI> p, columnar::ObjectRange<LT> container) const;

    private:

      // we need to use a columnar variant link here, because other
      // tools may send a link to a container we are not listing here.
      // however, at the same time having a variant link here also means
      // I can point to either of my two containers without extra
      // effort.
      using LTDef = columnar::VariantContainerId<columnar::ContainerId::particle1,columnar::ContainerId::particle1,columnar::ContainerId::particle2>;

      /// Object link decorator
      columnar::ColumnDecorator<CI,columnar::ObjectLink<LTDef>> m_linkDecorator;
      /// Corresponding object link accessor (for reading only)
      columnar::ColumnAccessor<CI,columnar::ObjectLink<LTDef>> m_linkAccessor;

  }; // class OverlapLinkHelper

  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  template<columnar::ContainerIdConcept CI>
  OverlapLinkHelper<CI>::OverlapLinkHelper(const std::string& linkLabel)
    : m_linkDecorator(*this,linkLabel),
      m_linkAccessor(*this,linkLabel)
  {}

  //---------------------------------------------------------------------------
  // Link p1 to p2
  //---------------------------------------------------------------------------
  template<columnar::ContainerIdConcept CI> template<columnar::ContainerIdConcept LT>
  StatusCode OverlapLinkHelper<CI>::addObjectLink
  (columnar::ObjectId<CI> p1, columnar::ObjectId<LT> p2) const
  {
    m_linkDecorator.set (p1, p2);
    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Retrieve an overlap-linked particle or null.
  //---------------------------------------------------------------------------
  template<columnar::ContainerIdConcept CI> template<columnar::ContainerIdConcept LT>
  columnar::OptObjectId<LT> OverlapLinkHelper<CI>::getObjectLink
  (columnar::ObjectId<CI> p, columnar::ObjectRange<LT> container) const
  {
    // Check if the decoration is present and valid
    if(!m_linkAccessor.isAvailable(p))
      return {};
    auto link = m_linkAccessor(p);
    if (!link)
      return {};
    if constexpr (columnar::ColumnarModeDefault::isXAOD)
    {
      if (link.getXAODObject()->container() != &container.getXAODObject())
        return {};
    }
    return link.template tryGetVariant<LT>();
  }

} // namespace ORUtils

#endif
