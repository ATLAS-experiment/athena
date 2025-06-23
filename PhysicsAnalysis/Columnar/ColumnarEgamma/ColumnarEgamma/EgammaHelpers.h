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



    template<ContainerId CI = ContainerId::egamma,typename CM=ColumnarModeDefault>
    class EnergyAccessor final
    {
      ColumnAccessor<CI,float,CM> m_ptAcc;
      ColumnAccessor<CI,float,CM> m_etaAcc;

    public:
      
      EnergyAccessor (ColumnarTool<CM>& columnarTool) : m_ptAcc (columnarTool, "pt"), m_etaAcc (columnarTool, "eta") {}

      float operator () (ObjectId<CI,CM> object) const
      {
        return m_ptAcc(object) * std::cosh(m_etaAcc(object));
      }
    };



    // not sure if this should live here, since it draws in a dependency
    // on ColumnarTracking/xAODTracking, but let's keep it here for now
    template<ContainerId CI = ContainerId::egamma,typename CM=ColumnarModeDefault>
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

      bool operator () (ObjectId<CI,CM> photon, bool excludeTRT) const
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
          return false;

        const auto vertices = m_vertexLinksAcc(photon);
        if (vertices.size() == 0) return false;
        auto vertex = vertices[0].value();
        bool hasTrk1 = false;
        bool hasTrk2 = false;
        std::uint8_t nSiHits1 = 0;
        std::uint8_t nSiHits2 = 0;
        if (m_trackParticleLinksAcc.isAvailable(vertex)) {
          const auto tracks = m_trackParticleLinksAcc(vertex);
          if (tracks.size() > 0 && tracks[0].has_value()) {
            hasTrk1 = true;
            nSiHits1 += m_numberOfPixelHitsAcc(tracks[0].value());
            nSiHits1 += m_numberOfSCTHitsAcc(tracks[0].value());
          }
          if (tracks.size() > 1 && tracks[1].has_value()) {
            hasTrk2 = true;
            nSiHits2 += m_numberOfPixelHitsAcc(tracks[1].value());
            nSiHits2 += m_numberOfSCTHitsAcc(tracks[1].value());
          }
        }

        auto conversionType = xAOD::EgammaDetails::conversionType(hasTrk1, hasTrk2, nSiHits1, nSiHits2);
        return xAOD::EgammaDetails::isConvertedPhoton (excludeTRT, photon(m_etaAcc), vertices.size(), conversionType);
      }
    };
  }
}

#endif
