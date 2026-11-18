/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETALIGNGENTOOLS_REFITSIONLYTOOL_H
#define INDETALIGNGENTOOLS_REFITSIONLYTOOL_H

//////////////////////////////////////////////////////////////////////////
// ================================================
// RefitSiOnly
// ================================================
//
// RefitSiOnlyTool.h
// Header file for RefitSiOnlyTool
//
// Namespace SiRobustAlign
//
// AlgTool to create refitted tracks with only silicon hits present


#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "InDetAlignToolInterfaces/ICollectionProcessor.h"
#include "TrkFitterInterfaces/ITrackFitter.h"
#include "TrkEventUtils/TrkParametersComparisonFunction.h"//typedef, cannot fwd declare
#include "TrkEventPrimitives/ParticleHypothesis.h"

namespace Trk {
  class Track;
}

class AtlasDetectorID;

namespace InDetAlignment {
  class RefitSiOnlyTool : public extends<AthAlgTool, InDetAlignment::ICollectionProcessor> {
  public:
    using base_class::base_class;
    virtual ~RefitSiOnlyTool(); //destructor

    virtual StatusCode initialize(); ///< Athena Tool methods
    virtual StatusCode finalize();   ///< Athena Tool methods

    /** does necessary configuration to the tool */
    //virtual void configure();
    virtual StatusCode configure();
    /** main processing of track collection -
        refits a track using only filtered hits (e.g. only silicon hits) */
    virtual const DataVector<Trk::Track>* processTrackCollection(const DataVector<Trk::Track>* trks);
    const AtlasDetectorID * m_idHelper{};
   
    void dumpTrackCol(DataVector<Trk::Track>* tracks);
   
  private:
    ///refits a track not using non-silicon hits
    Trk::Track* refit(const Trk::Track* tr) const;

    ///determines from flags, if this hit should be included in the fit or not
    bool filterHit(const Identifier& id) const;

       
    PublicToolHandle<Trk::ITrackFitter> m_ITrkFitter{this, "FitterTool", "Trk::KalmanFitter"};

    Trk::TrkParametersComparisonFunction*  m_comPar{nullptr};

    Gaudi::Property<bool>     m_doCosmic{this, "Cosmic", false};                 //!< Flag for cosmics
    Gaudi::Property<bool>     m_OutlierRemoval{this, "OutlierRemoval", true};    //!< flag for outlier removal

    Gaudi::Property<bool>     m_stripSCTHits{this, "StripSCTHits", false};       ///< flag to not consider SCT hits in the refit
    Gaudi::Property<bool>     m_stripPixelHits{this, "StripPixelHits", false};   ///< flag to not consider Pixel hits in the refit
    Gaudi::Property<bool>     m_stripTRTHits{this, "StripTRTHits", true};        ///< flag to not consider TRT hits in the refit

    Gaudi::Property<int>      m_ParticleNumber{this, "ParticleNumber", 2};
    Trk::ParticleHypothesis   m_ParticleHypothesis{Trk::nonInteracting};
    int                       m_trkindex{};



  };
} //namespace

#endif // INDETALIGNGENTOOLS_REFITSIONLYTOOL_H
