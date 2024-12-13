/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETTOOLHELPERS_MCJESINPUTBASE_H
#define JETTOOLHELPERS_MCJESINPUTBASE_H


#include "TString.h"
#include "TEnv.h"

#include "JetAnalysisInterfaces/IVarTool.h"
#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"
#include <memory>
#include <string>
class TAxis;

namespace JetHelper {

    /// Class MCJESInputBase
    /// This class implement common function used by Text file input 

class MCJESInputBase :public asg::AsgTool, virtual public IVarTool
{
    ASG_TOOL_CLASS(MCJESInputBase, IVarTool)

    public:
        /// Constructor for standalone usage
        MCJESInputBase(const std::string& myname);
        /// Destructor
        virtual ~MCJESInputBase() {};
        /// This function return the name of the file with the MCJES factors
        std::string getFileName() const { return m_fileName; };
        /// This property allow the user to choose the type of correction
        Gaudi::Property< std::string > m_corrName { this, "corrName", "JES", "MCJES input" };
   
    private:
        // support functions to extract information from text files
        double getLogPolN(const double *factors, double x) const;
        std::vector<double> VectorizeD(const TString& str, TString sep=" ") const;
        int getEtaBin(double eta_det) const; 
        /// name of the text file
        Gaudi::Property< std::string > m_fileName { this, "inputfile", "/afs/cern.ch/work/s/stapiaar/JetDev4/athena/JetToolHelpers/data/file_JES.config", "File containing histograms" };
        /// jet collection to be calibrated
        Gaudi::Property< std::string > m_jetAlgo { this, "jetAlgo", "AntiKt4EMPFlow", "jet collection" };

    protected:
        std::unique_ptr<TEnv> m_config;
        unsigned int m_nPar{}; // number of parameters in config file
        const static unsigned int s_nEtaBins = 90;
        const static unsigned int s_nParMax = 9; 
        double m_JESFactors[s_nEtaBins][s_nParMax]{};
        double m_etaCorrFactors[s_nEtaBins][s_nParMax]{};
        double m_energyFreezeJES[s_nEtaBins]{};
        TAxis * m_etaBinAxis{};
        
        /// This function fill the ntuples with calibration factors 
        bool readMCJESFromText();
        /// return MCJES calibration factor
        double getJES(const double X, const double Y=0, const double Emax=-1) const;
        /// return Eta correction
        double getEtaCorr(const double X, const double Y=0) const;
        /// return Emax
        double getEmaxJES(const double Y) const;
};
} // namespace JetHelper
#endif
