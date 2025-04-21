/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_TAUWPDECORATOR_H
#define TAURECTOOLS_TAUWPDECORATOR_H

#include "tauRecTools/TauRecToolBase.h"

#include "xAODTau/TauDefs.h"
#include "xAODEventInfo/EventInfo.h"
#include "AsgDataHandles/ReadDecorHandleKey.h"
#include "AsgTools/PropertyWrapper.h"

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

    // properties 
    Gaudi::Property<bool> m_useAbsEta{this, "UseAbsEta", false};
    Gaudi::Property<bool> m_defineWPs{this, "DefineWPs", false};
    Gaudi::Property<std::string> m_scoreName{this, "ScoreName", ""};
    Gaudi::Property<std::string> m_scoreNameTrans{this, "NewScoreName", ""}; 
    Gaudi::Property<std::string> m_file0p{this, "flatteningFile0Prong", ""};
    Gaudi::Property<std::string> m_file1p{this, "flatteningFile1Prong", ""};
    Gaudi::Property<std::string> m_file2p{this, "flatteningFile2Prong", ""};
    Gaudi::Property<std::string> m_file3p{this, "flatteningFile3Prong", ""}; 
    Gaudi::Property<std::vector<int>> m_EDMWPs{this, "CutEnumVals", {}};
    Gaudi::Property<std::vector<float>> m_EDMWPEffs0p{this, "SigEff0P", {}};
    Gaudi::Property<std::vector<float>> m_EDMWPEffs1p{this, "SigEff1P", {}};
    Gaudi::Property<std::vector<float>> m_EDMWPEffs2p{this, "SigEff2P", {}};
    Gaudi::Property<std::vector<float>> m_EDMWPEffs3p{this, "SigEff3P", {}};
    Gaudi::Property<std::vector<std::string>> m_decorWPs{this, "DecorWPNames", {}};
    Gaudi::Property<std::vector<float>> m_decorWPEffs0p{this, "DecorWPCutEffs0P", {}};
    Gaudi::Property<std::vector<float>> m_decorWPEffs1p{this, "DecorWPCutEffs1P", {}};
    Gaudi::Property<std::vector<float>> m_decorWPEffs2p{this, "DecorWPCutEffs2P", {}};
    Gaudi::Property<std::vector<float>> m_decorWPEffs3p{this, "DecorWPCutEffs3P", {}};

    std::vector<SG::Accessor<char>> m_charDecors; //!
    
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
