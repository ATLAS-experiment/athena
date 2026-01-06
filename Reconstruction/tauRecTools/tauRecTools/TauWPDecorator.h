/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_TAUWPDECORATOR_H
#define TAURECTOOLS_TAUWPDECORATOR_H

#include "tauRecTools/TauRecToolBase.h"
#include "AsgTools/PropertyWrapper.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTau/TauDefs.h"
#include "xAODEventInfo/EventInfo.h"
#include "AsgDataHandles/ReadDecorHandleKey.h"
#include "AsgDataHandles/WriteDecorHandleKeyArray.h"

#include <utility>
#include <memory>
#include <vector>
#include <map>

class TH2;

/**
 * @brief Implementation of tool to decorate flattened BDT score and working points
 * 
 *  Input comes from ROOT files with lists of TH2s containing BDT/RNN score distributions
 *  as a function of the dependent variables. For eVeto, the score distributions depend on
 *  tau pT and |eta| of the leading track. Otherwise, the score distributions depend on
 *  tau pT and pileup.
 *
 * @author P.O. DeViveiros
 * @author W. Davey
 * @author L. Hauswald
 */

class TauWPDecorator : public TauRecToolBase {
  public:
    
    ASG_TOOL_CLASS2(TauWPDecorator, TauRecToolBase, ITauToolBase)
    
    /** @brief Constructor */
    TauWPDecorator(const std::string& name="TauWPDecorator");
    
    /** @brief Destructor */
    ~TauWPDecorator();

    /** @brief Initialization of this tool */
    virtual StatusCode initialize() override;

    /** @brief Executation of this tool */
    virtual StatusCode execute(xAOD::TauJet& tau) const override;
    
  private:

    /** 
     * @brief Retrieve the histograms containing BDT/RNN score distributions as a function of dependent variables 
     * @param nProng Prong of the tau candidate
     */
    StatusCode retrieveHistos(int nProng);

    /**
     * @brief Obtain the limit of the dependent variables 
     * @param nProng Prong of the tau candidate
     */ 
    StatusCode storeLimits(int nProng);

    /**
     * @brief Obtain the flattened score
     * @param score Original BDT/RNN score
     * @param cutLow Lower score cut
     * @param effLow Efficiency of the lower cut
     * @param cutHigh Higher score cut
     * @param effHigh Efficiency of the higher cut
     */ 
    double transformScore(double score, double cutLow, double effLow, double cutHigh, double effHigh) const;

    Gaudi::Property<bool> m_useAbsEta{this, "UseAbsEta", false, "Whether we are flatterning electron veto WP"};
    Gaudi::Property<bool> m_defineWPs{this, "DefineWPs", false, "Whether to decorate the WPs"};
    Gaudi::Property<std::string> m_scoreName{this, "ScoreName", "", "Name of the original score"};
    Gaudi::Property<std::string> m_scoreNameTrans{this, "NewScoreName", "", "Name of the transformed score"};  
    Gaudi::Property<std::string> m_file0p{this, "flatteningFile0Prong", "", "Calibration file name of 0-prong taus"};
    Gaudi::Property<std::string> m_file1p{this, "flatteningFile1Prong", "", "Calibration file name of 1-prong taus"};
    Gaudi::Property<std::string> m_file2p{this, "flatteningFile2Prong", "", "Calibration file name of 2-prong taus"};
    Gaudi::Property<std::string> m_file3p{this, "flatteningFile3Prong", "", "Calibration file name of 3-prong taus"}; 
    Gaudi::Property<std::vector<int>> m_EDMWPs{this, "CutEnumVals", {}, "Vector of WPs in the EDM"};
    Gaudi::Property<std::vector<float>> m_EDMWPEffs0p{this, "SigEff0P", {}, "Efficiency of each WP in EDM for 0-prong taus"};
    Gaudi::Property<std::vector<float>> m_EDMWPEffs1p{this, "SigEff1P", {}, "Efficiency of each WP in EDM for 1-prong taus"};
    Gaudi::Property<std::vector<float>> m_EDMWPEffs2p{this, "SigEff2P", {}, "Efficiency of each WP in EDM for 2-prong taus"};
    Gaudi::Property<std::vector<float>> m_EDMWPEffs3p{this, "SigEff3P", {}, "Efficiency of each WP in EDM for 3-prong taus"}; 
    Gaudi::Property<std::vector<std::string>> m_decorWPs{this, "DecorWPNames", {}, "Name of WPs"};
    Gaudi::Property<std::vector<float>> m_decorWPEffs0p{this, "DecorWPCutEffs0P", {}, "Efficiency of each WP to be docorated for 0-prong taus"};
    Gaudi::Property<std::vector<float>> m_decorWPEffs1p{this, "DecorWPCutEffs1P", {}, "Efficiency of each WP to be docorated for 1-prong taus"};
    Gaudi::Property<std::vector<float>> m_decorWPEffs2p{this, "DecorWPCutEffs2P", {}, "Efficiency of each WP to be docorated for 2-prong taus"};
    Gaudi::Property<std::vector<float>> m_decorWPEffs3p{this, "DecorWPCutEffs3P", {}, "Efficiency of each WP to be docorated for 3-prong taus"};        
    // for WPs not implemented in the EDM (i.e. not encoded in IsTauFlag)
    // use Accessors unless necessary
    std::vector<SG::Accessor<char>> m_charDecors;
    // when data handles are required (currently in tau trigger offline monitoring), need to use Write(Read)DecorHandleKeys
    // redundant with above accessors, will be improved in the future but has implications for DAOD workflow
    Gaudi::Property<std::string> m_tauContainerName{this, "TauContainerName", "", "Name of TauJetContainer, must be set when using "};
    SG::WriteDecorHandleKeyArray<xAOD::TauJetContainer> m_decorHandleKeys {this, "DecorHandleKeys",{},"Name of WPs to be decorated"};
   
    SG::ReadDecorHandleKey<xAOD::EventInfo> m_aveIntPerXKey {this, 
        "averageInteractionsPerCrossingKey", 
        "EventInfo.averageInteractionsPerCrossing",
        "Decoration for Average Interaction Per Crossing"};
    
    typedef std::pair<double, std::shared_ptr<TH2> > m_pair_t;

    std::shared_ptr<std::vector<m_pair_t>> m_hists0p; //!< Efficiency and corresponding score distributions of 0-prong taus
    std::shared_ptr<std::vector<m_pair_t>> m_hists1p; //!< Efficiency and corresponding score distributions of 1-prong taus
    std::shared_ptr<std::vector<m_pair_t>> m_hists2p; //!< Efficiency and corresponding score distributions of 2-prong taus
    std::shared_ptr<std::vector<m_pair_t>> m_hists3p; //!< Efficiency and corresponding score distributions of 3-prong taus
    
    std::map<int, double> m_xMin; //!< Map of n-prong and the minimum value of x variables
    std::map<int, double> m_yMin; //!< Map of n-prong and the minimum value of y variables
    std::map<int, double> m_xMax; //!< Map of n-prong and the maximum value of x variables
    std::map<int, double> m_yMax; //!< Map of n-prong and the maximum value of y variables
};

#endif // TAURECTOOLS_TAUWPDECORATOR_H
