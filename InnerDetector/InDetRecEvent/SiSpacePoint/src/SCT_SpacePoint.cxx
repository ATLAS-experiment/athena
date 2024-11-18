/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "SiSpacePoint/SCT_SpacePoint.h"

#include <memory>
#include <ostream>
#include <sstream>

#include "EventPrimitives/EventPrimitivesToStringConverter.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "TrkEventPrimitives/LocalParameters.h"
#include "TrkPrepRawData/PrepRawData.h"
#include "TrkSurfaces/Surface.h"

namespace InDet {
//-------------------------------------------------------------

SCT_SpacePoint::SCT_SpacePoint(
    const std::pair<IdentifierHash, IdentifierHash>& elementIdList,
    const Amg::Vector3D& position,
    const std::pair<const Trk::PrepRawData*, const Trk::PrepRawData*>& clusList)
    : Trk::SpacePoint() {
  setup(elementIdList, position, clusList);
  setupLocalCovarianceSCT();
  setupGlobalFromLocalCovariance();
}

//-------------------------------------------------------------

void SCT_SpacePoint::setup(
    const std::pair<IdentifierHash, IdentifierHash>& elementIdList,
    const Amg::Vector3D& position,
    const std::pair<const Trk::PrepRawData*, const Trk::PrepRawData*>&
        clusList) {
  m_clusList = clusList;
  m_position = position;
  m_elemIdList.first = elementIdList.first;
  m_elemIdList.second = elementIdList.second;
  assert((clusList.first != 0) && (clusList.second != 0));
  assert(clusList.first->detectorElement());
  std::optional<Amg::Vector2D> locpos{
      clusList.first->detectorElement()->surface().globalToLocal(position)};
  assert(locpos);
  Trk::MeasurementBase::m_localParams = Trk::LocalParameters(*locpos);
}

//-------------------------------------------------------------

void SCT_SpacePoint::setupLocalCovarianceSCT() {

  /* For performance reason only, the error is assumed constant.
      numbers are taken from
      Trigger/TrigTools/TrigOfflineSpacePointTool/OfflineSpacePointProviderTool
   */
  constexpr double deltaY = 0.0004;  // roughly pitch of SCT (80 mu) / sqrt(12)
  constexpr double offdiag = 25. * deltaY;
  constexpr double elem11 = 1600. * deltaY;
  Amg::MatrixX cov(2, 2);
  cov << deltaY, offdiag,
      offdiag, elem11;

  Trk::MeasurementBase::m_localCovariance = std::move(cov);
}

//-------------------------------------------------------------

MsgStream& SCT_SpacePoint::dump(MsgStream& out) const {
  std::ostringstream os;
  dump(os);
  out << os.str();
  return out;
}

//-------------------------------------------------------------

std::ostream& SCT_SpacePoint::dump(std::ostream& out) const {
  const std::string lf{"\n"};  // linefeed
  out << "SCT_SpacePoint  contains: " << lf;
  out << "Identifier Hashes ( " << int(this->elementIdList().first) << " , ";
  out << int(this->elementIdList().second) << " ) " << lf;
  out << "Global Position:  " << Amg::toString(this->globalPosition(), 3) << lf;
  out << "Global Covariance Matrix " << Amg::toString(this->globCovariance(), 3)
      << lf;
  out << "Local Parameters " << this->localParameters() << lf;
  out << "Local Covariance " << Amg::toString(this->localCovariance()) << lf;
  out << "Cluster 1 :" << lf << (*this->clusterList().first) << lf;
  out << "Cluster 2 :" << lf << (*this->clusterList().second) << std::endl;

  return out;
}

}  // end of namespace InDet

