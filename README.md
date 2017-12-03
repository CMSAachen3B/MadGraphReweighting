### Check out the code

```bash
cd $CMSSW_BASE/src
git clone https://github.com/CMSAachen3B/MadGraphReweighting.git CMSAachen3B/MadGraphReweighting
scram b [-j 4]
```

### Running the code

#### Initialise objects of the `MadGraphTools` class.

Parameters are of the [`MadGraphTools` class](https://github.com/CMSAachen3B/MadGraphReweighting/blob/master/interface/MadGraphTools.h#L33-L34) are

1. the mixing angle as float in units of pi/2 (values in the range from 0 to 1)
1. path to the MadGraph process directories
1. path to param_card.dat for MadGraph
1. value for the strong coupling constant alpha_S. Set it to 0.118.
1. boolean to steer the sorting of the particles. Set it to false or keep the default parameter.

The process directories can be found here, where you need to choose between ggH and VBF:
- [`$CMSSW_BASE/src/CMSAachen3B/MadGraphReweighting/data/ggh_2p6/SubProcesses`](https://github.com/CMSAachen3B/MadGraphReweighting/tree/master/data/vbf_2p6/SubProcesses)
- [`$CMSSW_BASE/src/CMSAachen3B/MadGraphReweighting/data/vbf_2p6/SubProcesses`](https://github.com/CMSAachen3B/MadGraphReweighting/tree/master/data/vbf_2p6/SubProcesses)

The param cards are found here, where you currently have four choices
- [`$CMSSW_BASE/src/CMSAachen3B/MadGraphReweighting/data/ggh_2p6/Cards/param_card_default.dat`](https://github.com/CMSAachen3B/MadGraphReweighting/tree/master/data/ggh_2p6/Cards/param_card_default.dat)
- [`$CMSSW_BASE/src/CMSAachen3B/MadGraphReweighting/data/vbf_2p6/Cards/param_card_default.dat`](https://github.com/CMSAachen3B/MadGraphReweighting/tree/master/data/vbf_2p6/Cards/param_card_default.dat)

- [`$CMSSW_BASE/src/CMSAachen3B/MadGraphReweighting/data/ggh_2p6/Cards/param_card_cp.dat`](https://github.com/CMSAachen3B/MadGraphReweighting/tree/master/data/ggh_2p6/Cards/param_card_cp.dat)
- [`$CMSSW_BASE/src/CMSAachen3B/MadGraphReweighting/data/vbf_2p6/Cards/param_card_cp.dat`](https://github.com/CMSAachen3B/MadGraphReweighting/tree/master/data/vbf_2p6/Cards/param_card_cp.dat)

The first two correspond to the settings in the MC samples and are therefore used to calculate the matrix element of the events in the MC samples. The latter two are used to calculate the new matrix elements for any scenario. Note, that the cos(alpha) parameter is replaced by `$cosa` in these files, such that the C++/Python code can vary it later. You can initialise as many MadGraphTools objects as you want. Each becomes a different param card to yield different matrix elements.


#### Calculate squared matrix elements

Loop over the MC events and fill a `std::vector<LHEParticle*>`. The elements should contain the (cartesian) four-momenta and the PDG IDs of the LHE particles. (Generator particles should work as well, but should not contain more than two incoming partons, the Higgs boson and up to three outgoing gluons or quarks.) You find the class definition [here](https://github.com/CMSAachen3B/MadGraphReweighting/blob/master/interface/MadGraphTools.h#L12-L25).

Then call the function [`GetMatrixElementSquared`](https://github.com/CMSAachen3B/MadGraphReweighting/blob/master/interface/MadGraphTools.h#L38) for each object of the `MadGraphTools` class with the vector from above (note that the ordering of elements may be changed in place) and save the return value as the sqared matrix element.

This function calls Python code, which in turn is "compiled" from Fortran code. This solution is the only one that worked so far but it is not very fast. Based on the MadGraph model we use (HiggsCharacterisation) is is not possible to let MadGraph output valid C++ code. If there is a solution to call the Fortran code directly in C++, we would be very interested.


#### Reweight the MC events

Apply as a weight `|M_new|^2 / |M_old|^2`, where `|M_old|^2` corresponds to the squared matrix element calculated for the MC scenario and `|M_new|^2` corresponds to any new scenario the even should be reweighted to. More details are found [here](https://cp3.irmp.ucl.ac.be/projects/madgraph/wiki/Reweight).

