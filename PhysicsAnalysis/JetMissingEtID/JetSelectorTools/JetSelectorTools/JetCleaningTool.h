/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETSELECTORTOOLS_JETCLEANINGTOOL_H
#define JETSELECTORTOOLS_JETCLEANINGTOOL_H

/**
   @class JetCleaningTool
   @brief Class for selecting jets that pass cleaning cuts

   @author Zach Marshall
   @date   Feb 2014
*/

// Stdlib includes
#include <string>
#include <vector>
#include <unordered_map>

// Base classes
#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"
#include "AsgDataHandles/ReadDecorHandleKey.h"
#include "AsgDataHandles/WriteDecorHandleKey.h"
#include "JetInterface/IJetDecorator.h"
#include "JetInterface/IJetSelector.h" 


// The xAOD jet type
#include "xAODJet/Jet.h"

// Return object
#include "PATCore/AcceptInfo.h"
#include "PATCore/AcceptData.h"

namespace JCT { class HotCell; }

class JetCleaningTool : public asg::AsgTool, virtual public IJetDecorator, virtual public IJetSelector
{

  ASG_TOOL_CLASS2(JetCleaningTool, IJetDecorator, IJetSelector)

public:
  /** Levels of cut */
  enum CleaningLevel
  {
    SuperLooseBadLLP,
    VeryLooseBadLLP,
    LooseBad,
    LooseBadLLP,
    LooseBadTrigger,
    TightBad,
    UnknownCut
  };

  /** Standard constructor */
  JetCleaningTool(const std::string &name = "JetCleaningTool");

  /** Cut-based constructor */
  JetCleaningTool(const CleaningLevel alevel, const bool doUgly = false);

  /** Cut and string based constructor */
  JetCleaningTool(const std::string &name, const CleaningLevel alevel, const bool doUgly = false);

  /** Standard destructor */
  virtual ~JetCleaningTool();

  /** Initialize method */
  virtual StatusCode initialize() override;

  const asg::AcceptInfo &getAcceptInfo() const
  {
    return m_accept;
    }

    /** The DFCommonJets decoration accept method */
    asg::AcceptData accept( const int isJetClean, const int fmaxIndex ) const;
    
    /** The DFCommonJets decoration + tight method  */
    asg::AcceptData accept( const int isJetClean,
                                              const double sumpttrk, //in MeV, same as sumpttrk
                                              const double fmax,
					      const double eta,
					      const double pt, 
                                              const int    fmaxIndex ) const;

    /** The main accept method: the actual cuts are applied here */
    asg::AcceptData accept( const double emf,
                 const double hecf,
                 const double larq,
                 const double hecq,
                 const double sumpttrk, //in MeV, same as sumpttrk
                 const double eta,      //emscale Eta  
                 const double pt,       //in MeV, same as sumpttrk
                 const double fmax,
                 const double negE ,     //in MeV
                 const double AverageLArQF,
                 const int    fMaxIndex     ) const;

    /** The D3PDReader accept method */
    asg::AcceptData accept( const xAOD::Jet& jet) const;

    int keep(const xAOD::Jet& jet) const final
    { return static_cast<bool>(accept(jet)); }

    virtual StatusCode decorate(const xAOD::JetContainer &jets) const override;

    /** Hot cell checks */
    bool containsHotCells( const xAOD::Jet& jet, const unsigned int runNumber) const;

    /** Helpers for cut names */
    static CleaningLevel getCutLevel( const std::string& );
    static std::string   getCutName( const CleaningLevel );

  private:

    /** Name of the cut */    
    Gaudi::Property<std::string> m_cutName{this, "CutLevel", "" }; 
    CleaningLevel m_cutLevel{LooseBad};
    Gaudi::Property<bool> m_doUgly{this, "DoUgly", false};
    Gaudi::Property<bool> m_useDecorations{this, "UseDecorations", true};
    Gaudi::Property<bool> m_useLooseDecorForTightCut{this, "UseLooseDecorForTightCut", false};
    SG::ConstAccessor<char> m_acc_jetClean{"DFCommonJets_jetClean_LooseBad"};
  
    //
    Gaudi::Property<std::string> m_jetContainerName{this, "JetContainer", "", "SG key for input jet container"};
    SG::WriteDecorHandleKey<xAOD::JetContainer> m_jetCleanKey{this, "JetCleaningName", "isClean", "SG key for output jet cleaning decoration"};
    asg::AcceptInfo m_accept;

    /** Hot cells caching */
    Gaudi::Property<std::string> m_hotCellsFile{this, "HotCellsFile", ""};
    std::unordered_map<unsigned int, std::vector<std::unique_ptr<JCT::HotCell>>> m_hotCellsMap;
    StatusCode readHotCells();
    
    void missingVariable(const std::string& varName) const;

}; // End: class definition



#endif
