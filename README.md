stereosynth
===========

Stereo video synthesis using PatchMatch

Idea
----
Synthesize a stereo video from a collection of exemplar stereo videos and a new target mono video stream using a Nearest Neighbor Field (NNF) (here [PatchMatch](http://gfx.cs.princeton.edu/gfx/pubs/Barnes_2009_PAR/index.php)).

Method
------
**Basic method**:
Given exemplar stereo streams `(L,R)` and a new mono stream `L'` (assumed to be the left stream arbitrarily), we synthesize the right stream `R'` by finding nearest neighbor patches from `L'` to `L` and then using the corresponding patch of `R` for `R'`.

  * using 2D patches: temporal coherence might be a problem if treating the frames independently, if bootstrapping PatchMatch with the result of the previous (or next) frame, then it gets alleviated
  * using 3D patches: temporal coherence will be fine, but dimensionality will slow down the computation a lot (though it partially takes care of working with multiple exemplars)
  * using multiple exemplars: how do we sample a huge exemplar?

TODO
----

  1. Implement the basic method with 2D patches
  2. Test basic method by synthesizing a stereo stream from the left part of a stereo stream and comparing with the true right part
  3. Experiment with multiple patch sizes and image scales

Unknowns
--------

  * best patch size
  * best image resolution => maybe use a multi-resolution approach (at least for the NNF)
  * how to work with multiple exemplars?
  * whether to use 2D or 3D patches

License
-------
Released under the [MIT license](http://opensource.org/licenses/MIT).
