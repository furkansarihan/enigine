#include "sound_engine.h"

#define DR_WAV_IMPLEMENTATION
#include <dr_wav.h>

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

ALuint SoundEngine::loadWav(const char *path)
{
    // Read audio file
    drwav wav;
    if (!drwav_init_file(&wav, path, NULL))
    {
        throw "ERROR: dr_wav could not init audio file";
    }

    std::cout << "Audio channels: " << wav.channels << std::endl;
    std::cout << "Audio sample rate: " << wav.sampleRate << std::endl;
    std::cout << "Audio total PCM Frame count: " << wav.totalPCMFrameCount << std::endl;
    std::cout << "Audio bits per sample: " << wav.bitsPerSample << std::endl;

    ALenum format;
    if (wav.channels == 1 && wav.bitsPerSample == 8)
        format = AL_FORMAT_MONO8;
    else if (wav.channels == 1 && wav.bitsPerSample == 16)
        format = AL_FORMAT_MONO16;
    else if (wav.channels == 2 && wav.bitsPerSample == 8)
        format = AL_FORMAT_STEREO8;
    else if (wav.channels == 2 && wav.bitsPerSample == 16)
        format = AL_FORMAT_STEREO16;
    else
    {
        std::cerr
            << "ERROR: unrecognised wave format: "
            << wav.channels << " channels, "
            << wav.bitsPerSample << " bitsPerSample" << std::endl;
        throw "ERROR: unrecognised wave format";
    }

    // Create audio buffer
    ALuint buffer;
    alCall(alGenBuffers, 1, &buffer);

    if (format == AL_FORMAT_MONO8 || format == AL_FORMAT_STEREO8)
    {
        int32_t size = (size_t)wav.totalPCMFrameCount * wav.channels * sizeof(int8_t);
        int8_t *pSampleData = (int8_t *)malloc(size);
        drwav_read_pcm_frames(&wav, wav.totalPCMFrameCount, pSampleData);
        alCall(alBufferData, buffer, format, pSampleData, size, wav.sampleRate);
        drwav_uninit(&wav);
    }
    else if (format == AL_FORMAT_MONO16 || format == AL_FORMAT_STEREO16)
    {
        int32_t size = (size_t)wav.totalPCMFrameCount * wav.channels * sizeof(int16_t);
        int16_t *pSampleData = (int16_t *)malloc(size);
        drwav_read_pcm_frames_s16(&wav, wav.totalPCMFrameCount, pSampleData);
        alCall(alBufferData, buffer, format, pSampleData, size, wav.sampleRate);
        drwav_uninit(&wav);
    }

    return buffer;
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

glm::vec3 SoundEngine::getSourcePosition(const SoundSource soundSource)
{
    ALfloat sourcePosition[3];
    alCall(alGetSource3f, soundSource.sourceId, AL_POSITION, &sourcePosition[0], &sourcePosition[1], &sourcePosition[2]);
    glm::vec3 position(sourcePosition[0], sourcePosition[1], sourcePosition[2]);
    return position;
}

float SoundEngine::getPlayerPosition(const SoundSource soundSource)
{
    float position;
    alCall(alGetSourcef, soundSource.sourceId, AL_SEC_OFFSET, &position);
    return position;
}

void SoundEngine::setPlayerPosition(const SoundSource soundSource, const float positionInSeconds)
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
    std::unique_lock<std::mutex> lock(m_mutex);
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
