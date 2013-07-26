Riddim: A Rhythm Analysis and Decomposition Tool Based on Independent Subspace Analysis
------------

This repo contains all the source code (MATLAB m-files, MEX and C/C++ files) for Riddim, a Dartmouth College master's thesis.

This work has been [cited](http://scholar.google.com/scholar?oi=bibs&hl=en&cites=9249905419987405037) 20 times since its original publication in 2001. The author is grateful for the support of [Michael Casey](http://eamusic.dartmouth.edu/~mcasey/index.html), J.-F. Cardoso and [Anssi Klapuri](http://www.cs.tut.fi/~klap/) whose research and support was essential to this work. The source code and supporting materials have commited here, uniquely for archival and pedagodgical reasons. 


#### Citations
![citations](https://raw.github.com/ruohoruotsi/Riddim/master/docs/riddim_citations.png)


Abstract
------------
The goal of this thesis was to implement a tool that, given a digital audio input, can extract and represent rhythm and musical time. The purpose of the tool is to help develop better models of rhythm for real-time computer based performance and composition. This analysis tool, Riddim, uses Independent Subspace Analysis (ISA) and a robust onset detection scheme to separate and detect salient rhythmic and timing information from different sonic sources within the input. This information is then represented in a format that can be used by a variety of algorithms that interpret timing information to infer rhythmic and musical structure. 


Installation
------------
To use Riddim:

1. Download the entire project as a ZIP file
2. Launch MATLAB, navigate to the top level folder
3. Add the top level folder to your MATLAB path (recursively) 
4. Run `riddim_tool` from the MATLAB command prompt


Compatibility
------------
This 2001 work was written in and extensively tested with MATLAB version 6.0.0.88 (R12), which as of this README commit in 2013 is very old. There has not been any additional testing in the latest version of MATLAB. Your results may vary.


License
-------
Riddim - Copyright © 2001 iroro orife.

Source code is provided under GPL v3