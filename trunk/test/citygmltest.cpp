#include <iostream>
#include <fstream>
#include <time.h> 
#include "citygml.h"


int main( int argc, char **argv )
{
	if ( argc < 2 ) 
	{
		std::cout << "Usage: citygmltest <filename> [-log]" << std::endl; 
		return -1;
	}

	bool log = ( argc > 2 && !strcmp( argv[1], "-log" ) );

	std::cout << "Parsing CityGML file " << argv[1] << "..." << std::endl;

	 time_t start;
	 time( &start );
//#define USE_STREAM
#ifdef USE_STREAM
	std::ifstream file;
	file.open( argv[1], std::ifstream::in );
	citygml::CityModel *city = citygml::load( file, citygml::COT_All );
#else
	citygml::CityModel *city = citygml::load( argv[1], citygml::COT_All );
#endif

	time_t end;
	time( &end );

	if ( !city ) return NULL;

	std::cout << "Done in " << difftime( end, start ) << " seconds." << std::endl; 

	std::cout << city->size() << " city objects read." << std::endl;

	std::cout << "Analyzing the city objects..." << std::endl;
	
	const citygml::CityObjectsMap& cityObjectsMap = city->getCityObjectsMap();

	citygml::CityObjectsMap::const_iterator it = cityObjectsMap.begin();

	for ( ; it != cityObjectsMap.end(); it++ )
	{
		const citygml::CityObjects& v = it->second;

		std::cout << ( log ? " Analyzing " : " Found " ) << v.size() << " " << citygml::getCityObjectsClassName( it->first ) << ( ( v.size() > 1 ) ? "s" : "" ) << "..." << std::endl;

		if ( log ) 
		{
			for ( unsigned int i = 0; i < v.size(); i++ )
			{
				std::cout << "  + found object " << v[i]->getId();
				if ( v[i]->getChildCount() > 0 ) std::cout << " with " << v[i]->getChildCount() << " children";
				std::cout << " with " << v[i]->size() << " geometr" << ( ( v[i]->size() > 1 ) ? "ies" : "y" );
				std::cout << std::endl;
			}
		}
	}

	std::cout << "Done." << std::endl;
	
	return 0;
}
