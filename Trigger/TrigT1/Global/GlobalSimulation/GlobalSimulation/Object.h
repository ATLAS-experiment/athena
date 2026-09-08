/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef GLOBALSIM_OBJECT_H
#define GLOBALSIM_OBJECT_H

#include "AthContainers/AuxElement.h"
#include "xAODCore/tools/PrintHelpers.h"

namespace GlobalSim {

    template<typename Spec>
    class Object : public Spec::ObjectAcc {
    public:
        static constexpr Spec spec{}; // allows access to the BitSpec with obj.spec. syntax


        explicit Object(const SG::AuxElement &obj) : Spec::ObjectAcc(obj) {

        }

        explicit Object() : Object([]() -> const SG::AuxElement & {
            auto out = (new SG::AuxElement);
            out->makePrivateStore();
            return *out;
        }()) {
            // should remove this constructor in time ...
        }

        // ------------------------------------------------------------------------
        // Complete packed representation.
        // ------------------------------------------------------------------------

        Spec::bitset_type bits() const {
            return Spec::packFields(Spec::fields, Spec::ObjectAcc::m_obj);
        }

        void operator=(const Spec::bitset_type &bits) {
            std::apply([&](const auto *... field) {
                           (field->decodeAndAssignFrom(bits, const_cast<SG::AuxElement &>(Spec::ObjectAcc::m_obj)), ...);
                       },
                       Spec::fields);
        }

    public:
        const SG::AuxElement *objectPtr() const {
            return &this->Spec::ObjectAcc::m_obj;
        }

        void dump() const {
            xAOD::dump(*objectPtr());
        }


    };

}

#endif