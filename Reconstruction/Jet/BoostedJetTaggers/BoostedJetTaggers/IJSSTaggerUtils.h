// IJSSTaggerUtils.h

#ifndef IJSSTaggerUtils_H
#define IJSSTaggerUtils_H

#include "AsgTools/IAsgTool.h"

class IJSSTaggerUtils : virtual public asg::IAsgTool {
  ASG_TOOL_INTERFACE(IJSSTaggerUtils)

    public:

    virtual TH2D MakeJetImage(TString TagImage, const xAOD::Jet* jet, std::vector<xAOD::JetConstituent> constituents) const = 0;
    virtual StatusCode GetImageScore(const xAOD::JetContainer& jets) const = 0;
    virtual StatusCode GetConstScore(const xAOD::JetContainer& jets) const = 0;
    virtual StatusCode GetQGConstScore(const xAOD::JetContainer& jets) const = 0;
    virtual StatusCode GetTopConstScore(const xAOD::JetContainer& jets) const = 0;
    virtual StatusCode GetWConstScore(const xAOD::JetContainer& jets) const = 0;
    virtual StatusCode GetHLScore(const xAOD::JetContainer& jets) const = 0;
    virtual std::map<std::string, double> GetJSSVars(const xAOD::Jet& jet) const = 0;
    virtual StatusCode ReadScaler() = 0;

};

#endif
