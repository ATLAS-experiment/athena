/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTESTER_AUXELEMENTBRANCH_H
#define MUONTESTER_AUXELEMENTBRANCH_H

#include <MuonTesterTree/VectorBranch.h>
namespace MuonVal {
template <class T> class AuxElementBranch : public VectorBranch<T>, virtual public IAuxElementDecorationBranch {
public:
    void setAccessor(const std::string& name);
    void push_back(const SG::AuxElement* p) override;
    void push_back(const SG::AuxElement& p) override;
    void operator+=(const SG::AuxElement* p) override;
    void operator+=(const SG::AuxElement& p) override;

    using VectorBranch<T>::push_back;
    using VectorBranch<T>::initialized;
    using VectorBranch<T>::getDefault;
    using VectorBranch<T>::hasDefault;

    AuxElementBranch(TTree* t, const std::string& var_name, const std::string& acc = "");
    AuxElementBranch(MuonTesterTree& t, const std::string& var_name, const std::string& acc = "");

    virtual ~AuxElementBranch() = default;

private:
    SG::AuxElement::ConstAccessor<T> m_acc;
};
template <class T> class ParticleVariableBranch : public AuxElementBranch<T>, virtual public IParticleDecorationBranch {
public:
    ParticleVariableBranch(TTree* t, const std::string& var_name, const std::string& acc = "");
    ParticleVariableBranch(MuonTesterTree& t, const std::string& var_name, const std::string& acc = "");
    virtual ~ParticleVariableBranch() = default;

    void push_back(const xAOD::IParticle* p) override;
    void push_back(const xAOD::IParticle& p) override;
    using AuxElementBranch<T>::push_back;
    using AuxElementBranch<T>::operator+=;
    void operator+=(const xAOD::IParticle* p) override;
    void operator+=(const xAOD::IParticle& p) override;
};

template <class T> class ParticleVariableBranchGeV : public ParticleVariableBranch<T> {
public:
    ParticleVariableBranchGeV(TTree* t, const std::string& var_name, const std::string& acc = "");
    ParticleVariableBranchGeV(MuonTesterTree& t, const std::string& var_name, const std::string& acc = "");

    using ParticleVariableBranch<T>::push_back;
    using ParticleVariableBranch<T>::get;
    using ParticleVariableBranch<T>::size;
    void push_back(const xAOD::IParticle* p) override;
    void push_back(const xAOD::IParticle& p) override;
};
}
#include <MuonTesterTree/AuxElementBranch.icc>
#endif
