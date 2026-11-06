/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_GLOBALJet_TagALGTOOL_H
#define GLOBALSIM_GLOBALJet_TagALGTOOL_H

/*
  This algorithm simulates the large-R jet tagger (Jet_Tag) for the Global
  Trigger. It uses the same headers from TrigGepPerf.
  Input is taken only from the Jet1 (WTA cone) jets produced by the
  GlobalJet1AlgTool.

  Only the basic (v2) algorithm is implemented, which is the version
  integrated into the firmware chain.

  Input and output TOBs both carry digitized integer codes, so this runs the
  bitwise core of the tagger (Gep::JetTaggerLRJMaker) directly and never
  constructs a floating point quantity.
*/

#include "AthenaBaseComps/AthAlgTool.h"

#include "../GlobalSimComponents/IGlobalSimAlgTool.h"
#include "../IO/CommonTOBContainer.h"
#include "../IO/Jet1TOB.h"
#include "../Utilities/IDataCollector.h"

#include "TrigGepPerf/JetTaggerLRJMaker.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include <string>

namespace GlobalSim {

  class GlobalJet_TagAlgTool: public extends<AthAlgTool, IGlobalSimAlgTool> {

  public:

    /** @brief Main constructor */
    GlobalJet_TagAlgTool(const std::string& type, const std::string& name, const IInterface* parent);

    /** @brief Main destructor (explicitly defaulted) */
    ~GlobalJet_TagAlgTool() override = default;

    /** @brief Initialize function running before first event */
    virtual StatusCode initialize() override;

    /** @brief Main functional block running for each event */
    virtual StatusCode run(const std::unique_ptr<IDataCollector>&,
			   const EventContext& ctx) const override;

    /** @brief Overriding toString function from base class */
    virtual std::string toString() const override;

  private:

    /** @brief Read key for the input Jet1Jets as a Jet1TOBContainer */
    SG::ReadHandleKey<IOBitwise::Jet1TOBContainer>
    m_gblJet1JetsContainerKey {
        this,
	"GlobalJet1JetsKey",
	"GlobalJet1Jets",
	"Key to the container of generic TOBS containing the Jet1Jets"};

    /** @brief Write key for the output Jet_TagJets as a GenericTobContainer */
    SG::WriteHandleKey<IOBitwise::CommonTOBContainer>
    m_gblJet_TagJetsContainerKey {
        this,
	"GlobalJet_TagJetsKey",
	"GlobalJet_TagJets",
	"Key to the container of generic TOBS containing the Jet_TagJets"};

    // Algorithm parameters:
    // Defaults are the BasicV2 preset from TrigGepPerf's GepJetAlgConfig.py
    Gaudi::Property<unsigned int> m_nSeedsInput{this, "NSeedsInput", 10,
        "Leading small-R jets considered as seeds (LRJNSeedsInput)."};
    Gaudi::Property<unsigned int> m_nSeedsOutput{this, "NSeedsOutput", 2,
        "Large-R jets emitted per event - Multiplicity is fixed to 2, "
        "with events with <= 1 WTA-cone jets being zero-padded."};
    Gaudi::Property<unsigned int> m_maxObjectsConsidered{this, "MaxObjectsConsidered", 8,
        "Small-R jets considered as constituents (LRJMaxObjectsConsidered)."};
    Gaudi::Property<float> m_jetR{this, "JetR", 1.1,
        "Large-R jet radius, nominally 1.1"};

    /** @brief The bitwise tagger, configured once in initialize() */
    Gep::JetTaggerLRJMaker m_maker;
  };

} // namespace GlobalSim

#endif
