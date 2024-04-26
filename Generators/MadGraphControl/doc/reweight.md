# MadGraph5_aMC@NLO Reweighting Module for ATLAS

### Introduction Built-in in MadGraph5_aMC@NLO.2.3.2 and later, with significant updates in v2.4, v2.5, and v2.6

-   Module is described on [MadGraph reweight homepage](https://cp3.irmp.ucl.ac.be/projects/madgraph/wiki/Reweight)
-   Brief description:
    -   **Method consists in using a sample of events generated under a
        certain theoretical hypothesis and in associating with those
        events an additional weight that corresponds to a new
        theoretical hypothesis**
    -   Based solely on matrix-element computations, reweighting factor
        in LO: \|M\_{new}\|^2/\|M\_{old}\|^2
    -   New hypothesis can be new model or new parameter values
    -   Weights can be propagated through analysis chain to switch
        between hypotheses without re-doing full simulation
    -   Method exists in LO and in three variants in NLO (one of which
        is NLO accurate)
    -   Can be run during event generation (this is the normal use case)
        but also standalone on an LHE file

### Running reweighting in Athena

Follow the example in the main README.md

### The reweight card

The reweighting is steered by the `reweight_card.dat`. Possible commands
in the reweight card are described
[here](https://cp3.irmp.ucl.ac.be/projects/madgraph/wiki/Reweight#Contentofthereweight_card).

The following card will create three additional weights with the names
`cwww0_cw0`, `cwww20_cw0`, and `cwww0_cw20`. Here `dim6 1` and `dim6 2`
correspond to the first two entries in the block `dim6` in the
`param_card.dat` and they are modified in the reweighting.

    # content of reweight_card.dat ============

    launch --rwgt_name=cwww0_cw0
        set dim6 1 0
        set dim6 2 0

    launch --rwgt_name=cwww20_cw0
        set dim6 1 20
        set dim6 2 0

    launch --rwgt_name=cwww20_cw0
        set dim6 1 0
        set dim6 2 20
    # ====================================

If needed, changing of the model and the process can be performed as
follows:

    # content of another reweight_card.dat ======
    change model EWdim6
    change process p p > w+ w- NP=2 HIG=0

    launch --rwgt_name=EWdim6_cwww0_cw0
     /path/to/param_card_cwww0_cw0.dat

    launch --rwgt_name=EWdim6_cwww20_cw0
     /path/to/param_card_cwww20_cw0.dat

    launch --rwgt_name=EWdim6_cwww0_cw20
     /path/to/param_card_cwww0_cw20.dat
    # ====================================

Whenever the model is changed a completely new `param_card.dat` needs to
be provided (by specifying the path, as above), otherwise this is
optional.

#### Scan syntax (standalone runing only)

The MadGraph scan syntax can be used to express the above reweight card
equivalently using a python list.

    launch
       set Dim6 1 scan1:[20 if i==1 else 0 for i in range(3)] # == [0, 20, 0]
       set Dim6 2 scan1:[20 if i==2 else 0 for i in range(3)] # == [0, 0, 20]

There are two types of scan commands in MadGraph that can both be useful
for adding a large number of weights. The command `scan1` can be used to
modify multiple parameters simultaneously, as above. It can also be
combined with `scan2`, `scan3` etc for multidimensional scans. The
`scan` command scans all combinations of parameter values, so that the
example below will give 1000 additional weights with values of the 3
parameters between 0 and 9.

    launch 
       set dim6 1 scan:range(10)
       set dim6 2 scan:range(10)
       set dim6 3 scan:range(10)

Note that this syntax can also be used in the `param_card.dat` to
generate independet MCs with different parameter settings. This is
especially useful when validating the reweighting.

### Reading and applying weights

The weights obtained from the reweight module are alternative weights
and should replace the nominal event weight. In case the nominal event
weight does not correspond to the one from MadGraph (e.g. because it has
been renormalised or additional reweightings have been performed), the
new weight can be defined as =new_weight = old_weight \*
mgreweight_weight / nominal_weight=.

#### Reading the weights from the LHE file

Weights are stored in block called `rwgt` in the LHE file with the
user-defined `wgt id`. The LHE file header additionally contains
information about the parameters that were changed.

### Things to consider

#### Limitations

-   Parameters as defined in the `param_card.dat` can be changed but
    changes of PDFs or scales are not possible
-   It is possible to change the physics model, as long as the new model
    is an UFO model
-   The process can be changed as long as the final state remains the
    same
-   **Reweighting only gives adequate results if the events of the old
    MC covers the phase space allowed by a new model and parameters
    sufficiently dense**
    -   Reweighting of resonance masses is thus in general not possible
    -   Reweighting MC with large anomalous couplings to SM is more
        likely to be successful than reweighting an SM MC to one with
        anomalous couplings
    -   Nominal parameter values should be chosen such that phase space
        of all alternative hypotheses is covered -- in the case of
        anomalous couplings the values set in the `param_card.dat` will
        thus typically not be the SM values

#### Combination with other MadGraph features

-   Multiparticle syntax (e.g. =j = d u s c b=) is possible
-   Forcing a resonances decay using the syntax
    `p p > w+ w- > l+ vl l- vl~` or
    `p p > w+ w-, w+ > l+, w-> vl l- vl~` is possible, too
-   Reweighting can be combined with MadSpin
    -   The two modules are run sequentially without problems
    -   However, in case an alternative hypotheses also affects the
        resonance decay, using MadSpin and reweighting will not give the
        desired result
-   Reweighting can be combined with the calculation of scale and PDF
    systematics
    -   In that case the systematics weights are stored in a list
        together with the weights from the reweighting module in the
    -   The systematics weights and the weights of the reweighting
        module should be multiplied: =weight_new = weight_old \*
        weight_syst/weight_nominal \* weight_mgreweighting=, where
        weight_nominal is the default MadGraph weight
    -   Especially for reweighting far away from the nominal model the
        systematic weights might no longer be valid and the above
        approach breaks down -- this needs to be checked
-   The generation of Gridpacks works as described in the main README

#### Performance

-   Reweighting takes 1 ms per event and parameter point for the process
    `p p > w+ w-` at LO, the more complicated process
    `p p > l+ vl~ l- vl j` takes 3 ms per event, at NLO
    `p p > w+ w-[QCD]` takes 4ms and `p p > t t~ h[QCD]` takes 90 ms --
    all much faster then redoing the full simulation!
-   The time needed for reweighting increases approximately linearly
    with every additional reweighting point
-   The generation of O(1000) and possibly more weights is in general no
    problem
-   Reweighting can be combined with cluster and multicore mode (but
    only in multicore mode the reweighting is parallelized)

#### Known issues

-   Error due to alleged change of mass (only in MG5 2.6.0)
    <https://answers.launchpad.net/mg5amcnlo/+question/657960>
-   Crash due to ambiguity in final state
    <https://answers.launchpad.net/mg5amcnlo/+question/658027>. The
    process `p p > z j j, z > j j` can for example not be reweighted as
    the reweighting module does not know which of the final state jets
    are from the Z boson -- workaround would be using MadSpin for the Z
    decay
-   Crash of some reweighting jobs in multicore mode
    <https://answers.launchpad.net/mg5amcnlo/+question/658701> -- In
    multicore mode reweighting is split into multiple jobs. It can
    happen that some of theses jobs crash. MG will still create an LHE
    files, but some events will not contain the weights, careful!
-   Dependent parameters are not updated in reweighting. For example,
    usually changing G_F in MG will update the W-mass but during
    reweighting only the parameters specified explicitly are changed.
-   It can happen that weights of reweighted events become \>\>10 times
    larger than the nomial weight
    -   This will significantly reduce the statistical accuracy of the
        sample
    -   Usually this is a hint that the alternative and the nominal
        model are too different -- the alternative model lives in a
        phase space that is hardly populated by the nominal
    -   Especially for NLO reweighting it can however happen that there
        are only few outliers with extremely large weights -- after
        checking that this does not affect any relevant distributions,
        removing those outliers might improve the shape of distributions

### Validation

The reweighting is not guaranteed to work and validation is thus
mandatory. Comparing the cross sections obtained from reweighting and
independent MC can give a first Ideally all relevant physics
distribution should be compared for a representative set of parameter
settings.

