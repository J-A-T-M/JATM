#include <FMOD/fmod.hpp>
#include <FMOD/fmod_errors.h>  //only for error checking
#include <iostream>
#include "ISubscriber.h"
#include <stdlib.h>

typedef FMOD::Sound* SoundClass;

class SoundSystemClass : public ISubscriber {
public:
	// Pointer to the FMOD instance
	FMOD::System *m_pSystem;
	FMOD::Channel *channel;
	FMOD::Channel* menuChannel;
	FMOD::Channel* seChannel;
	int deathCount = 0;
	bool fade_in = false, fade_out = false, timestamp = true, finished=true;
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
		EventManager::subscribe(S_CLEAR, this);
	}

	~SoundSystemClass() {
		EventManager::unsubscribe(PLAY_BGM_N, this);
		EventManager::unsubscribe(PLAY_SE, this);
		EventManager::unsubscribe(FADE, this);
		EventManager::unsubscribe(S_CLEAR, this);
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

		m_pSystem->playSound(pSound, nullptr, false, &menuChannel);
	}

	void playSE(SoundClass pSound, bool bLoop = false) {
		if (!bLoop)
			pSound->setMode(FMOD_LOOP_OFF);
		else {
			pSound->setMode(FMOD_LOOP_NORMAL);
			pSound->setLoopCount(-1);
		}

		m_pSystem->playSound(pSound, nullptr, false, &seChannel);
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
					/*if (channel != 0 || menuChannel!=0) {
						createSound(&sound, "..\\assets\\sounds\\bgm1.wav");
						playSound(sound, true);
						channel->setPosition(5000,FMOD_TIMEUNIT_MS);
						channel->setVolume(0.0f);
						menuChannel->setPosition(5000, FMOD_TIMEUNIT_MS);
						menuChannel->setVolume(0.7f);
					}*/
					createSound(&sound, "..\\assets\\sounds\\bgm_menu.wav");
					playSoundMenu(sound, true);
					createSound(&sound, "..\\assets\\sounds\\bgm1.wav");
					playSound(sound, true);
					if (channel != 0) {
						channel->setVolume(0.0f);
					}
					if (menuChannel != 0) {
						menuChannel->setVolume(0.7f);
					}
				}
				else if (bgm_Type == 1) {  //In game Normal Type
					if (channel != 0) {
						//channel->setVolume(0.0f);
						fade_in = true;
					}
					if (menuChannel != 0) {
						//menuChannel->setVolume(0.0f);
						fade_out = true;
					}
				}
				else if (bgm_Type == 2) {
					if (deathCount >= 2) {
						channel->setVolume(0.0f);
						std::cout << "speed up" << std::endl;
						finished = false;
						createSound(&sound, "..\\assets\\sounds\\bgm2.wav");
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
					playSE(sound, false);
				}
				else if (se_Type == 1) {     //Hit(others)
					createSound(&sound, "..\\assets\\sounds\\hit2.wav");
					playSE(sound, false);
				}
				else if (se_Type == 2) {     //Death
					deathCount += 1;
					int random;
					random = rand() % 6;
					createSound(&sound, "..\\assets\\sounds\\hit_death.wav");
					playSE(sound, false);
					if (deathCount <= 1) {
						if (random < 3 ) {
							createSound(&sound, "..\\assets\\sounds\\one_player_down.wav");
						}
						else {
							createSound(&sound, "..\\assets\\sounds\\first_blood.wav");
						}
						playSE(sound, false);
					}
					else if (deathCount == 2) {
						if (random < 3) {
							createSound(&sound, "..\\assets\\sounds\\another_player_down.wav");
						}
						else {
							createSound(&sound, "..\\assets\\sounds\\only_two_players_left.wav");
						}
						playSE(sound, false);
					}
				}
				else if (se_Type == 3) {    //Finish!
					createSound(&sound, "..\\assets\\sounds\\match_finished.wav");
					deathCount = 0;
					finished = true;
					playSE(sound, false);
				}
				else if (se_Type == 4) {     //Ready_GO
					createSound(&sound, "..\\assets\\sounds\\ready_go.wav");
					playSE(sound, false);
				}
				else {
					createSound(&sound, " ");
					std::cout << "SE defines error!" << std::endl;
				}
				
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
					float nextVolume = /*volume +*/ (elapsedT-currentT) / 4.0f;
					//std::cout << volume << " + " << elapsedT-currentT << "  \n";
					if (nextVolume >= 0.5f) {
						channel->setVolume(0.5f);
						fade_in = false;
						timestamp = true;
					}
					else channel->setVolume(nextVolume);
				}
				else if (menuChannel != 0 && fade_out) {
					timestamp = false;
					float volume,c_volume;
					menuChannel->getVolume(&volume);
					channel->getVolume(&c_volume);
					float nextVolume = (elapsedT - currentT) / 1.0f; 
					//std::cout << volume << " + " << elapsedT - currentT << "  \n";
					if (0.7f - nextVolume <= 0.0f) {
						menuChannel->setVolume(0.0f);
						fade_out = false;
						timestamp = true;
					}
					else menuChannel->setVolume(0.7f - nextVolume);
				}
				if (!finished) {
					timestamp = false;
					SoundClass sound;
					if (elapsedT - currentT > 10) {
						createSound(&sound, "..\\assets\\sounds\\match_point.wav");
						playSE(sound, false);
						timestamp = true;
						finished = true;
					}
				}
				break;
			}
			case S_CLEAR: {
				printf("exe");
				channel->stop();
				menuChannel->stop();
				channel=0;
				menuChannel = 0;
				//seChannel->stop();
				//seChannel = 0;
				break;
			}
			default: {
				break;
			}
		}
	}
};

