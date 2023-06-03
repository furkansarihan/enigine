#include "sound_engine.h"

#define alCall(function, ...) alCallImpl(__FILE__, __LINE__, function, __VA_ARGS__)
#define alcCall(function, device, ...) alcCallImpl(__FILE__, __LINE__, function, device, __VA_ARGS__)

SoundEngine::SoundEngine()
{
}

// TODO: delete sources
SoundEngine::~SoundEngine()
{
    alcCall(alcMakeContextCurrent, this->contextMadeCurrent, this->openALDevice, nullptr);
    alcCall(alcDestroyContext, this->openALDevice, this->openALContext);

    ALCboolean closed;
    alcCall(alcCloseDevice, closed, this->openALDevice, this->openALDevice);
}

bool SoundEngine::init()
{
    this->openALDevice = alcOpenDevice(nullptr);
    if (!this->openALDevice)
    {
        std::cerr << "ERROR: Could not open audio device" << std::endl;
        return false;
    }

    if (!alcCall(alcCreateContext, this->openALContext, this->openALDevice, this->openALDevice, nullptr) || !this->openALContext)
    {
        std::cerr << "ERROR: Could not create audio context" << std::endl;
        return false;
    }
    this->contextMadeCurrent = false;
    if (!alcCall(alcMakeContextCurrent, this->contextMadeCurrent, this->openALDevice, this->openALContext) || this->contextMadeCurrent != ALC_TRUE)
    {
        std::cerr << "ERROR: Could not make audio context current" << std::endl;
        return false;
    }
    return true;
}

bool SoundEngine::check_al_errors(const std::string &filename, const std::uint_fast32_t line)
{
    ALenum error = alGetError();
    if (error != AL_NO_ERROR)
    {
        std::cerr << "***ERROR*** (" << filename << ": " << line << ")\n";
        switch (error)
        {
        case AL_INVALID_NAME:
            std::cerr << "AL_INVALID_NAME: a bad name (ID) was passed to an OpenAL function";
            break;
        case AL_INVALID_ENUM:
            std::cerr << "AL_INVALID_ENUM: an invalid enum value was passed to an OpenAL function";
            break;
        case AL_INVALID_VALUE:
            std::cerr << "AL_INVALID_VALUE: an invalid value was passed to an OpenAL function";
            break;
        case AL_INVALID_OPERATION:
            std::cerr << "AL_INVALID_OPERATION: the requested operation is not valid";
            break;
        case AL_OUT_OF_MEMORY:
            std::cerr << "AL_OUT_OF_MEMORY: the requested operation resulted in OpenAL running out of memory";
            break;
        default:
            std::cerr << "UNKNOWN AL ERROR: " << error;
        }
        std::cerr << std::endl;
        return false;
    }
    return true;
}

template <typename alFunction, typename... Params>
auto SoundEngine::alCallImpl(const char *filename,
                             const std::uint_fast32_t line,
                             alFunction function,
                             Params... params)
    -> typename std::enable_if_t<!std::is_same_v<void, decltype(function(params...))>, decltype(function(params...))>
{
    auto ret = function(std::forward<Params>(params)...);
    check_al_errors(filename, line);
    return ret;
}

template <typename alFunction, typename... Params>
auto SoundEngine::alCallImpl(const char *filename,
                             const std::uint_fast32_t line,
                             alFunction function,
                             Params... params)
    -> typename std::enable_if_t<std::is_same_v<void, decltype(function(params...))>, bool>
{
    function(std::forward<Params>(params)...);
    return check_al_errors(filename, line);
}

bool SoundEngine::check_alc_errors(const std::string &filename, const std::uint_fast32_t line, ALCdevice *device)
{
    ALCenum error = alcGetError(device);
    if (error != ALC_NO_ERROR)
    {
        std::cerr << "***ERROR*** (" << filename << ": " << line << ")\n";
        switch (error)
        {
        case ALC_INVALID_VALUE:
            std::cerr << "ALC_INVALID_VALUE: an invalid value was passed to an OpenAL function";
            break;
        case ALC_INVALID_DEVICE:
            std::cerr << "ALC_INVALID_DEVICE: a bad device was passed to an OpenAL function";
            break;
        case ALC_INVALID_CONTEXT:
            std::cerr << "ALC_INVALID_CONTEXT: a bad context was passed to an OpenAL function";
            break;
        case ALC_INVALID_ENUM:
            std::cerr << "ALC_INVALID_ENUM: an unknown enum value was passed to an OpenAL function";
            break;
        case ALC_OUT_OF_MEMORY:
            std::cerr << "ALC_OUT_OF_MEMORY: an unknown enum value was passed to an OpenAL function";
            break;
        default:
            std::cerr << "UNKNOWN ALC ERROR: " << error;
        }
        std::cerr << std::endl;
        return false;
    }
    return true;
}

template <typename alcFunction, typename... Params>
auto SoundEngine::alcCallImpl(const char *filename,
                              const std::uint_fast32_t line,
                              alcFunction function,
                              ALCdevice *device,
                              Params... params)
    -> typename std::enable_if_t<std::is_same_v<void, decltype(function(params...))>, bool>
{
    function(std::forward<Params>(params)...);
    return check_alc_errors(filename, line, device);
}

template <typename alcFunction, typename ReturnType, typename... Params>
auto SoundEngine::alcCallImpl(const char *filename,
                              const std::uint_fast32_t line,
                              alcFunction function,
                              ReturnType &returnValue,
                              ALCdevice *device,
                              Params... params)
    -> typename std::enable_if_t<!std::is_same_v<void, decltype(function(params...))>, bool>
{
    returnValue = function(std::forward<Params>(params)...);
    return check_alc_errors(filename, line, device);
}

ALuint SoundEngine::loadSound(const char *path)
{
    SF_INFO sfInfo;
    SNDFILE *sndFile = sf_open(path, SFM_READ, &sfInfo);

    std::cout << "Audio frames: " << sfInfo.frames << std::endl;
    std::cout << "Audio samplerate: " << sfInfo.samplerate << std::endl;
    std::cout << "Audio channels: " << sfInfo.channels << std::endl;
    std::cout << "Audio format: " << sfInfo.format << std::endl;
    std::cout << "Audio sections: " << sfInfo.sections << std::endl;
    std::cout << "Audio seekable: " << sfInfo.seekable << std::endl;

    int bufferSize = sfInfo.frames * sfInfo.channels;
    float *audioData = new float[bufferSize];

    sf_count_t framesRead = sf_readf_float(sndFile, audioData, sfInfo.frames);
    sf_close(sndFile);

    // Create audio buffer
    ALuint bufferId;
    alCall(alGenBuffers, 1, &bufferId);

    int bitsPerSample = 0;
    switch (sfInfo.format & SF_FORMAT_SUBMASK)
    {
    case SF_FORMAT_PCM_S8:
        bitsPerSample = 8;
        break;
    case SF_FORMAT_PCM_16:
        bitsPerSample = 16;
        break;
    case SF_FORMAT_MPEG_LAYER_III:
        // TODO: how to find?
        bitsPerSample = 32;
        break;
    default:
        break;
    }

    ALenum format;
    if (sfInfo.channels == 1 && bitsPerSample == 8)
        format = AL_FORMAT_MONO8;
    else if (sfInfo.channels == 1 && bitsPerSample == 16)
        format = AL_FORMAT_MONO16;
    else if (sfInfo.channels == 2 && bitsPerSample == 8)
        format = AL_FORMAT_STEREO8;
    else if (sfInfo.channels == 2 && bitsPerSample == 16)
        format = AL_FORMAT_STEREO16;
    else if (sfInfo.channels == 2 && bitsPerSample == 32)
        format = AL_FORMAT_STEREO_FLOAT32;
    else
    {
        std::cerr
            << "ERROR: unrecognised audio format: "
            << sfInfo.channels << " channels, "
            << bitsPerSample << " bitsPerSample" << std::endl;
        throw "ERROR: unrecognised audio format";
    }

    alCall(alBufferData, bufferId, format, audioData, bufferSize * sizeof(float), sfInfo.samplerate);
    delete[] audioData;

    return bufferId;
}

void SoundEngine::deleteSound(ALuint bufferId)
{
    alCall(alDeleteBuffers, 1, &bufferId);
}

SoundSource SoundEngine::createSource(ALuint bufferId)
{
    ALuint source;
    alCall(alGenSources, 1, &source);
    alCall(alSourcef, source, AL_PITCH, 1);
    alCall(alSourcef, source, AL_GAIN, 1.0f);
    alCall(alSource3f, source, AL_POSITION, 0, 0, 0);
    alCall(alSource3f, source, AL_VELOCITY, 0, 0, 0);
    alCall(alSourcei, source, AL_LOOPING, AL_TRUE);
    alCall(alSourcei, source, AL_BUFFER, bufferId);

    SoundSource soundSource;
    soundSource.sourceId = source;
    soundSource.bufferId = bufferId;
    return soundSource;
}

void SoundEngine::deleteSource(const SoundSource soundSource)
{
    alCall(alDeleteSources, 1, &soundSource.sourceId);
}

void SoundEngine::playSource(const SoundSource soundSource)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    alCall(alSourcePlay, soundSource.sourceId);
}

void SoundEngine::pauseSource(const SoundSource soundSource)
{
    alCall(alSourcePause, soundSource.sourceId);
}

void SoundEngine::stopSource(const SoundSource soundSource)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    alCall(alSourceStop, soundSource.sourceId);
}

ALint SoundEngine::getSourceState(const SoundSource soundSource)
{
    ALint state;
    alCall(alGetSourcei, soundSource.sourceId, AL_SOURCE_STATE, &state);
    return state;
}

float SoundEngine::getSourceGain(const SoundSource soundSource)
{
    float gain;
    alCall(alGetSourcef, soundSource.sourceId, AL_GAIN, &gain);
    return gain;
}

ALint SoundEngine::getSourceLooping(const SoundSource soundSource)
{
    ALint looping;
    alCall(alGetSourcei, soundSource.sourceId, AL_LOOPING, &looping);
    return looping;
}

float SoundEngine::getSourcePitch(const SoundSource soundSource)
{
    float pitch;
    alCall(alGetSourcef, soundSource.sourceId, AL_PITCH, &pitch);
    return pitch;
}

void SoundEngine::setPlaybackPosition(const SoundSource soundSource, const float positionInSeconds)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    alCall(alSourcef, soundSource.sourceId, AL_SEC_OFFSET, positionInSeconds);
}

void SoundEngine::setSourceGain(const SoundSource soundSource, const float gain)
{
    alCall(alSourcef, soundSource.sourceId, AL_GAIN, gain);
}

void SoundEngine::setSourcePitch(const SoundSource soundSource, const float pitch)
{
    alCall(alSourcef, soundSource.sourceId, AL_PITCH, pitch);
}

void SoundEngine::setSourceLooping(const SoundSource soundSource, const ALint looping)
{
    alCall(alSourcei, soundSource.sourceId, AL_LOOPING, looping);
}

void SoundEngine::setSourcePosition(const SoundSource soundSource, const float x, const float y, const float z)
{
    alCall(alSource3f, soundSource.sourceId, AL_POSITION, x, y, z);
}

void SoundEngine::setListenerPosition(const float x, const float y, const float z)
{
    alCall(alListener3f, AL_POSITION, x, y, z);
}

void SoundEngine::setListenerOrientation(const std::vector<float> *orientation)
{
    alCall(alListenerfv, AL_ORIENTATION, orientation->data());
}
