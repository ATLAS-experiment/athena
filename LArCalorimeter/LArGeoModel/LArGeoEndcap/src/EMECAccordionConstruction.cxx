/*
  Copyright (C) 2024-2025 CERN for the benefit of the ATLAS collaboration
*/

// EMECAccordionConstruction
// Construct internal structure of the EMEC Inner and Outer Wheels

// Revision history:
// 24-Jan-2024 Evgueni Tcherniaev: Initial version

#include <cmath>
#include <iostream>

#include "LArGeoEndcap/EMECAccordionConstruction.h"

#include "GeoModelKernel/GeoElement.h"
#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoVPhysVol.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoCons.h"
#include "GeoModelKernel/GeoPcon.h"
#include "GeoModelKernel/GeoGenericTrap.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelKernel/GeoIdentifierTag.h"
#include "GeoModelKernel/Units.h"

#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include "RDBAccessSvc/IRDBRecordset.h"

#include "StoreGate/StoreGateSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "GaudiKernel/StatusCode.h"
#include "GeoModelUtilities/DecodeVersionKey.h"
#include "GeoModelInterfaces/IGeoModelSvc.h"
#include "GeoModelInterfaces/IGeoDbTagSvc.h"

#define SI GeoModelKernelUnits

////////////////////////////////////////////////////////////////////////////////
//
//  Set parameters for accordion structure construction
//
void LArGeo::EMECAccordionConstruction::setWheelParameters()
{
    ISvcLocator *svcLocator = Gaudi::svcLocator();
    SmartIF<StoreGateSvc> detStore{svcLocator->service("DetectorStore")};
    if(!detStore.isValid()) {
      throw std::runtime_error("Error in EndcapCryostatConstruction, cannot access DetectorStore");
    }

    // Get GeoModelSvc and RDBAccessSvc
    
    SmartIF<IRDBAccessSvc> rdbAccess{svcLocator->service("RDBAccessSvc")};
    if(!rdbAccess.isValid()){
       throw std::runtime_error("EMECConstruction: cannot locate RDBAccessSvc!");
    }
    
    SmartIF<IGeoModelSvc> geoModelSvc{svcLocator->service("GeoModelSvc")};
    if(!geoModelSvc.isValid()) {
       throw std::runtime_error("EMECAccordionConstruction: cannot locate GeoModelSvc!");
    }
    
    DecodeVersionKey larVersionKey(geoModelSvc, "LAr");
    IRDBRecordset_ptr DB_EmecGeometry = rdbAccess->getRecordsetPtr("EmecGeometry", larVersionKey.tag(), larVersionKey.node());
    if(DB_EmecGeometry->size() == 0){
      DB_EmecGeometry = rdbAccess->getRecordsetPtr("EmecGeometry", "EmecGeometry-00");
    }
    
    IRDBRecordset_ptr emecWheelParameters = rdbAccess->getRecordsetPtr("EmecWheelParameters", larVersionKey.tag(), larVersionKey.node());
    if(emecWheelParameters->size() == 0){
      emecWheelParameters = rdbAccess->getRecordsetPtr("EmecWheelParameters", "EmecWheelParameters-00");
    }


    IRDBRecordset_ptr emecMagicNumbers = rdbAccess->getRecordsetPtr("EmecMagicNumbers", larVersionKey.tag(), larVersionKey.node());
    if(emecMagicNumbers->size() == 0){
      emecMagicNumbers = rdbAccess->getRecordsetPtr("EmecMagicNumbers", "EmecMagicNumbers-00");
    }

    IRDBRecordset_ptr coldContraction = rdbAccess->getRecordsetPtr("ColdContraction", larVersionKey.tag(), larVersionKey.node());
    if(coldContraction->size() == 0){
      coldContraction = rdbAccess->getRecordsetPtr("ColdContraction", "ColdContraction-00");
    }

    IRDBRecordset_ptr emecFan = rdbAccess->getRecordsetPtr("EmecFan", larVersionKey.tag(), larVersionKey.node());
    if(emecFan->size() == 0){
      emecFan = rdbAccess->getRecordsetPtr("EmecFan", "EmecFan-00");
    }


  // Inner wheel accordion wave parameters
  m_innerNoAbsorbes      = (*emecWheelParameters)[0]->getInt("NABS"); // 256
  m_innerNoElectrodes    = (*emecWheelParameters)[0]->getInt("NABS"); // 256
  m_innerNoWaves         = (*emecWheelParameters)[0]->getInt("NACC"); // 6

  m_innerLipWidth        = (*emecMagicNumbers)[0]->getDouble("STRAIGHTSTARTSECTION") * SI::mm; // 2 mm
  m_innerWaveZoneWidth   = (*emecMagicNumbers)[0]->getDouble("ACTIVELENGTH") * SI::mm;         // 510 mm
  m_innerWheelWidth      = m_innerWaveZoneWidth + 2*m_innerLipWidth;                          // 514 mm
  m_innerWaveWidth       = m_innerWaveZoneWidth/m_innerNoWaves; // 510/6 = 85 mm
  m_innerHalfWaveWidth   = m_innerWaveWidth/2;
  m_innerQuaterWaveWidth = m_innerWaveWidth/4;

  // Check values
  if (m_innerNoAbsorbes != 256) {
    throw std::runtime_error("LArGeo::EMECAccordionConstruction::setWheelParameters: wrong number of absorbers. (NABS = " + std::to_string( m_innerNoAbsorbes) + ") for Inner Wheel, expected 256!");
  }
  if (m_innerNoWaves != 6) {
    throw std::runtime_error("LArGeo::EMECAccordionConstruction::setWheelParameters: wrong number of waves. (NACC = " + std::to_string( m_innerNoAbsorbes ) + ") for Inner Wheel, expected 6!");
  }
  if (m_innerLipWidth != 2 * SI::mm) {
    throw std::runtime_error("LArGeo::EMECAccordionConstruction::setWheelParameters: wrong width of absorber lips. (STRAIGHTSTARTSECTION = " + std::to_string( m_innerNoAbsorbes/SI::mm) + "), expected 2 mm!" );
  }
  if (m_innerWaveZoneWidth != 510 * SI::mm) {
    throw std::runtime_error("LArGeo::EMECAccordionConstruction::setWheelParameters: wrong width of absorber active zone. (ACTIVELENGTH = " + std::to_string(m_innerWaveZoneWidth/SI::mm) + "), expected 510 mm!");
  }

  // Outer wheel accordion wave parameters
  m_outerNoAbsorbes      = (*emecWheelParameters)[1]->getInt("NABS"); // 768
  m_outerNoElectrodes    = (*emecWheelParameters)[1]->getInt("NABS"); // 768
  m_outerNoWaves         = (*emecWheelParameters)[1]->getInt("NACC"); // 9

  m_outerLipWidth        = m_innerLipWidth;                          // 2 mm
  m_outerWaveZoneWidth   = m_innerWaveZoneWidth;                     // 510 mm
  m_outerWheelWidth      = m_outerWaveZoneWidth + 2*m_outerLipWidth; // 514 mm
  m_outerWaveWidth       = m_outerWaveZoneWidth/m_outerNoWaves;      // 510/9 = 56.67 mm
  m_outerHalfWaveWidth   = m_outerWaveWidth/2;
  m_outerQuaterWaveWidth = m_outerWaveWidth/4;

  // Check values
  if (m_outerNoAbsorbes != 768) {
    throw std::runtime_error("LArGeo::EMECAccordionConstruction::setWheelParameters: wrong number of absorbers. (NABS = " + std::to_string (m_outerNoAbsorbes) + ") for Outer Wheel, expected 768!");
  }
  if (m_outerNoWaves != 9) {
     throw std::runtime_error("LArGeo::EMECAccordionConstruction::setWheelParameters: wrong number of waves. (NACC = " + std::to_string(m_outerNoAbsorbes) + ") for Outer Wheel, expected 9!" );
  }

  // Inner wheel thiknesses
  m_innerLeadThickness      = (*emecFan)[0]->getDouble("LEADTHICKNESSINNER") * SI::mm;      // 2.2 mm
  m_innerSteelThickness     = (*emecFan)[0]->getDouble("STEELTHICKNESS") * SI::mm;          // 0.2 mm
  m_innerGlueThickness      = (*emecFan)[0]->getDouble("GLUETHICKNESS") * SI::mm;           // 0.1 mm (TRD: 0.15 mm)
  m_innerElectrodeThickness = (*emecFan)[0]->getDouble("ELECTRODETOTALTHICKNESS") * SI::mm; // 0.275 mm

  // Outer wheel thiknesses
  m_outerLeadThickness      = (*emecFan)[0]->getDouble("LEADTHICKNESSOUTER") * SI::mm;      // 1.69 mm (TRD: 1.7 mm)
  m_outerSteelThickness     = m_innerSteelThickness;
  m_outerGlueThickness      = m_innerGlueThickness;
  m_outerElectrodeThickness = m_innerElectrodeThickness;

  // E.T. notes:
  // 1. In LArCustomShapeExtensionSolid.cxx the contraction factor was equal to 0.991, here 0.997
  // 2. There is a discrepancy between specified thickness of glue (0.1 mm) and its value in TDR (0.15 mm)
  // 3. There is a discrepancy between specified thickness of lead in the outer wheel (1.69 mm) and its value in TDR (1.7 mm)
  // 4. Application of the contraction factor to the thicknesses (?) is under question
  // 5. Materials are specified at room temperature (?) instead of LAr temperature
  // 6. Iron (?) is used instead of Steel in absorbers
  //
  // for more information see:
  // https://indico.cern.ch/event/1236615/contributions/5313135/attachments/2610372/4509836/2023.03.13-EMEC_discrepancies.pdf

  // Contraction factor
  m_kContraction = (*coldContraction)[0]->getDouble("ABSORBERCONTRACTION"); // 0.997, in LArCustomShapeExtensionSolid.cxx was 0.991

  // Applying contraction factor to thicknesses (?)
  m_innerLeadThickness  *= m_kContraction;
  m_innerSteelThickness *= m_kContraction;
  m_innerGlueThickness  *= m_kContraction;
  m_innerAbsorberThickness = m_innerLeadThickness + 2*m_innerSteelThickness + 2*m_innerGlueThickness;

  m_outerLeadThickness  *= m_kContraction;
  m_outerSteelThickness *= m_kContraction;
  m_outerGlueThickness  *= m_kContraction;
  m_outerAbsorberThickness = m_outerLeadThickness + 2*m_outerSteelThickness + 2*m_outerGlueThickness;

  // 1./eleInvContraction = 0.99639
  double eleInvContraction = (*coldContraction)[0]->getDouble("ELECTRODEINVCONTRACTION"); // 1.0036256
  m_innerElectrodeThickness /= eleInvContraction;
  m_outerElectrodeThickness /= eleInvContraction;
}

////////////////////////////////////////////////////////////////////////////////
//
//  Set pointer to Inner Wheeel envelope
//
void LArGeo::EMECAccordionConstruction::setInnerWheel(GeoFullPhysVol* innerWheel)
{
  m_innerWheel = innerWheel;
  const GeoShape* shape = innerWheel->getLogVol()->getShape();
    if (shape->type() != "Pcon") {
      throw std::runtime_error( "LArGeo::EMECAccordionConstruction::setInnerWheel: unexpected shape type '"+ shape->type() + "', expected 'Pcon'!");
  }
  const GeoPcon* pcon = static_cast<const GeoPcon*>(shape);
  auto nplanes = pcon->getNPlanes();
  if (nplanes != 2) {
    throw std::runtime_error("LArGeo::EMECAccordionConstruction::setInnerWheel: wrong number of Z planes '" + std::to_string(nplanes) + "', expected '2'!");
  }
  for (unsigned int i = 0; i < nplanes; ++i)
  {
    m_zWheelInner[i] = pcon->getZPlane(i);
    m_rMinInner[i] = pcon->getRMinPlane(i);
    m_rMaxInner[i] = pcon->getRMaxPlane(i);
  }
  // Set Inner Wheel base name
  m_nameInnerWheel = innerWheel->getLogVol()->getName();
}

////////////////////////////////////////////////////////////////////////////////
//
//  Set pointer to Outer Wheeel envelope
//
void LArGeo::EMECAccordionConstruction::setOuterWheel(GeoFullPhysVol* outerWheel)
{
  m_outerWheel = outerWheel;
  const GeoShape* shape = outerWheel->getLogVol()->getShape();
    if (shape->type() != "Pcon") {
    throw std::runtime_error( "LArGeo::EMECAccordionConstruction::setOuterWheel: unexpected shape type '"+ shape->type() + "', expected 'Pcon'!");
   
  }
  const GeoPcon* pcon = static_cast<const GeoPcon*>(shape);
  auto nplanes = pcon->getNPlanes();
  if (nplanes != 3) {
	  throw std::runtime_error("LArGeo::EMECAccordionConstruction::setOuterWheel: wrong number of Z planes" + std::to_string(nplanes) + "', expected '3'!" );
  }
  for (unsigned int i = 0; i < nplanes; ++i)
  {
    m_zWheelOuter[i] = pcon->getZPlane(i);
    m_rMinOuter[i] = pcon->getRMinPlane(i);
    m_rMaxOuter[i] = pcon->getRMaxPlane(i);
  }
  // Set Outer Wheel base name
  m_nameOuterWheel = outerWheel->getLogVol()->getName();
}

////////////////////////////////////////////////////////////////////////////////
//
//  Set Inner wheel slices: (lip)(quater_wave)(11 half_waves)(quater_wave)(lip)
//
void LArGeo::EMECAccordionConstruction::setInnerWheelSlices()
{
  if (!m_innerWheel)
  {
    throw std::runtime_error("LArGeo::EMECAccordionConstruction::setInnerWheelSlices: Inner Wheel volume is not set!" );
  }
  // Compute slices
  m_innerWheelRminIncrement = (m_rMinInner[1] - m_rMinInner[0])/(m_zWheelInner[1] - m_zWheelInner[0]);
  m_innerWheelRmaxIncrement = (m_rMaxInner[1] - m_rMaxInner[0])/(m_zWheelInner[1] - m_zWheelInner[0]);
  m_innerWheelZmin = m_zWheelInner[0];
  m_innerWheelZmax = m_zWheelInner[1];

  m_innerWheelZ[0] = m_innerWheelZmin;
  m_innerWheelZ[1] = m_innerWheelZ[0] + m_innerLipWidth;
  m_innerWheelZ[2] = m_innerWheelZ[1] + m_innerQuaterWaveWidth;
  for (int i = 3; i < s_innerNoBlades - 1; ++i)
  {
    m_innerWheelZ[i] = m_innerWheelZ[i - 1] + m_innerHalfWaveWidth;
  }
  m_innerWheelZ[s_innerNoBlades] = m_innerWheelZmax;
  m_innerWheelZ[s_innerNoBlades - 1] = m_innerWheelZ[s_innerNoBlades] - m_innerLipWidth;
  for (int i = 0; i < s_innerNoBlades + 1; ++i)
  {
    m_innerWheelRmin[i] = m_rMinInner[0] + (m_innerWheelZ[i] - m_innerWheelZ[0])*m_innerWheelRminIncrement;
    m_innerWheelRmax[i] = m_rMaxInner[0] + (m_innerWheelZ[i] - m_innerWheelZ[0])*m_innerWheelRmaxIncrement;
  }
}

////////////////////////////////////////////////////////////////////////////////
//
//  Set Outer wheel slices (lip)(quater_wave)(15_half_waves)(quater_wave)(lip)
//
void LArGeo::EMECAccordionConstruction::setOuterWheelSlices()
{
  if (!m_outerWheel)
  {
    throw std::runtime_error( "LArGeo::EMECAccordionConstruction::setOuterWheelSlices: Outer Wheel volume is not set!");
  }
  // Compute slices
  m_outerWheelRminIncrement[0] = (m_rMinOuter[1] - m_rMinOuter[0])/(m_zWheelOuter[1] - m_zWheelOuter[0]);
  m_outerWheelRminIncrement[1] = (m_rMinOuter[2] - m_rMinOuter[1])/(m_zWheelOuter[2] - m_zWheelOuter[1]);
  m_outerWheelRmaxIncrement[0] = (m_rMaxOuter[1] - m_rMaxOuter[0])/(m_zWheelOuter[1] - m_zWheelOuter[0]);
  m_outerWheelRmaxIncrement[1] = (m_rMaxOuter[2] - m_rMaxOuter[1])/(m_zWheelOuter[2] - m_zWheelOuter[1]);
  m_outerWheelZmin = m_zWheelOuter[0];
  m_outerWheelZmax = m_zWheelOuter[2];

  m_outerWheelZ[0] = m_outerWheelZmin;
  m_outerWheelZ[1] = m_outerWheelZ[0] + m_outerLipWidth;
  m_outerWheelZ[2] = m_outerWheelZ[1] + m_outerQuaterWaveWidth;
  for (int i = 3; i < s_outerNoBlades - 1; ++i)
  {
    m_outerWheelZ[i] = m_outerWheelZ[i - 1] + m_outerHalfWaveWidth;
  }
  m_outerWheelZ[s_outerNoBlades] = m_outerWheelZmax;
  m_outerWheelZ[s_outerNoBlades - 1] = m_outerWheelZ[s_outerNoBlades] - m_outerLipWidth;
  for (int i = 0; i < s_outerNoBlades + 1; ++i)
  {
    m_outerWheelRmin[i] = m_rMinOuter[0] + (m_outerWheelZ[i] - m_outerWheelZ[0])*m_outerWheelRminIncrement[0];
    m_outerWheelRmax[i] = m_rMaxOuter[0] + (m_outerWheelZ[i] - m_outerWheelZ[0])*m_outerWheelRmaxIncrement[0];
    if (m_outerWheelRmax[i] > m_rMaxOuter[2]) m_outerWheelRmax[i] = m_rMaxOuter[2]; // 2034 mm starting from i = 3
  }
}

////////////////////////////////////////////////////////////////////////////////
//
//  Set material
//
void
LArGeo::EMECAccordionConstruction::setMaterial(const std::string& name,
                                               const GeoMaterial* material)
{
  if (name == "LiquidArgon") m_materialLiquidArgon = material;
  else if (name == "Kapton") m_materialKapton      = material;
  else if (name == "Lead"  ) m_materialLead        = material;
  else if (name == "Steel" ) m_materialSteel       = material;
  else if (name == "Glue"  ) m_materialGlue        = material;
  else
  {
    throw std::runtime_error("LArGeo::EMECAccordionConstruction::setMaterial: unexpected material name '" + name + "'!");
  }
}

////////////////////////////////////////////////////////////////////////////////
//
//  Get Inner absorber data from technical information
//
void
LArGeo::EMECAccordionConstruction::getInnerAbsorberData(double& wmin, double& wmax,
                                                        double& llip1, double& ylip1,
                                                        double& llip2, double& ylip2) const
{
  // Points taken from technical drawing
  GeoTwoVector A[14+1] = {};               // A[0] is not used
  A[1]  = GeoTwoVector(  1.700, 1019.908) * SI::mm;
  A[2]  = GeoTwoVector( 36.595, 1022.643) * SI::mm;
  A[3]  = GeoTwoVector(110.351, 1024.416) * SI::mm;
  A[4]  = GeoTwoVector(184.545, 1020.841) * SI::mm;
  A[5]  = GeoTwoVector(258.785, 1011.865) * SI::mm;
  A[6]  = GeoTwoVector(332.678,  997.463) * SI::mm;
  A[7]  = GeoTwoVector(405.824,  977.640) * SI::mm;
  A[8]  = GeoTwoVector(477.824,  952.430) * SI::mm;
  A[9]  = GeoTwoVector(548.280,  921.895) * SI::mm;
  A[10] = GeoTwoVector(616.797,  886.130) * SI::mm;
  A[11] = GeoTwoVector(682.985,  845.257) * SI::mm;
  A[12] = GeoTwoVector(746.463,  799.426) * SI::mm;
  A[13] = GeoTwoVector(806.859,  748.819) * SI::mm;
  A[14] = GeoTwoVector(835.788,  723.065) * SI::mm;

  GeoTwoVector B[14+1] = {};               // B[0] is not used
  B[1]  = GeoTwoVector(  1.132, 710.702) * SI::mm;
  B[2]  = GeoTwoVector( 25.476, 711.930) * SI::mm;
  B[3]  = GeoTwoVector( 76.670, 711.749) * SI::mm;
  B[4]  = GeoTwoVector(127.967, 707.873) * SI::mm;
  B[5]  = GeoTwoVector(179.099, 700.288) * SI::mm;
  B[6]  = GeoTwoVector(229.797, 688.988) * SI::mm;
  B[7]  = GeoTwoVector(279.791, 674.025) * SI::mm;
  B[8]  = GeoTwoVector(328.814, 655.414) * SI::mm;
  B[9]  = GeoTwoVector(376.600, 633.227) * SI::mm;
  B[10] = GeoTwoVector(422.886, 607.546) * SI::mm;
  B[11] = GeoTwoVector(467.418, 578.473) * SI::mm;
  B[12] = GeoTwoVector(509.946, 546.127) * SI::mm;
  B[13] = GeoTwoVector(550.228, 510.649) * SI::mm;
  B[14] = GeoTwoVector(568.629, 492.593) * SI::mm;

  GeoTwoVector C[4+1] = {};               // C[0] is not used
  C[1]  = GeoTwoVector( -1.754, 978.281) * SI::mm;
  C[2]  = GeoTwoVector(  1.668, 978.279) * SI::mm;
  C[3]  = GeoTwoVector(803.643, 696.378) * SI::mm;
  C[4]  = GeoTwoVector(805.917, 693.745) * SI::mm;

  GeoTwoVector D[4+1] = {};               // D[0] is not used
  D[1]  = GeoTwoVector( -1.894, 786.281) * SI::mm;
  D[2]  = GeoTwoVector(  1.186, 786.279) * SI::mm;
  D[3]  = GeoTwoVector(610.986, 529.319) * SI::mm;
  D[4]  = GeoTwoVector(613.041, 526.940) * SI::mm;

  // Compute angle between rays
  int k1 = 2, k2 = 13; // take rays 2 and 13
  double cosa = A[k1].dot(A[k2])/(A[k1].norm()*A[k2].norm());
  double ang = std::acos(cosa)/(k2 - k1);

  // Compute bottom and top width of the accordion blade
  double rmin = B[1].norm();
  double rmax = A[14].norm();
  wmin = 2.*rmin*std::sin(ang/2.);
  wmax = 2.*rmax*std::sin(ang/2.);

  // Compute length and position of the first lip
  llip1 = (C[2] - D[2]).norm();
  ylip1 = (D[2] - B[1]).norm() + llip1/2.;

  // Compute length and position of the last lip
  llip2 = (C[3] - D[3]).norm();
  ylip2 = (D[3] - B[14]).norm() + llip2/2.;
}

////////////////////////////////////////////////////////////////////////////////
//
//  Get Outer absorber data from technical information
//
void
LArGeo::EMECAccordionConstruction::getOuterAbsorberData(double& wmin, double& wmax,
                                                        double& llip1, double& ylip1,
                                                        double& llip2, double& ylip2) const
{
  // Points taken from technical drawing
  GeoTwoVector A[20+1] = {};               // A[0] is not used
  A[1]  = GeoTwoVector(  1.773, 3438.343) * SI::mm;
  A[2]  = GeoTwoVector( 26.515, 3445.688) * SI::mm;
  A[3]  = GeoTwoVector( 79.893, 3460.218) * SI::mm;
  A[4]  = GeoTwoVector(133.523, 3468.701) * SI::mm;
  A[5]  = GeoTwoVector(186.888, 3466.236) * SI::mm;
  A[6]  = GeoTwoVector(240.209, 3462.949) * SI::mm;
  A[7]  = GeoTwoVector(293.473, 3458.842) * SI::mm;
  A[8]  = GeoTwoVector(346.668, 3453.916) * SI::mm;
  A[9]  = GeoTwoVector(399.780, 3448.172) * SI::mm;
  A[10] = GeoTwoVector(452.798, 3441.612) * SI::mm;
  A[11] = GeoTwoVector(505.708, 3434.236) * SI::mm;
  A[12] = GeoTwoVector(558.499, 3426.047) * SI::mm;
  A[13] = GeoTwoVector(611.157, 3417.046) * SI::mm;
  A[14] = GeoTwoVector(663.670, 3407.236) * SI::mm;
  A[15] = GeoTwoVector(716.027, 3396.619) * SI::mm;
  A[16] = GeoTwoVector(768.214, 3385.198) * SI::mm;
  A[17] = GeoTwoVector(820.219, 3372.975) * SI::mm;
  A[18] = GeoTwoVector(872.029, 3359.953) * SI::mm;
  A[19] = GeoTwoVector(923.633, 3346.135) * SI::mm;
  A[20] = GeoTwoVector(947.625, 3339.413) * SI::mm;

  GeoTwoVector B[20+1] = {};               // B[0] is not used
  B[1]  = GeoTwoVector(  0.810, 2046.995) * SI::mm;
  B[2]  = GeoTwoVector( 15.769, 2049.165) * SI::mm;
  B[3]  = GeoTwoVector( 47.410, 2053.375) * SI::mm;
  B[4]  = GeoTwoVector( 79.185, 2057.095) * SI::mm;
  B[5]  = GeoTwoVector(111.086, 2060.323) * SI::mm;
  B[6]  = GeoTwoVector(143.105, 2063.055) * SI::mm;
  B[7]  = GeoTwoVector(175.234, 2065.288) * SI::mm;
  B[8]  = GeoTwoVector(207.466, 2067.020) * SI::mm;
  B[9]  = GeoTwoVector(239.792, 2068.248) * SI::mm;
  B[10] = GeoTwoVector(272.205, 2068.969) * SI::mm;
  B[11] = GeoTwoVector(304.697, 2069.181) * SI::mm;
  B[12] = GeoTwoVector(337.260, 2068.883) * SI::mm;
  B[13] = GeoTwoVector(369.885, 2068.071) * SI::mm;
  B[14] = GeoTwoVector(402.566, 2066.744) * SI::mm;
  B[15] = GeoTwoVector(435.293, 2064.899) * SI::mm;
  B[16] = GeoTwoVector(468.058, 2062.536) * SI::mm;
  B[17] = GeoTwoVector(500.853, 2059.653) * SI::mm;
  B[18] = GeoTwoVector(533.670, 2056.247) * SI::mm;
  B[19] = GeoTwoVector(566.501, 2052.318) * SI::mm;
  B[20] = GeoTwoVector(582.048, 2050.271) * SI::mm;

  GeoTwoVector C[4+1] = {};                // C[0] is not used
  C[1]  = GeoTwoVector( -1.682, 3416.858) * SI::mm;
  C[2]  = GeoTwoVector(  1.756, 3416.858) * SI::mm;
  C[3]  = GeoTwoVector(932.782, 3287.070) * SI::mm;
  C[3]  = GeoTwoVector(936.090, 3286.130) * SI::mm;

  GeoTwoVector D[4+1] = {};                // D[0] is not used
  D[1]  = GeoTwoVector( -1.975, 2088.411) * SI::mm;
  D[2]  = GeoTwoVector(  0.839, 2088.410) * SI::mm;
  D[3]  = GeoTwoVector(593.526, 2090.743) * SI::mm;
  D[4]  = GeoTwoVector(596.271, 2089.963) * SI::mm;

  // Compute angle between rays
  int k1 = 4, k2 = 19; // take rays 4 and 19
  double cosa = A[k1].dot(A[k2])/(A[k1].norm()*A[k2].norm());
  double ang = std::acos(cosa)/(k2 - k1);

  // Compute bottom and top width of the accordion blade
  double rmin = B[1].norm();
  double rmax = A[19].norm(); // A[19] has slightly bigger radius than A[20]
  wmin = 2.*rmin*std::sin(ang/2.);
  wmax = 2.*rmax*std::sin(ang/2.);

  // Compute length and position of the first lip
  llip1 = (C[2] - D[2]).norm();
  ylip1 = (D[2] - B[1]).norm() + llip1/2.;

  // Compute length and position of the last lip
  llip2 = (C[3] - D[3]).norm();
  ylip2 = (D[3] - B[20]).norm() + llip2/2.;
}

////////////////////////////////////////////////////////////////////////////////
//
//  Compute uncut Blade corners
//
void
LArGeo::EMECAccordionConstruction::getBladeCorners(double wmin, double wmax, double thickness,
                                                   double rmin, double rmax, double zdel,
                                                   GeoThreeVector corners[8]) const
{
  double z = zdel/2.;

  double xmin = std::sqrt(wmin*wmin - zdel*zdel)/2.;
  double dxmin = (thickness/2.)*(wmin/zdel);
  double ymin = rmin;

  double xmax = std::sqrt(wmax*wmax - zdel*zdel)/2.;
  double dxmax = (thickness/2.)*(wmax/zdel);
  double xtmp = (xmax + dxmax);
  double ymax = std::sqrt(rmax*rmax - xtmp*xtmp);

  corners[0] = GeoThreeVector(-xmin + dxmin, ymin, -z);
  corners[1] = GeoThreeVector(-xmin - dxmin, ymin, -z);
  corners[2] = GeoThreeVector(-xmax - dxmax, ymax, -z);
  corners[3] = GeoThreeVector(-xmax + dxmax, ymax, -z);

  corners[4] = GeoThreeVector(+xmin + dxmin, ymin, +z);
  corners[5] = GeoThreeVector(+xmin - dxmin, ymin, +z);
  corners[6] = GeoThreeVector(+xmax - dxmax, ymax, +z);
  corners[7] = GeoThreeVector(+xmax + dxmax, ymax, +z);
}

////////////////////////////////////////////////////////////////////////////////
//
//  Compute cut plane for bottom of Blade
//
LArGeo::EMECAccordionConstruction::CutPlane
LArGeo::EMECAccordionConstruction::getBottomCutPlane(double zmin, double rmin,
                                                     double zmax, double rmax) const
{
  GeoThreeVector pmin(0, rmin, zmin);
  GeoThreeVector pmax(0, rmax, zmax);
  GeoThreeVector v = (pmax - pmin).normalized();

  LArGeo::EMECAccordionConstruction::CutPlane plane;
  plane.m_n = GeoThreeVector(0., -v.z(), v.y());
  plane.m_d = -plane.m_n.dot(pmin);
  return plane;
}

////////////////////////////////////////////////////////////////////////////////
//
//  Compute cut plane for top of Blade
//
LArGeo::EMECAccordionConstruction::CutPlane
LArGeo::EMECAccordionConstruction::getTopCutPlane(double zmin, double rmin,
                                                  double zmax, double rmax,
                                                  const GeoThreeVector corners[8]) const
{
  GeoThreeVector pbot(std::min(corners[0].x(), corners[1].x()), corners[0].y(), 0);
  GeoThreeVector ptop(std::min(corners[2].x(), corners[3].x()), corners[3].y(), 0);

  GeoThreeVector v = (ptop - pbot).normalized();
  double d = v.x()*pbot.y() - v.y()*pbot.x();
  double r = ptop.norm();
  double l = std::sqrt(r*r - d*d);
  double wmin = std::sqrt(rmin*rmin - d*d);
  double wmax = std::sqrt(rmax*rmax - d*d);
  double ymin = ptop.y() - v.y()*(l - wmin);
  double ymax = ptop.y() - v.y()*(l - wmax);

  GeoThreeVector pmin(0, ymin, zmin);
  GeoThreeVector pmax(0, ymax, zmax);
  v = (pmax - pmin).normalized();

  LArGeo::EMECAccordionConstruction::CutPlane plane;
  plane.m_n = GeoThreeVector(0., -v.z(), v.y());
  plane.m_d = -plane.m_n.dot(pmin);
  return plane;
}

////////////////////////////////////////////////////////////////////////////////
//
// Find intersection of line segment with plane
//
GeoThreeVector
LArGeo::EMECAccordionConstruction::IntersectionPoint(const GeoThreeVector& p1,
                                                     const GeoThreeVector& p2,
                                                     const LArGeo::EMECAccordionConstruction::CutPlane& plane) const
{
  double d1 = plane.m_n.dot(p1) + plane.m_d;
  double d2 = plane.m_n.dot(p2) + plane.m_d;
  return (d2*p1 - d1*p2)/(d2 - d1);
}

////////////////////////////////////////////////////////////////////////////////
//
// Cut Blade and construct solid
//   icase = 1 - first quater-wave, negative inclination
//   icase = 2 - half-wave, positive inclination
//   icase = 3 - half-wave, negative inclination
//   icase = 4 - last quater-wave, positive inclination
//
GeoShape*
LArGeo::EMECAccordionConstruction::constructBlade(int icase,
                                                  const GeoThreeVector corners[8], double xscale,
                                                  double pz1, double pr1min, double pr1max,
                                                  double pz2, double pr2min, double pr2max) const
{
  double z1 = pz1, r1min = pr1min, r1max = pr1max;
  double z2 = pz2, r2min = pr2min, r2max = pr2max;
  if (icase == 1) // First quater-wave blade
  {
    z1 -= (z2 - z1);
    r1min -= (r2min - r1min);
    r1max -= (r2max - r1max);
  }
  if (icase == 4) // Last quater-wave blade
  {
    z2 += (z2 - z1);
    r2min += (r2min - r1min);
    r2max += (r2max - r1max);
  }

  // Prepare Blade for cutting
  std::vector<GeoThreeVector> v3(8);
  double kx = (icase == 2) ? 1 : -1; // set inclination

  // Move Blade to required position
  for (int i = 0; i < 8; ++i)
  {
    v3[i] = GeoThreeVector(kx*corners[i].x(), corners[i].y(), corners[i].z() + (z1 + z2)/2.);
  }

  // Change order vertices in case of negative inclination
  if (kx < 0)
  {
    std::swap(v3[0],v3[1]);
    std::swap(v3[2],v3[3]);
    std::swap(v3[4],v3[5]);
    std::swap(v3[6],v3[7]);
  }

  // Cut Blade by bottom plane
  LArGeo::EMECAccordionConstruction::CutPlane botPlane = getBottomCutPlane(z1, r1min, z2, r2min);
  v3[0] = IntersectionPoint(v3[0], v3[3], botPlane);
  v3[1] = IntersectionPoint(v3[1], v3[2], botPlane);
  v3[4] = IntersectionPoint(v3[4], v3[7], botPlane);
  v3[5] = IntersectionPoint(v3[5], v3[6], botPlane);

  // Cut Blade by top plane
  LArGeo::EMECAccordionConstruction::CutPlane topPlane = getTopCutPlane(z1, r1max, z2, r2max, corners);
  v3[3] = IntersectionPoint(v3[0], v3[3], topPlane);
  v3[2] = IntersectionPoint(v3[1], v3[2], topPlane);
  v3[7] = IntersectionPoint(v3[4], v3[7], topPlane);
  v3[6] = IntersectionPoint(v3[5], v3[6], topPlane);

  if (icase == 1) // First quater-wave blade
  {
    v3[0] = (v3[0] + v3[4])/2.;
    v3[1] = (v3[1] + v3[5])/2.;
    v3[2] = (v3[2] + v3[6])/2.;
    v3[3] = (v3[3] + v3[7])/2.;
  }

  if (icase == 4) // Last quater-wave blade
  {
    v3[4] = (v3[0] + v3[4])/2.;
    v3[5] = (v3[1] + v3[5])/2.;
    v3[6] = (v3[2] + v3[6])/2.;
    v3[7] = (v3[3] + v3[7])/2.;
  }

  // Construct GenericTrap solid
  //
  //  +--------Z
  //  |
  //  |          3 +----------+ 7
  //  |   0 +------|---+ 4    |
  //  X     |    2 +----------+ 6
  //      1 +----------+ 5
  //
  std::vector<GeoTwoVector> v2(8);
  for (int i = 0; i < 8; i += 2)
  {
    double xmid = (v3[i + 1].x() + v3[i].x())/2.;
    double xdel = (v3[i + 1].x() - v3[i].x())*xscale/2.;
    v2[i + 0] = GeoTwoVector(xmid - xdel, v3[i + 0].y());
    v2[i + 1] = GeoTwoVector(xmid + xdel, v3[i + 1].y());
  }
  return new GeoGenericTrap((pz2 - pz1)/2., v2);
}

////////////////////////////////////////////////////////////////////////////////
//
// Construct Slices in Inner Wheel, set offsets
//
void LArGeo::EMECAccordionConstruction::constructInnerSlices()
{
  std::string name;
  GeoShape* solid;
  for (int islice = 0; islice < s_innerNoBlades; ++islice)
  {
    name = m_nameInnerWheel + m_nameSlice + m_nameSuffix[islice];
    double dz = 0.5*(m_innerWheelZ[islice+1] - m_innerWheelZ[islice]);
    solid = new GeoCons(m_innerWheelRmin[islice], m_innerWheelRmin[islice+1],
                        m_innerWheelRmax[islice], m_innerWheelRmax[islice+1],
                        dz, 0.*SI::deg, 360.*SI::deg);
    m_innerSlice[islice] = new GeoPhysVol(new GeoLogVol(name, solid, m_materialLiquidArgon));
    m_innerSliceOffset[islice] = GeoThreeVector(0., 0., m_innerWheelZ[islice] + dz);
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// Construct Slices in Outer Wheel, set offsets
//
void LArGeo::EMECAccordionConstruction::constructOuterSlices()
{
  std::string name;
  GeoShape* solid;
  for (int islice = 0; islice < s_outerNoBlades; ++islice)
  {
    name = m_nameOuterWheel + m_nameSlice + m_nameSuffix[islice];
    double dz = 0.5*(m_outerWheelZ[islice+1] - m_outerWheelZ[islice]);
    solid = new GeoCons(m_outerWheelRmin[islice], m_outerWheelRmin[islice+1],
                        m_outerWheelRmax[islice], m_outerWheelRmax[islice+1],
                        dz, 0.*SI::deg, 360.*SI::deg);
    m_outerSlice[islice] = new GeoPhysVol(new GeoLogVol(name, solid, m_materialLiquidArgon));
    m_outerSliceOffset[islice] = GeoThreeVector(0., 0., m_outerWheelZ[islice] + dz);
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// Construct Lips of Absorbers and Electrodes for Inner Wheel
// Set offsets
//
void
LArGeo::EMECAccordionConstruction::constructInnerLips(double innerLipLength1,
                                                      double innerLipPosition1,
                                                      double innerLipLength2,
                                                      double innerLipPosition2)
{
  double dx, dy, dz, xoffset, yoffset, zoffset;
  std::string name;
  GeoShape* solid;

  // Contruct logical volumes for 1st lip
  int iblade = 0;
  dx = 0.5*m_innerAbsorberThickness;
  dy = 0.5*innerLipLength1;
  dz = 0.5*m_innerLipWidth;
  name = m_nameInnerWheel + m_nameAbsorber + m_nameSuffix[iblade];
  solid = new GeoBox(dx, dy, dz);
  m_innerAbsorber[iblade] = new GeoPhysVol(new GeoLogVol(name, solid, m_materialSteel));

  name = m_nameInnerWheel + m_nameGlue + m_nameSuffix[iblade];
  solid = new GeoBox(dx*m_innerGlueRatio, dy, dz);
  m_innerGlue[iblade] = new GeoPhysVol(new GeoLogVol(name, solid, m_materialGlue));

  name = m_nameInnerWheel + m_nameLead + m_nameSuffix[iblade];
  solid = new GeoBox(dx*m_innerLeadRatio, dy, dz);
  m_innerLead[iblade] = new GeoPhysVol(new GeoLogVol(name, solid, m_materialLead));

  name = m_nameInnerWheel + m_nameElectrode + m_nameSuffix[iblade];
  dx = 0.5*m_innerElectrodeThickness;
  solid = new GeoBox(dx, dy, dz);
  m_innerElectrode[iblade] = new GeoPhysVol(new GeoLogVol(name, solid, m_materialKapton));

  // Set offsets
  xoffset = 0.;
  yoffset = m_innerWheelRmin[1] + innerLipPosition1;
  zoffset = m_innerWheelZmin + dz;
  m_innerAbsorberOffset[iblade] = GeoThreeVector(xoffset, yoffset, zoffset);
  m_innerElectrodeOffset[iblade] = GeoThreeVector(xoffset, yoffset, zoffset);

  // Contruct logical volumes for 2nd lip
  iblade = s_innerNoBlades - 1;
  dx = 0.5*m_innerAbsorberThickness;
  dy = 0.5*innerLipLength2;
  dz = 0.5*m_innerLipWidth;
  name = m_nameInnerWheel + m_nameAbsorber + m_nameSuffix[iblade];
  solid = new GeoBox(dx, dy, dz);
  m_innerAbsorber[iblade] = new GeoPhysVol(new GeoLogVol(name, solid, m_materialSteel));

  name = m_nameInnerWheel + m_nameGlue + m_nameSuffix[iblade];
  solid = new GeoBox(dx*m_innerGlueRatio, dy, dz);
  m_innerGlue[iblade] = new GeoPhysVol(new GeoLogVol(name, solid, m_materialGlue));

  name = m_nameInnerWheel + m_nameLead + m_nameSuffix[iblade];
  solid = new GeoBox(dx*m_innerLeadRatio, dy, dz);
  m_innerLead[iblade] = new GeoPhysVol(new GeoLogVol(name, solid, m_materialLead));

  name = m_nameInnerWheel + m_nameElectrode + m_nameSuffix[iblade];
  dx = 0.5*m_innerElectrodeThickness;
  solid = new GeoBox(dx, dy, dz);
  m_innerElectrode[iblade] = new GeoPhysVol(new GeoLogVol(name, solid, m_materialKapton));

  // Set offsets
  xoffset = 0.;
  yoffset = m_innerWheelRmin[s_innerNoBlades - 1] + innerLipPosition2;
  zoffset = m_innerWheelZmax - dz;
  m_innerAbsorberOffset[iblade] = GeoThreeVector(xoffset, yoffset, zoffset);
  m_innerElectrodeOffset[iblade] = GeoThreeVector(xoffset, yoffset, zoffset);
}

////////////////////////////////////////////////////////////////////////////////
//
// Construct Lips of Absorbers and Electrodes for Outer Wheel
// Set offsets
//
void LArGeo::EMECAccordionConstruction::constructOuterLips(double outerLipLength1,
                                                           double outerLipPosition1,
                                                           double outerLipLength2,
                                                           double outerLipPosition2)
{
  double dx, dy, dz, xoffset, yoffset, zoffset;
  std::string name;
  GeoShape* solid;

  // Contruct logical volumes for 1st lip
  int iblade = 0;
  dx = 0.5*m_outerAbsorberThickness;
  dy = 0.5*outerLipLength1;
  dz = 0.5*m_outerLipWidth;
  name = m_nameOuterWheel + m_nameAbsorber + m_nameSuffix[iblade];
  solid = new GeoBox(dx, dy, dz);
  m_outerAbsorber[iblade] = new GeoPhysVol(new GeoLogVol(name, solid, m_materialSteel));

  name = m_nameOuterWheel + m_nameGlue + m_nameSuffix[iblade];
  solid = new GeoBox(dx*m_outerGlueRatio, dy, dz);
  m_outerGlue[iblade] = new GeoPhysVol(new GeoLogVol(name, solid, m_materialGlue));

  name = m_nameOuterWheel + m_nameLead + m_nameSuffix[iblade];
  solid = new GeoBox(dx*m_outerLeadRatio, dy, dz);
  m_outerLead[iblade] = new GeoPhysVol(new GeoLogVol(name, solid, m_materialLead));

  name = m_nameOuterWheel + m_nameElectrode + m_nameSuffix[iblade];
  dx = 0.5*m_outerElectrodeThickness;
  solid = new GeoBox(dx, dy, dz);
  m_outerElectrode[iblade] = new GeoPhysVol(new GeoLogVol(name, solid, m_materialKapton));

  // Set offsets
  xoffset = 0.;
  yoffset = m_outerWheelRmin[1] + outerLipPosition1;
  zoffset = m_outerWheelZmin + dz;
  m_outerAbsorberOffset[iblade] = GeoThreeVector(xoffset, yoffset, zoffset);
  m_outerElectrodeOffset[iblade] = GeoThreeVector(xoffset, yoffset, zoffset);

  // Contruct logical volumes for 2nd lip
  iblade = s_outerNoBlades - 1;
  dx = 0.5*m_outerAbsorberThickness;
  dy = 0.5*outerLipLength2;
  dz = 0.5*m_outerLipWidth;
  name = m_nameOuterWheel + m_nameAbsorber + m_nameSuffix[iblade];
  solid = new GeoBox(dx, dy, dz);
  m_outerAbsorber[iblade] = new GeoPhysVol(new GeoLogVol(name, solid, m_materialSteel));

  name = m_nameOuterWheel + m_nameGlue + m_nameSuffix[iblade];
  solid = new GeoBox(dx*m_outerGlueRatio, dy, dz);
  m_outerGlue[iblade] = new GeoPhysVol(new GeoLogVol(name, solid, m_materialGlue));

  name = m_nameOuterWheel + m_nameLead + m_nameSuffix[iblade];
  solid = new GeoBox(dx*m_outerLeadRatio, dy, dz);
  m_outerLead[iblade] = new GeoPhysVol(new GeoLogVol(name, solid, m_materialLead));

  name = m_nameOuterWheel + m_nameElectrode + m_nameSuffix[iblade];
  dx = 0.5*m_outerElectrodeThickness;
  solid = new GeoBox(dx, dy, dz);
  m_outerElectrode[iblade] = new GeoPhysVol(new GeoLogVol(name, solid, m_materialKapton));

  // Set offsets
  xoffset = 0.;
  yoffset = m_outerWheelRmin[s_outerNoBlades -1] + outerLipPosition2;
  zoffset = m_outerWheelZmax - dz;
  m_outerAbsorberOffset[iblade] = GeoThreeVector(xoffset, yoffset, zoffset);
  m_outerElectrodeOffset[iblade] = GeoThreeVector(xoffset, yoffset, zoffset);
}

////////////////////////////////////////////////////////////////////////////////
//
// Construct Blades of Absorbers and Electrodes for Inner Wheel
// Set offsets
//
void
LArGeo::EMECAccordionConstruction::constructInnerBlades(const GeoThreeVector innerCorners[8],
                                                        const GeoThreeVector innerElectrodeCorners[8])
{
  std::string name;
  GeoShape* solid;

  for (int iblade = 1; iblade < s_innerNoBlades - 1; ++iblade)
  {
    int icase = (iblade%2 == 0) ? 2 : 3;        // positive/negative inclination
    if (iblade == 1) icase = 1;                 // first quater-wave
    if (iblade == s_innerNoBlades - 2) icase = 4; // last quater-wave

    double z1 = m_innerWheelZ[iblade];
    double r1min = m_innerWheelRmin[iblade];
    double r1max = m_innerWheelRmax[iblade];
    double z2 = m_innerWheelZ[iblade + 1];
    double r2min = m_innerWheelRmin[iblade + 1];
    double r2max = m_innerWheelRmax[iblade + 1];

    // Construct volumes for Absorber, Glue, Lead and Electrode
    name = m_nameInnerWheel + m_nameAbsorber + m_nameSuffix[iblade];
    solid = constructBlade(icase, innerCorners, 1., z1, r1min, r1max, z2, r2min, r2max);
    m_innerAbsorber[iblade] = new GeoPhysVol(new GeoLogVol(name, solid, m_materialSteel));

    name = m_nameInnerWheel + m_nameGlue + m_nameSuffix[iblade];
    solid = constructBlade(icase, innerCorners, m_innerGlueRatio, z1, r1min, r1max, z2, r2min, r2max);
    m_innerGlue[iblade] = new GeoPhysVol(new GeoLogVol(name, solid, m_materialGlue));

    name = m_nameInnerWheel + m_nameLead + m_nameSuffix[iblade];
    solid = constructBlade(icase, innerCorners, m_innerLeadRatio, z1, r1min, r1max, z2, r2min, r2max);
    m_innerLead[iblade] = new GeoPhysVol(new GeoLogVol(name, solid, m_materialLead));

    name = m_nameInnerWheel + m_nameElectrode + m_nameSuffix[iblade];
    solid = constructBlade(icase, innerElectrodeCorners, 1., z1, r1min, r1max, z2, r2min, r2max);
    m_innerElectrode[iblade] = new GeoPhysVol(new GeoLogVol(name, solid, m_materialKapton));

    // Set offsets
    m_innerAbsorberOffset[iblade]  = GeoThreeVector(0., 0., (z1 + z2)/2.);
    m_innerElectrodeOffset[iblade] = GeoThreeVector(0., 0., (z1 + z2)/2.);
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// Construct Blades of Absorbers and Electrodes for Outer Wheel
// Set offsets
//
void
LArGeo::EMECAccordionConstruction::constructOuterBlades(const GeoThreeVector outerCorners[8],
                                                        const GeoThreeVector outerElectrodeCorners[8])
{
  std::string name;
  GeoShape* solid;

  for (int iblade = 1; iblade < s_outerNoBlades - 1; ++iblade)
  {
    int icase = (iblade%2 == 0) ? 2 : 3;        // positive/negative inclination
    if (iblade == 1) icase = 1;                 // first quater-wave
    if (iblade == s_outerNoBlades - 2) icase = 4; // last quater-wave

    double z1 = m_outerWheelZ[iblade];
    double r1min = m_outerWheelRmin[iblade];
    double r1max = m_outerWheelRmax[iblade];
    double z2 = m_outerWheelZ[iblade + 1];
    double r2min = m_outerWheelRmin[iblade + 1];
    double r2max = m_outerWheelRmax[iblade + 1];

    // Construct volumes for Absorber, Glue, Lead and Electrode
    name = m_nameOuterWheel + m_nameAbsorber + m_nameSuffix[iblade];
    solid = constructBlade(icase, outerCorners, 1., z1, r1min, r1max, z2, r2min, r2max);
    m_outerAbsorber[iblade] = new GeoPhysVol(new GeoLogVol(name, solid, m_materialSteel));

    name = m_nameOuterWheel + m_nameGlue + m_nameSuffix[iblade];
    solid = constructBlade(icase, outerCorners, m_outerGlueRatio, z1, r1min, r1max, z2, r2min, r2max);
    m_outerGlue[iblade] = new GeoPhysVol(new GeoLogVol(name, solid, m_materialGlue));

    name = m_nameOuterWheel + m_nameLead + m_nameSuffix[iblade];
    solid = constructBlade(icase, outerCorners, m_outerLeadRatio, z1, r1min, r1max, z2, r2min, r2max);
    m_outerLead[iblade] = new GeoPhysVol(new GeoLogVol(name, solid, m_materialLead));

    // Construct solid for Electrode
    name = m_nameOuterWheel + m_nameElectrode + m_nameSuffix[iblade];
    solid = constructBlade(icase, outerElectrodeCorners, 1., z1, r1min, r1max, z2, r2min, r2max);
    m_outerElectrode[iblade] = new GeoPhysVol(new GeoLogVol(name, solid, m_materialKapton));

    // Set offsets
    m_outerAbsorberOffset[iblade]  = GeoThreeVector(0., 0., (z1 + z2)/2.);
    m_outerElectrodeOffset[iblade] = GeoThreeVector(0., 0., (z1 + z2)/2.);
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// Place Glue and Lead into Inner Wheel Absorbers
//
void LArGeo::EMECAccordionConstruction::placeInnerGlueAndLead()
{
  for (int iblade = 0; iblade < s_innerNoBlades; ++iblade)
  {
    m_innerGlue[iblade]->add(m_innerLead[iblade]);
    m_innerAbsorber[iblade]->add(m_innerGlue[iblade]);
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// Place Glue and Lead into Outer Wheel Absorbers
//
void LArGeo::EMECAccordionConstruction::placeOuterGlueAndLead()
{
  for (int iblade = 0; iblade < s_outerNoBlades; ++iblade)
  {
    m_outerGlue[iblade]->add(m_outerLead[iblade]);
    m_outerAbsorber[iblade]->add(m_outerGlue[iblade]);
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// Place Slices in Inner Wheel, if needed
//
void LArGeo::EMECAccordionConstruction::placeInnerSlices(bool makeSlices)
{
  if (!makeSlices) return;
  for (int islice = 0; islice < s_innerNoBlades; ++islice)
  {
    double x = m_innerSliceOffset[islice].x();
    double y = m_innerSliceOffset[islice].y();
    double z = m_innerSliceOffset[islice].z();
    m_innerWheel->add(new GeoTransform(GeoTrf::Translate3D(x, y, z)));
    m_innerWheel->add(m_innerSlice[islice]);
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// Place Slices in Outer Wheel, if needed
//
void LArGeo::EMECAccordionConstruction::placeOuterSlices(bool makeSlices)
{
  if (!makeSlices) return;
  for (int islice = 0; islice < s_outerNoBlades; ++islice)
  {
    double x = m_outerSliceOffset[islice].x();
    double y = m_outerSliceOffset[islice].y();
    double z = m_outerSliceOffset[islice].z();
    m_outerWheel->add(new GeoTransform(GeoTrf::Translate3D(x, y, z)));
    m_outerWheel->add(m_outerSlice[islice]);
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// Place Inner Wheel Electrodes and Absorbers
//
void
LArGeo::EMECAccordionConstruction::placeInnerAccordion(int innerNoSectors,
                                                       bool makeSlices,
                                                       bool makeSectors)
{
  int nphi = (makeSectors) ? m_innerNoElectrodes/innerNoSectors : m_innerNoElectrodes;
  double dphi = SI::twopi/m_innerNoElectrodes;
  GeoPhysVol* mother = nullptr;

  // Place Electordes
  for (int iphi = 0; iphi < nphi; ++iphi)
  {
    GeoTrf::RotateZ3D rotation(dphi*iphi - SI::halfpi);
    int copyNo = iphi + 1;
    for (int iblade = 0; iblade < s_innerNoBlades; ++iblade)
    {
      if (makeSlices)  mother = m_innerSlice[iblade];
      if (makeSectors) mother = m_innerSector[iblade];
      if (makeSlices || makeSectors)
      {
        double x = m_innerElectrodeOffset[iblade].x();
        double y = m_innerElectrodeOffset[iblade].y();
        double z = 0;
        GeoTrf::Translate3D translation(x, y, z);
        mother->add(new GeoIdentifierTag(copyNo));
        mother->add(new GeoTransform(rotation*translation));
        mother->add(m_innerElectrode[iblade]);
      }
      else
      {
        double x = m_innerElectrodeOffset[iblade].x();
        double y = m_innerElectrodeOffset[iblade].y();
        double z = m_innerElectrodeOffset[iblade].z();
        GeoTrf::Translate3D translation(x, y, z);
        m_innerWheel->add(new GeoIdentifierTag(copyNo));
        m_innerWheel->add(new GeoTransform(rotation*translation));
        m_innerWheel->add(m_innerElectrode[iblade]);
      }
    }
  }

  // Place Absorbers
  for (int iphi = 0; iphi < nphi; ++iphi)
  {
    GeoTrf::RotateZ3D rotation(dphi*(iphi+0.5) - SI::halfpi);
    int copyNo = iphi + 1;
    for (int iblade = 0; iblade < s_innerNoBlades; ++iblade)
    {
      if (makeSlices)  mother = m_innerSlice[iblade];
      if (makeSectors) mother = m_innerSector[iblade];
      if (makeSlices || makeSectors)
      {
        double x = m_innerAbsorberOffset[iblade].x();
        double y = m_innerAbsorberOffset[iblade].y();
        double z = 0;
        GeoTrf::Translate3D translation(x, y, z);
        mother->add(new GeoIdentifierTag(copyNo));
        mother->add(new GeoTransform(rotation*translation));
        mother->add(m_innerAbsorber[iblade]);
      }
      else
      {
        double x = m_innerAbsorberOffset[iblade].x();
        double y = m_innerAbsorberOffset[iblade].y();
        double z = m_innerAbsorberOffset[iblade].z();
        GeoTrf::Translate3D translation(x, y, z);
        m_innerWheel->add(new GeoIdentifierTag(copyNo));
        m_innerWheel->add(new GeoTransform(rotation*translation));
        m_innerWheel->add(m_innerAbsorber[iblade]);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// Place Outer Wheel Electrodes and Absorbers
//
void
LArGeo::EMECAccordionConstruction::placeOuterAccordion(int outerNoSectors,
                                                       bool makeSlices,
                                                       bool makeSectors)
{
  int nphi = (makeSectors) ? m_outerNoElectrodes/outerNoSectors : m_outerNoElectrodes;
  double dphi = SI::twopi/m_outerNoElectrodes;
  GeoPhysVol* mother = nullptr;

  // Place Electordes
  for (int iphi = 0; iphi < nphi; ++iphi)
  {
    GeoTrf::RotateZ3D rotation(dphi*iphi - SI::halfpi);
    int copyNo = iphi + 1;
    for (int iblade = 0; iblade < s_outerNoBlades; ++iblade)
    {
      if (makeSlices)  mother = m_outerSlice[iblade];
      if (makeSectors) mother = m_outerSector[iblade];
      if (makeSlices || makeSectors)
      {
        double x = m_outerElectrodeOffset[iblade].x();
        double y = m_outerElectrodeOffset[iblade].y();
        double z = 0;
        GeoTrf::Translate3D translation(x, y, z);
        mother->add(new GeoIdentifierTag(copyNo));
        mother->add(new GeoTransform(rotation*translation));
        mother->add(m_outerElectrode[iblade]);
      }
      else
      {
        double x = m_outerElectrodeOffset[iblade].x();
        double y = m_outerElectrodeOffset[iblade].y();
        double z = m_outerElectrodeOffset[iblade].z();
        GeoTrf::Translate3D translation(x, y, z);
        m_outerWheel->add(new GeoIdentifierTag(copyNo));
        m_outerWheel->add(new GeoTransform(rotation*translation));
        m_outerWheel->add(m_outerElectrode[iblade]);
      }
    }
  }

  // Place Absorbers
  for (int iphi = 0; iphi < nphi; ++iphi)
  {
    GeoTrf::RotateZ3D rotation(dphi*(iphi+0.5) - SI::halfpi);
    int copyNo = iphi + 1;
    for (int iblade = 0; iblade < s_outerNoBlades; ++iblade)
    {
      if (makeSlices)  mother = m_outerSlice[iblade];
      if (makeSectors) mother = m_outerSector[iblade];
      if (makeSlices || makeSectors)
      {
        double x = m_outerAbsorberOffset[iblade].x();
        double y = m_outerAbsorberOffset[iblade].y();
        double z = 0;
        GeoTrf::Translate3D translation(x, y, z);
        mother->add(new GeoIdentifierTag(copyNo));
        mother->add(new GeoTransform(rotation*translation));
        mother->add(m_outerAbsorber[iblade]);
      }
      else
      {
        double x = m_outerAbsorberOffset[iblade].x();
        double y = m_outerAbsorberOffset[iblade].y();
        double z = m_outerAbsorberOffset[iblade].z();
        GeoTrf::Translate3D translation(x, y, z);
        m_outerWheel->add(new GeoIdentifierTag(copyNo));
        m_outerWheel->add(new GeoTransform(rotation*translation));
        m_outerWheel->add(m_outerAbsorber[iblade]);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// Construct EMEC Inner Wheel Accordion
//
void
LArGeo::EMECAccordionConstruction::constructInnerWheelStructure(bool makeSlices)
{
  bool makeSectors = false;
  int innerNoSectors = 0;

  //   P R E L I M I N A R Y   C A L C U L A T I O N S
  //
  setInnerWheelSlices(); // define thickness of slices in Inner Wheel

  // Get enginneering data of absorbers
  double innerUncutBladeWidthMin, innerUncutBladeWidthMax;
  double innerLipLength1, innerLipPosition1;
  double innerLipLength2, innerLipPosition2;
  getInnerAbsorberData(innerUncutBladeWidthMin, innerUncutBladeWidthMax,
                       innerLipLength1, innerLipPosition1,
                       innerLipLength2, innerLipPosition2);
  innerUncutBladeWidthMin *= m_kContraction;
  innerUncutBladeWidthMax *= m_kContraction;
  innerLipLength1         *= m_kContraction;
  innerLipPosition1       *= m_kContraction;
  innerLipLength2         *= m_kContraction;
  innerLipPosition2       *= m_kContraction;

  //   C O M P U T E   U N C U T   B L A D E   C O R N E R S
  //
  double wmin, wmax, thickness, rmin, rmax, zdel;

  // Compute Inner Absorber Blade corners
  GeoThreeVector innerCorners[8];
  wmin = innerUncutBladeWidthMin;
  wmax = innerUncutBladeWidthMax;
  thickness = m_innerAbsorberThickness;
  rmin = m_innerWheelRmin[1];
  rmax = m_innerWheelRmax[s_innerNoBlades - 1];
  zdel = m_innerHalfWaveWidth; // slice width
  getBladeCorners(wmin, wmax, thickness, rmin, rmax, zdel, innerCorners);

  // Compute Inner Electrode Blade corners
  GeoThreeVector innerElectrodeCorners[8];
  thickness = m_innerElectrodeThickness;
  getBladeCorners(wmin, wmax, thickness, rmin, rmax, zdel, innerElectrodeCorners);

  //   C O N S T R U C T   L O G I C A L   V O L U M E S
  //
  // Compute X-scale factors for glue and lead in absorbers
  // Needed for construction of corresponding volumes
  m_innerGlueRatio = (m_innerLeadThickness + 2*m_innerGlueThickness)/m_innerAbsorberThickness;
  m_innerLeadRatio = m_innerLeadThickness/m_innerAbsorberThickness;

  // Construct Lips, set offsets for the case "no Slices, no Sectors"
  constructInnerLips(innerLipLength1, innerLipPosition1, innerLipLength2, innerLipPosition2);

  // Construct Blades, set offsets for the case "no Slices, no Sectors"
  constructInnerBlades(innerCorners, innerElectrodeCorners);

  // Construct Slices, set offsets
  constructInnerSlices();

  // Construct Sectors (disabled)
  // constructInnerSectors(innerNoSectors);

  //   C O N S T R U C T   P H Y S I C A L   V O L U M E S
  //
  placeInnerGlueAndLead();      // place Glue and Lead in Inner Wheel Absorbers
  placeInnerSlices(makeSlices); // place Slices, if needed
  // PlaceInnerSectors(innerNoSectors, makeSlices, makeSectors); // disabled
  placeInnerAccordion(innerNoSectors, makeSlices, makeSectors); // place Absorbers and Electrodes
}

////////////////////////////////////////////////////////////////////////////////
//
// Construct EMEC Inner Wheel Accordion
//
void
LArGeo::EMECAccordionConstruction::constructOuterWheelStructure(bool makeSlices)
{
  bool makeSectors = false;
  int outerNoSectors = 0;

  //   P R E L I M I N A R Y   C A L C U L A T I O N S
  //
  setOuterWheelSlices(); // define thickness of slices in Outer Wheel

  // Get enginneering data of absorbers
  double outerUncutBladeWidthMin, outerUncutBladeWidthMax;
  double outerLipLength1, outerLipPosition1;
  double outerLipLength2, outerLipPosition2;
  getOuterAbsorberData(outerUncutBladeWidthMin, outerUncutBladeWidthMax,
                       outerLipLength1, outerLipPosition1,
                       outerLipLength2, outerLipPosition2);
  outerUncutBladeWidthMin *= m_kContraction;
  outerUncutBladeWidthMax *= m_kContraction;
  outerLipLength1         *= m_kContraction;
  outerLipPosition1       *= m_kContraction;
  outerLipLength2         *= m_kContraction;
  outerLipPosition2       *= m_kContraction;

  //   C O M P U T E   U N C U T   B L A D E   C O R N E R S
  //
  double wmin, wmax, thickness, rmin, rmax, zdel;

  // Compute Outer Absorber Blade corners
  GeoThreeVector outerCorners[8];
  wmin = outerUncutBladeWidthMin;
  wmax = outerUncutBladeWidthMax;
  thickness = m_outerAbsorberThickness;
  rmin = m_outerWheelRmin[1];
  rmax = m_outerWheelRmax[s_outerNoBlades - 1];
  zdel = m_outerHalfWaveWidth; // slice width
  getBladeCorners(wmin, wmax, thickness, rmin, rmax, zdel, outerCorners);

  // Compute Outer Electrode Blade corners
  GeoThreeVector outerElectrodeCorners[8];
  thickness = m_outerElectrodeThickness;
  getBladeCorners(wmin, wmax, thickness, rmin, rmax, zdel, outerElectrodeCorners);

  //   C O N S T R U C T   L O G I C A L   V O L U M E S
  //
  // Compute X-scale factors for glue and lead in absorbers
  // Needed for construction of corresponding volumes
  m_outerGlueRatio = (m_outerLeadThickness + 2*m_outerGlueThickness)/m_outerAbsorberThickness;
  m_outerLeadRatio = m_outerLeadThickness/m_outerAbsorberThickness;

  // Construct Lips, set offsets for the case "no Slices, no Sectors"
  constructOuterLips(outerLipLength1, outerLipPosition1, outerLipLength2, outerLipPosition2);

  // Construct Blades, set offsets for the case "no Slices, no Sectors"
  constructOuterBlades(outerCorners, outerElectrodeCorners);

  // Construct Slices, set offsets
  constructOuterSlices();

  // Construct Sectors (disabled)
  // constructOuterSectors(outerNoSectors);

  //   C O N S T R U C T   P H Y S I C A L   V O L U M E S
  //
  placeOuterGlueAndLead();      // place Glue and Lead in Outer Wheel Absorbers
  placeOuterSlices(makeSlices); // place Slices, if needed
  // PlaceOuterSectors(outerNoSectors, makeSlices, makeSectors); // disabled
  placeOuterAccordion(outerNoSectors, makeSlices, makeSectors); // place Absorbers and Electrodes
}

