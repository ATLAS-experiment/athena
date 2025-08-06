/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODCore/AuxStoreAccessorMacros.h"
// Local include(s):
#include "xAODInDetMeasurement/versions/SpacePoint_v1.h"

// replace by handwritten version to avoid vector copy
// AUXSTORE_OBJECT_SETTER_AND_GETTER( xAOD::SpacePoint_v1, std::vector< xAOD::DetectorIDHashType >,
//  				      elementIdList, setElementIdList )

const std::vector< xAOD::DetectorIDHashType > & xAOD::SpacePoint_v1::elementIdList() const {
   static const Accessor< std::vector< xAOD::DetectorIDHashType >  > acc( "elementIdList" );
   return acc( *this );
}
void xAOD::SpacePoint_v1::setElementIdList(std::vector< xAOD::DetectorIDHashType > && value ) {
   static const Accessor< std::vector< xAOD::DetectorIDHashType >  > acc( "elementIdList" );
   acc( *this ) = std::move(value);
}

static const SG::AuxElement::Accessor< std::array< float, 3 > > globalPosAcc("globalPosition");

xAOD::SpacePoint_v1::ConstVectorMap xAOD::SpacePoint_v1::globalPosition() const {
  const auto& values = globalPosAcc(*this);
  return ConstVectorMap{values.data()};
}

xAOD::SpacePoint_v1::VectorMap xAOD::SpacePoint_v1::globalPosition() {
  auto& values = globalPosAcc(*this);
  return VectorMap{values.data()};
}

AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( xAOD::SpacePoint_v1, float,
				      radius, setRadius )

float xAOD::SpacePoint_v1::x() const
{
  return globalPosAcc(*this)[0];
}

float xAOD::SpacePoint_v1::y() const
{
  return globalPosAcc(*this)[1];
}

float xAOD::SpacePoint_v1::z() const
{
  return globalPosAcc(*this)[2];
}

AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( xAOD::SpacePoint_v1, float,
				      varianceR, setVarianceR )

AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( xAOD::SpacePoint_v1, float,
				      varianceZ, setVarianceZ )

// replace by handwritten version to avoid vector copy
// AUXSTORE_OBJECT_SETTER_AND_GETTER( xAOD::SpacePoint_v1, std::vector< const xAOD::UncalibratedMeasurement* >,
//				   measurements, setMeasurements )

const std::vector< const xAOD::UncalibratedMeasurement* > & xAOD::SpacePoint_v1::measurements() const {
   static const Accessor< std::vector< const xAOD::UncalibratedMeasurement* >  > acc( "measurements" );
   return acc( *this );
}
void xAOD::SpacePoint_v1::setMeasurements(std::vector< const xAOD::UncalibratedMeasurement* > && value ) {
   static const Accessor< std::vector< const xAOD::UncalibratedMeasurement* >  > acc( "measurements" );
   acc( *this ) = std::move(value);
}

AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( xAOD::SpacePoint_v1, float,
				      topHalfStripLength, setTopHalfStripLength )

AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( xAOD::SpacePoint_v1, float,
				      bottomHalfStripLength, setBottomHalfStripLength )

static const SG::AuxElement::Accessor< xAOD::ArrayFloat3 > topStripDirectionAcc( "topStripDirection" );
static const SG::AuxElement::Accessor< xAOD::ArrayFloat3 > bottomStripDirectionAcc( "bottomStripDirection" );
static const SG::AuxElement::Accessor< xAOD::ArrayFloat3 > stripCenterDistanceAcc( "stripCenterDistance" );
static const SG::AuxElement::Accessor< xAOD::ArrayFloat3 > topStripCenterAcc( "topStripCenter" );

xAOD::SpacePoint_v1::ConstVectorMap xAOD::SpacePoint_v1::topStripDirection() const {
  const auto& values = topStripDirectionAcc(*this);
  return ConstVectorMap{values.data()};
}

xAOD::SpacePoint_v1::ConstVectorMap xAOD::SpacePoint_v1::bottomStripDirection() const {
  const auto& values = bottomStripDirectionAcc(*this);
  return ConstVectorMap{values.data()};
}

xAOD::SpacePoint_v1::ConstVectorMap xAOD::SpacePoint_v1::stripCenterDistance() const {
  const auto& values = stripCenterDistanceAcc(*this);
  return ConstVectorMap{values.data()};
}

xAOD::SpacePoint_v1::ConstVectorMap xAOD::SpacePoint_v1::topStripCenter() const {
  const auto& values = topStripCenterAcc(*this);
  return ConstVectorMap{values.data()};
}

xAOD::SpacePoint_v1::VectorMap xAOD::SpacePoint_v1::topStripDirection() {
  auto& values = topStripDirectionAcc(*this);
  return VectorMap{values.data()};
}

xAOD::SpacePoint_v1::VectorMap xAOD::SpacePoint_v1::bottomStripDirection() {
  auto& values = bottomStripDirectionAcc(*this);
  return VectorMap{values.data()};
}

xAOD::SpacePoint_v1::VectorMap xAOD::SpacePoint_v1::stripCenterDistance() {
  auto& values = stripCenterDistanceAcc(*this);
  return VectorMap{values.data()};
}

xAOD::SpacePoint_v1::VectorMap xAOD::SpacePoint_v1::topStripCenter() {
  auto& values = topStripCenterAcc(*this);
  return VectorMap{values.data()};
}

void xAOD::SpacePoint_v1::setSpacePoint(DetectorIDHashType idHash,
					const Eigen::Matrix<float,3,1>& globPos,
					float cov_r, float cov_z,
					std::vector< const xAOD::UncalibratedMeasurement* >&& measurements)
{
  this->setElementIdList({idHash});
  this->globalPosition() = globPos;
  this->setRadius( std::sqrt( globPos(0,0) * globPos(0,0) + globPos(1,0) * globPos(1,0) ) );
  this->setVarianceR(cov_r);
  this->setVarianceZ(cov_z);
  this->setMeasurements(std::move(measurements));
}

void xAOD::SpacePoint_v1::setSpacePoint(std::vector<DetectorIDHashType>&& idHashes,
					const Eigen::Matrix<float,3,1>& globPos,
					float cov_r, float cov_z,
					std::vector< const xAOD::UncalibratedMeasurement* >&& measurements,
					float topHalfStripLength, 
					float bottomHalfStripLength,
					const Eigen::Matrix<float,3,1>& topStripDirection,
					const Eigen::Matrix<float,3,1>& bottomStripDirection,
					const Eigen::Matrix<float,3,1>& stripCenterDistance,
					const Eigen::Matrix<float,3,1>& topStripCenter)
{
  this->setElementIdList(std::move(idHashes));
  this->globalPosition() = globPos;
  this->setRadius( std::sqrt( globPos(0,0) * globPos(0,0) + globPos(1,0) * globPos(1,0) ) );
  this->setVarianceR(cov_r);
  this->setVarianceZ(cov_z);
  this->setMeasurements(std::move(measurements));

  this->setTopHalfStripLength(topHalfStripLength);
  this->setBottomHalfStripLength(bottomHalfStripLength);

  this->topStripDirection() = topStripDirection;
  this->bottomStripDirection() = bottomStripDirection;
  this->stripCenterDistance() = stripCenterDistance;
  this->topStripCenter() = topStripCenter;
}

