// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATRACKSIMEVENTSELECTIONTOOL_H
#define FPGATRACKSIMEVENTSELECTIONTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "xAODTruth/TruthParticle.h"
#include "GeneratorObjects/HepMcParticleLink.h"
#include "GeneratorObjects/xAODTruthParticleLink.h"

#include "FPGATrackSimConfTools/IFPGATrackSimEventSelectionSvc.h"
#include "FPGATrackSimObjects/FPGATrackSimTruthTrackCollection.h"


namespace FPGATrackSim {
    class FPGATrackSimEventSelectionTool : public AthAlgTool {
    private:
        // Handles
        ServiceHandle<IFPGATrackSimEventSelectionSvc> m_evtSel {this, "evtSelectionService", "", "Event selection Svc"};

    public:
        FPGATrackSimEventSelectionTool(const std::string&, const std::string&, const IInterface*);
        virtual ~FPGATrackSimEventSelectionTool() = default;

        virtual StatusCode initialize() override;
    
        bool selectEvent(const FPGATrackSimEventInputHeader & header) const;   
        void setSelectedEvent(bool s) { m_evtSel->setSelectedEvent(s); }
        unsigned getRegionID() const { return m_evtSel->getRegionID(); }
    };
}
#endif