/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <cmath>
#include <functional>
#include <type_traits>

#include "RpcRoadDefiner.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

StatusCode TrigL2MuonSA::RpcRoadDefiner::initialize()
{
  ATH_CHECK(m_idHelperSvc.retrieve());

  ATH_CHECK(m_regionSelector.retrieve());
  ATH_MSG_DEBUG("Retrieved the RegionSelector tool ");

  return StatusCode::SUCCESS;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

StatusCode TrigL2MuonSA::RpcRoadDefiner::defineRoad(const EventContext& ctx,
				const xAOD::MuonRoI* p_roi,
				const bool                          insideOut,
				TrigL2MuonSA::MuonRoad&             muonRoad,
                const TrigL2MuonSA::RpcLayerHits&   rpcLayerHits,
				const ToolHandle<RpcPatFinder>*     rpcPatFinder,
				TrigL2MuonSA::RpcFitResult&         rpcFitResult,
                const double                        roiEtaMinLow,
                const double                        roiEtaMaxLow,
                const double                        roiEtaMinHigh,
                const double                        roiEtaMaxHigh) const
{
    const double ZERO_LIMIT = 1e-5;

    if (m_use_rpc && !insideOut) {
        std::array<std::reference_wrapper<double>, 3> aw {
            rpcFitResult.slope_inner, rpcFitResult.slope_middle, rpcFitResult.slope_outer
        };
        std::array<std::reference_wrapper<double>, 3> bw {
            rpcFitResult.offset_inner, rpcFitResult.offset_middle, rpcFitResult.offset_outer
        };
        rpcFitResult.isSuccess = (*rpcPatFinder)->findPatternEta(aw, bw, rpcLayerHits);

        for(int i=0;i<3;i++){
            if(std::abs(aw[i].get()) <= ZERO_LIMIT) rpcFitResult.isSuccess = false;
        }

        double phi_middle;
        double phi_outer;
        if ( (*rpcPatFinder)->findPatternPhi(phi_middle, phi_outer, rpcLayerHits)) {
            rpcFitResult.phi = phi_middle;
            rpcFitResult.phi_middle = phi_middle;
            rpcFitResult.phi_outer = phi_outer;
        } else {
            rpcFitResult.phi = p_roi->phi();
        }
        ATH_MSG_DEBUG("RpcPatFinder: Found: " << rpcFitResult.isSuccess << " Slopes: " << aw[0] << "," << aw[1] << "," << aw[2] << " Offsets: " << bw[0] << "," << bw[1] << "," << bw[2]);
    } else {
        ATH_MSG_DEBUG("Skip rpcPatFinder");
    }

    muonRoad.isEndcap   = false;
    if(!insideOut) {
        muonRoad.phiMiddle  = rpcFitResult.phi;
    } else {
        muonRoad.phiMiddle  = muonRoad.extFtfMiddlePhi;
        rpcFitResult.phi = muonRoad.extFtfMiddlePhi;
        rpcFitResult.phi_middle = muonRoad.extFtfMiddlePhi;
        rpcFitResult.phi_outer = muonRoad.extFtfMiddlePhi;
    }
    muonRoad.phiRoI     = p_roi->phi();
    muonRoad.side       = (p_roi->phi()<0.)? 0 : 1;
    muonRoad.LargeSmall = ((p_roi->getSectorID() + 1)/2 )%2;

    const int PhysicsSector = ((p_roi->getSectorID() + 1)/4 )%8 + 1;

    int special = 0;
    if (muonRoad.LargeSmall == 0 && (PhysicsSector == 6 || PhysicsSector == 8 )) special = 1;     // BIM BIR
    if (muonRoad.LargeSmall == 1 && (PhysicsSector == 6 || PhysicsSector == 7 )) special = 1;     //feets
    muonRoad.Special = special;

    auto fillAllLayersWith = [&muonRoad](const int& station, const double& value) -> void {
        std::fill(std::begin(muonRoad.rWidth[station]), std::end(muonRoad.rWidth[station]), value);
    };

    if (!rpcFitResult.isSuccess) {
        if (!insideOut) {
            fillAllLayersWith(0, 500); // Inner
            fillAllLayersWith(1, 650); // Middle
            fillAllLayersWith(2, 800); // Outer
            fillAllLayersWith(3, 500); // EndcapInner
            fillAllLayersWith(9, 650); // BME
            fillAllLayersWith(10, 650); // BMG
        } else {
            fillAllLayersWith(0, 250); // Inner
            fillAllLayersWith(1, 400); // Middle
            fillAllLayersWith(2, 600); // Outer
            fillAllLayersWith(3, 300); // EndcapInner
            fillAllLayersWith(9, 400); // BME
            fillAllLayersWith(10, 400); // BMG
        }
    } else {
        fillAllLayersWith(0, 400); // Inner
        fillAllLayersWith(1, 200); // Middle
        fillAllLayersWith(2, 400); // Outer
        fillAllLayersWith(3, 400); // EndcapInner
        fillAllLayersWith(9, m_rWidth_RPC_Failed); // BME
        fillAllLayersWith(10, m_rWidth_RPC_Failed); // BMG
    }

    std::vector<IdentifierHash> mdtHashList;

    // get sector_trigger and sector_overlap by using the region selector
    IdContext context = m_idHelperSvc->mdtIdHelper().module_context();

    double etaMin =  p_roi->eta()-.02;
    double etaMax =  p_roi->eta()+.02;
    double phiMin = muonRoad.phiMiddle-.01;
    double phiMax = muonRoad.phiMiddle+.01;
    if(phiMax > M_PI) phiMax -= M_PI*2.;
    if(phiMin < -M_PI) phiMin += M_PI*2.;

    auto roi = std::make_unique<TrigRoiDescriptor>( p_roi->eta(), etaMin, etaMax, muonRoad.phiMiddle, phiMin, phiMax );

    if (roi) m_regionSelector->lookup(ctx)->HashIDList( *roi, mdtHashList);
    else {
        TrigRoiDescriptor fullscan_roi( true );
        m_regionSelector->lookup(ctx)->HashIDList(fullscan_roi, mdtHashList);
    }

    int &sector_trigger {muonRoad.MDT_sector_trigger}, &sector_overlap {muonRoad.MDT_sector_overlap};
    sector_trigger = (PhysicsSector - 1)*2 + muonRoad.LargeSmall;
    sector_overlap = 99;

    for( const IdentifierHash& hash : mdtHashList){

        Identifier id;
        if( m_idHelperSvc->mdtIdHelper().get_id(hash, id, &context) !=0 ) ATH_MSG_ERROR("problem converting hash list to id");

        muonRoad.stationList.push_back(id);
        const int stationPhi = m_idHelperSvc->mdtIdHelper().stationPhi(id);
        const std::string name = m_idHelperSvc->mdtIdHelper().stationNameString(m_idHelperSvc->mdtIdHelper().stationName(id));

        if ( name[1]=='M' && name[2]=='E' ) continue;//exclude BME
        if ( name[1]=='M' && name[2]=='G' ) continue;//exclude BMG

        const int LargeSmall = (name[2]=='S' || name[2]=='F' || name[2]=='G' ) ? 1 : 0;
        const int sector = (stationPhi-1)*2 + LargeSmall;

        if (sector != sector_trigger) sector_overlap = sector;
        if (sector != sector_trigger and sector_overlap != 99 and sector != sector_overlap) ATH_MSG_ERROR("Multiple sector overlaps not expected");
    }

    /// Copy a,b coefficients to muonRoad
    auto fillAllSectorsWith = [&muonRoad](const int& station, const double& aw, const double& bw) -> void {
        std::fill(std::begin(muonRoad.aw[station]), std::end(muonRoad.aw[station]), aw);
        std::fill(std::begin(muonRoad.bw[station]), std::end(muonRoad.bw[station]), bw);
    };

    if (!insideOut) {
        if (rpcFitResult.isSuccess) {       // MOD  this code is very suspicious
            fillAllSectorsWith(0, rpcFitResult.slope_inner, rpcFitResult.offset_inner);     //Barrel inner
            fillAllSectorsWith(1, rpcFitResult.slope_middle, rpcFitResult.offset_middle);   //Barrel middle
            fillAllSectorsWith(2, rpcFitResult.slope_outer, rpcFitResult.offset_outer);     //Barrel outer
            fillAllSectorsWith(3, rpcFitResult.slope_inner, rpcFitResult.offset_inner);     //Endcap inner
            fillAllSectorsWith(9, rpcFitResult.slope_middle, rpcFitResult.offset_middle);   //BME
            fillAllSectorsWith(10, rpcFitResult.slope_middle, rpcFitResult.offset_middle);  //BMG

        } else {
            auto compute_aw = [&ZERO_LIMIT](double etaMin, double etaMax) -> double {
                const double eta = 0.5 * (etaMin + etaMax);
                if (std::abs(eta) < ZERO_LIMIT) return 0.;
                const double theta = 2.* std::atan(std::exp(-std::abs(eta)));
                return std::tan(theta) * (eta / std::abs(eta));  // preserves sign
            };

            const double awLow  = compute_aw(roiEtaMinLow, roiEtaMaxLow);
            const double awHigh = compute_aw(roiEtaMinHigh, roiEtaMaxHigh);

            fillAllSectorsWith(0, awLow, 0.);       //Barrel inner
            fillAllSectorsWith(1, awLow, 0.);       //Barrel middle
            fillAllSectorsWith(2, awHigh, 0.);      //Barrel outer
            fillAllSectorsWith(3, awLow, 0.);       //Endcap inner
            fillAllSectorsWith(9, awLow, 0.);       //BME
            fillAllSectorsWith(10, awLow, 0.);      //BMG
        }
    } else {

        ATH_MSG_DEBUG("Use aw_ftf and bw_ftf as aw and bw");
        fillAllSectorsWith(0, muonRoad.aw_ftf[0][0], muonRoad.bw_ftf[0][0]);       //Barrel inner
        fillAllSectorsWith(1, muonRoad.aw_ftf[1][0], muonRoad.bw_ftf[1][0]);       //Barrel middle
        fillAllSectorsWith(2, muonRoad.aw_ftf[2][0], muonRoad.bw_ftf[2][0]);       //Barrel outer
        fillAllSectorsWith(3, muonRoad.aw_ftf[3][0], muonRoad.bw_ftf[3][0]);       //Endcap inner
        fillAllSectorsWith(9, muonRoad.aw_ftf[9][0], muonRoad.bw_ftf[9][0]);       //BME
        fillAllSectorsWith(10, muonRoad.aw_ftf[10][0], muonRoad.bw_ftf[10][0]);    //BMG
    }

  ATH_MSG_DEBUG("muonRoad.phiMiddle: " << muonRoad.phiMiddle);

  return StatusCode::SUCCESS;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
