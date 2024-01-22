/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <MuonReadoutGeometryR4/StripDesign.h>
#include <MuonReadoutGeometryR4/WireGroupDesign.h>
#include <MuonReadoutGeometryR4/RadialStripDesign.h>

#include <GaudiKernel/SystemOfUnits.h>

#include <TGraph.h>
#include <TFile.h>
using namespace MuonGMR4;

void addPoint(TGraph& graph, const Amg::Vector2D& point) {
    graph.SetPoint(graph.GetN(), point.x(), point.y());
}
void createGraph(const StripDesign& design, TFile& outFile, const std::string& graphName) {
    std::unique_ptr<TGraph> graph = std::make_unique<TGraph>();
    Eigen::Rotation2D rot{design.stereoAngle()};
    addPoint(*graph, design.cornerBotLeft());
    addPoint(*graph, design.cornerBotRight());
    addPoint(*graph, design.cornerTopRight());
    addPoint(*graph, design.cornerTopLeft());
    addPoint(*graph, design.cornerBotLeft());
    for (int strip = design.firstStripNumber(); strip <= design.numStrips(); ++strip) {
        addPoint(*graph, rot * design.leftEdge(strip).value_or(Amg::Vector2D::Zero()));
        addPoint(*graph, rot * design.rightEdge(strip).value_or(Amg::Vector2D::Zero()));
        addPoint(*graph, rot * design.leftEdge(strip).value_or(Amg::Vector2D::Zero()));
    }
    std::cout<<"################################################################"<<std::endl;
    std::cout<<design<<std::endl;
    std::cout<<"################################################################"<<std::endl;
    outFile.WriteObject(graph.get(), graphName.c_str());
}

int main() {
    constexpr double halfHeight = 200. * Gaudi::Units::mm;
    constexpr double shortEdge  = 150. * Gaudi::Units::mm;
    constexpr double longEdge   = 300. * Gaudi::Units::mm;

    constexpr double stripPitch = 5 * Gaudi::Units::mm;
    constexpr double stripWidth = stripPitch / 3;
    constexpr double stereoAngle = 20. * Gaudi::Units::deg;
    constexpr unsigned int numStrips = 2.*halfHeight / stripPitch -1; 
    std::unique_ptr<TFile> file = std::make_unique<TFile>("/srv/build/Strip.root", "RECREATE");
   
    StripDesign nominalDesign{};
    nominalDesign.defineTrapezoid(shortEdge, longEdge, halfHeight);
    nominalDesign.defineStripLayout(Amg::Vector2D{-halfHeight + 0.5*stripPitch,0},
                                     stripPitch, stripWidth, numStrips, 0);
    /// 
    createGraph(nominalDesign, *file, "NominalDesign");
    
    /// Flip the strip design
    StripDesign flippedDesign{};
    flippedDesign.defineTrapezoid(shortEdge, longEdge, halfHeight);
    flippedDesign.flipTrapezoid();
    constexpr unsigned numStripsRot = 2*longEdge / stripPitch -1;
    flippedDesign.defineStripLayout(Amg::Vector2D{-longEdge + 0.5*stripPitch,0},
                                     stripPitch, stripWidth, numStripsRot, 0);
   
    createGraph(flippedDesign,*file, "FlippedDesign");

    StripDesign rotatedDesign{};
    rotatedDesign.defineTrapezoid(shortEdge, longEdge, halfHeight, stereoAngle);
    rotatedDesign.defineStripLayout(Amg::Vector2D{-halfHeight + 0.5*stripPitch,0},
                                        stripPitch, stripWidth, numStrips, 0);
    /// 
    createGraph(rotatedDesign, *file, "StereoDesign");

    StripDesign rotatedDesignNeg{};
    rotatedDesignNeg.defineTrapezoid(shortEdge, longEdge, halfHeight, -stereoAngle);
    rotatedDesignNeg.defineStripLayout(Amg::Vector2D{-halfHeight + 0.5*stripPitch,0},
                                        stripPitch, stripWidth, numStrips, 0);
    /// 
    createGraph(rotatedDesignNeg, *file, "NegStereoDesign");
    
    StripDesign flippedRotated{};
    flippedRotated.defineTrapezoid(shortEdge, longEdge, halfHeight, stereoAngle);
    flippedRotated.defineStripLayout(Amg::Vector2D{-longEdge + 0.5*stripPitch,0},
                                        stripPitch, stripWidth, numStrips, 0);
    flippedRotated.flipTrapezoid();
    /// 
    createGraph(flippedRotated, *file, "StereoFlipped");   

    WireGroupDesign groupDesign{};
    groupDesign.defineTrapezoid(shortEdge, longEdge, halfHeight);
    {

    
        unsigned int wireCounter{1}, totWires{0}, nCycles{0};
        int sign{1};
        while(totWires< numStrips) {
            groupDesign.declareGroup(wireCounter);
            totWires+=wireCounter;
            if (wireCounter == 1) sign = 1;
            else if (wireCounter == 5) sign = -1;
            wireCounter+=sign;
            ++nCycles;
        }
        groupDesign.defineStripLayout(Amg::Vector2D{-halfHeight + 0.5*stripPitch,0},
                                                    stripPitch, stripWidth, nCycles, 0);

    }
    createGraph(groupDesign, *file, "WireGroups");

    WireGroupDesign flipedWireGroups{};
    flipedWireGroups.defineTrapezoid(shortEdge, longEdge, halfHeight);
    flipedWireGroups.flipTrapezoid();
    {

    
        unsigned int wireCounter{1}, totWires{0}, nCycles{0};
        int sign{1};
        while(totWires< numStrips) {
            flipedWireGroups.declareGroup(wireCounter);
            totWires+=wireCounter;
            if (wireCounter == 1) sign = 1;
            else if (wireCounter == 5) sign = -1;
            wireCounter+=sign;
            ++nCycles;
        }
        flipedWireGroups.defineStripLayout(Amg::Vector2D{-longEdge + 0.5*stripPitch,0},
                                                        stripPitch, stripWidth, nCycles, 0);

    }
    createGraph(flipedWireGroups, *file, "FlippedWireGroups");


    RadialStripDesign flippedRadialDesign{};
    flippedRadialDesign.defineTrapezoid(shortEdge, longEdge, halfHeight);
    flippedRadialDesign.flipTrapezoid();
    {
        constexpr std::array<double, 15> bottomMountings{-0.95 * shortEdge, -0.76 * shortEdge, -0.63 *shortEdge, 
                                                         -0.57 * shortEdge, -0.41 * shortEdge, -0.21 *shortEdge, 
                                                                         0,  0.16 * shortEdge,  0.34 *shortEdge, 
                                                         0.42 *  shortEdge,  0.53 * shortEdge,  0.66 *shortEdge,
                                                         0.75 *  shortEdge,  0.86 * shortEdge,  0.99 *shortEdge};
     
        constexpr std::array<double, 15> topMountings{-0.99 *longEdge, -0.86 *longEdge, -0.75 *longEdge, 
                                                      -0.66 *longEdge, -0.53 *longEdge, -0.42 *longEdge, 
                                                                    0,  0.21 *longEdge,  0.34 *longEdge, 
                                                       0.41 *longEdge,  0.57 *longEdge,  0.63 *longEdge,
                                                       0.75 *longEdge,  0.86 *longEdge,  0.99 *longEdge};

    
        for (size_t i =0; i < bottomMountings.size(); ++i) {
            flippedRadialDesign.addStrip(bottomMountings[i], topMountings[i]);
        }
    }    
    createGraph(flippedRadialDesign, *file, "FlippedRadialDesign");

    RadialStripDesign RadialDesign{};
    RadialDesign.defineTrapezoid(shortEdge, longEdge, halfHeight);
    {
        const double edgeLength = 0.5* std::hypot(shortEdge - longEdge, 2* halfHeight);
        std::array<double, 15> mountings{-0.95 * edgeLength, -0.76 * edgeLength, -0.63 *edgeLength, 
                                         -0.57 * edgeLength, -0.41 * edgeLength, -0.21 *edgeLength, 
                                                          0,  0.16 * edgeLength,  0.34 *edgeLength, 
                                         0.42 *  edgeLength,  0.53 * edgeLength,  0.66 *edgeLength,
                                         0.75 *  edgeLength,  0.86 * edgeLength,  0.99 *edgeLength};
     
        for (size_t i =0; i < mountings.size(); ++i) {
            RadialDesign.addStrip(mountings[i], -mountings[mountings.size()- 1 - i]);
        }
    }
    createGraph(RadialDesign, *file, "RadialDesign");
    return EXIT_SUCCESS;
}