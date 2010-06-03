///////////////////////////////////////////////////////////////////////////////
// OSG plugin for reading OGC CityGML v0.3 - v1.0 format using libcitygml
// http://code.google.com/p/libcitygml
// Copyright(c) 2010 Joachim Pouderoux, BRGM
//////////////////////////////////////////////////////////////////////////

#include <osg/Notify>
#include <osg/Node>
#include <osg/Geometry>
#include <osg/ProxyNode>
#include <osg/MatrixTransform>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/StateSet>
#include <osg/Material>
#include <osg/Texture2D>
#include <osg/TexGen>
#include <osg/TexMat>
#include <osg/ShapeDrawable>
#include <osg/Shape>
#include <osg/LightModel>
#include <osg/Point>

#include <osgText/Font>
#include <osgText/Text>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include "citygml.h"

#include <algorithm>
#include <cctype> 


class ReaderWriterCityGML : public osgDB::ReaderWriter
{
public:
	ReaderWriterCityGML( void )
	{ 
        supportsExtension( "citygml", "CityGML format" );

		supportsOption( "names", "Add the name of the city objects on top of them" );
		supportsOption( "objectsMask", "Set the objects mask" );
		supportsOption( "minLOD", "Minimum LOD level to fetch" );
		supportsOption( "maxLOD", "Maximum LOD level to fetch" );
		supportsOption( "pruneEmptyObjects", "Prune empty objects (ie. without -supported- geometry)" );
	}

	virtual const char* className( void ) const { return "CityGML Reader"; }
	
	virtual ReadResult readNode( const std::string&, const osgDB::ReaderWriter::Options* ) const;

private:
	class Settings 
	{
	public:
		Settings( void ) : _printNames( false ),
			_objectsMask( citygml::COT_All ), _minLOD( 1 ), _maxLOD( 4 ),
			_pruneEmptyObjects( false ) {}

		void parseOptions( const osgDB::ReaderWriter::Options* );

	public:
		bool _printNames;
		citygml::CityObjectsTypeMask _objectsMask;
		unsigned int _minLOD;
		unsigned int _maxLOD;
		bool _pruneEmptyObjects;
		std::map< std::string, osg::Texture2D* > _textureMap;
	};

private:
	osg::Geode* createCityObject( citygml::CityObject*, Settings& ) const;
};

// Register with Registry to instantiate the above reader/writer.
REGISTER_OSGPLUGIN( citygml, ReaderWriterCityGML )

void ReaderWriterCityGML::Settings::parseOptions( const osgDB::ReaderWriter::Options* options )
{
	if ( !options ) return;
	std::istringstream iss( options->getOptionString() );
	std::string currentOption;
	while ( iss >> currentOption )
	{
		std::transform( currentOption.begin(), currentOption.end(), currentOption.begin(), tolower );
		if ( currentOption == "names" ) _printNames = true;
		else if ( currentOption == "objectsmask" ) { int i; iss >> i; _objectsMask = i; }
		else if ( currentOption == "minlod" ) iss >> _minLOD;
		else if ( currentOption == "maxlod" ) iss >> _maxLOD;
		else if ( currentOption == "pruneemptyobjects" ) _pruneEmptyObjects = true;
	}
}

// Read CityGML file using libcitygml and generate the OSG scenegraph
osgDB::ReaderWriter::ReadResult ReaderWriterCityGML::readNode( const std::string& file, const osgDB::ReaderWriter::Options* options ) const
{
	std::string ext = osgDB::getLowerCaseFileExtension( file );
	if ( !acceptsExtension( ext ) ) return ReadResult::FILE_NOT_HANDLED;

	// try to open the file as is
	std::string fileName = osgDB::findDataFile( file, options );
	if ( fileName.empty() )
	{
		// not found, so remove the .citygml extension file
		std::string fname = osgDB::getNameLessExtension( file );
		fileName = osgDB::findDataFile( fname, options );
		if ( fileName.empty() ) return ReadResult::FILE_NOT_FOUND;
	}

	Settings settings;
	settings.parseOptions( options );

	osgDB::getDataFilePathList().push_front( osgDB::getFilePath( fileName ) );

	// Redirect both std::cout & std::cerr (used by libcitygml) to osg::notify stream
	std::streambuf* coutsb = std::cout.rdbuf( osg::notify(osg::NOTICE).rdbuf() );
	std::streambuf* cerrsb = std::cerr.rdbuf( osg::notify(osg::NOTICE).rdbuf() );

	osg::notify(osg::NOTICE) << "Parsing CityGML file " << fileName << "..." << std::endl;

	citygml::CityModel *city = citygml::load( fileName, settings._objectsMask, settings._minLOD, settings._maxLOD, settings._pruneEmptyObjects );
	
	if ( !city ) return NULL;

	osg::notify(osg::NOTICE) << city->size() << " city objects read." << std::endl;
	
	osg::notify(osg::NOTICE) << "Creation of the OSG city objects' geometry..." << std::endl;
	
	const citygml::CityObjectsMap& cityObjectsMap = city->getCityObjectsMap();

	citygml::CityObjectsMap::const_iterator it = cityObjectsMap.begin();

	osg::Group* root = new osg::Group();
	root->setName( fileName );

	for ( ; it != cityObjectsMap.end(); it++ )
	{
		const citygml::CityObjects& v = it->second;

		osg::notify(osg::NOTICE) << " Creation of " << v.size() << " " << citygml::getCityObjectsClassName( it->first ) << ( ( v.size() > 1 ) ? "s" : "" ) << "..." << std::endl;
		
		osg::Group* grp = new osg::Group;
		grp->setName( citygml::getCityObjectsClassName( it->first ) );
		
		for ( unsigned int i = 0; i < v.size(); i++ )
		{
			if ( osg::Geode* geode = createCityObject( v[i], settings ) )
				grp->addChild( geode );
		}
		root->addChild( grp );
	}

	osg::notify(osg::NOTICE) << "Done." << std::endl;

	delete city;
	
	// Restore cout/cerr streams
	std::cout.rdbuf( coutsb );
	std::cerr.rdbuf( cerrsb );

	return root;
}

osg::Geode* ReaderWriterCityGML::createCityObject( citygml::CityObject* object, Settings& settings ) const
{
	// Skip objects without geometry
	if ( object->size() == 0 ) return NULL;

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->setName( object->getId() );
	
	//osg::notify(osg::NOTICE) << "Creating object " << object->getId() << std::endl;

	// Get the default color for the whole city object
	osg::ref_ptr<osg::Vec4Array> shared_colors = new osg::Vec4Array;
    shared_colors->push_back( osg::Vec4( object->getDefaultColor().r, object->getDefaultColor().g, object->getDefaultColor().b, 1.f ) );

	osg::ref_ptr<osg::Vec4Array> roof_color = new osg::Vec4Array;
    roof_color->push_back( osg::Vec4( 0.9f, 0.1f, 0.1f, 1.0f ) );
	
	for ( unsigned int i = 0; i < object->size(); i++ ) 
	{
		const citygml::Geometry& geometry = *object->getGeometry( i );

		if ( ( geometry.getType() == citygml::GT_Wall )
			|| ( geometry.getType() == citygml::GT_Ground ) ) 
		{
			settings._foundationsElements[ geometry.getType() ].push_back( object->getGeometry( i ) );
		}

		for ( unsigned int j = 0; j < geometry.size(); j++ ) 
		{
			const citygml::Polygon* p = geometry[j];
			if ( !p || !p->getIndices() || p->getIndicesSize() == 0 ) continue;
			
			// Geometry management

			osg::Geometry* geom = new osg::Geometry();
			
			// Translate the vertices to the origin (first encountered point for now)
			osg::Vec3Array* vertices = new osg::Vec3Array;
			for ( unsigned int k = 0; k < p->size(); k++ )
			{
				osg::Vec3d pt( (*p)[k][0], (*p)[k][1], (*p)[k][2] );
				vertices->push_back( pt );
			}

			geom->setVertexArray( vertices );

			osg::DrawElementsUInt* triangles = new osg::DrawElementsUInt( osg::PrimitiveSet::TRIANGLES, 0 );
			triangles->reserve( p->getIndicesSize() );
			unsigned int* indices = p->getIndices();
			for ( unsigned int i = 0 ; i < p->getIndicesSize() / 3; i++ )
			{
				triangles->push_back( indices[ i * 3 + 0 ] );
				triangles->push_back( indices[ i * 3 + 1 ] );
				triangles->push_back( indices[ i * 3 + 2 ] );	
			}
			geom->addPrimitiveSet( triangles );
			
			// Normal management
			
			osg::ref_ptr<osg::Vec3Array> shared_normals = new osg::Vec3Array;
			shared_normals->push_back( osg::Vec3( p->getNormal().x, p->getNormal().y, p->getNormal().z ) );
			geom->setNormalArray( shared_normals.get() );
			geom->setNormalBinding( osg::Geometry::BIND_OVERALL );

			// Material management

			osg::ref_ptr<osg::StateSet> stateset = geom->getOrCreateStateSet();

			const citygml::Appearance *mat = p->getAppearance();

			bool colorset = false;
 
			if ( mat )
			{
				shared_colors->clear();
				shared_colors->push_back( osg::Vec4( 1.f, 1.f, 1.f, 1.f ) );

				if ( const citygml::Material* m = dynamic_cast<const citygml::Material*>( mat ) )
				{
#define TOVEC4(_t_) osg::Vec4( _t_.r, _t_.g, _t_.b, _t_.a ) 
					osg::ref_ptr<osg::Vec4Array> color = new osg::Vec4Array;
					TVec4f diffuse( m->getDiffuse(), 0.f );
					TVec4f emissive( m->getEmissive(), 0.f );
					TVec4f specular( m->getSpecular(), 0.f );
					float ambient = m->getAmbientIntensity();
										
					osg::Material* material = new osg::Material;
					material->setColorMode( osg::Material::OFF );
					material->setDiffuse( osg::Material::FRONT_AND_BACK, TOVEC4( diffuse ) );
					material->setSpecular( osg::Material::FRONT_AND_BACK, TOVEC4( specular ) );
					material->setEmission( osg::Material::FRONT_AND_BACK, TOVEC4( emissive ) );					
					material->setShininess( osg::Material::FRONT_AND_BACK, m->getShininess() );					
					material->setAmbient( osg::Material::FRONT_AND_BACK, osg::Vec4( ambient, ambient, ambient, 1.0 ) );
					stateset->setAttributeAndModes( material, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON );
					stateset->setMode( GL_LIGHTING, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON );
					
					colorset = true;
				}
				else if ( const citygml::Texture* t = dynamic_cast<const citygml::Texture*>( mat ) ) 
				{
					const citygml::TexCoords *texCoords = p->getTexCoords();

					if ( texCoords )
					{
						osg::Texture2D* texture = NULL;

						if ( settings._textureMap.find( t->getUrl() ) == settings._textureMap.end() )
						{
							// Load a new texture 
							osg::notify(osg::NOTICE) << "  Loading texture " << t->getUrl() << "..." << std::endl;

							if ( osg::Image* image = osgDB::readImageFile( t->getUrl() ) )
							{
								texture = new osg::Texture2D;
								texture->setImage( image );
								texture->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR );
								texture->setFilter( osg::Texture::MAG_FILTER, osg::Texture::NEAREST );
								texture->setWrap( osg::Texture::WRAP_S, osg::Texture::REPEAT );
								texture->setWrap( osg::Texture::WRAP_T, osg::Texture::REPEAT );
								texture->setWrap( osg::Texture::WRAP_R, osg::Texture::REPEAT );
							}
							else
								osg::notify(osg::NOTICE) << "  Warning: Texture " << t->getUrl() << " not found!" << std::endl;

							settings._textureMap[ t->getUrl() ] = texture;
						}
						else
							texture = settings._textureMap[ t->getUrl() ];

						if ( texture )
						{
							osg::ref_ptr<osg::Vec2Array> tex = new osg::Vec2Array;

							for ( unsigned int i = 0; i < texCoords->size(); i++ )
								tex->push_back( osg::Vec2( (*texCoords)[i].x, (*texCoords)[i].y ) );

							geom->setTexCoordArray( 0, tex );

							stateset->setTextureAttributeAndModes( 0, texture, osg::StateAttribute::ON );

							colorset = true;
						}
					}
					else 
						osg::notify(osg::NOTICE) << "  Warning: Texture coordinates not found for poly " << p->getId() << std::endl;
				}
			}
		
			// Color management

			geom->setColorArray( ( !colorset && geometry.getType() == citygml::GT_Roof ) ? roof_color.get() : shared_colors.get() );

			geom->setColorBinding( osg::Geometry::BIND_OVERALL );
#if 0
			// Set lighting model to two sided
			osg::ref_ptr< osg::LightModel > lightModel = new osg::LightModel;
			lightModel->setTwoSided( true );
			stateset->setAttributeAndModes( lightModel.get(), osg::StateAttribute::ON );
#endif
			// That's it!
			geode->addDrawable( geom );			
		}
	}

	if ( settings._printNames ) 
	{
		// Print the city object name on top of it
		geode->getBoundingBox().center();
		osg::ref_ptr<osgText::Text> text = new osgText::Text;
		text->setFont( "arial.ttf" );
		text->setCharacterSize( 2 );
		text->setBackdropType( osgText::Text::OUTLINE );
		text->setFontResolution( 64, 64 );
		text->setText( object->getId(), osgText::String::ENCODING_UTF8 );
		text->setCharacterSizeMode( osgText::TextBase::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT );
		text->setAxisAlignment( osgText::TextBase::SCREEN );
		text->setAlignment( osgText::TextBase::CENTER_BOTTOM );
		text->setPosition( geode->getBoundingBox().center() + osg::Vec3( 0, 0, geode->getBoundingBox().radius() ) );
		text->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OVERRIDE|osg::StateAttribute::OFF );
		geode->addDrawable( text.get() );
	}

	return geode.release();
}
