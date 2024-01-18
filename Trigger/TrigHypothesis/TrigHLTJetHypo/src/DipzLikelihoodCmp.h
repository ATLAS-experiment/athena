/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGHLTJETHYPO_DipzLIKELIHOODCMP_H
#define TRIGHLTJETHYPO_DipzLIKELIHOODCMP_H

/********************************************************************
 *
 * NAME:     DipzLikelihoodCmp.h
 * PACKAGE:  Trigger/TrigHypothesis/TrigHLTJetHypo
 *
 * Based on DipzMLPLCondition by I. Ochoa
 *
 * This file contains the following function objects:
 * 
 * DipzLilihood: This calculates the likelihood of a hypo jet vector
 *               according to the DIPz criteria. Clients of
 *               this calculation include the DIPZMLPLCondtion and
 *               the DIPZ-HT scenario.
 *
 * DipzLikeihoodCmp: This compares the likelihood of two hypo jet vectors.
 *                   The comparator is used in the DIPZ-HT scenario
 *                   to identify which hypoJetVector  taken from
 *                   a collection of hypoJetVectors maximises the likelihood.
 *              
 * 
 *********************************************************************/

#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/HypoJetDefs.h"
#include <string>


namespace HypoJet{
  class IJet;
}



class DipzLikelihood {
public:
  DipzLikelihood(const std::string &decName_z, 
		    const std::string &decName_negLogSigma2);

  ~DipzLikelihood() = default;
  double operator()(const HypoJetVector&) const;
  
 private:
  
  const std::string m_decName_z;
  const std::string m_decName_negLogSigma2;
  
  double getDipzMLPLDecValue(const pHypoJet &ip,
			     const std::string &decName) const;
  
  double calcNum(double acmlt, const pHypoJet &ip) const; 

  double calcDenom(double acmlt, const pHypoJet &ip) const;
  
  double calcLogTerm(double acmlt, const pHypoJet &ip, double zhat) const;
  
  double checkedRatio(double num, double den) const;
};


class DipzLikelihoodCmp {
public:
  DipzLikelihoodCmp(const std::string &decName_z, 
		    const std::string &decName_negLogSigma2);

  bool operator()(const HypoJetVector&, const HypoJetVector&);
private:

  DipzLikelihood m_likelihoodCalculator;

};


#endif
