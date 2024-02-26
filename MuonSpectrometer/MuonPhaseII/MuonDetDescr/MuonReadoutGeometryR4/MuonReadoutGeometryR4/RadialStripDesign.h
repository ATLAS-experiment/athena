/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONREADOUTGEOMETRYR4_RADIALSTRIPDESIGN_H
#define MUONREADOUTGEOMETRYR4_RADIALSTRIPDESIGN_H

#include <MuonReadoutGeometryR4/StripDesign.h>

namespace MuonGMR4{

/* The phi readout channels of the Tgc chambers are an essamble of wires that are readout 
 * together. The wires are equi-distant, but not every wire group in a gas gap contains the
 * same number of wires. Hence, the channel width & pitch vary across the board. The wire group 
 * design accounts for this detector design feature. It inherits from the StripDesign class and 
 * overloads the stripPosition feature to place the measurement onto the middle wire of each group.
 * Nevertheless, it's important to know about the position and the lenghts of each wire seperately
 * as these quantities are fed into the digitization. Therefore, extra methods are added to the design
 * to provide this information as well.
*/
class RadialStripDesign;
using RadialStripDesignPtr = GeoModel::TransientConstSharedPtr<RadialStripDesign>;
    
class RadialStripDesign: public StripDesign {
    public:
        RadialStripDesign() = default;
        /// set sorting operator
        bool operator<(const RadialStripDesign& other) const;
        
        /** @brief: Defines a new radial strip.
         *  @param: Intersection between the left strip edge & the bottom panel edge measured from the panel edge center
         *  @param: Intersection between the left strip edge & the top panel edge measured from the panel edge center
        */
        void addStrip(const double posOnBottom,
                      const double posOnTop);
        
        /** @brief: Returns the direction of the radial strip (Pointing from the bottom edge to the top edge)
          * @param: Strip number in the global scheme [1- nStrips()] 
        */
        Amg::Vector2D stripDir(int stripNumber) const;
        /** @brief: Returns the direction of the left edge */
        Amg::Vector2D stripLeftEdge(int stripNumber) const; 
        /** @brief: Returns the direction of the right edge */
        Amg::Vector2D stripRightEdge(int stripNumber) const;
        /** @bief: Returns the vector perpendicular to the stripDir and pointing to the next strip*/
        Amg::Vector2D stripNormal(int stripNumber) const;
        /** @brief: Returns the intersection of the left strip edge at the bottom panel's edge*/
        Amg::Vector2D stripLeftBottom(int stripNumber) const;
        /** @brief: Returns the intersecton of the strip right edge at the bottom panel's edge*/
        Amg::Vector2D stripRightBottom(int stripNumber) const;
        /** @brief: Returns the intersection of the left strip edge at the top panel's edge */
        Amg::Vector2D stripLeftTop(int stripNumber) const;
        /** @brief: Returns the intersecetion fo the right strip edge at the top panel's edge */
        Amg::Vector2D stripRightTop(int stripNumber) const;

        /// Returns the number of defined strips
        int numStrips() const override;
        /// Returns the associated channel number of an external vector
        int stripNumber(const Amg::Vector2D& extPos) const override final;


    private:
        CheckVector2D leftInterSect(int stripNum, bool uncapped = false) const override final;
        CheckVector2D rightInterSect(int stripNum, bool uncapped = false) const override final;
        /// @brief Helper struct to cache the mounting points of the strips with the bottom & top
        ///        edge of the Tgc panel. By convention, one instance of stripEdges represent the left
        ///        edge of strip i and the right ege of strip i-1, except for the very first & last one
        ///        which is just representing the left edge of the first & the last edge of the last strip.
        struct stripEdges{
            /// @brief Standard constrcutor
            /// @param dBot Mounting at the bottom edge of readout panel. 
            ///             Measured from the bottom left panel corner.
            /// @param dTop Mounting at the top edge of readout panel. 
            ///             Measured from the top left panel corner.
            /// @param _parent: Instance of the constructing RadialStripDesign to fetch the trapezoid boundaries
            stripEdges(double dBot, double dTop, 
                       const RadialStripDesign& _parent):
                parent{_parent},
                distOnBottom{dBot},
                distOnTop{dTop} {}

            /* Returns the bottom mounting point */
            Amg::Vector2D bottomMounting() const;
            /* Returns the top mounting point */
            Amg::Vector2D topMounting() const;
            /* Returns the connecting vector from bottom top */
            Amg::Vector2D fromBottomToTop() const;

            const RadialStripDesign& parent;
            double distOnBottom{0.};
            double distOnTop{0.};
        };
        using stripEdgeVec = std::vector<stripEdges>;
        using stripEdgeVecItr = stripEdgeVec::const_iterator;
        stripEdgeVec m_strips{};
        
        bool m_reversedStripOrder{false};

};

struct RadialDesignSorter{
    bool operator()(const RadialStripDesignPtr& a, const RadialStripDesignPtr& b) const {
            return (*a) < (*b);
    }
    bool operator()(const RadialStripDesign&a ,const RadialStripDesign& b) const {
        return a < b;
    }
};

using RadialStripDesignSet = std::set<RadialStripDesignPtr, RadialDesignSorter>;

}
#include <MuonReadoutGeometryR4/RadialStripDesign.icc>
#endif