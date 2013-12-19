/*
* 
* Copyright (c) 2013, Wieden+Kennedy
* 
* Stephen Schieberl
* Michael Latzoni
*
* All rights reserved.
* 
* Redistribution and use in source and binary forms, with or 
* without modification, are permitted provided that the following 
* conditions are met:
* 
* Redistributions of source code must retain the above copyright 
* notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright 
* notice, this list of conditions and the following disclaimer in 
* the documentation and/or other materials provided with the 
* distribution.
* 
* Neither the name of the Ban the Rewind nor the names of its 
* contributors may be used to endorse or promote products 
* derived from this software without specific prior written 
* permission.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* 
*/

#include "cinder/app/AppBasic.h"
#include "cinder/Camera.h"
#include "cinder/Channel.h"
#include "cinder/gl/Texture.h"
#include "Cinder-OpenNI.h"
#include "cinder/System.h"
#include "OscSender.h"

#define drawCross( img, center, color, d )\
	line(img, Point(center.x - d, center.y - d), Point(center.x + d, center.y + d), color, 2, CV_AA, 0);\
	line(img, Point(center.x + d, center.y - d), Point(center.x - d, center.y + d), color, 2, CV_AA, 0 )\



/* 
 * This application demonstrates how to display NiTE users.
 */
class UserApp : public ci::app::AppBasic 
{

public:
	void						draw();
	void						keyDown( ci::app::KeyEvent event );
	void						prepareSettings( ci::app::AppBasic::Settings* settings );
	void						setup();
	void						update();
private:
	struct Bone
	{
		Bone( nite::JointType a, nite::JointType b )
			: mJointA( a ), mJointB( b )
		{
		}
		nite::JointType			mJointA;
		nite::JointType			mJointB;
	};
	
	ci::CameraPersp				mCamera;

	std::vector<Bone>			mBones;
	ci::Channel16u				mChannel;
	OpenNI::DeviceManagerRef	mDeviceManager;
	OpenNI::DeviceRef			mDevice;
	ci::gl::TextureRef			mTexture;
	std::vector<nite::UserData>	mUsers;
	void						onUser( nite::UserTrackerFrameRef, const OpenNI::DeviceOptions& deviceOptions );

	void						screenShot();

	ci::Vec2f convert_real_world_to_projective(ci::Vec3f point);
	std::vector<ci::Colorf>     mUserClr ;

	//OSC
	ci::osc::Sender sender;
	std::string host;
	int 		port;
};

#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"

using namespace ci;
using namespace ci::app;
using namespace std;

void UserApp::draw()
{
	gl::setViewport( getWindowBounds() );
	gl::clear( Colorf::black() );
	gl::setMatricesWindow( getWindowSize() );

	gl::color( Colorf::white() );
	if ( mChannel ) {
		if ( mTexture ) {
			mTexture->update( Channel32f( mChannel ) );
		} else {
			mTexture = gl::Texture::create( Channel32f( mChannel ) );
		}
		gl::draw( mTexture, mTexture->getBounds(), getWindowBounds() );
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
			gl::begin( GL_LINES );
			for ( vector<Bone>::const_iterator iter = mBones.begin(); iter != mBones.end(); ++iter ) {
				const nite::SkeletonJoint& joint0 = skeleton.getJoint( iter->mJointA );
				const nite::SkeletonJoint& joint1 = skeleton.getJoint( iter->mJointB );

				Vec3f v0 = OpenNI::toVec3f( joint0.getPosition() );
				Vec3f v1 = OpenNI::toVec3f( joint1.getPosition() );
				v0.x = -v0.x;
				v1.x = -v1.x;

				gl::vertex( v0 );
				gl::vertex( v1 );
			}
			gl::end();
		}
	}
}

void UserApp::keyDown( KeyEvent event )
{
	switch ( event.getCode() ) {
	case KeyEvent::KEY_q:
		quit();
		break;
	case KeyEvent::KEY_f:
		setFullScreen( !isFullScreen() );
		break;
	case KeyEvent::KEY_s:
		screenShot();
		break;
	}
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

void UserApp::prepareSettings( Settings* settings )
{
	settings->setFrameRate( 60.0f );
	settings->setWindowSize( 640, 480 );
}

void UserApp::screenShot()
{
	writeImage( getAppPath() / fs::path( "frame" + toString( getElapsedFrames() ) + ".png" ), copyWindowSurface() );
}

Vec2f UserApp::convert_real_world_to_projective(Vec3f point)
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

return Vec2f(x,y);
}

void UserApp::update()
{
	osc::Message messageInit;
	messageInit.setAddress("/cinder/osc/init/");
	messageInit.setRemoteEndpoint(host, port);
	sender.sendMessage(messageInit);


	for ( std::vector<nite::UserData>::const_iterator iter = mUsers.begin(); iter != mUsers.end(); ++iter )
	{
		//if(iter->getCenterOfMass().x != 0.f && iter->getCenterOfMass().y != 0.f &&  iter->getCenterOfMass().z < 2000.f &&  iter->getCenterOfMass().z > 100.f)
		//{
			//ci::Vec2f CenterOfMassProjective =  convert_real_world_to_projective(ci::Vec3f(iter->getCenterOfMass().x, iter->getCenterOfMass().y, iter->getCenterOfMass().z));

			osc::Message message;
			message.addIntArg(iter->getId());
			message.addFloatArg(iter->getCenterOfMass().x);
			message.addFloatArg(iter->getCenterOfMass().y);
			message.addFloatArg(iter->getCenterOfMass().z);
			message.setAddress("/cinder/osc/");
			message.setRemoteEndpoint(host, port);
			sender.sendMessage(message);
			//gl::color(Color(1.f,1.f,0.f));
			//gl::drawStrokedRect( Rectf(iter->getBoundingBox().min.x, iter->getBoundingBox().min.y, iter->getBoundingBox().max.x,  iter->getBoundingBox().max.y ) /*+ Rectf(0.f, 0.f , getWindowWidth() * 0.5f,::getWindowHeight()*0.5f)*/);
		//}
	}
}


void UserApp::setup()
{
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

	mCamera = CameraPersp( getWindowWidth(), getWindowHeight(), 45.0f, 1.0f, 5000.0f );
	mCamera.lookAt( Vec3f::zero(), Vec3f::zAxis(), Vec3f::yAxis() );

	mDeviceManager = OpenNI::DeviceManager::create();
	try {
		mDevice = mDeviceManager->createDevice( OpenNI::DeviceOptions().enableUserTracking() );
	} catch ( OpenNI::ExcDeviceNotAvailable ex ) {
		console() << ex.what() << endl;
		quit();
		return;
	}

	mDevice->getUserTracker().setSkeletonSmoothingFactor( 0.5f );
	mDevice->connectUserEventHandler( &UserApp::onUser, this );
	mDevice->start();

	mUserClr.push_back(Colorf(0.f,1.f,0.f));
	mUserClr.push_back(Colorf(0.f,0.f,1.f));
	mUserClr.push_back(Colorf(1.f,1.f,0.f));
	mUserClr.push_back(Colorf(1.f,0.f,1.f));
	mUserClr.push_back(Colorf(0.f,1.f,1.f));
	mUserClr.push_back(Colorf(1.f,0.5f,0.f));

    //OSC
	port = 3000;
		host = System::getIpAddress();
	if( host.rfind( '.' ) != string::npos )
		host.replace( host.rfind( '.' ) + 1, 3, "255" );
	sender.setup( host, port, true );
}

CINDER_APP_BASIC( UserApp, RendererGl )
