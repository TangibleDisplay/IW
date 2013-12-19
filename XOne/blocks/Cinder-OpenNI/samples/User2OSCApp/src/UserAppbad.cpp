#include "cinder/app/AppBasic.h"
#include "cinder/Camera.h"
#include "cinder/Channel.h"
#include "cinder/gl/Texture.h"

#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"


#include "cinder/Utilities.h"
//#include "cinder/params/Params.h"

//#include "CinderOpenCv.h"
#include "Cinder-OpenNI.h"



//using namespace cv;

#define drawCross( img, center, color, d )\
	line(img, Point(center.x - d, center.y - d), Point(center.x + d, center.y + d), color, 2, CV_AA, 0);\
	line(img, Point(center.x + d, center.y - d), Point(center.x - d, center.y + d), color, 2, CV_AA, 0 )\


class UserApp : ci::app::AppBasic  {
  public:
	void setup();
	void update();
	void draw();
	void prepareSettings( Settings *settings );
	/*
	void shutdown();
	void resize();
	void mouseMove( MouseEvent event );	
	void mouseDown( MouseEvent event );	
	void mouseDrag( MouseEvent event );	
	void mouseUp( MouseEvent event );	*/
	
	void keyDown( ci::app::KeyEvent event );
private:
	OpenNI::DeviceRef			mDevice;
	OpenNI::DeviceManagerRef	mDeviceManager;
	void onColor( openni::VideoFrameRef frame, const OpenNI::DeviceOptions& deviceOptions );
	void onDepth( openni::VideoFrameRef frame, const OpenNI::DeviceOptions& deviceOptions );
	void onUser( nite::UserTrackerFrameRef, const OpenNI::DeviceOptions& deviceOptions );
	//void OpticalFow(ci::Surface8u in);
	ci::Vec2f convert_real_world_to_projective(ci::Vec3f point);
	ci::gl::TextureRef	mTexture;
	ci::gl::TextureRef mTextureDepth;
	ci::Surface8u				mSurface;
	ci::Channel16u				mChannel;
	//params::InterfaceGlRef	mParams;
	std::vector<nite::UserData>	mUsers;

	struct Bone
	{
		Bone( nite::JointType a, nite::JointType b )
			: mJointA( a ), mJointB( b )
		{
		}
		nite::JointType			mJointA;
		nite::JointType			mJointB;
	};

	std::vector<Bone>			mBones;
	ci::CameraPersp				mCamera;

	std::vector<ci::Colorf>     mUserClr ;
	ci::gl::TextureRef			mXBoxTextureRef;
	ci::Vec3f					mPosText;


};

using namespace ci;
using namespace ci::app;
using namespace std;


void UserApp::prepareSettings( Settings *settings )
{
	//settings->setResizable( true );
	settings->setWindowSize( 800, 600 ); // tout les coordonnée sont en résolution 1920.f 1080.f comme le PSD
	settings->setFrameRate(60.f);
	//settings->setFullScreen();
	//settings->enableMultiTouch();
	//settings->enableConsoleWindow();
}

void UserApp::keyDown( KeyEvent event )
{
	// warp editor did not handle the key, so handle it here
	switch( event.getCode() )
	{
	case KeyEvent::KEY_ESCAPE:
		// quit the application
		quit();
		break;
	case KeyEvent::KEY_f:
		// toggle full screen
		setFullScreen( ! isFullScreen() );
		break;
	}
}

void UserApp::setup()
{
	// The included image is copyright Trey Ratcliff
	// http://www.flickr.com/photos/stuckincustoms/4045813826/
	/*
	ci::Surface8u surface( loadImage( loadAsset( "dfw.jpg" ) ) );
	cv::Mat input( toOcv( surface ) );
	cv::Mat output;

	cv::medianBlur( input, output, 11 );
//	cv::Sobel( input, output, CV_8U, 0, 1 ); 
//	cv::threshold( input, output, 128, 255, CV_8U );

	mTexture = gl::Texture( fromOcv( output ) );
	*/
		// The device manager automatically initializes OpenNI and NiTE.
	// It's a good idea to check that initialization is complete
	// before accessing devices.

	//namedWindow( "Source", 1 );
	//cv::namedWindow( "Components", 1 );


	mBones.push_back( Bone( nite::JOINT_HEAD,			nite::JOINT_NECK ) );
	mBones.push_back( Bone( nite::JOINT_LEFT_SHOULDER,	nite::JOINT_LEFT_ELBOW ) );
	mBones.push_back( Bone( nite::JOINT_LEFT_ELBOW,		nite::JOINT_LEFT_HAND ) );
	mBones.push_back( Bone( nite::JOINT_RIGHT_SHOULDER, nite::JOINT_RIGHT_ELBOW ) );
	mBones.push_back( Bone( nite::JOINT_RIGHT_ELBOW,	nite::JOINT_RIGHT_HAND ) );
	mBones.push_back( Bone( nite::JOINT_LEFT_SHOULDER,	nite::JOINT_RIGHT_SHOULDER ) );
	mBones.push_back( Bone( nite::JOINT_LEFT_SHOULDER,	nite::JOINT_TORSO ) );
	mBones.push_back( Bone( nite::JOINT_RIGHT_SHOULDER, nite::JOINT_TORSO ) );
	mBones.push_back( Bone( nite::JOINT_TORSO,			nite::JOINT_LEFT_HIP ) );
	mBones.push_back( Bone( nite::JOINT_TORSO,			nite::JOINT_RIGHT_HIP ) );
	mBones.push_back( Bone( nite::JOINT_LEFT_HIP,		nite::JOINT_RIGHT_HIP ) );
	mBones.push_back( Bone( nite::JOINT_LEFT_HIP,		nite::JOINT_LEFT_KNEE ) );
	mBones.push_back( Bone( nite::JOINT_LEFT_KNEE,		nite::JOINT_LEFT_FOOT ) );
	mBones.push_back( Bone( nite::JOINT_RIGHT_HIP,		nite::JOINT_RIGHT_KNEE ) );
	mBones.push_back( Bone( nite::JOINT_RIGHT_KNEE,		nite::JOINT_RIGHT_FOOT ) );

	mUserClr.push_back(Colorf(0.f,1.f,0.f));
	mUserClr.push_back(Colorf(0.f,0.f,1.f));
	mUserClr.push_back(Colorf(1.f,1.f,0.f));
	mUserClr.push_back(Colorf(1.f,0.f,1.f));
	mUserClr.push_back(Colorf(0.f,1.f,1.f));
	mUserClr.push_back(Colorf(1.f,0.5f,0.f));


	mDeviceManager		= OpenNI::DeviceManager::create();

	//while ( !mDeviceManager->isInitialized() ) {
	//	Sleep(100);
	//}

		// Catching this exception prevents our application
		// from crashing when no devices are connected.
		try {
			// Because OpenNI works with several devices which do not have RGB cameras, 
			// the color stream is disabled by default. Enable color in DeviceOptions
			// to turn it on.
			mDevice = mDeviceManager->createDevice(OpenNI::DeviceOptions().enableHandTracking() ); // OpenNI::DeviceOptions()/*.enableColor()*/.enableDepth().enableUserTracking()
		} catch ( OpenNI::ExcDeviceNotFound ex ) {
			console() << ex.what() << endl;
			quit();
			return;
		} catch ( OpenNI::ExcDeviceNotAvailable ex ) {
			console() << ex.what() << endl;
			quit();
			return;
		}
		
		//if ( mDevice ) 
		//{
			// If we've successfully accessed a device, start and add a 
			// callback for the color stream.
			//mDevice->connectColorEventHandler( &XOneApp::onColor, this );

			// If we've successfully accessed a device, start and add a 
			// callback for the depth stream.
			//mDevice->connectDepthEventHandler( &XOneApp::onDepth, this );

			//user handler
			mDevice->getUserTracker().setSkeletonSmoothingFactor( 0.5f );
			mDevice->connectUserEventHandler( &UserApp::onUser, this );

			//start device
			mDevice->start();

		//}
	mCamera = CameraPersp( getWindowWidth(), getWindowHeight(), 45.0f, 1.0f, 5000.0f );
	mCamera.lookAt( ci::Vec3f::zero(), ci::Vec3f::zAxis(), ci::Vec3f::yAxis() );
	//}

	// params
	/*mParams = params::InterfaceGl::create( "Réglages", ci::Vec2i( 280, 200 ) );
	mParams->setOptions( "", "position='" + ci::toString(50) + " " + ci::toString(400) + "'" );
	mParams->addParam( "Position logo", &mPosText);

	mParams->addParam( "Scale logo X", &mScaleLogoX, "step=0.01f" ); // min=0.f max=20.f 
	mParams->addParam( "Scale logo Y", &mScaleLogoY, "step=0.01f" ); // min=0.f max=20.f 
	*/
}   

void UserApp::onUser( nite::UserTrackerFrameRef frame, const OpenNI::DeviceOptions& deviceOptions )
{
	mChannel	= OpenNI::toChannel16u( frame.getDepthFrame() ); 
	mUsers		= OpenNI::toVector( frame.getUsers() );
	for ( vector<nite::UserData>::iterator iter = mUsers.begin(); iter != mUsers.end(); ++iter ) {
		if ( iter->isNew() ) {
			mDevice->getUserTracker().startSkeletonTracking( iter->getId() );
		} else if ( iter->isLost() ) {
			mDevice->getUserTracker().stopSkeletonTracking( iter->getId() );
		}
	}
}

void UserApp::onColor( openni::VideoFrameRef frame, const OpenNI::DeviceOptions& deviceOptions )
{
	mSurface = OpenNI::toSurface8u( frame );
    //BlobDetection(mSurface);
}

void UserApp::onDepth( openni::VideoFrameRef frame, const OpenNI::DeviceOptions& deviceOptions )
{
	mChannel = OpenNI::toChannel16u( frame );
}

void UserApp::update()
{
}
/*
ci::Vec3f XOneApp::convert_projective_to_real_world(cv::Mat1f depth, ci::Vec2f point) 
{

	double FovH = 1.0122909661567111546157406457234;
	double FovV = 0.78539816339744830961566084581988; // Values for Asus Xtion Pro

	cv::Point3d result;

	result.x = ((point.x/640.0)-0.5)*depth(point.y,point.x)*(tan(FovH/2.0)*2);
	result.y = (0.5-(point.y/480.0))*depth(point.y,point.x)*(tan(FovV/2.0)*2);
	result.z = depth(point.y,point.x);

	return result;

}*/

ci::Vec2f UserApp::convert_real_world_to_projective(ci::Vec3f point)
{
	double FovH = 1.0122909661567111546157406457234;
	double FovV = 0.78539816339744830961566084581988;

	double XtoZ = tan(FovH / 2.0) * 2;
	double YtoZ = tan(FovV / 2.0) * 2;

	double Z = point.z;

	int x = -1;
	int y = -1;

	if (Z != 0){
	x = ((point.x/(Z*XtoZ)) + 0.5) * 640;
	y = (0.5 - point.y/(Z*YtoZ)) * 480;
	}

return ci::Vec2f(x,y);
}
void UserApp::draw()
{
	gl::setViewport( getWindowBounds() );
	gl::clear( Colorf::black() );
	gl::setMatricesWindow( getWindowSize() );
	//gl::enableAlphaBlending();
	//gl::color(ColorA(1.0,1.0,1.0,1.0));
	
	//gl::draw( mTexture );	
	/*
	gl::color(Color::white());
	if ( mSurface ) {
		if ( mTexture ) {
			mTexture->update( mSurface );
		} else {
			mTexture = gl::Texture::create( mSurface );
		}
	}
	gl::draw( mTexture, mTexture->getBounds(), getWindowBounds() );
	*/
	
	gl::color(Color::white());
	if ( mChannel ) 
	{
		if ( mTextureDepth ) {
			mTextureDepth->update( Channel32f( mChannel ) );
		} else {
			mTextureDepth = gl::Texture::create( Channel32f( mChannel ) );
		}
		gl::draw( mTextureDepth, mTextureDepth->getBounds(), getWindowBounds() );
	}
	
	for ( std::vector<nite::UserData>::const_iterator iter = mUsers.begin(); iter != mUsers.end(); ++iter )
	{
		//if(iter->getCenterOfMass().x != 0.f && iter->getCenterOfMass().y != 0.f &&  iter->getCenterOfMass().z < 2000.f &&  iter->getCenterOfMass().z > 100.f)
		//{
			gl::lineWidth(1.f);
			gl::color(Colorf(mUserClr.at(iter->getId() % mUserClr.size())));
			ci::Vec2f CenterOfMassProjective =  convert_real_world_to_projective(ci::Vec3f(iter->getCenterOfMass().x, iter->getCenterOfMass().y, iter->getCenterOfMass().z));
			gl::drawSolidCircle(CenterOfMassProjective /*+ getWindowSize()*0.5f*/, 20.f);
			//gl::color(Color(1.f,1.f,0.f));
			//gl::drawStrokedRect( Rectf(iter->getBoundingBox().min.x, iter->getBoundingBox().min.y, iter->getBoundingBox().max.x,  iter->getBoundingBox().max.y ) /*+ Rectf(0.f, 0.f , getWindowWidth() * 0.5f,::getWindowHeight()*0.5f)*/);
		//}
	}



	gl::setMatrices( mCamera );
	gl::color( Colorf( 1.0f, 0.0f, 0.0f ) );

	for ( std::vector<nite::UserData>::const_iterator iter = mUsers.begin(); iter != mUsers.end(); ++iter ) {
		const nite::Skeleton& skeleton = iter->getSkeleton();
		if ( skeleton.getState() == nite::SKELETON_TRACKED ) {
			gl::color(Colorf(mUserClr.at(iter->getId() % mUserClr.size())));
			gl::lineWidth(5.f);
			gl::begin( GL_LINES );
			for ( vector<Bone>::const_iterator iter = mBones.begin(); iter != mBones.end(); ++iter ) {
				const nite::SkeletonJoint& joint0 = skeleton.getJoint( iter->mJointA );
				const nite::SkeletonJoint& joint1 = skeleton.getJoint( iter->mJointB );

				ci::Vec3f v0 = OpenNI::toVec3f( joint0.getPosition() );
				ci::Vec3f v1 = OpenNI::toVec3f( joint1.getPosition() );
				v0.x = -v0.x;
				v1.x = -v1.x;

				gl::vertex( v0 );
				gl::vertex( v1 );
			}
			gl::end();
		}
	}
	//if(mParams) mParams->draw();
}

CINDER_APP_BASIC( UserApp, RendererGl )

