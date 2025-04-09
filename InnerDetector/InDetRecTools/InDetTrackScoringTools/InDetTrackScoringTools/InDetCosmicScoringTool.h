/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#ifndef INDETCOSMICSCORINGTOOL_H
#define INDETCOSMICSCORINGTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkEventPrimitives/TrackScore.h"
#include "TrkToolInterfaces/ITrackScoringTool.h"
#include <vector>
#include <string>

namespace Trk {
  class IExtrapolator;
  class Track;
  class TrackSummary;
}


namespace InDet {


/**Concrete implementation of the ITrackScoringTool pABC*/
class InDetCosmicScoringTool : virtual public Trk::ITrackScoringTool, public AthAlgTool
{

public:
  InDetCosmicScoringTool(const std::string&,const std::string&,const IInterface*);
  virtual ~InDetCosmicScoringTool () = default;

  /** check track selections independent from TrackSummary */
  virtual bool passBasicSelections( const Trk::Track& ) const override;

  /** create a score based on how good the passed track is*/
  virtual
  Trk::TrackScore score( const Trk::Track& track, bool checkBasicSel ) const override;
  
  /** create a score based on how good the passed TrackSummary is*/
  virtual
  Trk::TrackScore simpleScore( const Trk::Track& track, const Trk::TrackSummary& trackSum ) const override;
  
 private:

  IntegerProperty m_nWeightedClustersMin{this, "nWeightedClustersMin", 0};
  IntegerProperty m_minTRTHits{this, "minTRTHits", 0};

  DoubleProperty m_maxZImp
    {this, "maxZImp", 10000., "maximal z impact parameter cut"};

};


}
#endif 
