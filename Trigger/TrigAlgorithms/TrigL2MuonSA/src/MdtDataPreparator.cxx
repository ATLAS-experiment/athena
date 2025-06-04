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
                            TrigL2MuonSA::MdtHits&   mdtHits) const
{
  // define regions
  ATH_CHECK( m_mdtRegionDefiner->getMdtRegions(p_roids, rpcFitResult, muonRoad, mdtRegion) );

  ATH_CHECK( getMdtHits(p_roids, muonRoad, mdtHits) );

  return StatusCode::SUCCESS;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

StatusCode TrigL2MuonSA::MdtDataPreparator::prepareData(
                            const TrigRoiDescriptor*          p_roids,
                            const TrigL2MuonSA::TgcFitResult& tgcFitResult,
                            TrigL2MuonSA::MuonRoad&           muonRoad,
                            TrigL2MuonSA::MdtRegion&          mdtRegion,
                            TrigL2MuonSA::MdtHits&            mdtHits) const
{
  // define regions
  ATH_CHECK( m_mdtRegionDefiner->getMdtRegions(p_roids, tgcFitResult, muonRoad, mdtRegion) );

  ATH_CHECK( getMdtHits(p_roids, muonRoad, mdtHits) );

  return StatusCode::SUCCESS;
}


// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

StatusCode TrigL2MuonSA::MdtDataPreparator::getMdtHits(
                                const TrigRoiDescriptor* p_roids,
                                TrigL2MuonSA::MuonRoad& muonRoad,
                                TrigL2MuonSA::MdtHits& mdtHits) const
{
  std::vector<IdentifierHash> mdtHashList;
  const EventContext& ctx = Gaudi::Hive::currentContext();

  if (m_use_RoIBasedDataAccess) {

    ATH_MSG_DEBUG("Use RoI based data access");

    m_regionSelector->lookup( ctx )->HashIDList(*p_roids, mdtHashList);
    ATH_MSG_DEBUG("mdtHashList.size()=" << mdtHashList.size());

  } else {

    ATH_MSG_DEBUG("Use full data access");

    TrigRoiDescriptor fullscan_roi( true );
    m_regionSelector->lookup( ctx )->HashIDList(fullscan_roi, mdtHashList);
    ATH_MSG_DEBUG("mdtHashList.size()=" << mdtHashList.size());

  }

  ATH_CHECK( collectMdtHitsFromPrepData(ctx, mdtHashList, mdtHits, muonRoad) );

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

  for(const IdentifierHash& id : v_idHash) {

    // Get MDT collections
    auto mdtCol = mdtPrds->indexFindPtr(id);

    if( mdtCol == nullptr ) {
        ATH_MSG_DEBUG("MDT prep data collection not found in Hash ID" << (int)id);
        continue;
    }
    if( mdtCol->size() == 0 ) {
        ATH_MSG_DEBUG("MDT prep data collection is empty in Hash ID" << (int)id);
        continue;
    }

    ATH_MSG_DEBUG("Selected Mdt Collection: "
          << m_idHelperSvc->toStringChamber(mdtCol->identify())
          << " with size " << mdtCol->size()
          << "in Hash ID" << (int)id);

    mdtHits.reserve( mdtHits.size() + mdtCol->size() );

    for( const Muon::MdtPrepData* mdt : *mdtCol ) {

        Identifier id = mdt->identify();

        if(m_idHelperSvc->mdtIdHelper().stationName(id) == m_BMGid && m_DeadChannels.count(id)) {
            ATH_MSG_DEBUG("Skipping tube with identifier " << m_idHelperSvc->toString(id) );
            continue;
        }

        const MuonGM::MdtReadoutElement* mdtReadout = mdt->detectorElement();
        const MuonGM::MuonStation* muonStation = mdtReadout->parentMuonStation();

        TrigL2MuonSA::MdtHitData tmp;
        tmp.Id = id;

        int TubeLayers = mdtReadout->getNLayers();
        tmp.TubeLayer = m_idHelperSvc->mdtIdHelper().tubeLayer(id);
        if(tmp.TubeLayer > TubeLayers) tmp.TubeLayer -= TubeLayers;
        tmp.Tube = m_idHelperSvc->mdtIdHelper().tube(id);
        tmp.Multilayer = mdtReadout->getMultilayer();
        int Layer = (tmp.Multilayer-1)*TubeLayers + tmp.TubeLayer;
        tmp.Layer = Layer - 1;   

        if(Layer==0 or tmp.Tube ==0) continue;

        int drift = mdt->tdc();
        tmp.DriftTime = drift;
        tmp.LeadingCoarseTime  = (drift>>5) & 0xfff;
        tmp.LeadingFineTime    = drift & 0x1f;
        tmp.Adc = mdt->adc();

        tmp.name       = m_idHelperSvc->mdtIdHelper().stationName(id);
        tmp.StationEta = mdtReadout->getStationEta();
        tmp.StationPhi = mdtReadout->getStationPhi();
        
        std::string chamberType = mdtReadout->getStationType();
        std::copy_n(chamberType.begin(), std::min<size_t>(4, chamberType.size()), tmp.cType.begin());
        tmp.readEle = mdtReadout;

        int& chamber {tmp.Chamber};
        char st = chamberType[1];
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

        double &cXmid{tmp.cXmid}, &cYmid{tmp.cYmid}, &cPhip{tmp.cPhip};
        Amg::Transform3D trans = muonStation->getNominalAmdbLRSToGlobal();
        if(!muonStation->endcap()){
            cXmid = (trans.translation()).z();
            cYmid = ((trans.translation()).perp() + muonStation->RsizeMdtStation()/2.);
        }else{
            cXmid = (trans.translation()).perp();
            cYmid = (trans.translation()).z();
            if(cYmid>0) cYmid += muonStation->RsizeMdtStation()/2.;
            else cYmid -= muonStation->RsizeMdtStation()/2.;
        }
        cPhip = (trans.translation()).phi();

        double &R {tmp.R}, &Z {tmp.Z};
        R = -99999.; Z = -99999.;
        R = mdtReadout->center(tmp.TubeLayer, tmp.Tube).perp();
        Z = mdtReadout->center(tmp.TubeLayer, tmp.Tube).z();

        double dphi  = 0;
        double cphi  = muonRoad.phi[chamber][0];
        if( cPhip*cphi>0 ) {
            dphi = std::abs(cPhip - cphi);
        } else {
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

        double Rmin = (trans * muonStation->getBlineFixedPointInAmdbLRS()).perp();
        double OrtoRadialPos = mdtReadout->getStationS();
        tmp.cInCo = 1./std::cos(std::atan(OrtoRadialPos/Rmin));
        tmp.cPhi0 = cPhip - std::atan(OrtoRadialPos/Rmin);
        if(cPhip<0. && (std::abs(M_PI+cPhip) < 0.05) ) cPhip = M_PI;

        ATH_MSG_DEBUG(" ...MDT hit Z/R/chamber/MultiLater/TubeLayer/Tube/Layer/adc/tdc = "
            << Z << "/" << R << "/" << chamber << "/" << tmp.Multilayer << "/" << tmp.TubeLayer << "/"
            << tmp.Tube << "/" << Layer << "/" << tmp.Adc << "/" << drift);

        mdtHits.push_back(std::move(tmp));
        
    } // end of MdtPrepDataCollection loop
  } // end of hashList loop

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
