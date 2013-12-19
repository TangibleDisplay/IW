#pragma once

#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
#include "cinder/Camera.h"
#include "cinder/Channel.h"
#include "cinder/params/Params.h"
#include "cinder/Utilities.h"
#include "cinder/Timeline.h"
#include "cinder/Font.h"

#include "OscListener.h"

class Interaction
{
public:
	enum Scenario { A, B_1PASSANT_MARCHE, B_1PASSANT_ARRET, C_2PASSANT_MARCHE, C_1PASSANT_ARRET, C_2PASSANT_ARRET, D };
	enum Position { LEFT, RIGHT };
	Interaction();
	~Interaction();
	void setup();
	void update();
	void draw();
	//control en degree
	ci::Vec2f			GetControlRotate1() { return mControlRotate1.value(); }
	ci::Vec2f			GetControlRotate2() { return mControlRotate2.value(); }
	ci::Vec2f			GetControlRotate3() { return mControlRotate3.value(); }
	ci::Vec2f			GetControlRotate4() { return mControlRotate4.value(); }
	ci::Vec2f			GetControlRotate5() { return mControlRotate5.value(); }

	// position dans l'espace des boules
	ci::Vec3f			mPos1;
	ci::Vec3f			mPos2;
	ci::Vec3f			mPos3;
	ci::Vec3f			mPos4;
	ci::Vec3f			mPos5;

	ci::Vec3f			mCenterOfMassMain;
	ci::Quatf			angle( const ci::Vec3f &v1, const ci::Vec3f &rhs );
	std::string 		GetScenarioString();
	//pour sceario C
	std::map<int, int> mPositionBeginPerScreen;
	std::map<int, int> mPositionBeginPerUser;

	//init JSON
	ci::Vec2f mOffset;
	ci::Vec2f mFactor;
	ci::Vec2f mOffsetControl;
	ci::Vec2f mFactorControl;
	ci::Vec3f mCubeInteraction;
	float mOffsetLimitScreenZ;
	float mMoyDetectMovementProjectiveX;
	float mMoyDetectMovementZ;
	float mZLimit;
	ci::Vec2f mDefautRotation;

private:
	ci::osc::Listener 				listener;
	ci::Vec2f convert_real_world_to_projective(ci::Vec3f point);
	void ScenarioAInit();
	void ScenarioAllObjectTrackOneUser(int id = -1);
	void ScenarioLookThemSelfInit();
	void InitTimelineLoop(bool loop);
	void ScenarioAPositionInit();
	void ScenarioObjectTrackTwoUser();
	void MoveObjetMiddleInit();
	void ScenarioObjectTrackEnFaceTwoUser();
	void ComputeBeginComing();
	bool ComputeMoreNearAndMovement();
	bool ComputeMoreNear();
	void ScenarioObjectTrackOneUserStop();
	void ScenarioLookThemSelfInitUserStop();
	void ScenarioObjectTrackOneUserMoving();
	void ScenarioDInit();
	void ScenarioHandControl();
	void InitPositionBegin();
	void FinishNonnon1();
	void FinishNonnon2();
	void FinishNonnon3();
	void FinishNonnon4();
	void FinishNonnon5();
	bool mFinishNonnon1;
	bool mFinishNonnon2;
	bool mFinishNonnon3;
	bool mFinishNonnon4;
	bool mFinishNonnon5;
	struct User
	{
		User() {};
		User(int id, double elapsedSeconds, ci::Vec3f centerMass, ci::Vec2f centerOfMassProjective, ci::Vec2f centerOfMassProjectiveMap, bool mouvement, bool mouvementDilate, ci::Vec3f handRight, ci::Vec3f handLeft):mId(id), mElapsedSeconds(elapsedSeconds), mCenterMass(centerMass), mCenterOfMassProjective(centerOfMassProjective), mCenterOfMassProjectiveMap(centerOfMassProjectiveMap), mMovement(mouvement),mMovementDilate(mouvementDilate),mHandRight(handRight), mHandLeft(handLeft) {}
		int mId;
		ci::Vec3f mCenterMass;
		ci::Vec2f mCenterOfMassProjective;
		ci::Vec2f mCenterOfMassProjectiveMap;
		bool mMovement;
		bool mMovementDilate;
		double mElapsedSeconds;
		ci::Vec3f mHandRight;
		ci::Vec3f mHandLeft;
	};

	std::vector<std::map<int, User>>			mUsers;
	std::vector<ci::Colorf>     mUserClr;

	//control en degree
	ci::Anim<ci::Vec2f>			    mControlRotate1;
	ci::Anim<ci::Vec2f>				mControlRotate2;
	ci::Anim<ci::Vec2f>				mControlRotate3;
	ci::Anim<ci::Vec2f>				mControlRotate4;
	ci::Anim<ci::Vec2f>				mControlRotate5;

	//timeline
	ci::TimelineRef mTimeline;

	//machine à état Scénario
	int mScenario;

	//elapsed seconds
	double mlastPASSANT_ARRETElapsedTime;
	int mB_1PASSANT_ARRET;

	// easing
	struct EaseBox {
	  public:
		EaseBox( std::function<float (float)> fn, std::string name )
			: mFn( fn )
		{
		}
	
		std::function<float (float)>	mFn;
	};
	std::vector<EaseBox>		mEaseBoxes;



	//Lock
	bool mLock1;
	bool mLock2;
	bool mLock3;
	bool mLock4;
	bool mLock5;

	//font
	ci::Font			mFont;

	float round(float num, int precision);

	int mIdUserLastMoving;
	int mIdMoreNearAndMovement;
	double mElapsedSecondsNoMovement;
	bool mNoMovementTooLongTime;



};

