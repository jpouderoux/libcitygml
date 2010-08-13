/* -*-c++-*- libcitygml - Copyright (c) 2010 Joachim Pouderoux, BRGM
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

// libcitygml implements a SAX Parser for CityGML v0.3 - v1.0 file format
// See schemas at:
//  http://www.citygml.org/citygml/1/0/0/CityGML.xsd
//  http://www.citygml.org/fileadmin/citygml/docs/CityGML_1_0_0_UML_diagrams.pdf

#include "citygml.h"

#include <string>
#include <algorithm>
#include <stack>
#include <fstream>

namespace citygml {
	
	typedef enum CityGMLNodeType;
	
	// CityGML SAX parsing handler
	class CityGMLHandler
	{
	public:

		CityGMLHandler( const ParserParams& params );

		~CityGMLHandler( void );

		virtual void startDocument( void ) {}

		virtual void endDocument( void ) {}

		virtual void startElement( const std::string&, void* );

		virtual void endElement( const std::string& );

		virtual void fatalError( const std::string& error ) 
		{
			std::cerr << "Fatal error while parsing CityGML file: " << error << std::endl;
			std::cerr << "  Full path was: " << getFullPath() << std::endl;
		}

		inline CityModel* getModel( void ) { return _model; }

	protected:

		std::string getNodeName( const std::string& );

		inline int searchInNodePath( const std::string& name ) const 
		{
			for ( int i = _nodePath.size() - 1; i >= 0; i-- )
				if ( _nodePath[i] == name ) return i;
			return -1;
		}

		inline std::string getFullPath( void ) const 
		{
			std::stringstream ss;
			for ( unsigned int i = 0; i < _nodePath.size(); i++ )
				ss << _nodePath[i] << "/";
			return ss.str();
		}

		inline std::string getPrevNode( void ) const 
		{
			return _nodePath.size() > 2 ? _nodePath[ _nodePath.size() - 2 ] : "";
		}

		inline unsigned int getPathDepth( void ) const 
		{
			return _nodePath.size();
		}

		inline CityGMLNodeType getPrevNodeType( void ) const 
		{
			return getNodeTypeFromName( getPrevNode() );
		}

		inline void clearBuffer() { _buff.str(""); _buff.clear(); }  

		static void cityGMLInit( void );

		static CityGMLNodeType getNodeTypeFromName( const std::string& );

		inline void pushCityObject( CityObject* object ) {

			if ( _currentCityObject && object ) _currentCityObject->getChildren().push_back( object );
			_cityObjectStack.push( _currentCityObject );
			_currentCityObject = object;
		}

		inline void popCityObject( void ) {

			_currentCityObject = NULL; 
			if ( !_cityObjectStack.empty() ) {
				_currentCityObject = _cityObjectStack.top(); 
				_cityObjectStack.pop();
			}
		}

		virtual std::string getAttribute( void* attributes, const std::string& attname, const std::string& defvalue = "" ) = 0;

		inline std::string getGmlIdAttribute( void* attributes ) { return getAttribute( attributes, "gml:id", "" ); }

	protected:

		static std::map< std::string, CityGMLNodeType > s_cityGMLNodeTypeMap;
		static std::vector< std::string > s_knownNamespace;

		std::vector< std::string > _nodePath;

		std::stringstream _buff;

		ParserParams _params;

		CityModel* _model;

		CityObject* _currentCityObject;
		std::stack<CityObject*> _cityObjectStack;

		Geometry* _currentGeometry;

		Polygon* _currentPolygon;

		LinearRing* _currentRing;

		Appearance* _currentAppearance;

		CityObjectsTypeMask _objectsMask;

		int _currentLOD;

		bool _filterNodeType;
		unsigned int _filterDepth;

		std::vector<TVec3d> _points;

		int _srsDimension;

		char _orientation;

		bool _exterior;

		bool _appearanceAssigned;

		GeometryType _currentGeometryType;
	};

}