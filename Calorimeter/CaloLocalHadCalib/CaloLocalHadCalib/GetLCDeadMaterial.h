/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOLOCALHADCALIB_GETLCDEADMATERIAL_H
#define CALOLOCALHADCALIB_GETLCDEADMATERIAL_H

/**
 * @class GetLCDeadMaterial
 * @version \$Id: GetLCDeadMaterial.h,v 1.1 2009-03-03 17:30:21 pospelov Exp $
 * @author Gennady Pospelov <guennadi.pospelov@cern.ch>
 * @date 23-Januar-2009
 * @brief Top algorithm to final processing of dead material tree and producing of dead material constants.
 *
 * It runs sequentially following algorithms: CaloHadDMCoeffFit to fit TProfiles histograms,
 * CaloHadDMCoeffMinim to run minimisation, then it saves constants into pool and text files,
 * and finally it runs toy reconstruction to produce dmreco .vs. dmtruth plots
 *
 */

#include "AthenaBaseComps/AthAlgorithm.h"
#include "Gaudi/Property.h"
#include <vector>
#include <string>
#include "CxxUtils/checker_macros.h"

class TChain;
class TProfile;
class CaloLocalHadCoeff;


class ATLAS_NOT_THREAD_SAFE GetLCDeadMaterial : public AthAlgorithm {

  public:

    GetLCDeadMaterial(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~GetLCDeadMaterial();
    virtual StatusCode initialize();
    virtual StatusCode execute(const EventContext& ctx);
    virtual StatusCode finalize();

  private:

    /**
     * @brief Name of text file with initial parameters for coefficients calculation
     */
    Gaudi::Property<std::string> m_HadDMCoeffInputFile{this, "HadDMCoeffInputFile", "CaloHadDMCoeff_init_v2.txt", "Name of text file with initial parameters for coefficients calculation"};

    /**
     * @brief Name of text file with calculated coefficients
     */
    Gaudi::Property<std::string> m_HadDMCoeffOutputFile{this, "HadDMCoeffOutputFile", "CaloHadDMCoeff_output.txt", "Name of text file with calculated coefficients"};

    /**
     * @brief Name of postscript file for fit summary
     */
    Gaudi::Property<std::string> m_reportProfiles{this, "ReportProfiles", "report_CaloHadDMCoeff_profiles.ps", "Name of postscript file for fit summary"};

    /**
     * @brief Name of postscript file for minimization summary
     */
    Gaudi::Property<std::string> m_reportMinimization{this, "ReportMinimization", "report_CaloHadDMCoeff_minim.ps", "Name of postscript file for minimization summary"};

    /**
     * @brief Name of postscript file for results of toy reconstruction
     */
    Gaudi::Property<std::string> m_reportCheck{this, "ReportCheck", "report_CaloHadDMCoeff_check.ps", "Name of postscript file for results of toy reconstruction"};

    /**
     * @brief Do TProfile fitting 
     */
    Gaudi::Property<bool> m_doFit{this, "DoFit", true, "Do TProfile fitting"};

    /**
     * @brief Do minimization 
     */
    Gaudi::Property<bool> m_doMinimization{this, "DoMinimization", false, "Do minimization"};

    /**
     * @brief Do pool writing
     */
    Gaudi::Property<bool> m_doPool{this, "DoPool", true, "Do pool writing"};

    /**
     * @brief Do toy reconstruction for final validation of new dead material constants
     */
    Gaudi::Property<bool> m_doCheck{this, "DoCheck", true, "Do toy reconstruction for final validation of new dead material constants"};

    /**
     * @brief List of root files to process 
     */
    Gaudi::Property<std::vector<std::string> > m_inputRootFiles{this, "InputRootFiles", {}, "List of root files to process"};

    /**
     * @brief Name of key to write into pool file
     */
    Gaudi::Property<std::string> m_key{this, "CorrectionKey", "HadDMCoeff2", "Name of key to write into pool file"};

    /**
     * @brief H6 combined testbeam flag
     */
    Gaudi::Property<bool> m_isTestbeam{this, "isTestbeam", false, "H6 combined testbeam flag"};

    /**
     * @brief data sample contains single particles only
     */
    bool m_isSingleParticle{true};

    /**
     * @brief Normalization type for fit procedure
     */
    Gaudi::Property<std::string> m_NormalizationTypeForFit{this, "NormalizationTypeForFit", "Lin", "Normalization type for fit procedure"};

    /**
     * @brief Normalization type for minimization procedure
     */
    Gaudi::Property<std::string> m_NormalizationTypeForMinim{this, "NormalizationTypeForMinim", "Lin", "Normalization type for minimization procedure"};

    /**
     * @brief use particle id information
     */
    std::string m_ClassificationType{"pdg"};

    /**
     * @brief number of maximum events to process in one root files
     */
    Gaudi::Property<int> m_MaxEventsPerFile{this, "MaxEventsPerFile", 0, "Number of maximum events to process in one root files"};

};

#endif // CALOLOCALHADCALIB_GETLCDEADMATERIAL_H

