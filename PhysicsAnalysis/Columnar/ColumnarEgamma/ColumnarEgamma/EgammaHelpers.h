/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_EGAMMA_EGAMMA_HELPERS_H
#define COLUMNAR_EGAMMA_EGAMMA_HELPERS_H

#include <ColumnarCore/ColumnAccessor.h>
#include <ColumnarEgamma/EgammaDef.h>
#include <ColumnarTracking/TrackDef.h>
#include <xAODEgamma/EgammaDetails.h>

namespace columnar
{
  namespace EgammaHelpers
  {
    /// @file accessor for variables that have calculations in @ref xAOD::Egamma
    ///
    /// Essentially this just copies out the relevant parts of the xAOD
    /// class and makes them look like stand-alone accessors.  The name
    /// of each class is derived from the member function in the xAOD
    /// class.



    // not sure if this should live here, since it draws in a dependency
    // on ColumnarTracking/xAODTracking, but let's keep it here for now
    template<ContainerIdConcept CI = ContainerId::egamma,typename CM=ColumnarModeDefault>
    class IsConvertedPhotonAccessor final
    {
      ColumnAccessor<CI,float,CM> m_etaAcc;
      ColumnAccessor<CI,std::vector<OptObjectId<ContainerId::vertex,CM>>,CM> m_vertexLinksAcc;
      VertexAccessor<std::vector<OptObjectId<ContainerId::track,CM>>,CM> m_trackParticleLinksAcc;
      TrackAccessor<std::uint8_t,CM> m_numberOfPixelHitsAcc;
      TrackAccessor<std::uint8_t,CM> m_numberOfSCTHitsAcc;
      ColumnAccessor<CI,RetypeColumn<xAOD::Type::ObjectType,std::uint16_t>,CM> m_objectTypeAcc;

    public:
      IsConvertedPhotonAccessor (ColumnarTool<CM>& columnarTool)
        : m_etaAcc (columnarTool, "eta"),
          m_vertexLinksAcc (columnarTool, "vertexLinks"),
          m_trackParticleLinksAcc (columnarTool, "trackParticleLinks", {.isOptional = true}),
          m_numberOfPixelHitsAcc (columnarTool, "numberOfPixelHits"),
          m_numberOfSCTHitsAcc (columnarTool, "numberOfSCTHits")
      {
        if constexpr (!CM::isXAOD)
          resetAccessor (m_objectTypeAcc, columnarTool, "objectType", {.isOptional = true});
      }

      /// return whether the photon is converted, and a bitmask of
      /// missing links
      ///
      /// It is up to the called to decide whether they want to do
      /// anything for the missing links. The reason to report it out is
      /// that the caller will have a message stream, configurable
      /// properties, etc. which an accessor helper does not have.
      std::pair<bool,unsigned> operator () (ObjectId<CI,CM> photon, bool excludeTRT) const
      {
        // While the accessor is generally meant to be used with
        // photons, sometimes electrons are passed as photons for
        // performance studies, etc.  In xAOD mode we can just check
        // `IParticle::type()`, while in columnar mode we need the user
        // to pass that in as an extra column.  Having an extra column
        // for that feels like a bit of an overkill, but it is optional
        // here, and we can revisit it later if it becomes an issue.
        xAOD::Type::ObjectType type = xAOD::Type::Photon;
        if constexpr (CM::isXAOD)
        {
          type = photon.getXAODObjectNoexcept().type();
        } else
        {
          if (m_objectTypeAcc.isAvailable(photon))
            type = m_objectTypeAcc(photon);
        }
        if (type != xAOD::Type::Photon)
          return std::make_pair (false, 0x0);

        const auto vertices = m_vertexLinksAcc(photon);
        if (vertices.size() == 0) return std::make_pair (false, 0x0);
        if (!vertices[0].has_value())
          return std::make_pair (false, 0x1);

        unsigned missingLinks = 0x0;

        auto vertex = vertices[0].value();
        bool hasTrk1 = false;
        bool hasTrk2 = false;
        std::uint8_t nSiHits1 = 0;
        std::uint8_t nSiHits2 = 0;
        if (m_trackParticleLinksAcc.isAvailable(vertex)) {
          const auto tracks = m_trackParticleLinksAcc(vertex);
          if (tracks.size() > 0) {
            if (tracks[0].has_value()) {
              hasTrk1 = true;
              nSiHits1 += m_numberOfPixelHitsAcc(tracks[0].value());
              nSiHits1 += m_numberOfSCTHitsAcc(tracks[0].value());
            } else missingLinks |= 0x2;
          }
          if (tracks.size() > 1) {
            if (tracks[1].has_value()) {
              hasTrk2 = true;
              nSiHits2 += m_numberOfPixelHitsAcc(tracks[1].value());
              nSiHits2 += m_numberOfSCTHitsAcc(tracks[1].value());
            } else missingLinks |= 0x4;
          }
        }

        auto conversionType = xAOD::EgammaDetails::conversionType(hasTrk1, hasTrk2, nSiHits1, nSiHits2);
        return std::make_pair (xAOD::EgammaDetails::isConvertedPhoton (excludeTRT, photon(m_etaAcc), vertices.size(), conversionType), missingLinks);
      }
    };
  }
}

#endif
