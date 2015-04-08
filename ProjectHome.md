Last version: **libcitygml v.0.1.4**  &  **[citygml2vrml](http://code.google.com/p/libcitygml/wiki/citygml2vrml) v.0.1.3**

Want to join the libcitygml developer team? [Contact us!](http://code.google.com/u/jpouderoux/)

## News ##

_07.22.11_ - Some fixes were published on the SVN! libcitygml v.0.1.5 is coming soon.

_08.11.10_ - libcitygml v.0.1.4 (check the SVN!) adds support for SRS & coordinates transformations.

_07.30.10_ - New in libcitygml v.0.1.3: Support for [CityGML Bridge ADE](http://www.citygmlwiki.org/index.php/CityGML_Bridge_ADE) and [CityGML Tunnel ADE](http://www.citygmlwiki.org/index.php/CityGML_Tunnel_ADE)(aka Subsurface Structure ADE).

_06.14.10_ - Add support for LibXml2 instead of Xerces-c (both are now supported, easy to add support for an other SAX parsing library). Parsing parameters are now passed through a structure.

## About ##

[CityGML](http://www.citygml.org) (_City Geography Markup Language_) is an XML-based schema for the modelling and exchange of georeferenced 3D city and landscape models that is quickly being adopted on an international level.

**libcitygml** is a small and easy to use open source C++ library for parsing CityGML files in such a way that data can be easily exploited by 3D rendering applications (geometry data are tesselated and optimized for rendering during parsing). For instance, it can be used to develop readers of CityGML files in many 3D based applications (OpenGL, [OpenSceneGraph](http://www.openscenegraph.org), ...) Most metadata are not lost, they are available through a per-node hashmap.

**libcitygml** is developed by the 3D team of [BRGM](http://www.brgm.fr) (the French leading public institution involved in the Earth Science field
for the sustainable management of natural resources and surface and subsurface risks) for the research project <a href='http://www.deepcity3d.org'>DeepCity3D</a>.

The following screenshots were done using [the CityGML reader implemented for OpenSceneGraph](http://code.google.com/p/libcitygml/source/browse/trunk/test/osgplugin/ReaderWriterCityGML.cpp). The models used are freely available on [the official CityGML site](http://www.citygml.org/index.php?id=1539).

![![](http://libcitygml.googlecode.com/svn/tags/web/images/t_frank.jpg)](http://libcitygml.googlecode.com/svn/tags/web/images/frank.png)
![![](http://libcitygml.googlecode.com/svn/tags/web/images/t_munich.jpg)](http://libcitygml.googlecode.com/svn/tags/web/images/munich.png)
![![](http://libcitygml.googlecode.com/svn/tags/web/images/t_city4.jpg)](http://libcitygml.googlecode.com/svn/tags/web/images/city4.png)
![![](http://libcitygml.googlecode.com/svn/tags/web/images/t_city3.jpg)](http://libcitygml.googlecode.com/svn/tags/web/images/city3.png)
![![](http://libcitygml.googlecode.com/svn/tags/web/images/t_building1.jpg)](http://libcitygml.googlecode.com/svn/tags/web/images/building1.png)

The following screenshots are the conversion to VRML97 of the examples of CityGML <a href='http://www.citygmlwiki.org/index.php/CityGML_Tunnel_ADE'>Tunnel</a> and <a href='http://www.citygmlwiki.org/index.php/CityGML_Bridge_ADE'>Bridge</a> ADEs:

![![](http://libcitygml.googlecode.com/svn/tags/web/images/t_tunnel.jpg)](http://libcitygml.googlecode.com/svn/tags/web/images/tunnel.png)
![![](http://libcitygml.googlecode.com/svn/tags/web/images/t_bridge.jpg)](http://libcitygml.googlecode.com/svn/tags/web/images/bridge.png)

## Supported features ##

**libcitygml** is not a strict implementation of the CityGML standard and is not based on a particular schema. However is has been tested successfully with CityGML version 0.3 to 1.0 files, all LOD (from 0 to 4) and supports most of the model features including:

  * Buildings and building furnitures
  * Digital Terrain Models as triangulated irregular networks (TINs)
  * Vegetation (areas, volumes and solitary objects)
  * Water bodies (volumes, surfaces)
  * Transportation facilities (both graph structures and 3D surface data) like roads or railways
  * City furnitures
  * Tunnels (subsurface structures) & Bridges

Materials & textures are also well supported.

## Not-yet-supported features ##

  * CityGML write support
  * Some extensions like Noise ADE (Application Domain Extension)
  * Orthoreferenced textures with no texture coordinates
  * Digital Terrain Models as regular rasters, break and skeleton lines and mass points
  * _To be continued..._

## Download ##

Sources & precompiled versions of the library are available in the [Downloads tab](http://code.google.com/p/libcitygml/downloads/list).

**libcitygml**' latest sources are [available through the googlecode SVN server](http://code.google.com/p/libcitygml/source/checkout).

**[citygml2vrml](http://code.google.com/p/libcitygml/wiki/citygml2vrml)** is a simple program exploiting libcitygml which converts a CityGML file to a VRML97 representation. The source code is available on the SVN. You can also [download citygml2vrml binary for Windows](http://code.google.com/p/libcitygml/downloads/list).

### Dependencies ###

  * [CMake](http://www.cmake.org) is used for the build system. Use it to generate a Visual Studio 20xy solution, or a set of makefiles.

  * [Xerces-C++ XML Parser 3.x.y](http://xerces.apache.org/xerces-c/download.cgi) _OR_ [libxml2](http://www.xmlsoft.org/) SAX implementations are used for the XML parsing itself.

  * [GLU](http://en.wikipedia.org/wiki/OpenGL_Utility_Library) (_OpenGL Utility Library_) is used for polygon tesselation. GLU comes with your OpenGL implementation.

  * [GDAL/OGR](http://www.gdal.org) (optional) is used to manage SRS & coordinates transformation.

### Datasets ###

You can find some CityGML datasets on the [official CityGML site](http://www.citygml.org/index.php?id=1539).

## Usage ##

During SAX-parsing the file or the stream, **libcitygml** constructs a memory model of the scene (this model is simpler than DOM or XML data binding). The model is then prepared and optimized to be used by 3D applications. Finally, the object hiearchy is available through the main _citygml::CityModel_ object return by the parsing method [citygml::load()](http://code.google.com/p/libcitygml/source/browse/trunk/include/citygml.h).
The file _[citygml.h](http://code.google.com/p/libcitygml/source/browse/trunk/include/citygml.h)_ of **libcitygml** describes the city objects supported by the library.

Please see the header files and the 2 examples files (_[citygmltest.cpp](http://code.google.com/p/libcitygml/source/browse/trunk/test/citygmltest.cpp)_, console dump, and _[ReaderWriterCityGML.cpp](http://code.google.com/p/libcitygml/source/browse/trunk/test/osgplugin/ReaderWriterCityGML.cpp)_, the OpenSceneGraph reader) for more details on how to use the library.

**libcitygml** is rather fast. For instance, the 40MB sample file <i>Stadt-Ettenheim-LoD3_edited.gml</i> is parsed, optimized and tesselated in 12 seconds on an intel Core2 Quad CPU Q8400 @ 2GHz.


---


<a href='http://www.citygml.org'><img src='http://www.deepcity3d.eu/partners/PublishingImages/CityGML.png' alt='CityGML' border='0' height='60px' /></a> <a href='http://www.opengeospatial.org'><img src='http://www.deepcity3d.eu/partners/PublishingImages/OGC.png' alt='OGC' border='0' height='40px' /></a>

<a href='http://www.brgm.fr'><img src='http://www.deepcity3d.eu/partners/PublishingImages/logobrgm.gif' alt='BRGM' border='0' height='60px' /></a>