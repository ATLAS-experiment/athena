//////////////////////////////////////////////////////////////////
// TrkFilteringNodes.h
//   Header file for classes representing filtering nodes
///////////////////////////////////////////////////////////////////
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
///////////////////////////////////////////////////////////////////
// Author: Dmitry Emeliyanov, RAL
// D.Emeliyanov@rl.ac.uk
///////////////////////////////////////////////////////////////////

#ifndef __TRK_FILTERING_NODES_H__
#define __TRK_FILTERING_NODES_H__

class TrkBaseNode;
class TrkTrackState;
class TrkPlanarSurface;

class TrkFilteringNode1D : public TrkBaseNode
{
  public:
    TrkFilteringNode1D(){};
    virtual ~TrkFilteringNode1D(){};

    virtual int getKalmanGain(double[5][2]);
    virtual int getResiduals(double[2]);
    virtual int getInverseResidualVariance(double[2][2]);
    virtual int getMeasurementMatrix(double[2][5]);

  protected:
    void runKalmanFilter(TrkTrackState*);
    double calculateChi2();

    double m_B[5]{};
    double m_D{};
    double m_H[5]{};
    double m_K[5]{};
    double m_resid{};
    double m_m{};
    double m_V{};
};

class TrkFilteringNode2D : public TrkBaseNode
{
  public:
    TrkFilteringNode2D(){};
    virtual ~TrkFilteringNode2D(){};

    virtual int getKalmanGain(double[5][2]);
    virtual int getResiduals(double[2]);
    virtual int getInverseResidualVariance(double[2][2]);
    virtual int getMeasurementMatrix(double[2][5]);

  protected:
    void runKalmanFilter(TrkTrackState*);
    double calculateChi2();

    double m_B[5][2]{};
    double m_D[2][2]{};
    double m_H[2][5]{};
    double m_K[5][2]{};
    double m_resid[2]{};
    double m_m[2]{};
    double m_V[2][2]{};
};

class TrkClusterNode : public TrkFilteringNode1D
{
  public:
    TrkClusterNode(TrkPlanarSurface*,double,double,double);
    virtual ~TrkClusterNode(){};

    void validateMeasurement(TrkTrackState*);
    void updateTrackState(TrkTrackState*);
    double getChi2Distance(TrkTrackState*);
    void serialize(char*) {};
    void report(){};
};

class TrkEndCapClusterNode : public TrkFilteringNode1D
{
  public:
    TrkEndCapClusterNode(TrkPlanarSurface*,double,double,double,double);
    virtual ~TrkEndCapClusterNode(){};

    void validateMeasurement(TrkTrackState*);
    void updateTrackState(TrkTrackState*);
    double getChi2Distance(TrkTrackState*);
    void serialize(char*) {};
    void report(){};

  private:
    double m_Rc;
};

class TrkPixelNode : public TrkFilteringNode2D
{
  public:
    TrkPixelNode(TrkPlanarSurface*,double,double[2],double[4]);
    virtual ~TrkPixelNode(){};

    void validateMeasurement(TrkTrackState*);
    void updateTrackState(TrkTrackState*);
    double getChi2Distance(TrkTrackState*);
    void serialize(char*) {};
    void report();
};

#endif
