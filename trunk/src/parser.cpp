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

#define XERCES_STATIC_LIBRARY

#include <xercesc/util/XMLString.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/sax/InputSource.hpp>
#include <xercesc/util/BinInputStream.hpp>

#include <algorithm>
#include <stack>
#include <fstream>

namespace citygml
{

#define NODETYPE(_t_) CG_ ## _t_

	// CityGML node types
	typedef enum CityGMLNodeType
	{
		NODETYPE( Unknown ) = 0,

		// core
		NODETYPE( CityModel ),
		NODETYPE( cityObjectMember ),
		NODETYPE( creationDate ),	
		NODETYPE( terminationDate ),

		// gml
		NODETYPE( description ),
		NODETYPE( name ),
		NODETYPE( pos ),
		NODETYPE( boundedBy ),
		NODETYPE( Envelope ),
		NODETYPE( lowerCorner ),
		NODETYPE( upperCorner ),
		NODETYPE( Solid ),
		NODETYPE( surfaceMember ),
		NODETYPE( CompositeSurface ),
		NODETYPE( TriangulatedSurface ),
		NODETYPE( TexturedSurface ),
		NODETYPE( Triangle ),
		NODETYPE( Polygon ),
		NODETYPE( posList ),
		NODETYPE( OrientableSurface ),
		NODETYPE( LinearRing ),

		NODETYPE( lod1Solid ),
		NODETYPE( lod2Solid ),
		NODETYPE( lod3Solid ),
		NODETYPE( lod4Solid ),
		NODETYPE( lod1Geometry ),
		NODETYPE( lod2Geometry ),
		NODETYPE( lod3Geometry ),
		NODETYPE( lod4Geometry ),

		// bldg
		NODETYPE( Building ),
		NODETYPE( Room ),
		NODETYPE( Door ),
		NODETYPE( Window ),
		NODETYPE( BuildingInstallation ),
		NODETYPE( address ),
		NODETYPE( measuredHeight ),
		NODETYPE( class ),
		NODETYPE( function ),
		NODETYPE( usage ),

		// BoundarySurfaceType
		NODETYPE( WallSurface ),
		NODETYPE( RoofSurface ),
		NODETYPE( GroundSurface ),
		NODETYPE( ClosureSurface ),
		NODETYPE( FloorSurface ),
		NODETYPE( InteriorWallSurface ),
		NODETYPE( CeilingSurface ),
		NODETYPE( BuildingFurniture ),

		NODETYPE( CityFurniture ),

		NODETYPE( interior ),
		NODETYPE( exterior ),

		// wtr
		NODETYPE( WaterBody ),

		// veg
		NODETYPE( PlantCover ),
		NODETYPE( SolitaryVegetationObject ),

		// trans
		NODETYPE( TrafficArea ),
		NODETYPE( AuxiliaryTrafficArea ),
		NODETYPE( Track ),
		NODETYPE( Road ),
		NODETYPE( Railway ),
		NODETYPE( Square ),

		// luse
		NODETYPE( LandUse ),

		// dem
		NODETYPE( lod ),
		NODETYPE( TINRelief ),

		// gen
		NODETYPE( GenericCityObject ),

		// app
		NODETYPE( SimpleTexture ),	
		NODETYPE( ParameterizedTexture ),
		NODETYPE( GeoreferencedTexture ),
		NODETYPE( imageURI ),
		NODETYPE( textureMap ),
		NODETYPE( target ),
		NODETYPE( textureCoordinates ),
		NODETYPE( textureType ),
		NODETYPE( repeat ),

		NODETYPE( X3DMaterial ),
		NODETYPE( Material ),
		NODETYPE( appearanceMember ),
		NODETYPE( surfaceDataMember ),
		NODETYPE( shininess ),
		NODETYPE( transparency ),
		NODETYPE( specularColor ),
		NODETYPE( diffuseColor ),
		NODETYPE( emissiveColor ),
		NODETYPE( ambientIntensity )
	};

	inline std::string wstos( const XMLCh* const wstr );

	// CityGML SAX parsing handler
	class CityGMLHandler : public xercesc::HandlerBase 
	{
	public:

		CityGMLHandler( const std::string& objectsMask = "All", unsigned int minLOD = 0, unsigned int maxLOD = 4, bool optimize = true, bool pruneEmptyObjects = true, bool tesselate = true );

		~CityGMLHandler( void );

		virtual void startDocument( void ) {}

		virtual void endDocument( void ) {}

		virtual void startElement( const XMLCh* const, xercesc::AttributeList& );

		virtual void endElement( const XMLCh* const );

		virtual void characters( const XMLCh* const chars, const XMLSize_t length )
		{
			for ( unsigned int i = 0; i < length; i++ ) _buff << (char)chars[i]; 
		}

		virtual void fatalError( const xercesc::SAXParseException& e ) 
		{
			std::cerr << "Fatal error while parsing CityGML file: " << wstos( e.getMessage() ) << std::endl;
			std::cerr << "  Full path was: " << getFullPath() << std::endl;
		}

		inline CityModel* getModel( void ) { return _model; }

	protected:

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

		std::string getNodeName( const XMLCh* const );

		inline void pushCityObject( CityObject* object ) {
			
			if ( _currentCityObject && object ) _currentCityObject->_children.push_back( object );
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

	private:

		static std::map< std::string, CityGMLNodeType > s_cityGMLNodeTypeMap;
		static std::vector< std::string > s_knownNamespace;

		std::vector< std::string > _nodePath;

		std::stringstream _buff;

		bool _optimize;
		bool _tesselate;
	
		CityModel* _model;

		CityObject* _currentCityObject;
		std::stack<CityObject*> _cityObjectStack;

		Geometry* _currentGeometry;

		Polygon* _currentPolygon;

		LinearRing* _currentRing;

		Appearance* _currentAppearance;

		CityObjectsTypeMask _objectsMask;

		unsigned int _minLOD;
		unsigned int _maxLOD;
		int _currentLOD;

		bool _pruneEmptyObjects;

		bool _filterNodeType;
		unsigned int _filterDepth;

		std::vector<TVec3d> _points;

		int _srsDimension;

		char _orientation;

		bool _exterior;

		bool _appearanceAssigned;

		GeometryType _currentGeometryType;
	};

	///////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////

	std::map<std::string, CityGMLNodeType> CityGMLHandler::s_cityGMLNodeTypeMap;
	std::vector< std::string > CityGMLHandler::s_knownNamespace;

	CityGMLHandler::CityGMLHandler( const std::string& objectsMask, unsigned int minLOD, unsigned int maxLOD, bool optimize, bool pruneEmptyObjects, bool tesselate ) 
		: _tesselate( tesselate ), _model( NULL ), _currentCityObject( NULL ), 
		_currentGeometry( NULL ), _currentPolygon( NULL ), _currentRing( NULL ),  _currentGeometryType( GT_Unknown ),
		_currentAppearance( NULL ), _minLOD( minLOD ), _maxLOD( maxLOD ), _currentLOD( minLOD ), 
		_optimize( optimize ), _pruneEmptyObjects( pruneEmptyObjects ), _filterNodeType( false ), _filterDepth( 0 ), _exterior( true )
	{ 
		_objectsMask = getCityObjectsTypeMaskFromString( objectsMask );
		cityGMLInit(); 
	}

	CityGMLHandler::~CityGMLHandler( void ) 
	{
	}

	void CityGMLHandler::cityGMLInit( void ) 
	{
		if ( s_cityGMLNodeTypeMap.size() != 0 ) return;

#define INSERTNODETYPE(_t_) s_cityGMLNodeTypeMap[ #_t_ ] = CG_ ## _t_;

		// core
		INSERTNODETYPE( CityModel );
		INSERTNODETYPE( cityObjectMember );
		INSERTNODETYPE( creationDate );
		INSERTNODETYPE( terminationDate );

		// gml
		INSERTNODETYPE( name );
		INSERTNODETYPE( pos );
		INSERTNODETYPE( description );
		INSERTNODETYPE( boundedBy );
		INSERTNODETYPE( Envelope );
		INSERTNODETYPE( lowerCorner );
		INSERTNODETYPE( upperCorner );
		INSERTNODETYPE( Solid );
		INSERTNODETYPE( surfaceMember );
		INSERTNODETYPE( CompositeSurface );
		INSERTNODETYPE( TriangulatedSurface );
		INSERTNODETYPE( TexturedSurface );
		INSERTNODETYPE( Triangle );
		INSERTNODETYPE( Polygon );
		INSERTNODETYPE( posList );
		INSERTNODETYPE( OrientableSurface );
		INSERTNODETYPE( LinearRing );

		INSERTNODETYPE( lod1Solid );
		INSERTNODETYPE( lod2Solid );
		INSERTNODETYPE( lod3Solid );
		INSERTNODETYPE( lod4Solid );
		INSERTNODETYPE( lod1Geometry );
		INSERTNODETYPE( lod2Geometry );
		INSERTNODETYPE( lod3Geometry );
		INSERTNODETYPE( lod4Geometry );

		// bldg
		INSERTNODETYPE( Building );
		INSERTNODETYPE( Room );
		INSERTNODETYPE( Door );
		INSERTNODETYPE( Window );
		INSERTNODETYPE( BuildingInstallation );
		INSERTNODETYPE( address );
		INSERTNODETYPE( measuredHeight );
		INSERTNODETYPE( class );
		INSERTNODETYPE( function );
		INSERTNODETYPE( usage )
		
		// BoundarySurfaceType
		INSERTNODETYPE( WallSurface );
		INSERTNODETYPE( RoofSurface );
		INSERTNODETYPE( GroundSurface );
		INSERTNODETYPE( ClosureSurface );
		INSERTNODETYPE( FloorSurface );
		INSERTNODETYPE( InteriorWallSurface );
		INSERTNODETYPE( CeilingSurface );
		INSERTNODETYPE( BuildingFurniture );

		INSERTNODETYPE( CityFurniture );

		INSERTNODETYPE( interior );
		INSERTNODETYPE( exterior );

		// wtr
		INSERTNODETYPE( WaterBody );

		// veg
		INSERTNODETYPE( PlantCover );
		INSERTNODETYPE( SolitaryVegetationObject );

		// trans
		INSERTNODETYPE( TrafficArea );
		INSERTNODETYPE( AuxiliaryTrafficArea );
		INSERTNODETYPE( Track );
		INSERTNODETYPE( Road );
		INSERTNODETYPE( Railway );
		INSERTNODETYPE( Square );

		// luse
		INSERTNODETYPE( LandUse );

		// dem
		INSERTNODETYPE( lod );
		INSERTNODETYPE( TINRelief );

		// gen
		INSERTNODETYPE( GenericCityObject );

		// app
		INSERTNODETYPE( SimpleTexture );
		INSERTNODETYPE( ParameterizedTexture );
		INSERTNODETYPE( GeoreferencedTexture );
		INSERTNODETYPE( imageURI );
		INSERTNODETYPE( textureMap );
		INSERTNODETYPE( target );
		INSERTNODETYPE( textureCoordinates );
		INSERTNODETYPE( textureType );
		INSERTNODETYPE( repeat );

		INSERTNODETYPE( X3DMaterial );
		INSERTNODETYPE( Material );
		INSERTNODETYPE( appearanceMember );
		INSERTNODETYPE( surfaceDataMember );		
		INSERTNODETYPE( shininess );
		INSERTNODETYPE( transparency );
		INSERTNODETYPE( specularColor );
		INSERTNODETYPE( diffuseColor );
		INSERTNODETYPE( emissiveColor );
		INSERTNODETYPE( ambientIntensity );

		// Set the known namespaces

#define INSERTKNOWNNAMESPACE(_t_) s_knownNamespace.push_back( #_t_ );

		INSERTKNOWNNAMESPACE( gml );
		INSERTKNOWNNAMESPACE( citygml );
		INSERTKNOWNNAMESPACE( core );
		INSERTKNOWNNAMESPACE( app );
		INSERTKNOWNNAMESPACE( bldg );
		INSERTKNOWNNAMESPACE( frn );
		INSERTKNOWNNAMESPACE( grp );
		INSERTKNOWNNAMESPACE( gen );
		INSERTKNOWNNAMESPACE( luse );
		INSERTKNOWNNAMESPACE( dem );
		INSERTKNOWNNAMESPACE( tran );
		INSERTKNOWNNAMESPACE( trans );
		INSERTKNOWNNAMESPACE( veg );
		INSERTKNOWNNAMESPACE( wtr );
		INSERTKNOWNNAMESPACE( tex );	
	}

	CityGMLNodeType CityGMLHandler::getNodeTypeFromName( const std::string& name )
	{
		std::map<std::string, CityGMLNodeType>::const_iterator elt = s_cityGMLNodeTypeMap.find( name );

		if ( elt == s_cityGMLNodeTypeMap.end() ) return CG_Unknown;

		return elt->second;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Helpers

	inline std::string trim_left( const std::string& s, const std::string& t = " \t\r\n" ) 
	{ 
		std::string d( s ); 
		return d.erase( 0, s.find_first_not_of( t ) ); 
	} 

	inline std::string trim_right( const std::string& s, const std::string& t = " \t\r\n" )
	{
		std::string d( s ); 
		size_t endpos = d.find_last_not_of( t );
		if ( endpos != std::string::npos ) return d.erase( endpos + 1 );
		return d;
	}

	inline std::string trim( const std::string& s, const std::string& t = " \t\r\n" )
	{
		return trim_left( trim_right( s, t ), t );
	}

	inline std::string wstos( const XMLCh* const wstr ) 
	{
		std::wstring w( (const wchar_t*)wstr );
		return std::string( w.begin(), w.end() );
	}

	inline std::string wstos( const std::wstring& wstr ) 
	{
		return std::string( wstr.begin(), wstr.end() );
	}

	std::string getAttribute( xercesc::AttributeList& attributes, const std::string& attname, const std::string& defvalue = "" )
	{
		const XMLCh* att = attributes.getValue( attname.c_str() );
		return att ? wstos( att ) : defvalue;
	}

	std::string getGmlIdAttribute( xercesc::AttributeList& attributes )
	{
		return getAttribute( attributes, "gml:id", "" );
	}

	template<class T> inline void parseValue( std::stringstream &s, T &v ) 
	{
		if ( !s.eof() ) s >> v;
	}

	template<class T> inline void parseVecList( std::stringstream &s, std::vector<T> &vec ) 
	{
		while ( !s.eof() ) 
		{
			T v;
			s >> v;
			vec.push_back( v );
		}
	}

	///////////////////////////////////////////////////////////////////////////////

	std::string CityGMLHandler::getNodeName( const XMLCh* const wname ) 
	{
		std::string name = wstos( wname );

		// remove the known namespace if it exists

		size_t pos = name.find_first_of( ":" );
		if ( pos == std::string::npos ) return name;

		std::string nspace = name.substr( 0, pos );

		for ( int i = s_knownNamespace.size() - 1; i >= 0; i-- ) 
			if ( nspace == s_knownNamespace[i] ) 
				return  name.substr( s_knownNamespace[i].length() + 1 );

		return name;
	}

	void CityGMLHandler::startElement( const XMLCh* const wlocalname, xercesc::AttributeList& attributes ) 
	{
		std::string localname = getNodeName( wlocalname );

		_nodePath.push_back( localname );

		CityGMLNodeType nodeType = getNodeTypeFromName( localname );

		// get the LOD level if node name starts with 'lod'
		if ( localname.length() > 3 && localname.find( "lod" ) == 0 ) _currentLOD = localname[3] - '0';

#define LOD_FILTER() if ( _currentLOD < (int)_minLOD || _currentLOD > (int)_maxLOD ) break;

#define NODETYPE_FILTER() ( _filterNodeType && getPathDepth() > _filterDepth )

#define MODEL_FILTER() if ( !_model ) break;

		if ( NODETYPE_FILTER() ) return;

		switch ( nodeType ) 
		{
			case NODETYPE( CityModel ):
			_model = new CityModel();
			break;

			// City objects management
#define MANAGE_OBJECT( _t_ )\
	case CG_ ## _t_ :\
		if ( _objectsMask & COT_ ## _t_ )\
			{ pushCityObject( new _t_( getGmlIdAttribute( attributes ) ) ); /*std::cout << "new "<< #_t_ " - " << _currentCityObject->getId() << std::endl;*/ }\
		else { pushCityObject( NULL ); _filterNodeType = true; _filterDepth = getPathDepth(); }\
		break;

			MANAGE_OBJECT( GenericCityObject );
			MANAGE_OBJECT( Building );
			MANAGE_OBJECT( Room );
			MANAGE_OBJECT( BuildingInstallation );
			MANAGE_OBJECT( BuildingFurniture );
			MANAGE_OBJECT( Door );
			MANAGE_OBJECT( Window );
			MANAGE_OBJECT( CityFurniture );
			MANAGE_OBJECT( Track );
			MANAGE_OBJECT( Road );
			MANAGE_OBJECT( Railway );
			MANAGE_OBJECT( Square );
			MANAGE_OBJECT( PlantCover );
			MANAGE_OBJECT( SolitaryVegetationObject );
			MANAGE_OBJECT( WaterBody );
			MANAGE_OBJECT( TINRelief );
			MANAGE_OBJECT( LandUse );			
#undef MANAGE_OBJECT

			// BoundarySurfaceType
#define MANAGE_SURFACETYPE( _t_ ) case CG_ ## _t_ ## Surface : _currentGeometryType = GT_ ## _t_; break;
			MANAGE_SURFACETYPE( Wall );
			MANAGE_SURFACETYPE( Roof );

			MANAGE_SURFACETYPE( Ground );
			MANAGE_SURFACETYPE( Closure );
			MANAGE_SURFACETYPE( Floor );
			MANAGE_SURFACETYPE( InteriorWall );
			MANAGE_SURFACETYPE( Ceiling );
#undef MANAGE_SURFACETYPE

			// Geometry management

		case NODETYPE( TexturedSurface ):
		case NODETYPE( OrientableSurface ):
			_orientation = getAttribute( attributes, "orientation", "+" )[0];
			break;

		case NODETYPE( surfaceMember ):
		case NODETYPE( TriangulatedSurface ):
			LOD_FILTER();
			//_orientation = getAttribute( attributes, "orientation", "+" )[0];
			_orientation = '+';
			_currentGeometry = new Geometry( getGmlIdAttribute( attributes ), _currentGeometryType, _currentLOD );
			break;

		case NODETYPE( Triangle ):
		case NODETYPE( Polygon ):
			LOD_FILTER();
			_currentPolygon = new Polygon( getGmlIdAttribute( attributes ) );
			break;

		case NODETYPE( posList ):
			LOD_FILTER();
			_srsDimension = atoi( getAttribute( attributes, "srsDimension", "3" ).c_str() );
			if ( _srsDimension != 3 ) 
				std::cerr << "Warning ! srsDimension of gml:posList not set to 3!" << std::endl;
			break;

		case NODETYPE( interior ): _exterior = false; break;
		case NODETYPE( exterior ): _exterior = true;  break;

		case NODETYPE( LinearRing ): 
			LOD_FILTER();
			_currentRing = new LinearRing( getGmlIdAttribute( attributes ), _exterior ); 
			break;

			// Material management

		case NODETYPE( target ):
			if ( _currentAppearance ) 
			{
				std::string uri = getAttribute( attributes, "uri" );
				if ( uri != "" ) 
				{
					if ( uri.length() > 0 && uri[0] == '#' ) uri = uri.substr( 1 );		
					_model->_appearanceManager.assignNode( uri );
				}
				_appearanceAssigned = true;
			}
			break;

		case NODETYPE( SimpleTexture ):
		case NODETYPE( ParameterizedTexture ):
		case NODETYPE( GeoreferencedTexture ):
			_currentAppearance = new Texture( getGmlIdAttribute( attributes ) );
			_model->_appearanceManager.addAppearance( _currentAppearance );
			_appearanceAssigned = false;
			break;

		case NODETYPE( Material ):
		case NODETYPE( X3DMaterial ):
			_currentAppearance = new Material( getGmlIdAttribute( attributes ) );
			_model->_appearanceManager.addAppearance( _currentAppearance );
			_appearanceAssigned = false;
			break;

		default:
			break;
		};
	}

	void CityGMLHandler::endElement( const XMLCh* const wlocalname ) 
	{
		std::string localname = getNodeName( wlocalname );

		_nodePath.pop_back();

		CityGMLNodeType nodeType = getNodeTypeFromName( localname );
		
		if ( NODETYPE_FILTER() ) { clearBuffer(); return; }

		if ( nodeType == NODETYPE( Unknown ) ) // unknown node ? skip now to avoid the buffer triming pass
		{
			clearBuffer();
			return; 
		}

		// Trim the char buffer  
		std::stringstream buffer;
		buffer << trim( _buff.str() );

		// set the LOD level if node name starts with 'lod'
		if ( localname.find( "lod" ) == 0 ) _currentLOD = _minLOD;

		switch ( nodeType ) 
		{

		case NODETYPE( CityModel ):
			MODEL_FILTER();
			_model->finish( _optimize );
			break;

			// City objects management

		case NODETYPE( GenericCityObject ):
		case NODETYPE( Building ):
		case NODETYPE( Room ):
		case NODETYPE( BuildingInstallation ):
		case NODETYPE( BuildingFurniture ):
		case NODETYPE( Door ):
		case NODETYPE( Window ):
		case NODETYPE( CityFurniture ):
		case NODETYPE( Track ):
		case NODETYPE( Road ):
		case NODETYPE( Railway ):
		case NODETYPE( Square ):
		case NODETYPE( PlantCover ):
		case NODETYPE( SolitaryVegetationObject ):
		case NODETYPE( WaterBody ):
		case NODETYPE( TINRelief ):
		case NODETYPE( LandUse ):
			MODEL_FILTER();
			if ( _currentCityObject && ( _currentCityObject->size() > 0 || !_pruneEmptyObjects ) ) 
			{	// Prune empty objects 
				_model->addCityObject( _currentCityObject );
				if ( _cityObjectStack.size() == 1 ) _model->addCityObjectAsRoot( _currentCityObject );
			}
			else delete _currentCityObject; 
			popCityObject();
			_filterNodeType = false;
			break;

		case NODETYPE( Envelope ): 
			MODEL_FILTER();
			if ( _points.size() >= 2 ) 
			{
				if ( getPathDepth() == 2 ) // CityModel envelope
				{
					_model->_envelope._lowerBound = _points[0];
					_model->_envelope._upperBound = _points[1];
				}
				else if ( _currentCityObject )
				{
					_currentCityObject->_envelope._lowerBound = _points[0];
					_currentCityObject->_envelope._upperBound = _points[1];
				}
			}
			_points.clear();
			break;

		case NODETYPE( lowerCorner ):
		case NODETYPE( upperCorner ):
			{
				TVec3d p;
				buffer >> p;
				if ( nodeType == NODETYPE( lowerCorner ) )
					_points.insert( _points.begin(), p );
				else
					_points.push_back( p );
			}
			break;

		case NODETYPE( lod ):
			parseValue( buffer, _currentLOD );
			break;

	case NODETYPE( class ):
	case NODETYPE( function ):
	case NODETYPE( usage ):
	case NODETYPE( measuredHeight ):
	case NODETYPE( name ):
	case NODETYPE( description ):
	case NODETYPE( creationDate ):
	case NODETYPE( terminationDate ):
		if ( _currentCityObject ) _currentCityObject->setProp( localname, buffer.str() );
		break;

		// Geometry management 

	case NODETYPE( WallSurface ):
	case NODETYPE( RoofSurface ):
	case NODETYPE( GroundSurface ):
	case NODETYPE( ClosureSurface ):
	case NODETYPE( FloorSurface ):
	case NODETYPE( InteriorWallSurface ):
	case NODETYPE( CeilingSurface ):
		_currentGeometryType = GT_Unknown;
		break;

	case NODETYPE( surfaceMember ):
	case NODETYPE( TriangulatedSurface ):
		if ( _currentCityObject && _currentGeometry ) 
			_currentCityObject->_geometries.push_back( _currentGeometry );
		else 
			delete _currentGeometry;
		_currentGeometry = NULL;
		break;

	case NODETYPE( Triangle ):
	case NODETYPE( Polygon ):
		if ( _currentGeometry && _currentPolygon )
		{
			_currentPolygon->finish( ( nodeType == NODETYPE( Triangle ) ) ? false : _tesselate );							
			_currentGeometry->addPolygon( _currentPolygon );
		}
		_currentPolygon = NULL;
		break;

	case NODETYPE( pos ):
		if ( _currentCityObject )
		{
			TVec3d p;
			parseValue( buffer, p );
			if ( !_currentPolygon )
				_points.push_back( p );
			else if ( _currentRing )
				_currentRing->addVertex( p );
		}
		break;

	case NODETYPE( posList ):
		if ( !_currentPolygon ) break;
		_currentPolygon->_negNormal = ( _orientation != '+' );
		if ( _currentRing ) 
			parseVecList( buffer, _currentRing->getVertices() );
		break;

	case NODETYPE( interior ):
	case NODETYPE( exterior ): _exterior = true; break;

	case NODETYPE( LinearRing ): 
		if ( _currentPolygon && _currentRing ) 
			_currentPolygon->addRing( _currentRing );	
		_currentRing = NULL;
		break;

		// Material management

	case NODETYPE( textureMap ):
	case NODETYPE( imageURI ):
		if ( Texture* texture = dynamic_cast<Texture*>( _currentAppearance ) ) 
		{
			texture->_url = buffer.str();
			std::replace( texture->_url.begin(), texture->_url.end(), '\\', '/' );
		}
		break;

	case NODETYPE( target ):
		MODEL_FILTER();
		if ( _currentAppearance )
		{
			std::string uri = buffer.str();
			if ( uri != "" ) 
			{
				if ( uri.length() > 0 && uri[0] == '#' ) uri = uri.substr( 1 );
				_model->_appearanceManager.assignNode( uri );
			}
		}
		break;

	case NODETYPE( textureCoordinates ):
		MODEL_FILTER();
		if ( Texture* texture = dynamic_cast<Texture*>( _currentAppearance ) ) 
		{
			TexCoords *vec = new TexCoords();
			parseVecList( buffer, *vec );
			_model->_appearanceManager.assignTexCoords( vec );		
		}
		break;

	case NODETYPE( SimpleTexture ):
	case NODETYPE( ParameterizedTexture ):
	case NODETYPE( GeoreferencedTexture ):
	case NODETYPE( Material ):
	case NODETYPE( X3DMaterial ):
		if ( _currentAppearance && _currentGeometry && !_appearanceAssigned ) _model->_appearanceManager.assignNode( _currentGeometry->getId() );
		_currentAppearance = NULL;
		break;

	case NODETYPE( diffuseColor ):
	case NODETYPE( emissiveColor ):
	case NODETYPE( specularColor ):
		if ( Material* mat = dynamic_cast<Material*>( _currentAppearance ) ) 
		{
			TVec3f col;
			parseValue( buffer, col );	
			if ( nodeType == NODETYPE( diffuseColor ) ) mat->_diffuse = col;
			else if ( nodeType == NODETYPE( emissiveColor ) ) mat->_emissive = col;
			else if ( nodeType == NODETYPE( specularColor ) ) mat->_specular = col;
		}
		break;

	case NODETYPE( ambientIntensity ):
	case NODETYPE( shininess ):
	case NODETYPE( transparency ):
		if ( Material* mat = dynamic_cast<Material*>( _currentAppearance ) ) 
		{
			float val;
			parseValue( buffer, val );	
			if ( nodeType == NODETYPE( shininess ) ) mat->_shininess = val;
			else if ( nodeType == NODETYPE( transparency ) ) mat->_transparency = val;
			else if ( nodeType == NODETYPE( ambientIntensity ) ) mat->_ambientIntensity = val;
		}
		break;

	default:
		break;
		};

		clearBuffer();
	}

	///////////////////////////////////////////////////////////////////////////////

	class StdBinInputStream : public xercesc::BinInputStream
	{
	public:
		StdBinInputStream( std::istream& stream ) : BinInputStream(), m_stream( stream ) {}

		virtual ~StdBinInputStream( void ) {}

		virtual XMLFilePos curPos( void ) const { return m_stream.tellg(); }

		virtual XMLSize_t readBytes( XMLByte* const buf, const XMLSize_t maxToRead )
		{
			assert( sizeof(XMLByte) == sizeof(char) );
			if ( !m_stream ) return 0;
			m_stream.read( reinterpret_cast<char*>(buf), maxToRead );
			return m_stream.gcount();
		}

		virtual const XMLCh* getContentType() const { return NULL; }

	private:
		std::istream& m_stream;
	};

	class StdBinInputSource : public xercesc::InputSource
	{
	public:
		StdBinInputSource( std::istream& stream ) : m_stream( stream ) {}

		virtual xercesc::BinInputStream* makeStream() const 
		{
			return new StdBinInputStream( m_stream );
		}
	private:
		std::istream& m_stream;
	};

	// Parsing methods

	CityModel* load( std::istream& stream, const std::string& objectsMask, unsigned int minLOD, unsigned int maxLOD, bool optimize, bool pruneEmptyObjects, bool tessalate )
	{
		try 
		{
			xercesc::XMLPlatformUtils::Initialize();
		}
		catch ( const xercesc::XMLException& e ) 
		{
			std::cerr << "CityGML: XML Exception occures during initialization!" << std::endl << wstos( e.getMessage() ) << std::endl;
			return false;
		}

		CityGMLHandler* handler = new CityGMLHandler( objectsMask, minLOD, maxLOD, optimize, pruneEmptyObjects, tessalate );

		xercesc::SAXParser* parser = new xercesc::SAXParser();
		parser->setDoNamespaces( false );    	
		parser->setDocumentHandler( handler );
		parser->setErrorHandler( handler );

		CityModel* model = NULL;

		try 
		{
			StdBinInputSource input( stream );			
			parser->parse( input );
			model = handler->getModel();
		}
		catch ( const xercesc::XMLException& e ) 
		{
			std::cerr << "CityGML: XML Exception occures!" << std::endl << wstos( e.getMessage() ) << std::endl;
			delete handler->getModel();
		}
		catch ( const xercesc::SAXParseException& e ) 
		{
			std::cerr << "CityGML: SAXParser Exception occures!" << std::endl << wstos( e.getMessage() ) << std::endl;
			delete handler->getModel();
		}
		catch ( ... ) 
		{
			std::cerr << "CityGML: Unexpected Exception occures!" << std::endl ;
			delete handler->getModel();
		}
		
		delete parser;
		delete handler;
		return model;
	}

	CityModel* load( const std::string& fname, const std::string& objectsMask, unsigned int minLOD, unsigned int maxLOD, bool optimize, bool pruneEmptyObjects, bool tessalate )
	{
		std::ifstream file;
		file.open( fname.c_str(), std::ifstream::in );
		if ( file.fail() ) { std::cerr << "CityGML: Unable to open file " << fname << "!" << std::endl; return NULL; }
		CityModel* model = load( file, objectsMask, minLOD, maxLOD, optimize, pruneEmptyObjects, tessalate );
		file.close();
		return model;
	}
}

