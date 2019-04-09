#include <FMOD/fmod.hpp>
#include <FMOD/fmod_errors.h>  //only for error checking
#include <iostream>
#include "ISubscriber.h"

typedef FMOD::Sound* SoundClass;

class SoundSystemClass : public ISubscriber {
public:
	// Pointer to the FMOD instance
	FMOD::System *m_pSystem;
	FMOD::Channel *channel;
	FMOD::Channel* currentSong;
	int deathCount = 0;
	bool fade_in = false, timestamp=true;
	float currentT;
	SoundSystemClass() {
		if (FMOD::System_Create(&m_pSystem) != FMOD_OK) {
			// Report Error
			return;
		}

		int driverCount = 0;
		m_pSystem->getNumDrivers(&driverCount);

		if (driverCount == 0) {
			// Report Error
			return;
		}

		// Initialize our Instance with 36 Channels
		m_pSystem->init(36, FMOD_INIT_NORMAL, nullptr);
		EventManager::subscribe(PLAY_BGM_N, this);
		EventManager::subscribe(PLAY_SE, this);
		EventManager::subscribe(FADE, this);
	}

	~SoundSystemClass() {
		EventManager::unsubscribe(PLAY_BGM_N, this);
		EventManager::unsubscribe(PLAY_SE, this);
		EventManager::unsubscribe(FADE, this);
	}

	void createSound(SoundClass *pSound, const char* pFile) {
		FMOD_RESULT result = m_pSystem->createSound(pFile, FMOD_DEFAULT, 0, pSound);
		FMODErrorCheck(result);
	}

	void playSound(SoundClass pSound, bool bLoop = false) {
		if (!bLoop)
			pSound->setMode(FMOD_LOOP_OFF);
		else {
			pSound->setMode(FMOD_LOOP_NORMAL);
			pSound->setLoopCount(-1);
		}

		m_pSystem->playSound(pSound, nullptr, false, &channel);
	}

	void playSoundMenu(SoundClass pSound, bool bLoop = false) {
		if (!bLoop)
			pSound->setMode(FMOD_LOOP_OFF);
		else {
			pSound->setMode(FMOD_LOOP_NORMAL);
			pSound->setLoopCount(-1);
		}

		m_pSystem->playSound(pSound, nullptr, false, &currentSong);
	}

	void releaseSound(SoundClass pSound) {
		pSound->release();
	}

private:
	void FMODErrorCheck(FMOD_RESULT result) {
		if (result != FMOD_OK) {
			std::cout << "FMOD error! (" << result << ") " << FMOD_ErrorString(result) << std::endl;
		}
	}

	void notify(EventName eventName, Param* params) {
		switch (eventName) {
			case PLAY_BGM_N: {
				TypeParam<int> *p = dynamic_cast<TypeParam<int> *>(params);
				int bgm_Type = p->Param;
				//FMOD::Channel* currentSong;

				SoundClass sound;
				if (bgm_Type == 0) {  //Menu
					std::cout<<"normal"<<std::endl;
					createSound(&sound, "..\\assets\\sounds\\bgm_menu.wav");
					//createSound(&sound, "..\\assets\\sounds\\bgm2.mp3");
					playSoundMenu(sound, true);
					createSound(&sound, "..\\assets\\sounds\\bgm1.wav");
					playSound(sound, true);
					if (channel != 0) {
						channel->setVolume(0.0f);
					}
					//if (currentSong != 0) {
						currentSong->setVolume(0.6f);
					//}
				}
				else if (bgm_Type == 1) {  //In game Normal Type
					std::cout << "11" << std::endl;
					if (channel != 0) {
						channel->setVolume(0.5f);
					}
					if (currentSong != 0) {
						currentSong->setVolume(0.0f);
					}
				}
				else if (bgm_Type == 2) {
					if (deathCount >= 2) {
						std::cout << "speed up" << std::endl;
						currentSong->setVolume(0.0f);
						createSound(&sound, "..\\assets\\sounds\\bgm2.mp3");
						playSound(sound, true);
						channel->setVolume(0.0f);
						fade_in = true;
					}
				}
				else {
					createSound(&sound, " ");
					std::cout << "BGM defines error!" << std::endl;
				}
				
				break;
			}
			case PLAY_SE: {
				TypeParam<int> *p = dynamic_cast<TypeParam<int> *>(params);
				int se_Type = p->Param;

				SoundClass sound;
				if (se_Type == 0) {    //Hit(you)
					createSound(&sound, "..\\assets\\sounds\\hit1.wav");
				}
				else if (se_Type == 1) {     //Hit(others)
					createSound(&sound, "..\\assets\\sounds\\hit2.wav");
				}
				else if (se_Type == 2) {     //Death
					createSound(&sound, "..\\assets\\sounds\\hit_death.wav");
					deathCount += 1;
				}
				else if (se_Type == 3) {    //Finish!
					createSound(&sound, "..\\assets\\sounds\\finish.wav");
					deathCount = 0;
				}
				else {
					createSound(&sound, " ");
					std::cout << "SE defines error!" << std::endl;
				}
				playSound(sound, false);
				break;
			}
			case FADE: {
				TypeParam<float> *p = dynamic_cast<TypeParam<float> *>(params);
				float elapsedT = p->Param;
				
				if (timestamp) {
					currentT = elapsedT;
				}
				if (channel != 0 && fade_in) {
					timestamp = false;
					float volume;
					channel->getVolume(&volume);
					float nextVolume = /*volume +*/ (elapsedT-currentT) / 5.0f;
					std::cout << volume << " + " << elapsedT-currentT << "  ";
					if (nextVolume >= 1.0f) {
						channel->setVolume(1.0f);
						fade_in = false;
					}
					else channel->setVolume(nextVolume);
				}
				break;
			}
			default: {
				break;
			}
		}
	}
};

