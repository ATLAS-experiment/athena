/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETCALIBTOOLS_GENERIC4VECCORRECTION_H
#define JETCALIBTOOLS_GENERIC4VECCORRECTION_H

#include <memory>
#include <vector>
#include "TString.h"
#include "TH2.h"

#include "JetCalibTools/JetCalibrationStep.h"

#include <nlohmann/json.hpp>

class TEnv;
class Generic4VecCorrection
    : virtual public JetCalibrationStep
{

  public:

    // Enums for corrections supported by this tool
    enum JET_CORRTYPE{
        UNKNOWN = 0,
        PTRESIDUAL = 1,
        MC2MC = 2,
        FASTSIM = 3
    };

    // Constructor/destructor/init
    Generic4VecCorrection();
    Generic4VecCorrection(const std::string& name, TEnv* config, TString jetAlgo, TString calibAreaTag, TString forceCalibFile, JET_CORRTYPE correctionType, TString mcCampaign="", TString simFlavour="", int mcDSID=-1, TString generatorsInfo="");
    virtual ~Generic4VecCorrection();
    virtual StatusCode initialize() override;
    virtual StatusCode calibrate(xAOD::Jet& jet, JetEventInfo&) const override;

  private:
    // Extract correction value from chosen 2D histogram
    StatusCode readHisto(float& correctionFactor, TH2* h_correction_2D, float x, float y) const;

    // Dedicated initialize functions for supported corrections
    StatusCode initialize_correctionResponse();
    StatusCode initialize_MC2MC();

    // For MC2MC, parse the calibration showerModel from sample metadata
    StatusCode parse_showerModel(TString& showerModel, int mcDSID, TString generatorsInfo) const;
    
    StatusCode load_json(nlohmann::json& json_object, std::string json_filepath) const;

    // Class variables from constructor
    TEnv* m_config;
    const TString m_jetAlgo;
    const TString m_calibAreaTag;
    JET_CORRTYPE m_correctionType;
    const TString m_simFlavour;

    // Variables for MC2MC Correction
    int m_mcDSID;
    const TString m_generatorsInfo;
    const TString m_mcCampaign;
    const TString m_forceCalibFile; 

    // Option to skip correction if input file does not conform to requested correction
    bool m_skipCorrection;

    // Input and output jet scales
    TString m_inJetScale;
    TString m_outJetScale;

    // Correction histograms
    TString m_correctionFilePath;
    std::map<int, TH2*> m_correctionHists;  // If several possible corrections
    TH2* m_only_correction_2D{};            // If only one correction

};

#endif