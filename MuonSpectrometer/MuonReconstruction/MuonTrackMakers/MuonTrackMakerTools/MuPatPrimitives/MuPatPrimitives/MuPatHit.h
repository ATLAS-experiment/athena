/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUPATHIT_H
#define MUPATHIT_H

#include <list>
#include <mutex>

#include "CxxUtils/atomic_fetch_minmax.h"
#include "Identifier/Identifier.h"
#include "MuonStationIndex/MuonStationIndex.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkEventPrimitives/TrkObjectCounter.h"
#include "TrkMeasurementBase/MeasurementBase.h"


namespace Muon {

    /**
        List of MuPatHit pointers.
    */
    class MuPatHit;
    using MuPatHitPtr = std::shared_ptr<MuPatHit>;    
    using MuPatHitList = std::vector<MuPatHitPtr>;
    using MuPatHitCit = MuPatHitList::const_iterator;
    using MuPatHitIt = MuPatHitList::iterator;

    class MuPatHit : public Trk::ObjectCounter<MuPatHit> {
    public:
        enum Type { UnknownType = -1, MDT = 0, RPC = 1, TGC = 2, CSC = 3, MM = 4, sTGC = 5, PREC = 6, Pseudo = 7, Scatterer = 8 };
        enum Status { UnknownStatus = -1, OnTrack = 0, Outlier, NotOnTrack };
        enum HitSelection { UnknownSelection = -1, Precise = 0, Broad = 1 };

        struct Info {
            Info() = default;
            /// Identifier of the measurement. Is invalid in cases of pseudo measurements
            Identifier id{};

            /// Measurement type as defined above
            Type type{Pseudo};
            //// Type of the MDT hit
            HitSelection selection{UnknownSelection};
            /// Flag whether the hit is on Tack or not
            Status status{OnTrack};
            /// Station index of the Identifier BI
            MuonStationIndex::StIndex stIdx{MuonStationIndex::StIndex::StUnknown};
            /// Chamber index of the Identifier
            MuonStationIndex::ChIndex chIdx{MuonStationIndex::ChIndex::ChUnknown};
            /// Small or large sector?
            bool isSmall{false};
            /// Hit in the endcap?
            bool isEndcap{false};
            /// Does the hit constrain phi?
            bool measuresPhi{true};
            ///
        };

        /** @brief construction taking all members as argument, ownership is taken only of the broadMeas.
          @param pars         predicted TrackParameters at the surface of the measurement
          @param presMeas     precisely calibrated measurement
          @param broadMeas    measurement with enlarged errors
          @param presResPull  residual and pull of the hit for the precise measurement
          @param broadResPull residual and pull of the hit for the broad measurement
          @param id           Hit Identifier (can be invalid (Pseudos), user should check validity)
          @param type         Hit type enum
          @param measuresPhi boolean indicating whether this is an eta or phi measurement
          @param used        enum indicating the hit status
        */
        MuPatHit(std::shared_ptr<const Trk::TrackParameters> pars, 
                 std::shared_ptr<const Trk::MeasurementBase> presMeas,
                 std::shared_ptr<const Trk::MeasurementBase> broadMeas,
                 const Info& info);

        /** @brief copy constructor */
        MuPatHit(const MuPatHit& hit);

        /** assignment operator */
        MuPatHit& operator=(const MuPatHit&);

        /** destructor */
        virtual ~MuPatHit() = default;

        /** @brief returns a reference to the TrackParameters */        
        const Trk::TrackParameters& parameters() const;

        /** @brief returns a reference to the selected measurement */
        const Trk::MeasurementBase& measurement() const;

        /** @brief returns a reference to the hit info */
        const Info& info() const;

        /** @brief returns a reference to the hit info */
        Info& info();

        /** @brief clones the MuPatHit */
        std::unique_ptr<MuPatHit> clone() const { return std::make_unique<MuPatHit>(*this); }

        /** @brief returns precise measurement */
        const Trk::MeasurementBase& preciseMeasurement() const;

        /** @brief returns broad measurement */
        const Trk::MeasurementBase& broadMeasurement() const;
        
        void setResidual(double residual, double pull);

        /** @brief returns the residual of the measurement */
        double residual() const;
        /** @brief returns the pull of the measurement */
        double pull() const;

        /** @brief maximum number of objects of this type in memory */
        static unsigned int maxNumberOfInstantiations();

        /** current number of objects of this type in memory */
        static unsigned int numberOfInstantiations();

        /** @brief number of times the copy constructor was called since last reset */
        static unsigned int numberOfCopies();

    private:
        /** @brief copy hit */
        void copy(const MuPatHit& hit);

        // private member data
        std::shared_ptr<const Trk::TrackParameters> m_pars{};
        std::shared_ptr<const Trk::MeasurementBase> m_precisionMeas{};
        std::shared_ptr<const Trk::MeasurementBase> m_broadMeas;
        Info m_info{};
        double m_residual{0.};
        double m_pull{0.};
    };  // class MuPatHit

}  // namespace Muon

#endif
