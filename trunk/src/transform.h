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

#ifndef __PROJECTION_H__
#define __PROJECTION_H__

#include "citygml.h"
#include "ogrsf_frmts.h"

class GeoTransform 
{
public:
	GeoTransform( const std::string& sourceURN, const std::string& destURN ) : _sourceURN( sourceURN ), _destURN( destURN )
	{
#ifdef USE_GDAL
		_sourceSRS = getProjection( _sourceURN );
		_destSRS = getProjection( _destURN );
		_trans = ( _sourceSRS && _destSRS ) ? OGRCreateCoordinateTransformation( (OGRSpatialReference*)_sourceSRS, (OGRSpatialReference*)_destSRS ) : NULL;
#else
		_sourceSRS = NULL;
		_destSRS = NULL;
		_trans = NULL;
#endif
	}

	~GeoTransform( void )
	{
#ifdef USE_GDAL
		if ( _sourceSRS ) OSRDestroySpatialReference( _sourceSRS );
		if ( _destSRS ) OSRDestroySpatialReference( _destSRS );
		if ( _trans ) OCTDestroyCoordinateTransformation( _trans );
#endif
	}

	inline void transform( TVec3d &p ) const
	{
#ifdef USE_GDAL
		if ( _trans ) ((OGRCoordinateTransformation*)_trans)->Transform( 1, &p.x, &p.y );
#endif
	}

	inline const std::string& getSourceURN( void ) const { return _sourceURN; }

#ifdef USE_GDAL
	static void* getProjection( const std::string &str )
	{
		OGRSpatialReference* proj = (OGRSpatialReference*)OSRNewSpatialReference(NULL);
		OGRErr err = proj->SetFromUserInput( str.c_str() );
		if ( err != OGRERR_NONE )
		{
			delete proj; 
			proj = NULL;
			std::cerr << "Error : Unable to create projection from definition " << str << " (error code: " << err << ")" << std::endl;
			std::cerr << "        Did you correctly set the GDAL_DATA env. var?" << std::endl;
		}
		return proj;
	}
#endif

private:
	std::string _sourceURN;
	std::string _destURN;
	void* _sourceSRS;
	void* _destSRS;
	void* _trans;
};

#endif // __PROJECTION_H__
