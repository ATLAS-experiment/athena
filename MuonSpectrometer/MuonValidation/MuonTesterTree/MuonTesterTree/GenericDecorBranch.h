/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTESTER_GENERICDECORBRANCH_H
#define MUONTESTER_GENERICDECORBRANCH_H
#include <MuonTesterTree/VectorBranch.h>
#include <functional>

namespace MuonVal{
  template<class AuxType, class dType>
      class GenericAuxEleBranch : public VectorBranch<dType>, virtual public IAuxElementDecorationBranch  {
          public:
            /** @brief Defined the function signature to extract the information from the
             *         AuxElement*/  
            using Filler_t = std::function<dType(const AuxType& )>;

            /** @brief Constructor taking the raw pointer to the underlying TTree object.
             *  @param t: Pointer to the TTree
             *  @param branchName: Name of the output branch inside the TTree
             *  @param fillFunc: Function to extract the information from the parsed object */
            GenericAuxEleBranch(TTree* t, const std::string& branchName, Filler_t fillFunc);
            /** @brief Constructor taking the raw pointer to the underlying TTree object.
             *  @param t: Pointer to the TTree
             *  @param branchName: Name of the output branch inside the TTree
             *  @param fillFunc: Function to extract the information from the parsed object */
            GenericAuxEleBranch(MuonTesterTree& t, const std::string& var_name, Filler_t fillFunc);


            void push_back(const SG::AuxElement* p) override;
            void push_back(const SG::AuxElement& p) override;
            void operator+=(const SG::AuxElement* p) override;
            void operator+=(const SG::AuxElement& p) override;
        
            using VectorBranch<dType>::push_back;
        
            virtual ~GenericAuxEleBranch() = default;
        private:
          Filler_t m_fillFunc;
    };

    template <class PartType, class dType> 
        class GenericPartDecorBranch: public GenericAuxEleBranch<PartType, dType>,  
                                      virtual public IParticleDecorationBranch {
          public:
              using GenericAuxEleBranch<PartType, dType>::GenericAuxEleBranch;

              using GenericAuxEleBranch<PartType, dType>::push_back;
              using GenericAuxEleBranch<PartType, dType>::operator+=;

              void push_back(const xAOD::IParticle* p) override;
              void push_back(const xAOD::IParticle& p) override;
              void operator+=(const xAOD::IParticle* p) override;
              void operator+=(const xAOD::IParticle& p) override;
    };
}

#include "MuonTesterTree/GenericDecorBranch.icc"
#endif
