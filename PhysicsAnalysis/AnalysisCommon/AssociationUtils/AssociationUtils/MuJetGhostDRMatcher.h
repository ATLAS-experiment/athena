/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASSOCIATIONUTILS_MUJETGHOSTMATCHER_H
#define ASSOCIATIONUTILS_MUJETGHOSTMATCHER_H

// System includes
#include <memory>

// Infrastructure includes
#include "AsgMessaging/AsgMessaging.h"

// Columnar includes
#include "ColumnarCore/ColumnAccessor.h"
#include "ColumnarCore/VectorColumn.h"
#include "ColumnarTracking/TrackDef.h"
#include "ColumnarVariant/VariantDef.h"
#include "ColumnarVariant/VariantLinkColumn.h"

// Local includes
#include "AssociationUtils/IObjectAssociator.h"

namespace ORUtils
{

  /// @class MuJetGhostDRMatcher
  /// @brief Matches a muon to a jet via ghost association or a delta-R.
  ///
  /// Ghost association is done by looking for the muon's ID track in the
  /// jet's list of ghost tracks. The Delta-R matching is implemented via the
  /// DeltaRMatcher. The final result is a match if either the ghost
  /// association or the delta-R succeeds.
  ///
  /// @author Steve Farrell <Steven.Farrell@cern.ch>
  ///
  class MuJetGhostDRMatcher final : public IParticleAssociator,
                              public asg::AsgMessaging
  {

    public:

      /// Constructor takes same arguments as the DeltaRMatcher.
      MuJetGhostDRMatcher(double dR, bool useRapidity=true);

      /// Set the object types to be used in the association.
      virtual StatusCode setObjectTypes (xAODType::ObjectType type1,
                                   xAODType::ObjectType type2) override;

      /// Check for a match via ghost association or delta-R
      virtual bool objectsMatch
      (columnar::Particle1Id mu, columnar::Particle2Id jet, bool swapArgs = false) const override;

    private:

      /// The delta-R matcher
      std::unique_ptr<IParticleAssociator> m_drMatcher;

      /// IDTrack type
      using MyTrackDef = columnar::VariantContainerId<columnar::ContainerId::track0,columnar::ContainerId::track0, columnar::ContainerId::track1>;

      columnar::Particle1Accessor<columnar::ObjectLink<MyTrackDef>> m_muonTrkAcc {*this, "inDetTrackParticleLink"};

      // Ghost track list accessor
      columnar::Particle2Accessor<std::vector<columnar::LinkCastColumn<MyTrackDef,xAOD::IParticleContainer>>> m_ghostAcc {*this, "GhostTrack"};

  }; // class MuJetGhostDRMatcher

} // namespace ORUtils

#endif
