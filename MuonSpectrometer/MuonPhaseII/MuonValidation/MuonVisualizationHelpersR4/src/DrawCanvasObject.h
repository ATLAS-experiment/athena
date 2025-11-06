/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONVISUALIZATIONHELPERSR4_DRAWCANVASOBJECT_H
#define MUONVISUALIZATIONHELPERSR4_DRAWCANVASOBJECT_H
#include <MuonRecToolInterfacesR4/IRootVisualizationService.h>

#include <TObject.h>
#include <Acts/Utilities/ArrayHelpers.hpp>
#include <climits>
#include <atomic>

namespace MuonValR4::detail {
    class DrawCanvasObject : public IRootVisualizationService::ICanvasObject {
        public:
            using PrimitivePtr_t = IRootVisualizationService::PrimitivePtr_t;
            using PrimitiveVec_t = std::vector<std::pair<PrimitivePtr_t, std::string>>;
            using Range_t = std::array<double, 2>;
            using Range2D_t = std::array<Range_t, 2>;

            DrawCanvasObject(const std::string& canvasName, 
                             const std::size_t evtNumber);
            virtual ~DrawCanvasObject() = default;
            
            virtual void expandPad(const double x, const double y) override final;

            virtual void add(PrimitivePtr_t&& drawMe,
                             const std::string& drawOpt ="") override final;
            virtual void add(std::vector<PrimitivePtr_t>&& drawMe) override final;

            virtual double corner(const AxisRanges r) const override final;
            virtual void setRangeScale(const double s, bool quadCan) override final;
            virtual void trash() override final;
                    
            const PrimitiveVec_t& primitives() const;
            PrimitiveVec_t& primitives();

            virtual void setAxisTitles(const std::string& xTitle,
                                       const std::string& yTitle,
                                       const std::string& zTitle) override final;
            /** @brief Returns the title of the x-axis */
            const std::string& xTitle() const;
            /** @brief Returns the title of the y-axis */
            const std::string& yTitle() const;
            /** @brief Returns the title of the z-axis */
            const std::string& zTitle() const; 
            /** @brief Event number in which the canvas has been created */
            std::size_t event() const;
            /** @brief Name of the canvas */
            const std::string& name() const;
            /** @brief Returns whether the canvas has been trashed */
            bool trashed() const;


        private:
            static constexpr double s_dblMax = std::numeric_limits<double>::max();
            Range2D_t m_axisRanges{Acts::filledArray<Range_t, 2>(Range_t{s_dblMax, -s_dblMax})};
            double m_axisScale{1.};
            PrimitiveVec_t m_primitives{};
            
            std::string m_name{};            
            std::size_t m_evt{};

            std::string m_xTitle{};
            std::string m_yTitle{};
            std::string m_zTitle{};

            bool m_quadCan{false};
            std::atomic<bool> m_isTrashed{false};


 
    };
}

#endif
