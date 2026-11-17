/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonPrepRawData/MMPrepData.h"
#include "MuonEventTPCnv/MuonPrepRawData/MMPrepDataCnv_p2.h"

namespace {
    template <class T>  T copy(const T& obj) { return T{obj}; }
}

/// function used to create an MMPrepDataObject when converting Pers to trans
Muon::MMPrepData
MMPrepDataCnv_p2::
createMMPrepData ( const Muon::MMPrepData_p2 *persObj,
		               Identifier clusId,
                   const MuonGM::MMReadoutElement* detEl,
                   MsgStream & /**log*/ )  {
    // Fill localposition
    Amg::Vector2D localPos{persObj->m_locX, 0.};
    // copy the list of identifiers of the cluster
    std::vector<Identifier> rdoList;
    unsigned int clusIdCompact = clusId.get_identifier32().get_compact();
    std::vector<signed char> rdoListPers = persObj->m_rdoList;
    for ( auto& diff : rdoListPers ) {
      Identifier rdoId (clusIdCompact + diff);
      rdoList.push_back(std::move(rdoId));
    }
    auto cmat = Amg::MatrixX(1,1);
    cmat(0,0) = static_cast<double>(persObj->m_errorMat);

  Muon::MMPrepData data (clusId, 
                         0, //collectionHash
                         std::move(localPos),
                         std::move(rdoList),
                         std::move(cmat),
                         detEl,
	                   copy(persObj->m_time),
	                   copy(persObj->m_charge),
	                   copy(persObj->m_driftDist),
	                   copy(persObj->m_stripNumbers),
	                   copy(persObj->m_stripTimes),
	                   copy(persObj->m_stripCharges));

  // se the microTPC parameters
  data.setMicroTPC(copy(persObj->m_angle), 
                   copy(persObj->m_chisqProb));
  // set the drift distances
  data.setDriftDist(copy(persObj->m_stripDriftDist),
                    copy(persObj->m_stripDriftErrors_0_0), 
                    copy(persObj->m_stripDriftErrors_1_1));
  data.setAuthor(static_cast<Muon::MMPrepData::Author>(persObj->m_author));
  data.setQuality(static_cast<Muon::MMPrepData::Quality>(persObj->m_quality));

  return data;
}

void MMPrepDataCnv_p2::persToTrans( const Muon::MMPrepData_p2 *persObj, Muon::MMPrepData *transObj, MsgStream & log ) {
  *transObj = createMMPrepData (persObj,
                                transObj->identify(),
                                transObj->detectorElement(),
                                log);
}

void MMPrepDataCnv_p2::transToPers( const Muon::MMPrepData *transObj, Muon::MMPrepData_p2 *persObj, MsgStream &) {
    persObj->m_locX     = transObj->localPosition()[Trk::locX];
    persObj->m_errorMat = (transObj->localCovariance())(0,0);
    
    persObj->m_time     = static_cast<short int>(transObj->time());
    persObj->m_charge   =  static_cast<int>(transObj->charge());
    persObj->m_driftDist   = static_cast<float>(transObj->driftDist());

    persObj->m_angle     = static_cast<float>(transObj->angle());
    persObj->m_chisqProb = static_cast<float>(transObj->chisqProb());

    persObj->m_stripNumbers = transObj->stripNumbers();
    persObj->m_stripTimes   = transObj->stripTimes();
    persObj->m_stripCharges = transObj->stripCharges();

    persObj->m_stripDriftDist   = transObj->stripDriftDist();
    persObj->m_stripDriftErrors_0_0 = transObj->stripDriftErrors_0_0();
    persObj->m_stripDriftErrors_1_1 = transObj->stripDriftErrors_1_1();

    persObj->m_author = static_cast<short>(transObj->author());
    persObj->m_quality = static_cast<uint8_t>(transObj->quality());

    /// store the rdoList in a vector with the difference with respect to the 32-bit cluster identifier
    Identifier32::value_type clusIdCompact = transObj->identify().get_identifier32().get_compact(); // unsigned int
    std::vector<signed char> rdoListPers;
    const std::vector<Identifier>& rdoListTrans = transObj->rdoList();
    for ( const auto& rdo_id : rdoListTrans ) {
      // get the difference with respect to the 32-bit cluster identifier
      // (this only works if the absolute value of the difference is smaller than 128)
      Identifier32::value_type rdoIdCompact = rdo_id.get_identifier32().get_compact(); // unsigned int
      int diff = static_cast<int>(rdoIdCompact-clusIdCompact);
      rdoListPers.push_back(static_cast<signed char>(diff));
    } 
   
    persObj->m_rdoList = std::move(rdoListPers);
}


