/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4__MUONPATTERNHELPERS_COMBINATORIALSEEDSOLVER__H
#define MUONR4__MUONPATTERNHELPERS_COMBINATORIALSEEDSOLVER__H

#include "GeoPrimitives/GeoPrimitives.h"
#include <AthenaBaseComps/AthMessaging.h>

#include<iostream>
#include <ranges>
#include <vector>
#include <concepts>

namespace MuonR4{

  namespace CombinatorialSeedSolver{


      //solve the layers equation to define the seed (M,DM) with the combinatoric hits
      //***The layers equations for the four layers (Si,Di) can be */
      // S1 + lamda*D1 = M
      // S2 + alpha*D2 = M + A*Dm
      // S3 + gamma*D3 = M + G*Dm
      // S4 + kappa*D4 = M + K*Dm
      // where {Si,Di}  are the position and direction of the strip
      // {M,Dm} the position and direction of the muon's trajectory on the 1st plane
      // solving the system we can reduce it to a 2x2 system for kappa and gamma ---> //Johanes' documentation with analytical solution
      // Bx1*kappa + Bx2*gamma = Y2x
      // By1*kappa + By2*gamma = Y2y

      // check if the space points of the container satisfy the require to access to the position and direction in chamber's frame
      template<typename Container>
      concept hasPositionAndDirection = requires(typename Container::value_type& v){

          {v->directionInChamber()}->std::same_as<const Amg::Vector3D&>;
          {v->positionInChamber()}->std::same_as<const Amg::Vector3D&>;

      }; 

      //check if they are raw pointers 
      template<typename Container>
      concept hasPointerValues = requires {
      typename Container::value_type; 
      requires std::is_pointer_v<typename Container::value_type>; 
      };


      // check if the container is valid ( e.g std::array or std::vector)
      template<typename Container>
      concept acceptedContainer = requires(const Container& c){
        {c.size()} ->std::same_as<std::size_t>;
        requires hasPositionAndDirection<Container>;

      };  

   

      /// @brief defines the betaMatrix calculated from the combinatoric hits
      /// @tparam spacePointContainer the space point container 
      /// @param spacePoints the space points of the combinatorics
      /// @return the 2x2 beta matrix
      template<typename spacePointContainer>
      requires acceptedContainer<spacePointContainer> && hasPointerValues<spacePointContainer>
      AmgSymMatrix(2) betaMatrix(const spacePointContainer& spacePoints){

        AmgSymMatrix(2) bMatrix{AmgSymMatrix(2)::Identity()};
        Amg::Vector3D b1Aterm = (spacePoints[3]->directionInChamber().dot(spacePoints[1]->directionInChamber()))*spacePoints[1]->directionInChamber()-spacePoints[3]->directionInChamber();
        Amg::Vector3D b1Gterm = (spacePoints[3]->directionInChamber().dot(spacePoints[0]->directionInChamber()))*spacePoints[0]->directionInChamber() - (spacePoints[3]->directionInChamber().dot(spacePoints[1]->directionInChamber()))*spacePoints[1]->directionInChamber(); 

        Amg::Vector3D b2Aterm = spacePoints[2]->directionInChamber()- (spacePoints[2]->directionInChamber().dot(spacePoints[1]->directionInChamber()))*spacePoints[1]->directionInChamber();
        Amg::Vector3D b2Kterm = (spacePoints[2]->directionInChamber().dot(spacePoints[1]->directionInChamber()))*spacePoints[1]->directionInChamber() - (spacePoints[2]->directionInChamber().dot(spacePoints[0]->directionInChamber()))*spacePoints[0]->directionInChamber();
    
        //get the distances of the layers along z direction
        double A = (spacePoints[0]->positionInChamber().z() - spacePoints[1]->positionInChamber().z());
        double G = (spacePoints[0]->positionInChamber().z() - spacePoints[2]->positionInChamber().z());
        double K = (spacePoints[0]->positionInChamber().z() - spacePoints[3]->positionInChamber().z());

        //define B matrix
        Amg::Vector3D b1 = A*b1Aterm + G*b1Gterm;
        Amg::Vector3D b2 = K*b2Kterm + A*b2Aterm;

        bMatrix.col(0) = Amg::Vector2D(b1.x(), b1.y());
        bMatrix.col(1) = Amg::Vector2D(b2.x(), b2.y());

        return bMatrix;
      };

      /// @brief calculates the parameters lamda,alpha,gamma,kappa of the system
      /// @tparam spacePointContainer the space point container
      /// @param betaMatrix the betaMatrix for the system 
      /// @param spacePoints the space points of the combinatorics
      /// @return an array of the calculated parameters
      template<typename spacePointContainer>     
      requires acceptedContainer<spacePointContainer> && hasPointerValues<spacePointContainer>
      std::array<double,4> defineParameters(AmgSymMatrix(2) betaMatrix, const spacePointContainer& spacePoints){

        double A = (spacePoints[0]->positionInChamber().z() - spacePoints[1]->positionInChamber().z());
        double G = (spacePoints[0]->positionInChamber().z() - spacePoints[2]->positionInChamber().z());
        double K = (spacePoints[0]->positionInChamber().z() - spacePoints[3]->positionInChamber().z());

        //Define y2 for the linear system
        Amg::Vector3D y0 = K*(spacePoints[2]->positionInChamber()- spacePoints[0]->positionInChamber()) + G*(spacePoints[0]->positionInChamber()- spacePoints[3]->positionInChamber());
        Amg::Vector3D y1 = A*(spacePoints[3]->positionInChamber() - spacePoints[2]->positionInChamber()) + G*(spacePoints[1]->positionInChamber() - spacePoints[3]->positionInChamber()) + K*(spacePoints[2]->positionInChamber()-spacePoints[1]->positionInChamber());
        Amg::Vector3D y2 = (K-G)*(spacePoints[0]->positionInChamber()- spacePoints[1]->positionInChamber()) - (y1.dot(spacePoints[1]->directionInChamber()))*spacePoints[1]->directionInChamber() + (y0.dot(spacePoints[0]->directionInChamber()))*spacePoints[0]->directionInChamber() +A*(spacePoints[3]->positionInChamber()- spacePoints[2]->positionInChamber());

        Amg::Vector2D solution = betaMatrix.inverse()*y2.block<2,1>(0,0);
        double kappa = solution.x();
        double gamma = solution.y();

        double lamda = (y0.dot(spacePoints[0]->directionInChamber()) + K*gamma*(spacePoints[0]->directionInChamber().dot(spacePoints[2]->directionInChamber())) -G*kappa*(spacePoints[0]->directionInChamber().dot(spacePoints[3]->directionInChamber())))/(K-G);
        double alpha = (y1.dot(spacePoints[1]->directionInChamber())+(A-G)*kappa*spacePoints[3]->directionInChamber().dot(spacePoints[1]->directionInChamber()) + (K-A)*gamma*spacePoints[2]->directionInChamber().dot(spacePoints[1]->directionInChamber()))/(K-G);
        
        return std::array<double,4>({lamda,alpha,gamma,kappa});

      };

      //Solve the equations for the seed position and direction (M,DM)
      /// @brief solves the equation system to calculate the seed
      /// @tparam spacePointContainr the space point container
      /// @param spacePoints the space points of the combinatorics
      /// @param parameters the lamda,alpha,gamma,kappa paramaters of the four layers
      /// @return the pair of the seed position and direction
      template<typename spacePointContainer>     
      requires acceptedContainer<spacePointContainer> && hasPointerValues<spacePointContainer>
      std::pair<Amg::Vector3D, Amg::Vector3D> seedSolution(const spacePointContainer& spacePoints, const std::array<double,4>& parameters){
        
        //estimate the seed positionInChamber from the 1st equation of the system of the layer equations
        Amg::Vector3D seedPosition = spacePoints[0]->positionInChamber() + parameters[0]*spacePoints[0]->directionInChamber();

        //estimate the seed direction from the 2nd equation of the system of the layer equations
        Amg::Vector3D seedDirection = ((spacePoints[1]->positionInChamber() + parameters[1]*spacePoints[1]->directionInChamber() - seedPosition)).unit();

        Amg::Vector3D seedPositionZ0 = seedPosition + Amg::intersect<3>(seedPosition, seedDirection, Amg::Vector3D::UnitZ(), 0.).value_or(0.)*seedDirection;

    

        return std::make_pair(seedPositionZ0, seedDirection.z() > 0 ? seedDirection : -seedDirection);

      };


    };

}

#endif