      subroutine herwig7_init(maxev,file)
      implicit none
      integer maxev
      character *(*) file

      character * 6 WHCPRG
      common/cWHCPRG/WHCPRG
      include 'herwigsettings.inc'
      real * 8 powheginput
      external powheginput
      logical, save :: ini = .true.
      integer, save :: saved_maxev
      integer innlodec
      common/c_innlodec/innlodec
      integer itmp
      real * 8 ptmin,ptsqmin
      common/ptmin/ptmin
      real *8 btildecorr, remncorr
      common/corrfactors/btildecorr, remncorr
      integer maxweights, numweights,radtype
      parameter(maxweights=50)
      real *8 weight
      common/weights/weight(maxweights),numweights,radtype


      if(ini) then
         if( powheginput("#ttb_NLO_dec") == 1) then
            innlodec = 1
         else
            innlodec = 0
         endif
         write(*,*)'herwig7_init: trying to open file ',
     $        '<'//trim(file)//'>'
         call opencountlocal(file,maxev)
         write(*,*)'herwig7_init: found ',maxev,' events'
         itmp = powheginput("#maxev")
         if( itmp > 0) then
            maxev = min(maxev, itmp)
         endif
         saved_maxev = maxev
         ptsqmin = powheginput("#ptsqmin")
         if(ptsqmin .le. 0d0) ptsqmin = 0.8d0
         ptmin = sqrt(ptsqmin)
   
      else
c     Herwig calls open many times; we count the events
c     and initialize things only once         
         maxev=saved_maxev
         return
      endif
      ini = .false.

      WHCPRG='HERWIG'
      
      eventCounter = 0


      nohad = powheginput("#nohad")
c     read allrad (default is 1)
      allrad = powheginput("#allrad")
      if(allrad<0) allrad=1
c     this flag must be present if we are running the hvq generator.
c     the hvq program generates charm, bottom or top depending upon the qmass value
      hvq_gen = powheginput("#qmass") > 0
c     read nlowhich (default is 0). This flag ment:
c     0 all that can radiate is active
c     1 only radiation in production
c     2 only radiation in ??? (check in ttb_NLO_dec
c     It is relevant for the ttb_NLO_dec generator. Here
c     we keep it zero.

      
      nlowhich=powheginput("#nlowhich")
      if(nlowhich.lt.0) nlowhich=0

      if(nlowhich.gt.1) then
         write(*,*) 'supported only nlowhich=0 (def), 1, exiting ...'
         call exit(-1)
      endif         
         

      if(powheginput("#LOevents") == 1) then
         weveto = .false.
      elseif((hvq_gen).or.(nlowhich.eq.1)) then
         weveto = .false.
c     The hvq program does not generate radiation in decay.
c     The ttb_NLO_dec one does not generate radiation in decay if nlowhich=1.
c     In all cases Herwig should not veto radiation in decays. The
c     following should not matter.
         nlowhich = 1
      elseif(allrad.ne.0) then
c     allrad 1 is for running bbar4l in default mode, and also for ttb_NLO_dec
         allrad=1
         weveto = .true.
      else
c     this covers the allrad 0 case
         weveto = .true.
      endif


      if(powheginput('#herwigveto') == 1) then
         if(.not.  weveto) then
            write(*,*)' herwig7_init: something wrong: for weveto'//
     1           'to be false it must be either LOevent, or hvq '//
     2           'or nlowhich 1; in all these cases we would not want '//
     3           'herwigveto 1. Exiting ...'
            call exit(-1)
         endif
         weveto=.false.
      endif
      

      scalupfac=powheginput('#scalupfac')
      if(scalupfac.lt.0) scalupfac=1

c     read in btilde and remn corrections factors (used together with
c     ubexcess_correct at the generation stage)

      ub_btilde_corr = powheginput('#ub_btilde_corr')
      if (ub_btilde_corr < 0d0) then
        ub_btilde_corr = 1d0
      endif
      ub_remn_corr = powheginput('#ub_remn_corr')
      if (ub_remn_corr < 0d0) then
        ub_remn_corr = 1d0
      endif
      btildecorr = ub_btilde_corr
      remncorr = ub_remn_corr 
      call init_hist

      contains
      subroutine opencountlocal(file,maxev)
      implicit none
      include 'pwhg_rnd.h'
      integer maxev,iun
      character * (*) file
      character * 20 pwgprefix
      integer lprefix
      common/cpwgprefix/pwgprefix,lprefix
      integer ios
      character * 7 string
      real * 8 powheginput
      external powheginput
      integer nev,j
      maxev=0
      call pwhg_io_open_read(trim(file),iun,ios)
      if(ios.ne.0) then
         write(*,*) 'cannot open; aborting ...'
         call exit(-1)
      endif
 1    continue
      call pwhg_io_read(iun,string,ios)
      if(ios /= 0) goto 2
      if(string.eq.'</event') then
         maxev=maxev+1
         goto 1
      endif
      goto 1
 2    continue
      write(*,*) ' Found ',maxev,' events in file ',file
      if (maxev.eq.0) then
         write(*,*) ' NO EVENTS!! Program exits'
         call exit(3)
      endif
      call pwhg_io_close(iun)
      end subroutine

      end

      subroutine herwig7_calculatescales()
      implicit none
      include 'LesHouches.h'
      include 'nlegborn.h'
      include 'pwhg_rad.h'
      include 'pwhg_rwl.h'
      integer j
      real * 8 vetoscaletp,vetoscaletm,
     1         vetoscalewp,vetoscalewm
      common/resonancevetos/vetoscaletp,vetoscaletm,
     1                      vetoscalewp,vetoscalewm
      real * 8 powheginput
      external powheginput
      include 'herwigsettings.inc'
      
      scalup=scalupfac*scalup

      if(nlowhich.eq.0) then
         if(allrad.eq.1) then
            call findresscale( 6,vetoscaletp)
            call findresscale(-6,vetoscaletm)
c 7 and 9 are the fermions from W+ or W-; if they are hadrons,
c find the veto scales.
            if(abs(idup(7)).lt.5) then
               call findresscale(24,vetoscalewp)
            else
               vetoscalewp = 1d30
            endif
            if(abs(idup(9)).lt.5) then
               call findresscale(-24,vetoscalewm)
            else
               vetoscalewm = 1d30
            endif
         else
            vetoscaletp = scalup
            vetoscaletm = scalup
         endif
      elseif(nlowhich.eq.1) then
         continue
      else
         write(*,*) 'nlowhich = ',nlowhich,' not yet implemented'
         call pwhg_exit(-1)
      endif

c useful to check that he7 is doing what we are asking
      eventCounter = eventCounter+1
      if(eventCounter.lt.10) then
      write(*,*) '************************************************************'
      write(*,*) 'veto scales: ',eventCounter,scalup,vetoscaletp,vetoscalewp,
     1     vetoscaletm,vetoscalewm
      write(*,*) '*************************************************************'
      endif
      flush(6)      
      end

c     idres = idhep of resonance
c     scale = transverse momentum of the (powheg) emission
c             in the resonance decay
      subroutine findresscale(idres,scale)
      implicit none
      include "LesHouches.h"
      real * 8 ptmin
      common/ptmin/ptmin
      integer idres
      real * 8 scale
      real * 8 p(0:3,3),p0(0:3),dotp,
     1     pg(0:3), pb(0:3), pbg(0:3), pw(0:3), pq(0:3), pa(0:3)
      real * 8 modpg,modpb,modpbg,tvirt2,wvirt2,tvirt
      integer nres,ids(3)
      real * 8 yq,ya,csi,q2
      integer j,k
      real * 8 q_herwig, z_herwig, ctheta_bg, ctheta_b, ctheta_g, nvec(0:3)
      nres=0
      q_herwig=1d10
      z_herwig=1d0
      do j=3,nup
         if(idup(j).eq.idres) then
            p0(1:3) = pup(1:3,j)
            p0(0) = pup(4,j)
            do k=3,nup
               if(mothup(1,k).eq.j) then
                  if(nres.ge.3) then
                     write(*,*)
     1                    ' findresscale: error: more than 3 '
     1                    //'particles in resonance decay'
                     call exit(-1)
                  endif
                  nres = nres+1
                  p(1:3,nres) = pup(1:3,k)
                  p(0,nres) = pup(4,k)
                  ids(nres) = idup(k)
               endif
            enddo
            goto 10
         endif
      enddo
 10   continue
      if(nres == 0) then
c     No resonance found, set scale to high value
c     Herwig will shower any MC generated resonance unrestricted
         scale = 1d30
         return
      elseif(nres.lt.3) then
c No radiating resonance found
         scale = ptmin
         return
      endif
      call boost2reson(p0,nres,p,p)
      if(abs(idres).eq.6) then
         do j=1,3
            if(abs(ids(j)).eq.24) then
               pw = p(:,j)
            elseif(abs(ids(j)).eq.5) then
               pb = p(:,j)
            elseif(abs(ids(j)).eq.21) then
               pg = p(:,j)
            endif
         enddo

         scale = sqrt( 2 * dotp(pg,pb) * pg(0)/pb(0) )


c     q_tilde Herwig scale as if the radiation was generated
c     by Herwig itself. May be useful for some checks in the future
         if(.false.) then
            pbg = pb + pg
         
            modpb = sqrt(pb(1)**2+pb(2)**2+pb(3)**2)
            modpbg = sqrt(pbg(1)**2+pbg(2)**2+pbg(3)**2)
            modpg = pg(0)
            ctheta_b = (pb(1)*pbg(1)+pb(2)*pbg(2)+pb(3)*pbg(3))/
     1           (modpb*modpbg)
            ctheta_g = (pg(1)*pbg(1)+pg(2)*pbg(2)+pg(3)*pbg(3))/
     1           (modpg*modpbg)
            ctheta_bg = (pb(1)*pg(1)+pb(2)*pg(2)+pb(3)*pg(3))/
     1           (modpb*modpg)

c       Light-like vector for the computation of the z components
            nvec(1:3) = - pbg(1:3)/modpbg
            nvec(0) = 1
            
            tvirt2 = dotp(p0,p0)
            tvirt  = sqrt(tvirt2)
            wvirt2 = dotp(pw,pw)
            

c     from formula 6.10 of arXiv/0803.0883v3 page 35;
c     theta_{\tilde{ij}} is zero for the first emission
            q_herwig = sqrt(2d0 * pbg(0)**2 * (1-ctheta_bg)
     1           *  (1+1)**2/(1+ctheta_b)/(1+ctheta_g) )
            
            z_herwig = dotp(nvec,pb)/dotp(nvec,pbg)
         endif
         
      elseif(abs(idres).eq.24) then
         do j=1,3
            if(ids(j).eq.21) then
               pg = p(:,j)
            elseif(ids(j).gt.0) then
               pq = p(:,j)
            elseif(ids(j).lt.0) then
               pa = p(:,j)
            endif
         enddo
         p0 = pg + pq + pa
         q2 = dotp(p0,p0)
         csi = 2*pg(0)/sqrt(q2)
         yq = 1 - dotp(pg,pq)/(pg(0)*pq(0))
         ya = 1 - dotp(pg,pa)/(pg(0)*pa(0))
         scale = sqrt(min(1-yq,1-ya)*csi**2*q2/2)
      endif
      end
      
