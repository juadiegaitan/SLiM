PLEASE NOTE THAT THIS VERSION OF SLiM IS PRERELEASE SOFTWARE THAT IS UNDER ACTIVE DEVELOPMENT!
---------------------------------------------------------------------------------------------
It is strongly recommended that all end users of SLiM use version 1.8, released at [http://messerlab.org/software/](http://messerlab.org/software/).
---------------------------------------------------------------------------------------------
 

SLiM 2.0 (prerelease)
=================

**S**election on **Li**nked **M**utations: a forward population genetic simulation for studying linkage effects, such as hitchhiking, background selection, and Hill-Robertson interference.

SLiM can incorporate complex scenarios of demography and population substructure, various models for selection and dominance of new mutations, realistic gene and chromosome structure, and user-defined recombination maps. Emphasis was further placed on the ability to model and track individual selective sweeps – both complete and partial. While retaining all capabilities of a forward simulation, SLiM utilizes sophisticated algorithms and optimized data structures that enable simulations on the scale of entire eukaryotic chromosomes in reasonably large populations. All these features are implemented in an easy-to-use C++ command line program.

SLiM is a product of the Messer Lab at Cornell University. It was developed by Philipp Messer, and is now maintained and extended by Philipp Messer and Ben Haller.

GitHub home page for SLiM: [https://github.com/MesserLab/SLiM](https://github.com/MesserLab/SLiM)

Messer Lab home page for SLiM: [http://messerlab.org/software/](http://messerlab.org/software/)

License
----------

Copyright (c) 2014 Philipp Messer.  All rights reserved.

This file is part of SLiM.

SLiM is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

SLiM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with SLiM.  If not, see [http://www.gnu.org/licenses/](http://www.gnu.org/licenses/).

Building and Running SLiM
------------------------------------
For Mac OS X users, an Xcode project is provided that can be used to build SLiM. For users on other platforms, or for those who prefer not to use Xcode, SLiM can be built in Terminal with the commands:

```
cd SLiM
g++ -O3 ./core/*.cpp -lgsl -lgslcblas -o slim
```

If your GNU Standard Library headers are not in the default search paths for g++, you will need to supply them on the command line.  You can find out the right command-line arguments to use for this by executing:

```
gsl-config --cflags --libs
```

For example, I have installed gsl using MacPorts, so my compilation command looks like:

```
g++ -O3 ./core/*.cpp -I/opt/local/include -L/opt/local/lib -lgsl -lgslcblas -o slim
```

Once SLiM is built, just run it at Terminal's command line. For example, to run the first example provided in SLiM's distribution, execute:

```
cd SLiM
./slim ./examples/input_example_1.txt
```

If you have made a Release build of SLiM with Xcode, it should be at /usr/local/bin/slim; you can provide that path or ensure that it is part of your shell's default search path.

Development & Feedback
-----------------------------------
SLiM is under active development, and our goal is to make it as broadly useful as possible.  If you have feedback or feature requests, or if you are interested in contributing to SLiM, please contact Philipp Messer at [messer@cornell.edu](mailto:messer@cornell.edu). Please note that Philipp is also looking for graduate students and postdocs.

References
---------------
<u>The original paper on SLiM:</u>

Messer, P. W. (2013). SLiM: Simulating evolution with selection and linkage. *Genetics 194*(4), 1037-1039.

<u>Publications that have used SLiM:</u>

Enard, D., Messer, P. W., & Petrov, D. A. (2014). Genome-wide signals of positive selection in human evolution. *Genome research*.

Kousathanas, A., & Keightley, P. D. (2013). A comparison of models to infer the distribution of fitness effects of new mutations. *Genetics 193*(4), 1197-1208.

Messer, P. W., & Petrov, D. A. (2013). Frequent adaptation and the McDonald–Kreitman test. *Proceedings of the National Academy of Sciences 110*(21), 8615-8620.

Using SLiM in Your Research
---------------------------------------

SLiM is open-source and can be used without restriction in scientific research.  We have only three requests:

  * If you use SLiM to obtain data used in a publication, please ***cite SLiM*** by citing the original paper about SLiM (Messer 2013 *Genetics*).
  * Please also notify Philipp Messer of your publication at [messer@cornell.edu](mailto:messer@cornell.edu), so that he can add it to the list of publications above.
  * If you request a new feature in SLiM and we implement it for you, co-authorship is generally appropriate.

Thanks, and enjoy!