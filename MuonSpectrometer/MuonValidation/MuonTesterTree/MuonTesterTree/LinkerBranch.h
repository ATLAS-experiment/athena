/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTESTER_LINKERBRANCH_H
#define MUONTESTER_LINKERBRANCH_H

#include <MuonTesterTree/VectorBranch.h>
#include <MuonTesterTree/IParticleFourMomBranch.h>

namespace MuonVal{
    /** @brief The linker branch is a helper class to establish linking indices
     *         between dumped particle collections. Once the primary particle is
     *         added to the collection, the linker branch calls the linker function
     *         to find the related particle and to add it into it's forseen collection.
     *         The position where the related particle is saved in the tree, is then 
     *         saved by the linker branch */
    class LinkerBranch : public VectorBranch<unsigned short>,
                         virtual public IParticleDecorationBranch {
    public:
        /** @brief Abreviation of the pointer to the particle branch. */
        using ParticleBranch_ptr  = std::shared_ptr<IParticleFourMomBranch>;

        /** @brief Typedef of the linker function. The function takes as argument the primary
         *         particle of interest and shall then return the particle to associate with */
        using Linker_t = std::function<const xAOD::IParticle*(const xAOD::IParticle*)>;

        /** @brief Standard constructor fo the LinkerBranch
         *  @param parent: Particle branch collection where the linker branch is appended to. 
         *                 The primary particle of interest is provided by this collection.
         *  @param linkColl: Particle branch collection where the linked particle shall be appended
         *  @param linker: Function to find the linked particle from the primary particle
         *  @param altName: by default the name is <primaryColl>_linked<Secondary>. 
         *                  Parse an alternative name */
        LinkerBranch(IParticleFourMomBranch& parent,
                     ParticleBranch_ptr linkColl,
                     Linker_t  linker,
                     const std::string& altName = "");
        /** @brief Use the push back methods of the parent class */
        using VectorBranch<unsigned short>::push_back;
        /** @brief Interface methods to handle the particle */
        void push_back(const xAOD::IParticle* p) override;
        void push_back(const xAOD::IParticle& p) override;
        void operator+=(const xAOD::IParticle* p) override;
        void operator+=(const xAOD::IParticle& p) override;

    private:
        std::weak_ptr<IParticleFourMomBranch> m_linkColl;
        Linker_t m_linkerFunc;
    };

    class BilateralLinkerBranch: public VectorBranch<unsigned short>,
                                 virtual public IParticleDecorationBranch {
        public:
            using Linker_t = LinkerBranch::Linker_t;
            using ParticleBranch_ptr = LinkerBranch::ParticleBranch_ptr;
            /** @brief Use the push back methods of the parent class */
            using VectorBranch<unsigned short>::push_back;
            /** @brief Interface methods to handle the particle */
            void push_back(const xAOD::IParticle* p) override;
            void push_back(const xAOD::IParticle& p) override;
            void operator+=(const xAOD::IParticle* p) override;
            void operator+=(const xAOD::IParticle& p) override;
        
            bool fill(const EventContext& ctx) override;

            static bool connectCollections(ParticleBranch_ptr primColl,
                                           ParticleBranch_ptr secondColl,
                                           Linker_t fromPrimToSec,
                                           const std::string& altPrimName ="",
                                           const std::string& altSecName = "");
        private:
            BilateralLinkerBranch(IParticleFourMomBranch& bilatColl,
                                  ParticleBranch_ptr primColl,
                                  Linker_t linker,
                                  const std::string& altName);
            const IParticleFourMomBranch& m_parent;
            std::weak_ptr<IParticleFourMomBranch> m_linkColl;
            Linker_t m_linkerFunc;
    

    };
}
#endif