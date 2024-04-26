## Check event weights in LHE file

Open the `tmp_run_01._00001.events.events` file and looking for the
`<initrwgt>` block in the header and the `<rwgt>` block in each event.

Also, once you have run showering (see below) you can use the
`checkSGMeta.py` script to check the weight names in your EVNT file:

     
    checkSGMeta.py EVNT.root

For more detailed instructions on how to check e.g. the nominal weight
is consistent between LHE and EVNT, you can follow the instructions
below.

<span class="twiki-macro TWISTY" mode="div"
showlink="Show weight checking instructions"
hidelink="Hide weight checking instructions"
showimgleft="%ICONURLPATH{toggleopen-small}%"
hideimgleft="%ICONURLPATH{toggleclose-small}%"></span> The steps are: 1)
Check LHE weight names 2) Check LHE weight values for a given index 3)
Check EVNT weight names 4) Check EVNT weight values for a given index

The commands and example output for each of these steps are given below:

1\)

    $ grep -A 15 "<initrwgt>" events.lhe 
    <initrwgt>
    <weightgroup name="Central scale variation" combine="envelope">
    <weight id="1" MUR="0.5" MUF="0.5" PDF="247000" > MUR=0.5 MUF=0.5  </weight>
    <weight id="2" MUR="0.5" MUF="1.0" PDF="247000" > MUR=0.5  </weight>
    <weight id="3" MUR="0.5" MUF="2.0" PDF="247000" > MUR=0.5 MUF=2.0  </weight>
    <weight id="4" MUR="1.0" MUF="0.5" PDF="247000" > MUF=0.5  </weight>
    <weight id="5" MUR="1.0" MUF="2.0" PDF="247000" > MUF=2.0  </weight>
    <weight id="6" MUR="2.0" MUF="0.5" PDF="247000" > MUR=2.0 MUF=0.5  </weight>
    <weight id="7" MUR="2.0" MUF="1.0" PDF="247000" > MUR=2.0  </weight>
    <weight id="8" MUR="2.0" MUF="2.0" PDF="247000" > MUR=2.0 MUF=2.0  </weight>
    </weightgroup> # scale
    <weightgroup name="NNPDF23_lo_as_0130_qed" combine="replicas"> # 247000: LO QCD + LO QED PDF set, alphas(MZ)=0.130. mem=0 ; average on replicas; mem=1-100 ; PDF replicas
    <weight id="9" MUR="1.0" MUF="1.0" PDF="247000" >  </weight>
    <weight id="10" MUR="1.0" MUF="1.0" PDF="247001" > PDF=247000 MemberID=1 </weight>
    <weight id="11" MUR="1.0" MUF="1.0" PDF="247002" > PDF=247000 MemberID=2 </weight>
    <weight id="12" MUR="1.0" MUF="1.0" PDF="247003" > PDF=247000 MemberID=3 </weight>

\(2\)

    $ grep "<wgt id='9'>" events.lhe 
    <wgt id='9'> +6.6195745e-05 </wgt>
    <wgt id='9'> +6.6195745e-05 </wgt>
    <wgt id='9'> +6.6195745e-05 </wgt>
    <wgt id='9'> +6.6195745e-05 </wgt>
    <wgt id='9'> +6.6195745e-05 </wgt>
    <wgt id='9'> +6.6195745e-05 </wgt>
    <wgt id='9'> +6.6195745e-05 </wgt>
    <wgt id='9'> +6.6195745e-05 </wgt>
    <wgt id='9'> +6.6195745e-05 </wgt>
    <wgt id='9'> +6.6195745e-05 </wgt>
    <wgt id='9'> +6.6195745e-05 </wgt>
    <wgt id='9'> +6.6195745e-05 </wgt>
    <wgt id='9'> +6.6195745e-05 </wgt>
    <wgt id='9'> +6.6195745e-05 </wgt>
    <wgt id='9'> +6.6195745e-05 </wgt>
    <wgt id='9'> +6.6195745e-05 </wgt>
    <wgt id='9'> +6.6195745e-05 </wgt>
    <wgt id='9'> +6.6195745e-05 </wgt>
    <wgt id='9'> +6.6195745e-05 </wgt>
    <wgt id='9'> +6.6195745e-05 </wgt>

\(3\)

    $ grep HepMCWeightSvc log.generate 
    10:59:07 HepMCWeightSvc       INFO Storing /Generation/Parameters :: WeightNames = {' MUF=0.5  ':46, ' MUF=2.0  ':57, ' MUR=0.5  ':24, ' MUR=0.5 MUF=0.5  ':0, ' MUR=0.5 MUF=2.0  ':35, ' MUR=2.0  ':79, ' MUR=2.0 MUF=0.5  ':68, ' MUR=2.0 MUF=2.0  ':90, ' PDF=13205 MemberID=0 ':14, ' PDF=247000 MemberID=1 ':1, ' PDF=247000 MemberID=10 ':23, ' PDF=247000 MemberID=100 ':11, ' PDF=247000 MemberID=11 ':25, ' PDF=247000 MemberID=12 ':26, ' PDF=247000 MemberID=13 ':27, ' PDF=247000 MemberID=14 ':28, ' PDF=247000 MemberID=15 ':29, ' PDF=247000 MemberID=16 ':30, ' PDF=247000 MemberID=17 ':31, ' PDF=247000 MemberID=18 ':32, ' PDF=247000 MemberID=19 ':33, ' PDF=247000 MemberID=2 ':12, ' PDF=247000 MemberID=20 ':34, ' PDF=247000 MemberID=21 ':36, ' PDF=247000 MemberID=22 ':37, ' PDF=247000 MemberID=23 ':38, ' PDF=247000 MemberID=24 ':39, ' PDF=247000 MemberID=25 ':40, ' PDF=247000 MemberID=26 ':41, ' PDF=247000 MemberID=27 ':42, ' PDF=247000 MemberID=28 ':43, ' PDF=247000 MemberID=29 ':44, ' PDF=247000 MemberID=3 ':16, ' PDF=247000 MemberID=30 ':45, ' PDF=247000 MemberID=31 ':47, ' PDF=247000 MemberID=32 ':48, ' PDF=247000 MemberID=33 ':49, ' PDF=247000 MemberID=34 ':50, ' PDF=247000 MemberID=35 ':51, ' PDF=247000 MemberID=36 ':52, ' PDF=247000 MemberID=37 ':53, ' PDF=247000 MemberID=38 ':54, ' PDF=247000 MemberID=39 ':55, ' PDF=247000 MemberID=4 ':17, ' PDF=247000 MemberID=40 ':56, ' PDF=247000 MemberID=41 ':58, ' PDF=247000 MemberID=42 ':59, ' PDF=247000 MemberID=43 ':60, ' PDF=247000 MemberID=44 ':61, ' PDF=247000 MemberID=45 ':62, ' PDF=247000 MemberID=46 ':63, ' PDF=247000 MemberID=47 ':64, ' PDF=247000 MemberID=48 ':65, ' PDF=247000 MemberID=49 ':66, ' PDF=247000 MemberID=5 ':18, ' PDF=247000 MemberID=50 ':67, ' PDF=247000 MemberID=51 ':69, ' PDF=247000 MemberID=52 ':70, ' PDF=247000 MemberID=53 ':71, ' PDF=247000 MemberID=54 ':72, ' PDF=247000 MemberID=55 ':73, ' PDF=247000 MemberID=56 ':74, ' PDF=247000 MemberID=57 ':75, ' PDF=247000 MemberID=58 ':76, ' PDF=247000 MemberID=59 ':77, ' PDF=247000 MemberID=6 ':19, ' PDF=247000 MemberID=60 ':78, ' PDF=247000 MemberID=61 ':80, ' PDF=247000 MemberID=62 ':81, ' PDF=247000 MemberID=63 ':82, ' PDF=247000 MemberID=64 ':83, ' PDF=247000 MemberID=65 ':84, ' PDF=247000 MemberID=66 ':85, ' PDF=247000 MemberID=67 ':86, ' PDF=247000 MemberID=68 ':87, ' PDF=247000 MemberID=69 ':88, ' PDF=247000 MemberID=7 ':20, ' PDF=247000 MemberID=70 ':89, ' PDF=247000 MemberID=71 ':91, ' PDF=247000 MemberID=72 ':92, ' PDF=247000 MemberID=73 ':93, ' PDF=247000 MemberID=74 ':94, ' PDF=247000 MemberID=75 ':95, ' PDF=247000 MemberID=76 ':96, ' PDF=247000 MemberID=77 ':97, ' PDF=247000 MemberID=78 ':98, ' PDF=247000 MemberID=79 ':99, ' PDF=247000 MemberID=8 ':21, ' PDF=247000 MemberID=80 ':100, ' PDF=247000 MemberID=81 ':102, ' PDF=247000 MemberID=82 ':103, ' PDF=247000 MemberID=83 ':104, ' PDF=247000 MemberID=84 ':105, ' PDF=247000 MemberID=85 ':106, ' PDF=247000 MemberID=86 ':107, ' PDF=247000 MemberID=87 ':108, ' PDF=247000 MemberID=88 ':109, ' PDF=247000 MemberID=89 ':110, ' PDF=247000 MemberID=9 ':22, ' PDF=247000 MemberID=90 ':111, ' PDF=247000 MemberID=91 ':2, ' PDF=247000 MemberID=92 ':3, ' PDF=247000 MemberID=93 ':4, ' PDF=247000 MemberID=94 ':5, ' PDF=247000 MemberID=95 ':6, ' PDF=247000 MemberID=96 ':7, ' PDF=247000 MemberID=97 ':8, ' PDF=247000 MemberID=98 ':9, ' PDF=247000 MemberID=99 ':10, ' PDF=25000 MemberID=0 ':15, '110':13, '9':101}

\(4\)

    $ root EVNT.pool.root
    root [0] 
    Attaching file EVNT.pool.root as _file0...
    (class TFile *) 0x3ee2ee0
    root [1] TTree* t=(TTree*) _file0->Get("CollectionTree")
    (class TTree *) 0x4b325f0
    root [2] t->Scan("McEventCollection_p5_GEN_EVENT.m_genEvents.m_weights[0][101]")
    ************************
    *    Row   * McEventCo *
    ************************
    *        0 * 6.619e-05 *
    *        1 * 6.619e-05 *
    *        2 * 6.619e-05 *
    *        3 * 6.619e-05 *
    *        4 * 6.619e-05 *
    *        5 * 6.619e-05 *
    *        6 * 6.619e-05 *
    *        7 * 6.619e-05 *
    *        8 * 6.619e-05 *
    *        9 * 6.619e-05 *
    *       10 * 6.619e-05 *
    *       11 * 6.619e-05 *
    *       12 * 6.619e-05 *
    *       13 * 6.619e-05 *
    *       14 * 6.619e-05 *
    *       15 * 6.619e-05 *
    *       16 * 6.619e-05 *
    *       17 * 6.619e-05 *
    *       18 * 6.619e-05 *
    *       19 * 6.619e-05 *
    *       20 * 6.619e-05 *
    *       21 * 6.619e-05 *
    *       22 * 6.619e-05 *
    *       23 * 6.619e-05 *
    *       24 * 6.619e-05 *
