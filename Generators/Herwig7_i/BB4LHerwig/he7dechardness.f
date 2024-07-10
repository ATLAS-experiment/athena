      function veto_rad_in_decay(ichtop)
c ichtop = +- 1, is the charge of the top.
c it returns the hardness of b radiation
      implicit none
      logical veto_rad_in_decay
      integer ichtop
      real * 8 vetoscaletp,vetoscaletm,
     1     vetoscalewp,vetoscalewm
      common/resonancevetos/vetoscaletp,vetoscaletm,
     1     vetoscalewp,vetoscalewm
      real * 8 hardness_b, hardness_g, resscale
      integer i
      include 'hepevt.h'
      integer jhep, lastIstance
      integer i_top,i_b,i_w,i_g,j,k,wid,bid,tid
      real * 8 pchain(4,3), pb(4), pg(4), pson(4), pb1(4)
      real * 8 phepdot, angle

      goto 156
      if (ichtop==1) then
         write(*,*) 'BEGIN EVENT'
         do jhep=1,nhep
            write(*,*) jhep, idhep(jhep), isthep(jhep), JMOHEP(1:2,jhep),jdahep(1:2,jhep)         
         enddo
         write(*,*) 'END EVENT'
      endif
 156  continue

      veto_rad_in_decay=.false.
      tid = 6*ichtop
      wid = 24*ichtop
      bid = 5*ichtop
c     find last top in record
      i_top = -1
      do jhep=1,nhep
         if(idhep(jhep).eq.tid) then
            i_top = jhep
         endif
      enddo
      if(i_top == -1) then
         return
      endif
      pchain(:,1)=phep(1:4,i_top)   !the first element in the chain must be the top quark

      if(tid.eq.6) then
         resscale=vetoscaletp
      else
         resscale=vetoscaletm
      endif

      hardness_b =-1            ! Max hardness from bottom 
      hardness_g =-1            ! Max hardness from gluon (if present at LHE level)
      
      i_w = 0                   ! Position of the W
      i_b = 0                   ! Position of the bottom
      i_g = 0                   ! Position of the gluon (if present at lhe level)

c     look for top direct sons (they can be 2, Wb, or 3, Wbg)    
      if(jdahep(2,i_top)-jdahep(1,i_top).eq.1) then         
         if(idhep(jdahep(1,i_top)).eq.wid) then
            i_w = jdahep(1,i_top)
            i_b = jdahep(2,i_top)
         else
            i_w = jdahep(2,i_top)
            i_b = jdahep(1,i_top)
         endif         
         if(idhep(i_w).ne.wid) then
            write(*,*) ' top did not go in W! The program exits'
            call pwhg_exit(-1)
         elseif(idhep(i_b).ne.bid) then
            write(*,*) ' top did not go in b! The program exits'
            call pwhg_exit(-1)
         endif         
      elseif(jdahep(2,i_top)-jdahep(1,i_top).eq.2) then
         do i=jdahep(1,i_top), jdahep(2,i_top)
            if(idhep(i).eq.wid) then
               i_w = i
            elseif (idhep(i).eq.bid) then
               i_b = i
            elseif (idhep(i).eq.21) then  
               i_g = i
            endif
         enddo
         if ( (i_w.eq.0) .or. (i_b.eq.0) .or. (i_g .eq. 0)) then
            write(*,*) ' Not found the W or the b or the gluon'
            write(*,*) ' from top decay. The program exits'
            call pwhg_exit(-1)
         endif
      else
         write(*,*) 'Top decay products not found. The program exits'
         call pwhg_exit(-1)
      endif

c     Take the last istance of the top sons
      i_b = lastIstance(i_b)    !Last b reshuffled...
      if(i_g > 0) then
         i_g=lastIstance(i_g)  
      endif


cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c     Radiation from bottom quark: we follow the bottom line to find
c     the hardest b -> bg emission; if there is no b->bg emission
c     hardness_b remains -1.
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc


c     continue untill we find a final state bottom
      do while( isthep(i_b) .ne. 1)
         if(jdahep(2,i_b)-jdahep(1,i_b).eq.0) then
c     1 son: it can be a reshuffled b quark or a cluster (if hadronization is turned on)
            i_b=jdahep(1,i_b)
c     If the b quark entered a cluster, leave the loop
            if(idhep(i_b) .ne. bid) exit
         elseif(jdahep(2,i_b)-jdahep(1,i_b).eq.1) then
c     2 sons: b -> b g emission
            if(idhep(jdahep(1,i_b)).eq.bid) then
               pchain(:,2) = phep(1:4,jdahep(1,i_b))
               pchain(:,3) = phep(1:4,jdahep(2,i_b))
               i_b = jdahep(1,i_b)
            else
               pchain(:,2) = phep(1:4,jdahep(2,i_b))
               pchain(:,3) = phep(1:4,jdahep(1,i_b))
               i_b = jdahep(2,i_b)
            endif
            call boost2reson4(pchain(:,1),2,pchain(:,2:3),pchain(:,2:3))
            hardness_b = sqrt( 2 * phepdot(pchain(:,2),pchain(:,3))
     2           * pchain(4,3)/pchain(4,2) )
            if(hardness_b > resscale) then
               veto_rad_in_decay=.true.
               return
            endif
         else
            write(*,*) 'found b -> more than 2 particles, exiting...'
            call pwhg_exit(-1)
         endif
      enddo



cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c     Radiation from gluon: we search for the hardest gluon
c     splittig, in case of gg splitting we follow the gluon with the
c     largest z.  We stop in case of qqbar splitting or 
c     hadronization. If there is no gluon splitting, or there is no
c     gluon at the LHE level, hardness_g remains -1.
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc


      if(i_g.ne.0) then
         do while( isthep(i_g) .ne. 1)
            if(jdahep(2,i_g)-jdahep(1,i_g).eq.0) then
c     1 son: it can be a reshuffled b quark or a cluster (if hadronization is turned on)
               i_g=jdahep(1,i_g)
c     If the gluon entered a cluster, leave the loop
               if(idhep(i_g) .ne. 21) exit
            elseif(jdahep(2,i_g)-jdahep(1,i_g).eq.1) then
c     2 sons: g splitting
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!     if(idhep(jdahep(1,i_g)).ne.21) exit
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
               pchain(:,2) = phep(1:4,jdahep(1,i_g))
               pchain(:,3) = phep(1:4,jdahep(2,i_g))
               call boost2reson4(pchain(:,1),2,pchain(:,2:3),pchain(:,2:3))   
               hardness_g = sqrt( 2 * phepdot(pchain(:,2),pchain(:,3))
     2              * (pchain(4,3)*pchain(4,2))/(pchain(4,3)+pchain(4,2))**2)
               if(hardness_g > resscale) then
                  veto_rad_in_decay=.true.
                  return
               endif
               if(idhep(jdahep(1,i_g)).eq.21) then
c     In case of gg splitting, we follow the one with the largest z
                  if(pchain(4,3) > pchain(4,2) ) then
                     i_g = jdahep(2,i_g)
                  else
                     i_g = jdahep(1,i_g)
                  endif
               else
c     In case of a g- > q q~ splitting we end our search.
                  exit
               endif
            else
               write(*,*) 'found g -> more than 2 particles, exiting...'
               call pwhg_exit(-1)
            endif
         enddo
      endif
      
      end
      
      function lastIstance(istance)
      implicit none
      include 'hepevt.h'
      integer lastIstance, istance, id
      lastIstance = istance
      do while( (isthep(lastIstance) .ne. 1) .and.
     $     (jdahep(2,lastIstance)-jdahep(1,lastIstance).eq.0) )  ! 1 son       
         if(idhep(lastIstance) .eq. idhep(jdahep(1,lastIstance)) ) then
            lastIstance = jdahep(1,lastIstance) ! The son id is the same as the particle id (to avoid cluster)
         else
           exit !(the son is a cluster, keep our lastIstance and exit)
         endif
      enddo
      end

      
      function angle(pin1, pin2)
      real *8 pin1(1:4), pin2(1:4), angle, ScalarProd, mod1, mod2
      integer i
      ScalarProd =0
      do i=1,3
         ScalarProd =ScalarProd+ pin1(i)*pin2(i)
         mod1= mod1+pin1(i)**2
         mod2= mod2+pin2(i)**2
      enddo
      angle = acos(abs(ScalarProd)/sqrt(mod1*mod2)) 
      end

      function phepdot(vec1, vec2)
      implicit none
      real * 8 phepdot
      real * 8 vec1(1:4), vec2(1:4)
      intent(in) :: vec1, vec2
c     ordering in phep seems to be px, py, pz, e
      phepdot = - vec1(1) * vec2(1) - vec1(2) * vec2(2) - vec1(3) * vec2(3) + vec1(4) * vec2(4)

      end function phepdot




      subroutine he7dechardness(ichtop,hardness)
c ichtop = +- 1, is the charge of the top.
c it returns the hardness of b radiation
      implicit none
      integer ichtop
      real * 8 hardness, hardness_b, hardness_g
      integer i
      include 'hepevt.h'
      integer jhep, lastIstance
      integer i_top,i_b,i_w,i_g,j,k,wid,bid,tid
      real * 8 pchain(4,3), pb(4), pg(4), pson(4), pb1(4)
      real * 8 phepdot, angle
      tid = 6*ichtop
      wid = 24*ichtop
      bid = 5*ichtop
c     find last top in record
      i_top = -1
      do jhep=1,nhep
         if(idhep(jhep).eq.tid) then
            i_top = jhep
         endif
      enddo
      if(i_top == -1) then
         hardness = 0
         return
      endif
      pchain(:,1)=phep(1:4,i_top)   !the first element in the chain must be the top quark


      hardness = -1             ! Max hardness 
      hardness_b =-1            ! Max hardness from bottom 
      hardness_g =-1            ! Max hardness from gluon (if present at lhe level)
      
      i_w = 0                   ! Position of the W
      i_b = 0                   ! Position of the bottom
      i_g = 0                   ! Position of the gluon (if present at lhe level)

c     look for top direct sons (they can be 2, Wb, or 3, Wbg)    
      if(jdahep(2,i_top)-jdahep(1,i_top).eq.1) then         
         if(idhep(jdahep(1,i_top)).eq.wid) then
            i_w = jdahep(1,i_top)
            i_b = jdahep(2,i_top)
         else
            i_w = jdahep(2,i_top)
            i_b = jdahep(1,i_top)
         endif         
         if(idhep(i_w).ne.wid) then
            write(*,*) ' top did not go in W!'
            goto 998
         elseif(idhep(i_b).ne.bid) then
            write(*,*) ' top did not go in b!'
            goto 998
         endif         
      elseif(jdahep(2,i_top)-jdahep(1,i_top).eq.2) then
         do i=jdahep(1,i_top), jdahep(2,i_top)
            if(idhep(i).eq.wid) then
               i_w = i
            elseif (idhep(i).eq.bid) then
               i_b = i
            elseif (idhep(i).eq.21) then  
               i_g = i
            endif
         enddo
         if ( (i_w.eq.0) .or. (i_b.eq.0) .or. (i_g .eq. 0)) then
            write(*,*) ' was not expecting this!'
            goto 998
         endif
      else
         write(*,*) ' was not expecting this!'
         goto 998
      endif

c     Take the last istance of the top sons
      i_b = lastIstance(i_b)    !Last b reshuffled...
      if(i_g >0) then
         i_g=lastIstance(i_g)  
      endif



cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c     Radiation from bottom quark: we follow the bottom line to find
c     the hardest b -> bg emission; if there is no b->bg emission
c     hardness_b remains -1.
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      
c     If it enters here at the first iteration (hardness_b .eq. -1)
c     it is an error (goto 998); otherwise it could mean that the
c     last bottom has hadronized: we keep the last hardess_b found.         
 111  if(isthep(i_b) .eq. 1) then
         go to 222
      elseif(jdahep(2,i_b)-jdahep(1,i_b).gt.1) then
         if(hardness_b .eq. -1) then
            write(*,*) ' found b-> more than 2 particles'
            goto 998
         else
            go to 222
         endif
c     b->bg emission: we save in the chain the b and the g momenta
c     and we store in i_b the position of the last b found
      elseif(idhep(jdahep(1,i_b)).eq.bid
     1        .and.idhep(jdahep(2,i_b)).eq.21) then
c     the b has radiated a gluon
         pchain(:,2) = phep(1:4,jdahep(1,i_b))
         pchain(:,3) = phep(1:4,jdahep(2,i_b))
         i_b = lastIstance(jdahep(1,i_b))
      elseif(idhep(jdahep(2,i_b)).eq.bid
     1        .and.idhep(jdahep(1,i_b)).eq.21) then
c     the b has radiated a gluon
         pchain(:,2) = phep(1:4,jdahep(2,i_b))
         pchain(:,3) = phep(1:4,jdahep(1,i_b))
         i_b = lastIstance(jdahep(2,i_b))
      else
c     Last bottom (before hadronization), we keep the last hardness_b
c     found.  
         go to 222            
      endif
      
c     now pchain contains the 4-momenta of the top and the b-g pair:
c     we compute the hardness of this emission and repeat this procedure
c     with the new bottom found (we boost the bg in the t reference frame)    
      call boost2reson4( pchain(:,1), 2, pchain(:,2:3), pchain(:,2:3) )      
      hardness_b = max(hardness_b,
     1     sqrt( 2 * phepdot(pchain(:,2),pchain(:,3))
     2     * pchain(4,3)/pchain(4,2) ) )
      go to 111
      
 222  continue

cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c     Radiation from gluon: we search for the hardest gluon
c     splittig, in case of gg splitting we follow the gluon with the
c     largest z.  We stop in case of qqbar splitting or 
c     hadronization. If there is no gluon splitting, or there is no
c     gluon at the lhe level, hardness_g remains -1.
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      
c     Skip the analysis of the gluon radiation from top if it is
c     absent at the lhe level
      if (i_g .eq. 0 ) go to 444

c     If it enters here at the first iteration (hardness_g .eq. -1)
c     it is an error (goto 998); otherwise it could mean that the
c     last gluon has hadronized: we keep the last hardess_g found.
 333  if(jdahep(2,i_g)-jdahep(1,i_g).gt.1) then
         if(hardness_g .eq. -1) then
            write(*,*) ' found g-> more than 2 particles'
            goto 998
         else
            go to 444
         endif
c     Gluon splitting
      elseif(jdahep(2,i_g)-jdahep(1,i_g).eq.1) then
c     g->gg
         if(idhep(jdahep(1,i_g)).eq. 21) then
            pchain(:,2) = phep(1:4,jdahep(1,i_g))
            pchain(:,3) = phep(1:4,jdahep(2,i_g))
c     we boost the gg pair in the t reference frame before
c     computing the hardness of this process
            call boost2reson4(pchain(:,1),2,pchain(:,2:3),pchain(:,2:3))   
            hardness_g = max(hardness_g,
     1           sqrt( 2 * phepdot(pchain(:,2),pchain(:,3))
     2           * (pchain(4,3)*pchain(4,2))
     3           /(pchain(4,3)+pchain(4,2))**2))
            
c     In case of gg splitting, we take the one with the largest z
            if(pchain(4,3) > pchain(4,2) ) then
               i_g = jdahep(2,i_g)
            else
               i_g = jdahep(1,i_g)
            endif
            i_g = lastIstance(i_g)
            go to 333
c     In case of qqbar splitting we don't proceede any further
         else
            go to 444
         endif
c     Final state gluon (it could happen if hadr is off)
      else
          go to 444
      endif

cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c     Take the maximum value
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
 444  continue
      hardness = max(hardness_b, hardness_g)
      goto 999

cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c     Error message
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc      
 998  continue
      write(*,*) 'top=',i_top
      do j=1,nhep
         write(*,100)j,isthep(j),idhep(j),jmohep(1,j),
     1        jmohep(2,j),jdahep(1,j),jdahep(2,j), (phep(k,j),k=1,5)
      enddo
      call exit(-1)
 100  format(i4,2x,i5,2x,i5,2x,i4,1x,i4,2x,i4,1x,i4,2x,5(d10.4,1x))
 999  end


      
