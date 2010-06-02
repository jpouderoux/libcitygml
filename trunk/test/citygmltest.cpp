#include <iostream>
#include "CityGMLParser.h"


int main( int argc, char **argv )
{
	if ( argc < 2 ) 
	{
		std::cout << "Usage: citygmltest <filename>" << std::endl; 
		return -1;
	}

	std::cout << "Parsing CityGML file " << argv[1] << "..." << std::endl;

	citygml::CityModel *city = citygml::load( argv[1], citygml::COT_All );
	
	if ( !city ) return NULL;

	std::cout << city->size() << " city objects read." << std::endl;

	std::cout << "Analyzing the city objects..." << std::endl;
	
	const citygml::CityObjectsMap& cityObjectsMap = city->getCityObjectsMap();

	citygml::CityObjectsMap::const_iterator it = cityObjectsMap.begin();

	for ( ; it != cityObjectsMap.end(); it++ )
	{
		const citygml::CityObjects& v = it->second;

		std::cout << " Analyzing " << v.size() << " " << citygml::getCityObjectsClassName( it->first ) << ( ( v.size() > 1 ) ? "s" : "" ) << "..." << std::endl;

		for ( unsigned int i = 0; i < v.size(); i++ )
		{
			std::cout << "  + found object " << v[i]->getId();
			if ( v[i]->getChildCount() > 0 ) std::cout << " with " << v[i]->getChildCount() << " children";
			std::cout << " with " << v[i]->size() << " geometries";
			std::cout << std::endl;
		}
	}

	std::cout << "Done." << std::endl;
	
	return 0;
}
