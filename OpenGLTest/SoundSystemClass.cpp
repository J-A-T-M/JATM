#include "SoundSystemClass.h"

#include <Algorithm> // only for min
#include <iostream>
#include <FMOD/fmod_errors.h> // only for error checking

#include "EventManager.h"

SoundSystemClass::SoundSystemClass() {
	if (FMOD::System_Create(&system) != FMOD_OK) {
		// Report Error
		std::cout << "FMOD could not create system" << std::endl;
		return;
	}

	int driverCount = 0;
	system->getNumDrivers(&driverCount);
	if (driverCount == 0) {
		// Report Error
		std::cout << "FMOD could not find any drivers" << std::endl;
		return;
	}

	// Initialize our Instance with 36 Channels
	system->init(36, FMOD_INIT_NORMAL, nullptr);
	
	time = 0.0f;
	fadeIn = false;
	fadeDuration = 0.01f;
	fadeStart = 0.0f;

	EventManager::subscribe(SOUND_SET_NEXT_BGM, this);
	EventManager::subscribe(SOUND_FADE_NEXT_BGM, this);
	EventManager::subscribe(SOUND_PLAY_SE, this);
}

SoundSystemClass::~SoundSystemClass() {
	EventManager::unsubscribe(SOUND_SET_NEXT_BGM, this);
	EventManager::unsubscribe(SOUND_FADE_NEXT_BGM, this);
	EventManager::unsubscribe(SOUND_PLAY_SE, this);
}

void SoundSystemClass::Update(float delta) {
	system->update();
	time += delta;

	if (fadeIn) {
		float nextVolume = std::min((time - fadeStart) / fadeDuration, 1.0f);
		bgm1->setVolume(nextVolume);
		bgm2->setVolume(1.0f - nextVolume);

		if (nextVolume == 1.0f) {
			fadeIn = false;
		}
	}
}

void SoundSystemClass::FMODErrorCheck(FMOD_RESULT result) {
	if (result != FMOD_OK) {
		std::cout << "FMOD error! (" << result << ") " << FMOD_ErrorString(result) << std::endl;
	}
}

void SoundSystemClass::createSound(FMOD::Sound ** pSound, const char * pFile) {
	FMOD_RESULT result = system->createSound(pFile, FMOD_DEFAULT, 0, pSound);
	FMODErrorCheck(result);
}

void SoundSystemClass::playSound(FMOD::Sound * pSound, FMOD::Channel *& channel, bool bLoop) {
	if (!bLoop)
		pSound->setMode(FMOD_LOOP_OFF);
	else {
		pSound->setMode(FMOD_LOOP_NORMAL);
		pSound->setLoopCount(-1);
	}
	system->playSound(pSound, nullptr, false, &channel);
}

void SoundSystemClass::releaseSound(FMOD::Sound* pSound) {
	pSound->release();
}

void SoundSystemClass::notify(EventName eventName, Param * params) {
	switch (eventName) {
		case SOUND_SET_NEXT_BGM: {
			TypeParam<int> *p = dynamic_cast<TypeParam<int> *>(params);
			int bgm_Type = p->Param;

			bgm2->stop();
			bgm2 = bgm1;

			FMOD::Sound* sound;
			if (bgm_Type == 0) {		//Menu
				createSound(&sound, "..\\assets\\sounds\\bgm_menu.wav");
				playSound(sound, bgm1, true);
			} else if (bgm_Type == 1) {	//In-game normal
				createSound(&sound, "..\\assets\\sounds\\bgm1.wav");
				playSound(sound, bgm1, true);
			} else if (bgm_Type == 2) {	//In-game fast
				createSound(&sound, "..\\assets\\sounds\\bgm2.wav");
				playSound(sound, bgm1, true);
			}

			bgm2->setVolume(1.0);
			bgm1->setVolume(0.0);
			fadeIn = false;
			break;
		}
		case SOUND_FADE_NEXT_BGM: {
			TypeParam<float> *p = dynamic_cast<TypeParam<float> *>(params);
			fadeIn = true;
			fadeDuration = std::max(std::fabs(p->Param), 0.01f);
			fadeStart = time;
			break;
		}
		case SOUND_PLAY_SE: {
			TypeParam<int> *p = dynamic_cast<TypeParam<int> *>(params);
			int se_Type = p->Param;

			FMOD::Sound* sound;
			if (se_Type == 0) {			// Hit
				createSound(&sound, "..\\assets\\sounds\\hit1.wav");
				playSound(sound, se, false);
			} else if (se_Type == 1) {	//Death
				createSound(&sound, "..\\assets\\sounds\\hit_death.wav");
				playSound(sound, se, false);
			} else if (se_Type == 2) {	//Finish!
				createSound(&sound, "..\\assets\\sounds\\match_finished.wav");
				playSound(sound, se, false);
			} else if (se_Type == 3) {	//Ready_GO
				createSound(&sound, "..\\assets\\sounds\\ready_go.wav");
				playSound(sound, se, false);
			} else if (se_Type == 4) {	//First blood
				createSound(&sound, "..\\assets\\sounds\\first_blood.wav");
				playSound(sound, se, false);
			} else if (se_Type == 5) {	//Another player down
				createSound(&sound, "..\\assets\\sounds\\another_player_down.wav");
				playSound(sound, se, false);
			} else if (se_Type == 6) {	//Two players left
				createSound(&sound, "..\\assets\\sounds\\only_two_players_left.wav");
				playSound(sound, se, false);
			}
			break;
		}
	}
}