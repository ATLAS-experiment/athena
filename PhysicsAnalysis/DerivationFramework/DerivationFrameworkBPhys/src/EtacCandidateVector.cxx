/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
  Contact: Xin Chen <xin.chen@cern.ch>
*/
#include "DerivationFrameworkBPhys/EtacCandidateVector.h"

namespace DerivationFramework {

  RhoCandidateVector::RhoCandidateVector(size_t num, bool orderByPt = true) {
    m_num = num;
    m_orderByPt = orderByPt;
  }

  void RhoCandidateVector::AddElement(const RhoCandidate& rho) {
    if(m_num>0 && m_vector.size()>=m_num) {
      if(m_orderByPt && rho.ptTot<=m_vector.back().ptTot) return;
      else if(!m_orderByPt && rho.chi2NDF>=m_vector.back().chi2NDF) return;
    }
    auto pos = m_vector.cend();
    for(auto iter=m_vector.cbegin(); iter!=m_vector.cend(); iter++) {
      if(m_orderByPt) {
	if(rho.ptTot>iter->ptTot) { pos = iter; break; }
      }
      else {
	if(rho.chi2NDF<iter->chi2NDF) { pos = iter; break; }
      }
    }
    m_vector.insert(pos, rho);
    if(m_num>0 && m_vector.size()>m_num) m_vector.pop_back();
  }
  
  const std::vector<RhoCandidate>& RhoCandidateVector::GetVector() const {
    return m_vector;
  }

  EtacCandidateVector::EtacCandidateVector(size_t num, bool orderByPt = true) {
    m_num = num;
    m_orderByPt = orderByPt;
  }

  void EtacCandidateVector::AddElement(const EtacCandidate& etac) {
    if(m_num>0 && m_vector.size()>=m_num) {
      if(m_orderByPt && etac.ptTot<=m_vector.back().ptTot) return;
      else if(!m_orderByPt && etac.chi2NDFSum>=m_vector.back().chi2NDFSum) return;
    }
    auto pos = m_vector.cend();
    for(auto iter=m_vector.cbegin(); iter!=m_vector.cend(); iter++) {
      if(m_orderByPt) {
	if(etac.ptTot>iter->ptTot) { pos = iter; break; }
      }
      else {
	if(etac.chi2NDFSum<iter->chi2NDFSum) { pos = iter; break; }
      }
    }
    m_vector.insert(pos, etac);
    if(m_num>0 && m_vector.size()>m_num) m_vector.pop_back();
  }

  const std::vector<EtacCandidate>& EtacCandidateVector::GetVector() const {
    return m_vector;
  }
}
