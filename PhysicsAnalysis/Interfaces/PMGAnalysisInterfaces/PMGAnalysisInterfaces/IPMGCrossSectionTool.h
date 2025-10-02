// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PMGANALYSISINTERFACES_IPMGCROSSSECTIONTOOL_H
#define PMGANALYSISINTERFACES_IPMGCROSSSECTIONTOOL_H

// Infrastructure include(s):
#include "AsgTools/IAsgTool.h"

#include <vector>

namespace PMGTools {
  
  // store all information for certain DSID in structure
  struct AllSampleInfo{
    int dsid = 0;
    std::string containerName;
    double amiXSec = 0;
    double filterEff = 0;
    double kFactor = 0;
    double XSecUncUP = 0;
    double XSecUncDOWN = 0;
    int etag = 0;
    double br = 0;
    double higherOrderXsecTotal = 0;
    double higherOrderXsecSample = 0;
  };

    
  class IPMGCrossSectionTool : public virtual asg::IAsgTool { 
    
    // Declare the interface that the class provides
    ASG_TOOL_INTERFACE( PMGTools::IPMGCrossSectionTool )
    
    public:
    
    /// read infos from file, store them in the structure and make a vector that keeps all of them 
    virtual bool readInfosFromFiles(const std::vector<std::string> &) = 0;
    
    /// read infos from all files in dir 
    virtual bool readInfosFromDir(const std::string& inputDir) = 0;
    
    /// return filter efficiency for DSID
    virtual double getFilterEff(const int dsid, const int etag = -1) const = 0;
    
    /// return the sample name for DSID
    virtual std::string getSampleName(const int dsid, const int etag = -1) const = 0;
    
    /// return the AMI cross-section for DSID
    virtual double getAMIXsection(const int dsid, const int etag = -1) const = 0;

    /// return the cross-section uncertainty for DSID       
    virtual double getXsectionUncertainty(const int dsid, const int etag = -1) const = 0;

    /// return the cross-section uncertainty for DSID
    virtual double getXsectionUncertaintyUP(const int dsid, const int etag = -1) const = 0;

    /// return the cross-section uncertainty for DSID
    virtual double getXsectionUncertaintyDOWN(const int dsid, const int etag = -1) const = 0;
    
    // :: below is for future use? 
    /// return the branching ratio for DSID
    //virtual double getBR(const int dsid, const int etag = -1) const = 0;
    
    /// return the k-factor for DSID
    virtual double getKfactor(const int dsid, const int etag = -1) const = 0;
    
    /// return the sample cross-section for DSID
    virtual double getSampleXsection(const int dsid, const int etag = -1) const = 0;
    
    /// get a list of the DSID for the loaded samples
    virtual std::vector<int> getLoadedDSIDs() const = 0;
        
  }; // class IPMGCrossSectionTool
  
} // namespace PMGTools

#endif //> !PMGANALYSISINTERFACES_IPMGCROSSSECTIONTOOL_H
