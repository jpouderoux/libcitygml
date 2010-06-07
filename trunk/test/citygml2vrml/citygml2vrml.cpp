/* -*-c++-*- citygml2vrml - Copyright (c) 2010 Joachim Pouderoux, BRGM
*
* This file is part of libcitygml library
* http://code.google.com/p/libcitygml
*
* libcitygml is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 2.1 of the License, or
* (at your option) any later version.
*
* libcitygml is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*/

#include <iostream>
#include <fstream>
#include <time.h> 
#include "citygml.h"


class VRML97Converter 
{
public:

	VRML97Converter( citygml::CityModel* city ) : _cityModel( city ), _indentCount( 0 ) {}

	bool convert( const std::string& outFilename );

private:

	void dumpCityObject( const citygml::CityObject* );

	void dumpGeometry( const citygml::CityObject*, const citygml::Geometry* );

	void dumpPolygon( const citygml::CityObject*, const citygml::Geometry*, const citygml::Polygon* );

	// VRML97 Helpers

	inline void printIndent() { for ( int i = 0; i < _indentCount; i++ ) _out << "\t"; }

	inline void addComment(const std::string& cmt) {  printIndent(); _out << "# " << cmt << std::endl; }

	inline void beginNode( const std::string &node ) { printIndent(); _out << node << " {" << std::endl; _indentCount++; }

	inline void endNode() { _indentCount--; printIndent(); _out << "}" << std::endl; }

	inline void addAttribute( const std::string &attr ) { printIndent(); _out << attr << " "; }

	//template<class T> inline void addAttributeValue( const std::string &attr, T &val ) { printIndent(); _out << attr << " " << val << std::endl; }

	template<class T> inline void addAttributeValue( const std::string &attr, T val ) { printIndent(); _out << attr << " " << val << std::endl; }

	inline void addAttributeValue( const std::string &attr, const char* val ) { printIndent(); _out << attr << " " << val << std::endl; }

	inline void beginAttributeNode( const std::string &attr, const std::string &node ) { printIndent(); _out << attr << " " << node << " {" << std::endl; _indentCount++; }

	inline void beginAttributeArray( const std::string &attr ) { addAttribute( attr ); _out << " [" << std::endl; _indentCount++; }

	inline void endAttributeArray() { _indentCount--; printIndent(); _out << "]" << std::endl; }

	inline void beginGroup() { beginNode("Group"); beginAttributeArray( "children" ); }

	inline void endGroup() { endAttributeArray(); endNode(); }

private:
	citygml::CityModel* _cityModel;
	int _indentCount;
	std::ofstream _out;

};

int main( int argc, char **argv )
{
	std::cout << "citygml2vrml v.0.1 (c) 2010 Joachim Pouderoux, BRGM" << std::endl;

	if ( argc < 3 ) 
	{
		std::cout << std::endl << "This program converts CityGML files to a VRML97 representation" << std::endl;
		std::cout << "More info & updates on http://code.google.com/p/libcitygml" << std::endl;
		std::cout << "Version built on " << __DATE__ << " at " << __TIME__ << std::endl << std::endl;
		std::cout << "\tUsage: citygml2vrml [-optimize] <inputfile.gml> <outputfile.wrl>" << std::endl; 
		std::cout << "\tOptions:" << std::endl;
		std::cout << "\t -optimize   Merge geometries & polygons with similar properties to reduce file & scene size" << std::endl;
		return -1;
	}

	int fargc = 1;

	bool optimize = false;

	if ( std::string( argv[1] ) == "-optimize" ) { optimize = true; fargc++; }

	std::cout << "Parsing CityGML file " << argv[fargc] << "..." << std::endl;

	time_t start;
	time( &start );

	citygml::CityModel *city = citygml::load( argv[fargc], citygml::COT_All, 0, 4, optimize );

	time_t end;
	time( &end );

	if ( !city ) return NULL;

	std::cout << "Done in " << difftime( end, start ) << " seconds." << std::endl << city->size() << " city objects read." << std::endl;


	std::cout << "Converting the city objects to VRML97..." << std::endl;

	VRML97Converter converter( city );

	if ( converter.convert( argv[fargc+1] ) )
		std::cout << "Done." << std::endl;
	else 
		std::cout << "Failed!" << std::endl;

	return 0;
}


// VRML97 city converter

bool VRML97Converter::convert( const std::string& outFilename )
{
	if ( !_cityModel ) return false;

	_out.open( outFilename.c_str() );

	if ( _out.fail() ) { std::cerr << "Unable to create file " << outFilename << "!" << std::endl; return false; }

	_out << "#VRML V2.0 utf8" << std::endl;
	_out << "# Converted from a CityGML model using citygml2vrml (http://code.google.com/p/libcitygml)" << std::endl << std::endl;

	const citygml::CityObjectsMap& cityObjectsMap = _cityModel->getCityObjectsMap();

	citygml::CityObjectsMap::const_iterator it = cityObjectsMap.begin();

	for ( ; it != cityObjectsMap.end(); it++ )
	{
		const citygml::CityObjects& v = it->second;

		std::cout << " Creation of " << v.size() << " " << citygml::getCityObjectsClassName( it->first ) << ( ( v.size() > 1 ) ? "s" : "" ) << "..." << std::endl;

		addComment( "Object type: " + citygml::getCityObjectsClassName( it->first ) + ( ( v.size() > 1 ) ? "s" : "" ) );
		beginGroup();

		for ( unsigned int i = 0; i < v.size(); i++ ) dumpCityObject( v[i] );

		endGroup();
	}

	_out.close();

	return true;
}

void VRML97Converter::dumpCityObject( const citygml::CityObject* object ) 
{
	if ( !object || object->size() == 0 ) return;

	addComment(  object->getTypeAsString() + ": " + object->getId() );

	beginGroup();	

	for ( unsigned int i = 0; i < object->size(); i++ ) dumpGeometry( object, object->getGeometry( i ) );

	endGroup();
}

void VRML97Converter::dumpGeometry( const citygml::CityObject* object, const citygml::Geometry* g )
{
	if ( !g ) return;

	addComment( "Geometry: " + g->getId() );

	for ( unsigned int i = 0; i < g->size(); i++ ) dumpPolygon( object, g, (*g)[i] );
}

void VRML97Converter::dumpPolygon( const citygml::CityObject* object, const citygml::Geometry* g, const citygml::Polygon* p )
{
	if ( !p || p->getIndices().size() == 0 ) return;

	std::stringstream ss;
	ss << "  " << p->size() << " points & " << p->getIndices().size() << " triangles & " << p->getNormals().size() << " normals & " << p->getTexCoords().size() << " texCoords";
	addComment( "Polygon: " + p->getId() + ss.str() );

	beginNode( "Shape" );

	// Geometry management

	beginAttributeNode( "geometry", "IndexedFaceSet" );

	{
		beginAttributeNode( "coord", "Coordinate" );

		beginAttributeArray( "point" );
		printIndent();
		for ( unsigned int k = 0; k < p->size(); k++ ) _out << (*p)[k] << ", ";
		_out << std::endl;
		endAttributeArray();

		endNode();
	}

	if ( p->getIndices().size() > 0 )
	{
		const std::vector<unsigned int>& indices = p->getIndices();
		beginAttributeArray( "coordIndex" );
		printIndent();
		for ( unsigned int k = 0 ; k < indices.size() / 3; k++ )
			_out << indices[ k * 3 + 0 ] << " " << indices[ k * 3 + 1 ] << " " << indices[ k * 3 + 2 ] << " -1, ";
		_out << std::endl;
		endAttributeArray();
	}

	// Normal management

	if ( p->getNormals().size() > 0 )
	{
		const std::vector<TVec3f>& normals = p->getNormals();
		beginAttributeNode( "normal", "Normal" );
		beginAttributeArray( "vector" );
		printIndent();
		for ( unsigned int k = 0 ; k < normals.size(); k++ )
			_out << normals[k] << ", ";
		_out << std::endl;
		endAttributeArray();
		endNode();
		addAttributeValue( "normalPerVertex", "TRUE" );
	}

	// Texture coordinates

	if ( p->getTexCoords().size() > 0 )
	{
		const citygml::TexCoords& texCoords = p->getTexCoords();
		beginAttributeNode( "texCoord", "TextureCoordinate" );

		beginAttributeArray( "point" );
		printIndent();
		for ( unsigned int k = 0; k < texCoords.size(); k++ ) _out << texCoords[k] << ", ";
		_out << std::endl;
		endAttributeArray();

		endNode();
	}

	endNode();

	// Material management
	{
		beginAttributeNode( "appearance", "Appearance" );

		bool colorset = false;

		const citygml::Appearance *mat = p->getAppearance();

		if ( const citygml::Material* m = dynamic_cast<const citygml::Material*>( mat ) )
		{
			beginAttributeNode( "material", "Material" );

			addAttributeValue( "diffuseColor", m->getDiffuse() );
			addAttributeValue( "ambientIntensity", m->getAmbientIntensity() );
			addAttributeValue( "specularColor", m->getSpecular() );
			addAttributeValue( "emissiveColor", m->getEmissive() );
			addAttributeValue( "shininess", m->getShininess() );
			addAttributeValue( "transparency", m->getTransparency() );

			endNode();
			colorset = true;
		}
		
		if ( const citygml::Texture* t = dynamic_cast<const citygml::Texture*>( mat ) ) 
		{
			beginAttributeNode( "texture", "ImageTexture" );
			addAttributeValue( "url", "\"" + t->getUrl() + "\"" );
			endNode();
			colorset = true;
		}

		if ( !colorset )
		{
			beginAttributeNode( "material", "Material" );

			TVec3f color( object->getDefaultColor().rgba );
			if ( g->getType() == citygml::GT_Roof )
				color = TVec3f( 0.9f, 0.1f, 0.1f );
			addAttributeValue( "diffuseColor", (TVec3f&)color );

			endNode();
		}

		endNode();
	}

	// That's it!		
	endNode();
}
