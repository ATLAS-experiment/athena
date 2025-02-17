/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtDataPreparator.h"

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"

#include "MdtRegionDefiner.h"

#include "xAODTrigMuon/TrigMuonDefs.h"

#include "TrigSteeringEvent/TrigRoiDescriptor.h"

#include "MuonReadoutGeometry/MuonStation.h"

#include "GeoModelUtilities/GeoGetIds.h"
#include "MuonIdHelpers/MdtIdHelper.h"
#include <unordered_set>
namespace {
  // the tube number of a tube in a tubeLayer in encoded in the GeoSerialIdentifier (modulo maxNTubesPerLayer)
  constexpr unsigned int maxNTubesPerLayer = MdtIdHelper::maxNTubesPerLayer;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

StatusCode TrigL2MuonSA::MdtDataPreparator::initialize()
{

  // Locate RegionSelector
  ATH_CHECK( m_regionSelector.retrieve());

  ATH_CHECK( m_mdtRegionDefiner.retrieve() );
  ATH_MSG_DEBUG("Retrieved service " << m_mdtRegionDefiner);

  ATH_CHECK( m_idHelperSvc.retrieve() );
  ATH_CHECK(m_mdtPrepContainerKey.initialize());

  if(!m_isPhase2){

    const MuonGM::MuonDetectorManager* muonDetMgr=nullptr;
    ATH_CHECK( detStore()->retrieve(muonDetMgr) );
    ATH_MSG_DEBUG("Retrieved GeoModel from DetectorStore.");
  
    if(m_idHelperSvc->mdtIdHelper().stationNameIndex("BMG") != -1){     //if we have BMGs

      ATH_MSG_INFO("Processing configuration for layouts with BMG chambers.");
      m_BMGid = m_idHelperSvc->mdtIdHelper().stationNameIndex("BMG");
      for(int phi=6; phi<8; phi++) { // phi sectors - BMGs are ony in (6 aka 12) and (7 aka 14)
        for(int eta=1; eta<4; eta++) { // eta sectors - BMGs are in eta 1 to 3
          for(int side=-1; side<2; side+=2) { // side - both sides have BMGs
            if( !muonDetMgr->getMuonStation("BMG", side*eta, phi) ) continue;
            for(int roe=1; roe<=( muonDetMgr->getMuonStation("BMG", side*eta, phi) )->nMuonReadoutElements(); roe++) { // iterate on readout elemets
              const MuonGM::MdtReadoutElement* mdtRE =
              dynamic_cast<const MuonGM::MdtReadoutElement*> ( ( muonDetMgr->getMuonStation("BMG", side*eta, phi) )->getMuonReadoutElement(roe) ); // has to be an MDT
              if(mdtRE) initDeadChannels(mdtRE);
            }
          }
        }
      }
    }
  }
  return StatusCode::SUCCESS;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

StatusCode TrigL2MuonSA::MdtDataPreparator::prepareData(
                            const TrigRoiDescriptor*    p_roids,
                            const TrigL2MuonSA::RpcFitResult& rpcFitResult,
                            TrigL2MuonSA::MuonRoad&  muonRoad,
                            TrigL2MuonSA::MdtRegion& mdtRegion,
                            TrigL2MuonSA::MdtHits&   mdtHits_normal) const
{
  // define regions
  ATH_CHECK( m_mdtRegionDefiner->getMdtRegions(p_roids, rpcFitResult, muonRoad, mdtRegion) );

  ATH_CHECK( getMdtHits(p_roids, muonRoad, mdtHits_normal) );

  return StatusCode::SUCCESS;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

StatusCode TrigL2MuonSA::MdtDataPreparator::prepareData(
                            const TrigRoiDescriptor*          p_roids,
                            const TrigL2MuonSA::TgcFitResult& tgcFitResult,
                            TrigL2MuonSA::MuonRoad&           muonRoad,
                            TrigL2MuonSA::MdtRegion&          mdtRegion,
                            TrigL2MuonSA::MdtHits&            mdtHits_normal) const
{
  // define regions
  ATH_CHECK( m_mdtRegionDefiner->getMdtRegions(p_roids, tgcFitResult, muonRoad, mdtRegion) );

  ATH_CHECK( getMdtHits(p_roids, muonRoad, mdtHits_normal) );

  return StatusCode::SUCCESS;
}


// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

StatusCode TrigL2MuonSA::MdtDataPreparator::getMdtHits(
                                const TrigRoiDescriptor* p_roids,
                                TrigL2MuonSA::MuonRoad& muonRoad,
                                TrigL2MuonSA::MdtHits& mdtHits_normal) const
{

  // preload ROBs
  std::vector<uint32_t> v_robIds;
  std::vector<IdentifierHash> mdtHashList;
  const EventContext& ctx = Gaudi::Hive::currentContext();

  if (m_use_RoIBasedDataAccess) {

    ATH_MSG_DEBUG("Use RoI based data access");

    const IRoiDescriptor* iroi = (IRoiDescriptor*) p_roids;

    m_regionSelector->lookup( ctx )->HashIDList(*iroi, mdtHashList);
    ATH_MSG_DEBUG("mdtHashList.size()=" << mdtHashList.size());

    m_regionSelector->lookup( ctx )->ROBIDList(*iroi, v_robIds);

  } else {

    ATH_MSG_DEBUG("Use full data access");

    TrigRoiDescriptor fullscan_roi( true );
    m_regionSelector->lookup( ctx )->HashIDList(fullscan_roi, mdtHashList);
    ATH_MSG_DEBUG("mdtHashList.size()=" << mdtHashList.size());

    m_regionSelector->lookup( ctx )->ROBIDList(fullscan_roi, v_robIds);
  }

  ATH_CHECK( collectMdtHitsFromPrepData(ctx, mdtHashList, mdtHits_normal, muonRoad) );

  return StatusCode::SUCCESS;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

StatusCode TrigL2MuonSA::MdtDataPreparator::collectMdtHitsFromPrepData(const EventContext& ctx,
                                      const std::vector<IdentifierHash>& v_idHash,
                                      TrigL2MuonSA::MdtHits& mdtHits,
                                      const TrigL2MuonSA::MuonRoad& muonRoad) const
{

  // Get MDT container
  if (v_idHash.empty()) {
    ATH_MSG_DEBUG("Hash list is empty");
    return StatusCode::SUCCESS;
  }
  SG::ReadHandle mdtPrds{m_mdtPrepContainerKey, ctx};
  ATH_CHECK(mdtPrds.isPresent());

  // Get MDT collections
  ///// Vectors of prep data collections
  std::vector<const Muon::MdtPrepDataCollection*> mdtCols{};
  mdtCols.reserve(v_idHash.size());

  for(const IdentifierHash& id : v_idHash) {

    auto MDTcoll = mdtPrds->indexFindPtr(id);

    if( MDTcoll == nullptr ) {
      ATH_MSG_DEBUG("MDT prep data collection not found in Hash ID" << (int)id);
      continue;
    }

    if( MDTcoll->size() == 0 ) {
      ATH_MSG_DEBUG("MDT prep data collection is empty in Hash ID" << (int)id);
      continue;
    }

    mdtCols.push_back(MDTcoll);

    ATH_MSG_DEBUG("Selected Mdt Collection: "
          << m_idHelperSvc->toStringChamber(MDTcoll->identify())
          << " with size " << MDTcoll->size()
          << "in Hash ID" << (int)id);
  }

  for( const Muon::MdtPrepDataCollection* mdtCol : mdtCols ){

    mdtHits.reserve( mdtHits.size() + mdtCol->size() );
    for( const Muon::MdtPrepData* mdt : *mdtCol ) {

      const MuonGM::MdtReadoutElement* mdtReadout = mdt->detectorElement();

      const MuonGM::MuonStation* muonStation = mdtReadout->parentMuonStation();

      int StationPhi = mdtReadout->getStationPhi();
      int StationEta = mdtReadout->getStationEta();
      int MultiLayer = mdtReadout->getMultilayer();
      double cXmid{0.}, cYmid{0.}, cAmid{0.}, cPhip{0.};

      Identifier id = mdt->identify();
      int adc       = mdt->adc();
      int drift     = mdt->tdc();

      int TubeLayers = mdtReadout->getNLayers();
      int TubeLayer = m_idHelperSvc->mdtIdHelper().tubeLayer(id);
      if(TubeLayer > TubeLayers) TubeLayer -= TubeLayers;
      int Layer = (MultiLayer-1)*TubeLayers + TubeLayer;
      int Tube = m_idHelperSvc->mdtIdHelper().tube(id);

      double OrtoRadialPos = mdtReadout->getStationS();
      std::string chamberType = mdtReadout->getStationType();
      char st = chamberType[1];

      int chamber = 0;
      if (chamberType[0]=='E') {
        /// Endcap
        if (st=='I') chamber = xAOD::L2MuonParameters::Chamber::EndcapInner;
        if (st=='M') chamber = xAOD::L2MuonParameters::Chamber::EndcapMiddle;
        if (st=='O') chamber = xAOD::L2MuonParameters::Chamber::EndcapOuter;
        if (st=='E') chamber = xAOD::L2MuonParameters::Chamber::EndcapExtra;
      } 
      else {
        /// Barrel
        if (st=='I') chamber = xAOD::L2MuonParameters::Chamber::BarrelInner;
        if (st=='M') chamber = xAOD::L2MuonParameters::Chamber::BarrelMiddle;
        if (st=='O') chamber = xAOD::L2MuonParameters::Chamber::BarrelOuter;
        if (st=='E' && chamberType[2]=='E') chamber = xAOD::L2MuonParameters::Chamber::BEE;
        if (st=='M' && chamberType[2]=='E') chamber = xAOD::L2MuonParameters::Chamber::BME;
        if (st=='M' && chamberType[2]=='G') chamber = xAOD::L2MuonParameters::Chamber::Backup;
      }

      double R = -99999., Z = -99999.;
      if(m_idHelperSvc->mdtIdHelper().stationName(id) == m_BMGid && m_DeadChannels.count(id)) {
        ATH_MSG_DEBUG("Skipping tube with identifier " << m_idHelperSvc->toString(id) );
        continue;
      }
      R = mdtReadout->center(TubeLayer, Tube).perp();
      Z = mdtReadout->center(TubeLayer, Tube).z();

      Amg::Transform3D trans = muonStation->getNominalAmdbLRSToGlobal();
      if(muonStation->endcap()==0){
        cXmid = (trans.translation()).z();
        double halfRadialThicknessOfMultilayer = muonStation->RsizeMdtStation()/2.;
        cYmid = ((trans.translation()).perp()+halfRadialThicknessOfMultilayer);
      }
      else{
        cXmid = (trans.translation()).perp();
        double halfZThicknessOfMultilayer = muonStation->ZsizeMdtStation()/2.;
        cYmid = (trans.translation()).z();
        if(cYmid>0) cYmid += halfZThicknessOfMultilayer;
        else cYmid -= halfZThicknessOfMultilayer;
      }
      cPhip = (trans.translation()).phi();

      double dphi  = 0;
      double cphi  = muonRoad.phi[chamber][0];
      if( cPhip*cphi>0 ) {
        dphi = std::abs(cPhip - cphi);
      } 
      else {
        if(std::abs(cphi) > M_PI/2.) {
          double phi1 = (cPhip>0.)? cPhip-M_PI : cPhip+M_PI;
          double phi2 = (cphi >0.)? cphi -M_PI : cphi +M_PI;
          dphi = std::abs(phi1) + std::abs(phi2);
        }
        else {
          dphi = std::abs(cPhip) + std::abs(cphi);
        }
      }

      if(muonStation->endcap()==1) R = R *std::hypot(1, std::tan(dphi));

      Amg::Vector3D OrigOfMdtInAmdbFrame =  muonStation->getBlineFixedPointInAmdbLRS() ;
      double Rmin =(trans*OrigOfMdtInAmdbFrame).perp();

      float cInCo = 1./std::cos(std::abs(std::atan(OrtoRadialPos/Rmin)));
      float cPhi0 = cPhip - std::atan(OrtoRadialPos/Rmin);
      if(cPhi0 > M_PI) cPhip -= 2*M_PI;
      if(cPhip<0. && (std::abs(M_PI+cPhip) < 0.05) ) cPhip = M_PI;

      ATH_MSG_DEBUG(" ...MDT hit Z/R/chamber/MultiLater/TubeLayer/Tube/Layer/adc/tdc = "
            << Z << "/" << R << "/" << chamber << "/" << MultiLayer << "/" << TubeLayer << "/"
            << Tube << "/" << Layer << "/" << adc << "/" << drift);

      // no residual check for the moment
      // (residual check at pattern finder)
      if(Layer!=0 && Tube !=0) {

        // create the new digit
        TrigL2MuonSA::MdtHitData tmp;
        tmp.name       = m_idHelperSvc->mdtIdHelper().stationName(id);
        tmp.StationEta = StationEta;
        tmp.StationPhi = StationPhi;
        tmp.Multilayer = MultiLayer;
        tmp.Layer      = Layer - 1;
        tmp.TubeLayer  = TubeLayer;
        tmp.Tube       = Tube;
        tmp.cYmid      = cYmid;
        tmp.cXmid      = cXmid;
        tmp.cAmid      = cAmid;
        tmp.cPhip      = cPhip;
        tmp.cInCo      = cInCo;
        tmp.cPhi0      = cPhi0;
        for(unsigned int i=0; i<4; i++) { tmp.cType[i] = chamberType[i]; }
        tmp.Z          = Z;
        tmp.R          = R;
        tmp.DriftTime  = drift;
        tmp.Adc        = adc;
        tmp.LeadingCoarseTime  = (drift>>5) & 0xfff;
        tmp.LeadingFineTime    = drift & 0x1f;
        tmp.Chamber = chamber;
        tmp.readEle = mdtReadout;
        tmp.Id = id;

        mdtHits.push_back(std::move(tmp));
      }
    } // end of MdtPrepDataCollection loop
  } // end of MdtPrepDataCollection vector loop

  return StatusCode::SUCCESS;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

void TrigL2MuonSA::MdtDataPreparator::initDeadChannels(const MuonGM::MdtReadoutElement* mydetEl) {
  PVConstLink cv = mydetEl->getMaterialGeom(); // it is "Multilayer"
  int nGrandchildren = cv->getNChildVols();
  if(nGrandchildren <= 0) return;

  std::vector<int> tubes;
  geoGetIds ([&] (int id) { tubes.push_back (id); }, cv);
  std::sort (tubes.begin(), tubes.end());

  Identifier detElId = mydetEl->identify();

  int name = m_idHelperSvc->mdtIdHelper().stationName(detElId);
  int eta = m_idHelperSvc->mdtIdHelper().stationEta(detElId);
  int phi = m_idHelperSvc->mdtIdHelper().stationPhi(detElId);
  int ml = m_idHelperSvc->mdtIdHelper().multilayer(detElId);

  std::vector<int>::iterator it = tubes.begin();
  for(int layer = 1; layer <= mydetEl->getNLayers(); layer++){
    for(int tube = 1; tube <= mydetEl->getNtubesperlayer(); tube++){
      int want_id = layer*maxNTubesPerLayer + tube;
      if (it != tubes.end() && *it == want_id) {
        ++it;
      }
      else {
        it = std::lower_bound (tubes.begin(), tubes.end(), want_id);
        if (it != tubes.end() && *it == want_id) {
          ++it;
        }
        else {
          Identifier deadTubeId = m_idHelperSvc->mdtIdHelper().channelID( name, eta, phi, ml, layer, tube );
          m_DeadChannels.insert(deadTubeId);
          ATH_MSG_VERBOSE("adding dead tube (" << tube  << "), layer(" <<  layer
                          << "), phi(" << phi << "), eta(" << eta << "), name(" << name
                          << "), multilayerId(" << ml << ") and identifier " << deadTubeId <<" .");
        }
      }
    }
  }
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
