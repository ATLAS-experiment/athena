/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//-----------------------------------------------------------------------------
//
// file:   TrackCnv_p4.cxx
//
//-----------------------------------------------------------------------------
#include "TrkTrack/Track.h"
#include "TrkTrack/TrackInfo.h"
#include "TrkEventTPCnv/TrkTrack/TrackCnv_p4.h"
#include "TrkTrack/TrackStateOnSurface.h"
#include "AthContainers/ConstDataVector.h"

namespace {
unsigned int
keepTSOS(const Trk::TrackStateOnSurface* tsos)
{
  if (!tsos) {
    return false;
  }
  std::bitset<Trk::TrackStateOnSurface::NumberOfPersistencyHints>
    persHints = tsos->hints();
  return (!persHints.test(Trk::TrackStateOnSurface::PartialPersistification) ||
          persHints.test(Trk::TrackStateOnSurface::PersistifyTrackParameters) ||
          persHints.test(Trk::TrackStateOnSurface::PersistifyMeasurement));
}
}

//-----------------------------------------------------------------------------
// Persistent to transient
//-----------------------------------------------------------------------------
void TrackCnv_p4::persToTrans( const Trk::Track_p4 *persObj,
             Trk::Track    *transObj,
             MsgStream       &log )
{
  using namespace Trk;
   transObj->info().m_fitter              = static_cast<Trk::TrackInfo::TrackFitter>(persObj->m_fitter);
   transObj->info().m_particleHypo        = static_cast<Trk::ParticleHypothesis>(persObj->m_particleHypo);

   transObj->info().m_properties 	  = std::bitset<Trk::TrackInfo::NumberOfTrackProperties>(persObj->m_properties);

  // set first 32 bits
   transObj->info().m_patternRecognition  = std::bitset<Trk::TrackInfo::NumberOfTrackRecoInfo>(persObj->m_patternRecognition);
   for (unsigned int i = 32;i<Trk::TrackInfo::NumberOfTrackRecoInfo;++i){
     unsigned int mask = (1<<(i-32));
     transObj->info().m_patternRecognition[i] = (persObj->m_extPatternRecognition & mask );
   }

   // Should always be a FQ so let's just go ahead and make it...
  transObj->m_fitQuality   = std::make_unique<FitQuality>(persObj->m_chiSquared, persObj->m_numberDoF);

  bool isMulti = false;
  if (!persObj->m_trackState.empty()) {
    ITPConverter* cnv = m_topCnv->converterForRef(persObj->m_trackState[0]);
    isMulti =
        (dynamic_cast<ITPConverterFor<Trk::MultiComponentStateOnSurface>*>(
             cnv) != nullptr);
  }

  if (isMulti) {
    std::unique_ptr<MultiComponentStateOnSurfaceDV> sink(
        m_multiStateVectorCnv.createTransient(&persObj->m_trackState, log));
    transObj->m_trackStateVector = std::move(sink);
  } else {
    std::unique_ptr<Trk::TrackStates> sink(
        m_trackStateVectorCnv.createTransient(&persObj->m_trackState, log));
    transObj->m_trackStateVector = std::move(sink);
  }
}

//-----------------------------------------------------------------------------
// Transient to persistent
//-----------------------------------------------------------------------------
void
TrackCnv_p4::transToPers(const Trk::Track* transObj, Trk::Track_p4* persObj, MsgStream& log)
{

  persObj->m_fitter = static_cast<unsigned int>(transObj->info().m_fitter);
  persObj->m_particleHypo = static_cast<unsigned int>(transObj->info().m_particleHypo);
  persObj->m_properties = transObj->info().m_properties.to_ulong();

  if (transObj->info().m_patternRecognition.size()<32) {
    persObj->m_patternRecognition  = transObj->info().m_patternRecognition.to_ulong();
  } else {
    // more 32 bits so have to do it the hard way.
    unsigned int i = 0;
    unsigned int size = transObj->info().m_patternRecognition.size();
    for (; i < 32; ++i) {
      persObj->m_patternRecognition |= ((transObj->info().m_patternRecognition[i]) << i);
    }
    for (i = 32; i < size; ++i) {
      persObj->m_extPatternRecognition |= ((transObj->info().m_patternRecognition[i]) << (i - 32));
    }
  }

  assert(transObj->fitQuality());
  if (transObj->m_fitQuality) {
    persObj->m_chiSquared = transObj->m_fitQuality->chiSquared();
    persObj->m_numberDoF = transObj->m_fitQuality->numberDoF();
  } else {
    log << MSG::WARNING << "No FitQuality on track at [" << transObj << "]"
        << " with info=" << transObj->info().dumpInfo() << endmsg;
  }

  if (transObj->m_trackStateVector && ! transObj->m_trackStateVector->empty()) {
    // Hints based slimming check if we need to persistify less TSOS
    unsigned int n_elms = 0;
    for (const Trk::TrackStateOnSurface* tsos : *(transObj->m_trackStateVector)) {
      if (keepTSOS(tsos)) {
        ++n_elms;
      }
    }
    //Check if we have a Track with Multi TSOS
    bool isMulti = (transObj->m_trackStateVector->at(0)->variety() == Trk::TrackStateOnSurface::MultiComponent);
    if (n_elms != transObj->m_trackStateVector->size()) { //We need to persistify less TSOS
      if (!isMulti) {
        // Track std TSOS
        ConstDataVector<Trk::TrackStates> tsosDV(SG::VIEW_ELEMENTS);
        tsosDV.reserve(n_elms);
        for (const Trk::TrackStateOnSurface* tsos : *(transObj->m_trackStateVector)) {
          if (keepTSOS(tsos)) {
            tsosDV.push_back(tsos);
          }
        }
        m_trackStateVectorCnv.transToPers(tsosDV.asDataVector(), &persObj->m_trackState, log);
      } else {
        // Track with Multi TSOS
        ConstDataVector<MultiComponentStateOnSurfaceDV> multiDV(SG::VIEW_ELEMENTS);
        multiDV.reserve(n_elms);
        for (const Trk::TrackStateOnSurface* tsos : *(transObj->m_trackStateVector)) {
          if (keepTSOS(tsos)) {
            multiDV.push_back(static_cast<const Trk::MultiComponentStateOnSurface*>(tsos));
          }
        }
        m_multiStateVectorCnv.transToPers(multiDV.asDataVector(), &persObj->m_trackState, log);
      }
    } else { // We need to persistify all TSOS
      if (!isMulti) {
        //Track with std TSOS
        m_trackStateVectorCnv.transToPers(transObj->m_trackStateVector.get(), &persObj->m_trackState, log);
      } else {
        // Multi TSOS so we cast the container to the "right" type
        const MultiComponentStateOnSurfaceDV* multiDV =
            static_cast<MultiComponentStateOnSurfaceDV*>(
                transObj->m_trackStateVector.get());
        m_multiStateVectorCnv.transToPers(multiDV, &persObj->m_trackState, log);
      }
    }
  } else { // empty
    if (auto p = transObj->m_trackStateVector.get(); p != nullptr)[[likely]]{
      m_trackStateVectorCnv.transToPers(p, &persObj->m_trackState, log);
    }
  }
}
