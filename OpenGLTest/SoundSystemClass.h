#include <FMOD/fmod.hpp>

#include "ISubscriber.h"

class SoundSystemClass : public ISubscriber {
	public:
		SoundSystemClass();
		~SoundSystemClass();
		void Update(float delta);
	private:
		FMOD::System *system;

		FMOD::Channel* bgm1; // bgm being faded in
		FMOD::Channel* bgm2; // bgm being faded out
		FMOD::Channel* se;

		float time;
		bool fadeIn; // is bgm currently fading in
		float fadeDuration; // how long should bgm take to fade in
		float fadeStart; // time bgm fading started

		void FMODErrorCheck(FMOD_RESULT result);
		void createSound(FMOD::Sound** pSound, const char* pFile);
		void playSound(FMOD::Sound* pSound, FMOD::Channel* &channel, bool bLoop = false);
		void releaseSound(FMOD::Sound * pSound);
		void notify(EventName eventName, Param* params);
};

