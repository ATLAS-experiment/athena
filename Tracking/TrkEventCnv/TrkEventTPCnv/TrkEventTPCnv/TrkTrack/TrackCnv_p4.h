/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRACK_CNV_P4_H
#define TRACK_CNV_P4_H


#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"

#include "TrkTrack/Track.h"
#include "TrkEventTPCnv/TrkTrack/Track_p4.h"

#include "TrkEventTPCnv/TrkTrack/TrackStateOnSurfaceCnv_p3.h"

#include "AthContainers/ConstDataVector.h"
class MsgStream;

class TrackCnv_p4: public T_AthenaPoolTPCnvBase<Trk :: Track,
                                                Trk :: Track_p4>
{
public:
  TrackCnv_p4() {}
  ~TrackCnv_p4(){}

  virtual void persToTrans( const Trk::Track_p4 *, Trk::Track *, MsgStream& );
  virtual void transToPers( const Trk::Track *, Trk::Track_p4 *, MsgStream& );

  virtual void  initPrivateConverters( AthenaPoolTopLevelTPCnvBase *topCnv )
  {
   m_trackStateVectorCnv.setTopConverter( topCnv, 0 );
   m_multiStateVectorCnv.setTopConverter( topCnv, 0 );
   m_topCnv  = topCnv;
  }

 protected:
  typedef T_AthenaPoolTPPtrVectorCnv<Trk::TrackStates,
                                     std::vector<TPObjRef>,
                                     TrackStateOnSurfaceCnv_p3>
      TrackStateOSVectorCnv_p3;

  TrackStateOSVectorCnv_p3 m_trackStateVectorCnv;

  typedef T_AthenaPoolTPPtrVectorCnv<MultiComponentStateOnSurfaceDV,
                                     std::vector<TPObjRef>,
                                     MultiComponentStateOnSurfaceCnv_p1>
      MultiStateOSVectorCnv_p1;

  MultiStateOSVectorCnv_p1 m_multiStateVectorCnv;

  AthenaPoolTopLevelTPCnvBase *m_topCnv{};
};

#endif // TRACK_CNV_P3_H
