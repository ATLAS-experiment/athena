/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PROMPT_PROMPTUTILS_H
#define PROMPT_PROMPTUTILS_H

/**********************************************************************************
 * @Package: LeptonTaggers
 * @Class  : PromptUtils
 * @Author : Rustem Ospanov
 * @Author : Rhys Roberts
 *
 * @Brief  :
 *
 *  Helper functions
 *
 **********************************************************************************/

// Local
#include "VarHolder.h"

// Athena
#include "AthContainers/AuxElement.h"
#include "GaudiKernel/MsgStream.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/Vertex.h"

// ROOT
#include "TStopwatch.h"
#include "TH1.h"

// C/C++
#include <string>

namespace Prompt
{
  //=============================================================================
  double getVertexFitProb(const xAOD::Vertex *vtx);

  std::string vtxAsStr(const xAOD::Vertex        *vtx, bool print_tracks=false);
  std::string trkAsStr(const xAOD::TrackParticle *trk);

  std::string truthAsStr(const xAOD::IParticle &particle);

  double getDistance(const xAOD::Vertex  *vtx1, const xAOD::Vertex  *vtx2);
  double getDistance(const Amg::Vector3D &vtx1, const Amg::Vector3D &vtx2);

  double getNormDist(const Amg::Vector3D &PrimVtx, const Amg::Vector3D &SecVtx, const std::vector<float> &ErrorMatrix, MsgStream &msg);

  void fillTH1(TH1 *h, double val, double weight = 1.0);

  std::string printPromptVertexAsStr(const xAOD::Vertex *vtx, MsgStream &msg);

  //=============================================================================
  struct SortByIDTrackPt
  {
    bool operator()(const xAOD::TrackParticle *lhs, const xAOD::TrackParticle *rhs) { return lhs->pt() > rhs->pt(); }
  };

  //=============================================================================
  template<class T1, class T2> bool getVar(T1 &obj, T2 &value, const std::string &var_name)
  {
    if (!obj) {
      std::cerr << "getVar - received a null object" << std::endl;
      return false;
    }

    //
    // get the int aux-variable
    //
    typename SG::Accessor<T2> acc(var_name);

    if(!acc.isAvailable(*obj)) {
      return false;
    }

    value = acc(*obj);
    return true;
  }

  //=============================================================================
  template<class T1, class T2> bool GetAuxVar(const T1 &obj, T2 &value, const std::string &var_name)
  {
    //
    // get the int aux-variable
    //
    typename SG::Accessor<T2> acc(var_name);

    if(!acc.isAvailable(obj)) {
      return false;
    }

    value = acc(obj);
    return true;
  }

  //=============================================================================
  std::string PrintResetStopWatch(TStopwatch &watch);

  class TimerScopeHelper
  {
  public:

    explicit TimerScopeHelper(TStopwatch &timer)
      :m_fTimer(timer) { m_fTimer.Start(false); }
    ~TimerScopeHelper() { m_fTimer.Stop(); }

  private:

    TStopwatch &m_fTimer;
  };


  //======================================================================================================
  struct SortObjectByVar
  {
    explicit SortObjectByVar(const unsigned v, MsgStream &m, bool inverse=false):m_var(v), m_inv(inverse), m_msg(m) {}

    template<class T> bool operator()(const T &lhs, const T &rhs)
    {
      double val_rhs = 0.0;
      double val_lhs = 0.0;

      if(!lhs.getVar(m_var, val_lhs) || !rhs.getVar(m_var, val_rhs)) {
        m_msg << MSG::WARNING << "SortObjectByVar - missing var" << std::endl;
      }

      if(m_inv) {
        return val_lhs > val_rhs;
      }

      return val_lhs < val_rhs;
    }

  private:

    SortObjectByVar();

    unsigned   m_var;
    bool       m_inv;
    MsgStream &m_msg;

  };
}

#endif //PROMPT_PROMPTUTILS_H