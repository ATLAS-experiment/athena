/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Package : MMRawDataMonAlg
// Authors:   M. Biglietti, E. Rossi (Roma Tre)
//
// DESCRIPTION:
// Subject: MM-->Offline Muon Data Quality
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/MuonStation.h"
#include "MuonReadoutGeometry/MMReadoutElement.h"
#include "MuonDQAUtils/MuonChamberNameConverter.h"
#include "MuonDQAUtils/MuonChambersRange.h"
#include "MuonCalibIdentifier/MuonFixedId.h"

#include "MMRawDataMonAlg.h"

#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackingPrimitives.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonRIO_OnTrack/MMClusterOnTrack.h"
#include "AthenaMonitoring/AthenaMonManager.h"
#include "MuonPrepRawData/MMPrepData.h"
#include "MuonSegment/MuonSegment.h"
#include <stdexcept>


namespace {

	//1e=1.6X10-4 fC
	static constexpr double conversion_charge=1.6E-04;

	static const std::array<std::string,2> MM_Side = {"CSide", "ASide"};
	static const std::array<std::string,2> EtaSector = {"1", "2"};

	struct MMOverviewHistogramStruct {
	  std::vector<int> statEta_strip;
	  std::vector<float> charge_all;
	  std::vector<int> strp_times;
	  std::vector<float> cl_times;
	  std::vector<int> strip_number;
	  std::vector<int> numberofstrips_percluster;
	  std::vector<float> R_mon;
	  std::vector<float> z_mon;
	  std::vector<float> x_mon;
	  std::vector<float> y_mon;

	  std::vector<int> stationPhi_ASide_ontrack;
	  std::vector<int> stationPhi_CSide_ontrack;
	  std::vector<int> sector_ASide_ontrack;
	  std::vector<int> sector_CSide_ontrack;
	  std::vector<int> stationPhi_CSide;
	  std::vector<int> stationPhi_ASide;
	  std::vector<int> sector_CSide;
	  std::vector<int> sector_ASide;
	  std::vector<int> stationPhi_ASide_onseg;
	  std::vector<int> stationPhi_CSide_onseg;
	  std::vector<int> sector_ASide_onseg;
	  std::vector<int> sector_CSide_onseg;

	};

	struct MMByPhiStruct {
	  	std::vector<int> sector_lb;
	  	std::vector<int> sector_lb_ontrack;
	  	std::vector<int> sector_lb_onseg;
	};

	struct MMSummaryHistogramStruct {
	  std::vector<int> cl_size;
	  std::vector<int> pcb;
	  std::vector<int> pcb_strip;
	  std::vector<int> strip_number;
	  std::vector<int> sector_strip;
	  std::vector<float> charge;
	  std::vector<int> strp_times;
	  std::vector<float> cl_times;
	  std::vector<float> x_ontrack;
	  std::vector<float> y_ontrack;
	  std::vector<float> residuals;
	};

	struct MMEfficiencyHistogramStruct {
		std::vector<int> num;
		std::vector<int> nGaps;
	};
}

/////////////////////////////////////////////////////////////////////////////
// *********************************************************************
// Public Methods
// ********************************************************************* 

MMRawDataMonAlg::MMRawDataMonAlg( const std::string& name, ISvcLocator* pSvcLocator ) : 
	AthMonitorAlgorithm(name,pSvcLocator)
{ }

/*---------------------------------------------------------*/
StatusCode MMRawDataMonAlg::initialize()
/*---------------------------------------------------------*/
{
	//init message stream
	ATH_MSG_DEBUG("initialize MMRawDataMonAlg");
	ATH_MSG_DEBUG("******************");
	ATH_MSG_DEBUG("doMMESD: " << m_doMMESD );
	ATH_MSG_DEBUG("******************");

	ATH_CHECK(AthMonitorAlgorithm::initialize());
	ATH_CHECK(m_DetectorManagerKey.initialize());
	ATH_CHECK(m_idHelperSvc.retrieve());

	ATH_MSG_INFO(" Found the MuonIdHelperSvc ");
	ATH_CHECK(m_muonKey.initialize());
	ATH_CHECK(m_MMContainerKey.initialize());
	ATH_CHECK(m_meTrkKey.initialize());
      	ATH_CHECK(m_segm_type.initialize());
	ATH_CHECK(m_mmtpRdoKey.initialize());

	ATH_MSG_DEBUG(" end of initialize " );
	ATH_MSG_INFO("MMRawDataMonAlg initialization DONE " );

	return StatusCode::SUCCESS;
} 

StatusCode MMRawDataMonAlg::fillHistograms(const EventContext& ctx) const
{
	int lumiblock = -1;
	lumiblock = GetEventInfo(ctx)->lumiBlock();
	ATH_MSG_DEBUG("MMRawDataMonAlg::MM RawData Monitoring Histograms being filled" );

	SG::ReadHandle<Muon::MMPrepDataContainer> mm_container(m_MMContainerKey,ctx);
	ATH_MSG_DEBUG("****** mmContainer->size() : " << mm_container->size());

	if(m_doMMESD) {
		MMOverviewHistogramStruct overviewPlots;
		MMSummaryHistogramStruct summaryPlots[2][16][2][2][4];
		MMByPhiStruct occupancyPlots[16][2];

		//loop in MMPrepDataContainer
		for(const Muon::MMPrepDataCollection* coll : *mm_container) {
		  for(const Muon::MMPrepData* prd : *coll) {
		    ATH_CHECK(fillMMOverviewVects(prd, overviewPlots, occupancyPlots));
				ATH_CHECK(fillMMSummaryVects(prd, summaryPlots));
				ATH_CHECK(fillMMHistograms(prd));
		  }
		}
		
		if(m_do_mm_overview) fillMMOverviewHistograms(overviewPlots, occupancyPlots, lumiblock);
		
		ATH_CHECK(fillMMSummaryHistograms(summaryPlots));
		SG::ReadHandle<xAOD::TrackParticleContainer> meTPContainer{m_meTrkKey,ctx};
		if (!meTPContainer.isValid()) {
		  ATH_MSG_FATAL("Nope. Could not retrieve "<<m_meTrkKey.fullKey());
		  return StatusCode::FAILURE;
		}
		clusterFromTrack(meTPContainer.cptr(),lumiblock);
		MMEfficiency(meTPContainer.cptr());


		
		//trigger
		SG::ReadHandle<xAOD::NSWMMTPRDOContainer> rdos =  SG::ReadHandle<xAOD::NSWMMTPRDOContainer>{m_mmtpRdoKey, ctx};
		if (not rdos.isValid()) {
		  ATH_MSG_INFO("NSW MMTP failed. Skipping");
		  // return StatusCode::SUCCESS;
		} else fillMMTrigger(rdos.cptr(),lumiblock);
		
		SG::ReadHandle<Trk::SegmentCollection> segms(m_segm_type, ctx);

		if (!segms.isValid()) {
		  ATH_MSG_INFO("evtStore() does not contain MM segms Collection with name " << m_segm_type);
		  //		  return StatusCode::FAILURE;
		}else
		  clusterFromSegments(segms.cptr(),lumiblock);
	}


	return StatusCode::SUCCESS;
}

StatusCode MMRawDataMonAlg::fillMMOverviewVects( const Muon::MMPrepData* prd, MMOverviewHistogramStruct& vects, MMByPhiStruct (&occupancyPlots)[16][2] ) const
{
	Identifier Id = prd->identify();
	const std::vector<Identifier>& stripIds = prd->rdoList();
	unsigned int nStrips = stripIds.size(); // number of strips in this cluster (cluster size)
	const std::vector<uint16_t>& stripNumbers = prd->stripNumbers();

	std::string stName = m_idHelperSvc->mmIdHelper().stationNameString(m_idHelperSvc->mmIdHelper().stationName(Id));
	int gas_gap 	   = m_idHelperSvc->mmIdHelper().gasGap(Id);
	int stationEta 	   = m_idHelperSvc->mmIdHelper().stationEta(Id);
	int stationPhi     = m_idHelperSvc->mmIdHelper().stationPhi(Id);
	int multiplet 	   = m_idHelperSvc->mmIdHelper().multilayer(Id);
	int channel        = m_idHelperSvc->mmIdHelper().channel(Id);

	// Returns the charge (number of electrons) converted in fC
	float charge = prd->charge()*conversion_charge;
	// Returns the times of each strip (in ns)
	std::vector<short int> strip_times = prd->stripTimes();

	Amg::Vector3D pos = prd->globalPosition();
	float R = std::hypot(pos.x(),pos.y());

	// MM gaps are back to back, so the direction of the drift (time) is different for the even and odd gaps -> flip for the even gaps

	vects.charge_all.push_back(charge);
	vects.numberofstrips_percluster.push_back(nStrips);
	vects.x_mon.push_back(pos.x());
	vects.y_mon.push_back(pos.y());
	vects.z_mon.push_back(pos.z());
	vects.R_mon.push_back(R);

	// 16 phi sectors, 8 stationPhi times 2 stName, MMS and MML
	int sectorPhi = get_sectorPhi_from_stationPhi_stName(stationPhi,stName);

	// Occupancy plots with PCB granularity further divided for each eta sector: -2, -1, 1, 2
	// CSide and ASide
	int iside = (stationEta>0) ? 1 : 0;

	auto& thisSect = occupancyPlots[sectorPhi-1][iside];
	const int gap_offset=4;
	int gas_gap8 = (multiplet==1) ?  gas_gap :  gas_gap + gap_offset; 
	int FEB = get_FEB_from_channel(channel, stationEta);
	
	int bin = get_bin_for_feb_occ(gas_gap8,FEB);

	thisSect.sector_lb.push_back(bin);

	if(stationEta<0) {
	  vects.sector_CSide.push_back(bin);
	  vects.stationPhi_CSide.push_back(sectorPhi);
	} else {
	  vects.sector_ASide.push_back(bin);
	  vects.stationPhi_ASide.push_back(sectorPhi);
	}

	// loop on each strip
	int sIdx = 0; // index-counter for the vector of Id's
	float cluster_time = 0;
	for(const Identifier& id: stripIds) {
		
		std::string stName_strip = m_idHelperSvc->mmIdHelper().stationNameString(m_idHelperSvc->mmIdHelper().stationName(id));
		int stationEta_strip 	 = m_idHelperSvc->mmIdHelper().stationEta(id);
		vects.statEta_strip.push_back(stationEta_strip);
		vects.strip_number.push_back(stripNumbers[sIdx]);
		vects.strp_times.push_back(strip_times.at(sIdx));
		cluster_time += strip_times.at(sIdx);
		++sIdx;
	}
	cluster_time /= strip_times.size();
	vects.cl_times.push_back(cluster_time);

	return StatusCode::SUCCESS;
}

void MMRawDataMonAlg::fillMMOverviewHistograms( const MMOverviewHistogramStruct& vects,  MMByPhiStruct (&occupancyPlots)[16][2], int lb ) const 
{
	auto charge_all = Monitored::Collection("charge_all", vects.charge_all);
	auto numberofstrips_percluster = Monitored::Collection("numberofstrips_percluster", vects.numberofstrips_percluster);
	fill("mmMonitor", charge_all, numberofstrips_percluster);

	auto strip_times = Monitored::Collection("strip_times", vects.strp_times);
	auto cluster_times = Monitored::Collection("cluster_times", vects.cl_times);
	auto strip_number = Monitored::Collection("strip_number", vects.strip_number);
	auto statEta_strip = Monitored::Collection("statEta_strip", vects.statEta_strip);
	fill("mmMonitor", strip_times, cluster_times, strip_number, statEta_strip);

	auto x_mon = Monitored::Collection("x_mon", vects.x_mon);
	auto y_mon = Monitored::Collection("y_mon", vects.y_mon);
	auto z_mon = Monitored::Collection("z_mon", vects.z_mon);
	auto R_mon = Monitored::Collection("R_mon", vects.R_mon);
	fill("mmMonitor", x_mon, y_mon, z_mon, R_mon);

	auto lb_mon = Monitored::Scalar<int>("lb_mon", lb);

	for(int statPhi=0; statPhi<16; ++statPhi) {
		for(int iside=0; iside<2; ++iside) {
			auto& occ_lb = occupancyPlots[statPhi][iside];
			auto sector_lb = Monitored::Collection("sector_lb_"+MM_Side[iside]+"_phi"+std::to_string(statPhi+1),occ_lb.sector_lb);
			std::string MM_sideGroup = "MM_sideGroup" + MM_Side[iside];
			fill(MM_sideGroup, lb_mon, sector_lb);
		}
	}
	auto sector_CSide 	   = Monitored::Collection("sector_CSide",vects.sector_CSide);
	auto sector_ASide 	   = Monitored::Collection("sector_ASide",vects.sector_ASide);
	auto stationPhi_CSide = Monitored::Collection("stationPhi_CSide",vects.stationPhi_CSide);
	auto stationPhi_ASide = Monitored::Collection("stationPhi_ASide",vects.stationPhi_ASide);

	fill("mmMonitor", sector_CSide, sector_ASide, stationPhi_CSide, stationPhi_ASide );
}

StatusCode MMRawDataMonAlg::fillMMSummaryVects( const Muon::MMPrepData* prd, MMSummaryHistogramStruct (&vects)[2][16][2][2][4]) const
{
  Identifier Id = prd->identify();
  const std::vector<Identifier>& stripIds = prd->rdoList();
  
  std::string stName   	= m_idHelperSvc->mmIdHelper().stationNameString(m_idHelperSvc->mmIdHelper().stationName(Id));
  int thisStationEta      = m_idHelperSvc->mmIdHelper().stationEta(Id);
  int thisStationPhi       = m_idHelperSvc->mmIdHelper().stationPhi(Id);
  int thisMultiplet        = m_idHelperSvc->mmIdHelper().multilayer(Id);
  int thisGasgap          = m_idHelperSvc->mmIdHelper().gasGap(Id);
  int ch             = m_idHelperSvc->mmIdHelper().channel(Id);
  float thisCharge=prd->charge()*conversion_charge;
  std::vector<short int> strip_times = prd->stripTimes();

    
    int phi=get_sectorPhi_from_stationPhi_stName(thisStationPhi ,stName);

    // CSide and ASide
    int iside = (thisStationEta>0) ? 1 : 0;

    // 2 eta sectors depending on Eta=+-1 (0) and +-2 (1)
    int sectorEta=get_sectorEta_from_stationEta(thisStationEta);
    unsigned int csize = stripIds.size();
    int PCB = get_PCB_from_channel(ch);
    auto& Vectors = vects[iside][phi-1][sectorEta][thisMultiplet-1][thisGasgap-1];    

    // loop on strips
    int sIdx = 0;
    const std::vector<uint16_t>& stripNumbers=prd->stripNumbers();
    float cluster_time = 0;
    for ( const Identifier& id : stripIds) {
      
      int stationEta       = m_idHelperSvc->mmIdHelper().stationEta(id);
      int gas_gap          = m_idHelperSvc->mmIdHelper().gasGap(Id);
      int multiplet        = m_idHelperSvc->mmIdHelper().multilayer(Id);
      // Filling Vectors for both sides, considering each strip
      if(m_doDetailedHists){
	Vectors.strp_times.push_back(strip_times.at(sIdx));
	cluster_time += strip_times.at(sIdx);
      }
      Vectors.strip_number.push_back(stripNumbers[sIdx]);
      Vectors.pcb_strip.push_back( get_PCB_from_channel(stripNumbers[sIdx]));
      ++sIdx;
      if(iside==1)    Vectors.sector_strip.push_back(get_bin_for_occ_ASide_hist(stationEta,multiplet,gas_gap));
      if(iside==0)    Vectors.sector_strip.push_back(get_bin_for_occ_CSide_hist(stationEta,multiplet,gas_gap));
    }
    if(m_doDetailedHists){
      Vectors.cl_size.push_back(csize);
      Vectors.pcb.push_back(PCB);
      cluster_time /= strip_times.size();
      Vectors.cl_times.push_back(cluster_time);
      Vectors.charge.push_back(thisCharge);
    }      
    return StatusCode::SUCCESS;
}

StatusCode MMRawDataMonAlg::fillMMSummaryHistograms( const MMSummaryHistogramStruct (&vects)[2][16][2][2][4]) const {

	for(int iside=0; iside<2; ++iside) {
		std::string MM_sideGroup = "MM_sideGroup" + MM_Side[iside];



		for(int statPhi=0; statPhi<16; ++statPhi) {
		  for(int multiplet=0; multiplet<2; ++multiplet) {


		    for(int statEta=0; statEta<2; ++statEta) {

		      for(int gas_gap=0; gas_gap<4; ++gas_gap) {
						auto& Vectors = vects[iside][statPhi][statEta][multiplet][gas_gap];
 						auto sector_strip = Monitored::Collection("sector_strip_" + MM_Side[iside] + "_phi" + std::to_string(statPhi+1), Vectors.sector_strip);
						auto strip_number = Monitored::Collection("strip_number_" + MM_Side[iside] + "_phi" + std::to_string(statPhi+1), Vectors.strip_number);
						if(m_doDetailedHists){
						  if(!Vectors.strip_number.empty())
						    {
						      auto cluster_size = Monitored::Collection("cluster_size_" + MM_Side[iside] + "_phi" + std::to_string(statPhi+1) + "_eta" + std::to_string(statEta+1) + "_ml" + std::to_string(multiplet+1) + "_gap" + std::to_string(gas_gap+1), Vectors.cl_size);
						      auto strip_times = Monitored::Collection("strp_time_" + MM_Side[iside] + "_phi" + std::to_string(statPhi+1) + "_eta" + std::to_string(statEta+1) + "_ml" + std::to_string(multiplet+1) + "_gap" + std::to_string(gas_gap+1), Vectors.strp_times);
						      auto cluster_time = Monitored::Collection("cluster_time_" + MM_Side[iside] + "_phi" + std::to_string(statPhi+1) + "_eta" + std::to_string(statEta+1) + "_ml" + std::to_string(multiplet+1) + "_gap" + std::to_string(gas_gap+1), Vectors.cl_times);
						      auto charge_perPCB = Monitored::Collection("charge_perPCB_" + MM_Side[iside] + "_phi" + std::to_string(statPhi+1) + "_eta" + std::to_string(statEta+1) + "_ml" + std::to_string(multiplet+1) + "_gap" + std::to_string(gas_gap+1), Vectors.charge);
						      auto charge_perlayer = Monitored::Collection("charge_perlayer_" + MM_Side[iside] + "_phi" + std::to_string(statPhi+1) + "_ml" + std::to_string(multiplet+1) + "_gap" + std::to_string(gas_gap+1), Vectors.charge);
						      auto cluster_size_perlayer = Monitored::Collection("cluster_size_perlayer_" + MM_Side[iside] + "_phi" + std::to_string(statPhi+1) + "_ml" + std::to_string(multiplet+1) + "_gap" + std::to_string(gas_gap+1), Vectors.cl_size);
						      auto pcb_mon = Monitored::Collection("pcb_mon_" + MM_Side[iside] + "_phi" + std::to_string(statPhi+1) + "_eta" + std::to_string(statEta+1) + "_ml" + std::to_string(multiplet+1) + "_gap" + std::to_string(gas_gap+1), Vectors.pcb);
						      auto pcb_strip_mon = Monitored::Collection("pcb_strip_mon_" + MM_Side[iside] + "_phi" + std::to_string(statPhi+1) + "_eta" + std::to_string(statEta+1) + "_ml" + std::to_string(multiplet+1) + "_gap" + std::to_string(gas_gap+1), Vectors.pcb_strip);
						      fill(MM_sideGroup, cluster_size, strip_times, cluster_time, charge_perPCB, pcb_mon, pcb_strip_mon, charge_perlayer, cluster_size_perlayer);
						    }
						}
					
						fill(MM_sideGroup, strip_number, sector_strip);
					}
				}
			}
		}
	}

	return StatusCode::SUCCESS;
}

StatusCode MMRawDataMonAlg::fillMMHistograms( const Muon::MMPrepData* ) const{
  return StatusCode::SUCCESS;
}

StatusCode  MMRawDataMonAlg::fillMMTrigger(const xAOD::NSWMMTPRDOContainer* mmtp, const int lb) const{

    auto lb_tri=Monitored::Scalar<int>("lb_tri",lb);

    for (const auto* rdo : *mmtp) {
       auto sourceID = rdo->sourceID();
       auto moduleID = rdo->moduleID();
       int s_side = (sourceID >> 16) == 107 ? 1:-1;                  // 1 for A; -1 for C
       auto side = Monitored::Scalar<int>("tri_side", s_side);
       int iside= s_side>0 ? s_side : 0;			    
       uint s_sector = (moduleID & 0xF) + 1;                  //0-15 --> 1-16
       int oct = (int)((s_sector-1)/2.);
       float sector_pos=(45/180.)*M_PI*oct; //large
       if(s_sector%2==0 ) sector_pos=(45*oct+22.5)*M_PI/180.; //small
       if(sector_pos>M_PI) sector_pos=sector_pos-2*M_PI;

       auto trig_sector = Monitored::Scalar<int>("trig_sector", s_sector*s_side);

       auto event_bcid=rdo->ROD_BCID();

       auto bcid=Monitored::Scalar<int>("bcid",event_bcid);

       std::vector<short unsigned int> art_bcids = rdo->art_BCID();
       std::vector<unsigned char> layers=rdo->art_layer();
       auto channels=rdo->art_channel();

       fill("mmTrigger", trig_sector, lb_tri);

       for (int i=0; i< rdo->art_BCID().size(); i++ ){
	 auto art_layer=static_cast<unsigned int>(layers[i]);
	 auto art_channel = Monitored::Scalar<int>("art_channel", channels[i]);
	 auto art_sector_layer = Monitored::Scalar<int>("art_sector_layer", s_side*8*(s_sector-1)+art_layer);

	 const int rollover=3564;
	 int art_bc=art_bcids[i];
	 int relative = art_bc - event_bcid;;
	 if (relative > rollover / 2) {
	   relative -= rollover;
	 } else if (relative <= -rollover / 2) {
	   relative += rollover;
	 }
	 if (relative > (rollover-2048) / 2) {
	   relative -= (rollover-2048);
	 }
	 auto art_deltaBC=Monitored::Scalar<int>("art_deltaBC",relative);
	 auto art_deltaBC_perSector=Monitored::Scalar<int>("art_deltaBC_"+MM_Side[iside]+"_s"+std::to_string(s_sector),relative);
	 auto art_bc_mon=Monitored::Scalar<int>("art_bc",art_bc);
	 
	 fill("mmTrigger", art_channel, trig_sector, art_sector_layer, art_deltaBC, art_bc_mon, bcid, lb_tri, art_deltaBC_perSector);
       }

       auto bcids =rdo->trig_BCID();
       auto dthetas=rdo->trig_dTheta();
       auto rids=rdo->trig_ROI_rID();
       auto phiids=rdo->trig_ROI_phiID() ;


       std::unordered_map<int, int> NROIPerBC;
       for (int bc : bcids) 	 NROIPerBC[bc]++;
       for (const auto& [value, count] : NROIPerBC) {
	 auto nROIPerBC=Monitored::Scalar<int>("nROIPerBC",count);
	 fill("mmTrigger_roi", nROIPerBC, trig_sector);
       }

       int nROI=rdo->trig_BCID().size();
       for (int i=0; i< nROI; i++ ){
	 auto phiID = (phiids[i] & 0b11111) * ((phiids[i] >> 5) ? 1 : -1);
	 int sign = phiID > 0 ? 1 : -1;
	 auto phi_conv = (phiID-0.5*sign)*(16./31.)*M_PI/180. + sector_pos; 
	 if(phi_conv> M_PI)phi_conv = phi_conv - 2*M_PI;

	 const float z_ref=7824.46;
	 const float r_step=(5000-900)/256.;
	 auto rID = static_cast<unsigned int>(rids[i]);
	 float r_conv=r_step*rID+900;
	 float eta_conv=-log(0.5*atan(r_conv/z_ref))*s_side;
	 //	 auto dTheta=Monitored::Scalar<float>("dTheta_roi",static_cast<float>(dthetas[i])); 
	 auto deltaBC=   Monitored::Scalar<int>("deltaBC", bcids[i]-event_bcid);
	 auto deltaBC_perSector=   Monitored::Scalar<int>("deltaBC_"+MM_Side[iside]+"_s"+std::to_string(s_sector), bcids[i]-event_bcid);
	 auto rid=Monitored::Scalar<int>("rid",rID);
	 auto phiid=Monitored::Scalar<int>("phiid",phiID);
	 auto rid_sector=Monitored::Scalar<int>("rid_"+MM_Side[iside]+"_s"+std::to_string(s_sector),rID);
	 auto phiid_sector=Monitored::Scalar<int>("phiid_"+MM_Side[iside]+"_s"+std::to_string(s_sector),phiID);
	 auto r_roi=Monitored::Scalar<float>("r_roi",r_conv);
	 auto phi_roi=Monitored::Scalar<float>("phi_roi",phi_conv);
	 auto eta_roi=Monitored::Scalar<float>("eta_roi",eta_conv);
	 auto x_roi_sideA=Monitored::Scalar<float>("x_roi_sideA", r_conv*cos(phi_roi));
	 auto y_roi_sideA=Monitored::Scalar<float>("y_roi_sideA", r_conv*sin(phi_roi));
	 auto x_roi_sideC=Monitored::Scalar<float>("x_roi_sideC", r_conv*cos(phi_roi) );
	 auto y_roi_sideC=Monitored::Scalar<float>("y_roi_sideC", r_conv*sin(phi_roi));

	 fill("mmTrigger_roi", deltaBC, phiid, rid, trig_sector, phi_roi, eta_roi,r_roi, lb_tri, rid_sector, phiid_sector, deltaBC_perSector);
	 if(s_side>0) fill("mmTrigger_roi", x_roi_sideA, y_roi_sideA);
	 if(s_side<0) fill("mmTrigger_roi", x_roi_sideC, y_roi_sideC);

     }
  }
  
  return StatusCode::SUCCESS;
  
}

void MMRawDataMonAlg::clusterFromTrack(const xAOD::TrackParticleContainer*  muonContainer, int lb) const
{
	MMSummaryHistogramStruct summaryPlots[2][2][4]; // side, multilayer, gas gap
	MMSummaryHistogramStruct summaryPlots_full[2][16][2][2][4]; // side, phi, eta, multilayer, gas gap
	MMSummaryHistogramStruct sumPlots[2][16][2][2][4]; // side, phi, eta, multilayer, gas gap
	MMOverviewHistogramStruct overviewPlots;
	MMByPhiStruct occupancyPlots[16][2]; // sector, side
	int ntrk=0;
	for(const xAOD::TrackParticle* meTP : *muonContainer) {

		if(!meTP) continue;
		auto eta_trk = Monitored::Scalar<float>("eta_trk", meTP->eta());
		auto phi_trk = Monitored::Scalar<float>("phi_trk", meTP->phi());
		auto pt_trk = Monitored::Scalar<float>("pt_trk", meTP->pt()/1000.);

		//retrieve the original track
		const Trk::Track* meTrack = meTP->track();

		if(!meTrack) continue;
		
		// get the vector of measurements on track
		const DataVector<const Trk::MeasurementBase>* meas = meTrack->measurementsOnTrack();
		bool isMM=false;
		for(const Trk::MeasurementBase* it : *meas) {
			const Trk::RIO_OnTrack* rot = dynamic_cast<const Trk::RIO_OnTrack*>(it);
			if(!rot) continue;
			Identifier rot_id = rot->identify();
			if(!m_idHelperSvc->isMM(rot_id)) continue;
			isMM=true;
			const Muon::MMClusterOnTrack* cluster = dynamic_cast<const Muon::MMClusterOnTrack*>(rot);
			if(!cluster) continue;

			std::string stName = m_idHelperSvc->mmIdHelper().stationNameString(m_idHelperSvc->mmIdHelper().stationName(rot_id));
			int stEta          = m_idHelperSvc->mmIdHelper().stationEta(rot_id);
			int stPhi          = m_idHelperSvc->mmIdHelper().stationPhi(rot_id);
			int multi          = m_idHelperSvc->mmIdHelper().multilayer(rot_id);
			int gap            = m_idHelperSvc->mmIdHelper().gasGap(rot_id);
			int ch             = m_idHelperSvc->mmIdHelper().channel(rot_id);
			
			// MMS and MML phi sectors
			//				int phisec = (stNumber%2==0) ? 1 : 0;
			int sectorPhi = get_sectorPhi_from_stationPhi_stName(stPhi,stName); // 1->16
			int PCB = get_PCB_from_channel(ch);
			int iside = (stEta > 0) ? 1 : 0;
			auto& vects = overviewPlots;
			auto& thisSect = occupancyPlots[sectorPhi-1][iside];
			

			const Muon::MMPrepData* prd = cluster->prepRawData();
			const std::vector<Identifier>& stripIds = prd->rdoList();
			unsigned int csize = stripIds.size();
			const std::vector<uint16_t>& stripNumbers = prd->stripNumbers();
			float charge = prd->charge()*conversion_charge;
			std::vector<short int> s_times = prd->stripTimes();

			vects.charge_all.push_back(charge);
			
			float c_time = 0;
			for(unsigned int sIdx=0; sIdx<stripIds.size(); ++sIdx){
			  vects.strp_times.push_back(s_times.at(sIdx));
			  c_time += s_times.at(sIdx);
			}
			c_time /= s_times.size();
			vects.cl_times.push_back(c_time);

			if(m_doDetailedHists){
			  auto& vect = sumPlots[iside][sectorPhi-1][std::abs(stEta)-1][multi-1][gap-1];
			  vect.cl_size.push_back(csize);
			  vect.pcb.push_back(PCB);
			  for(unsigned int sIdx=0; sIdx<stripIds.size(); ++sIdx)
			    { 
			      vect.strip_number.push_back(stripNumbers[sIdx]);
			      vect.strp_times.push_back(s_times.at(sIdx));
			      vect.pcb_strip.push_back(get_PCB_from_channel(stripNumbers[sIdx]));
			    }
			  vect.cl_times.push_back(c_time);
			  vect.charge.push_back(charge);
			}


			const int gap_offset=4;
			int gas_gap8 = (multi==1) ?  gap :  gap + gap_offset; 

			int FEB = get_FEB_from_channel(ch, stEta);
			int bin = get_bin_for_feb_occ(gas_gap8,FEB);
			thisSect.sector_lb_ontrack.push_back(bin);
			// Occupancy plots with FEB granularity further divided for each eta sector: -2, -1, 1, 2
			// Filling Vectors for stationEta=-1 - cluster on track
			if(stEta<0) {
			  vects.sector_CSide_ontrack.push_back(bin);
			  vects.stationPhi_CSide_ontrack.push_back(sectorPhi);
			} else {
			  vects.sector_ASide_ontrack.push_back(bin);
			  vects.stationPhi_ASide_ontrack.push_back(sectorPhi);
			}

			float x = cluster->localParameters()[Trk::loc1];
			for(const Trk::TrackStateOnSurface* trkState : *meTrack->trackStateOnSurfaces()) {
					
				if(!(trkState)) continue;
				if (!trkState->type(Trk::TrackStateOnSurface::Measurement)) continue;

				Identifier surfaceId = (trkState)->surface().associatedDetectorElementIdentifier();
				if(!m_idHelperSvc->isMM(surfaceId)) continue;
			
				int trk_stEta = m_idHelperSvc->mmIdHelper().stationEta(surfaceId);
				int trk_stPhi = m_idHelperSvc->mmIdHelper().stationPhi(surfaceId);
				int trk_multi = m_idHelperSvc->mmIdHelper().multilayer(surfaceId);
				int trk_gap   = m_idHelperSvc->mmIdHelper().gasGap(surfaceId);

				if( (trk_stPhi == stPhi) && (trk_stEta == stEta) && (trk_multi == multi) && (trk_gap == gap)) {
				  double x_trk = trkState->trackParameters()->parameters()[Trk::loc1];
				  int sectorPhi = get_sectorPhi_from_stationPhi_stName(trk_stPhi,stName); // 1->16
				  int side 	= (stEta > 0) ? 1 : 0;
				  float res_stereo = (x - x_trk);
					if(m_do_stereoCorrection) {
						float stereo_angle = ((multi == 1 && gap < 3) || (multi == 2 && gap > 2)) ? 0 : 0.02618;
						double y_trk = trkState->trackParameters()->parameters()[Trk::locY];
						float stereo_correction = ( (multi == 1 && gap < 3) || (multi == 2 && gap > 2) ) ? 0 : ( ((multi == 1 && gap == 3) || (multi == 2 && gap ==1 )) ? (-std::sin(stereo_angle)*y_trk) : std::sin(stereo_angle)*y_trk );
						res_stereo = (x - x_trk)*std::cos(stereo_angle) - stereo_correction;
					}
					auto residual_mon = Monitored::Scalar<float>("residual", res_stereo);
					auto stPhi_mon = Monitored::Scalar<float>("stPhi_mon",sectorPhi);

					fill("mmMonitor", residual_mon, eta_trk, phi_trk, stPhi_mon);
					int abs_stEta = get_sectorEta_from_stationEta(stEta); // 0 or 1

					if(m_doDetailedHists){
					  auto& vectors = summaryPlots_full[side][sectorPhi-1][abs_stEta][multi-1][gap-1];
					  vectors.residuals.push_back(res_stereo);
					}
				}
			}//TrackStates

		} // loop on meas
		if(isMM) {
		  ++ntrk;
		  fill("mmMonitor", pt_trk);
		}

		if(m_doDetailedHists){
		  for(int iside = 0; iside < 2; ++iside) {
		    std::string MM_sideGroup = "MM_sideGroup" + MM_Side[iside];
		    for(int statPhi = 0; statPhi < 16; ++statPhi) {
		      //				for(int statEta = 0; statEta < 2; ++statEta) {
		      for(int multiplet = 0; multiplet < 2; ++multiplet) {
			for(int gas_gap = 0; gas_gap < 4; ++gas_gap) {
			  auto layer=gas_gap+multiplet*4;
			  MMSummaryHistogramStruct vects;
			  for(int statEta = 0; statEta < 2; ++statEta) {
			    vects = summaryPlots_full[iside][statPhi][statEta][multiplet][gas_gap];
			    auto residuals_gap = Monitored::Collection("residuals_"+MM_Side[iside]+"_phi"+std::to_string(statPhi+1)+"_stationEta"+EtaSector[statEta]+"_multiplet"+std::to_string(multiplet+1)+"_gas_gap"+std::to_string(gas_gap+1),vects.residuals);
			    auto residuals_layer = Monitored::Collection("residuals_"+MM_Side[iside]+"_phi"+std::to_string(statPhi+1)+"_layer"+std::to_string(layer+1),vects.residuals);
			    
			    fill(MM_sideGroup, residuals_gap,residuals_layer);
			  }
			}
		      }
		    }
		  }
		}

		for(const Trk::TrackStateOnSurface* trkState : *meTrack->trackStateOnSurfaces()) {
			if(!(trkState)) continue;
			if (!trkState->type(Trk::TrackStateOnSurface::Measurement)) continue;
			Identifier surfaceId = (trkState)->surface().associatedDetectorElementIdentifier();
			if(!m_idHelperSvc->isMM(surfaceId)) continue;

			const Trk::MeasurementBase* meas = trkState->measurementOnTrack() ;
			if(!meas) continue;
			
			const Trk::RIO_OnTrack* rot = dynamic_cast<const Trk::RIO_OnTrack*>(meas);
                        if(!rot) continue;
                        Identifier rot_id = rot->identify();
                        if(!m_idHelperSvc->isMM(rot_id)) continue;
			
			const Amg::Vector3D& pos = (trkState)->trackParameters()->position();
			int stEta = m_idHelperSvc->mmIdHelper().stationEta(surfaceId);
			int multi = m_idHelperSvc->mmIdHelper().multilayer(surfaceId);
			int gap   = m_idHelperSvc->mmIdHelper().gasGap(surfaceId);

			// CSide and ASide
			int iside = (stEta > 0) ? 1 : 0;
			auto& Vectors = summaryPlots[iside][multi-1][gap-1];

			// Filling x-y position vectors using the trackStateonSurface
			Vectors.x_ontrack.push_back(pos.x());
			Vectors.y_ontrack.push_back(pos.y());
		}
	} // loop on muonContainer

	auto ntrack = Monitored::Scalar<int>("ntrk",ntrk);
	fill("mmMonitor", ntrack);

	auto& vects = overviewPlots;
	auto stationPhi_CSide_ontrack = Monitored::Collection("stationPhi_CSide_ontrack",vects.stationPhi_CSide_ontrack);
	auto stationPhi_ASide_ontrack = Monitored::Collection("stationPhi_ASide_ontrack",vects.stationPhi_ASide_ontrack);
	auto sector_ASide_ontrack = Monitored::Collection("sector_ASide_ontrack",vects.sector_ASide_ontrack);
	auto sector_CSide_ontrack = Monitored::Collection("sector_CSide_ontrack",vects.sector_CSide_ontrack);

	auto lb_ontrack = Monitored::Scalar<int>("lb_ontrack", lb);
	auto csize = Monitored::Collection("nstrips_ontrack", vects.numberofstrips_percluster);
	auto charge = Monitored::Collection("charge_ontrack", vects.charge_all);
	auto stime = Monitored::Collection("strip_time_on_track", vects.strp_times);
	auto ctime = Monitored::Collection("cluster_time_on_track", vects.cl_times);

	fill("mmMonitor", csize, charge, stime, ctime, stationPhi_CSide_ontrack, stationPhi_ASide_ontrack, sector_CSide_ontrack,sector_ASide_ontrack, lb_ontrack);

	for(int iside = 0; iside < 2; ++iside) {
		std::string MM_sideGroup = "MM_sideGroup" + MM_Side[iside];
		for(int statPhi = 0; statPhi < 16; ++statPhi) {
			for(int statEta = 0; statEta < 2; ++statEta) {
				for(int multiplet = 0; multiplet < 2; ++multiplet) {
					for(int gas_gap = 0; gas_gap < 4; ++gas_gap) {
						auto& vects = sumPlots[iside][statPhi][statEta][multiplet][gas_gap];
						if(m_doDetailedHists){
						  if(!vects.strip_number.empty())
						    {
						      auto clus_size = Monitored::Collection("cluster_size_ontrack_" + MM_Side[iside] + "_phi" + std::to_string(statPhi+1) + "_eta" + std::to_string(statEta+1) + "_ml" + std::to_string(multiplet+1) + "_gap" + std::to_string(gas_gap+1), vects.cl_size);
						      auto strip_times = Monitored::Collection("strp_time_ontrack_" + MM_Side[iside] + "_phi" + std::to_string(statPhi+1) + "_eta" + std::to_string(statEta+1) + "_ml" + std::to_string(multiplet+1) + "_gap" + std::to_string(gas_gap+1), vects.strp_times);
						      auto cluster_time = Monitored::Collection("cluster_time_ontrack_" + MM_Side[iside] + "_phi" + std::to_string(statPhi+1) + "_eta" + std::to_string(statEta+1) + "_ml" + std::to_string(multiplet+1) + "_gap" + std::to_string(gas_gap+1), vects.cl_times);
						      auto charge_perPCB = Monitored::Collection("charge_perPCB_ontrack_" + MM_Side[iside] + "_phi" + std::to_string(statPhi+1) + "_eta" + std::to_string(statEta+1) + "_ml" + std::to_string(multiplet+1) + "_gap" + std::to_string(gas_gap+1), vects.charge);
						      auto charge_perlayer = Monitored::Collection("charge_perlayer_ontrack_" + MM_Side[iside] + "_phi" + std::to_string(statPhi+1) + "_ml" + std::to_string(multiplet+1) + "_gap" + std::to_string(gas_gap+1), vects.charge);
						      auto clus_size_perlayer = Monitored::Collection("cluster_size_perlayer_ontrack_" + MM_Side[iside] + "_phi" + std::to_string(statPhi+1) + "_ml" + std::to_string(multiplet+1) + "_gap" + std::to_string(gas_gap+1), vects.cl_size);
						      auto pcb_mon = Monitored::Collection("pcb_mon_ontrack_" + MM_Side[iside] + "_phi" + std::to_string(statPhi+1) + "_eta" + std::to_string(statEta+1) + "_ml" + std::to_string(multiplet+1) + "_gap" + std::to_string(gas_gap+1), vects.pcb);
						      auto pcb_strip_mon = Monitored::Collection("pcb_strip_mon_ontrack_" + MM_Side[iside] + "_phi" + std::to_string(statPhi+1) + "_eta" + std::to_string(statEta+1) + "_ml" + std::to_string(multiplet+1) + "_gap" + std::to_string(gas_gap+1), vects.pcb_strip);
						      
						      fill(MM_sideGroup, clus_size, strip_times, cluster_time, charge_perPCB, pcb_mon, pcb_strip_mon,charge_perlayer,clus_size_perlayer);
						    }
						}

					}
				}
			}
			auto& occ_lb = occupancyPlots[statPhi][iside];
			auto sector_lb_ontrack = Monitored::Collection("sector_lb_"+MM_Side[iside]+"_phi"+std::to_string(statPhi+1)+"_ontrack",occ_lb.sector_lb_ontrack);
			fill(MM_sideGroup, lb_ontrack, sector_lb_ontrack);
		}
		for(int multiplet=0; multiplet<2; ++multiplet) {
			for(int gas_gap=0; gas_gap<4; ++gas_gap) {
				auto& Vectors = summaryPlots[iside][multiplet][gas_gap];
				auto x_ontrack = Monitored::Collection("x_"+MM_Side[iside]+"_multiplet"+std::to_string(multiplet+1)+"_gas_gap_"+std::to_string(gas_gap+1)+"_ontrack", Vectors.x_ontrack);
				auto y_ontrack = Monitored::Collection("y_"+MM_Side[iside]+"_multiplet"+std::to_string(multiplet+1)+"_gas_gap_"+std::to_string(gas_gap+1)+"_ontrack", Vectors.y_ontrack);
				fill(MM_sideGroup, x_ontrack, y_ontrack);
			}
		}
	}

}

void MMRawDataMonAlg::MMEfficiency( const xAOD::TrackParticleContainer*  muonContainer) const
{
	MMEfficiencyHistogramStruct effPlots[2][2][16][2][4];
	MMEfficiencyHistogramStruct Gaps[2][2][16][2];

	static const std::array<std::string,2> MM_Side = {"CSide", "ASide"};
	static const std::array<std::string,2> EtaSector = {"1","2"};

	for (const xAOD::TrackParticle* meTP  : *muonContainer) {
		if (!meTP) continue;
		auto eta_trk = Monitored::Scalar<float>("eta_trk", meTP->eta());
		auto phi_trk = Monitored::Scalar<float>("phi_trk", meTP->phi());

		float pt_trk = meTP->pt();
		if(pt_trk < m_cut_pt) continue;
		// retrieve the original track
		const Trk::Track* meTrack = meTP->track();
		if(!meTrack) continue;
		// get the vector of measurements on track
		const DataVector<const Trk::MeasurementBase>* meas = meTrack->measurementsOnTrack();

		for(const Trk::MeasurementBase* it: *meas) {
			const Trk::RIO_OnTrack* rot = dynamic_cast<const Trk::RIO_OnTrack*>(it);
			if (!rot) continue;
			Identifier rot_id = rot->identify();
			if (!m_idHelperSvc->isMM(rot_id)) continue;

			const Muon::MMClusterOnTrack* cluster = dynamic_cast<const Muon::MMClusterOnTrack*>(rot);
			if (!cluster) continue;
			std::string stName   = m_idHelperSvc->mmIdHelper().stationNameString(m_idHelperSvc->mmIdHelper().stationName(rot_id));
			int stEta= m_idHelperSvc->mmIdHelper().stationEta(rot_id);
			int stPhi= m_idHelperSvc->mmIdHelper().stationPhi(rot_id);
			int phi = get_sectorPhi_from_stationPhi_stName(stPhi,stName);
			int multi = m_idHelperSvc->mmIdHelper().multilayer(rot_id);
			int gap=  m_idHelperSvc->mmIdHelper().gasGap(rot_id);
			int ch=  m_idHelperSvc->mmIdHelper().channel(rot_id);
			int pcb=get_PCB_from_channel(ch);
			int abs_stEta= get_sectorEta_from_stationEta(stEta);
			int iside = (stEta > 0) ? 1 : 0;
                        if( ! (std::find( Gaps[iside][abs_stEta][phi-1][multi-1].nGaps.begin(), Gaps[iside][abs_stEta][phi-1][multi-1].nGaps.end(), gap ) != Gaps[iside][abs_stEta][phi-1][multi-1].nGaps.end()) )
                          Gaps[iside][abs_stEta][phi-1][multi-1].nGaps.push_back(gap);
			//numerator
			if(effPlots[iside][abs_stEta][phi-1][multi-1][gap-1].num.size()==0) effPlots[iside][abs_stEta][phi-1][multi-1][gap-1].num.push_back(pcb-1);
		}
	} // loop on tracks

	unsigned  int nGaptag=3;

	for(int s=0; s<2; ++s) {
		std::string MM_sideGroup = "MM_sideGroup"+MM_Side[s];
		for(int e=0; e<2; ++e) {
			for(int p=0; p<16; ++p) {
				for(int m=0; m<2; ++m) {
					if(Gaps[s][e][p][m].nGaps.size()<nGaptag) continue;
                                        if(Gaps[s][e][p][m].nGaps.size()>4) continue;
                                        //find the missing gap                                                                                                                                                                         
                                        int gapsum=0;
					for (unsigned int g=0; g<Gaps[s][e][p][m].nGaps.size(); ++g)
					  gapsum+= Gaps[s][e][p][m].nGaps.at(g);
					int missing_gap=10-gapsum-1;
					//if missing gap = -1 --> nGaps=4 --> all efficient                                                                                                                                           
					if(Gaps[s][e][p][m].nGaps.size()==4){
					  for (unsigned int ga=0; ga<Gaps[s][e][p][m].nGaps.size(); ++ga){
					    for (unsigned int i=0; i<effPlots[s][e][p][m][ga].num.size(); ++i){
					      int pcb = effPlots[s][e][p][m][ga].num.at(i);
					      auto traversed_pcb = Monitored::Scalar<int>("pcb_eta"+std::to_string(e+1)+"_"+MM_Side[s]+"_phi"+std::to_string(p)+"_multiplet"+std::to_string(m+1)+"_gas_gap"+std::to_string(ga+1),pcb);
					      int layer=ga+4*m+8*p;
					      
					      auto traversed_gap = Monitored::Scalar<int>(MM_Side[s]+"_eta"+std::to_string(e+1),layer);
					      auto isHit = 1;
					      auto hitcut = Monitored::Scalar<int>("hitcut", (int)isHit);
					      fill(MM_sideGroup, traversed_pcb, traversed_gap, hitcut);
					    }
					  }
					} else {// 3 gaps, the fourth is inefficient                                                                                                                                                  
					  int ref_gap = missing_gap+1;
					  if(missing_gap==3) ref_gap=0;
					  if (ref_gap>3){
					    ATH_MSG_FATAL("ref_gap is out of range in MMRawDataMonAlg::MMEfficiency");
					    return;
					  }
					  int ref_pcb=effPlots[s][e][p][m][ref_gap].num.at(0);
					  auto traversed_pcb = Monitored::Scalar<int>("pcb_eta"+std::to_string(e+1)+"_"+MM_Side[s]+"_phi"+std::to_string(p)+"_multiplet"+std::to_string(m+1)+"_gas_gap"+std::to_string(missing_gap+1), ref_pcb);
					  int layer=missing_gap+4*m+8*p;
					  auto traversed_gap = Monitored::Scalar<int>(MM_Side[s]+"_eta"+std::to_string(e+1),layer);
					  auto isHit = 0;
					  auto hitcut = Monitored::Scalar<int>("hitcut", (int)isHit);
					  fill(MM_sideGroup, traversed_pcb, traversed_gap, hitcut);
					}


				}
			}
		}
	}
}


void MMRawDataMonAlg::clusterFromSegments(const Trk::SegmentCollection* segms, int lb) const
{
	MMOverviewHistogramStruct overviewPlots;
  	MMByPhiStruct occupancyPlots[16][2];
  	MMSummaryHistogramStruct summaryPlots[2][16][2][2][4];
	int nseg=0;

  	for (Trk::SegmentCollection::const_iterator s = segms->begin(); s != segms->end(); ++s) {
  		const Muon::MuonSegment* segment = dynamic_cast<const Muon::MuonSegment*>(*s);
  		if (segment == nullptr) {
  			ATH_MSG_DEBUG("no pointer to segment!!!");
  			break;
		}
		bool isMM=false;
    	for(unsigned int irot=0;irot<segment->numberOfContainedROTs();irot++){
    		const Trk::RIO_OnTrack* rot = segment->rioOnTrack(irot);
    		if(!rot) continue;
    		Identifier rot_id = rot->identify();
    		if(!m_idHelperSvc->isMM(rot_id)) continue;
		isMM=true;
    		const Muon::MMClusterOnTrack* cluster = dynamic_cast<const Muon::MMClusterOnTrack*>(rot);
    		if(!cluster) continue;

    		std::string stName = m_idHelperSvc->mmIdHelper().stationNameString(m_idHelperSvc->mmIdHelper().stationName(rot_id));
		int stEta          = m_idHelperSvc->mmIdHelper().stationEta(rot_id);
		int stPhi          = m_idHelperSvc->mmIdHelper().stationPhi(rot_id);
		int multi          = m_idHelperSvc->mmIdHelper().multilayer(rot_id);
		int gap            = m_idHelperSvc->mmIdHelper().gasGap(rot_id);
		int ch             = m_idHelperSvc->mmIdHelper().channel(rot_id);

      		// MMS and MML phi sectors                                                                                                                                                                                                  
      		int sectorPhi = get_sectorPhi_from_stationPhi_stName(stPhi,stName);
      		int PCB = get_PCB_from_channel(ch);
      		int iside = (stEta > 0) ? 1 : 0;

      		const Muon::MMPrepData* prd = cluster->prepRawData();
      		const std::vector<Identifier>& stripIds = prd->rdoList();
      		unsigned int csize = stripIds.size();
      		const std::vector<uint16_t>& stripNumbers = prd->stripNumbers();

      		auto& pcb_vects = summaryPlots[iside][sectorPhi-1][std::abs(stEta)-1][multi-1][gap-1];
      		pcb_vects.cl_size.push_back(csize);
		pcb_vects.pcb.push_back(PCB);
      		std::vector<short int> s_times = prd->stripTimes();
      		float c_time = 0;
      		for(unsigned int sIdx=0; sIdx<csize; ++sIdx) {
		  pcb_vects.strp_times.push_back(s_times.at(sIdx));
		  pcb_vects.pcb_strip.push_back( get_PCB_from_channel(stripNumbers[sIdx]));
		  c_time += s_times.at(sIdx);
		}
      		c_time /= s_times.size();
      		pcb_vects.cl_times.push_back(c_time);

      		float charge = prd->charge()*conversion_charge;
		pcb_vects.charge.push_back(charge);

      		auto& vects = overviewPlots;

      		auto& thisSect = occupancyPlots[sectorPhi-1][iside];

		const int gap_offset=4;
		int gas_gap8 = (multi==1) ?  gap :  gap + gap_offset;
		int FEB = get_FEB_from_channel(ch, stEta);

		int bin=get_bin_for_feb_occ(gas_gap8,FEB);
		thisSect.sector_lb_onseg.push_back(bin);

		if(stEta<0) {
		  vects.sector_CSide_onseg.push_back(bin);
		  vects.stationPhi_CSide_onseg.push_back(sectorPhi);
		} else {
		  vects.sector_ASide_onseg.push_back(bin);
		  vects.stationPhi_ASide_onseg.push_back(sectorPhi);
		}

    	} // loop on ROT container                                                                                                                                                                                                           
	if (isMM==true) ++nseg;

  	} // loop on segment collection                                                                                                                                                                  
	auto nsegs = Monitored::Scalar<int>("nseg",nseg);
	fill("mmMonitor", nsegs);

  	auto& vects = overviewPlots;
	auto stationPhi_CSide_onseg = Monitored::Collection("stationPhi_CSide_onseg",vects.stationPhi_CSide_onseg);
  	auto stationPhi_ASide_onseg = Monitored::Collection("stationPhi_ASide_onseg",vects.stationPhi_ASide_onseg);
	auto sector_ASide_onseg = Monitored::Collection("sector_ASide_onseg",vects.sector_ASide_onseg);
  	auto sector_CSide_onseg = Monitored::Collection("sector_CSide_onseg",vects.sector_CSide_onseg);

	auto lb_onseg = Monitored::Scalar<int>("lb_onseg", lb);

	fill("mmMonitor", stationPhi_CSide_onseg, stationPhi_ASide_onseg, sector_CSide_onseg, sector_ASide_onseg, lb_onseg);

  	for(int iside = 0; iside < 2; ++iside) {
    	std::string MM_sideGroup = "MM_sideGroup" + MM_Side[iside];
    	for(int statPhi=0; statPhi<16; ++statPhi) {
      		auto& occ_lb = occupancyPlots[statPhi][iside];
      		auto sector_lb_onseg = Monitored::Collection("sector_lb_"+MM_Side[iside]+"_phi"+std::to_string(statPhi+1)+"_onseg",occ_lb.sector_lb_onseg);
      		fill(MM_sideGroup, lb_onseg, sector_lb_onseg);

      		for(int statEta = 0; statEta < 2; ++statEta) {
				for(int multiplet = 0; multiplet < 2; ++multiplet) {
	  				for(int gas_gap = 0; gas_gap < 4; ++gas_gap) {
	    				auto& pcb_vects = summaryPlots[iside][statPhi][statEta][multiplet][gas_gap];

	    				if(pcb_vects.pcb.empty()) continue;
					if(m_doDetailedHists){
					  auto pcb_mon = Monitored::Collection("pcb_mon_onseg_" + MM_Side[iside] + "_phi" + std::to_string(statPhi+1) + "_eta" + std::to_string(statEta+1) + "_ml" + std::to_string(multiplet+1) + "_gap" + std::to_string(gas_gap+1), pcb_vects.pcb);
					  auto pcb_strip_mon = Monitored::Collection("pcb_strip_mon_onseg_" + MM_Side[iside] + "_phi" + std::to_string(statPhi+1) + "_eta" + std::to_string(statEta+1) + "_ml" + std::to_string(multiplet+1) + "_gap" + std::to_string(gas_gap+1), pcb_vects.pcb_strip);
					  auto strip_times = Monitored::Collection("strp_time_onseg_" + MM_Side[iside] + "_phi" + std::to_string(statPhi+1) + "_eta" + std::to_string(statEta+1) + "_ml" + std::to_string(multiplet+1) + "_gap" + std::to_string(gas_gap+1), pcb_vects.strp_times);
					  auto cluster_time = Monitored::Collection("cluster_time_onseg_" + MM_Side[iside] + "_phi" + std::to_string(statPhi+1) + "_eta" + std::to_string(statEta+1) + "_ml" + std::to_string(multiplet+1) + "_gap" + std::to_string(gas_gap+1), pcb_vects.cl_times);
					  auto clus_size = Monitored::Collection("cluster_size_onseg_" + MM_Side[iside] + "_phi" + std::to_string(statPhi+1) + "_eta" + std::to_string(statEta+1) + "_ml" + std::to_string(multiplet+1) + "_gap" + std::to_string(gas_gap+1), pcb_vects.cl_size);
					  auto charge_perPCB = Monitored::Collection("charge_perPCB_onseg_" + MM_Side[iside] + "_phi" + std::to_string(statPhi+1) + "_eta" + std::to_string(statEta+1) + "_ml" + std::to_string(multiplet+1) + "_gap" + std::to_string(gas_gap+1), pcb_vects.charge);
					  
					  fill(MM_sideGroup, clus_size, strip_times, charge_perPCB, cluster_time, pcb_mon, pcb_strip_mon);
					}
					auto clus_size_all = Monitored::Collection("cluster_size_onseg", pcb_vects.cl_size); 
					auto charge_all = Monitored::Collection("charge_onseg", pcb_vects.charge);
					auto strip_times_all = Monitored::Collection("strp_time_onseg", pcb_vects.strp_times);
					fill("mmMonitor", clus_size_all, charge_all, strip_times_all);
	  				}
				}
      		}
    	}
  	}
}
