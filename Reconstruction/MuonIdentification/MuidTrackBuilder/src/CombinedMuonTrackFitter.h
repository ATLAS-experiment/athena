/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////////////////
// CombinedMuonTrackFitter
//  AlgTool gathering  material effects along a combined muon track, in
//  particular the TSOS'es representing the calorimeter energy deposit and
//  Coulomb scattering.
//  The resulting track is fitted at the IP
//
//////////////////////////////////////////////////////////////////////////////
#ifndef MUIDTRACKBUILDER_COMBINEDMUONTRACKFITTER_H
#define MUIDTRACKBUILDER_COMBINEDMUONTRACKFITTER_H

#include <atomic>

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"
#include "MuidInterfaces/ICombinedMuonTrackFitter.h"
#include "MuidInterfaces/IMuidCaloTrackStateOnSurface.h"
#include "MuidInterfaces/IMuonTrackQuery.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRecHelperTools/MuonEDMPrinterTool.h"
#include "MuonRecToolInterfaces/IMuonErrorOptimisationTool.h"
#include "MuonRecToolInterfaces/IMuonTrackCleaner.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "TrkDetDescrInterfaces/ITrackingVolumesSvc.h"
#include "TrkFitterInterfaces/ITrackFitter.h"
#include "TrkGeometry/TrackingGeometry.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkToolInterfaces/ITrackSummaryTool.h"
#include "TrkToolInterfaces/ITrkMaterialProviderTool.h"
#include "TrkTrack/TrackInfo.h"
#include "MuonRecHelperTools/IMuonEDMHelperSvc.h"
#include "TrkiPatFitterUtils/MessageHelper.h"


namespace Rec {

    class CombinedMuonTrackFitter : public AthAlgTool, virtual public ICombinedMuonTrackFitter {
    public:
        CombinedMuonTrackFitter(const std::string& type, const std::string& name, const IInterface* parent);
        virtual ~CombinedMuonTrackFitter();

        virtual StatusCode initialize() override;
        virtual StatusCode finalize() override;

        /*refit a track */
        virtual std::unique_ptr<Trk::Track> fit(const EventContext& ctx, const Trk::Track& track, const Trk::RunOutlierRemoval runOutlier,
                                                const Trk::ParticleHypothesis particleHypothesis) const override;
    protected:
        /**
            fit a set of MeasurementBase objects with starting value for perigeeParameters */
        std::unique_ptr<Trk::Track> fit(const EventContext& ctx, const Trk::MeasurementSet& measurementSet,
                                        const Trk::TrackParameters& perigeeStartValue, const Trk::RunOutlierRemoval runOutlier,
                                        const Trk::ParticleHypothesis particleHypothesis) const;

        /**
            combined muon fit */
        std::unique_ptr<Trk::Track> fit(const EventContext& ctx, const Trk::Track& indetTrack, Trk::Track& extrapolatedTrack,
                                        const Trk::RunOutlierRemoval runOutlier,
                                        const Trk::ParticleHypothesis particleHypothesis) const;
    
        double normalizedChi2(const Trk::Track& track) const;

        
        bool checkTrack(std::string_view txt, const Trk::Track* newTrack) const;

   
        unsigned int countAEOTs(const Trk::Track& track, const std::string& txt) const;

        bool loadMagneticField(const EventContext& ctx, MagField::AtlasFieldCache& field_cache) const;

    private:
    
        bool optimizeErrors(const EventContext& ctx, Trk::Track& track) const;
   
        // helpers, managers, tools
        ToolHandle<Muon::IMuonTrackCleaner> m_cleaner{
            this,
            "Cleaner",
            "Muon::MuonTrackCleaner/MuidTrackCleaner",
        };       
        ToolHandle<Trk::ITrackFitter> m_fitter{
            this,
            "Fitter",
            "Trk::iPatFitter/iPatFitter",
        };  // curved track fitter
        ToolHandle<Trk::ITrackFitter> m_fitterSL{
            this,
            "SLFitter",
            "Trk::iPatFitter/iPatSLFitter",
        };  // straight line fitter
    
    protected:
        ToolHandle<Rec::IMuidCaloTrackStateOnSurface> m_caloTSOS{
            this,
            "CaloTSOS",
            "",
        };       
        ToolHandle<Muon::IMuonErrorOptimisationTool> m_muonErrorOptimizer{
            this,
            "MuonErrorOptimizer",
            "",
        };
        PublicToolHandle<Muon::MuonEDMPrinterTool> m_printer{
            this,
            "Printer",
            "Muon::MuonEDMPrinterTool/MuonEDMPrinterTool",
        };
        ToolHandle<Rec::IMuonTrackQuery> m_trackQuery{
            this,
            "TrackQuery",
            "Rec::MuonTrackQuery/MuonTrackQuery",
        };
        ToolHandle<Trk::ITrackSummaryTool> m_trackSummary{
            this,
            "TrackSummaryTool",
            "Trk::TrackSummaryTool/MuidTrackSummaryTool",
        };
        ToolHandle<Trk::ITrkMaterialProviderTool> m_materialUpdator{
            this,
            "CaloMaterialProvider",
            "",
        };
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    private:
        // // Read handle for conditions object to get the field cache
        SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCacheCondObjInputKey{this, "AtlasFieldCacheCondObj", "fieldCondObj",
                                                                                  "Name of the Magnetic Field conditions object key"};

        ServiceHandle<Trk::ITrackingVolumesSvc> m_trackingVolumesSvc{this, "TrackingVolumesSvc", "Trk::TrackingVolumesSvc/TrackingVolumesSvc"};

       
        Gaudi::Property<bool> m_allowCleanerVeto{this, "AllowCleanerVeto", true};       
        Gaudi::Property<unsigned> m_maxWarnings{this, "MaxNumberOfWarnings", 10,
                                                "Maximum number of permitted WARNING messages per message type."};
       
    protected:
        Gaudi::Property<double> m_badFitChi2{this, "BadFitChi2", 2.5};       

        Gaudi::Property<double> m_zECToroid{this, "zECToroid", 10. * Gaudi::Units::meter};
        Gaudi::Property<bool> m_updateWithCaloTG{this, "UpdateWithCaloTG", false};
        Gaudi::Property<bool> m_useCaloTG{this, "UseCaloTG", false};
   
        // // constants
        std::unique_ptr<const Trk::Volume> m_indetVolume{nullptr};
        std::unique_ptr<const Trk::Volume> m_calorimeterVolume{nullptr};

        // // count warnings
        std::unique_ptr<MessageHelper> m_messageHelper{std::make_unique<MessageHelper>(*this, 50)};
       
    private:
        mutable std::atomic_uint m_countCombinedCleanerVeto{0};
        mutable std::atomic_uint m_countExtensionCleanerVeto{0};
        mutable std::atomic_uint m_countStandaloneCleanerVeto{0};
    };  // end of class CombinedMuonTrackFitter

}  // end of namespace Rec

#endif  // MUIDTRACKBUILDER_COMBINEDMUONTRACKBUILDER_H
