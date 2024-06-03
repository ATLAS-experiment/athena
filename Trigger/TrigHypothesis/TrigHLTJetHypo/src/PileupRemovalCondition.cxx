/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#
#include "./PileupRemovalCondition.h"
#include "./ITrigJetHypoInfoCollector.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/IJet.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "CaloEvent/CaloClusterContainer.h"
#include <sstream>
#include <cmath>
#include <TLorentzVector.h>
#include "CxxUtils/fpcompare.h"
#include "CxxUtils/phihelper.h"
#include "FourMomUtils/xAODP4Helpers.h"

PileupRemovalCondition::PileupRemovalCondition(double mMin,double mMax) :
m_min(mMin), m_max(mMax)  {
}

bool PileupRemovalCondition::isSatisfied(const pHypoJet& ip,
				const std::unique_ptr<ITrigJetHypoInfoCollector>& collector) const {
  float jetEMF = -999;
  double LR=-1;
  ip->getAttribute("EMFrac",jetEMF);
  if (CxxUtils::fpcompare::less_equal (double(jetEMF),0))LR=-999; 
  if(CxxUtils::fpcompare::greater_equal(double(jetEMF),1.)) LR=-999;
  else  LR= log10(double(1./jetEMF - 1.));
  bool pass = LR > m_max;
  // Recalculating the LR only if it's not satisfied the LR>m_max criteria
  if (!pass){
     auto jetPhi= ip->phi();
     auto jetEta= ip->eta();
     // Recaulculate LR removing pileup clusters contribution if LR passes the m_min cut
     if(  LR > m_min  ) {
        size_t nClusters =  (*ip->xAODJet())->numConstituents();
        double clusterPU_sumEEM = 0; double clusterPU_sumE = 0;
        for (size_t clust = 0; clust < nClusters; clust++) {
           const xAOD::CaloCluster * aCluster = dynamic_cast<const xAOD::CaloCluster*> ((*ip->xAODJet())->rawConstituent(clust));
           if (not aCluster) continue;
           double clusEEM = 0;
           clusEEM+=(aCluster)->eSample(CaloSampling::EMB1);
           clusEEM+=(aCluster)->eSample(CaloSampling::EMB2);
           clusEEM+=(aCluster)->eSample(CaloSampling::EMB3);
           clusEEM+=(aCluster)->eSample(CaloSampling::EME1);
           clusEEM+=(aCluster)->eSample(CaloSampling::EME2);
           clusEEM+=(aCluster)->eSample(CaloSampling::EME3);
           clusEEM+=(aCluster)->eSample(CaloSampling::FCAL1);
           double lambda = aCluster->getMomentValue(xAOD::CaloCluster::CENTER_LAMBDA);

           if (lambda > 500) continue;
           double d_eta = aCluster->rawEta() - jetEta;
           double d_phi = xAOD::P4Helpers::deltaPhi(aCluster->rawPhi(),jetPhi);
           double d_R2 = d_eta*d_eta + d_phi*d_phi;

           if (d_R2 < 0.15*0.15) continue;
           clusterPU_sumEEM+=clusEEM/1000.;
           clusterPU_sumE+=aCluster->rawE()/1000.;
         }
         double jetEEM_EMscale = 0; double jetE_EMscale = 0;  //Working on EM scale because calE() doesn't always return correct EEM and cluster moment EMF not accessable during testing
         std::vector<double> samplingEnergy =  (*ip->xAODJet())->getAttribute<std::vector<double> >("EnergyPerSampling");  

         for(size_t s=0; s<samplingEnergy.size(); s++) {
           double samplingE = 0.001*(samplingEnergy.at(s));
           if ( s < 8 || (s > 20 && s < 28) ) jetEEM_EMscale+=samplingE; // EM layers 0-7 and 21-27
           jetE_EMscale+=samplingE; 
         }
         jetEMF = (jetEEM_EMscale - clusterPU_sumEEM)/(jetE_EMscale - clusterPU_sumE);
         if (CxxUtils::fpcompare::less_equal (double(jetEMF),0)) LR = -999.;
         if(CxxUtils::fpcompare::greater_equal(double(jetEMF),1.0)) LR = -999.;
         else LR = log10(double(1./jetEMF - 1.));
         pass=LR>m_max;
    }
  }
  if(collector){
    const void* address = static_cast<const void*>(this);

    std::stringstream ss0;
    ss0 << "PileupRemovalCondition: (" << address << ") " 
        << " LR desired cut, LR necessary cut to start pileup removal algo " <<m_max<<","<<m_min
        << " pass: "  << std::boolalpha << pass << '\n';

    auto j_addr = static_cast<const void*>(ip.get());
    std::stringstream ss1;
    ss1 <<  "     jet : ("<< j_addr << ")"
        " LR desired cut, LR necessary cut to start pileup removal algo " <<m_max<<","<<m_min<< '\n';
    
    collector->collect(ss0.str(), ss1.str());

  }
  return pass;
}


bool 
PileupRemovalCondition::isSatisfied(const HypoJetVector& ips,
			   const std::unique_ptr<ITrigJetHypoInfoCollector>& c) const {
  auto result =  isSatisfied(ips[0], c);
  return result;
}


std::string PileupRemovalCondition::toString() const {
  std::stringstream ss;
  ss << "PileupRemovalCondition (" << this << ") "
     << " LR thresh, LRcorr thresh "
     <<m_max<<","<<m_min
     <<'\n';

  return ss.str();
}
