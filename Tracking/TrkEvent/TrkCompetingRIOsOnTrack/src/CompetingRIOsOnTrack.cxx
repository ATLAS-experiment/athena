/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// CompetingRIOsOnTrack.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "TrkCompetingRIOsOnTrack/CompetingRIOsOnTrack.h"

#include "TrkEventPrimitives/LocalParameters.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "GaudiKernel/MsgStream.h"
#include <format>
#include <iostream>

// default constructor
Trk::CompetingRIOsOnTrack::CompetingRIOsOnTrack()
  : Trk::MeasurementBase()
  , m_assignProb()
{
}
// constructor with parameters
Trk::CompetingRIOsOnTrack::CompetingRIOsOnTrack(std::vector<AssignmentProb>&& assgnProb)
  : Trk::MeasurementBase()
  , m_assignProb(std::move(assgnProb))
{
}

// return the effective LocalParameters according to the weights (assignment probabilities)
//@f$ \~m = (\sum_i p_i G_i)^{-1} \sum_i p_i G_i m_i @f$
void
Trk::CompetingRIOsOnTrack::setLocalParametersAndErrorMatrix()
{

  if (ROTsHaveCommonSurface()) {
    // std::cout << "CompROT - weight matrices: " << std::endl;
    // std::cout << "CompROT - [1] " << rioOnTrack(0).localCovariance() << std::endl;
    Amg::MatrixX meanWeightMatrix =
      assignmentProbability(0) * rioOnTrack(0).localCovariance().inverse();
    for (unsigned int i = 1; i < numberOfContainedROTs(); i++) {
      meanWeightMatrix += assignmentProbability(i) * rioOnTrack(i).localCovariance().inverse();
      // std::cout << "CompROT - ["<< i << "] " << rioOnTrack(i).localCovariance() <<
      // std::endl;
    }
    // limit weight values against values too close to 0, otherwise inversion will fail!
    if (meanWeightMatrix.trace() <= 1.0e-15) {
      meanWeightMatrix = Amg::MatrixX(rioOnTrack(0).localCovariance().rows(),
                                      rioOnTrack(0).localCovariance().cols());
      meanWeightMatrix.setZero();
      for (int i = 0; i < meanWeightMatrix.cols(); ++i) {
        meanWeightMatrix(i, i) = 1.0e-10;
      }
    }
    // std::cout << "CompROT - mean weight: " << meanWeightMatrix << std::endl;
    m_localCovariance = meanWeightMatrix.inverse();
    // std::cout << "CompROT - mean covariance: " << localCovariance() << std::endl;

    if (numberOfContainedROTs() == 1) {
      m_localParams = rioOnTrack(0).localParameters();
    } else {
      Amg::MatrixX weight = rioOnTrack(0).localCovariance().inverse();
      Amg::VectorX meanParams =
        assignmentProbability(0) * weight * rioOnTrack(0).localParameters();
      for (unsigned int i = 1; i < numberOfContainedROTs(); i++) {
        weight = rioOnTrack(i).localCovariance().inverse();
        meanParams =
          meanParams + assignmentProbability(i) * weight * rioOnTrack(i).localParameters();
      }
      // std::cout << "CompROT - sum params: " << meanParams << std::endl;
      meanParams = localCovariance() * meanParams;
      // std::cout << "CompROT - mean params: " << meanParams << std::endl;
      int paramKey = rioOnTrack(0).localParameters().parameterKey();
      if (paramKey == 1) {
        Trk::DefinedParameter Par1(meanParams[Trk::loc1], Trk::loc1);
        m_localParams = Trk::LocalParameters(Par1);
      } else if (paramKey == 3) {
        Trk::DefinedParameter Par1(meanParams[Trk::loc1], Trk::loc1);
        Trk::DefinedParameter Par2(meanParams[Trk::loc2], Trk::loc2);
        m_localParams = Trk::LocalParameters(Par1, Par2);
      } else {
        std::cout << "Trk::CompetingRIOsOnTrack: can not handle parameter key " << paramKey
                  << std::endl;
      }
    }
  } else {
    // --------------------------------------------------
    // Warning: effective localParams cannot be calculated when ROTs don't lie on the
    //          associated surface without detector specific knowledge.
    // --------------------------------------------------
    // return 0;
    std::cout
      << "Trk::CompetingRIOsOnTrack: can not handle ROTs in different surfaces without "
         "detector specific knowledge "
      << std::endl;
  }
}

unsigned int
Trk::CompetingRIOsOnTrack::indexOfMaxAssignProb() const
{
  unsigned int index = 0;
  double maxAssgnProb = 0;
  for (unsigned int i = 0; i < numberOfContainedROTs(); i++) {
    if (m_assignProb[i] >= maxAssgnProb) {
      index = i;
      maxAssgnProb = m_assignProb[i];
    }
  }
  return index;
}

MsgStream&
Trk::CompetingRIOsOnTrack::dump(MsgStream& out) const
{
  out << "  - effective pars   locX : ";
  if (m_localParams.contains(Trk::locX)) {
    out << (m_localParams)[Trk::locX];
    if (m_localParams.contains(Trk::locY)) {
      out << " and " << (m_localParams)[Trk::locY];
    }
  } else {
    out << "ill-defined! They are " << m_localParams;
  }
  ////out << std::endl<<"  - effective error dLocX : " << m_localErrMat.error(Trk::locX);
  if (m_localParams.contains(Trk::locY)) {
    ////out << " and " << m_localErrMat.error(Trk::locY);
    out << std::endl;
  }
  unsigned int indexOfMaxProb = indexOfMaxAssignProb();
  out << "  - Contains: |ROT# identifier    locX      dLocX       locY       dlocY     "
         "ass.Prob   |";
  for (unsigned int i = 0; i < this->numberOfContainedROTs(); ++i) {
    out << std::endl
        << "              | " << i << "  " << this->rioOnTrack(i).identify() << "  ";
    if (this->rioOnTrack(i).localParameters().contains(Trk::locX)) {
      out << std::format("{:>10.6f}  {}",
                         this->rioOnTrack(i).localParameters()[Trk::locX],
                         1 / this->rioOnTrack(i).localCovariance()(Trk::locX, Trk::locX));
    } else {
      out << "                         ";
    }
    if (this->rioOnTrack(i).localParameters().contains(Trk::locY)) {
      out << std::format("  {:>10.6f}  {}  ",
                         this->rioOnTrack(i).localParameters()[Trk::locY],
                         1 / this->rioOnTrack(i).localCovariance()(Trk::locY, Trk::locY));
    } else {
      out << "                         ";
    }
    out << "  " << m_assignProb.at(i) << (indexOfMaxProb == i ? " **|" : "   |");
  }
  return out;
}

std::ostream&
Trk::CompetingRIOsOnTrack::dump(std::ostream& out) const
{
  out << "  - effective pars   locX : ";
  if (m_localParams.contains(Trk::locX)) {
    out << (m_localParams)[Trk::locX];
    if (m_localParams.contains(Trk::locY)) {
      out << " and " << (m_localParams)[Trk::locY];
    }
  } else {
    out << "ill-defined! They are " << m_localParams;
  }
  ////out <<std::endl<< "  - effective error dLocX : " << m_localErrMat.error(Trk::locX);
  if (m_localParams.contains(Trk::locY)) {
    ////out <<" and "<< m_localErrMat.error(Trk::locY);
    out << std::endl;
  }

  unsigned int indexOfMaxProb = indexOfMaxAssignProb();
  out << "  - Contains: |ROT# identifier    locX      dLocX       locY       dlocY     "
         "ass.Prob   |";
  for (unsigned int i = 0; i < this->numberOfContainedROTs(); ++i) {
    out << std::endl
        << "              | " << i << "  " << this->rioOnTrack(i).identify() << "  ";
    if (this->rioOnTrack(i).localParameters().contains(Trk::locX)) {
      out << std::format("{:>10.6f}  {}",
                         this->rioOnTrack(i).localParameters()[Trk::locX],
                         1 / this->rioOnTrack(i).localCovariance()(Trk::locX, Trk::locX));
    } else {
      out << "                         ";
    }
    if (this->rioOnTrack(i).localParameters().contains(Trk::locY)) {
      out << std::format("  {:>10.6f}  {}  ",
                         this->rioOnTrack(i).localParameters()[Trk::locY],
                         1 / this->rioOnTrack(i).localCovariance()(Trk::locY, Trk::locY));
    } else {
      out << "                         ";
    }
    out << "  " << m_assignProb.at(i) << (indexOfMaxProb == i ? " **|" : "   |");
  }
  return out;
}
