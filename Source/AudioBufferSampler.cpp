/*
  ==============================================================================

    AudioBufferSampler.cpp
    Created: 1 Oct 2021 10:03:25pm
    Author:  Andrew Fyfe

  ==============================================================================
*/

#include "AudioBufferSampler.h"

AudioBufferSamplerSound::AudioBufferSamplerSound (const String& soundName,
                         AudioBuffer<float>& source,
                         double sourceSamplerate,
                         const BigInteger& notes,
                         int midiNoteForNormalPitch,
                         double attackTimeSecs,
                         double releaseTimeSecs,
                         double maxSampleLengthSeconds)
    : name (soundName),
      sourceSampleRate (sourceSamplerate),
      midiNotes (notes),
      midiRootNote (midiNoteForNormalPitch)
{
    if (sourceSampleRate > 0 && source.getNumSamples() > 0)
    {
        length = jmin ((int) source.getNumSamples(),
                       (int) (maxSampleLengthSeconds * sourceSampleRate));
        
        data.reset(new AudioBuffer<float>());
        
        *data = source;

        params.attack  = static_cast<float> (attackTimeSecs);
        params.release = static_cast<float> (releaseTimeSecs);
    }
}


bool AudioBufferSamplerVoice::canPlaySound (SynthesiserSound* sound)
{
    return dynamic_cast<const AudioBufferSamplerSound*> (sound) != nullptr;
}

void AudioBufferSamplerVoice::startNote (int midiNoteNumber, float velocity, SynthesiserSound* s, int pitchWheel)
{
    if (auto* sound = dynamic_cast<const AudioBufferSamplerSound*> (s))
    {
        pitchRatio = std::pow (2.0, (midiNoteNumber - sound->midiRootNote) / 12.0)
                        * sound->sourceSampleRate / getSampleRate();

        sourceSamplePosition = 0.0;
        lgain = velocity;
        rgain = velocity;

        adsr.setSampleRate (sound->sourceSampleRate);
        adsr.setParameters (sound->params);

        adsr.noteOn();
    }
    else
    {
        jassertfalse; // this object can only play SamplerSounds!
    }
}

void AudioBufferSamplerVoice::stopNote (float velocity, bool allowTailOff)
{
    if (allowTailOff)
    {
        adsr.noteOff();
    }
    else
    {
        clearCurrentNote();
        adsr.reset();
    }
}

void AudioBufferSamplerVoice::renderNextBlock (AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (auto* playingSound = static_cast<AudioBufferSamplerSound*> (getCurrentlyPlayingSound().get()))
    {
        auto& data = *playingSound->data;
        const float* const inL = data.getReadPointer (0);
        const float* const inR = data.getNumChannels() > 1 ? data.getReadPointer (1) : nullptr;

        float* outL = outputBuffer.getWritePointer (0, startSample);
        float* outR = outputBuffer.getNumChannels() > 1 ? outputBuffer.getWritePointer (1, startSample) : nullptr;

        while (--numSamples >= 0)
        {
            auto pos = (int) sourceSamplePosition;
            auto alpha = (float) (sourceSamplePosition - pos);
            auto invAlpha = 1.0f - alpha;

            // just using a very simple linear interpolation here..
            float l = (inL[pos] * invAlpha + inL[pos + 1] * alpha);
            float r = (inR != nullptr) ? (inR[pos] * invAlpha + inR[pos + 1] * alpha)
                                       : l;

            auto envelopeValue = adsr.getNextSample();

            l *= lgain * envelopeValue;
            r *= rgain * envelopeValue;

            if (outR != nullptr)
            {
                *outL++ += l;
                *outR++ += r;
            }
            else
            {
                *outL++ += (l + r) * 0.5f;
            }

            sourceSamplePosition += pitchRatio;

            if (sourceSamplePosition > playingSound->length)
            {
                stopNote (0.0f, false);
                break;
            }
        }
    }
}
