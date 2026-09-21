/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_GLOBALMETALGTOOL_H
#define GLOBALSIM_GLOBALMETALGTOOL_H

/*
  This algorithm simulates the GEP MET algorithm (Total MET) for the Global Trigger.
  It uses the same headers from TrigGepPerf.

  Total MET is the scale-factor-weighted combination of a tower term, built from the
  cell towers produced by GlobalCellTowerAlgTool, and a jet term, built from the Jet1
  (WTA cone) jets produced by GlobalJet1AlgTool.

  Input and output TOBs both carry digitized integer values, so this runs the bitwise
  core (Gep::TotalMETMaker::makeMETDigitized) directly and never constructs a floating
  point quantity.

  Unlike every other GlobalSim AlgTool so far, this one takes TWO inputs: it needs the
  towers and the jets built from those same towers.

  Only total MET is emitted, one TOB per event, matching the single word MET_Engine.v
  produces. The jet and tower terms are still computed internally, since total MET is
  their weighted sum; they are simply not written out.
*/

#include "AthenaBaseComps/AthAlgTool.h"

#include "../GlobalSimComponents/IGlobalSimAlgTool.h"
#include "../IO/CommonTOBContainer.h"
#include "../IO/Jet1TOB.h"
#include "../IO/METTOB.h"
#include "../Utilities/IDataCollector.h"

#include "TrigGepPerf/TotalMETMaker.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include <string>

namespace GlobalSim {

  class GlobalMETAlgTool: public extends<AthAlgTool, IGlobalSimAlgTool> {

  public:

    /** @brief Main constructor */
    GlobalMETAlgTool(const std::string& type, const std::string& name, const IInterface* parent);

    /** @brief Main destructor (explicitly defaulted) */
    ~GlobalMETAlgTool() override = default;

    /** @brief Initialize function running before first event */
    virtual StatusCode initialize() override;

    /** @brief Main functional block running for each event */
    virtual StatusCode run(const std::unique_ptr<IDataCollector>&,
			   const EventContext& ctx) const override;

    /** @brief Overriding toString function from base class */
    virtual std::string toString() const override;

  private:

    /** @brief Read key for the input cell towers as a CommonTOBContainer */
    SG::ReadHandleKey<IOBitwise::CommonTOBContainer>
    m_gblCellTowersKey {
        this,
	"GlobalCellTowersKey",
	"GlobalCellTowers",
	"Key to the container of generic TOBS containing the cell towers"};

    /** @brief Read key for the input Jet1Jets as a Jet1TOBContainer */
    SG::ReadHandleKey<IOBitwise::Jet1TOBContainer>
    m_gblJet1JetsContainerKey {
        this,
	"GlobalJet1JetsKey",
	"GlobalJet1Jets",
	"Key to the container of generic TOBS containing the Jet1Jets"};

    /** @brief Write key for the output MET as a METTOBContainer */
    SG::WriteHandleKey<IOBitwise::METTOBContainer>
    m_gblMETContainerKey {
        this,
	"GlobalMETKey",
	"GlobalMET",
	"Key to the container of TOBS containing the total MET"};

    // Algorithm parameters.
    // Defaults are those of TrigGepPerf's GepTotalMETAlgCfg, which is the same
    // algorithm driven from floating point input.
    //
    // The jet cone radius and the jet multiplicity are deliberately NOT properties here.
    // Both belong to the upstream WTACone/JET1 algorithm that produced the jets -- the
    // radius is its Jet_dR and the multiplicity is how many jets it emits -- so MET takes
    // what it is given rather than declaring its own and risking disagreement.
    Gaudi::Property<unsigned int> m_maxTowersConsidered{this, "MaxTowersConsidered", 4096,
        "Towers considered per event."};
    Gaudi::Property<float> m_jetEtThresholdGeV{this, "JetEtThresholdGeV", 0.0,
        "Minimum jet E_T (GeV) entering the jet MET sum."};
    Gaudi::Property<float> m_towerEtThresholdGeV{this, "TowerEtThresholdGeV", 0.0,
        "Minimum tower E_T (GeV) entering the tower MET sum."};
    Gaudi::Property<bool> m_doJetTowerOverlapRemoval{this, "DoJetTowerOverlapRemoval", false,
        "Drop towers within the upstream jet cone radius of a jet that passed JetEtThresholdGeV."};
    Gaudi::Property<float> m_towerScaleFactor{this, "TowerScaleFactor", 1.0,
        "Scalar weight on the tower term in the total."};
    Gaudi::Property<float> m_jetScaleFactor{this, "JetScaleFactor", 1.0,
        "Scalar weight on the jet term in the total."};

    /** @brief The bitwise MET core, configured once in initialize() */
    Gep::TotalMETMaker m_maker;
  };

} // namespace GlobalSim

#endif
