#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"

#include "CinderOpenCv.h"

using namespace ci;
using namespace ci::app;

class BeatApp : public AppNative {
  public:
    void prepareSettings( Settings *settings );
	void setup();
	void draw();
	
	gl::Texture	mTexture;
};

void BeatApp::prepareSettings( Settings *settings ){
    settings->setWindowSize( 1920, 1080 );
    settings->setFrameRate( 60.0f );
}

void BeatApp::setup()
{
	ci::Surface8u surface( loadImage( loadAsset( "test.jpg" ) ) );
	cv::Mat input( toOcv( surface ) );
	cv::Mat output;

	mTexture = gl::Texture( fromOcv( input ) );
}   

void BeatApp::draw()
{
	gl::clear();
	gl::draw( mTexture );
}

CINDER_APP_NATIVE( BeatApp, RendererGl )
