/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * dual-use tool for converting xAOD truth events back to HepMC
 * Principal Authors (responsible for the core conversion algorithm): Josh McFayden and James Catmore
 * Tool Author: Jim Lacey (Carleton University) 
 * ... updated tool interface to be dual-use
 * ... added missing PDF information and requirements to allow running on full sim (remove Geant4 secondaries)
 * <james.lacey@cern.ch,jlacey@physics.carleton.ca>
 * <dag.gillberg@cern.ch>
 */

#ifndef TRUTHCONVERTERS_XAODTOHEPMCTOOL_H
#define TRUTHCONVERTERS_XAODTOHEPMCTOOL_H 1

#include "AsgTools/AsgTool.h"
#include "GenInterfaces/IxAODtoHepMCTool.h"


class xAODtoHepMCTool: public asg::AsgTool, public virtual IxAODtoHepMCTool { 
public: 
    ASG_TOOL_CLASS( xAODtoHepMCTool , IxAODtoHepMCTool )
    xAODtoHepMCTool( const std::string& name );
    virtual ~xAODtoHepMCTool () { };

    virtual StatusCode  initialize() override;
    StatusCode finalize () override;

public:
    std::vector<HepMC::GenEvent> getHepMCEvents(const xAOD::TruthEventContainer* xTruthEventContainer, const xAOD::EventInfo* eventInfo) const override;

private:
    HepMC::GenEvent createHepMCEvent(const xAOD::TruthEvent* xEvt, const xAOD::EventInfo* eventInfo) const;
    HepMC::GenVertexPtr vertexHelper(const xAOD::TruthVertex*,std::map<const xAOD::TruthVertex*,HepMC::GenVertexPtr>&,bool&) const;
    HepMC::GenParticlePtr createHepMCParticle(const xAOD::TruthParticle*) const;
    HepMC::GenVertexPtr createHepMCVertex(const xAOD::TruthVertex*) const;
    void printxAODEvent(const xAOD::TruthEvent* event, const xAOD::EventInfo* eventInfo) const;
   
private:
    /// Input container key (job property)
    Gaudi::Property<float> m_momFac {
            this, "MomentFactor", 0.001,
            "Scale factor to be applied to truth energy and momenta to convert from MeV, e.g. 0.001 to go to GeV"};
    Gaudi::Property<float> m_lenFac {
            this, "LengthFactor", 1.0,
            "Scale factor to be applied to truth lengths to convert from mm, e.g. 0.01 to go to cm"};
    Gaudi::Property<bool> m_signalOnly {
            this, "SignalOnly", true,
            "Convert only the signal event (true), or all events found"};
    Gaudi::Property<int> m_maxCount {
            this, "PrintNevents", 0,
            "Maximum number of events to print out"};
    /// Counters
    //not sure if these need to be atomic but just in case this will run in MT
    mutable std::atomic<int> m_evtCount{};
    mutable std::atomic<int> m_badSuggest{};
    int m_noProdVtx{};
    int m_badBeams{};

}; 

#endif //> !XAODTOHEPMC_XAODTOHEPMCTOOL_H
