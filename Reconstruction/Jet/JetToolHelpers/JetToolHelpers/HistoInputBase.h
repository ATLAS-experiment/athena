/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETTOOLHELPERS_HISTOINPUTBASE_H
#define JETTOOLHELPERS_HISTOINPUTBASE_H


#include "JetAnalysisInterfaces/IVarTool.h"
#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"

#include "TH1.h" //unique_ptr member, and d'tor defined here
#include <string>
#include <memory>

class TAxis;

namespace JetHelper {

    /// Class HistoInputBase
    /// This class implement common function used by HistoInput1D and HistoInput2D 

class HistoInputBase :public asg::AsgTool, virtual public IVarTool
{
    ASG_TOOL_CLASS(HistoInputBase, IVarTool)

    public:
        /// Constructor for standalone usage
        HistoInputBase(const std::string& myname);
        /// Destructor
        virtual ~HistoInputBase() {};
        /// Return the name of the file that containt the histograms
        std::string getFileName() const { return m_fileName; };
        /// Return the name of the histogram 
        std::string getHistName() const { return m_histName; };
	/// Returns the underlying histogram
        TH1& getHistogram() { return *m_hist; };
 
    private:
        /// path to the file with histrograms
        Gaudi::Property< std::string > m_fileName { this, "inputfile", "JetUncertainties/CalibArea-08/rel21/Summer2019/R4_AllComponents.root", "File containing histograms" };
        /// name of the histogram
        Gaudi::Property< std::string > m_histName { this, "histName", "Zjet_MuStat3_AntiKt4EMPFlow", "name of histrogram" };

    protected:
        std::unique_ptr<TH1> m_hist ATLAS_THREAD_SAFE {};
        /// This function open the InputFile and assign the histogram to m_hist
        bool readHistoFromFile();
        /// This function ensure that the histogram is NOT evaluated outside range
        double enforceAxisRange(const TAxis& axis, const double inputValue) const;
        /// This function evaluate the histogram using the TH1::Interpolate
        double readFromHisto(const double X, const double Y=0, const double Z=0) const;
        // Interpolation strategies
        Gaudi::Property<std::string> m_interpStr {this, "InterpType", "Full", "Interpolation to run: Full/OnlyX/OnlyY/None"};
        enum class InterpType
        {
            UNKNOWN=0,  // Unknown/unset/etc
            Full,       // Full interpolation
            None,       // No interpolation
            OnlyX,      // Interpolate only in the x dimension
            OnlyY       // Interpolate only in the y dimension
        };
        InterpType m_interpNum{InterpType::UNKNOWN};
    
};
} // namespace JetHelper
#endif
