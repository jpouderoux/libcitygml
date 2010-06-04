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

	inline void addNode( const std::string &node ) { printIndent(); _out << node << " {" << std::endl; _indentCount++; }

	inline void endNode() { _indentCount--; printIndent(); _out << "}" << std::endl; }

	inline void addAttribute( const std::string &attr ) { printIndent(); _out << attr << " "; }

	//template<class T> inline void addAttributeValue( const std::string &attr, T &val ) { printIndent(); _out << attr << " " << val << std::endl; }

	template<class T> inline void addAttributeValue( const std::string &attr, T val ) { printIndent(); _out << attr << " " << val << std::endl; }

	inline void addAttributeValue( const std::string &attr, const char* val ) { printIndent(); _out << attr << " " << val << std::endl; }

	inline void addAttributeNode( const std::string &attr, const std::string &node ) { printIndent(); _out << attr << " " << node << " {" << std::endl; _indentCount++; }

	inline void addAttributeArray( const std::string &attr ) { addAttribute( attr ); _out << " [" << std::endl; _indentCount++; }

	inline void endAttributeArray() { _indentCount--; printIndent(); _out << "]" << std::endl; }

	inline void beginGroup() { addNode("Group"); addAttributeArray( "children" ); }

	inline void endGroup() { endAttributeArray(); endNode(); }

private:
	citygml::CityModel* _cityModel;
	int _indentCount;
	std::ofstream _out;

};

int main( int argc, char **argv )
{
	std::cout << "citygml2vrml v.0.1" << std::endl << "(c) 2010 Joachim Pouderoux, BRGM" << std::endl;

	if ( argc < 3 ) 
	{
		std::cout << "This program converts CityGML files to a VRML97 representation" << std::endl;
		std::cout << "Usage: citygml2vrml <inputfile.gml> <outputfile.wrl>" << std::endl; 
		return -1;
	}


	std::cout << "Parsing CityGML file " << argv[1] << "..." << std::endl;

	time_t start;
	time( &start );

	citygml::CityModel *city = citygml::load( argv[1], citygml::COT_All );

	time_t end;
	time( &end );

	if ( !city ) return NULL;

	std::cout << "Done in " << difftime( end, start ) << " seconds." << std::endl << city->size() << " city objects read." << std::endl;


	std::cout << "Converting the city objects to VRML97..." << std::endl;

	VRML97Converter converter( city );

	if ( converter.convert( argv[2] ) )
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

	const citygml::CityObjectsMap& cityObjectsMap = _cityModel->getCityObjectsMap();

	citygml::CityObjectsMap::const_iterator it = cityObjectsMap.begin();

	for ( ; it != cityObjectsMap.end(); it++ )
	{
		const citygml::CityObjects& v = it->second;

		std::cout << " Creation of " << v.size() << " " << citygml::getCityObjectsClassName( it->first ) << ( ( v.size() > 1 ) ? "s" : "" ) << "..." << std::endl;

		addComment( citygml::getCityObjectsClassName( it->first ) );
		beginGroup();

		for ( unsigned int i = 0; i < v.size(); i++ ) dumpCityObject( v[i] );

		endGroup();
		break;
	}

	_out.close();

	return true;
}

void VRML97Converter::dumpCityObject( const citygml::CityObject* object ) 
{
	if ( !object || object->size() == 0 ) return;

	addComment( object->getId() );

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
	if ( !p || !p->getIndices() || p->getIndicesSize() == 0 ) return;

	unsigned int* indices = p->getIndices();
	std::stringstream ss;
	ss << "  " << p->size() << " points & " << p->getIndicesSize() << " triangles";
	addComment( "Polygon: " + p->getId() + ss.str() );

	addNode( "Shape" );

	// Geometry management

	addAttributeNode( "geometry", "IndexedFaceSet" );

	{
		addAttributeNode( "coord", "Coordinate" );

		addAttributeArray( "point" );
		printIndent();
		for ( unsigned int k = 0; k < p->size(); k++ ) _out << (*p)[k] << ", ";
		_out << std::endl;
		endAttributeArray();

		endNode();
	}

	if ( p->getIndices() )
	{
		addAttributeArray( "coordIndex" );
		printIndent();
		for ( unsigned int k = 0 ; k < p->getIndicesSize() / 3; k++ )
			_out << indices[ k * 3 + 0 ] << " " << indices[ k * 3 + 1 ] << " " << indices[ k * 3 + 2 ] << " -1, ";
		_out << std::endl;
		endAttributeArray();
	}

	// Normal management

	if ( p->getNormal() )
	{
		addAttributeNode( "normal", "Normal" );

		addAttributeArray( "vector" );
		printIndent();
		_out << p->getNormal() << std::endl;
		endAttributeArray();

		endNode();
	}

	addAttributeValue( "solid", "FALSE" );
	addAttributeValue( "normalPerVertex", "FALSE" );

	// Texture coordinates

	const citygml::TexCoords *texCoords = p->getTexCoords();

	if ( texCoords )
	{
		addAttributeNode( "texCoord", "TextureCoordinate" );

		addAttributeArray( "point" );
		printIndent();
		for ( unsigned int k = 0; k < p->size(); k++ ) _out << (*texCoords)[k] << ", ";
		_out << std::endl;
		endAttributeArray();

		endNode();
	}

	endNode();

	// Material management

	const citygml::Appearance *mat = p->getAppearance();

	bool colorset = false;

	//if ( mat )
	{
		addAttributeNode( "appearance", "Appearance" );

		bool colorset = false;

		if ( const citygml::Material* m = dynamic_cast<const citygml::Material*>( mat ) )
		{
			addAttributeNode( "material", "Material" );

			addAttributeValue( "diffuseColor", m->getDiffuse() );
			addAttributeValue( "ambientIntensity", m->getAmbientIntensity() );
			addAttributeValue( "specularColor", m->getSpecular() );
			addAttributeValue( "emissiveColor", m->getEmissive() );
			addAttributeValue( "shininess", m->getShininess() );
			addAttributeValue( "transparency", m->getTransparency() );

			endNode();
			colorset = true;
		}
		else if ( const citygml::Texture* t = dynamic_cast<const citygml::Texture*>( mat ) ) 
		{

			addAttributeNode( "texture", "ImageTexture" );
			addAttributeValue( "url", t->getUrl() );
			endNode();
			//colorset = true;
		}
		if ( !colorset )
		{
			addAttributeNode( "material", "Material" );

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
