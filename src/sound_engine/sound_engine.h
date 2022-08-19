#ifndef sound_engine
#define sound_engine

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>

#include <AL/al.h>
#include <AL/alc.h>

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
    bool init();
    // Source
    SoundSource loadSource(const char *path);
    void deleteSource(const SoundSource soundSource);
    // Source state
    void playSource(const SoundSource soundSource);
    void pauseSource(const SoundSource soundSource);
    // Source getters
    ALint getSourceState(const SoundSource soundSource);
    float getSourceGain(const SoundSource soundSource);
    float getSourcePitch(const SoundSource soundSource);
    ALint getSourceLooping(const SoundSource soundSource);
    // Source setters
    void setSourceGain(const SoundSource soundSource, const float gain);
    void setSourcePitch(const SoundSource soundSource, const float pitch);
    void setSourceLooping(const SoundSource soundSource, const ALint looping);

    // Listener
    void setListenerPosition(const float x, const float y, const float z);
    void setListenerOrientation(const std::vector<float> *orientation);

private:
    // Variables
    ALCdevice *openALDevice;
    ALCcontext *openALContext;
    ALCboolean contextMadeCurrent;
    // Functions
    bool check_al_errors(const std::string &filename, const std::uint_fast32_t line);
    template <typename alFunction, typename... Params>
    auto alCallImpl(const char *filename,
                    const std::uint_fast32_t line,
                    alFunction function,
                    Params... params)
        -> typename std::enable_if_t<!std::is_same_v<void, decltype(function(params...))>, decltype(function(params...))>;
    template <typename alFunction, typename... Params>
    auto alCallImpl(const char *filename,
                    const std::uint_fast32_t line,
                    alFunction function,
                    Params... params)
        -> typename std::enable_if_t<std::is_same_v<void, decltype(function(params...))>, bool>;
    bool check_alc_errors(const std::string &filename, const std::uint_fast32_t line, ALCdevice *device);
    template <typename alcFunction, typename... Params>
    auto alcCallImpl(const char *filename,
                     const std::uint_fast32_t line,
                     alcFunction function,
                     ALCdevice *device,
                     Params... params)
        -> typename std::enable_if_t<std::is_same_v<void, decltype(function(params...))>, bool>;
    template <typename alcFunction, typename ReturnType, typename... Params>
    auto alcCallImpl(const char *filename,
                     const std::uint_fast32_t line,
                     alcFunction function,
                     ReturnType &returnValue,
                     ALCdevice *device,
                     Params... params)
        -> typename std::enable_if_t<!std::is_same_v<void, decltype(function(params...))>, bool>;
};
#endif /* sound_engine */
