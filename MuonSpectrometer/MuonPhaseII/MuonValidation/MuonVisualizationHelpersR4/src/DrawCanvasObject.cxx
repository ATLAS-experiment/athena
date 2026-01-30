

#include "DrawCanvasObject.h"


namespace MuonValR4::detail {
    using PrimitivePtr_t = DrawCanvasObject::PrimitivePtr_t;
    using PrimitiveVec_t = DrawCanvasObject::PrimitiveVec_t;
    using Range_t = DrawCanvasObject::Range_t;

    inline constexpr void expand(const double v, Range_t& r) {
        r[0] = std::min(r[0], v);
        r[1] = std::max(r[1], v);
    }
    inline void assignIfNotEmpty(const std::string& inStr, std::string& outStr) {
        if (!inStr.empty()){
            outStr = inStr;
        }
    }

    DrawCanvasObject::DrawCanvasObject(const std::string& canvasName, 
                                       const std::size_t evtNumber):
        m_name{canvasName},
        m_evt{evtNumber}{}

    double DrawCanvasObject::corner(const AxisRanges r) const {
        const double deltaX = 0.5*(m_axisRanges[0][1] - m_axisRanges[0][0]) * m_axisScale;
        const double deltaY = 0.5*(m_axisRanges[1][1] - m_axisRanges[1][0]) * m_axisScale;
        switch (r) {
            using enum AxisRanges;
            case xLow:
            case xHigh: {
                const double mid   = 0.5*(m_axisRanges[0][1] + m_axisRanges[0][0]);
                return mid + (r == xLow ? -1. : 1.) * std::max(deltaX, deltaY * (1. -2.*m_quadCan));
            } case yLow:
              case yHigh: {
                const double mid   = 0.5*(m_axisRanges[1][1] + m_axisRanges[1][0]);
                return mid + (r == yLow ? -1. : 1.) * std::max(deltaY, deltaX * (1. -2.*m_quadCan));
            }
        }
        return s_dblMax;
    }
    void DrawCanvasObject::setRangeScale(const double s, bool quadCan) { 
        m_axisScale = std::max(1., s); 
        m_quadCan = quadCan;
    }
    void DrawCanvasObject::trash() { m_isTrashed = true; }
    bool DrawCanvasObject::trashed() const { return m_isTrashed; }
    const std::string& DrawCanvasObject::name() const { return m_name; }
    std::size_t DrawCanvasObject::event() const { return m_evt; }
    const std::string& DrawCanvasObject::xTitle() const { return m_xTitle; }
    const std::string& DrawCanvasObject::yTitle() const { return m_yTitle; }
    const std::string& DrawCanvasObject::zTitle() const { return m_zTitle; }
    const PrimitiveVec_t& DrawCanvasObject::primitives() const { return m_primitives;  }
    PrimitiveVec_t& DrawCanvasObject::primitives() { return m_primitives; }
    void DrawCanvasObject::expandPad(const double x, const double y) {
        expand(x, m_axisRanges[0]);
        expand(y, m_axisRanges[1]);
    }
    void DrawCanvasObject::add(PrimitivePtr_t&& drawMe, const std::string& drawOpt) {
        m_primitives.emplace_back(std::move(drawMe), drawOpt);
    }
    void DrawCanvasObject::add(std::vector<PrimitivePtr_t> && drawMe) {
        for (PrimitivePtr_t& obj : drawMe) {
            add(std::move(obj));
        }
    }
    void DrawCanvasObject::setAxisTitles(const std::string& xTitle,
                                         const std::string& yTitle,
                                         const std::string& zTitle){
        assignIfNotEmpty(xTitle, m_xTitle);
        assignIfNotEmpty(yTitle, m_yTitle);
        assignIfNotEmpty(zTitle, m_zTitle);
    }
}