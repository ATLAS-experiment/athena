/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//============================================================================
// BMuonTrackIsoTool.h
//============================================================================
// 
// Author : Wolfgang Walkowiak <Wolfgang.Walkowiak@cern.ch.>
// Changes:
//
// Add muon track isolation information for different configurations,
// different track selections and different PV-to-SV association methods.
//
// For an usage example see BPHY8.py .
//
//============================================================================
//
#ifndef DERIVATIONFRAMEWORK_BMuonTrackIsoTool_H
#define DERIVATIONFRAMEWORK_BMuonTrackIsoTool_H

#include "DerivationFrameworkBPhys/BPhysVertexTrackBase.h"
#include "xAODMuon/MuonContainer.h"
#include "boost/multi_array.hpp"

namespace InDet {
  class IInDetTrackSelectionTool;
}

namespace DerivationFramework {
  
  class ATLAS_NOT_THREAD_SAFE BMuonTrackIsoTool : public BPhysVertexTrackBase {

  private:
    typedef BPhysVertexTrackBase super;
    
    //
    // internal helper class
    //
  protected:
    class MuIsoItem : public BaseItem {
    
  public:
    MuIsoItem(const std::string& Name="_none_",
              const std::string& Bname="muiso",
	      const std::string& Prefix="");
    virtual ~MuIsoItem();
	
    virtual void        resetVals() override;
    virtual void        copyVals(const BaseItem& item) override;
    void        copyVals(const MuIsoItem& item);
    void        fill(double isoValue=-2., int nTracks=-1,
			     const xAOD::Muon* muon=NULL);
    std::string muIsoName() const;
    std::string nTracksName() const;
    std::string muLinkName() const;
    
  public:
    std::vector<float>  vIsoValues;
    std::vector<int>    vNTracks;
    MuonBag             vMuons{};
  }; // MuIsoItem
    
  public: 
      BMuonTrackIsoTool(const std::string& t, const std::string& n,
			const IInterface* p);

  protected:
      // Hook methods 
      virtual StatusCode  initializeHook() override;
      virtual StatusCode  finalizeHook() override;
      
      virtual StatusCode  addBranchesVCSetupHook(size_t ivc) const override;

      virtual StatusCode  addBranchesSVLoopHook(const xAOD::Vertex* vtx) const override;

      virtual StatusCode  calcValuesHook(const xAOD::Vertex* vtx,
					 const unsigned int ipv,
					 const unsigned int its,
					 const unsigned int itt) const override;
      virtual bool        fastFillHook(const xAOD::Vertex* vtx,
				       const int ipv) const override;
      
  private:
      StatusCode  saveIsolation(const xAOD::Vertex* vtx) const;
      void        initResults();
      void        setResultsPrefix(std::string prefix) const;
      
      std::string buildBranchName(unsigned int ic,
					  unsigned int its,
					  unsigned int ipv,
					  unsigned int itt) const;
      
  private:      
      // job options
      std::string                      m_muonContainerName;
      std::vector<double>              m_isoConeSizes;
      std::vector<double>              m_isoTrkImpLogChi2Max;
      std::vector<int>                 m_isoDoTrkImpLogChi2Cut;

      // containers
      mutable const xAOD::MuonContainer* m_muons{};
      
      
      // results array
      typedef boost::multi_array<MuIsoItem, 4> MuIsoItem4_t;
      mutable MuIsoItem4_t m_results;

  }; // BMuonTrackIsoTool
} // namespace

#endif // DERIVATIONFRAMEWORK_BMuonTrackIsoTool_H
