#include <FMOD/fmod.hpp>
#include <FMOD/fmod_errors.h>  //only for error checking
#include <iostream>
#include "ISubscriber.h"

typedef FMOD::Sound* SoundClass;

class SoundSystemClass : public ISubscriber {
public:
	// Pointer to the FMOD instance
	FMOD::System *m_pSystem;
	int bgm_Type, se_Type;

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
	}

	~SoundSystemClass() {
		EventManager::unsubscribe(PLAY_BGM_N, this);
		EventManager::unsubscribe(PLAY_SE, this);
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

		FMOD::Channel *channel;
		m_pSystem->playSound(pSound, nullptr, false, &channel);
	}

	void releaseSound(SoundClass pSound) {
		pSound->release();
	}

	void BGM_type(int pType) {
		if (pType == 0) {
			//std::cout << "aaa" << std::endl;
			bgm_Type=0;
		}
		else if(pType==1){
			//std::cout << "bbb" << std::endl;
			bgm_Type=1;
		}
		else
			bgm_Type = -1;
	}
	void SE_type(int pType) {
		if (pType == 0)
			std::cout << "SE0" << std::endl;
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
				
				SoundClass sound;
				if (SoundSystemClass::bgm_Type == 0) {
					std::cout<<"normal"<<std::endl;
					createSound(&sound, "..\\assets\\sounds\\12_mixdown.wav");
				}
				else if(SoundSystemClass::bgm_Type == -1){
					std::cout << "BGM defines error!" << std::endl;
					createSound(&sound, " ");
				}
				
				playSound(sound, true);
				//std::cout << "test" << std::endl;
				break;
			}
			case PLAY_SE: {
				SoundClass sound;
				if (SoundSystemClass::se_Type == 0) {
					createSound(&sound, "..\\assets\\sounds\\Blow1.wav");
				}
				break;
			}
			default: {
				break;
			}
		}
	}
	//EventManager::subscribe(PLAY_BGM_N, this);
};

