libossia
===

libossia is a modern C++, cross-environment distributed object model for creative coding.

It allows to declare the architecture of your creative coding application's functions as a **tree of OSC nodes and parameters**. These nodes/parameters can have attributes, which allow to declare many of their properties, such as their values, types, units, ranges, etc....

This OSC tree-based architecture (coined "device" in the OSSIA terminology) can then be exposed over the network under several protocols, some of which allow this architecture, and the properties and values of its nodes, to be fully explored and queried. For now, protocols available in the implemenations are: plain OSC, OSCquery, and Minuit - more are part of libossia and will be made available in the future.

libossia offers bindings and implementations for several environments: PureData, Max/MSP, Python, C, C++, openframeworks, Unity3D, QML, Faust, SuperCollider.

You can find some documentation for the library and its various implementations is at https://ossia.github.io

Please feel free to ask questions on the [forum](http://forum.ossia.io/c/libossia), or to come and chat with us ! [![Gitter](https://badges.gitter.im/ossia/libossia.svg)](https://gitter.im/ossia/libossia?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)

More about the general project at: http://ossia.io

Download
========
* Latest release : https://github.com/ossia/libossia/releases/latest
* Latest CI-build (might be buggy): https://github.com/ossia/libossia/releases/tag/latest-ci-build

Documentation
=============
* [Lib and Implementations](https://ossia.github.io/ossia-docs)
* [Examples](https://github.com/ossia/libossia/tree/master/examples)
* [Doxygen](http://ossia.github.io/libossia/html)
* [Building](https://github.com/ossia/libossia/wiki/Building)
* [Code style](https://github.com/ossia/libossia/wiki/Code-style-guide)

Build Status
============
* [![Build Status](https://github.com/ossia/libossia/actions/workflows/benchmarks.yml/badge.svg)](https://github.com/ossia/libossia/actions/workflows/benchmarks.yml)

* <img src="https://upload.wikimedia.org/wikipedia/commons/thumb/9/93/Logo_Max_8_software.jpg/244px-Logo_Max_8_software.jpg" alt="Max" height="20"/> [![Build Status](https://github.com/ossia/libossia/actions/workflows/ossia-max.yml/badge.svg)](https://github.com/ossia/libossia/actions/workflows/ossia-max.yml)

* <img src="https://patchstorage.com/wp-content/themes/bayside/images/logo-pd-vanilla.png" alt="Pd" height="20"/> [![Build Status](https://github.com/ossia/libossia/actions/workflows/ossia-pd.yml/badge.svg)](https://github.com/ossia/libossia/actions/workflows/ossia-pd.yml)

* <img src="https://unity3d.com/profiles/unity3d/themes/unity/images/pages/branding_trademarks/unity-tab-square-black.png" alt="Unity3D" height="20"/> [![Build Status](https://github.com/ossia/libossia/actions/workflows/ossia-unity.yml/badge.svg)](https://github.com/ossia/libossia/actions/workflows/ossia-unity.yml)

* [![Build Status](https://github.com/ossia/libossia/actions/workflows/doc.yml/badge.svg)](https://github.com/ossia/libossia/actions/workflows/doc.yml)

* [![Build Status](https://github.com/ossia/libossia/actions/workflows/coverage.yml/badge.svg)](https://github.com/ossia/libossia/actions/workflows/coverage.yml)

* Coveralls : [![Coverage Status](https://coveralls.io/repos/github/ossia/libossia/badge.svg?branch=master)](https://coveralls.io/github/ossia/libossia?branch=master)

Latest ci builds are available [here](https://github.com/ossia/libossia/releases/tag/latest-ci-build). But please note that those are only for testing purpose.

License
=======
Available under both LGPLv3 and CeCILL-C

[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bhttps%3A%2F%2Fgithub.com%2FOSSIA%2Flibossia.svg?type=shield)](https://app.fossa.io/projects/git%2Bhttps%3A%2F%2Fgithub.com%2FOSSIA%2Flibossia?ref=badge_shield)
