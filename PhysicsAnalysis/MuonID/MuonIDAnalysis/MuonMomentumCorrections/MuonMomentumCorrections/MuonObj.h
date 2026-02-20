/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MCPMUONOBJ_H
#define MCPMUONOBJ_H

#include "xAODMuon/Muon.h"
#include "MuonMomentumCorrections/EnumDef.h"
#include "MuonAnalysisInterfaces/IMuonSelectionTool.h"

#include "ColumnarCore/ColumnAccessor.h"
#include "ColumnarCore/ColumnarTool.h"
#include "ColumnarCore/LinkColumn.h"
#include <ColumnarCore/ObjectColumn.h>
#include "ColumnarCore/OptObjectId.h"
#include "ColumnarEventInfo/EventInfoHelpers.h"
#include "ColumnarMuon/MuonDef.h"
#include "ColumnarMuon/MuonTrackHelpers.h"
#include "ColumnarTracking/TrackHelpers.h"
#include "ColumnarVariant/VariantAccessor.h"
#include "ColumnarVariant/VariantDef.h"
#include "ColumnarVariant/VariantLinkColumn.h"

#include <optional>

namespace
{
    static constexpr double GeVtoMeV = 1e+3;
    static constexpr double MeVtoGeV = 1e-3;
} // namespace


namespace MCP {

    /// Accessors for the MuonCalibTool
    struct MuonCalibToolAccessors : public columnar::ColumnarTool<> {
        MuonCalibToolAccessors(columnar::ColumnarTool<>& base) : columnar::ColumnarTool<>(&base) {}
        columnar::EventInfoAccessor<columnar::ObjectColumn> m_eventInfoCol {*this, "EventInfo", {.addMTDependency=true}};
        columnar::MuonAccessor<columnar::ObjectColumn> m_muons {*this, "Muons"};
        columnar::Track0Accessor<columnar::ObjectColumn> m_tracksID {*this, "InDetTrackParticles"};
        columnar::Track1Accessor<columnar::ObjectColumn> m_tracksCB {*this, "CombinedMuonTrackParticles"};
        columnar::Track2Accessor<columnar::ObjectColumn> m_tracksME {*this, "ExtrapolatedMuonTrackParticles"};
        columnar::Track3Accessor<columnar::ObjectColumn> m_tracksFID {*this, "InDetForwardTrackParticles"};

        columnar::EventInfoHelpers::EventTypeAccessor<> eventTypeAcc {*this};
        columnar::EventInfoAccessor<uint32_t> runNumberAcc {*this, "runNumber"};
        columnar::EventInfoAccessor<uint64_t> eventNumberAcc {*this, "eventNumber"};
        columnar::EventInfoAccessor<unsigned int> acc_rnd{*this, "RandomRunNumber"};

        columnar::MuonAccessor<columnar::RetypeColumn<double,float>> ptAcc {*this, "pt"};
        columnar::MuonDecorator<float> ptOutDec {*this, "ptOut", {.replacesColumn = "pt"}};
        columnar::MuonAccessor<columnar::RetypeColumn<double,float>> etaAcc {*this, "eta"};
        columnar::MuonAccessor<columnar::RetypeColumn<double,float>> phiAcc {*this, "phi"};
        columnar::MuonAccessor<float> chargeAcc {*this, "charge"};
        columnar::MuonDecorator<float> chargeOutDec {*this, "chargeOut", {.replacesColumn = "charge"}};
        columnar::MuonAccessor<columnar::RetypeColumn<xAOD::Muon::MuonType,std::uint16_t>> muonTypeAcc {*this, "muonType"};
        columnar::MuonAccessor<columnar::RetypeColumn<xAOD::Muon::Author,std::uint16_t>> authorAcc {*this, "author"};
        columnar::MuonDecorator<float> dec_idPt{*this, "InnerDetectorPt"};
        columnar::MuonDecorator<float> dec_mePt{*this, "MuonSpectrometerPt"};
        columnar::MuonDecorator<float> dec_idCharge{*this, "InnerDetectorCharge"};
        columnar::MuonDecorator<float> dec_meCharge{*this, "MuonSpectrometerCharge"};
        columnar::MuonAccessor<columnar::OptTrack1Id> combinedTrackParticleLinkAcc{*this, "combinedTrackParticleLink"};
        columnar::MuonAccessor<columnar::ObjectLink<columnar::MuonTrackDef>> inDetTrackParticleLinkAcc{*this, "inDetTrackParticleLink"};
        columnar::MuonAccessor<columnar::OptTrack2Id> extrapolatedMuonSpectrometerTrackParticleLinkAcc{*this, "extrapolatedMuonSpectrometerTrackParticleLink"};

        columnar::TrackHelpers::ChargeAccessor<columnar::MuonTrackDef> trkChargeAcc {*this};
        columnar::TrackHelpers::TrackMomentumAccessors<columnar::MuonTrackDef> trkMomentumAcc {*this};
        columnar::TrackHelpers::DefiningParametersAccessor<columnar::MuonTrackDef> trkDefiningParametersAcc {*this};
        columnar::TrackHelpers::DefiningParametersCovAccessor<columnar::MuonTrackDef> trkDefiningParametersCovAcc {*this};
    };

   /// Basic object to cache all relevant information from the track
   struct TrackCalibObj{
        TrackCalibObj() = default;
        TrackCalibObj(const MuonCalibToolAccessors& acc, columnar::OptObjectId<columnar::MuonTrackDef> track, TrackType t, int charge,
                      DataYear year, bool isData)
            : type{t},
              is_valid{track.has_value()},
              uncalib_pt{(track.has_value()) ? acc.trkMomentumAcc.pt(track.value(),ParticleConstants::muonMassInMeV) * MeVtoGeV : 0},
              calib_pt{uncalib_pt},
              eta{(track.has_value()) ? acc.trkMomentumAcc.eta(track.value(),ParticleConstants::muonMassInMeV) : FLT_MAX},
              phi{(track.has_value()) ? acc.trkMomentumAcc.phi(track.value(),ParticleConstants::muonMassInMeV) : FLT_MAX},
              mass{(track.has_value()) ? ParticleConstants::muonMassInMeV : 0},
              uncalib_charge{charge},
              calib_charge{uncalib_charge},
              year{year},
              isData{isData},
              pars{(track.has_value()) ? acc.trkDefiningParametersAcc(track.value())
                                      : AmgVector(5)::Zero()},
              covariance{(track.has_value())
                             ? acc.trkDefiningParametersCovAcc(track.value())
                             : AmgSymMatrix(5)::Zero()} {}

        TrackCalibObj(const MuonCalibToolAccessors& acc, columnar::OptObjectId<columnar::MuonTrackDef> track, TrackType t, int charge,
                      double eta, double phi, DataYear year, bool isData)
            : type{t},
              is_valid{track.has_value()},
              uncalib_pt{(track.has_value()) ? acc.trkMomentumAcc.pt(track.value(),ParticleConstants::muonMassInMeV) * MeVtoGeV : 0},
              calib_pt{uncalib_pt},
              eta{eta},
              phi{phi},
              mass{(track.has_value()) ? ParticleConstants::muonMassInMeV : 0},
              uncalib_charge{charge},
              calib_charge{uncalib_charge},
              year{year},
              isData{isData},
              pars{(track.has_value()) ? acc.trkDefiningParametersAcc(track.value())
                                      : AmgVector(5)()},
              covariance{(track.has_value())
                             ? acc.trkDefiningParametersCovAcc(track.value())
                             : AmgSymMatrix(5)()} {}

        TrackCalibObj(TrackType t, int charge, double pt, double eta, double phi, double mass, AmgVector(5) pars, AmgSymMatrix(5) cov, DataYear year, bool isData):
          type{t},
          is_valid{true},
          uncalib_pt{pt*MeVtoGeV},
          calib_pt{uncalib_pt},
          eta{eta},
          phi{phi},
          mass{mass},
          uncalib_charge{charge},
          calib_charge{uncalib_charge},
          year{year},
          isData{isData},
          pars {pars},
          covariance{cov}
          {}

        TrackCalibObj(TrackType t, double pt, double eta, double phi, DataYear year, bool isData):
          type{t},
          is_valid{true},
          uncalib_pt{pt*MeVtoGeV},
          calib_pt{uncalib_pt},
          eta{eta},
          phi{phi},
          mass{-999},
          uncalib_charge{-999},
          calib_charge{-999},
          year{year},
          isData{isData},
          pars (AmgVector(5)::Zero()),
          covariance (AmgSymMatrix(5)::Zero())
          {}



        /// Flag telling the code whether this is CB/ME/ID
        const TrackType type{};
        /// Flag telling whether the track particle exists at all
        const bool is_valid{false};
        /// Value of the track-pt pre-calibration
        const double uncalib_pt{0.};
        /// Smeared track pt
        double calib_pt{0.};
        /// Value of the track-eta
        const double eta{0.};
        /// Value of the track-phi
        const double phi{0.};
        /// Value of the track-mass
        const double mass{0.};
        /// Value of the track-charge (before calibration)
        const int uncalib_charge{0};
        /// Value of the track-charge (after calibration)
        int calib_charge{0};
        // Data year
        const DataYear year{};
        // isData
        const bool isData{};

        /// Track perigee parameters.
        const AmgVector(5) pars{AmgVector(5)::Zero()};
        /// Full track covariance matrix
        const AmgSymMatrix(5) covariance{AmgSymMatrix(5)::Zero()};
    };


    struct MuonObj 
    {
        MuonObj(const TrackCalibObj& CB, const TrackCalibObj& ID, const TrackCalibObj& ME): ID{ID}, ME{ME}, CB{CB}{}
        
        TrackCalibObj ID{};
        TrackCalibObj ME{};
        TrackCalibObj CB{};

        /// Random numbers helping for the calibration
        double rnd_g0{0.};
        double rnd_g1{0.};
        double rnd_g2{0.};
        double rnd_g3{0.};
        double rnd_g4{0.};
        double rnd_g_highPt{0.};

        // the resolution category as given by the muon selection tool.
        // this isn't always read or used, so it is an `std::optional`
        // to generate an error if it is used without being set.
        using ResolutionCategory = CP::IMuonSelectionTool::ResolutionCategory;
        std::optional<ResolutionCategory> raw_mst_category;

        // Expected resolution number for statistical combination
        double expectedResID{0.};
        double expectedResME{0.};

        double expectedPercentResID{0.};
        double expectedPercentResME{0.};

        inline double getCalibpt(TrackType type) const
        {
            if(type == MCP::TrackType::CB)      return CB.calib_pt;
            else if(type == MCP::TrackType::ID) return ID.calib_pt;
            else if(type == MCP::TrackType::ME) return ME.calib_pt;
            return 0;
        }  
    };
} 
#endif
