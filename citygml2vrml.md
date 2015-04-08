**citygml2vrml** is a command line utility to convert CityGML files to a VRML97 representation.

Last version: **citygml2vrml v.0.1.3**

## Usage ##
```
 citygml2vrml [-options ...] <input.gml> [output.wrl]
```

## Options ##
```
  -optimize       Merge geometries & polygons with similar properties to
                  reduce file & scene size (recommended)

  -comments       Add comments about the object ids to the VRML file

  -center         Center the model around the first encountered point
                  (may be used to reduce z-fighting artifacts)

  -filter <mask>  CityGML objects to parse (default:All)
                  The mask is composed of:
                   GenericCityObject, Building, Room,
                   BuildingInstallation, BuildingFurniture, Door, Window,
                   CityFurniture, Track, Road, Railway, Square, PlantCover,
                   SolitaryVegetationObject, WaterBody, TINRelief, LandUse,
                   Tunnel, Bridge, BridgeConstructionElement,
                   BridgeInstallation, BridgePart,  All
                  and seperators |,&,~.
                  Examples:
                  "All&~Track&~Room" to parse everything but tracks & rooms
                  "Road&Railway" to parse only roads & railways

  -minLOD <level> Minimum LOD level to parse (default:0)

  -maxLOD <level> Maximum LOD level to parse (default:4)
```

## Download ##

The source code of last version of **citygml2vrml** is available on the SVN repository: [citygml2vrml.cpp](http://code.google.com/p/libcitygml/source/browse/trunk/test/citygml2vrml/citygml2vrml.cpp).

You can download a binary distribution for Windows [here](http://code.google.com/p/libcitygml/downloads/list).