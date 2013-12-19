
#include "Interaction.h"
#include "cinder/Rand.h"
#include "cinder/CinderMath.h"
using namespace ci;
using namespace ci::app;
using namespace std;

//V1 mPos1(Vec3f(400.f,300.f, 1000.f)),  mPos2(Vec3f(200.f,300.f, 1000.f )), mPos3(Vec3f(0.f, 300.f, 1000.f)), mPos4(Vec3f(-200.f, 300.f, 1000.f)), mPos5(Vec3f(-400.f, 300.f, 1000.f))
//V2 mPos1(Vec3f(405,176,1366)),  mPos2(Vec3f(213,185,1376)), mPos3(Vec3f(-20, 185, 1420)), mPos4(Vec3f(-210, 186,1376)), mPos5(Vec3f(-407, 185, 1373)) 85 + 100
//v3 mPos1(Vec3f(305,176,1366)),  mPos2(Vec3f(150,185,1376)), mPos3(Vec3f(0, 185, 1420)), mPos4(Vec3f(-150, 186,1376)), mPos5(Vec3f(-300, 185, 1373))
//v4 mPos1(Vec3f(100,185,1366)),  mPos2(Vec3f(50,185,1376)), mPos3(Vec3f(0, 185, 1420)), mPos4(Vec3f(-50, 185,1376)), mPos5(Vec3f(-100, 185, 1373))
//v5 mPos1(Vec3f(320,500,1350)),  mPos2(Vec3f(160,500,1350)), mPos3(Vec3f(0, 500, 1350)), mPos4(Vec3f(-160, 500,1350)), mPos5(Vec3f(-320, 500, 1350))
Interaction::Interaction():mScenario(Scenario::A), mPos1(Vec3f(320,500,1350)),  mPos2(Vec3f(160,500,1350)), mPos3(Vec3f(0, 500, 1350)), mPos4(Vec3f(-160, 500,1350)), mPos5(Vec3f(-320, 500, 1350)) , mlastPASSANT_ARRETElapsedTime(0.0),
	mLock1(false), mLock2(false), mLock3(false), mLock4(false), mLock5(false),
	mFinishNonnon1(true), mFinishNonnon2(true), mFinishNonnon3(true), mFinishNonnon4(true), mFinishNonnon5(true)
{

}

Interaction::~Interaction()
{

}

string Interaction::GetScenarioString()
{
	string ret;
	switch(mScenario)
	{
		case Scenario::A:
			ret = "A";
		break;
		case Scenario::B_1PASSANT_MARCHE:
			ret = "B_1PASSANT_MARCHE";
		break;
		case Scenario::B_1PASSANT_ARRET:
			ret = "B_1PASSANT_ARRET";
		break;
		case Scenario::C_2PASSANT_MARCHE:
			ret = "C_2PASSANT_MARCHE";
		break;
		case Scenario::C_1PASSANT_ARRET:
			ret = "C_1PASSANT_ARRET";
		break;
		case Scenario::C_2PASSANT_ARRET:
			ret = "C_2PASSANT_ARRET";
		break;
		case Scenario::D:
			ret = "D";
		break;
		default:
			ret = "NO SCENARIO";
	}
	return ret;
}

ci::Vec2f Interaction::convert_real_world_to_projective(ci::Vec3f point)
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

void Interaction::setup()
{
	//OSC
	listener.setup( 3000 );

	//couleur des utilisateurs
	mUserClr.push_back(Colorf(0.f,1.f,0.f));
	mUserClr.push_back(Colorf(0.f,0.f,1.f));
	mUserClr.push_back(Colorf(1.f,1.f,0.f));
	mUserClr.push_back(Colorf(1.f,0.f,1.f));
	mUserClr.push_back(Colorf(0.f,1.f,1.f));
	mUserClr.push_back(Colorf(1.f,0.5f,0.f));

	//control
	mControlRotate1 = Vec2f();
	mControlRotate2 = Vec2f();
	mControlRotate3 = Vec2f();
	mControlRotate4 = Vec2f();
	mControlRotate5 = Vec2f();

	//timeline
	mTimeline = Timeline::create();
	timeline().add(mTimeline);
	/*mTimeline->setInfinite( false );
	mTimeline->setLoop( true );
	mTimeline->setDefaultAutoRemove( false );
	mTimeline->setAutoRemove( false );
	*/

	//
	//mOffset = Vec2f(-10.f, 20.f);
	//mFactor = Vec2f(3.f, -2.f);
	/*
	mOffset = Vec2f(0.f, 0.f);
	mFactor = Vec2f(10.f, -1.f);
	mOffsetControl = Vec2f(0.f, 0.f);
	mFactorControl = Vec2f(20.f, -20.f);
	mCubeInteraction = Vec3f(300.f, 300.f,500.f);
	mOffsetLimitScreenZ = 1500.f;
	mMoyDetectMovementProjectiveX = 15.f;
	mMoyDetectMovementZ = 120.f;
	mZLimit = 2600.f;
	mDefautRotation = Vec2f(0.f, 20.f);
	*/

	ScenarioAInit();

	// init easing
	mEaseBoxes.push_back( EaseBox( EaseInQuad(), "EaseInQuad" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutQuad(), "EaseOutQuad" ) );
	mEaseBoxes.push_back( EaseBox( EaseInOutQuad(), "EaseInOutQuad" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutInQuad(), "EaseOutInQuad" ) );

	mEaseBoxes.push_back( EaseBox( EaseInCubic(), "EaseInCubic" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutCubic(), "EaseOutCubic" ) );
	mEaseBoxes.push_back( EaseBox( EaseInOutCubic(), "EaseInOutCubic" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutInCubic(), "EaseOutInCubic" ) );

	mEaseBoxes.push_back( EaseBox( EaseInQuart(), "EaseInQuart" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutQuart(), "EaseOutQuart" ) );
	mEaseBoxes.push_back( EaseBox( EaseInOutQuart(), "EaseInOutQuart" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutInQuart(), "EaseOutInQuart" ) );

	mEaseBoxes.push_back( EaseBox( EaseInQuint(), "EaseInQuint" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutQuint(), "EaseOutQuint" ) );
	mEaseBoxes.push_back( EaseBox( EaseInOutQuint(), "EaseInOutQuint" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutInQuint(), "EaseOutInQuint" ) );

	mEaseBoxes.push_back( EaseBox( EaseInSine(), "EaseInSine" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutSine(), "EaseOutSine" ) );
	mEaseBoxes.push_back( EaseBox( EaseInOutSine(), "EaseInOutSine" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutInSine(), "EaseOutInSine" ) );

	mEaseBoxes.push_back( EaseBox( EaseInExpo(), "EaseInExpo" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutExpo(), "EaseOutExpo" ) );
	mEaseBoxes.push_back( EaseBox( EaseInOutExpo(), "EaseInOutExpo" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutInExpo(), "EaseOutInExpo" ) );

	mEaseBoxes.push_back( EaseBox( EaseInCirc(), "EaseInCirc" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutCirc(), "EaseOutCirc" ) );
	mEaseBoxes.push_back( EaseBox( EaseInOutCirc(), "EaseInOutCirc" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutInCirc(), "EaseOutInCirc" ) );

	mEaseBoxes.push_back( EaseBox( EaseInAtan(), "EaseInAtan" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutAtan(), "EaseOutAtan" ) );
	mEaseBoxes.push_back( EaseBox( EaseInOutAtan(), "EaseInOutAtan" ) );
	mEaseBoxes.push_back( EaseBox( EaseNone(), "EaseNone" ) );

	mEaseBoxes.push_back( EaseBox( EaseInBack(), "EaseInBack" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutBack(), "EaseOutBack" ) );
	mEaseBoxes.push_back( EaseBox( EaseInOutBack(), "EaseInOutBack" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutInBack(), "EaseOutInBack" ) );

	mEaseBoxes.push_back( EaseBox( EaseInBounce(), "EaseInBounce" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutBounce(), "EaseOutBounce" ) );
	mEaseBoxes.push_back( EaseBox( EaseInOutBounce(), "EaseInOutBounce" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutInBounce(), "EaseOutInBounce" ) );

	mEaseBoxes.push_back( EaseBox( EaseInElastic( 2, 1 ), "EaseInElastic(2, 1)" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutElastic( 1, 4 ), "EaseOutElastic(1, 4)" ) );
	mEaseBoxes.push_back( EaseBox( EaseInOutElastic( 2, 1 ), "EaseInOutElastic( 2, 1 )" ) );
	mEaseBoxes.push_back( EaseBox( EaseOutInElastic( 1, 4 ), "EaseOutInElastic( 4, 1 )" ) );

	//font
	//vector<string>	fontNames = Font::getNames();

	mFont = Font( "Arial", 40.f );
}

void Interaction::InitTimelineLoop(bool loop)
{
	mTimeline->clear();
	mTimeline->reset();
	if(loop)
	{
		mTimeline->setInfinite( false );
		mTimeline->setLoop( true );
		mTimeline->setDefaultAutoRemove( false );
		mTimeline->setAutoRemove( false );
	} else
	{
		mTimeline->setInfinite( true );
		mTimeline->setLoop(false );
		mTimeline->setDefaultAutoRemove( true );
		mTimeline->setAutoRemove( true );
	}
}

void Interaction::ScenarioAPositionInit()
{
	//Scénario A
	//control 1
	mTimeline->apply( &mControlRotate1, mControlRotate1.value(), Vec2f(), 1.0f, EaseOutCubic() );
	mTimeline->appendTo(&mControlRotate1,  mControlRotate1.value(), 2.0f, EaseOutCubic()).finishFn(std::bind(&Interaction::ScenarioAInit, this));

	//control 2
	mTimeline->apply( &mControlRotate2, mControlRotate2.value(), Vec2f(), 1.0f, EaseOutCubic() );
	mTimeline->appendTo(&mControlRotate2,  mControlRotate2.value(), 2.0f, EaseOutCubic());

	//control 3
	mTimeline->apply( &mControlRotate3, mControlRotate3.value(), Vec2f(), 1.0f, EaseOutCubic() );
	mTimeline->appendTo(&mControlRotate3,  mControlRotate3.value(), 2.0f, EaseOutCubic());

	//control 4
	mTimeline->apply( &mControlRotate4, mControlRotate4.value(), Vec2f(), 1.0f, EaseOutCubic() );
	mTimeline->appendTo(&mControlRotate4,  mControlRotate4.value(), 2.0f, EaseOutCubic());

	//control 5
	mTimeline->apply( &mControlRotate5, mControlRotate5.value(), Vec2f(), 1.0f, EaseOutCubic() );
	mTimeline->appendTo(&mControlRotate5,  mControlRotate5.value(), 2.0f, EaseOutCubic());
}


void Interaction::ScenarioAInit()
{

	//InitTimelineLoop(true);

	vector<float> valueRotation;
	valueRotation.push_back(-45.f);
	valueRotation.push_back( -30.f);
	valueRotation.push_back(-45.f);
	valueRotation.push_back( 30.f);

	//Vec2f mDefautRotation = Vec2f(0.f, 20.f);
	float mTimePause = 5.f;

	//Scénario A
	//control 1
	
	mTimeline->apply( &mControlRotate1, mControlRotate1.value(), Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f );
	mTimeline->appendTo(&mControlRotate1,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate1,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate1,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate1,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate1,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate1,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate1,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate1,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate1,  mDefautRotation, 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate1,  mDefautRotation, mTimePause, EaseOutCubic()).finishFn(std::bind(&Interaction::ScenarioAInit, this));

	//control 2
	mTimeline->apply( &mControlRotate2, mControlRotate2.value(), Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f );
	mTimeline->appendTo(&mControlRotate2,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate2,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate2,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate2,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate2,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate2,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate2,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate2,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate2,  mDefautRotation, 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate2,  mDefautRotation, mTimePause, EaseOutCubic());

	//control 3
	mTimeline->apply( &mControlRotate3,    mControlRotate3.value(), Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f );
	mTimeline->appendTo(&mControlRotate3,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate3,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate3,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate3,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate3,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate3,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate3,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate3,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate3,  mDefautRotation, 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate3,  mDefautRotation, mTimePause, EaseOutCubic());
	//control 4
	mTimeline->apply( &mControlRotate4,   mControlRotate4.value(), Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f);
	mTimeline->appendTo(&mControlRotate4,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate4,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate4,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate4,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate4,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate4,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate4,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate4,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate4,  mDefautRotation, 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate4,  mDefautRotation, mTimePause, EaseOutCubic());

	//control 5
	mTimeline->apply( &mControlRotate5, mControlRotate5.value(), Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f );
	mTimeline->appendTo(&mControlRotate5,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate5,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate5,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate5,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate5,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate5,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate5,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate5,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate5,  mDefautRotation, 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate5,  mDefautRotation, mTimePause, EaseOutCubic());

}

void Interaction::ScenarioDInit()
{

	//Anarchie rapide

	float time = 0.5f;
	vector<float> valueRotation;
	valueRotation.push_back(-45.f);
	valueRotation.push_back( -30.f);
	valueRotation.push_back(-45.f);
	valueRotation.push_back( 30.f);

	//control 1
	mTimeline->apply( &mControlRotate1, mControlRotate1.value(), Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn );
	mTimeline->appendTo(&mControlRotate1,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate1,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate1,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate1,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate1,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate1,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate1,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate1,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate1,  Vec2f(), 1.0f, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);

	//control 2
	mTimeline->apply( &mControlRotate2, mControlRotate2.value(), Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time );
	mTimeline->appendTo(&mControlRotate2,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate2,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate2,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate2,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate2,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate2,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate2,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate2,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate2,  Vec2f(), 1.0f, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);

	//control 3
	mTimeline->apply( &mControlRotate3,    mControlRotate3.value(), Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time );
	mTimeline->appendTo(&mControlRotate3,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate3,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate3,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate3,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate3,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate3,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate3,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate3,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate3,  Vec2f(), 1.0f, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);

	//control 4
	mTimeline->apply( &mControlRotate4,   mControlRotate4.value(), Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time);
	mTimeline->appendTo(&mControlRotate4,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate4,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate4,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate4,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate4,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate4,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate4,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate4,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate4,  Vec2f(), 1.0f, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);

	//control 5
	mTimeline->apply( &mControlRotate5, mControlRotate5.value(), Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), 1.0f );
	mTimeline->appendTo(&mControlRotate5,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate5,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate5,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate5,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate5,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate5,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate5,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate5,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);
	mTimeline->appendTo(&mControlRotate5,  Vec2f(), 1.0f, mEaseBoxes.at(Rand::randInt(mEaseBoxes.size())).mFn);



	//Track themself

	float time2 = 0.8f;
		
	vector<float> valueRotation2;
	valueRotation2.push_back(45.f);
	valueRotation2.push_back( -45.f);
	valueRotation2.push_back( 45.f);
	valueRotation2.push_back( -45.f);
	valueRotation2.push_back( 45.f);
	valueRotation2.push_back( -45.f);

	//control 1
	mTimeline->appendTo(&mControlRotate1, Vec2f(valueRotation2.at(1), Rand::randFloat(-5.f,5.f)) , time2, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate1, Vec2f(valueRotation2.at(0), Rand::randFloat(-5.f,5.f)) , time2, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate1, Vec2f(valueRotation2.at(1), Rand::randFloat(-5.f,5.f)) , time2, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate1, Vec2f(valueRotation2.at(0), Rand::randFloat(-5.f,5.f)) , time2, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate1, Vec2f(valueRotation2.at(1), Rand::randFloat(-5.f,5.f)) , time2, EaseOutCubic());

	mTimeline->appendTo(&mControlRotate2, Vec2f(valueRotation2.at(2), Rand::randFloat(-5.f,5.f)) , time2, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate2, Vec2f(valueRotation2.at(1), Rand::randFloat(-5.f,5.f)) , time2, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate2, Vec2f(valueRotation2.at(2), Rand::randFloat(-5.f,5.f)) , time2, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate2, Vec2f(valueRotation2.at(1), Rand::randFloat(-5.f,5.f)) , time2, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate2, Vec2f(valueRotation2.at(2), Rand::randFloat(-5.f,5.f)) , time2, EaseOutCubic());

	mTimeline->appendTo(&mControlRotate3, Vec2f(valueRotation2.at(3), Rand::randFloat(-5.f,5.f)) , time2, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate3, Vec2f(valueRotation2.at(2), Rand::randFloat(-5.f,5.f)) , time2, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate3, Vec2f(valueRotation2.at(3), Rand::randFloat(-5.f,5.f)) , time2, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate3, Vec2f(valueRotation2.at(2), Rand::randFloat(-5.f,5.f)) , time2, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate3, Vec2f(valueRotation2.at(3), Rand::randFloat(-5.f,5.f)) , time2, EaseOutCubic());

	mTimeline->appendTo(&mControlRotate4, Vec2f(valueRotation2.at(4), Rand::randFloat(-5.f,5.f)) , time2, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate4, Vec2f(valueRotation2.at(3), Rand::randFloat(-5.f,5.f)) , time2, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate4, Vec2f(valueRotation2.at(4), Rand::randFloat(-5.f,5.f)) , time2, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate4, Vec2f(valueRotation2.at(3), Rand::randFloat(-5.f,5.f)) , time2, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate4, Vec2f(valueRotation2.at(4), Rand::randFloat(-5.f,5.f)) , time2, EaseOutCubic());

	mTimeline->appendTo(&mControlRotate5, Vec2f(valueRotation2.at(5), Rand::randFloat(-5.f,5.f)) , time2, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate5, Vec2f(valueRotation2.at(4), Rand::randFloat(-5.f,5.f)) , time2, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate5, Vec2f(valueRotation2.at(5), Rand::randFloat(-5.f,5.f)) , time2, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate5, Vec2f(valueRotation2.at(4), Rand::randFloat(-5.f,5.f)) , time2, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate5, Vec2f(valueRotation2.at(5), Rand::randFloat(-5.f,5.f)) , time2, EaseOutCubic());

	// position initial

	//control 1
	mTimeline->appendTo( &mControlRotate1, mControlRotate1.value(), Vec2f(), 1.0f, EaseOutCubic() );
	mTimeline->appendTo(&mControlRotate1,  mControlRotate1.value(), 2.0f, EaseOutCubic()).finishFn(std::bind(&Interaction::ScenarioDInit, this)); // boucle

	//control 2
	mTimeline->appendTo( &mControlRotate2, mControlRotate2.value(), Vec2f(), 1.0f, EaseOutCubic() );
	mTimeline->appendTo(&mControlRotate2,  mControlRotate2.value(), 2.0f, EaseOutCubic());

	//control 3
	mTimeline->appendTo( &mControlRotate3, mControlRotate3.value(), Vec2f(), 1.0f, EaseOutCubic() );
	mTimeline->appendTo(&mControlRotate3,  mControlRotate3.value(), 2.0f, EaseOutCubic());

	//control 4
	mTimeline->appendTo( &mControlRotate4, mControlRotate4.value(), Vec2f(), 1.0f, EaseOutCubic() );
	mTimeline->appendTo(&mControlRotate4,  mControlRotate4.value(), 2.0f, EaseOutCubic());

	//control 5
	mTimeline->appendTo( &mControlRotate5, mControlRotate5.value(), Vec2f(), 1.0f, EaseOutCubic() );
	mTimeline->appendTo(&mControlRotate5,  mControlRotate5.value(), 2.0f, EaseOutCubic());


}

void Interaction::ScenarioAllObjectTrackOneUser(int id)
{
	if(mUsers.size() > 0 && mUsers.back().size() > 0 )
	{
		Vec3f head;
		if (id != -1 && mUsers.back().find(id) != mUsers.back().end()) head = mUsers.back()[id].mCenterMass;
		else head = mUsers.back().begin()->second.mCenterMass;
		//head.y += 400.f;
		Quatf angle1 = 	angle(Vec3f(-mPos1.x, mPos1.y, mPos1.z) , head); // =  mPos1. ; //math<Vec3f>::
		Quatf angle2 = 	angle(Vec3f(-mPos2.x, mPos2.y, mPos2.z), head);
		Quatf angle3 = 	angle(Vec3f(-mPos3.x, mPos3.y, mPos3.z), head);
		Quatf angle4 = 	angle(Vec3f(-mPos4.x, mPos4.y, mPos4.z), head);
		Quatf angle5 = 	angle(Vec3f(-mPos5.x, mPos5.y, mPos5.z), head);
		//Vec2f offset = Vec2f(-10.f, 20.f);
		//Vec2f factor = Vec2f(3.f, -2.f);
		if(!mLock1) mTimeline->apply( &mControlRotate1, mControlRotate1.value(), Vec2f(toDegrees(mFactor.x*angle1.getRoll()) + mOffset.x,toDegrees(mFactor.y*angle1.getPitch()) + mOffset.y), 0.05f );
		if(!mLock2) mTimeline->apply( &mControlRotate2, mControlRotate2.value(), Vec2f(toDegrees(mFactor.x*angle2.getRoll()) + mOffset.x,toDegrees(mFactor.y*angle2.getPitch()) + mOffset.y), 0.05f );
		if(!mLock3) mTimeline->apply( &mControlRotate3, mControlRotate3.value(), Vec2f(toDegrees(mFactor.x*angle3.getRoll()) + mOffset.x,toDegrees(mFactor.y*angle3.getPitch()) + mOffset.y), 0.05f );
		if(!mLock4) mTimeline->apply( &mControlRotate4, mControlRotate4.value(), Vec2f(toDegrees(mFactor.x*angle4.getRoll()) + mOffset.x,toDegrees(mFactor.y*angle4.getPitch()) + mOffset.y), 0.05f );
		if(!mLock5) mTimeline->apply( &mControlRotate5, mControlRotate5.value(), Vec2f(toDegrees(mFactor.x*angle5.getRoll()) + mOffset.x,toDegrees(mFactor.y*angle5.getPitch()) + mOffset.y), 0.05f );
	}
}

void Interaction::ScenarioObjectTrackTwoUser()
{
	if(mUsers.size() > 0 && mUsers.back().size() > 1 )
	{
		//mPositionBeginPerScreen
		/*
		int idLeft;
		int idRight;
		if(mPositionBeginPerUser.size() > 1)
		{
			if (mPositionBeginPerUser.begin()->second == Position::LEFT)
			{
				idLeft = mPositionBeginPerUser.begin()->first;
				idRight = mPositionBeginPerUser.end()->first;
			}
			else		
			{
				idLeft = mPositionBeginPerUser.end()->first;
				idRight = mPositionBeginPerUser.begin()->first;
			}
		}
		*/
		
		if ((mUsers.back().find(mPositionBeginPerUser[Position::LEFT]) != mUsers.back().end()) &&  (mUsers.back().find(mPositionBeginPerUser[Position::RIGHT]) != mUsers.back().end())) 
		{
			Vec3f headLeft = mUsers.back()[mPositionBeginPerUser[Position::LEFT]].mCenterMass;
			Vec3f headRight = mUsers.back()[mPositionBeginPerUser[Position::RIGHT]].mCenterMass;
			Vec3f headCenter = headLeft; //defaut
			if(mUsers.back().find(mIdUserLastMoving) != mUsers.back().end()) headCenter = mUsers.back()[mIdUserLastMoving].mCenterMass; 

			//head.y += 400.f;
			Quatf angle1 = 	angle(Vec3f(-mPos1.x, mPos1.y, mPos1.z), headLeft); // =  mPos1. ; //math<Vec3f>::
			Quatf angle2 = 	angle(Vec3f(-mPos2.x, mPos2.y, mPos2.z), headLeft);
			Quatf angle3 = 	angle(Vec3f(-mPos3.x, mPos3.y, mPos3.z), headCenter);
			Quatf angle4 = 	angle(Vec3f(-mPos4.x, mPos4.y, mPos4.z), headRight);
			Quatf angle5 = 	angle(Vec3f(-mPos5.x, mPos5.y, mPos5.z), headRight);
			//Vec2f offset = Vec2f(-10.f, 20.f);
			//Vec2f factor = Vec2f(3.f, -2.f);
			if(!mLock1) mTimeline->apply( &mControlRotate1, mControlRotate1.value(), Vec2f(toDegrees(mFactor.x*angle1.getRoll()) + mOffset.x,toDegrees(mFactor.y*angle1.getPitch()) + mOffset.y), 0.1f );
			if(!mLock2) mTimeline->apply( &mControlRotate2, mControlRotate2.value(), Vec2f(toDegrees(mFactor.x*angle2.getRoll()) + mOffset.x,toDegrees(mFactor.y*angle2.getPitch()) + mOffset.y), 0.1f );
			if(!mLock3) mTimeline->apply( &mControlRotate3, mControlRotate3.value(), Vec2f(toDegrees(mFactor.x*angle3.getRoll()) + mOffset.x,toDegrees(mFactor.y*angle3.getPitch()) + mOffset.y), 0.1f );
			if(!mLock4) mTimeline->apply( &mControlRotate4, mControlRotate4.value(), Vec2f(toDegrees(mFactor.x*angle4.getRoll()) + mOffset.x,toDegrees(mFactor.y*angle4.getPitch()) + mOffset.y), 0.1f );
			if(!mLock5) mTimeline->apply( &mControlRotate5, mControlRotate5.value(), Vec2f(toDegrees(mFactor.x*angle5.getRoll()) + mOffset.x,toDegrees(mFactor.y*angle5.getPitch()) + mOffset.y), 0.1f );
		}
		else console()<<"Ne trouve pas mPositionBeginPerUser[Position::LEFT] ou mPositionBeginPerUser[Position::RIGHT]"<<endl;
	}

}

void Interaction::MoveObjetMiddleInit()
{
	vector<float> valueRotation;
	valueRotation.push_back(-45.f);
	valueRotation.push_back( -30.f);
	valueRotation.push_back( 45.f);
	valueRotation.push_back( 30.f);
	valueRotation.push_back( 10.f);
	valueRotation.push_back( -10.f);
	valueRotation.push_back( 0.f);
	valueRotation.push_back( 5.f);
	valueRotation.push_back( -5.f);
	valueRotation.push_back( 37.f);
	valueRotation.push_back( -37.f);
	valueRotation.push_back( 0.f);

	float time = 0.5f;
	//control 3
	mTimeline->apply( &mControlRotate3,    mControlRotate3.value(), Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time );
	mTimeline->appendTo(&mControlRotate3,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate3,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate3,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate3,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate3,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate3,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate3,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate3,  Vec2f(valueRotation.at(Rand::randInt(valueRotation.size())),valueRotation.at(Rand::randInt(valueRotation.size()))), time, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate3,  Vec2f(), time, EaseOutCubic()).finishFn(std::bind(&Interaction::MoveObjetMiddleInit, this));;
}

void Interaction::ScenarioObjectTrackEnFaceTwoUser()
{

}

void Interaction::ScenarioLookThemSelfInit()
{
	//InitTimelineLoop(true);

	vector<float> valueRotation;
	valueRotation.push_back(45.f);
	valueRotation.push_back( -45.f);
	valueRotation.push_back( 45.f);
	valueRotation.push_back( -45.f);
	valueRotation.push_back( 45.f);
	valueRotation.push_back( -45.f);

	//control 1
	//float value1 =  valueRotation.at(Rand::randInt(2)) ;
	mTimeline->apply( &mControlRotate1, mControlRotate1.value(), Vec2f(valueRotation.at(0), Rand::randFloat(-5.f,5.f)), 1.0f );
	mTimeline->appendTo(&mControlRotate1, Vec2f(valueRotation.at(1), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate1, Vec2f(valueRotation.at(0), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate1, Vec2f(valueRotation.at(1), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate1, Vec2f(valueRotation.at(0), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate1, Vec2f(valueRotation.at(1), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());


	mTimeline->apply( &mControlRotate2, mControlRotate2.value(), Vec2f(valueRotation.at(1), Rand::randFloat(-5.f,5.f)), 1.0f );
	mTimeline->appendTo(&mControlRotate2, Vec2f(valueRotation.at(2), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate2, Vec2f(valueRotation.at(1), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate2, Vec2f(valueRotation.at(2), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate2, Vec2f(valueRotation.at(1), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate2, Vec2f(valueRotation.at(2), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());

	mTimeline->apply( &mControlRotate3, mControlRotate3.value(), Vec2f(valueRotation.at(2), Rand::randFloat(-5.f,5.f)), 1.0f );
	mTimeline->appendTo(&mControlRotate3, Vec2f(valueRotation.at(3), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate3, Vec2f(valueRotation.at(2), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate3, Vec2f(valueRotation.at(3), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate3, Vec2f(valueRotation.at(2), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
	//mTimeline->appendTo(&mControlRotate3, Vec2f(valueRotation.at(3), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());

	mTimeline->apply( &mControlRotate4, mControlRotate4.value(), Vec2f(valueRotation.at(3), Rand::randFloat(-5.f,5.f)), 1.0f );
	mTimeline->appendTo(&mControlRotate4, Vec2f(valueRotation.at(4), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate4, Vec2f(valueRotation.at(3), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate4, Vec2f(valueRotation.at(4), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate4, Vec2f(valueRotation.at(3), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate4, Vec2f(valueRotation.at(4), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());

	mTimeline->apply( &mControlRotate5, mControlRotate5.value(), Vec2f(valueRotation.at(4),Rand::randFloat(-5.f,5.f)), 1.0f );
	mTimeline->appendTo(&mControlRotate5, Vec2f(valueRotation.at(5), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate5, Vec2f(valueRotation.at(4), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate5, Vec2f(valueRotation.at(5), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate5, Vec2f(valueRotation.at(4), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
	mTimeline->appendTo(&mControlRotate5, Vec2f(valueRotation.at(5), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
}


void Interaction::ScenarioObjectTrackOneUserStop()
{
	if(mUsers.size() > 0 && mUsers.back().size() > 1 )
	{
		// que 2 users
		int idUserStop;
		int positionBegin;
		if(!mUsers.back().begin()->second.mMovement) idUserStop = mUsers.back().begin()->first;
		else 
		{
			auto iter2 = mUsers.back().begin();
			iter2++;
			idUserStop = iter2->first;
		}

		if (mPositionBeginPerUser[Position::LEFT] == idUserStop) positionBegin = Position::LEFT;
		else positionBegin = Position::RIGHT;

		Vec3f head = mUsers.back()[idUserStop].mCenterMass;

		//head.x *=-1;
		//head.y += 400.f;
		Quatf angle1 = 	angle(Vec3f(-mPos1.x, mPos1.y, mPos1.z), head); 
		Quatf angle2 = 	angle(Vec3f(-mPos2.x, mPos2.y, mPos2.z), head);
		Quatf angle3 = 	angle(Vec3f(-mPos3.x, mPos3.y, mPos3.z), head);
		Quatf angle4 = 	angle(Vec3f(-mPos4.x, mPos4.y, mPos4.z), head);
		Quatf angle5 = 	angle(Vec3f(-mPos5.x, mPos5.y, mPos5.z), head);
		//Vec2f offset = Vec2f(-10.f, 20.f);
		//Vec2f factor = Vec2f(3.f, -2.f);
		if(positionBegin == Position::LEFT)
		{
			mTimeline->apply( &mControlRotate1, mControlRotate1.value(), Vec2f(toDegrees(mFactor.x*angle1.getRoll()) + mOffset.x,toDegrees(mFactor.y*angle1.getPitch()) + mOffset.y), 0.1f );
			mTimeline->apply( &mControlRotate2, mControlRotate2.value(), Vec2f(toDegrees(mFactor.x*angle2.getRoll()) + mOffset.x,toDegrees(mFactor.y*angle2.getPitch()) + mOffset.y), 0.1f );
		}
		else
		{
			mTimeline->apply( &mControlRotate4, mControlRotate4.value(), Vec2f(toDegrees(mFactor.x*angle4.getRoll()) + mOffset.x,toDegrees(mFactor.y*angle4.getPitch()) + mOffset.y), 0.1f );
			mTimeline->apply( &mControlRotate5, mControlRotate5.value(), Vec2f(toDegrees(mFactor.x*angle5.getRoll()) + mOffset.x,toDegrees(mFactor.y*angle5.getPitch()) + mOffset.y), 0.1f );
		}
		//if (mIdUserLastMoving == idUserStop) mTimeline->apply( &mControlRotate3, mControlRotate3.value(), Vec2f(toDegrees(mFactor.x*angle3.getRoll()) + mOffset.x,toDegrees(mFactor.y*angle3.getPitch()) + mOffset.y), 0.1f );

	}
}

void Interaction::ScenarioObjectTrackOneUserMoving()
{
	if(mUsers.size() > 0 && mUsers.back().size() > 1 )
	{
		// que 2 users
		int idUserMoving;
		int positionBegin;
		if(mUsers.back().begin()->second.mMovement) idUserMoving = mUsers.back().begin()->first;
		else
		{
			auto iter2 =  mUsers.back().begin();
			iter2++;
			idUserMoving = iter2->first;
		}

		if (mPositionBeginPerUser[Position::LEFT] == idUserMoving) positionBegin = Position::LEFT;
		else positionBegin = Position::RIGHT;

		Vec3f head = mUsers.back()[idUserMoving].mCenterMass;
		//head.x *=-1;
		//head.y += 400.f;
		Quatf angle1 = 	angle(Vec3f(-mPos1.x, mPos1.y, mPos1.z), head); 
		Quatf angle2 = 	angle(Vec3f(-mPos2.x, mPos2.y, mPos2.z), head);
		Quatf angle3 = 	angle(Vec3f(-mPos3.x, mPos3.y, mPos3.z), head);
		Quatf angle4 = 	angle(Vec3f(-mPos4.x, mPos4.y, mPos4.z), head);
		Quatf angle5 = 	angle(Vec3f(-mPos5.x, mPos5.y, mPos5.z), head);
		//Vec2f offset = Vec2f(-10.f, 20.f);
		//Vec2f factor = Vec2f(3.f, -2.f);
		if(positionBegin == Position::LEFT)
		{
			mTimeline->apply( &mControlRotate1, mControlRotate1.value(), Vec2f(toDegrees(mFactor.x*angle1.getRoll()) + mOffset.x,toDegrees(mFactor.y*angle1.getPitch()) + mOffset.y), 0.1f );
			mTimeline->apply( &mControlRotate2, mControlRotate2.value(), Vec2f(toDegrees(mFactor.x*angle2.getRoll()) + mOffset.x,toDegrees(mFactor.y*angle2.getPitch()) + mOffset.y), 0.1f );
		}
		else
		{
			mTimeline->apply( &mControlRotate4, mControlRotate4.value(), Vec2f(toDegrees(mFactor.x*angle4.getRoll()) + mOffset.x,toDegrees(mFactor.y*angle4.getPitch()) + mOffset.y), 0.1f );
			mTimeline->apply( &mControlRotate5, mControlRotate5.value(), Vec2f(toDegrees(mFactor.x*angle5.getRoll()) + mOffset.x,toDegrees(mFactor.y*angle5.getPitch()) + mOffset.y), 0.1f );
		}
		if (mIdUserLastMoving == idUserMoving) mTimeline->apply( &mControlRotate3, mControlRotate3.value(), Vec2f(toDegrees(mFactor.x*angle3.getRoll()) + mOffset.x,toDegrees(mFactor.y*angle3.getPitch()) + mOffset.y), 0.1f );

	}
}


void Interaction::ScenarioLookThemSelfInitUserStop()
{
	if(mUsers.size() > 0 && mUsers.back().size() > 1 )
	{
		// que 2 users
		int idUserStop;
		int positionBegin;
		if(!mUsers.back().begin()->second.mMovementDilate) idUserStop = mUsers.back().begin()->first;
		else 
		{
			auto iter2 = mUsers.back().begin();
			iter2++;
			idUserStop =iter2->first;
		}

		if (mPositionBeginPerUser[Position::LEFT] == idUserStop) positionBegin = Position::LEFT;
		else positionBegin = Position::RIGHT;
		vector<float> valueRotation;
		valueRotation.push_back(45.f);
		valueRotation.push_back( -45.f);
		valueRotation.push_back( 45.f);
		valueRotation.push_back( -45.f);
		valueRotation.push_back( 45.f);
		valueRotation.push_back( -45.f);

		//control 1
		//float value1 =  valueRotation.at(Rand::randInt(2)) ;
		if(positionBegin == Position::RIGHT)
		{
			mTimeline->apply( &mControlRotate1, mControlRotate1.value(), Vec2f(valueRotation.at(0), Rand::randFloat(-5.f,5.f)), 1.0f );
			mTimeline->appendTo(&mControlRotate1, Vec2f(valueRotation.at(1), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
			mTimeline->appendTo(&mControlRotate1, Vec2f(valueRotation.at(0), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
			mTimeline->appendTo(&mControlRotate1, Vec2f(valueRotation.at(1), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
			mTimeline->appendTo(&mControlRotate1, Vec2f(valueRotation.at(0), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
			mTimeline->appendTo(&mControlRotate1, Vec2f(valueRotation.at(1), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());


			mTimeline->apply( &mControlRotate2, mControlRotate2.value(), Vec2f(valueRotation.at(1), Rand::randFloat(-5.f,5.f)), 1.0f );
			mTimeline->appendTo(&mControlRotate2, Vec2f(valueRotation.at(2), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
			mTimeline->appendTo(&mControlRotate2, Vec2f(valueRotation.at(1), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
			mTimeline->appendTo(&mControlRotate2, Vec2f(valueRotation.at(2), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
			mTimeline->appendTo(&mControlRotate2, Vec2f(valueRotation.at(1), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
			mTimeline->appendTo(&mControlRotate2, Vec2f(valueRotation.at(2), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
		}
		/*
		mTimeline->apply( &mControlRotate3, mControlRotate3.value(), Vec2f(valueRotation.at(2), Rand::randFloat(-5.f,5.f)), 1.0f );
		mTimeline->appendTo(&mControlRotate3, Vec2f(valueRotation.at(3), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
		mTimeline->appendTo(&mControlRotate3, Vec2f(valueRotation.at(2), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
		mTimeline->appendTo(&mControlRotate3, Vec2f(valueRotation.at(3), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
		mTimeline->appendTo(&mControlRotate3, Vec2f(valueRotation.at(2), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
		mTimeline->appendTo(&mControlRotate3, Vec2f(valueRotation.at(3), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
		*/

		else
		{
			mTimeline->apply( &mControlRotate4, mControlRotate4.value(), Vec2f(valueRotation.at(3), Rand::randFloat(-5.f,5.f)), 1.0f );
			mTimeline->appendTo(&mControlRotate4, Vec2f(valueRotation.at(4), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
			mTimeline->appendTo(&mControlRotate4, Vec2f(valueRotation.at(3), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
			mTimeline->appendTo(&mControlRotate4, Vec2f(valueRotation.at(4), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
			mTimeline->appendTo(&mControlRotate4, Vec2f(valueRotation.at(3), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
			mTimeline->appendTo(&mControlRotate4, Vec2f(valueRotation.at(4), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());

			mTimeline->apply( &mControlRotate5, mControlRotate5.value(), Vec2f(valueRotation.at(4),Rand::randFloat(-5.f,5.f)), 1.0f );
			mTimeline->appendTo(&mControlRotate5, Vec2f(valueRotation.at(5), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
			mTimeline->appendTo(&mControlRotate5, Vec2f(valueRotation.at(4), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
			mTimeline->appendTo(&mControlRotate5, Vec2f(valueRotation.at(5), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
			mTimeline->appendTo(&mControlRotate5, Vec2f(valueRotation.at(4), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
			mTimeline->appendTo(&mControlRotate5, Vec2f(valueRotation.at(5), Rand::randFloat(-5.f,5.f)) , 1.0f, EaseOutCubic());
		}
	}
}


void Interaction::update()
{
	    // clear
		if(mUsers.size() > 0 && mUsers.back().size() > 0 && (getElapsedSeconds() - mUsers.back().begin()->second.mElapsedSeconds) > 2.f) mUsers.clear();


		while( listener.hasWaitingMessages() ) {
		osc::Message message;
		listener.getNextMessage( &message );
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
        
		if (message.getAddress() == "/cinder/osc/init/") 
		{
			//mUsers.clear();
			if (mUsers.size() > 15) mUsers.erase(mUsers.begin()); //20
			mUsers.push_back(map<int, User>());
		}
		else if ( message.getAddress() == "/cinder/osc/")
		{
			if( message.getNumArgs() > 9 && message.getArgType( 0 ) == osc::TYPE_INT32 && message.getArgType( 1 ) == osc::TYPE_FLOAT && message.getArgType( 2 ) == osc::TYPE_FLOAT && message.getArgType( 3 ) == osc::TYPE_FLOAT && message.getArgType( 4 ) == osc::TYPE_FLOAT && message.getArgType( 5 ) == osc::TYPE_FLOAT && message.getArgType( 6 ) == osc::TYPE_FLOAT && message.getArgType( 7 ) == osc::TYPE_FLOAT && message.getArgType( 8 ) == osc::TYPE_FLOAT && message.getArgType( 9 ) == osc::TYPE_FLOAT)
			{
				//get value
				int id = message.getArgAsInt32(0);
				Vec3f centerOfMass = Vec3f(message.getArgAsFloat(1), message.getArgAsFloat(2), message.getArgAsFloat(3));
				Vec3f handRight = Vec3f(message.getArgAsFloat(4), message.getArgAsFloat(5), message.getArgAsFloat(6));
				Vec3f handLeft = Vec3f(message.getArgAsFloat(7), message.getArgAsFloat(8), message.getArgAsFloat(9));

						//calcul 
				ci::Vec2f CenterOfMassProjective =  convert_real_world_to_projective(centerOfMass);
				ci::Vec2f CenterOfMassProjectiveMap = lmap(Vec2f(640.f - CenterOfMassProjective.x, CenterOfMassProjective.y), Vec2f(0.f,0.f), Vec2f(640.f, 480.f), Vec2f(0.f,0.f), Vec2f( getWindowWidth(), getWindowHeight()*0.5f));			
				
				if(centerOfMass.x != 0.f && centerOfMass.y != 0.f && centerOfMass.z > 100.f && centerOfMass.z < mZLimit /*&& CenterOfMassProjective.x <600.f && CenterOfMassProjective.x > 50.f*/) // filtre 
				{
					//console() << "centerOfMass.x = "<<centerOfMass.x << ", centerOfMass.y = "<<centerOfMass.y <<", centerOfMass.z "<<centerOfMass.z <<endl;

					//calcul si mouvement d'une personne
					Vec2f moyBefore = Vec2f();
					Vec2f moyAfter = Vec2f();
					Vec3f moyBeforeCenterOfMass = Vec3f();
					Vec3f moyAfterCenterOfMass = Vec3f();

					int countBefore = 0;
					int countAfter = 1;
					for(int numUser = 0 ; numUser < (mUsers.size()-1) ; ++numUser)
					{
						for (std::map<int,User>::iterator iter = mUsers.at(numUser).begin(); iter!=mUsers.at(numUser).end(); ++iter)
						{
							if(iter->first == id && mUsers.size() > 8)
							{
								if(numUser < (int) ((float) mUsers.size() /2.f))
								{
									moyBefore += iter->second.mCenterOfMassProjective;
									moyBeforeCenterOfMass += iter->second.mCenterMass;
									countBefore ++;

								}
								else
								{
									moyAfter += iter->second.mCenterOfMassProjective;
									moyAfterCenterOfMass += iter->second.mCenterMass;
									countAfter ++;
								}
							}
						}
					}
					moyAfter += CenterOfMassProjective;
					moyAfterCenterOfMass += centerOfMass;
					moyBefore /= ((float) countBefore);
					moyAfter /= ((float) countAfter);
					moyBeforeCenterOfMass /= ((float) countBefore);
					moyAfterCenterOfMass /=  ((float) countAfter);

					bool mouvement = false;
					//float mMoyDetectMovementProjectiveX = 15.f;
					//float mMoyDetectMovementZ = 120.f;
					if (countBefore > 5 && countAfter > 5 ) // une moyenne n'a de sens que si il y a une multitude d'échantillons sinon
					{
						if(abs((moyAfter.x - moyBefore.x)) > mMoyDetectMovementProjectiveX) // 40.f 10.f  20. is last
						{
							mouvement = true;
							//Beep(400,10);
						}

						if((abs(moyAfterCenterOfMass.z - moyBeforeCenterOfMass.z)) > mMoyDetectMovementZ) // 40.f
						{
							mouvement = true;
							//Beep(400,10);
						}
					}

					// dilatation de mouvement
					bool mouvementDilate = false;
					int compteur = 0;
					if (mouvement) compteur++;
					for(int numUser = 0 ; numUser < (mUsers.size()-1) ; ++numUser)
					{
						for (std::map<int,User>::iterator iter = mUsers.at(numUser).begin(); iter!=mUsers.at(numUser).end(); ++iter)
						{
							if(iter->first == id)
							{
								if(iter->second.mMovement == true) compteur++;
							}
						}
					}
					if (compteur > 1) mouvementDilate = true;

					//ajout de la personne
					if(mUsers.size() > 0 ) mUsers.back().insert(make_pair(id, User(id, getElapsedSeconds(), centerOfMass, CenterOfMassProjective, CenterOfMassProjectiveMap, mouvement, mouvementDilate, handRight, handLeft)));
					if (mUsers.back().size() > 1)
					{
						//console()<<"2users"<<endl;
					}
				}
			}
		}
	}

	if(mUsers.size() > 0 && mUsers.back().size() == 0  ) mUsers.pop_back();


	// center of mass main for debug
	if(mUsers.size() > 0 && mUsers.back().size() > 0 ) mCenterOfMassMain = mUsers.back().begin()->second.mCenterMass;

	//Change scenario
	int mScenarioBefore = mScenario;
	mScenario = Scenario::A; // default scenario
	if(mUsers.size() > 0 && mUsers.back().size() == 1 ) // 1 Spectacteur
    {
		if(mUsers.back().begin()->second.mMovement) mScenario = Scenario::B_1PASSANT_MARCHE;
		else  
		{
			mScenario = Scenario::B_1PASSANT_ARRET;
		}
	}
	else if(mUsers.size() > 0 && mUsers.back().size() == 2 ) // 2 spectacteurs
    {
		if (mScenarioBefore != Scenario::C_1PASSANT_ARRET && mScenarioBefore != Scenario::C_2PASSANT_ARRET &&  mScenarioBefore != Scenario::C_2PASSANT_MARCHE) ComputeBeginComing();

		int id1 = mUsers.back().begin()->first;
		auto iter2 = mUsers.back().begin();
		iter2++;
		int id2 =  iter2->first;
		if (mUsers.back().begin()->second.mMovement && !iter2->second.mMovement) mIdUserLastMoving = id1;
		else if (!mUsers.back().begin()->second.mMovement && iter2->second.mMovement) mIdUserLastMoving = id2;


		if(mUsers.back()[id1].mMovement && mUsers.back()[id2].mMovement) mScenario = Scenario::C_2PASSANT_MARCHE;
		else if(((mUsers.back()[id1].mMovementDilate) && (mUsers.back()[id2].mMovementDilate == false)) || ((mUsers.back()[id1].mMovementDilate == false) && (mUsers.back()[id2].mMovementDilate)))
		{
			mScenario = Scenario::C_1PASSANT_ARRET;
		}
		else  
		{
			mScenario = Scenario::C_2PASSANT_ARRET;
		}
	}
	else if(mUsers.size() > 0 && mUsers.back().size() > 2 ) mScenario = Scenario::D; // plus de 2 spectacteurs
	

	
	//action selon scenario
	switch(mScenario)
	{
		case Scenario::A:
		{
			if(mScenario != mScenarioBefore) 
			{
				mTimeline->clear();
				ScenarioAPositionInit();
				InitPositionBegin();
			}

			break;
		}
		case Scenario::B_1PASSANT_ARRET:
		{
			if(mScenario != mScenarioBefore)
			{
				//InitTimelineLoop(false);
				mlastPASSANT_ARRETElapsedTime = getElapsedSeconds();
				InitPositionBegin();
				mTimeline->clear();
			}
			if(getElapsedSeconds() - mlastPASSANT_ARRETElapsedTime < 5.f)
			{
				//if(mB_1PASSANT_ARRET == 1) InitTimelineLoop(false);
				mB_1PASSANT_ARRET = 0;
				ScenarioAllObjectTrackOneUser();
			} 
			else if(getElapsedSeconds() - mlastPASSANT_ARRETElapsedTime < 10.f)
			{
				if(mB_1PASSANT_ARRET == 0)
				{
					ScenarioLookThemSelfInit();
					mB_1PASSANT_ARRET = 1;
				}
			}
			else if(getElapsedSeconds() - mlastPASSANT_ARRETElapsedTime >= 10.f)
			{
				mlastPASSANT_ARRETElapsedTime = getElapsedSeconds();
			}
			break;
		}
		case Scenario::B_1PASSANT_MARCHE:
		{
			if(mScenario != mScenarioBefore) 
			{
				InitPositionBegin();
				mTimeline->clear();
			}
			ScenarioAllObjectTrackOneUser();
			break;
		}
		case Scenario::C_2PASSANT_MARCHE:
		{
			if(mScenario != mScenarioBefore)
			{
				// init 2 passants mode C
				//if( mPositionBeginPerUser.empty()) ComputeBeginComing();
				mTimeline->clear(); //last
				MoveObjetMiddleInit();
			}
			ScenarioObjectTrackTwoUser();
			break;
		}
		case Scenario::C_1PASSANT_ARRET:
		{
			//if(mScenario != mScenarioBefore && mPositionBeginPerUser.empty()) ComputeBeginComing(); // si on est pas passé par C_2PASSANT_MARCHE avant on calcule
			
			if(mScenario != mScenarioBefore)
			{
				mlastPASSANT_ARRETElapsedTime = getElapsedSeconds();
				mTimeline->clear();
				//MoveObjetMiddleInit();
			}
			
			if(getElapsedSeconds() - mlastPASSANT_ARRETElapsedTime < 5.f)
			{
				mB_1PASSANT_ARRET = 0;
				ScenarioObjectTrackOneUserStop(); //a remettre
			} 
			else if(getElapsedSeconds() - mlastPASSANT_ARRETElapsedTime < 10.f)
			{
				if(mB_1PASSANT_ARRET == 0)
				{
					mTimeline->clear();
					//MoveObjetMiddleInit();
					ScenarioLookThemSelfInitUserStop();
					mB_1PASSANT_ARRET = 1;
				}
			}
			else if(getElapsedSeconds() - mlastPASSANT_ARRETElapsedTime >= 10.f)
			{
				mlastPASSANT_ARRETElapsedTime = getElapsedSeconds();
			}
			ScenarioObjectTrackOneUserMoving(); //a remettre
			
			break;
		}
		case Scenario::C_2PASSANT_ARRET:
		{
			if(mScenario != mScenarioBefore)
			{
				mlastPASSANT_ARRETElapsedTime = getElapsedSeconds();
				//if(mPositionBeginPerUser.empty()) ComputeBeginComing(); // si on est pas passé par C_2PASSANT_MARCHE avant on calcule
				//MoveObjetMiddleInit();
				mTimeline->clear(); //last
				mB_1PASSANT_ARRET = 1;
			}
			if(getElapsedSeconds() - mlastPASSANT_ARRETElapsedTime < 5.f)
			{
				//if(mB_1PASSANT_ARRET == 1) MoveObjetMiddleInit();
				mB_1PASSANT_ARRET = 0;
				ScenarioObjectTrackTwoUser();
			} 
			else if(getElapsedSeconds() - mlastPASSANT_ARRETElapsedTime < 10.f)
			{
				if(mB_1PASSANT_ARRET == 0)
				{
					mTimeline->clear();
					ScenarioLookThemSelfInit();
					mB_1PASSANT_ARRET = 1;
				}
			}
			else if(getElapsedSeconds() - mlastPASSANT_ARRETElapsedTime >= 10.f)
			{
				mlastPASSANT_ARRETElapsedTime = getElapsedSeconds();
				//MoveObjetMiddleInit();
				//Beep(400,10);
			}


			//ScenarioObjectTrackEnFaceTwoUser();
			break;
		}
		case Scenario::D:
		{
			if(mScenario != mScenarioBefore) 
			{
				//ScenarioDInit();
				InitPositionBegin();
				mTimeline->clear(); //last
			}
			bool noMovementTooLongTimeBefore = mNoMovementTooLongTime;
			mNoMovementTooLongTime = ComputeMoreNearAndMovement() ; //ComputeMoreNear(); //ComputeMoreNearAndMovement();
			if (!noMovementTooLongTimeBefore && mNoMovementTooLongTime ) ScenarioAInit();
			if (!mNoMovementTooLongTime) ScenarioAllObjectTrackOneUser(mIdMoreNearAndMovement);

			break;
		}
	}
	
	ScenarioHandControl();
}

void Interaction::InitPositionBegin()
{
	mPositionBeginPerUser[Position::LEFT] = -1;
	mPositionBeginPerUser[Position::RIGHT] = -1;
	mPositionBeginPerScreen[Position::LEFT] = -1;
	mPositionBeginPerScreen[Position::RIGHT] = -1;
}


bool Interaction::ComputeMoreNearAndMovement()
{
	float zmin = 100000000.f;
	int idMoreNearAndMovementBefore = mIdMoreNearAndMovement;
	mIdMoreNearAndMovement = -1;
	for ( auto iter = mUsers.back().begin() ; iter !=  mUsers.back().end() ; ++ iter)
	{
		if(iter->second.mMovement && iter->second.mCenterMass.z < zmin) 
		{
			mIdMoreNearAndMovement = iter->first;
			zmin = iter->second.mCenterMass.z;
		}
	}
	if (mIdMoreNearAndMovement == -1 && idMoreNearAndMovementBefore != -1 ) mElapsedSecondsNoMovement = getElapsedSeconds();
	
	if (getElapsedSeconds() - mElapsedSecondsNoMovement > 7.0 ) return true;

	return false;
}

bool Interaction::ComputeMoreNear()
{
	float zmin = 100000000.f;
	int idMoreNearBefore = mIdMoreNearAndMovement;
	mIdMoreNearAndMovement = -1;
	for ( auto iter = mUsers.back().begin() ; iter !=  mUsers.back().end() ; ++ iter)
	{
		if(/*iter->second.mMovement && */iter->second.mCenterMass.z < zmin) 
		{
			mIdMoreNearAndMovement = iter->first;
			zmin = iter->second.mCenterMass.z;
		}
	}
	if (mIdMoreNearAndMovement == -1 && idMoreNearBefore != -1 ) mElapsedSecondsNoMovement = getElapsedSeconds();
	
	if (getElapsedSeconds() - mElapsedSecondsNoMovement > 7.0 ) return true;

	return false;
}

void Interaction::ScenarioHandControl()
{
	if(mUsers.size() > 0 && mUsers.back().size() > 0 )
	{

		for ( auto iter = mUsers.back().begin() ; iter !=  mUsers.back().end() ; ++ iter)
		{
			Vec3f handRightDiff1 = iter->second.mHandRight - mPos1 ;
			handRightDiff1 = Vec3f(abs(handRightDiff1.x), abs(handRightDiff1.y), abs(handRightDiff1.z));
			Vec3f handRightDiff2 = iter->second.mHandRight - mPos2 ;
			handRightDiff2 = Vec3f(abs(handRightDiff2.x), abs(handRightDiff2.y), abs(handRightDiff2.z));
			Vec3f handRightDiff3 = iter->second.mHandRight - mPos3 ;
			handRightDiff3 = Vec3f(abs(handRightDiff3.x), abs(handRightDiff3.y), abs(handRightDiff3.z));
			Vec3f handRightDiff4 = iter->second.mHandRight - mPos4 ;
			handRightDiff4 = Vec3f(abs(handRightDiff4.x), abs(handRightDiff4.y), abs(handRightDiff4.z));
			Vec3f handRightDiff5 = iter->second.mHandRight - mPos5 ;
			handRightDiff5 = Vec3f(abs(handRightDiff5.x), abs(handRightDiff5.y), abs(handRightDiff5.z));


			Vec3f handLeftDiff1 = iter->second.mHandLeft - mPos1 ;
			handLeftDiff1 = Vec3f(abs(handLeftDiff1.x), abs(handLeftDiff1.y), abs(handLeftDiff1.z));
			Vec3f handLeftDiff2 = iter->second.mHandLeft - mPos2 ;
			handLeftDiff2 = Vec3f(abs(handLeftDiff2.x), abs(handLeftDiff2.y), abs(handLeftDiff2.z));
			Vec3f handLeftDiff3 = iter->second.mHandLeft - mPos3 ;
			handLeftDiff3 = Vec3f(abs(handLeftDiff3.x), abs(handLeftDiff3.y), abs(handLeftDiff3.z));
			Vec3f handLeftDiff4 = iter->second.mHandLeft - mPos4 ;
			handLeftDiff4 = Vec3f(abs(handLeftDiff4.x), abs(handLeftDiff4.y), abs(handLeftDiff4.z));
			Vec3f handLeftDiff5 = iter->second.mHandLeft - mPos5 ;
			handLeftDiff5 = Vec3f(abs(handLeftDiff5.x), abs(handLeftDiff5.y), abs(handLeftDiff5.z));

			/*
			Vec2f mOffsetControl = Vec2f(0.f, 0.f);
			Vec2f mFactorControl = Vec2f(20.f, -20.f);
			Vec3f mCubeInteraction = Vec3f(300.f, 300.f,500.f);
			float mOffsetLimitScreenZ = 1500.f;*/

			Vec3f handControl;
			Vec3f handControlDiff1;
			Vec3f handControlDiff2;
			Vec3f handControlDiff3;
			Vec3f handControlDiff4;
			Vec3f handControlDiff5;
			if (iter->second.mHandRight.z < iter->second.mHandLeft.z) 
			{
				handControl = iter->second.mHandRight;
				handControlDiff1 = handRightDiff1;
				handControlDiff2 = handRightDiff2;
				handControlDiff3 = handRightDiff3;
				handControlDiff4 = handRightDiff4;
				handControlDiff5 = handRightDiff5;
			}
			else
			{
				handControl = iter->second.mHandLeft;
				handControlDiff1 = handLeftDiff1;
				handControlDiff2 = handLeftDiff2;
				handControlDiff3 = handLeftDiff3;
				handControlDiff4 = handLeftDiff4;
				handControlDiff5 = handLeftDiff5;
			}

			//offset
			handControl.y += 50.f;

			vector<float> valueRotation;
			valueRotation.push_back(45.f);
			valueRotation.push_back( -45.f);
			
			float speed = 0.1f;
			float limitMaxY = 80.f;
			float limitMinY = -40.f;
			//control 1
			if ( handControlDiff1.x < mCubeInteraction.x && handControlDiff1.y < mCubeInteraction.y && handControlDiff1.z < mCubeInteraction.z)
			{
				mLock1 = true;
				if(handControl.z >  mOffsetLimitScreenZ) 
				{
					Quatf angle1 = 	angle(mPos1, handControl);
					float angleY = constrain(toDegrees(mFactorControl.y*angle1.getPitch()),limitMinY, limitMaxY);
					mTimeline->apply( &mControlRotate1, mControlRotate1.value(), Vec2f(toDegrees(mFactorControl.x*angle1.getRoll()) + mOffsetControl.x, angleY + mOffsetControl.y), 0.05f );
				}
				/*
				else if (mFinishNonnon1)
				{
					mFinishNonnon1 = false;
					//nonnon
					mTimeline->apply( &mControlRotate1, mControlRotate1.value(), Vec2f(valueRotation.at(0), Rand::randFloat(-5.f,5.f)), speed );
					mTimeline->appendTo(&mControlRotate1, Vec2f(valueRotation.at(1), Rand::randFloat(-5.f,5.f)) , speed, EaseOutCubic());
					mTimeline->appendTo(&mControlRotate1, Vec2f(valueRotation.at(0), Rand::randFloat(-5.f,5.f)) , speed, EaseOutCubic());
					mTimeline->appendTo(&mControlRotate1, Vec2f(valueRotation.at(1), Rand::randFloat(-5.f,5.f)) , speed, EaseOutCubic());
					mTimeline->appendTo(&mControlRotate1, Vec2f(valueRotation.at(0), Rand::randFloat(-5.f,5.f)) , speed, EaseOutCubic());
					mTimeline->appendTo(&mControlRotate1, Vec2f(valueRotation.at(1), Rand::randFloat(-5.f,5.f)) , speed, EaseOutCubic()).finishFn(bind(&Interaction::FinishNonnon1, this));
				}*/
			} else mLock1 = false;
			//control 2
			if ( handControlDiff2.x < mCubeInteraction.x && handControlDiff2.y < mCubeInteraction.y && handControlDiff2.z < mCubeInteraction.z)
			{
				mLock2 = true;
				if(handControl.z >  mOffsetLimitScreenZ) 
				{
					Quatf angle2 = 	angle(mPos2, handControl);
					float angleY = constrain(toDegrees(mFactorControl.y*angle2.getPitch()),limitMinY, limitMaxY);
					mTimeline->apply( &mControlRotate2, mControlRotate2.value(), Vec2f(toDegrees(mFactorControl.x*angle2.getRoll()) + mOffsetControl.x, angleY + mOffsetControl.y), 0.05f );
				}
				/*
				else if(mFinishNonnon2)
				{
					mFinishNonnon2 = false;
					//nonnon
					mTimeline->apply( &mControlRotate2, mControlRotate1.value(), Vec2f(valueRotation.at(0), Rand::randFloat(-5.f,5.f)), speed );
					mTimeline->appendTo(&mControlRotate2, Vec2f(valueRotation.at(1), Rand::randFloat(-5.f,5.f)) , speed, EaseOutCubic());
					mTimeline->appendTo(&mControlRotate2, Vec2f(valueRotation.at(0), Rand::randFloat(-5.f,5.f)) , speed, EaseOutCubic());
					mTimeline->appendTo(&mControlRotate2, Vec2f(valueRotation.at(1), Rand::randFloat(-5.f,5.f)) , speed, EaseOutCubic());
					mTimeline->appendTo(&mControlRotate2, Vec2f(valueRotation.at(0), Rand::randFloat(-5.f,5.f)) , speed, EaseOutCubic());
					mTimeline->appendTo(&mControlRotate2, Vec2f(valueRotation.at(1), Rand::randFloat(-5.f,5.f)) , speed, EaseOutCubic()).finishFn(bind(&Interaction::FinishNonnon2, this));
				}*/
			} else mLock2 = false;
			//control 3

			if (  handControlDiff3.x < mCubeInteraction.x && handControlDiff3.y < mCubeInteraction.y && handControlDiff3.z < mCubeInteraction.z)
			{
				mLock3 = true;

				if(handControl.z >  mOffsetLimitScreenZ) 
				{
					Quatf angle3 = 	angle(mPos3, handControl);
					float angleY = constrain(toDegrees(mFactorControl.y*angle3.getPitch()),limitMinY, limitMaxY);
					mTimeline->apply( &mControlRotate3, mControlRotate3.value(), Vec2f(toDegrees(mFactorControl.x*angle3.getRoll()) + mOffsetControl.x,angleY + mOffsetControl.y), 0.05f );
				}
				/*
				else if( mFinishNonnon3 )
				{
					mFinishNonnon3 = false;
					//nonnon
					mTimeline->apply( &mControlRotate3, mControlRotate1.value(), Vec2f(valueRotation.at(0), Rand::randFloat(-5.f,5.f)), speed );
					mTimeline->appendTo(&mControlRotate3, Vec2f(valueRotation.at(1), Rand::randFloat(-5.f,5.f)) , speed, EaseOutCubic());
					mTimeline->appendTo(&mControlRotate3, Vec2f(valueRotation.at(0), Rand::randFloat(-5.f,5.f)) , speed, EaseOutCubic());
					mTimeline->appendTo(&mControlRotate3, Vec2f(valueRotation.at(1), Rand::randFloat(-5.f,5.f)) , speed, EaseOutCubic());
					mTimeline->appendTo(&mControlRotate3, Vec2f(valueRotation.at(0), Rand::randFloat(-5.f,5.f)) , speed, EaseOutCubic());
					mTimeline->appendTo(&mControlRotate3, Vec2f(valueRotation.at(1), Rand::randFloat(-5.f,5.f)) , speed, EaseOutCubic()).finishFn(bind(&Interaction::FinishNonnon3, this));
				}*/
			} else mLock3 = false;
			//control 4
			if ( mFinishNonnon4 && handControlDiff4.x < mCubeInteraction.x && handControlDiff4.y < mCubeInteraction.y && handControlDiff4.z < mCubeInteraction.z)
			{
				mLock4 = true;
				if(handControl.z >  + mOffsetLimitScreenZ) 
				{
					Quatf angle4 = 	angle(mPos4, handControl);
					float angleY = constrain(toDegrees(mFactorControl.y*angle4.getPitch()),limitMinY, limitMaxY);
					mTimeline->apply( &mControlRotate4, mControlRotate4.value(), Vec2f(toDegrees(mFactorControl.x*angle4.getRoll()) + mOffsetControl.x,angleY + mOffsetControl.y), 0.05f );
				}
				/*
				else if (mFinishNonnon4)
				{
					mFinishNonnon4 = false;
					//nonnon
					mTimeline->apply( &mControlRotate4, mControlRotate1.value(), Vec2f(valueRotation.at(0), Rand::randFloat(-5.f,5.f)), speed );
					mTimeline->appendTo(&mControlRotate4, Vec2f(valueRotation.at(1), Rand::randFloat(-5.f,5.f)) , speed, EaseOutCubic());
					mTimeline->appendTo(&mControlRotate4, Vec2f(valueRotation.at(0), Rand::randFloat(-5.f,5.f)) , speed, EaseOutCubic());
					mTimeline->appendTo(&mControlRotate4, Vec2f(valueRotation.at(1), Rand::randFloat(-5.f,5.f)) , speed, EaseOutCubic());
					mTimeline->appendTo(&mControlRotate4, Vec2f(valueRotation.at(0), Rand::randFloat(-5.f,5.f)) , speed, EaseOutCubic());
					mTimeline->appendTo(&mControlRotate4, Vec2f(valueRotation.at(1), Rand::randFloat(-5.f,5.f)) , speed, EaseOutCubic()).finishFn(bind(&Interaction::FinishNonnon4, this));
				}*/
			} else mLock4 = false;
			//control 5
			if ( handControlDiff5.x < mCubeInteraction.x && handControlDiff5.y < mCubeInteraction.y && handControlDiff5.z < mCubeInteraction.z)
			{
				mLock5 = true;
				if(handControl.z >  mOffsetLimitScreenZ) 
				{
					Quatf angle5 = 	angle(mPos5, handControl);
					float angleY = constrain(toDegrees(mFactorControl.y*angle5.getPitch()), limitMinY, limitMaxY);
					mTimeline->apply( &mControlRotate5, mControlRotate5.value(), Vec2f(toDegrees(mFactorControl.x*angle5.getRoll()) + mOffsetControl.x, angleY + mOffsetControl.y), 0.05f );
				}/*
				else if( mFinishNonnon5 )
				{
					mFinishNonnon5 = false;
					//nonnon
					mTimeline->apply( &mControlRotate5, mControlRotate1.value(), Vec2f(valueRotation.at(0), Rand::randFloat(-5.f,5.f)), speed );
					mTimeline->appendTo(&mControlRotate5, Vec2f(valueRotation.at(1), Rand::randFloat(-5.f,5.f)) , speed, EaseOutCubic());
					mTimeline->appendTo(&mControlRotate5, Vec2f(valueRotation.at(0), Rand::randFloat(-5.f,5.f)) , speed, EaseOutCubic());
					mTimeline->appendTo(&mControlRotate5, Vec2f(valueRotation.at(1), Rand::randFloat(-5.f,5.f)) , speed, EaseOutCubic());
					mTimeline->appendTo(&mControlRotate5, Vec2f(valueRotation.at(0), Rand::randFloat(-5.f,5.f)) , speed, EaseOutCubic());
					mTimeline->appendTo(&mControlRotate5, Vec2f(valueRotation.at(1), Rand::randFloat(-5.f,5.f)) , speed, EaseOutCubic()).finishFn(bind(&Interaction::FinishNonnon5, this));
				}*/
			} else mLock5 = false;
		}
	}
}

void Interaction::FinishNonnon1()
{
	mFinishNonnon1 = true;
}
void Interaction::FinishNonnon2()
{
	mFinishNonnon2 = true;
}
void Interaction::FinishNonnon3()
{
	mFinishNonnon3 = true;
}
void Interaction::FinishNonnon4()
{
	mFinishNonnon4 = true;
}
void Interaction::FinishNonnon5()
{
	mFinishNonnon5 = true;
}

void Interaction::ComputeBeginComing()
{
	// calcul d'ou viennent les personne pour les tracker par 2 objets
	if(mUsers.size() > 0 && mUsers.back().size() == 2 ) // 2 spectacteurs
	{
		int id1 = mUsers.back().begin()->first;
		auto iter2 = mUsers.back().begin();
		iter2++;
		int id2 = iter2->first;
					
		//Position d'arrivée par rapport au milieu de l'ecran
		if(mUsers.back()[id1].mCenterOfMassProjective.x < 320.f) mPositionBeginPerScreen[Position::LEFT] = id1 ;
		else mPositionBeginPerScreen[Position::RIGHT] = id1;
		if(mUsers.back()[id2].mCenterOfMassProjective.x < 320.f) mPositionBeginPerScreen[Position::LEFT] = id2 ;
		else mPositionBeginPerScreen[Position::RIGHT] = id2;

		//Position d'arrivée entre les 2 utilisateurs
		if (mUsers.back()[id1].mCenterOfMassProjective.x < mUsers.back()[id2].mCenterOfMassProjective.x )
		{
			mPositionBeginPerUser[Position::LEFT] = id1;
			mPositionBeginPerUser[Position::RIGHT] = id2;
		} else 
		{
			mPositionBeginPerUser[Position::RIGHT] = id1 ;
			mPositionBeginPerUser[Position::LEFT] = id2;
		}
	}
}

Quatf Interaction::angle( const Vec3f &v1, const Vec3f &rhs )
{
      float angle = math<float>::acos( v1.dot(rhs) / ( v1.length() * rhs.length() ) ) / 2;
      Vec3f axis = v1.cross(rhs).normalized();
      float s = math<float>::sin(angle);

      return Quatf( math<float>::cos(angle), axis.x * s, axis.y * s, axis.z * s ); 
}

void Interaction::draw()
{
	if(mUsers.size() > 0)
	{
		for (std::map<int,User>::iterator iter = mUsers.back().begin(); iter!=mUsers.back().end(); ++iter)
		{
			Vec2f handLeftProjective = convert_real_world_to_projective(iter->second.mHandLeft);
			Vec2f handLeftProjectiveMap = lmap(Vec2f(640.f - handLeftProjective.x, handLeftProjective.y), Vec2f(0.f,0.f), Vec2f(640.f, 480.f), Vec2f(0.f,0.f), Vec2f( getWindowWidth(), getWindowHeight()*0.5f));
			Vec2f handRightProjective = convert_real_world_to_projective(iter->second.mHandRight);
			Vec2f handRightProjectiveMap = lmap(Vec2f(640.f - handRightProjective.x, handRightProjective.y), Vec2f(0.f,0.f), Vec2f(640.f, 480.f), Vec2f(0.f,0.f), Vec2f( getWindowWidth(), getWindowHeight()*0.5f));

			gl::lineWidth(1.f);
			Colorf colorUser = Colorf(mUserClr.at(iter->second.mId % mUserClr.size()));
			gl::color(colorUser);
			//ci::Vec2f CenterOfMassProjective =  convert_real_world_to_projective(ci::Vec3f(iter->second.mCenterMassX, iter->second.mCenterMassY, iter->second.mCenterMassZ));
			//ci::Vec2f CenterOfMassProjectiveMap = lmap(Vec2f(640.f - CenterOfMassProjective.x, CenterOfMassProjective.y), Vec2f(0.f,0.f), Vec2f(640.f, 480.f), Vec2f(0.f,0.f), Vec2f( getWindowWidth(), getWindowHeight()*0.5f));
			gl::pushMatrices();
			gl::translate(Vec2f(0.f, getWindowHeight()*0.5f));
			gl::drawSolidCircle(iter->second.mCenterOfMassProjectiveMap, 20.f);
			gl::drawSolidCircle(handLeftProjectiveMap, 10.f);
			gl::drawSolidCircle(handRightProjectiveMap, 10.f);
			/*
			try {
				gl::pushMatrices();
				//gl::translate(Vec2f(getWindowWidth(), 0.f));
				//gl::scale(-1,1,1);
				//gl::drawString("x =" + boost::lexical_cast<string>(round(iter->second.mHandLeft.x, 5))  + "y =" + boost::lexical_cast<string>(round(iter->second.mHandLeft.y, 5)) + "z =" + boost::lexical_cast<string>(round(iter->second.mHandLeft.z, 5)), Vec2f(getWindowWidth() -handLeftProjectiveMap.x +20.f, handLeftProjectiveMap.y ),colorUser, mFont);
				//gl::drawString("x =" + boost::lexical_cast<string>(round(iter->second.mHandRight.x, 5)) + "y =" + boost::lexical_cast<string>(round(iter->second.mHandRight.y,5)) + "z =" + boost::lexical_cast<string>(round(iter->second.mHandRight.z, 5)), Vec2f(getWindowWidth() - handRightProjectiveMap.x +20.f, handRightProjectiveMap.y ), colorUser, mFont);
				
				if(mLock1)	gl::drawString("Cube interatif 1", Vec2f(iter->second.mCenterOfMassProjectiveMap.x +20.f, iter->second.mCenterOfMassProjectiveMap.y) ,colorUser, mFont);
				if(mLock2)	gl::drawString("Cube interatif 2", Vec2f(iter->second.mCenterOfMassProjectiveMap.x +20.f, iter->second.mCenterOfMassProjectiveMap.y + 10.f ) ,colorUser, mFont);
				if(mLock3)	gl::drawString("Cube interatif 3", Vec2f(iter->second.mCenterOfMassProjectiveMap.x +20.f, iter->second.mCenterOfMassProjectiveMap.y + 20.f ) ,colorUser, mFont);
				if(mLock4)	gl::drawString("Cube interatif 4", Vec2f(iter->second.mCenterOfMassProjectiveMap.x +20.f, iter->second.mCenterOfMassProjectiveMap.y + 30.f ) ,colorUser, mFont);
				if(mLock5)	gl::drawString("Cube interatif 5", Vec2f(iter->second.mCenterOfMassProjectiveMap.x +20.f, iter->second.mCenterOfMassProjectiveMap.y + 40.f ) ,colorUser, mFont);
				
				gl::popMatrices();
			} catch (...) 
			{
				console()<<"error draw string"<<endl;
			}
			*/
			gl::popMatrices();


			//gl::color(Color(1.f,1.f,0.f));
			//gl::drawStrokedRect( Rectf(iter->getBoundingBox().min.x, iter->getBoundingBox().min.y, iter->getBoundingBox().max.x,  iter->getBoundingBox().max.y ) /*+ Rectf(0.f, 0.f , getWindowWidth() * 0.5f,::getWindowHeight()*0.5f)*/);
		}
	}
}

float Interaction::round(float num, int precision)
{
	return floorf(num * pow (10.f, precision) +.5f)/ pow(10.f, precision);
}


