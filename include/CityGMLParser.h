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

#ifndef __CITYGMLPARSER_H__
#define __CITYGMLPARSER_H__

#include "CityGMLObjects.h"

namespace citygml 
{
	LIBCITYGML_EXPORT CityModel* load( const std::string& fileName, CityObjectsTypeMask objectsMask = COT_All, 
		unsigned int minLOD = 1, unsigned int maxLOD = 4, 
		bool pruneEmptyObjects = true, bool triangulate = true );
}

#endif // __CITYGMLPARSER_H__
