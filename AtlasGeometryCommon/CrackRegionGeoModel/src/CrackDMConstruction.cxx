/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "CrackRegionGeoModel/CrackDMConstruction.h"

#include "GeoModelKernel/GeoElement.h"
#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoVPhysVol.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelKernel/GeoAlignableTransform.h"
#include "GeoModelKernel/GeoIdentifierTag.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoSerialTransformer.h"
#include "GeoModelKernel/GeoXF.h"
#include "GeoModelKernel/GeoTube.h"
#include "GeoModelKernel/GeoPcon.h"
#include "GeoModelKernel/GeoTubs.h"
#include "GeoModelKernel/GeoCons.h"
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoTrap.h"
#include "GeoModelKernel/GeoTrd.h"
#include "GeoModelKernel/GeoShape.h"
#include "GeoModelKernel/GeoShapeUnion.h"
#include "GeoModelKernel/GeoShapeShift.h"
#include "GeoModelKernel/GeoShapeSubtraction.h"
#include "GeoModelKernel/Units.h"

#include "StoreGate/StoreGateSvc.h"
#include "GeoModelInterfaces/StoredMaterialManager.h"
#include "GeoModelInterfaces/IGeoModelSvc.h"
#include "GeoModelUtilities/DecodeVersionKey.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "RDBAccessSvc/IRDBRecord.h"

// For units:
#include "GaudiKernel/PhysicalConstants.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Bootstrap.h"

// For functions:
#include "GeoGenericFunctions/Variable.h"

using namespace GeoGenfun;
using namespace GeoXF;

static const unsigned int NCrates=16;
static const double Alfa=360*Gaudi::Units::deg/NCrates;
static const double Enda=1155;
static const double Endb=1695.2;
static const double Endc=2771.6;
static const double DYa=1155*tan(Alfa/2);
static const double DYb=1695.2*tan(Alfa/2);
static const double DYc=2771.6*tan(Alfa/2);


void
createSectorEnvelopes2FromDB (GeoFullPhysVol* envelope,
                              StoredMaterialManager* materialManager,
                              std::map<std::string, unsigned int>& trdMap,
                              IRDBRecordset& BarrelDMTrds,
                              std::map<std::string, unsigned int>& trapMap,
                              IRDBRecordset& BarrelDMTraps,
                              std::map<std::string, unsigned int>& boxMap,
                              IRDBRecordset& BarrelDMBoxes,
                              GENFUNCTION& f,
                              GeoBox* Box)
{
  unsigned int recordIndex = trdMap["SecE2"];
  double SecE2xhlen1 = BarrelDMTrds[recordIndex]->getDouble("XHLEN1");
  double SecE2xhlen2 = BarrelDMTrds[recordIndex]->getDouble("XHLEN2");
  double SecE2ztr = BarrelDMTrds[recordIndex]->getDouble("ZTR");

  recordIndex = trdMap["Spb0"];
  double Spb0xhlen1 = BarrelDMTrds[recordIndex]->getDouble("XHLEN1");
  double Spb0xhlen2 = BarrelDMTrds[recordIndex]->getDouble("XHLEN2");
  double Spb0yhlen1 = BarrelDMTrds[recordIndex]->getDouble("YHLEN1");
  double Spb0yhlen2 = BarrelDMTrds[recordIndex]->getDouble("YHLEN2");
  double Spb0zhlen = BarrelDMTrds[recordIndex]->getDouble("ZHLEN");
  double Spb0ytr = BarrelDMTrds[recordIndex]->getDouble("YTR");
  double Spb0xrot = BarrelDMTrds[recordIndex]->getDouble("XROT");

  recordIndex = boxMap["Box"];
  //double Boxhlen = BarrelDMBoxes[recordIndex]->getDouble("HLEN");
  //double Boxhwdt = BarrelDMBoxes[recordIndex]->getDouble("HWDT");
  //double Boxhhgt = BarrelDMBoxes[recordIndex]->getDouble("HHGT");
  double Boxxtr = BarrelDMBoxes[recordIndex]->getDouble("XTR");
  double Boxytr = BarrelDMBoxes[recordIndex]->getDouble("YTR");
  double Boxztr = BarrelDMBoxes[recordIndex]->getDouble("ZTR");
  //double Boxxrot = BarrelDMBoxes[recordIndex]->getDouble("XROT");

  recordIndex = boxMap["SplBox"];
  double SplBoxhlen = BarrelDMBoxes[recordIndex]->getDouble("HLEN");
  double SplBoxhwdt = BarrelDMBoxes[recordIndex]->getDouble("HWDT");
  double SplBoxhhgt = BarrelDMBoxes[recordIndex]->getDouble("HHGT");
  double SplBoxytr = BarrelDMBoxes[recordIndex]->getDouble("YTR");
  double SplBoxztr = BarrelDMBoxes[recordIndex]->getDouble("ZTR");
  double SplBoxxrot = BarrelDMBoxes[recordIndex]->getDouble("XROT");

  recordIndex = trapMap["Spb1"];
  double Spb1zhlen = BarrelDMTraps[recordIndex]->getDouble("ZHLEN");
  double Spb1theta = BarrelDMTraps[recordIndex]->getDouble("THETA");
  double Spb1phi = BarrelDMTraps[recordIndex]->getDouble("PHI");
  double Spb1yzn = BarrelDMTraps[recordIndex]->getDouble("YZN");
  double Spb1xynzn = BarrelDMTraps[recordIndex]->getDouble("XYNZN");
  double Spb1xypzn = BarrelDMTraps[recordIndex]->getDouble("XYPZN");
  double Spb1angn = BarrelDMTraps[recordIndex]->getDouble("ANGN");
  double Spb1yzp = BarrelDMTraps[recordIndex]->getDouble("YZP");
  double Spb1xynzp = BarrelDMTraps[recordIndex]->getDouble("XYNZP");
  double Spb1xypzp = BarrelDMTraps[recordIndex]->getDouble("XYPZP");
  double Spb1angp = BarrelDMTraps[recordIndex]->getDouble("ANGP");
  double Spb1xtr = BarrelDMTraps[recordIndex]->getDouble("XTR");
  double Spb1ytr = BarrelDMTraps[recordIndex]->getDouble("YTR");
  double Spb1ztr = BarrelDMTraps[recordIndex]->getDouble("ZTR");
  double Spb1xrot = BarrelDMTraps[recordIndex]->getDouble("XROT");

  recordIndex = trapMap["Spb3"];
  double Spb3zhlen = BarrelDMTraps[recordIndex]->getDouble("ZHLEN");
  double Spb3theta = BarrelDMTraps[recordIndex]->getDouble("THETA");
  double Spb3phi = BarrelDMTraps[recordIndex]->getDouble("PHI");
  double Spb3yzn = BarrelDMTraps[recordIndex]->getDouble("YZN");
  double Spb3xynzn = BarrelDMTraps[recordIndex]->getDouble("XYNZN");
  double Spb3xypzn = BarrelDMTraps[recordIndex]->getDouble("XYPZN");
  double Spb3angn = BarrelDMTraps[recordIndex]->getDouble("ANGN");
  double Spb3yzp = BarrelDMTraps[recordIndex]->getDouble("YZP");
  double Spb3xynzp = BarrelDMTraps[recordIndex]->getDouble("XYNZP");
  double Spb3xypzp = BarrelDMTraps[recordIndex]->getDouble("XYPZP");
  double Spb3angp = BarrelDMTraps[recordIndex]->getDouble("ANGP");
  double Spb3ztr = BarrelDMTraps[recordIndex]->getDouble("ZTR");

  recordIndex = trapMap["Spb2"];
  double Spb2zhlen = BarrelDMTraps[recordIndex]->getDouble("ZHLEN");
  double Spb2theta = BarrelDMTraps[recordIndex]->getDouble("THETA");
  double Spb2phi = BarrelDMTraps[recordIndex]->getDouble("PHI");
  double Spb2yzn = BarrelDMTraps[recordIndex]->getDouble("YZN");
  double Spb2xynzn = BarrelDMTraps[recordIndex]->getDouble("XYNZN");
  double Spb2xypzn = BarrelDMTraps[recordIndex]->getDouble("XYPZN");
  double Spb2angn = BarrelDMTraps[recordIndex]->getDouble("ANGN");
  double Spb2yzp = BarrelDMTraps[recordIndex]->getDouble("YZP");
  double Spb2xynzp = BarrelDMTraps[recordIndex]->getDouble("XYNZP");
  double Spb2xypzp = BarrelDMTraps[recordIndex]->getDouble("XYPZP");
  double Spb2angp = BarrelDMTraps[recordIndex]->getDouble("ANGP");
  double Spb2ytr = BarrelDMTraps[recordIndex]->getDouble("YTR");
  double Spb2ztr = BarrelDMTraps[recordIndex]->getDouble("ZTR");

  const GeoMaterial* matLArServices17  = materialManager->getMaterial("LAr::LArServices17");// 0.035*gram/cm3
  const GeoMaterial* matLArServices18  = materialManager->getMaterial("LAr::LArServices18");// 0.240*gram/cm3
  const GeoMaterial* matLArServices19  = materialManager->getMaterial("LAr::LArServices19");// 0.469*gram/cm3
  const GeoMaterial* matLArServices20  = materialManager->getMaterial("LAr::LArServices20");// 0.353*gram/cm3
  const GeoMaterial *alu               = materialManager->getMaterial("std::Aluminium"); //2.7 g/cm3
  const GeoMaterial *air               = materialManager->getMaterial("std::Air"); //0.001214 g/cm3

  GeoTrf::Transform3D Cut3Boxe  = GeoTrf::Translate3D(Boxxtr, Boxytr, Boxztr)*GeoTrf::RotateX3D(-20*Gaudi::Units::deg)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  GeoTrf::Transform3D Cut4Boxe  = GeoTrf::Translate3D(Boxxtr, -Boxytr,Boxztr)*GeoTrf::RotateX3D(20*Gaudi::Units::deg)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);

  // build 5 instances of SectorEnvelopes1 with 3 different materials!
  GeoTrd   *Trdair2  = new GeoTrd(SecE2xhlen1, SecE2xhlen2, DYb, DYc, (Endc-Endb)/2);
  const GeoShape & SectorEnvelope= ((*Trdair2).
                                    subtract((*Box)  <<GeoTrf::Transform3D(Cut3Boxe)).
                                    subtract((*Box)  <<GeoTrf::Transform3D(Cut4Boxe)));

  const GeoShape & SectorEnvelopes= ((SectorEnvelope).
                                     add(SectorEnvelope  << GeoTrf::TranslateY3D(-(DYb+DYc)*cos(Alfa/2)*cos(Alfa/2))*GeoTrf::TranslateZ3D(-(DYb+DYc)*0.5*sin(Alfa))*GeoTrf::RotateX3D(Alfa)));

  GeoLogVol  *lvse2r          = new GeoLogVol("LAr::DM::SectorEnvelopes2r",&SectorEnvelopes,matLArServices20);
  GeoPhysVol *sectorenvelopes2r    = new GeoPhysVol(lvse2r);  // for right-handed splice boxes

  GeoLogVol  *lvse2l          = new GeoLogVol("LAr::DM::SectorEnvelopes2l",&SectorEnvelopes,matLArServices20);
  GeoPhysVol *sectorenvelopes2l    = new GeoPhysVol(lvse2l);  // for left-handed splice boxes

  GeoLogVol  *lvse2h          = new GeoLogVol("LAr::DM::SectorEnvelopes2h",&SectorEnvelopes,matLArServices19);
  GeoPhysVol *sectorenvelopes2h    = new GeoPhysVol(lvse2h);  // no splice boxes horizontal at 0 & 180 Gaudi::Units::deg.

  GeoLogVol  *lvse2vup          = new GeoLogVol("LAr::DM::SectorEnvelopes2vup",&SectorEnvelopes,matLArServices17);
  GeoPhysVol *sectorenvelopes2vup    = new GeoPhysVol(lvse2vup);  // no splice boxes vertical up at 90 Gaudi::Units::deg

  GeoLogVol  *lvse2vd          = new GeoLogVol("LAr::DM::SectorEnvelopes2Vd",&SectorEnvelopes,matLArServices18);
  GeoPhysVol *sectorenvelopes2vd    = new GeoPhysVol(lvse2vd);  // no splice boxes vertical down at 270 Gaudi::Units::deg

  //---------- Build Splice boxes for InDet optical fibers--------

  GeoTrap  *GeoTrap1  = new GeoTrap(Spb1zhlen, Spb1theta, Spb1phi, Spb1yzn, Spb1xynzn, Spb1xypzn, Spb1angn, Spb1yzp, Spb1xynzp, Spb1xypzp, Spb1angp);
  GeoBox   *Box1   = new GeoBox(SplBoxhlen, SplBoxhwdt, SplBoxhhgt);
  const GeoShape & SpliceBox = ((*GeoTrap1).
                                subtract(*Box1 << GeoTrf::TranslateZ3D(SplBoxztr)*GeoTrf::TranslateY3D(-SplBoxytr)*GeoTrf::RotateX3D(SplBoxxrot*Gaudi::Units::deg)));

  GeoTransform *xtr = new GeoTransform (GeoTrf::TranslateZ3D(Spb1ztr)*GeoTrf::TranslateY3D(-Spb1ytr)*GeoTrf::TranslateX3D(Spb1xtr)*GeoTrf::RotateX3D(Spb1xrot*Gaudi::Units::deg));
  sectorenvelopes2r->add(xtr);
  GeoLogVol  *lvspbr     = new GeoLogVol("LAr::DM::SPliceBoxr",&SpliceBox,alu);
  GeoPhysVol *spliceboxr       = new GeoPhysVol(lvspbr);
  sectorenvelopes2r->add(spliceboxr);

  GeoTransform *xtl = new GeoTransform (GeoTrf::TranslateZ3D(Spb1ztr)*GeoTrf::TranslateY3D(-Spb1ytr)*GeoTrf::TranslateX3D(Spb1xtr)*GeoTrf::RotateY3D(-180*Gaudi::Units::deg)*GeoTrf::RotateX3D(-(Alfa/2)));
  sectorenvelopes2l->add(xtl);
  GeoLogVol  *lvspbl     = new GeoLogVol("LAr::DM::SpliceBoxl",&SpliceBox,alu);
  GeoPhysVol *spliceboxl       = new GeoPhysVol(lvspbl);
  sectorenvelopes2l->add(spliceboxl);

  ////
  GeoTrd   *Trd1  = new GeoTrd(Spb0xhlen1, Spb0xhlen2, Spb0yhlen1, Spb0yhlen2, Spb0zhlen);
  GeoTrap  *GeoTrap2  = new GeoTrap(Spb2zhlen, Spb2theta, Spb2phi, Spb2yzn, Spb2xynzn, Spb2xypzn, Spb2angn, Spb2yzp, Spb2xynzp, Spb2xypzp, Spb2angp);
  GeoTrap  *GeoTrap3  = new GeoTrap(Spb3zhlen, Spb3theta, Spb3phi, Spb3yzn, Spb3xynzn, Spb3xypzn, Spb3angn, Spb3yzp, Spb3xynzp, Spb3xypzp, Spb3angp);

  GeoTransform *xt1 = new GeoTransform (GeoTrf::TranslateY3D(-Spb0ytr)*GeoTrf::RotateX3D(Spb0xrot*Gaudi::Units::deg));
  spliceboxr->add(xt1);
  spliceboxl->add(xt1);
  GeoLogVol  *lt1     = new GeoLogVol("LAr::DM::TBox1",Trd1,air);
  GeoPhysVol *tbox1       = new GeoPhysVol(lt1);
  spliceboxr->add(tbox1);
  spliceboxl->add(tbox1);

  GeoTransform *xt2 = new GeoTransform (GeoTrf::TranslateZ3D(Spb2ztr)*GeoTrf::TranslateY3D(Spb2ytr));
  spliceboxr->add(xt2);
  spliceboxl->add(xt2);
  GeoLogVol  *lt2     = new GeoLogVol("LAr::DM::TBox2",GeoTrap2,air);
  GeoPhysVol *tbox2       = new GeoPhysVol(lt2);
  spliceboxr->add(tbox2);
  spliceboxl->add(tbox2);

  GeoTransform *xt3 = new GeoTransform (GeoTrf::TranslateZ3D(-Spb3ztr));
  spliceboxr->add(xt3);
  spliceboxl->add(xt3);
  GeoLogVol  *lt3     = new GeoLogVol("LAr::DM::TBox3",GeoTrap3,air);
  GeoPhysVol *tbox3       = new GeoPhysVol(lt3);
  spliceboxr->add(tbox3);
  spliceboxl->add(tbox3);

  //-------------- Place volumes in LAr Envelope -------------------

  TRANSFUNCTION seA2r = Pow(GeoTrf::RotateZ3D(1.0),8*f-(3*Alfa/2))*GeoTrf::TranslateX3D((Endb+Endc)/2)*GeoTrf::TranslateZ3D(SecE2ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seA2l = Pow(GeoTrf::RotateZ3D(1.0),8*f+(5*Alfa/2))*GeoTrf::TranslateX3D((Endb+Endc)/2)*GeoTrf::TranslateZ3D(SecE2ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seC2r = Pow(GeoTrf::RotateZ3D(1.0),8*f-(3*Alfa/2))*GeoTrf::TranslateX3D((Endb+Endc)/2)*GeoTrf::TranslateZ3D(-SecE2ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seC2l = Pow(GeoTrf::RotateZ3D(1.0),8*f+(5*Alfa/2))*GeoTrf::TranslateX3D((Endb+Endc)/2)*GeoTrf::TranslateZ3D(-SecE2ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seA2Vup = Pow(GeoTrf::RotateZ3D(1.0),f+(9*Alfa/2))*GeoTrf::TranslateX3D((Endb+Endc)/2)*GeoTrf::TranslateZ3D(SecE2ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seA2Vd = Pow(GeoTrf::RotateZ3D(1.0),f-(7*Alfa/2))*GeoTrf::TranslateX3D((Endb+Endc)/2)*GeoTrf::TranslateZ3D(SecE2ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seA2H = Pow(GeoTrf::RotateZ3D(1.0),8*f+(Alfa/2))*GeoTrf::TranslateX3D((Endb+Endc)/2)*GeoTrf::TranslateZ3D(SecE2ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seC2Vup = Pow(GeoTrf::RotateZ3D(1.0),f+(9*Alfa/2))*GeoTrf::TranslateX3D((Endb+Endc)/2)*GeoTrf::TranslateZ3D(-SecE2ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seC2Vd = Pow(GeoTrf::RotateZ3D(1.0),f-(7*Alfa/2))*GeoTrf::TranslateX3D((Endb+Endc)/2)*GeoTrf::TranslateZ3D(-SecE2ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seC2H = Pow(GeoTrf::RotateZ3D(1.0),8*f+(Alfa/2))*GeoTrf::TranslateX3D((Endb+Endc)/2)*GeoTrf::TranslateZ3D(-SecE2ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);

  GeoSerialTransformer *setA2r = new GeoSerialTransformer(sectorenvelopes2r,&seA2r, 2);
  GeoSerialTransformer *setA2l = new GeoSerialTransformer(sectorenvelopes2l,&seA2l, 2);
  GeoSerialTransformer *setC2r = new GeoSerialTransformer(sectorenvelopes2r,&seC2r, 2);
  GeoSerialTransformer *setC2l = new GeoSerialTransformer(sectorenvelopes2l,&seC2l, 2);
  GeoSerialTransformer *setA2Vup = new GeoSerialTransformer(sectorenvelopes2vup,&seA2Vup, 1);
  GeoSerialTransformer *setA2Vd = new GeoSerialTransformer(sectorenvelopes2vd,&seA2Vd, 1);
  GeoSerialTransformer *setA2H = new GeoSerialTransformer(sectorenvelopes2h,&seA2H, 2);
  GeoSerialTransformer *setC2Vup = new GeoSerialTransformer(sectorenvelopes2vup,&seC2Vup, 1);
  GeoSerialTransformer *setC2Vd = new GeoSerialTransformer(sectorenvelopes2vd,&seC2Vd, 1);
  GeoSerialTransformer *setC2H = new GeoSerialTransformer(sectorenvelopes2h,&seC2H, 2);

  envelope->add(setA2r);
  envelope->add(setA2l);
  envelope->add(setC2r);
  envelope->add(setC2l);
  envelope->add(setA2Vup);
  envelope->add(setA2Vd);
  envelope->add(setA2H);
  envelope->add(setC2Vup);
  envelope->add(setC2Vd);
  envelope->add(setC2H);
}


void
createBridgeEnvelopesFromDB (GeoFullPhysVol* envelope,
                             std::map<std::string, unsigned int>& trapMap,
                             IRDBRecordset& BarrelDMTraps,
                             const GeoMaterial* matLArServices8,
                             GENFUNCTION& f)
{
  unsigned int recordIndex = trapMap["BridgeE"];
  const IRDBRecord* r = BarrelDMTraps[recordIndex];
  double BridgeEzhlen = r->getDouble("ZHLEN");
  double BridgeEtheta = r->getDouble("THETA");
  double BridgeEphi = r->getDouble("PHI");
  double BridgeEyzn = r->getDouble("YZN");
  double BridgeExynzn = r->getDouble("XYNZN");
  double BridgeExypzn = r->getDouble("XYPZN");
  double BridgeEangn = r->getDouble("ANGN");
  double BridgeEyzp = r->getDouble("YZP");
  double BridgeExynzp = r->getDouble("XYNZP");
  double BridgeExypzp = r->getDouble("XYPZP");
  double BridgeEangp = r->getDouble("ANGP");
  double BridgeExtr = r->getDouble("XTR");
  double BridgeEztr = r->getDouble("ZTR");

  GeoTrap  *Trapair  = new GeoTrap(BridgeEzhlen, BridgeEtheta*Gaudi::Units::deg, BridgeEphi, BridgeEyzn, BridgeExynzn, BridgeExypzn, BridgeEangn, BridgeEyzp, BridgeExynzp, BridgeExypzp, BridgeEangp);
  GeoLogVol  *lvbre        = new GeoLogVol("LAr::DM::BridgeEnvelopes",Trapair,matLArServices8);//In the end Density at least >= than SE1 because of Cryo Pipes
  GeoPhysVol *bridgeenvelopes    = new GeoPhysVol(lvbre);

  TRANSFUNCTION breA = Pow(GeoTrf::RotateZ3D(1.0),f-(Alfa/2))*GeoTrf::TranslateX3D(BridgeExtr)*GeoTrf::TranslateZ3D(BridgeEztr)*GeoTrf::RotateZ3D(90*Gaudi::Units::deg)*GeoTrf::RotateY3D(90*Gaudi::Units::deg)*GeoTrf::RotateX3D(90*Gaudi::Units::deg);
  TRANSFUNCTION breC = Pow(GeoTrf::RotateZ3D(1.0),f-(Alfa/2))*GeoTrf::TranslateX3D(BridgeExtr)*GeoTrf::TranslateZ3D(-BridgeEztr)*GeoTrf::RotateZ3D(-90*Gaudi::Units::deg)*GeoTrf::RotateY3D(-90*Gaudi::Units::deg)*GeoTrf::RotateX3D(-90*Gaudi::Units::deg);
  GeoSerialTransformer *bretA = new GeoSerialTransformer(bridgeenvelopes,&breA, NCrates);
  GeoSerialTransformer *bretC = new GeoSerialTransformer(bridgeenvelopes,&breC, NCrates);
  envelope->add(bretA);
  envelope->add(bretC);
}


void
createBaseEnvelopesFromDB (GeoFullPhysVol* envelope,
                           std::map<std::string, unsigned int>& trdMap,
                           IRDBRecordset& BarrelDMTrds,
                           const GeoMaterial* matLArServices8,
                           GENFUNCTION& f)
{
  unsigned int recordIndex = trdMap["BaseE"];
  const IRDBRecord* r = BarrelDMTrds[recordIndex];
  double BaseExhlen1 = r->getDouble("XHLEN1");
  double BaseExhlen2 = r->getDouble("XHLEN2");
  double BaseEyhlen1 = r->getDouble("YHLEN1");
  double BaseEyhlen2 = r->getDouble("YHLEN2");
  double BaseEzhlen = r->getDouble("ZHLEN");//
  double BaseExtr = r->getDouble("XTR");
  double BaseEztr = r->getDouble("ZTR");

  GeoTrd   *Trd1air  = new GeoTrd(BaseExhlen1, BaseExhlen2, BaseEyhlen1, BaseEyhlen2, BaseEzhlen);
  GeoLogVol  *lvbe          = new GeoLogVol("LAr::DM::BaseEnvelopes",Trd1air,matLArServices8); //In the end Density at least >= than SE1 because of Cryo Pipes
  GeoPhysVol *baseenvelopes    = new GeoPhysVol(lvbe);

  TRANSFUNCTION beA = Pow(GeoTrf::RotateZ3D(1.0),f-(Alfa/2))*GeoTrf::TranslateX3D(BaseExtr)*GeoTrf::TranslateZ3D(BaseEztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION beC = Pow(GeoTrf::RotateZ3D(1.0),f+(Alfa/2))*GeoTrf::TranslateX3D(BaseExtr)*GeoTrf::TranslateZ3D(-BaseEztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  GeoSerialTransformer *betA = new GeoSerialTransformer(baseenvelopes,&beA, NCrates);
  GeoSerialTransformer *betC = new GeoSerialTransformer(baseenvelopes,&beC, NCrates);
  envelope->add(betA);
  envelope->add(betC);
}

void createFromDB (GeoFullPhysVol* envelope,
                   IRDBAccessSvc* rdbAccess,
                   IGeoModelSvc* geoModel,
                   StoredMaterialManager* materialManager)
{
  // Use Geometry Database
  DecodeVersionKey keyLAr(geoModel,"LAr");
  IRDBRecordset_ptr BarrelDMTraps = rdbAccess->getRecordsetPtr("BarrelDMTraps",keyLAr.tag(),keyLAr.node());
  IRDBRecordset_ptr BarrelDMTrds = rdbAccess->getRecordsetPtr("BarrelDMTrds",keyLAr.tag(),keyLAr.node());
  IRDBRecordset_ptr BarrelDMTubes = rdbAccess->getRecordsetPtr("BarrelDMTubes",keyLAr.tag(),keyLAr.node());
  IRDBRecordset_ptr BarrelDMBoxes = rdbAccess->getRecordsetPtr("BarrelDMBoxes",keyLAr.tag(),keyLAr.node());

  std::map<std::string, unsigned int> tubeMap;
  for (unsigned int i=0; i<BarrelDMTubes->size(); i++)
  {
    const std::string& key = (*BarrelDMTubes)[i]->getString("TUBENAME");
    tubeMap[key] = i;
  }
  std::map<std::string, unsigned int> boxMap;
  for (unsigned int j=0; j<BarrelDMBoxes->size(); j++)
  {
    const std::string& key = (*BarrelDMBoxes)[j]->getString("BOXNAME");
    boxMap[key] = j;
  }
  std::map<std::string, unsigned int> trdMap;
  for (unsigned int k=0; k<BarrelDMTrds->size(); k++)
  {
    const std::string& key = (*BarrelDMTrds)[k]->getString("TRDNAME");
    trdMap[key] = k;
  }
  std::map<std::string, unsigned int> trapMap;
  for (unsigned int l=0; l<BarrelDMTraps->size(); l++)
  {
    const std::string& key = (*BarrelDMTraps)[l]->getString("TRAPNAME");
    trapMap[key] = l;
  }

  unsigned int recordIndex;

  // Get materials
  const GeoMaterial *alu               = materialManager->getMaterial("std::Aluminium"); //2.7 g/cm3
  const GeoMaterial* matBoardsEnvelope = materialManager->getMaterial("LAr::BoardsEnvelope");// 0.932*gram/cm3);
  const GeoMaterial* matLArServices1   = materialManager->getMaterial("LAr::LArServices1");// 1.020*gram/cm3
  const GeoMaterial* matLArServices2   = materialManager->getMaterial("LAr::LArServices2");// 0.955*gram/cm3
  const GeoMaterial* matLArServices3   = materialManager->getMaterial("LAr::LArServices3");// 1.005*gram/cm3
  const GeoMaterial* matLArServices4   = materialManager->getMaterial("LAr::LArServices4");// 0.460*gram/cm3
  const GeoMaterial* matLArServices5   = materialManager->getMaterial("LAr::LArServices5");// 0.480*gram/cm3
  const GeoMaterial* matLArServices6   = materialManager->getMaterial("LAr::LArServices6");// 1.000*gram/cm3
  const GeoMaterial* matLArServices7   = materialManager->getMaterial("LAr::LArServices7");// 0.935*gram/cm3
  const GeoMaterial* matLArServices8   = materialManager->getMaterial("LAr::LArServices8");// 1.070*gram/cm3
  const GeoMaterial* matLArServices9   = materialManager->getMaterial("LAr::LArServices9");// 1.020*gram/cm3
  const GeoMaterial* matLArServices10  = materialManager->getMaterial("LAr::LArServices10");// 0.995*gram/cm3
  const GeoMaterial* matLArServices11  = materialManager->getMaterial("LAr::LArServices11");// 0.835*gram/cm3
  const GeoMaterial* matLArServices12  = materialManager->getMaterial("LAr::LArServices12");// 0.640*gram/cm3
  const GeoMaterial* matLArServices13  = materialManager->getMaterial("LAr::LArServices13");// 0.690*gram/cm3
  const GeoMaterial* matLArServices14  = materialManager->getMaterial("LAr::LArServices14");// 0.825*gram/cm3
  const GeoMaterial* matLArServices15  = materialManager->getMaterial("LAr::LArServices15");// 0.875*gram/cm3
  const GeoMaterial* matLArServices16  = materialManager->getMaterial("LAr::LArServices16");// 1.035*gram/cm3

  const double inv_Endab = 1. / (Endb - Enda);
  Variable       i;
  GENFUNCTION    f = Alfa*i;

  ////----------- Building Front-end crates --------------------

  recordIndex = tubeMap["Ped2"];
  double ped2zhlen = (*BarrelDMTubes)[recordIndex]->getDouble("ZHLEN");
  double ped2minr = (*BarrelDMTubes)[recordIndex]->getDouble("MINR");
  double ped2maxr = (*BarrelDMTubes)[recordIndex]->getDouble("MAXR");
  double ped2ytr = (*BarrelDMTubes)[recordIndex]->getDouble("YTR");
  recordIndex = tubeMap["Ped3"];
  double ped3zhlen = (*BarrelDMTubes)[recordIndex]->getDouble("ZHLEN");
  double ped3minr = (*BarrelDMTubes)[recordIndex]->getDouble("MINR");
  double ped3maxr = (*BarrelDMTubes)[recordIndex]->getDouble("MAXR");
  double ped3xtr = (*BarrelDMTubes)[recordIndex]->getDouble("XTR");

  recordIndex = boxMap["Pedest"];
  double pedesthlen = (*BarrelDMBoxes)[recordIndex]->getDouble("HLEN");
  double pedesthwdt = (*BarrelDMBoxes)[recordIndex]->getDouble("HWDT");
  double pedesthhgt = (*BarrelDMBoxes)[recordIndex]->getDouble("HHGT");
  double pedestxtr = (*BarrelDMBoxes)[recordIndex]->getDouble("XTR");
  double pedestztr = (*BarrelDMBoxes)[recordIndex]->getDouble("ZTR");
  recordIndex = boxMap["Ped1"];
  double ped1hlen = (*BarrelDMBoxes)[recordIndex]->getDouble("HLEN");
  double ped1hwdt = (*BarrelDMBoxes)[recordIndex]->getDouble("HWDT");
  double ped1hhgt = (*BarrelDMBoxes)[recordIndex]->getDouble("HHGT");
  recordIndex = boxMap["Crate1"];
  double crate1hlen = (*BarrelDMBoxes)[recordIndex]->getDouble("HLEN");
  double crate1hwdt = (*BarrelDMBoxes)[recordIndex]->getDouble("HWDT");
  double crate1hhgt = (*BarrelDMBoxes)[recordIndex]->getDouble("HHGT");
  double crate1xtr = (*BarrelDMBoxes)[recordIndex]->getDouble("XTR");
  double crate1ztr = (*BarrelDMBoxes)[recordIndex]->getDouble("ZTR");
  recordIndex = boxMap["Crate2"];
  double crate2hlen = (*BarrelDMBoxes)[recordIndex]->getDouble("HLEN");
  double crate2hwdt = (*BarrelDMBoxes)[recordIndex]->getDouble("HWDT");
  double crate2hhgt = (*BarrelDMBoxes)[recordIndex]->getDouble("HHGT");
  recordIndex = boxMap["Crate3"];
  double crate3hlen = (*BarrelDMBoxes)[recordIndex]->getDouble("HLEN");
  double crate3hwdt = (*BarrelDMBoxes)[recordIndex]->getDouble("HWDT");
  double crate3hhgt = (*BarrelDMBoxes)[recordIndex]->getDouble("HHGT");
  double crate3xtr = (*BarrelDMBoxes)[recordIndex]->getDouble("XTR");
  recordIndex = boxMap["BoardE"];
  double BoardEhlen = (*BarrelDMBoxes)[recordIndex]->getDouble("HLEN");
  double BoardEhwdt = (*BarrelDMBoxes)[recordIndex]->getDouble("HWDT");
  double BoardEhhgt = (*BarrelDMBoxes)[recordIndex]->getDouble("HHGT");
  double BoardExtr = (*BarrelDMBoxes)[recordIndex]->getDouble("XTR");
  double BoardEytr = (*BarrelDMBoxes)[recordIndex]->getDouble("YTR");
  double BoardEztr = (*BarrelDMBoxes)[recordIndex]->getDouble("ZTR");
  recordIndex = boxMap["Box"];
  double Boxhlen = (*BarrelDMBoxes)[recordIndex]->getDouble("HLEN");
  double Boxhwdt = (*BarrelDMBoxes)[recordIndex]->getDouble("HWDT");
  double Boxhhgt = (*BarrelDMBoxes)[recordIndex]->getDouble("HHGT");
  double Boxxtr = (*BarrelDMBoxes)[recordIndex]->getDouble("XTR");
  double Boxytr = (*BarrelDMBoxes)[recordIndex]->getDouble("YTR");
  double Boxxrot = (*BarrelDMBoxes)[recordIndex]->getDouble("XROT");

  // ----- build pedestal -----
  GeoBox     *Pedestal = new GeoBox(pedesthlen, pedesthwdt, pedesthhgt);
  GeoBox     *Ped1     = new GeoBox(ped1hlen, ped1hwdt, ped1hhgt);
  GeoTube    *Ped2     = new GeoTube(ped2minr, ped2maxr, ped2zhlen);
  GeoTube    *Ped3     = new GeoTube(ped3minr,ped3maxr , ped3zhlen);
  const GeoShape & CratePed=((*Pedestal).subtract(*Ped1).
                             subtract((*Ped2)  <<GeoTrf::TranslateY3D(-ped2ytr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg)).
                             subtract((*Ped3)  <<GeoTrf::TranslateX3D(-ped3xtr)).
                             subtract((*Ped2)  <<GeoTrf::TranslateY3D(ped2ytr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg)));

  GeoLogVol  *lvped   = new GeoLogVol("LAr::DM::Ped",&CratePed,alu);
  GeoPhysVol *pedestal   = new GeoPhysVol(lvped);

  // ----- build crates -----
  GeoBox     *Crate1   = new GeoBox(crate1hlen, crate1hwdt, crate1hhgt);
  GeoBox     *Crate2   = new GeoBox(crate2hlen, crate2hwdt, crate2hhgt);
  GeoBox     *Crate3   = new GeoBox(crate3hlen, crate3hwdt, crate3hhgt);
  const GeoShape & FEBCrate=(*Crate1).subtract(*Crate2).add((*Crate3)  <<GeoTrf::TranslateX3D(-crate3xtr));

  GeoLogVol  *lvcrate = new GeoLogVol("LAr::DM::Crate",&FEBCrate,alu);
  GeoPhysVol *crate   = new GeoPhysVol(lvcrate);

  // ----- build boardenvelopes -----
  GeoBox     *BoardEnvelope   = new GeoBox(BoardEhlen, BoardEhwdt, BoardEhhgt);
  GeoLogVol  *lvbenv = new GeoLogVol("LAr::DM::FEBoard",BoardEnvelope,matBoardsEnvelope);
  GeoPhysVol *boardenvelope   = new GeoPhysVol(lvbenv);

  //-------------- Place volumes in envelope ----------------------------

  //Crates
  TRANSFUNCTION crA = Pow(GeoTrf::RotateZ3D(1.0),f)*GeoTrf::TranslateX3D(crate1xtr)*GeoTrf::TranslateZ3D(crate1ztr);
  TRANSFUNCTION crC = Pow(GeoTrf::RotateZ3D(1.0),f)*GeoTrf::TranslateX3D(crate1xtr)*GeoTrf::TranslateZ3D(-crate1ztr);
  GeoSerialTransformer *crtA = new GeoSerialTransformer(crate,&crA, NCrates);
  GeoSerialTransformer *crtC = new GeoSerialTransformer(crate,&crC, NCrates);
  envelope->add(crtA);
  envelope->add(crtC);

  //Pedestals
  TRANSFUNCTION pedA = Pow(GeoTrf::RotateZ3D(1.0),f)*GeoTrf::TranslateX3D(pedestxtr)*GeoTrf::TranslateZ3D(pedestztr);
  TRANSFUNCTION pedC = Pow(GeoTrf::RotateZ3D(1.0),f)*GeoTrf::TranslateX3D(pedestxtr)*GeoTrf::TranslateZ3D(-pedestztr);
  GeoSerialTransformer *pedtA = new GeoSerialTransformer(pedestal,&pedA, NCrates);
  GeoSerialTransformer *pedtC = new GeoSerialTransformer(pedestal,&pedC, NCrates);
  envelope->add(pedtA);
  envelope->add(pedtC);

  //FEBoards
  TRANSFUNCTION feb1A = Pow(GeoTrf::RotateZ3D(1.0),f)*GeoTrf::TranslateY3D(BoardEytr)*GeoTrf::TranslateX3D(BoardExtr)*GeoTrf::TranslateZ3D(BoardEztr);
  TRANSFUNCTION feb2A = Pow(GeoTrf::RotateZ3D(1.0),f)*GeoTrf::TranslateY3D(-BoardEytr)*GeoTrf::TranslateX3D(BoardExtr)*GeoTrf::TranslateZ3D(BoardEztr);
  TRANSFUNCTION feb1C = Pow(GeoTrf::RotateZ3D(1.0),f)*GeoTrf::TranslateY3D(BoardEytr)*GeoTrf::TranslateX3D(BoardExtr)*GeoTrf::TranslateZ3D(-BoardEztr);
  TRANSFUNCTION feb2C = Pow(GeoTrf::RotateZ3D(1.0),f)*GeoTrf::TranslateY3D(-BoardEytr)*GeoTrf::TranslateX3D(BoardExtr)*GeoTrf::TranslateZ3D(-BoardEztr);
  GeoSerialTransformer *febt1A = new GeoSerialTransformer(boardenvelope,&feb1A, NCrates);
  GeoSerialTransformer *febt1C = new GeoSerialTransformer(boardenvelope,&feb1C, NCrates);
  GeoSerialTransformer *febt2A = new GeoSerialTransformer(boardenvelope,&feb2A, NCrates);
  GeoSerialTransformer *febt2C = new GeoSerialTransformer(boardenvelope,&feb2C, NCrates);
  envelope->add(febt1A);
  envelope->add(febt1C);
  envelope->add(febt2A);
  envelope->add(febt2C);

  ////----------- Building envelopes and support plates for Cables and Tubes --------------
  recordIndex = trdMap["SecP"];
  double SecPxhlen1 = (*BarrelDMTrds)[recordIndex]->getDouble("XHLEN1");
  double SecPxhlen2 = (*BarrelDMTrds)[recordIndex]->getDouble("XHLEN2");
  double SecPyhlen1 = (*BarrelDMTrds)[recordIndex]->getDouble("YHLEN1");//
  double SecPyhlen2 = (*BarrelDMTrds)[recordIndex]->getDouble("YHLEN2");//
  double SecPzhlen = (*BarrelDMTrds)[recordIndex]->getDouble("ZHLEN");//
  double SecPxtr = (*BarrelDMTrds)[recordIndex]->getDouble("XTR");
  double SecPztr = (*BarrelDMTrds)[recordIndex]->getDouble("ZTR");
  recordIndex = trdMap["BaseP"];
  double BasePxhlen1 = (*BarrelDMTrds)[recordIndex]->getDouble("XHLEN1");
  double BasePxhlen2 = (*BarrelDMTrds)[recordIndex]->getDouble("XHLEN2");
  double BasePyhlen1 = (*BarrelDMTrds)[recordIndex]->getDouble("YHLEN1");
  double BasePyhlen2 = (*BarrelDMTrds)[recordIndex]->getDouble("YHLEN2");
  double BasePzhlen = (*BarrelDMTrds)[recordIndex]->getDouble("ZHLEN");
  double BasePxtr = (*BarrelDMTrds)[recordIndex]->getDouble("XTR");
  double BasePztr = (*BarrelDMTrds)[recordIndex]->getDouble("ZTR");
  recordIndex = trdMap["SecE1"];
  double SecE1xhlen1 = (*BarrelDMTrds)[recordIndex]->getDouble("XHLEN1");
  double SecE1xhlen2 = (*BarrelDMTrds)[recordIndex]->getDouble("XHLEN2");
  double SecE1ztr = (*BarrelDMTrds)[recordIndex]->getDouble("ZTR");
  recordIndex = trapMap["BridgeP"];
  double BridgePzhlen = (*BarrelDMTraps)[recordIndex]->getDouble("ZHLEN");
  double BridgePtheta = (*BarrelDMTraps)[recordIndex]->getDouble("THETA");
  double BridgePphi = (*BarrelDMTraps)[recordIndex]->getDouble("PHI");
  double BridgePyzn = (*BarrelDMTraps)[recordIndex]->getDouble("YZN");
  double BridgePxynzn = (*BarrelDMTraps)[recordIndex]->getDouble("XYNZN");
  double BridgePxypzn = (*BarrelDMTraps)[recordIndex]->getDouble("XYPZN");
  double BridgePangn = (*BarrelDMTraps)[recordIndex]->getDouble("ANGN");
  double BridgePyzp = (*BarrelDMTraps)[recordIndex]->getDouble("YZP");
  double BridgePxynzp = (*BarrelDMTraps)[recordIndex]->getDouble("XYNZP");
  double BridgePxypzp = (*BarrelDMTraps)[recordIndex]->getDouble("XYPZP");
  double BridgePangp = (*BarrelDMTraps)[recordIndex]->getDouble("ANGP");
  double BridgePxtr = (*BarrelDMTraps)[recordIndex]->getDouble("XTR");
  double BridgePztr = (*BarrelDMTraps)[recordIndex]->getDouble("ZTR");

  // transforms
  GeoBox   *Box   = new GeoBox(Boxhlen, Boxhwdt, Boxhhgt);

  GeoTrf::Transform3D Cut3Boxp  = GeoTrf::Translate3D(Boxxtr, Boxytr, Boxxrot)*GeoTrf::RotateX3D(-20*Gaudi::Units::deg)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  GeoTrf::Transform3D Cut4Boxp  = GeoTrf::Translate3D(Boxxtr, -Boxytr,Boxxrot)*GeoTrf::RotateX3D(20*Gaudi::Units::deg)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);

  // ----- build sector envelopes -----
  // build 16 instances of SectorEnvelopes1 each with its own material!
  GeoTrd   *Trdair1  = new GeoTrd(SecE1xhlen1, SecE1xhlen2, DYa, DYb, (Endb-Enda)/2);

  GeoLogVol  *lvse1g1          = new GeoLogVol("LAr::DM::SectorEnvelopes1g1",Trdair1,matLArServices1);
  GeoPhysVol *sectorenvelopes1g1    = new GeoPhysVol(lvse1g1);

  GeoLogVol  *lvse1g2          = new GeoLogVol("LAr::DM::SectorEnvelopes1g3",Trdair1,matLArServices2);
  GeoPhysVol *sectorenvelopes1g2    = new GeoPhysVol(lvse1g2);

  GeoLogVol  *lvse1g3          = new GeoLogVol("LAr::DM::SectorEnvelopes1g3",Trdair1,matLArServices3);
  GeoPhysVol *sectorenvelopes1g3    = new GeoPhysVol(lvse1g3);

  GeoLogVol  *lvse1g4          = new GeoLogVol("LAr::DM::SectorEnvelopes1g4",Trdair1,matLArServices4);
  GeoPhysVol *sectorenvelopes1g4    = new GeoPhysVol(lvse1g4);

  GeoLogVol  *lvse1g5          = new GeoLogVol("LAr::DM::SectorEnvelopes1g5",Trdair1,matLArServices5);
  GeoPhysVol *sectorenvelopes1g5    = new GeoPhysVol(lvse1g5);

  GeoLogVol  *lvse1g6          = new GeoLogVol("LAr::DM::SectorEnvelopes1g6",Trdair1,matLArServices6);
  GeoPhysVol *sectorenvelopes1g6    = new GeoPhysVol(lvse1g6);

  GeoLogVol  *lvse1g7          = new GeoLogVol("LAr::DM::SectorEnvelopes1g7",Trdair1,matLArServices7);
  GeoPhysVol *sectorenvelopes1g7    = new GeoPhysVol(lvse1g7);

  GeoLogVol  *lvse1g8          = new GeoLogVol("LAr::DM::SectorEnvelopes1g8",Trdair1,matLArServices8);
  GeoPhysVol *sectorenvelopes1g8    = new GeoPhysVol(lvse1g8);

  GeoLogVol  *lvse1g9          = new GeoLogVol("LAr::DM::SectorEnvelopes1g9",Trdair1,matLArServices9);
  GeoPhysVol *sectorenvelopes1g9    = new GeoPhysVol(lvse1g9);

  GeoLogVol  *lvse1g10          = new GeoLogVol("LAr::DM::SectorEnvelopes1g10",Trdair1,matLArServices10);
  GeoPhysVol *sectorenvelopes1g10    = new GeoPhysVol(lvse1g10);

  GeoLogVol  *lvse1g11          = new GeoLogVol("LAr::DM::SectorEnvelopes1g11",Trdair1,matLArServices11);
  GeoPhysVol *sectorenvelopes1g11    = new GeoPhysVol(lvse1g11);

  GeoLogVol  *lvse1g12          = new GeoLogVol("LAr::DM::SectorEnvelopes1g12",Trdair1,matLArServices12);
  GeoPhysVol *sectorenvelopes1g12    = new GeoPhysVol(lvse1g12);

  GeoLogVol  *lvse1g13          = new GeoLogVol("LAr::DM::SectorEnvelopes1g13",Trdair1,matLArServices13);
  GeoPhysVol *sectorenvelopes1g13    = new GeoPhysVol(lvse1g13);

  GeoLogVol  *lvse1g14          = new GeoLogVol("LAr::DM::SectorEnvelopes1g14",Trdair1,matLArServices14);
  GeoPhysVol *sectorenvelopes1g14    = new GeoPhysVol(lvse1g14);

  GeoLogVol  *lvse1g15          = new GeoLogVol("LAr::DM::SectorEnvelopes1g15",Trdair1,matLArServices15);
  GeoPhysVol *sectorenvelopes1g15    = new GeoPhysVol(lvse1g15);

  GeoLogVol  *lvse1g16          = new GeoLogVol("LAr::DM::SectorEnvelopes1g16",Trdair1,matLArServices16);
  GeoPhysVol *sectorenvelopes1g16    = new GeoPhysVol(lvse1g16);


  std::vector<GeoPhysVol*> se1List;
  se1List.push_back(sectorenvelopes1g1);
  se1List.push_back(sectorenvelopes1g2);
  se1List.push_back(sectorenvelopes1g3);
  se1List.push_back(sectorenvelopes1g4);
  se1List.push_back(sectorenvelopes1g5);
  se1List.push_back(sectorenvelopes1g6);
  se1List.push_back(sectorenvelopes1g7);
  se1List.push_back(sectorenvelopes1g8);
  se1List.push_back(sectorenvelopes1g9);
  se1List.push_back(sectorenvelopes1g10);
  se1List.push_back(sectorenvelopes1g11);
  se1List.push_back(sectorenvelopes1g12);
  se1List.push_back(sectorenvelopes1g13);
  se1List.push_back(sectorenvelopes1g14);
  se1List.push_back(sectorenvelopes1g15);
  se1List.push_back(sectorenvelopes1g16);

  // dedicated volumes to add in SectorEnvelopes1
  //    (a) volume in Trd shape to add slice of extra material in a given eta bin
  IRDBRecordset_ptr BarrelDMRing = rdbAccess->getRecordsetPtr("LArDMEnv1Ring",keyLAr.tag(),keyLAr.node());
  for (unsigned int i=0;i<BarrelDMRing->size();i++) {
    double etaMin=(*BarrelDMRing)[i]->getDouble("ETAMIN");
    double etaMax=(*BarrelDMRing)[i]->getDouble("ETAMAX");
    double thicknessExtra=(*BarrelDMRing)[i]->getDouble("THICKNESS");
    std::string ringName = "LAr::DM::SectorEnvelopes1::"+(*BarrelDMRing)[i]->getString("RINGNAME");
    
    double radiusMin=SecE1ztr/sinh(etaMax);
    double radiusMax=SecE1ztr/sinh(etaMin);
    double dy1 = DYa + (DYb-DYa)*(radiusMin-Enda)*inv_Endab;
    double dy2 = DYa + (DYb-DYa)*(radiusMax-Enda)*inv_Endab;
    double zpos=0.5*(radiusMax+radiusMin - (Endb+Enda));
    const GeoMaterial *matExtraTdr = materialManager->getMaterial((*BarrelDMRing)[i]->getString("MATERIAL"));
    
    GeoTrd   *extraMatTdr  = new GeoTrd(thicknessExtra/2., thicknessExtra/2., dy1, dy2, (radiusMax-radiusMin)/2);
    GeoLogVol *extraMatLog = new GeoLogVol(ringName,extraMatTdr,matExtraTdr);
    GeoPhysVol *extraMatPhys = new GeoPhysVol(extraMatLog);
    for (unsigned int isect=0;isect<se1List.size();isect++) {
      se1List[isect]->add(new GeoTransform(GeoTrf::TranslateZ3D(zpos)));
      se1List[isect]->add(extraMatPhys);
    }
  }

  // (b) extra material at fixed phi locations in the PPF1 area
  IRDBRecordset_ptr BarrelDMPhiBox = rdbAccess->getRecordsetPtr("LArDMEnv1PhiBox",keyLAr.tag(),keyLAr.node());
  for (unsigned int i=0;i<BarrelDMPhiBox->size();i++) {
    double eta=(*BarrelDMPhiBox)[i]->getDouble("ETA");
    double phi0=(*BarrelDMPhiBox)[i]->getDouble("PHI0");
    double deltaR=(*BarrelDMPhiBox)[i]->getDouble("DELTAR");
    double deltaRphi=(*BarrelDMPhiBox)[i]->getDouble("DELTARPHI");
    double thickness=(*BarrelDMPhiBox)[i]->getDouble("THICKNESS");
    int nphi=(*BarrelDMPhiBox)[i]->getInt("NPHI");
    int noHorizontal = (*BarrelDMPhiBox)[i]->getInt("NOHORIZ");
    
    const GeoMaterial* matExtraPPF1 = materialManager->getMaterial((*BarrelDMPhiBox)[i]->getString("MATERIAL"));
    std::string boxName = "LAr::DM::SectorEnvelopes1::"+(*BarrelDMPhiBox)[i]->getString("BOXNAME");
    
    GeoBox *ppf1Box = new GeoBox(thickness/2.,deltaRphi/2.,deltaR/2.);
    GeoLogVol *ppf1Log = new GeoLogVol(boxName,ppf1Box,matExtraPPF1);
    GeoPhysVol *ppf1Phys = new GeoPhysVol(ppf1Log);
    
    int nPerEnv1 = nphi/NCrates;
    double dphi=2.*M_PI/((float)(nphi));
    double radius=SecE1ztr/sinh(eta);
    for (int iphi=0;iphi<nPerEnv1;iphi++) {
      double xpos=0.;
      double ypos=radius*sin(phi0+((float)(iphi))*dphi);
      double zpos=radius*cos(phi0+((float)(iphi))*dphi) - (Endb+Enda)/2.;
      for (unsigned int isect=0;isect<se1List.size();isect++) {
	// no PPF1 box around phi=0 and phi=pi
	if (noHorizontal>0 && ((isect==7 && iphi==1) || (isect==8 && iphi==0) || (isect==15 && iphi==1) || (isect==0 && iphi==0) ) ) continue;
	se1List[isect]->add(new GeoTransform(GeoTrf::Translate3D(xpos,ypos,zpos)));
	se1List[isect]->add(ppf1Phys);
      }
    }
  }

  // ----- build base plates -----
  GeoTrd   *Trd1alu  = new GeoTrd(BasePxhlen1, BasePxhlen2, BasePyhlen1, BasePyhlen2, BasePzhlen);
  GeoLogVol  *lvbp          = new GeoLogVol("LAr::DM::BasePlates",Trd1alu,alu);
  GeoPhysVol *baseplates    = new GeoPhysVol(lvbp);

  // ----- build bridge plates -----
  GeoTrap  *Trapalu  = new GeoTrap(BridgePzhlen, BridgePtheta*Gaudi::Units::deg, BridgePphi, BridgePyzn, BridgePxynzn, BridgePxypzn, BridgePangn, BridgePyzp, BridgePxynzp, BridgePxypzp, BridgePangp);
  GeoLogVol  *lvbrp          = new GeoLogVol("LAr::DM::BridgePlates",Trapalu,alu);
  GeoPhysVol *bridgeplates    = new GeoPhysVol(lvbrp);


  // ----- build sector plates -----
  GeoTrd   *Trd2alu  = new GeoTrd(SecPxhlen1, SecPxhlen2, SecPyhlen1, SecPyhlen2, SecPzhlen );///
  const GeoShape & SectorPlates= ((*Trd2alu).
                                  subtract((*Box)  <<GeoTrf::Transform3D(Cut3Boxp)).
                                  subtract((*Box)  <<GeoTrf::Transform3D(Cut4Boxp)));
  GeoLogVol  *lvsp          = new GeoLogVol("LAr::DM::SectorPlates",&SectorPlates,alu);
  GeoPhysVol *sectorplates    = new GeoPhysVol(lvsp);


  //-------------- Place volumes in LAr Envelope -------------------

  //sectorPlates
  TRANSFUNCTION spA = Pow(GeoTrf::RotateZ3D(1.0),f-(Alfa/2))*GeoTrf::TranslateX3D(SecPxtr)*GeoTrf::TranslateZ3D(SecPztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);///
  TRANSFUNCTION spC = Pow(GeoTrf::RotateZ3D(1.0),f+(Alfa/2))*GeoTrf::TranslateX3D(SecPxtr)*GeoTrf::TranslateZ3D(-SecPztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);///
  GeoSerialTransformer *sptA = new GeoSerialTransformer(sectorplates,&spA, NCrates);
  GeoSerialTransformer *sptC = new GeoSerialTransformer(sectorplates,&spC, NCrates);
  envelope->add(sptA);
  envelope->add(sptC);

  //bridgePlates
  TRANSFUNCTION brpA1 = Pow(GeoTrf::RotateZ3D(1.0),f-(5*Alfa/2))*GeoTrf::TranslateX3D(BridgePxtr)*GeoTrf::TranslateZ3D(BridgePztr)*GeoTrf::RotateZ3D(90*Gaudi::Units::deg)*GeoTrf::RotateY3D(90*Gaudi::Units::deg)*GeoTrf::RotateX3D(90*Gaudi::Units::deg);
  TRANSFUNCTION brpA2 = Pow(GeoTrf::RotateZ3D(1.0),f+(13*Alfa/2))*GeoTrf::TranslateX3D(BridgePxtr)*GeoTrf::TranslateZ3D(BridgePztr)*GeoTrf::RotateZ3D(90*Gaudi::Units::deg)*GeoTrf::RotateY3D(90*Gaudi::Units::deg)*GeoTrf::RotateX3D(90*Gaudi::Units::deg);
  TRANSFUNCTION brpC1 = Pow(GeoTrf::RotateZ3D(1.0),f-(5*Alfa/2))*GeoTrf::TranslateX3D(BridgePxtr)*GeoTrf::TranslateZ3D(-BridgePztr)*GeoTrf::RotateZ3D(-90*Gaudi::Units::deg)*GeoTrf::RotateY3D(-90*Gaudi::Units::deg)*GeoTrf::RotateX3D(-90*Gaudi::Units::deg);
  TRANSFUNCTION brpC2 = Pow(GeoTrf::RotateZ3D(1.0),f+(13*Alfa/2))*GeoTrf::TranslateX3D(BridgePxtr)*GeoTrf::TranslateZ3D(-BridgePztr)*GeoTrf::RotateZ3D(-90*Gaudi::Units::deg)*GeoTrf::RotateY3D(-90*Gaudi::Units::deg)*GeoTrf::RotateX3D(-90*Gaudi::Units::deg);   GeoSerialTransformer *brptA1 = new GeoSerialTransformer(bridgeplates,&brpA1, 5);
  GeoSerialTransformer *brptA2 = new GeoSerialTransformer(bridgeplates,&brpA2, 5);
  GeoSerialTransformer *brptC1 = new GeoSerialTransformer(bridgeplates,&brpC1, 5);
  GeoSerialTransformer *brptC2 = new GeoSerialTransformer(bridgeplates,&brpC2, 5);
  envelope->add(brptA1);
  envelope->add(brptA2);
  envelope->add(brptC1);
  envelope->add(brptC2);

  //basePlates
  TRANSFUNCTION bpA = Pow(GeoTrf::RotateZ3D(1.0),f-(Alfa/2))*GeoTrf::TranslateX3D(BasePxtr)*GeoTrf::TranslateZ3D(BasePztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION bpC = Pow(GeoTrf::RotateZ3D(1.0),f+(Alfa/2))*GeoTrf::TranslateX3D(BasePxtr)*GeoTrf::TranslateZ3D(-BasePztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  GeoSerialTransformer *bptA = new GeoSerialTransformer(baseplates,&bpA, NCrates);
  GeoSerialTransformer *bptC = new GeoSerialTransformer(baseplates,&bpC, NCrates);
  envelope->add(bptA);
  envelope->add(bptC);

  //sectorEnvelopes1
  //counter-clockwise from top if taking sideA for reference (clockwise for sideC)
  TRANSFUNCTION seA1G5 = Pow(GeoTrf::RotateZ3D(1.0),f+(9*Alfa/2))*GeoTrf::TranslateX3D((Endb+Enda)/2)*GeoTrf::TranslateZ3D(SecE1ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seC1G5 = Pow(GeoTrf::RotateZ3D(1.0),f+(9*Alfa/2))*GeoTrf::TranslateX3D((Endb+Enda)/2)*GeoTrf::TranslateZ3D(-SecE1ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seA1G6 = Pow(GeoTrf::RotateZ3D(1.0),f+(11*Alfa/2))*GeoTrf::TranslateX3D((Endb+Enda)/2)*GeoTrf::TranslateZ3D(SecE1ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seC1G6 = Pow(GeoTrf::RotateZ3D(1.0),f+(11*Alfa/2))*GeoTrf::TranslateX3D((Endb+Enda)/2)*GeoTrf::TranslateZ3D(-SecE1ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seA1G7 = Pow(GeoTrf::RotateZ3D(1.0),f+(13*Alfa/2))*GeoTrf::TranslateX3D((Endb+Enda)/2)*GeoTrf::TranslateZ3D(SecE1ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seC1G7 = Pow(GeoTrf::RotateZ3D(1.0),f+(13*Alfa/2))*GeoTrf::TranslateX3D((Endb+Enda)/2)*GeoTrf::TranslateZ3D(-SecE1ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seA1G8 = Pow(GeoTrf::RotateZ3D(1.0),f+(15*Alfa/2))*GeoTrf::TranslateX3D((Endb+Enda)/2)*GeoTrf::TranslateZ3D(SecE1ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seC1G8 = Pow(GeoTrf::RotateZ3D(1.0),f+(15*Alfa/2))*GeoTrf::TranslateX3D((Endb+Enda)/2)*GeoTrf::TranslateZ3D(-SecE1ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seA1G9 = Pow(GeoTrf::RotateZ3D(1.0),f+(17*Alfa/2))*GeoTrf::TranslateX3D((Endb+Enda)/2)*GeoTrf::TranslateZ3D(SecE1ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seC1G9 = Pow(GeoTrf::RotateZ3D(1.0),f+(17*Alfa/2))*GeoTrf::TranslateX3D((Endb+Enda)/2)*GeoTrf::TranslateZ3D(-SecE1ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seA1G10 = Pow(GeoTrf::RotateZ3D(1.0),f+(19*Alfa/2))*GeoTrf::TranslateX3D((Endb+Enda)/2)*GeoTrf::TranslateZ3D(SecE1ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seC1G10 = Pow(GeoTrf::RotateZ3D(1.0),f+(19*Alfa/2))*GeoTrf::TranslateX3D((Endb+Enda)/2)*GeoTrf::TranslateZ3D(-SecE1ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seA1G11 = Pow(GeoTrf::RotateZ3D(1.0),f+(21*Alfa/2))*GeoTrf::TranslateX3D((Endb+Enda)/2)*GeoTrf::TranslateZ3D(SecE1ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seC1G11 = Pow(GeoTrf::RotateZ3D(1.0),f+(21*Alfa/2))*GeoTrf::TranslateX3D((Endb+Enda)/2)*GeoTrf::TranslateZ3D(-SecE1ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seA1G12 = Pow(GeoTrf::RotateZ3D(1.0),f+(23*Alfa/2))*GeoTrf::TranslateX3D((Endb+Enda)/2)*GeoTrf::TranslateZ3D(SecE1ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seC1G12 = Pow(GeoTrf::RotateZ3D(1.0),f+(23*Alfa/2))*GeoTrf::TranslateX3D((Endb+Enda)/2)*GeoTrf::TranslateZ3D(-SecE1ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  //clockwise from top if taking sideA for reference (counter-clockwise for sideC)
  TRANSFUNCTION seA1G4 = Pow(GeoTrf::RotateZ3D(1.0),f+(7*Alfa/2))*GeoTrf::TranslateX3D((Endb+Enda)/2)*GeoTrf::TranslateZ3D(SecE1ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seC1G4 = Pow(GeoTrf::RotateZ3D(1.0),f+(7*Alfa/2))*GeoTrf::TranslateX3D((Endb+Enda)/2)*GeoTrf::TranslateZ3D(-SecE1ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seA1G3 = Pow(GeoTrf::RotateZ3D(1.0),f+(5*Alfa/2))*GeoTrf::TranslateX3D((Endb+Enda)/2)*GeoTrf::TranslateZ3D(SecE1ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seC1G3 = Pow(GeoTrf::RotateZ3D(1.0),f+(5*Alfa/2))*GeoTrf::TranslateX3D((Endb+Enda)/2)*GeoTrf::TranslateZ3D(-SecE1ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seA1G2 = Pow(GeoTrf::RotateZ3D(1.0),f+(3*Alfa/2))*GeoTrf::TranslateX3D((Endb+Enda)/2)*GeoTrf::TranslateZ3D(SecE1ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seC1G2 = Pow(GeoTrf::RotateZ3D(1.0),f+(3*Alfa/2))*GeoTrf::TranslateX3D((Endb+Enda)/2)*GeoTrf::TranslateZ3D(-SecE1ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seA1G1 = Pow(GeoTrf::RotateZ3D(1.0),f+(1*Alfa/2))*GeoTrf::TranslateX3D((Endb+Enda)/2)*GeoTrf::TranslateZ3D(SecE1ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seC1G1 = Pow(GeoTrf::RotateZ3D(1.0),f+(1*Alfa/2))*GeoTrf::TranslateX3D((Endb+Enda)/2)*GeoTrf::TranslateZ3D(-SecE1ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seA1G16 = Pow(GeoTrf::RotateZ3D(1.0),f-(1*Alfa/2))*GeoTrf::TranslateX3D((Endb+Enda)/2)*GeoTrf::TranslateZ3D(SecE1ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seC1G16 = Pow(GeoTrf::RotateZ3D(1.0),f-(1*Alfa/2))*GeoTrf::TranslateX3D((Endb+Enda)/2)*GeoTrf::TranslateZ3D(-SecE1ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seA1G15 = Pow(GeoTrf::RotateZ3D(1.0),f-(3*Alfa/2))*GeoTrf::TranslateX3D((Endb+Enda)/2)*GeoTrf::TranslateZ3D(SecE1ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seC1G15 = Pow(GeoTrf::RotateZ3D(1.0),f-(3*Alfa/2))*GeoTrf::TranslateX3D((Endb+Enda)/2)*GeoTrf::TranslateZ3D(-SecE1ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seA1G14 = Pow(GeoTrf::RotateZ3D(1.0),f-(5*Alfa/2))*GeoTrf::TranslateX3D((Endb+Enda)/2)*GeoTrf::TranslateZ3D(SecE1ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seC1G14 = Pow(GeoTrf::RotateZ3D(1.0),f-(5*Alfa/2))*GeoTrf::TranslateX3D((Endb+Enda)/2)*GeoTrf::TranslateZ3D(-SecE1ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seA1G13 = Pow(GeoTrf::RotateZ3D(1.0),f-(7*Alfa/2))*GeoTrf::TranslateX3D((Endb+Enda)/2)*GeoTrf::TranslateZ3D(SecE1ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
  TRANSFUNCTION seC1G13 = Pow(GeoTrf::RotateZ3D(1.0),f-(7*Alfa/2))*GeoTrf::TranslateX3D((Endb+Enda)/2)*GeoTrf::TranslateZ3D(-SecE1ztr)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);

  GeoSerialTransformer *setA1G5 = new GeoSerialTransformer(sectorenvelopes1g5,&seA1G5, 1);
  GeoSerialTransformer *setC1G5 = new GeoSerialTransformer(sectorenvelopes1g5,&seC1G5, 1);
  GeoSerialTransformer *setA1G6 = new GeoSerialTransformer(sectorenvelopes1g6,&seA1G6, 1);
  GeoSerialTransformer *setC1G6 = new GeoSerialTransformer(sectorenvelopes1g6,&seC1G6, 1);
  GeoSerialTransformer *setA1G7 = new GeoSerialTransformer(sectorenvelopes1g7,&seA1G7, 1);
  GeoSerialTransformer *setC1G7 = new GeoSerialTransformer(sectorenvelopes1g7,&seC1G7, 1);
  GeoSerialTransformer *setA1G8 = new GeoSerialTransformer(sectorenvelopes1g8,&seA1G8, 1);
  GeoSerialTransformer *setC1G8 = new GeoSerialTransformer(sectorenvelopes1g8,&seC1G8, 1);
  GeoSerialTransformer *setA1G9 = new GeoSerialTransformer(sectorenvelopes1g9,&seA1G9, 1);
  GeoSerialTransformer *setC1G9 = new GeoSerialTransformer(sectorenvelopes1g9,&seC1G9, 1);
  GeoSerialTransformer *setA1G10 = new GeoSerialTransformer(sectorenvelopes1g10,&seA1G10, 1);
  GeoSerialTransformer *setC1G10 = new GeoSerialTransformer(sectorenvelopes1g10,&seC1G10, 1);
  GeoSerialTransformer *setA1G11 = new GeoSerialTransformer(sectorenvelopes1g11,&seA1G11, 1);
  GeoSerialTransformer *setC1G11 = new GeoSerialTransformer(sectorenvelopes1g11,&seC1G11, 1);
  GeoSerialTransformer *setA1G12 = new GeoSerialTransformer(sectorenvelopes1g12,&seA1G12, 1);
  GeoSerialTransformer *setC1G12 = new GeoSerialTransformer(sectorenvelopes1g12,&seC1G12, 1);

  GeoSerialTransformer *setA1G4 = new GeoSerialTransformer(sectorenvelopes1g4,&seA1G4, 1);
  GeoSerialTransformer *setC1G4 = new GeoSerialTransformer(sectorenvelopes1g4,&seC1G4, 1);
  GeoSerialTransformer *setA1G3 = new GeoSerialTransformer(sectorenvelopes1g3,&seA1G3, 1);
  GeoSerialTransformer *setC1G3 = new GeoSerialTransformer(sectorenvelopes1g3,&seC1G3, 1);
  GeoSerialTransformer *setA1G2 = new GeoSerialTransformer(sectorenvelopes1g2,&seA1G2, 1);
  GeoSerialTransformer *setC1G2 = new GeoSerialTransformer(sectorenvelopes1g2,&seC1G2, 1);
  GeoSerialTransformer *setA1G1 = new GeoSerialTransformer(sectorenvelopes1g1,&seA1G1, 1);
  GeoSerialTransformer *setC1G1 = new GeoSerialTransformer(sectorenvelopes1g1,&seC1G1, 1);
  GeoSerialTransformer *setA1G16 = new GeoSerialTransformer(sectorenvelopes1g16,&seA1G16, 1);
  GeoSerialTransformer *setC1G16 = new GeoSerialTransformer(sectorenvelopes1g16,&seC1G16, 1);
  GeoSerialTransformer *setA1G15 = new GeoSerialTransformer(sectorenvelopes1g15,&seA1G15, 1);
  GeoSerialTransformer *setC1G15 = new GeoSerialTransformer(sectorenvelopes1g15,&seC1G15, 1);
  GeoSerialTransformer *setA1G14 = new GeoSerialTransformer(sectorenvelopes1g14,&seA1G14, 1);
  GeoSerialTransformer *setC1G14 = new GeoSerialTransformer(sectorenvelopes1g14,&seC1G14, 1);
  GeoSerialTransformer *setA1G13 = new GeoSerialTransformer(sectorenvelopes1g13,&seA1G13, 1);
  GeoSerialTransformer *setC1G13 = new GeoSerialTransformer(sectorenvelopes1g13,&seC1G13, 1);

  envelope->add(setA1G5);
  envelope->add(setC1G5);
  envelope->add(setA1G6);
  envelope->add(setC1G6);
  envelope->add(setA1G7);
  envelope->add(setC1G7);
  envelope->add(setA1G8);
  envelope->add(setC1G8);
  envelope->add(setA1G9);
  envelope->add(setC1G9);
  envelope->add(setA1G10);
  envelope->add(setC1G10);
  envelope->add(setA1G11);
  envelope->add(setC1G11);
  envelope->add(setA1G12);
  envelope->add(setC1G12);

  envelope->add(setA1G4);
  envelope->add(setC1G4);
  envelope->add(setA1G3);
  envelope->add(setC1G3);
  envelope->add(setA1G2);
  envelope->add(setC1G2);
  envelope->add(setA1G1);
  envelope->add(setC1G1);
  envelope->add(setA1G16);
  envelope->add(setC1G16);
  envelope->add(setA1G15);
  envelope->add(setC1G15);
  envelope->add(setA1G14);
  envelope->add(setC1G14);
  envelope->add(setA1G13);
  envelope->add(setC1G13);

  createSectorEnvelopes2FromDB (envelope, materialManager,
                                trdMap, *BarrelDMTrds,
                                trapMap, *BarrelDMTraps,
                                boxMap, *BarrelDMBoxes,
                                f, Box);
  createBridgeEnvelopesFromDB (envelope, trapMap, *BarrelDMTraps,
                             matLArServices8, f);
  createBaseEnvelopesFromDB (envelope, trdMap, *BarrelDMTrds,
                             matLArServices8, f);
}


CrackDMConstruction::CrackDMConstruction(IRDBAccessSvc* rdbAccess
					 , IGeoModelSvc* geoModel
					 , StoredMaterialManager* materialManager
					 , bool activateFT)
  : AthMessaging("CrackDMConstruction")
  , m_rdbAccess(rdbAccess)
  , m_geoModel(geoModel)
  , m_materialManager(materialManager)
  , m_activateFT(activateFT)
{
}

void CrackDMConstruction::create(GeoFullPhysVol* envelope)
{
  createFromDB(envelope
	       , m_rdbAccess
	       , m_geoModel
	       , m_materialManager);

  if(m_activateFT){
    std::string name = "LAr::Barrel::SignalFT::";
    ATH_MSG_DEBUG("creating " << name << " volumes");

    const GeoMaterial* iron = m_materialManager->getMaterial("std::Iron");

    const double wflange_height = 37.*Gaudi::Units::mm;
    const double wflange_R = 0.5*360.*Gaudi::Units::mm;
    const GeoMaterial* wflange_mat = m_materialManager->getMaterial("LAr::FT::WarmFlange");

    GeoShape* wflange = new GeoTube(0., wflange_R, wflange_height/2);
    GeoLogVol* wflangeLV = new GeoLogVol(name + "WarmFlange", wflange, wflange_mat);
    GeoPhysVol* wflangePV = new GeoPhysVol(wflangeLV);

    const double bellow_height = 225.*Gaudi::Units::mm;
    const double bellow_Router = 0.5*299.*Gaudi::Units::mm; // this also to be cut in warm wall
    const double bellow_wall = 15.*Gaudi::Units::mm;
    const GeoMaterial* bellow_mat = m_materialManager->getMaterial("LAr::FT::Bellow");

    const double bellow_Rinner = bellow_Router - bellow_wall;
    GeoShape* bellow = new GeoTube(bellow_Rinner, bellow_Router, bellow_height/2);
    GeoLogVol* bellowLV = new GeoLogVol(name + "Bellow", bellow, bellow_mat);
    GeoPhysVol* bellowPV = new GeoPhysVol(bellowLV);

    const GeoMaterial* vcables_mat = m_materialManager->getMaterial("LAr::FT::VacuumCables");

    GeoShape* vcables = new GeoTube(0., bellow_Rinner, bellow_height/2);
    GeoLogVol* vcablesLV = new GeoLogVol(name + "VacuumCables", vcables, vcables_mat);
    GeoPhysVol* vcablesPV = new GeoPhysVol(vcablesLV);

    const double cflange_height = 35.*Gaudi::Units::mm;
    const double cflange_Router = 0.5*283.*Gaudi::Units::mm;
    const GeoMaterial* cflange_mat = m_materialManager->getMaterial("LAr::FT::ColdFlange");
    GeoShape* cflange = new GeoTube(0., cflange_Router, cflange_height/2);
    GeoLogVol* cflangeLV = new GeoLogVol(name + "ColdFlange", cflange, cflange_mat);
    GeoPhysVol* cflangePV = new GeoPhysVol(cflangeLV);

    const double coldbox1_Router = cflange_Router;
    const double coldbox1_wall = 0.134*2.54*Gaudi::Units::cm;
    const double coldbox1_height = 90.*Gaudi::Units::mm;
    const double coldbox2_height = 16.*Gaudi::Units::mm;
    const double hole_r = 0.5*133.*Gaudi::Units::mm;
    const double hole_shift = -31.*Gaudi::Units::mm;
    const double coldbox3_Router = 0.5*140.*Gaudi::Units::mm; // this also to be cut in cold wall
    const double barrel_dist_from_outer_warm_wall_to_inner_cold_wall =
        500.*Gaudi::Units::mm;
    const double coldbox3_height = // adjust to match dist between cryo walls
        barrel_dist_from_outer_warm_wall_to_inner_cold_wall
        - bellow_height - coldbox1_height - coldbox2_height;
    ATH_MSG_DEBUG("funnel tube len = " << coldbox3_height / Gaudi::Units::mm << " mm ");
    const GeoMaterial* coldbox_mat = iron;
    GeoShape* coldbox1 = new GeoTube(coldbox1_Router - coldbox1_wall, coldbox1_Router, coldbox1_height/2); // wide part
    GeoLogVol* coldbox1LV = new GeoLogVol(name + "ColdBox1", coldbox1, coldbox_mat);
    GeoPhysVol* coldbox1PV = new GeoPhysVol(coldbox1LV);
    GeoShape* coldbox21 = new GeoTube(0., coldbox1_Router, coldbox2_height/2); // plate
    GeoShape* coldbox22 = new GeoTube(0., hole_r, coldbox2_height); // hole in the plate
    const GeoShape& coldbox2 = coldbox21->subtract((*coldbox22) << GeoTrf::TranslateY3D(hole_shift));
    GeoLogVol* coldbox2LV = new GeoLogVol(name + "ColdBox2", &coldbox2, coldbox_mat);
    GeoPhysVol* coldbox2PV = new GeoPhysVol(coldbox2LV);
    GeoShape* coldbox3 = new GeoTube(hole_r, coldbox3_Router, coldbox3_height/2); // narrow part
    GeoShape* coldbox31 = new GeoTube(0., coldbox3_Router, coldbox3_height/2); // narrow part for FTenvelope
    GeoLogVol* coldbox3LV = new GeoLogVol(name + "ColdBox3", coldbox3, coldbox_mat);
    GeoPhysVol* coldbox3PV = new GeoPhysVol(coldbox3LV);

    GeoTrf::TranslateZ3D bellow_pos(-wflange_height/2 - bellow_height/2);
    GeoTrf::TranslateZ3D cflange_pos(-wflange_height/2 - bellow_height - cflange_height/2);
    GeoTrf::TranslateZ3D coldbox1_pos(-wflange_height/2 - bellow_height - cflange_height - coldbox1_height/2);
    GeoTrf::TranslateZ3D coldbox2_pos(-wflange_height/2 - bellow_height - cflange_height - coldbox1_height - coldbox2_height/2);
    GeoTrf::Translate3D coldbox2hole_pos(
        0., hole_shift,
        -wflange_height/2 - bellow_height - cflange_height - coldbox1_height - coldbox2_height/2
    );
    GeoTrf::Translate3D coldbox3_pos(
        0., hole_shift,
        -wflange_height/2 - bellow_height - cflange_height - coldbox1_height - coldbox2_height - coldbox3_height/2
    );

    // The SignalFT air envelope is built as one coaxial polycone, plus one
    // boolean union with the non-coaxial ColdBox3 tube below.  From negative
    // to positive z, the polycone covers these coaxial envelope sections:
    // - z_min to the bellow: ColdBox2 plate, ColdBox1 and ColdFlange,
    //   all enclosed at coldbox1_Router;
    // - bellow: Bellow and VacuumCables, enclosed at bellow_Router;
    // - warm end: WarmFlange, enclosed at wflange_R.
    // Repeated z planes below encode the radius steps between these sections.
    GeoPcon* coaxialEnvelope = new GeoPcon(0., 360.*Gaudi::Units::deg);
    coaxialEnvelope->addPlane(
        -wflange_height/2 - bellow_height - cflange_height - coldbox1_height - coldbox2_height,
        0., coldbox1_Router
    );
    coaxialEnvelope->addPlane(
        -wflange_height/2 - bellow_height,
        0., coldbox1_Router
    );
    coaxialEnvelope->addPlane(
        -wflange_height/2 - bellow_height,
        0., bellow_Router
    );
    coaxialEnvelope->addPlane(
        -wflange_height/2,
        0., bellow_Router
    );
    coaxialEnvelope->addPlane(
        -wflange_height/2,
        0., wflange_R
    );
    coaxialEnvelope->addPlane(
        wflange_height/2,
        0., wflange_R
    );

    const GeoShape& FTenvelope = coaxialEnvelope->add((*coldbox31) << coldbox3_pos);

    GeoLogVol* FTLV = new GeoLogVol(name + "Envelope",
        &FTenvelope,
        m_materialManager->getMaterial("std::Air")
    );
    GeoPhysVol* FTPV = new GeoPhysVol(FTLV);

    FTPV->add(wflangePV);
    GeoTransform *bellow_trf = new GeoTransform(bellow_pos);
    FTPV->add(bellow_trf);
    FTPV->add(bellowPV);
    FTPV->add(bellow_trf);
    FTPV->add(vcablesPV);
    GeoTransform *cflange_trf = new GeoTransform(cflange_pos);
    FTPV->add(cflange_trf);
    FTPV->add(cflangePV);
    GeoTransform *coldbox1_trf = new GeoTransform(coldbox1_pos);
    FTPV->add(coldbox1_trf);
    FTPV->add(coldbox1PV);
    GeoTransform *coldbox2_trf = new GeoTransform(coldbox2_pos);
    FTPV->add(coldbox2_trf);
    FTPV->add(coldbox2PV);
    GeoTransform *coldbox3_trf = new GeoTransform(coldbox3_pos);
    FTPV->add(coldbox3_trf);
    FTPV->add(coldbox3PV);

    const GeoMaterial* lar_mat = m_materialManager->getMaterial("std::LiquidArgon");
    GeoShape* lar1 = new GeoTube(0., coldbox1_Router - coldbox1_wall, coldbox1_height/2);
    GeoShape* lar2 = new GeoTube(0., hole_r, coldbox1_height);
    const GeoShape& lar = lar1->subtract((*lar2) << GeoTrf::TranslateY3D(hole_shift));
    GeoLogVol* larLV = new GeoLogVol(name + "LAr", &lar, lar_mat);
    GeoPhysVol* larPV = new GeoPhysVol(larLV);
    FTPV->add(coldbox1_trf);
    FTPV->add(larPV);

    const GeoMaterial *pigtail_mat = m_materialManager->getMaterial("LAr::FT::Pigtail");

    const double pth = (coldbox1_height + coldbox2_height + coldbox3_height) / 2;
    GeoTransform *pigtail_trf = new GeoTransform(
        GeoTrf::Translate3D(0, hole_shift,
        -wflange_height/2 - bellow_height - cflange_height - pth
    ));
    GeoShape* pigtail = new GeoTube(0., hole_r, pth);
    GeoLogVol* pigtailLV = new GeoLogVol(name + "Pigtail", pigtail, pigtail_mat);
    GeoPhysVol* pigtailPV = new GeoPhysVol(pigtailLV);
    FTPV->add(pigtail_trf);
    FTPV->add(pigtailPV);

    // total lenght should be 28.5 cm
    const double ocable_len = 10.*Gaudi::Units::cm;
    const double ocable_R = (1.1/2)*sqrt(1920*2.85)*Gaudi::Units::mm;
    const GeoMaterial* ocable_mat = m_materialManager->getMaterial("LAr::FT::Cable");
    GeoShape* ocable = new GeoTube(0., ocable_R, ocable_len / 2);
    GeoLogVol* ocableLV = new GeoLogVol("LAr::Barrel::FTCables", ocable, ocable_mat);
    GeoPhysVol* ocablePV = new GeoPhysVol(ocableLV);

    // todo: take cryostat parameters from DB
    const double r0 = 277.5*Gaudi::Units::cm // cryo warm wall outer radius
                    + wflange_height/2; // warm flange is abobe the wall
    const double r2 = r0 + wflange_height/2 + ocable_len/2;
    const double z_pos = 3170.*Gaudi::Units::mm;
    const int NCrates = 16;
    const double dphi = 4.*Gaudi::Units::deg;

    auto put1 = [&envelope](GeoPhysVol *object, double r, double phi, double z)
    {
        envelope->add(new GeoTransform(
            GeoTrf::Translate3D(r*cos(phi), r*sin(phi), z) *
            GeoTrf::RotateX3D(90.*Gaudi::Units::deg) *
            GeoTrf::RotateY3D(phi + 90.*Gaudi::Units::deg)
        ));
        if(z < 0){
            envelope->add(new GeoTransform(GeoTrf::RotateZ3D(180.*Gaudi::Units::deg)));
        }
        envelope->add(object);
    };

    auto put = [&put1, &FTPV, &ocablePV, r0, r2](double phi, double z)
    {
        put1(FTPV, r0, phi, z);
        put1(ocablePV, r2, phi, z);
    };

    for(int i = 0; i < 16; ++ i){
        const double phi = 360.*Gaudi::Units::deg / NCrates * i;
        put(phi - dphi, z_pos);
        put(phi + dphi, z_pos);
        put(phi - dphi, -z_pos);
        put(phi + dphi, -z_pos);
    }
  }

}

