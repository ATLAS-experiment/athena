/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsGeometryInterfaces/TransformStore.h"
#include "GeoModelKernel/throwExcept.h"

#include <mutex>
namespace{
    std::mutex s_ticketMutex{};
    static const Amg::Transform3D dummy{Amg::Transform3D::Identity()};
}

namespace ActsTrk::detail {
    std::string TransformStore::toString(const Mode m) {
        switch (m) {
          using enum Mode; 
          case LazyFill: return "LazyFill";
          case Block: return "Block";
        }
        return "";
    }
    std::ostream& TransformStore::print(std::ostream& ostr) const {
        const auto f = filled();
        const auto s = size();
        ostr<<detectorType()<<" transform store in mode: "<<mode()
            <<", size: "<<s<<", (un)allocated: ("<<(s- f)<<")"<<f;
        return ostr;
    }

    TransformStore::Storage_t TransformStore::initStorage(const DetectorType dType,
                                                          const Mode mode) noexcept {
        const unsigned size = TrfStoreTicketCounter::distributedTickets(dType);
        switch (mode) {
            using enum Mode;
            case LazyFill: {
              LazyStorage_t store(size);
              return store;
            } case Block: {
               TrfVec_t trfStore(size, Amg::Transform3D::Identity());
               CheckVec_t isSet(size, 0);
               return std::make_pair(std::move(trfStore), std::move(isSet));
            }
        }
        return Storage_t{};
    }
    TransformStore::TransformStore(const DetectorType detType,
                                   const Mode m):
        m_storage{initStorage(detType, m)},
        m_detType{detType} {}
    TransformStore::TransformStore(const TransformStore& other) noexcept :
        m_storage{initStorage(other.detectorType(), other.mode())},
        m_detType{other.detectorType()} {
        if (mode() == Mode::Block){
            for (std::size_t t = 0; t < size(); ++t) {
                if (const Amg::Transform3D* copyMe = other.getTransform(t); copyMe != nullptr) {
                    setTransform(t, Amg::Transform3D{*copyMe});
                }
            }
        }
    }
            
    TransformStore& TransformStore::operator=(const TransformStore& other) noexcept {
        if (&other != this) {
            m_storage = initStorage(other.detectorType(), other.mode());
            m_detType = other.detectorType();
            if (mode() == Mode::Block){
              for (std::size_t t = 0; t < size(); ++t) {
                if (const Amg::Transform3D* copyMe = other.getTransform(t); 
                    copyMe != nullptr) {
                    setTransform(t, Amg::Transform3D{*copyMe});
                }
              }
            }
        }
        return (*this);
    }
    DetectorType TransformStore::detectorType() const { return m_detType; }
    const Amg::Transform3D& TransformStore::setTransform(const unsigned ticketNo, Amg::Transform3D && trf) const {
      return std::visit([&](auto& store) -> const Amg::Transform3D& {
          using Store_t = std::decay_t<decltype(store)>;
          if constexpr(std::is_same_v<Store_t, LazyStorage_t>){
              return (*store.at(ticketNo).set(std::make_unique<Amg::Transform3D>(std::move(trf))));
          } else if constexpr( std::is_same_v<Store_t, BlockStorage_t>) {
              THROW_EXCEPTION("The transform store "<<(*this)<<" has been initialized with"
                            <<" block storage caching. Cannot assign "<<ticketNo<<".");
              return dummy;
          } else {
            THROW_EXCEPTION("Unsupported storage type "<<(*this));
            return dummy;
          }
        }, m_storage);
    }
    std::size_t TransformStore::size() const {
       return std::visit([](const auto& store){
          using Store_t = std::decay_t<decltype(store)>;
          if constexpr(std::is_same_v<Store_t, LazyStorage_t>) {
              return store.size();
          } else {
              return store.first.size();
          }
       }, m_storage);
    }

    const Amg::Transform3D& TransformStore::setTransform(const unsigned ticketNo, Amg::Transform3D && trf) {
        return std::visit([&](auto& store) -> const Amg::Transform3D& {
          using Store_t = std::decay_t<decltype(store)>;
          if constexpr(std::is_same_v<Store_t, LazyStorage_t>) {
              return (*store.at(ticketNo).set(std::make_unique<Amg::Transform3D>(std::move(trf))));
          } else if constexpr(std::is_same_v<Store_t, BlockStorage_t>) {
            store.second.at(ticketNo) = true;
            return (store.first.at(ticketNo) = std::move(trf));
          } else {
            THROW_EXCEPTION("Unsupported storage type "<<(*this));
            return dummy;
          }
        }, m_storage);
    }

    TransformStore::Mode TransformStore::mode() const {
        return std::visit([&](const auto& store) {
          using Store_t = std::decay_t<decltype(store)>;
          if constexpr(std::is_same_v<Store_t, LazyStorage_t>) {
              return Mode::LazyFill;
          } else if constexpr(std::is_same_v<Store_t, BlockStorage_t>) {
              return Mode::Block;
          }
        }, m_storage);
    }

    std::size_t TransformStore::filled() const {
        return std::visit([](const auto& store) {
          using Store_t = std::decay_t<decltype(store)>;
          if constexpr(std::is_same_v<Store_t, LazyStorage_t>) {
              return std::count_if(store.begin(), store.end(), [](const auto& trf){
                  return trf.get() != nullptr;
              });
          } else if constexpr(std::is_same_v<Store_t, BlockStorage_t>) {
             return std::count_if(store.second.begin(), store.second.end(), [](const char filled){
                  return filled;
              });
          }
        }, m_storage);
    }



    using TicketCounterArr = TrfStoreTicketCounter::TicketCounterArr; 
    using ReturnedTicketArr = TrfStoreTicketCounter::ReturnedTicketArr;
    using ReturnedHintArr = TrfStoreTicketCounter::ReturnedHintArr;
    TicketCounterArr TrfStoreTicketCounter::s_clientCounter{};
    ReturnedTicketArr TrfStoreTicketCounter::s_returnedTickets{};
    ReturnedHintArr TrfStoreTicketCounter::s_returnedHints{};

    unsigned TrfStoreTicketCounter::drawTicket(const DetectorType type) { 
        std::unique_lock guard{s_ticketMutex};
        const unsigned idx = static_cast<unsigned>(type);
        std::vector<char>& returnedPool = s_returnedTickets[idx];
        int& returnedHint = s_returnedHints[idx];
        if (returnedPool.size() && returnedHint >= 0) {
            for (size_t i = returnedHint; i < returnedPool.size(); ++i) {
              if (returnedPool[i]) {
                returnedPool[i] = false;

                returnedHint = i+1;
                if (static_cast<size_t>(returnedHint) >= returnedPool.size()) {
                  returnedHint = 0;
                }
                return i;
              }
            }

            for (size_t i = 0; i < static_cast<size_t>(returnedHint); ++i) {
              if (returnedPool[i]) {
                returnedPool[i] = false;
                returnedHint = i+1;
                return i;
              }
            }
            returnedHint = -1;
        } else {
          returnedHint = -1;
        }
        return s_clientCounter[idx]++;
    }            
    unsigned TrfStoreTicketCounter::distributedTickets(const DetectorType type) { 
        std::unique_lock guard{s_ticketMutex};
        return s_clientCounter[static_cast<unsigned>(type)]; 
    }
    void TrfStoreTicketCounter::giveBackTicket(const DetectorType type, 
                                               unsigned ticketNo) {
        std::unique_lock guard{s_ticketMutex};
        const unsigned idx = static_cast<unsigned>(type);
        std::vector<char>& returnedPool = s_returnedTickets[idx];
        int& returnedHint = s_returnedHints[idx];
        /// The ticket which was handed out at the very latest is returned. Remove all returned tickets from before
        const unsigned distributed = s_clientCounter[idx];
        if (ticketNo == distributed -1) {

           if (ticketNo > 0 && ticketNo-1 < returnedPool.size()) {
             for (; ticketNo > 0 && returnedPool[ticketNo-1]; --ticketNo){}
             returnedPool.resize (ticketNo);
           }
           /// Remove all trailing ticket numbers
           s_clientCounter[idx] = ticketNo;
           if (returnedHint >= static_cast<int>(ticketNo)) {
             returnedHint = 0;
           }
        } else {
            if (returnedPool.size() <= ticketNo) {
              returnedPool.resize (ticketNo+1);
            }
            returnedPool[ticketNo] = true;
            if (returnedHint < 0 || static_cast<int>(ticketNo) < returnedHint) {
              returnedHint = ticketNo;
            }
        }
    }
}