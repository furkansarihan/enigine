#ifndef sound_engine
#define sound_engine

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <mutex>
#include <cstdint>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
// #include <sndfile.h>
#include <glm/glm.hpp>

struct SoundSource
{
    ALuint sourceId;
    ALuint bufferId;
};

class SoundEngine
{
public:
    SoundEngine();
    ~SoundEngine();
    std::mutex m_mutex;
    bool init();
    // Buffer
    // TODO: resource manager
    // TODO: mp3
    ALuint loadWav(const char *path);
    void deleteSound(ALuint bufferId);
    // Source
    SoundSource createSource(ALuint bufferId);
    void deleteSource(const SoundSource soundSource);
    // Source state
    void playSource(const SoundSource soundSource);
    void pauseSource(const SoundSource soundSource);
    void stopSource(const SoundSource soundSource);
    // Source getters
    float getPlayerPosition(const SoundSource soundSource);
    ALint getSourceState(const SoundSource soundSource);
    float getSourceGain(const SoundSource soundSource);
    float getSourcePitch(const SoundSource soundSource);
    ALint getSourceLooping(const SoundSource soundSource);
    glm::vec3 getSourcePosition(const SoundSource soundSource);
    // Source setters
    void setPlayerPosition(const SoundSource soundSource, const float positionInSeconds);
    void setSourceGain(const SoundSource soundSource, const float gain);
    void setSourcePitch(const SoundSource soundSource, const float pitch);
    void setSourceLooping(const SoundSource soundSource, const ALint looping);
    void setSourcePosition(const SoundSource soundSource, const float x, const float y, const float z);

    // Listener
    void setListenerPosition(const float x, const float y, const float z);
    void setListenerOrientation(const std::vector<float> *orientation);

private:
    // Variables
    ALCdevice *openALDevice;
    ALCcontext *openALContext;
    ALCboolean contextMadeCurrent;
};
#endif /* sound_engine */
