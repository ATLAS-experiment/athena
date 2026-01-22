/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef _TrkVKalVrtCore_Derivt_H
#define _TrkVKalVrtCore_Derivt_H

#include <vector>

#include "TrkVKalVrtCore/CommonPars.h"
#include "TrkVKalVrtCore/Derclc1.h"
#include "TrkVKalVrtCore/Derclc2.h"
#include "TrkVKalVrtCore/DerclcAng.h"
#include "TrkVKalVrtCore/TrkVKalUtils.h"
#include "TrkVKalVrtCore/TrkVKalVrtCoreBase.h"
namespace Trk {

// Base class for any constraint
// Contains derivatines needed for application
//

class VKVertex;
enum class VKContraintType { Mass, Phi, Theta, Point, Plane, Radius };
class VKConstraintBase {
 public:
  VKConstraintBase(const int, int, VKContraintType, VKVertex*);
  virtual ~VKConstraintBase();

 public:
  const VKVertex* getOriginVertex() const { return m_originVertex; }
  VKContraintType getType() const { return m_type; }
  virtual VKConstraintBase* clone() const = 0;
  virtual void applyConstraint() = 0;
  int NCDim;                               // constraint dimension
  int NTrk;                                // number of tracks
  std::vector<double> aa;                  // Constraint values
  std::vector<std::vector<Vect3DF> > f0t;  // Constraint momentum derivatives
  std::vector<Vect3DF> h0t;                // Constraint space derivatives
 protected:
  VKVertex* m_originVertex;
  const VKContraintType m_type;
};
//
//  Mass constraint
//
class VKMassConstraint final : public VKConstraintBase {
 public:
  VKMassConstraint(int, double, VKVertex*);
  VKMassConstraint(int, double, std::vector<int>, VKVertex*);
  ~VKMassConstraint();
  friend std::ostream& operator<<(std::ostream& out, const VKMassConstraint&);
  virtual VKConstraintBase* clone() const override;

 public:
  void setTargetMass(double M) { m_targetMass = M; };
  virtual void applyConstraint() override;
  double getTargetMass() const { return m_targetMass; };
  const std::vector<int>& getUsedParticles() const { return m_usedParticles; };

 private:
  std::vector<int> m_usedParticles;
  double m_targetMass;
};
//
//  Angular constraints
//
class VKPhiConstraint final : public VKConstraintBase {
 public:
  VKPhiConstraint(int, VKVertex*);
  ~VKPhiConstraint();
  virtual VKConstraintBase* clone() const override;
  virtual void applyConstraint() override;
  friend std::ostream& operator<<(std::ostream& out, const VKPhiConstraint&);
};
class VKThetaConstraint : public VKConstraintBase {
 public:
  VKThetaConstraint(int, VKVertex*);
  ~VKThetaConstraint();
  virtual VKConstraintBase* clone() const override;
  virtual void applyConstraint() override;
  friend std::ostream& operator<<(std::ostream& out, const VKThetaConstraint&);
};

//
//  Pointing constraints
//
class VKPointConstraint final : public VKConstraintBase {
 public:
  VKPointConstraint(int, const double[3], VKVertex*, bool);
  ~VKPointConstraint();
  friend std::ostream& operator<<(std::ostream& out, const VKPointConstraint&);
  bool onlyZ() const { return m_onlyZ; };
  void setTargetVertex(double VRT[3]) {
    m_targetVertex[0] = VRT[0];
    m_targetVertex[1] = VRT[1];
    m_targetVertex[2] = VRT[2];
  };
  const double* getTargetVertex() const { return m_targetVertex; };
  virtual VKConstraintBase* clone() const override;
  virtual void applyConstraint() override;

 private:
  bool m_onlyZ;
  double m_targetVertex[3]{};  // Target vertex is in global reference system
};
//
//  Vertex in plane constraint
//
class VKPlaneConstraint final : public VKConstraintBase {
 public:
  VKPlaneConstraint(int, double, double, double, double, VKVertex*);
  ~VKPlaneConstraint();
  friend std::ostream& operator<<(std::ostream& out, const VKPlaneConstraint&);
  double getA() const { return m_A; }
  double getB() const { return m_B; }
  double getC() const { return m_C; }
  double getD() const { return m_D; }
  virtual void applyConstraint() override;
  virtual VKConstraintBase* clone() const override;

 private:
  double m_A, m_B, m_C, m_D;
};
//
//  Vertex at fixed radius constraint
//
class VKRadiusConstraint final : public VKConstraintBase {
 public:
  VKRadiusConstraint(int, double, double[], VKVertex*);
  ~VKRadiusConstraint();
  friend std::ostream& operator<<(std::ostream& out, const VKRadiusConstraint&);
  double getRC() const { return m_RC; }
  double getRefX() const { return m_refP[0]; }
  double getRefY() const { return m_refP[1]; }
  virtual void applyConstraint() override;
  virtual VKConstraintBase* clone() const override;

 private:
  double m_RC;
  double m_refP[2];
};


inline VKConstraintBase::VKConstraintBase(const int NC, int NTRK,
                                          VKContraintType t, VKVertex* vrt)
    : NCDim(NC),
      NTrk(NTRK),
      aa(NC, 0.),
      f0t(NTRK, std::vector<Vect3DF>(NC, Vect3DF())),
      h0t(NC, Vect3DF()),
      m_originVertex(vrt),
      m_type(t) {}
inline VKConstraintBase::~VKConstraintBase() = default;
// MASS constraint
inline VKMassConstraint::VKMassConstraint(int NTRK, double mass, VKVertex* vk)
    : VKConstraintBase(1, NTRK, VKContraintType::Mass, vk),
      m_usedParticles(NTRK, 0),
      m_targetMass(mass) {
  for (int i = 0; i < NTrk; i++)
    m_usedParticles[i] = i;
}

inline VKMassConstraint::VKMassConstraint(int NTRK, double mass,
                                          std::vector<int> listTrk,
                                          VKVertex* vk)
    : VKConstraintBase(1, NTRK, VKContraintType::Mass, vk),
      m_usedParticles(std::move(listTrk)),
      m_targetMass(mass) {
  m_originVertex = vk;
}
inline VKMassConstraint::~VKMassConstraint() = default;

//                   Angular constraints
inline VKPhiConstraint::VKPhiConstraint(int NTRK, VKVertex* vk)
    : VKConstraintBase(1, NTRK, VKContraintType::Phi, vk) {
  m_originVertex = vk;
}
inline VKPhiConstraint::~VKPhiConstraint() = default;

inline VKThetaConstraint::VKThetaConstraint(int NTRK, VKVertex* vk)
    : VKConstraintBase(1, NTRK, VKContraintType::Theta, vk) {
  m_originVertex = vk;
}
inline VKThetaConstraint::~VKThetaConstraint() = default;

//                   Pointing constraint
inline VKPointConstraint::VKPointConstraint(int NTRK, const double vrt[3],
                                            VKVertex* vk, bool onlyZ = false)
    : VKConstraintBase(2, NTRK, VKContraintType::Point, vk), m_onlyZ(onlyZ) {
  m_originVertex = vk;
  m_targetVertex[0] = vrt[0];
  m_targetVertex[1] = vrt[1];
  m_targetVertex[2] = vrt[2];
  // For Z only constraint
}
inline VKPointConstraint::~VKPointConstraint() = default;

//                   Vertex in plane constraint
inline VKPlaneConstraint::VKPlaneConstraint(int NTRK, double a, double b,
                                            double c, double d, VKVertex* vk)
    : VKConstraintBase(1, NTRK, VKContraintType::Plane, vk),
      m_A(a),
      m_B(b),
      m_C(c),
      m_D(d) {}

inline VKPlaneConstraint::~VKPlaneConstraint() = default;

//     Vertex at fixed radius
inline VKRadiusConstraint::VKRadiusConstraint(int NTRK, double RC, double RefP[2], VKVertex* vk)
    : VKConstraintBase(1, NTRK, VKContraintType::Radius, vk),
      m_RC(RC) {m_refP[0]=RefP[0]; m_refP[1]=RefP[1];}
inline VKRadiusConstraint::~VKRadiusConstraint() = default;


inline VKConstraintBase* VKMassConstraint::clone() const {
  return new VKMassConstraint(*this);
}
inline VKConstraintBase* VKPhiConstraint::clone() const {
  return new VKPhiConstraint(*this);
}
inline VKConstraintBase* VKThetaConstraint::clone() const {
  return new VKThetaConstraint(*this);
}
inline VKConstraintBase* VKPointConstraint::clone() const {
  return new VKPointConstraint(*this);
}
inline VKConstraintBase* VKPlaneConstraint::clone() const {
  return new VKPlaneConstraint(*this);
}
inline VKConstraintBase* VKRadiusConstraint::clone() const {
  return new VKRadiusConstraint(*this);
}


inline void VKMassConstraint::applyConstraint() {
  calcMassConstraint(this);
}
inline void VKPhiConstraint::applyConstraint() {
  calcPhiConstraint(this);
}
inline void VKThetaConstraint::applyConstraint() {
  calcThetaConstraint(this);
}
inline void VKPointConstraint::applyConstraint() {
  calcPointConstraint(this);
}
inline void VKPlaneConstraint::applyConstraint() {
  calcPlaneConstraint(this);
}
inline void VKRadiusConstraint::applyConstraint() {
  calcRadiusConstraint(this);
}


}  // namespace Trk
#endif
