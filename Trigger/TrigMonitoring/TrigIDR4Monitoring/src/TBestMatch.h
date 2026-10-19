/* emacs: this is -*- c++ -*- */
/**
 **     @file    TBestMatch.h
 **
 **              Different matchers all using the stable marriage algorithm,
 **              with different distance measures.
 **              NB: some of these take the sqrt of the (squared) distances
 **                  but it is more efficient to just sort on the squared
 **                  distance measure.
 **
 **     @author  mark sutton
 **     @date    Wed 28 Oct 2009 02:23:23 CET
 **
 **     Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/


#ifndef TRIGIDR4MONITORING_BESTMATCH_H
#define TRIGIDR4MONITORING_BESTMATCH_H

#include <set>
#include <map>
#include <memory>

#include <stdexcept>

#include "TrigInDetAnalysis/TrackAssociator.h"
#include "TrigInDetAnalysisUtils/BestMatcher.h"


template<typename T, typename R=T>
class TDeltaRMatcher : public BestMatcher<T,R> {

public:

  TDeltaRMatcher(const std::string& name, double d) :
    BestMatcher<T,R>(name, d*d)  /// stored the squared distance parameter
  { }

  virtual ~TDeltaRMatcher() { }

  virtual std::unique_ptr<TIDA::Associator<T,R>> uclone() const override { return std::make_unique<TDeltaRMatcher<T,R>>(*this); }

  virtual double distance( const T* t0, const R* t1 ) const override {
    double deta = t0->eta()-t1->eta();
    double dphi = t0->phi()-t1->phi();
    if ( dphi> M_PI ) dphi-=2*M_PI;
    if ( dphi<-M_PI ) dphi+=2*M_PI;
    return ( deta*deta + dphi*dphi );
  }

};



template<typename T, typename R=T>
class TSecondBestpTMatcher : public BestMatcher<T,R> {

public:

  TSecondBestpTMatcher(const std::string& name, double d) :
    BestMatcher<T,R>(name, d)
  { }

  virtual ~TSecondBestpTMatcher() { }

  virtual std::unique_ptr<TIDA::Associator<T,R>> uclone() const override { return std::make_unique<TSecondBestpTMatcher<T,R>>(*this); }

  virtual double distance( const T* t0, const R* t1 ) const override {
    double dpT = fabs( t0->pt() - t1->pt() );
    return dpT/(fabs(t0->pt()));
  }

};




template<typename T, typename R=T>
class TDeltaRZMatcher : public BestMatcher<T,R> {

public:

  TDeltaRZMatcher(const std::string& name, double deta=0.05, double dphi=0.05, double dzed=5) :
    BestMatcher<T,R>(name, 1),
    m_deta2(deta*deta),
    m_dphi2(dphi*dphi),
    m_dzed2(dzed*dzed)
  { }

  virtual ~TDeltaRZMatcher() { }

  virtual std::unique_ptr<TIDA::Associator<T,R>> uclone() const override { return std::make_unique<TDeltaRZMatcher<T,R>>(*this); }

  virtual double distance( const T* t0, const R* t1 ) const override {
    double deta = t0->eta()-t1->eta();
    double dphi = t0->phi()-t1->phi(); if ( dphi> M_PI ) dphi-=2*M_PI;  if ( dphi<-M_PI ) dphi+=2*M_PI;
    double dzed = t0->z0()-t1->z0();

    double dr = std::sqrt( deta*deta/m_deta2 + dphi*dphi/m_dphi2 + dzed*dzed/m_dzed2 );

    return dr;
  }

protected:

  double m_deta2;
  double m_dphi2;
  double m_dzed2;

};




template<typename T, typename R=T>
class TDeltaRZSinThetaMatcher : public BestMatcher<T,R> {

public:

  TDeltaRZSinThetaMatcher(const std::string& name, double deta=0.05, double dphi=0.05, double dzed=5) :
    BestMatcher<T,R>(name, 1),
    m_deta2(deta*deta),
    m_dphi2(dphi*dphi),
    m_dzed2(dzed*dzed)
  {  }

  virtual ~TDeltaRZSinThetaMatcher() { }

  virtual std::unique_ptr<TIDA::Associator<T,R>> uclone() const override { return std::make_unique<TDeltaRZSinThetaMatcher<T,R>>(*this); }

  virtual double distance( const T* t0, const R* t1 ) const override {
    double theta = 2*std::atan(std::exp(-t1->eta()));

    double deta = t0->eta()-t1->eta();
    double dphi = t0->phi()-t1->phi(); if ( dphi> M_PI ) dphi-=2*M_PI;  if ( dphi<-M_PI ) dphi+=2*M_PI;

    double dzed = (t0->z0()-t1->z0())*std::sin(theta);

    double dr = std::sqrt( deta*deta/m_deta2 + dphi*dphi/m_dphi2 + dzed*dzed/m_dzed2 );

    return dr;
  }

protected:

  double m_deta2;
  double m_dphi2;
  double m_dzed2;

};



template<typename T, typename R=T>
class TSigmaMatcher : public BestMatcher<T,R> {

public:

  TSigmaMatcher(const std::string& name, double dsigma=1) :
    BestMatcher<T,R>(name, dsigma)
  {  }

  virtual ~TSigmaMatcher() { }

  virtual std::unique_ptr<TIDA::Associator<T,R>> uclone() const override { return std::make_unique<TSigmaMatcher<T,R>>(*this); }

  virtual double distance( const T* t0, const R* t1 ) const override {
    double deta = t1->eta()-t0->eta();
    double dphi = t1->phi()-t0->phi();
    if ( dphi> M_PI ) dphi-=2*M_PI;
    if ( dphi<-M_PI ) dphi+=2*M_PI;
    double dzed = t1->z0()-t0->z0();

    deta /= t1->deta();
    dphi /= t1->dphi();
    dzed /= t1->dz0();

    double dr = std::sqrt( deta*deta + dphi*dphi + dzed*dzed );

    return dr;
  }

};




template<typename T, typename R=T>
class TDeltaPhiMatcher : public BestMatcher<T,R> {

public:

  TDeltaPhiMatcher(const std::string& name, double p) :
    BestMatcher<T,R>(name, p)
  { }

  virtual ~TDeltaPhiMatcher() { }

  virtual std::unique_ptr<TIDA::Associator<T,R>> uclone() const override { return std::make_unique<TDeltaPhiMatcher<T,R>>(*this); }

  virtual double distance( const T* t0, const R* t1 ) const override {
    double dphi = t0->phi()-t1->phi();
    if ( dphi> M_PI ) dphi-=2*M_PI;
    if ( dphi<-M_PI ) dphi+=2*M_PI;
    return std::fabs( dphi );
  }

};




template<typename T, typename R=T>
class TDeltaZMatcher : public BestMatcher<T,R> {

public:

  TDeltaZMatcher(const std::string& name, double p) :
    BestMatcher<T,R>(name, p)
  { }

  virtual ~TDeltaZMatcher() { }

  virtual std::unique_ptr<TIDA::Associator<T,R>> uclone() const override { return std::make_unique<TDeltaZMatcher<T,R>>(*this); }

  virtual double distance( const T* t0, const R* t1 ) const override {
    double dzed = t0->z()-t1->z();
    return std::fabs( dzed );
  }

};



#endif //  TRIGIDR4MONITORING_BESTMATCH_H
