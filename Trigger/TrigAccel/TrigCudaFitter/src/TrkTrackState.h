//////////////////////////////////////////////////////////////////
// TrkTrackState.h
//   Header file for class TrkTrackState
///////////////////////////////////////////////////////////////////
// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
///////////////////////////////////////////////////////////////////
// Author: Dmitry Emeliyanov, RAL
// D.Emeliyanov@rl.ac.uk
///////////////////////////////////////////////////////////////////

#ifndef __TRK_TRACK_STATE_H__
#define __TRK_TRACK_STATE_H__

class TrkBaseNode;
class TrkPlanarSurface;
class TrkTrackState
{
  public:
    TrkTrackState() = default;
    TrkTrackState(const double[5]);
    TrkTrackState(const TrkTrackState*);
    ~TrkTrackState(){};

    void report();
    void resetCovariance();
    void serialize(char fileName[]);
    TrkPlanarSurface* getSurface();
    void attachToSurface(TrkPlanarSurface*);
    void setScatteringMode(int);
    int getScatteringMode();
    void applyMaterialEffects();
    void applyMultipleScattering();
    void updateTrackState(double*);
    void updateTrackCovariance(double*);
    void setTrackCovariance(double A[5][5]);
    void setTrackState(double A[5]);
    void setSmootherGain(double A[5][5]);
    void setPreviousState(TrkTrackState*);
    void runSmoother();
    inline double getTrackState(int i)
    {
      return m_Rk[i];
    }
    inline double getTrackCovariance(int i, int j)
    {
      return m_Gk[i][j];
    }
    inline int getStateSize()
    {
      return 5;
    }

  protected:
    double m_Rk[5]{};
    double m_Re[5]{};
    double m_Gk[5][5]{};
    double m_Ge[5][5]{};
    int m_scattMode{0};
    bool m_isScattered{false};
    TrkPlanarSurface* m_pSurface{nullptr};
    TrkTrackState* m_pPrevState{nullptr};
    double m_A[5][5]{};

  private:
    void applyEnergyLoss(int);
};

#endif
