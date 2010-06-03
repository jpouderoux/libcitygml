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

std::ofstream out;

void createCityModel( citygml::CityModel* city );
void createCityObject( citygml::CityObject* object );

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

	std::cout << "Done in " << difftime( end, start ) << " seconds." << std::endl; 

	std::cout << city->size() << " city objects read." << std::endl;

	std::cout << "Converting the city objects to VRML97..." << std::endl;

	out.open( argv[2] );
	if ( out.fail() ) { std::cerr << "Unable to create file " << argv[2] << "!" << std::endl; return -1; }

	out << "#VRML V2.0 utf8" << std::endl;

	createCityModel( city );

	out.close();

	std::cout << "Done." << std::endl;

	return 0;
}

// Helpers

static int indentCount = 0;

inline void printIndent() { for ( int i = 0; i < indentCount; i++ ) out << "\t"; }

inline void addComment(const std::string& cmt) {  printIndent(); out << "# " << cmt << std::endl; }

inline void addNode( const std::string &node ) { printIndent(); out << node << " {" << std::endl; indentCount++; }

inline void endNode() { indentCount--; printIndent(); out << "}" << std::endl; }

inline void addAttribute( const std::string &attr ) { printIndent(); out << attr << " "; }

template<class T> inline void addAttributeValue( const std::string &attr, T &val ) { printIndent(); out << attr << " " << val << std::endl; }

template<class T> inline void addAttributeValue( const std::string &attr, T val ) { printIndent(); out << attr << " " << val << std::endl; }

inline void addAttributeValue( const std::string &attr, const char* val ) { printIndent(); out << attr << " " << val << std::endl; }

inline void addAttributeNode( const std::string &attr, const std::string &node ) { printIndent(); out << attr << " " << node << " {" << std::endl; indentCount++; }

inline void addAttributeArray( const std::string &attr ) { addAttribute( attr ); out << " [" << std::endl; indentCount++; }

inline void endAttributeArray() { indentCount--; printIndent(); out << "]" << std::endl; }

inline void beginGroup() { addNode("Group"); addAttributeArray( "children" ); }

inline void endGroup() { endAttributeArray(); endNode(); }

// VRML97 city converter

void createCityModel( citygml::CityModel* city )
{
	const citygml::CityObjectsMap& cityObjectsMap = city->getCityObjectsMap();

	citygml::CityObjectsMap::const_iterator it = cityObjectsMap.begin();

	for ( ; it != cityObjectsMap.end(); it++ )
	{
		const citygml::CityObjects& v = it->second;

		std::cout << " Creation of " << v.size() << " " << citygml::getCityObjectsClassName( it->first ) << ( ( v.size() > 1 ) ? "s" : "" ) << "..." << std::endl;
		
		addComment( citygml::getCityObjectsClassName( it->first ) );
		beginGroup();
		
		for ( unsigned int i = 0; i < v.size(); i++ ) createCityObject( v[i] );
	
		endGroup();
		break;
	}
}

// VRML97 object converter

void createCityObject( citygml::CityObject* object ) 
{
	if ( object->size() == 0 ) return;

	beginGroup();
	
	addComment( object->getId() );
	
	//std::cout << "Creating object " << object->getId() << std::endl;
	
	for ( unsigned int i = 0; i < object->size(); i++ ) 
	{
		const citygml::Geometry& geometry = *object->getGeometry( i );

		for ( unsigned int j = 0; j < geometry.size(); j++ ) 
		{
			const citygml::Polygon* p = geometry[j];
			if ( !p || !p->getIndices() || p->getIndicesSize() == 0 ) continue;
			
			addNode( "Shape" );

			// Geometry management

			addAttributeNode( "geometry", "IndexedFaceSet" );

			{
				addAttributeNode( "coord", "Coordinate" );

				addAttributeArray( "point" );
				printIndent();
				for ( unsigned int k = 0; k < p->size(); k++ ) out << (*p)[k] << ", ";
				out << std::endl;
				endAttributeArray();

				endNode();
			}

			if ( p->getIndices() )
			{
				addAttributeArray( "coordIndex" );
				printIndent();
				unsigned int* indices = p->getIndices();
				for ( unsigned int k = 0 ; k < p->getIndicesSize() / 3; k++ )
					out << indices[ k * 3 + 0 ] << " " << indices[ k * 3 + 1 ] << " " << indices[ k * 3 + 2 ] << ", ";
				out << std::endl;
				endAttributeArray();
			}

			// Normal management

			if ( p->getNormal() )
			{
				addAttributeNode( "normal", "Normal" );

				addAttributeArray( "vector" );
				printIndent();
				out << p->getNormal() << std::endl;
				endAttributeArray();

				endNode();
			}

			addAttributeValue( "normalPerVertex", "FALSE" );

			// Texture coordinates

			const citygml::TexCoords *texCoords = p->getTexCoords();

			if ( texCoords )
			{
				addAttributeNode( "texCoord", "TextureCoordinate" );

				addAttributeArray( "point" );
				printIndent();
				for ( unsigned int k = 0; k < p->size(); k++ ) out << (*texCoords)[k] << ", ";
				out << std::endl;
				endAttributeArray();

				endNode();
			}

			endNode();
			
			// Material management

			const citygml::Appearance *mat = p->getAppearance();

			bool colorset = false;
 
			if ( mat )
			{
				addAttributeNode( "appearance", "Appearance" );

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
				}
				else if ( const citygml::Texture* t = dynamic_cast<const citygml::Texture*>( mat ) ) 
				{
					
					addAttributeNode( "texture", "ImageTexture" );
					addAttributeValue( "url", t->getUrl() );
					endNode();
					
				}
				endNode();
			}

			// That's it!		
			endNode();

			//break;
		}
		//break;
	}
	
	endGroup();
}
