#ifndef _VEC4_H_
#define _VEC4_H_

#include <sstream>

// 4D vector class.

template< class T > class TVec4
{
public:
	union
	{
		T xyzw[4];
		T rgba[4];
		struct { T x, y, z, w; };
		struct { T r, g, b, a; };
	};

public:
	TVec4( const T x = (T)0, const T y = (T)0, const T z = (T)0, const T w = (T)0 )
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}
	
	TVec4( const T vec[], const T w = (T)0 ) { memcpy( xyzw, vec, 4 * sizeof(T) ); this->w = w; }

	TVec4( const T vec[] ) { memcpy( xyzw, vec, 4 * sizeof(T) ); }
};

template<class T> inline std::ostream& operator<<( std::ostream & os, TVec4<T> const & v )
{
	return os << std::fixed << v.x << " " << std::fixed << v.y << " " << std::fixed << v.z << " " << std::fixed << v.w;
}

template<class T> inline std::istream& operator>>( std::istream & is, TVec4<T> & v )
{
	return is >> v.x >> v.y >> v.z >>  v.w;
}

typedef TVec4< float >			TVec4f;
typedef TVec4< double >			TVec4d;

#endif
