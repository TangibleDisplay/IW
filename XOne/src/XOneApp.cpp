#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
#include "cinder/Camera.h"
#include "cinder/Channel.h"
#include "cinder/params/Params.h"
#include "cinder/Utilities.h"
#include "cinder/ObjLoader.h"
#include "cinder/MayaCamUI.h"
#include "cinder/Sphere.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Json.h"

#include "WarpBilinear.h"
#include "WarpPerspective.h"
#include "WarpPerspectiveBilinear.h"


#include "Gizmo.h"
#include "Interaction.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace ph::warping;

#define drawCross( img, center, color, d )\
	line(img, Point(center.x - d, center.y - d), Point(center.x + d, center.y + d), color, 2, CV_AA, 0);\
	line(img, Point(center.x + d, center.y - d), Point(center.x - d, center.y + d), color, 2, CV_AA, 0 )\

class XOneApp : public AppNative {
  public:
	void setup();
	void update();
	void draw();
	void prepareSettings( Settings *settings );
	void shutdown();
	void resize();
	void mouseMove( MouseEvent event );	
	void mouseDown( MouseEvent event );	
	void mouseDrag( MouseEvent event );	
	void mouseUp( MouseEvent event );	
	
	void keyDown( KeyEvent event );
	void keyUp( KeyEvent event );
	void frameCurrentObject();
	bool readJSONInit( const string &path );
private:
	//ci::Vec2f convert_real_world_to_projective(ci::Vec3f point);
	gl::TextureRef	mTexture;
	gl::TextureRef mTextureDepth;
	ci::Surface8u				mSurface;
	ci::Channel16u				mChannel;
	params::InterfaceGlRef	mParams;
	ci::CameraPersp				mCamera;

	//std::vector<ci::Colorf>     mUserClr ;
	ci::gl::TextureRef			mXBoxTextureRef;
	ci::gl::TextureRef			mXBoxSanBouleTextureRef;

	WarpList					mWarps;
	bool						mUseBeginEnd;
	float						mScaleLogoX;
	float						mScaleLogoY;
	//osc::Listener 				listener;
/*
	struct User
	{
		User() {};
		User(int id, float centerMassX, float centerMassY, float centerMassZ):mId(id), mCenterMassX(centerMassX), mCenterMassY(centerMassY), mCenterMassZ(centerMassZ) {}
		int mId;
		float mCenterMassX;
		float mCenterMassY;
		float mCenterMassZ;
	};

	map<int, User> mUsers;
	*/
	TriMesh			mMesh;
	gl::VboMesh		mVBO;
	Vec2f			mMousePos;
	Vec2f			mMouseDownPos;
	Vec3f			mPosText;
	Vec2f			mTranslaleAllObj;
	Vec2f			mControlRotate1;
	Vec2f			mControlRotate2;
	Vec2f			mControlRotate3;
	Vec2f			mControlRotate4;
	Vec2f			mControlRotate5;
	Vec2f			mTranslate1;
	Vec2f			mTranslate2;
	Vec2f			mTranslate3;
	Vec2f			mTranslate4;
	Vec2f			mTranslate5;
	float			mTranslateZ;
	float			mScale1;
	float			mScale2;
	float			mScale3;
	float			mScale4;
	float			mScale5;

	MayaCamUI		mMayaCam;
	CameraOrtho		mOrthoCam;

	Matrix44f       mTransform;
	gl::GlslProg	mShader;
	gl::Texture		mTextureObj;
	//GizmoRef        mGizmo;
	bool			mSaveParam;
	bool			mDisplayInfos;

	Interaction		mInteraction;
	Vec3f			mAngleInitObj;
	std::string			mtextScenario;

	bool mDisplayPositionObjets;

};
void XOneApp::prepareSettings( Settings *settings )
{
	settings->setResizable( true );
	settings->setWindowSize( 1280, 768 ); // tout les coordonnée sont en résolution 1920.f 1080.f comme le PSD
	settings->setFrameRate(60.f);
	settings->setFullScreen();
	//settings->enableMultiTouch();
	//settings->enableConsoleWindow();
}

bool XOneApp::readJSONInit( const std::string &path )
{
	try 
	{
		string pathToFile = ( getAssetPath("") / path ).string();
		if ( !fs::exists( pathToFile ) ) 
		{
			console()<<("fichier json non trouvé: " + path); 
			return false;
		}
		JsonTree json( loadFile( pathToFile ) );
		JsonTree visu = json.getChild( "config.visu" ); 
		JsonTree interaction = json.getChild( "config.interaction" ); 

		//visu
		mScaleLogoX = visu.getChild( "mScaleLogoX" ).getValue<float>();//0.40f;
		mScaleLogoY = visu.getChild( "mScaleLogoY" ).getValue<float>();0.91f; 
		mPosText = Vec3f(visu.getChild( "mPosText" ).getChild( "x").getValue<float>(), visu.getChild( "mPosText" ).getChild( "y").getValue<float>(), visu.getChild( "mPosText" ).getChild( "z").getValue<float>());//Vec3f(0.f, -100.f, 0.f); 
		mTranslaleAllObj = Vec2f(visu.getChild( "mTranslaleAllObj" ).getChild( "x").getValue<float>(), visu.getChild( "mTranslaleAllObj" ).getChild( "y").getValue<float>());//Vec2f(0.03f, 0.06f);
		mTranslate1 = Vec2f(visu.getChild( "mTranslate1" ).getChild( "x").getValue<float>(), visu.getChild( "mTranslate1" ).getChild( "y").getValue<float>());//Vec2f(-1.f, -0.23f);
		mTranslate2 = Vec2f(visu.getChild( "mTranslate2" ).getChild( "x").getValue<float>(), visu.getChild( "mTranslate2" ).getChild( "y").getValue<float>());//Vec2f(-0.49f, -0.23f);
		mTranslate3 = Vec2f(visu.getChild( "mTranslate3" ).getChild( "x").getValue<float>(), visu.getChild( "mTranslate3" ).getChild( "y").getValue<float>());//Vec2f(0.01f, -0.23f);
		mTranslate4 = Vec2f(visu.getChild( "mTranslate4" ).getChild( "x").getValue<float>(), visu.getChild( "mTranslate4" ).getChild( "y").getValue<float>());//Vec2f(0.48f, -0.23f);
		mTranslate5 = Vec2f(visu.getChild( "mTranslate5" ).getChild( "x").getValue<float>(), visu.getChild( "mTranslate5" ).getChild( "y").getValue<float>());//Vec2f(0.96f, -0.23f);
		mTranslateZ = visu.getChild( "mTranslateZ" ).getValue<float>();//0.f;
		mScale1 = visu.getChild( "mScale1" ).getValue<float>();//0.11f;
		mScale2 = visu.getChild( "mScale2" ).getValue<float>();//0.11f;
		mScale3 = visu.getChild( "mScale3" ).getValue<float>();//0.11f;
		mScale4 = visu.getChild( "mScale4" ).getValue<float>();//0.11f;
		mScale5 = visu.getChild( "mScale5" ).getValue<float>();//0.11f;
		mSaveParam = visu.getChild( "mSaveParam" ).getValue<bool>();//true;
		mDisplayInfos = visu.getChild( "mDisplayInfos" ).getValue<bool>();//false;
		mAngleInitObj = Vec3f(visu.getChild( "mAngleInitObj" ).getChild( "x").getValue<float>(), visu.getChild( "mAngleInitObj" ).getChild( "y").getValue<float>(), visu.getChild( "mAngleInitObj" ).getChild( "z").getValue<float>());//Vec3f(-45.0f, 45.f, 0.0f);
		
		//interaction
		mInteraction.mOffset = Vec2f(interaction.getChild( "mOffset" ).getChild( "x").getValue<float>(), interaction.getChild( "mOffset" ).getChild( "y").getValue<float>());// Vec2f(0.f, 0.f);
		mInteraction.mFactor = Vec2f(interaction.getChild( "mFactor" ).getChild( "x").getValue<float>(), interaction.getChild( "mFactor" ).getChild( "y").getValue<float>());//Vec2f(10.f, -1.f);
		mInteraction.mOffsetControl = Vec2f(interaction.getChild( "mOffsetControl" ).getChild( "x").getValue<float>(), interaction.getChild( "mOffsetControl" ).getChild( "y").getValue<float>());//Vec2f(0.f, 0.f);
		mInteraction.mFactorControl = Vec2f(interaction.getChild( "mFactorControl" ).getChild( "x").getValue<float>(), interaction.getChild( "mFactorControl" ).getChild( "y").getValue<float>());//Vec2f(20.f, -20.f);
		mInteraction.mCubeInteraction = Vec3f(interaction.getChild( "mCubeInteraction" ).getChild( "x").getValue<float>(), interaction.getChild( "mCubeInteraction" ).getChild( "y").getValue<float>(), interaction.getChild( "mCubeInteraction" ).getChild( "z").getValue<float>());// Vec3f(300.f, 300.f,500.f);
		mInteraction.mOffsetLimitScreenZ = interaction.getChild( "mOffsetLimitScreenZ" ).getValue<float>();//1500.f;
		mInteraction.mMoyDetectMovementProjectiveX = interaction.getChild( "mMoyDetectMovementProjectiveX" ).getValue<float>();//15.f;
		mInteraction.mMoyDetectMovementZ = interaction.getChild( "mMoyDetectMovementZ" ).getValue<float>();//120.f;
		mInteraction.mZLimit = interaction.getChild( "mZLimit" ).getValue<float>();//2600.f;
		mInteraction.mDefautRotation = Vec2f(interaction.getChild( "mDefautRotation" ).getChild( "x").getValue<float>(), interaction.getChild( "mDefautRotation" ).getChild( "y").getValue<float>());//
	
	} 
	catch ( ... ) 
	{
		console()<<"Erreur sur fichier JSON: " + path<<endl;
		return false;
	}
	return true;
}


void XOneApp::shutdown()
{
	// save warp settings
	fs::path settings = getAssetPath("") / "warps.xml";
	Warp::writeSettings( mWarps, writeFile( settings ) );
}

void XOneApp::resize()
{
	// tell the warps our window has been resized, so they properly scale up or down
	Warp::handleResize( mWarps );

	//mGizmo->resize();
}

void XOneApp::mouseMove( MouseEvent event )
{
	// pass this mouse event to the warp editor first
	if( ! Warp::handleMouseMove( mWarps, event ) )
	{
		// let your application perform its mouseMove handling here
	}
	//mGizmo->mouseMove(event);
}

void XOneApp::mouseDown( MouseEvent event )
{
	// pass this mouse event to the warp editor first
	if( ! Warp::handleMouseDown( mWarps, event ) )
	{
		// let your application perform its mouseDown handling here
	}
	mMouseDownPos =  event.getPos();

	//mGizmo->mouseDown(event);
}

void XOneApp::mouseDrag( MouseEvent event )
{
	// pass this mouse event to the warp editor first
	if( ! Warp::handleMouseDrag( mWarps, event ) )
	{
		// let your application perform its mouseDrag handling here
	}
	mMousePos = event.getPos() - mMouseDownPos;
	//mGizmo->mouseDrag(event);
	           
	//mTransform.translate( Vec3f(0,-200,0) );      // position is a Vec3f
	//mTransform.rotate( Vec3f::xAxis(), mMousePos.x*M_PI/180.f );
	//mTransform.rotate( Vec3f::zAxis(), mMousePos.y*M_PI/180.f );
	            // scale is a Vec3f
	//mTransform.translate(-1.0f * Vec3f(-0.9f, 4.2f, .0f)  );        // pivot is a Vec3f
}

void XOneApp::mouseUp( MouseEvent event )
{
	// pass this mouse event to the warp editor first
	if( ! Warp::handleMouseUp( mWarps, event ) )
	{
		// let your application perform its mouseUp handling here
	}
}

void XOneApp::keyDown( KeyEvent event )
{
	// pass this key event to the warp editor first
	if( ! Warp::handleKeyDown( mWarps, event ) )
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
			mTextureObj.bind();
			mShader.bind();
			mShader.uniform( "tex0", 0 );
			break;
		case KeyEvent::KEY_w:
			// toggle warp edit mode
			Warp::enableEditMode( ! Warp::isEditModeEnabled() );
			break;
		case KeyEvent::KEY_SPACE:
			// toggle drawing mode
			mUseBeginEnd = !mUseBeginEnd;
			break;
		case  KeyEvent::KEY_l:
			frameCurrentObject();
			break;
		case KeyEvent::KEY_1: 
			//mGizmo->setMode( Gizmo::TRANSLATE );
			break;
		case KeyEvent::KEY_2: 
			//mGizmo->setMode( Gizmo::ROTATE );
			break;
		case KeyEvent::KEY_3: 
			//mGizmo->setMode( Gizmo::SCALE );
			break;
		case KeyEvent::KEY_o:  
		{
			//CameraPersp centered = mMayaCam.getCamera();
			//centered.setCenterOfInterestPoint( mGizmo->getTranslate() );
			//mMayaCam.setCurrentCam( centered );
			break;
		}
		case KeyEvent::KEY_i: 
			mDisplayInfos = !mDisplayInfos; 
			break;
		}
	}
}

void XOneApp::frameCurrentObject()
{
	Sphere boundingSphere = Sphere::calculateBoundingSphere( mMesh.getVertices() );
	
	mMayaCam.setCurrentCam( mMayaCam.getCamera().getFrameSphere( boundingSphere, 100 ) );
}

void XOneApp::keyUp( KeyEvent event )
{
	// pass this key event to the warp editor first
	if( ! Warp::handleKeyUp( mWarps, event ) )
	{
		// let your application perform its keyUp handling here
	}
}
void XOneApp::setup()
{
	//read JSON
	readJSONInit("init.json");


	try 
	{
		mXBoxTextureRef = gl::Texture::create(loadImage(loadAsset("logo_def_xb1_hotel_avec_boules_gros_trait.png")));// logo_def_xb1_hotel_sans_logo.png  //logo_def_xb1_hotel.png
	} 
	catch(...) 
	{ 
		console()<<"unable to la TextureRef logo_def_xb1_hotel.png"<<endl;
	}

	try 
	{
		mXBoxSanBouleTextureRef = gl::Texture::create(loadImage(loadAsset("logo_def_xb1_hotel_sans_boules_gros_trait.png"))); //logo_def_xb1_hotel_sans boule.png
	} 
	catch(...) 
	{ 
		console()<<"unable to la TextureRef logo_def_xb1_hotel_sans boule.png"<<endl;
	}
/*
	mUserClr.push_back(Colorf(0.f,1.f,0.f));
	mUserClr.push_back(Colorf(0.f,0.f,1.f));
	mUserClr.push_back(Colorf(1.f,1.f,0.f));
	mUserClr.push_back(Colorf(1.f,0.f,1.f));
	mUserClr.push_back(Colorf(0.f,1.f,1.f));
	mUserClr.push_back(Colorf(1.f,0.5f,0.f));
*/

	mCamera = CameraPersp( getWindowWidth(), getWindowHeight(), 45.0f, 1.0f, 5000.0f );
	mCamera.lookAt( ci::Vec3f::zero(), ci::Vec3f::zAxis(), ci::Vec3f::yAxis() );

	CameraPersp initialCam;
	initialCam.setPerspective( 1.0f, mXBoxTextureRef->getAspectRatio() /*getWindowAspectRatio()*/, 0.1, 10000 );
	mMayaCam.setCurrentCam( initialCam );


	mOrthoCam.setOrtho( 0, getWindowWidth(), getWindowHeight(), 0,  -1, 1);
	//mOrthoCam.setOrtho(-100, 100, -100, 100, -1, 1 );
	

	//}

	// params
	mParams = params::InterfaceGl::create( "Réglages", ci::Vec2i( 280, 200 ) );
	mParams->setOptions( "", "position='" + boost::lexical_cast<string>(50) + " " + boost::lexical_cast<string>(400) + "'" );

	mParams->addParam( "SCENARIO", &mtextScenario);

	mParams->addParam( "Position logo", &mPosText);

	mParams->addParam( "Position all OBJ x", &mTranslaleAllObj.x, "step=0.01f");
	mParams->addParam( "Position all OBJ y", &mTranslaleAllObj.y, "step=0.01f");

	mParams->addParam( "Scale logo X", &mScaleLogoX, "step=0.01f" ); // min=0.f max=20.f 
	mParams->addParam( "Scale logo Y", &mScaleLogoY, "step=0.01f" ); // min=0.f max=20.f

	mParams->addParam( "Rotate OBJ 1 X", &mControlRotate1.x, "step=0.002f" ); // min=0.f max=20.f 
	mParams->addParam( "Rotate OBJ 1 Y", &mControlRotate1.y, "step=0.002f" ); // min=0.f max=20.f 	

	mParams->addParam( "Rotate OBJ 2 X", &mControlRotate2.x, "step=0.002f" ); // min=0.f max=20.f 
	mParams->addParam( "Rotate OBJ 2 Y", &mControlRotate2.y, "step=0.002f" ); // min=0.f max=20.f 

	mParams->addParam( "Rotate OBJ 3 X", &mControlRotate3.x, "step=0.002f" ); // min=0.f max=20.f 
	mParams->addParam( "Rotate OBJ 3 Y", &mControlRotate3.y, "step=0.002f" ); // min=0.f max=20.f 

	mParams->addParam( "Rotate OBJ 4 X", &mControlRotate4.x, "step=0.002f" ); // min=0.f max=20.f 
	mParams->addParam( "Rotate OBJ 4 Y", &mControlRotate4.y, "step=0.002f" ); // min=0.f max=20.f

	mParams->addParam( "Rotate OBJ 5 X", &mControlRotate5.x, "step=0.002f" ); // min=0.f max=20.f 
	mParams->addParam( "Rotate OBJ 5 Y", &mControlRotate5.y, "step=0.001f" ); // min=0.f max=20.f 	

	mParams->addParam( "Translate OBJ 1 X", &mTranslate1.x, "step=0.002f" ); // min=0.f max=20.f 
	mParams->addParam( "Translate OBJ 1 Y", &mTranslate1.y, "step=0.002f" ); // min=0.f max=20.f 

	mParams->addParam( "Translate OBJ 2 X", &mTranslate2.x, "step=0.002f" ); // min=0.f max=20.f 
	mParams->addParam( "Translate OBJ 2 Y", &mTranslate2.y, "step=0.002f" ); // min=0.f max=20.f 

	mParams->addParam( "Translate OBJ 3 X", &mTranslate3.x, "step=0.002f" ); // min=0.f max=20.f 
	mParams->addParam( "Translate OBJ 3 Y", &mTranslate3.y, "step=0.002f" ); // min=0.f max=20.f 

	mParams->addParam( "Translate OBJ 4 X", &mTranslate4.x, "step=0.002f" ); // min=0.f max=20.f 
	mParams->addParam( "Translate OBJ 4 Y", &mTranslate4.y, "step=0.002f" ); // min=0.f max=20.f 

	mParams->addParam( "Translate OBJ 5 X", &mTranslate5.x, "step=0.002f" ); // min=0.f max=20.f 
	mParams->addParam( "Translate OBJ 5 Y", &mTranslate5.y, "step=0.002f" ); // min=0.f max=20.f 

	mParams->addParam( "Translate z all OBJ", &mTranslateZ, "step=0.01f" );

	mParams->addParam( "Scale OBJ 1", &mScale1, "step=0.002f" ); // min=0.f max=20.f
	mParams->addParam( "Scale OBJ 2", &mScale2, "step=0.002f" ); // min=0.f max=20.f 
	mParams->addParam( "Scale OBJ 3", &mScale3, "step=0.002f" ); // min=0.f max=20.f 
	mParams->addParam( "Scale OBJ 4", &mScale4, "step=0.002f" ); // min=0.f max=20.f 
	mParams->addParam( "Scale OBJ 5", &mScale5, "step=0.002f" ); // min=0.f max=20.f 

	mParams->addParam( "Pos centre of mass Main xtion", &mInteraction.mCenterOfMassMain, "step=0.01f" );

	mParams->addParam( "Angle init OBJ", &mAngleInitObj, "step=0.01f" );

	mParams->addParam( "Position Begin User LEFT", &mInteraction.mPositionBeginPerUser[Interaction::Position::LEFT] );
	mParams->addParam( "Position Begin User RIGHT", &mInteraction.mPositionBeginPerUser[Interaction::Position::RIGHT] );
	
	mParams->addParam( "Display infos", &mDisplayInfos );
	mParams->addParam( "Display position objets", &mDisplayPositionObjets );
	mParams->addParam( "Save params to XML", &mSaveParam );
	
	mDisplayPositionObjets = false;
	//init value
	/*
	mScaleLogoX = 0.40f; //0.64f;
	mScaleLogoY = 0.91f; //0.65f;
	mPosText = Vec3f(0.f, -100.f, 0.f); //ci::Vec3f(-24.74f, 323.24f,0.f);
	mTranslaleAllObj = Vec2f(0.03f, 0.06f);
	mControlRotate1 = Vec2f();
	mControlRotate2 = Vec2f();
	mControlRotate3 = Vec2f();
	mControlRotate4 = Vec2f();
	mControlRotate5 = Vec2f();
	mTranslate1 = Vec2f(-1.f, -0.23f);
	mTranslate2 = Vec2f(-0.49f, -0.23f);
	mTranslate3 = Vec2f(0.01f, -0.23f);
	mTranslate4 = Vec2f(0.48f, -0.23f);
	mTranslate5 = Vec2f(0.96f, -0.23f);
	mTranslateZ = 0.f;
	mScale1 = 0.11f;
	mScale2 = 0.11f;
	mScale3 = 0.11f;
	mScale4 = 0.11f;
	mScale5 = 0.11f;
	mSaveParam = true;
	mDisplayInfos = false;
	mAngleInitObj = Vec3f(-45.0f, 45.f, 0.0f);
	*/

	// initialize warps
	fs::path settings = getAssetPath("") / "warps.xml";
	if( fs::exists( settings ) )
	{
		// load warp settings from file if one exists
		mWarps = Warp::readSettings( loadFile( settings ) );
	}
	else
	{
		// otherwise create a warp from scratch
		mWarps.push_back( WarpPerspectiveBilinearRef( new WarpPerspectiveBilinear() ) );
		//mWarps.push_back( WarpPerspectiveRef( new WarpPerspective() ) );
		//mWarps.push_back( WarpBilinearRef( new WarpBilinear() ) );
	}
			// adjust the content size of the warps
	Warp::setSize( mWarps, mXBoxTextureRef->getSize() /*getWindowSize()*/ );
	mUseBeginEnd = true;

	//listener.setup( 3000 );
	//Interaction
	mInteraction.setup();

	//Objet
	ObjLoader loader( (DataSourceRef)loadAsset( "EXPORTMODIF01.obj" ) ); //xoneV0.obj  xboxSphere.obj  boulev3.obj
	loader.load( &mMesh );
	mVBO = gl::VboMesh( mMesh );

	mTextureObj = gl::Texture( loadImage( loadAsset( "logoxbox.png" ) ) );
	mShader = gl::GlslProg( loadAsset( "shader.vert" ), loadAsset( "shader.frag" ) );

	/*
	mTextureObj.bind();
	mShader.bind();
	mShader.uniform( "tex0", 0 );
	*/
	// Create a reference to our gizmo object 
    //mGizmo = Gizmo::create( getWindowSize(), false ); 
}   

void XOneApp::update()
{
	mInteraction.update();
	mtextScenario = mInteraction.GetScenarioString();

	mControlRotate1 = mInteraction.GetControlRotate1();
	mControlRotate2 = mInteraction.GetControlRotate2();
	mControlRotate3 = mInteraction.GetControlRotate3();
	mControlRotate4 = mInteraction.GetControlRotate4();
	mControlRotate5 = mInteraction.GetControlRotate5();

	/*mTransform.setToIdentity(); 
	//mTransform.translate (mMayaCam.getCamera().getViewDirection());//translate( Vec3f(0.f, 1.f, 0.f) );
	mTransform.rotate( Vec3f::xAxis(), mMousePos.x*M_PI/180.f + mControlRotate1.x );
	mTransform.rotate( Vec3f::zAxis(), mMousePos.y*M_PI/180.f + mControlRotate1.y );
	mTransform.scale( Vec3f(5,5,5) );
	*/
	
	//mGizmo->setMatrices( mMayaCam.getCamera() );
	//mGizmo->setRotate(mTransform);

	/*
	while( listener.hasWaitingMessages() ) {
		osc::Message message;
		listener.getNextMessage( &message );
		*/
		/*
		console() << "New message received" << std::endl;
		console() << "Address: " << message.getAddress() << std::endl;
		console() << "Num Arg: " << message.getNumArgs() << std::endl;
		for (int i = 0; i < message.getNumArgs(); i++) {
			console() << "-- Argument " << i << std::endl;
			console() << "---- type: " << message.getArgTypeName(i) << std::endl;
			if( message.getArgType(i) == osc::TYPE_INT32 ) {
				try {
					console() << "------ value: "<< message.getArgAsInt32(i) << std::endl;
				}
				catch (...) {
					console() << "Exception reading argument as int32" << std::endl;
				}
			}
			else if( message.getArgType(i) == osc::TYPE_FLOAT ) {
				try {
					console() << "------ value: " << message.getArgAsFloat(i) << std::endl;
				}
				catch (...) {
					console() << "Exception reading argument as float" << std::endl;
				}
			}
			else if( message.getArgType(i) == osc::TYPE_STRING) {
				try {
					console() << "------ value: " << message.getArgAsString(i).c_str() << std::endl;
				}
				catch (...) {
					console() << "Exception reading argument as string" << std::endl;
				}
			}
		}*/
      /*  
		if (message.getAddress() == "/cinder/osc/init/") mUsers.clear();
		else if ( message.getAddress() == "/cinder/osc/")
		{
			if( message.getNumArgs() > 3 && message.getArgType( 0 ) == osc::TYPE_INT32 && message.getArgType( 1 ) == osc::TYPE_FLOAT && message.getArgType( 2 ) == osc::TYPE_FLOAT && message.getArgType( 3 ) == osc::TYPE_FLOAT)
			mUsers[message.getArgAsInt32(0)] = User(message.getArgAsInt32(0), message.getArgAsFloat(1), message.getArgAsFloat(2), message.getArgAsFloat(3));
		}
	}
	*/
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

/*
ci::Vec2f XOneApp::convert_real_world_to_projective(ci::Vec3f point)
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
*/
void XOneApp::draw()
{
	gl::setViewport( getWindowBounds() );
	gl::clear( ColorAf::black() );
	gl::color(ColorAf::white());

	// test 3d 
	/*
	gl::setMatricesWindowPersp( getWindowSize() );
	gl::enableDepthWrite();
	gl::enableDepthRead();
	glDisable( GL_CULL_FACE );
	gl::setMatrices( mMayaCam.getCamera() );



	
	Matrix44f transform;
    // turn to correct position on Earth
    transform.rotate( Vec3f(0.0f, toRadians(mControlRotate.x + 45.f), 0.0f) );
    transform.rotate( Vec3f(toRadians(-mControlRotate.y), 0.0f, 0.0f) );

	//
    gl::pushModelView();
    gl::multModelView( transform );
    gl::drawCoordinateFrame(5.f, 1.f, 1.f); // or simply draw your shape
	mShader.bind();
	gl::draw( mVBO );
	mShader.unbind();
    gl::popModelView();
	*/

	/*
	mGizmo->draw();

	// draw Objets
	mShader.bind();
	gl::pushModelView();
	gl::multModelView( mGizmo->getTransform() ); //mTransform
	//gl::translate(getWindowSize() * 0.5f);
	//gl::rotate( Vec3f(0.f, mMousePos.x, mMousePos.y));
	//gl::scale(5.f, 5.f, 5.f);
	gl::draw( mVBO );
	gl::popModelView();
	mShader.unbind();

	*/
	
    
    
    // Move and draw a cube with it
    /*
    gl::pushModelView();
    gl::multModelView( mGizmo->getTransform() );
    gl::color( 1.0f, 1.0f, 1.0f );
    gl::drawStrokedCube( Vec3f::zero(), Vec3f( 10.0f, 10.0f, 10.0f ));
    gl::popModelView();
	*/

	gl::setMatricesWindow( getWindowSize() );
	gl::enableAlphaBlending();
	// iterate over the warps and draw their content
	for(WarpConstIter itr=mWarps.begin();itr!=mWarps.end();++itr)
	{
		// create a readable reference to our warp, to prevent code like this: (*itr)->begin();
		WarpRef warp( *itr );

		// there are two ways you can use the warps:
		if( mUseBeginEnd )
		{
			// a) issue your draw commands between begin() and end() statements
			warp->begin();
			gl::clear( ColorAf::black() );
			//gl::enableDepthWrite();
			//gl::enableDepthRead();
			gl::enableAlphaBlending();
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
				/*
				gl::color(Color::white());
				if ( mChannel ) 
				{
					if ( mTextureDepth ) {
						mTextureDepth->update( Channel32f( mChannel ) );
					} else {
						mTextureDepth = gl::Texture::create( Channel32f( mChannel ) );
					}
					gl::draw( mTextureDepth, mTextureDepth->getBounds(), getWindowBounds() );
				}*/
	
			/*
				for (std::map<int,User>::iterator iter = mUsers.begin(); iter!=mUsers.end(); ++iter)
				{
					//if(iter->getCenterOfMass().x != 0.f && iter->getCenterOfMass().y != 0.f &&  iter->getCenterOfMass().z < 2000.f &&  iter->getCenterOfMass().z > 100.f)
					//{
						gl::lineWidth(1.f);
						gl::color(Colorf(mUserClr.at(iter->second.mId % mUserClr.size())));
						ci::Vec2f CenterOfMassProjective =  convert_real_world_to_projective(ci::Vec3f(iter->second.mCenterMassX, iter->second.mCenterMassY, iter->second.mCenterMassZ));
						gl::drawSolidCircle(CenterOfMassProjective , 20.f);
						//gl::color(Color(1.f,1.f,0.f));
						//gl::drawStrokedRect( Rectf(iter->getBoundingBox().min.x, iter->getBoundingBox().min.y, iter->getBoundingBox().max.x,  iter->getBoundingBox().max.y ) );
					//}
				}
				*/
				//mInteraction.draw();

				/*
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
	*/






				// test 3d 
				gl::setMatricesWindowPersp( getWindowSize() );
				gl::enableDepthWrite();
				gl::enableDepthRead();
				glDisable( GL_CULL_FACE );
				gl::setMatrices( mMayaCam.getCamera() );
				//gl::setMatrices(mOrthoCam);
	
				Vec3f mAngleInitObjRad = Vec3f(toRadians(mAngleInitObj.x), toRadians(mAngleInitObj.y), toRadians(mAngleInitObj.z));
				vector<Matrix44f> transform;
				transform.push_back(Matrix44f());
				transform.back().rotate( mAngleInitObjRad );
				transform.back().translate(Vec3f(mTranslate1.x, mTranslate1.y , 0.f));
				transform.back().translate(Vec3f(mTranslaleAllObj.x, mTranslaleAllObj.y,0.f));
				transform.back().rotate( Vec3f(0.0f, toRadians(mControlRotate1.x), 0.0f) );
				transform.back().rotate( Vec3f(toRadians(-mControlRotate1.y), 0.0f, 0.0f) );
				transform.back().scale( Vec3f(mScale1, mScale1, mScale1));

				transform.push_back(Matrix44f());
				transform.back().rotate( mAngleInitObjRad );
				transform.back().translate(Vec3f(mTranslate2.x, mTranslate2.y , 0.f));
				transform.back().translate(Vec3f(mTranslaleAllObj.x, mTranslaleAllObj.y,0.f));
				transform.back().rotate( Vec3f(0.0f, toRadians(mControlRotate2.x), 0.0f) );
				transform.back().rotate( Vec3f(toRadians(-mControlRotate2.y), 0.0f, 0.0f) );
				transform.back().scale( Vec3f(mScale2, mScale2, mScale2));

				transform.push_back(Matrix44f());
				transform.back().rotate( mAngleInitObjRad );
				transform.back().translate(Vec3f(mTranslate3.x, mTranslate3.y , 0.f));
				transform.back().translate(Vec3f(mTranslaleAllObj.x, mTranslaleAllObj.y,0.f));
				transform.back().rotate( Vec3f(0.0f, toRadians(mControlRotate3.x ), 0.0f) );
				transform.back().rotate( Vec3f(toRadians(-mControlRotate3.y), 0.0f, 0.0f) );
				transform.back().scale( Vec3f(mScale3, mScale3, mScale3));

				transform.push_back(Matrix44f());
				transform.back().rotate( mAngleInitObjRad );
				transform.back().translate(Vec3f(mTranslate4.x, mTranslate4.y , 0.f));
				transform.back().translate(Vec3f(mTranslaleAllObj.x, mTranslaleAllObj.y,0.f));
				transform.back().rotate( Vec3f(0.0f, toRadians(mControlRotate4.x), 0.0f) );
				transform.back().rotate( Vec3f(toRadians(-mControlRotate4.y), 0.0f, 0.0f) );
				transform.back().scale( Vec3f(mScale4, mScale4, mScale4));

				transform.push_back(Matrix44f());
				transform.back().rotate( mAngleInitObjRad );
				transform.back().translate(Vec3f(mTranslate5.x, mTranslate5.y , 0.f));
				transform.back().translate(Vec3f(mTranslaleAllObj.x, mTranslaleAllObj.y,0.f));
				transform.back().rotate( Vec3f(0.0f, toRadians(mControlRotate5.x ), 0.0f) );
				transform.back().rotate( Vec3f(toRadians(-mControlRotate5.y), 0.0f, 0.0f) );
				transform.back().scale( Vec3f(mScale5, mScale5, mScale5));

				/*
				vector<Vec2f> translates;
				translates.push_back(mTranslate1);
				translates.push_back(mTranslate2);
				translates.push_back(mTranslate3);
				translates.push_back(mTranslate4);
				translates.push_back(mTranslate5);*/

				for (int numTransform = 0 ; numTransform < transform.size() ; ++numTransform)
				{
					gl::pushModelView();
					gl::multModelView( transform.at(numTransform) );
					//gl::translate(translates.at(numTransform).x, translates.at(numTransform).y, 1.f);
					if( mDisplayInfos)
					{
						gl::pushMatrices();
						gl::scale(0.4f, 0.4f , 0.4f);
						gl::drawCoordinateFrame(5.f, 1.f, 1.f); // or simply draw your shape
						gl::popMatrices();
					}
					//gl::enable(GL_CULL_FACE);
					//glCullFace(GL_BACK);
					//glFrontFace(GL_CW);

					mTextureObj.bind();
					mShader.bind();
					mShader.uniform( "tex0", 0 );
					//mShader.bind();

					//gl::scale(Vec3f(5.f,5.f,5.f));
					gl::draw( mVBO );
					//gl::drawSphere(Vec3f(0.0,0.f,0.f),1.f,100);
					mShader.unbind();
					gl::popModelView();
				}


				gl::setMatricesWindow(getWindowWidth(), getWindowHeight(), true);
				// in this demo, we want to draw a specific area of our image,
				// but if you want to draw the whole image, you can simply use: gl::draw( mImage );
				//gl::draw( mImage, mSrcArea, warp->getBounds() );

				// flip horizontally
				gl::pushModelView();
				gl::translate(getWindowSize().x * 0.5f, getWindowSize().y * 0.5f, mTranslateZ);
				//gl::translate( ci::Vec2f( getWindowWidth(), 0 ) );
				gl::scale( ci::Vec2f(-1.f*mScaleLogoX, 1.f*mScaleLogoY) );

				gl::color(ColorAf::white());
				
				if(!mDisplayPositionObjets && mXBoxSanBouleTextureRef) gl::draw(mXBoxSanBouleTextureRef, ci::Vec2f(mPosText.x, mPosText.y) - 0.5f*mXBoxSanBouleTextureRef->getSize());
				if(mDisplayPositionObjets && mXBoxTextureRef) gl::draw(mXBoxTextureRef, ci::Vec2f(mPosText.x, mPosText.y) - 0.5f*mXBoxTextureRef->getSize());//552
				
				// restore the flip
				gl::popModelView();



				warp->end();
		}
		else
		{
			// flip horizontally
//			gl::pushModelView();
//			gl::translate(getWindowSize() * 0.5f);
			//gl::translate( ci::Vec2f( getWindowWidth(), 0 ) );
//			gl::scale( ci::Vec2f(-1.f*mScaleLogo, 1.f*mScaleLogo) );
			// b) simply draw a texture on them (ideal for video)

			// in this demo, we want to draw a specific area of our image,
			// but if you want to draw the whole image, you can simply use: warp->draw( mImage );
//			gl::translate(ci::Vec2f(mPosText.x, mPosText.y) - 0.5f*mXBoxTextureRef->getSize());
			//warp->draw(*mXBoxTextureRef, Area((float) -0.5f*mXBoxTextureRef->getWidth()*mScaleLogoX + mPosText.x, (float)  -0.5f*mXBoxTextureRef->getHeight()*mScaleLogoY + mPosText.y, (float) mXBoxTextureRef->getWidth()*mScaleLogoX + mPosText.x, (float)  mXBoxTextureRef->getHeight()*mScaleLogoY + mPosText.y) , Rectf(0.f, 0.f, -(float) warp->getBounds().getWidth(), -(float) warp->getBounds().getHeight() ));
			warp->draw(*mXBoxTextureRef);
			// restore the flip
//			gl::popModelView();
		}
	}
	if (mDisplayInfos)
	{
		mInteraction.draw();
		if(mParams) mParams->draw();
	}

}

CINDER_APP_NATIVE( XOneApp, RendererGl )
