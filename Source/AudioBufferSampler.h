/*
  ==============================================================================

    AudioBufferSampler.h
    Created: 1 Oct 2021 10:03:25pm
    Author:  Andrew Fyfe

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

using namespace juce;

class AudioBufferSamplerVoice;

class  AudioBufferSamplerSound    : public juce::SynthesiserSound
{
public:
    //==============================================================================
    AudioBufferSamplerSound (const String& soundName,
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
//            for (int i = 0; i < data->getNumSamples(); ++i) {
//                DBG(data->getSample(0, i));
//            }

            params.attack  = static_cast<float> (attackTimeSecs);
            params.release = static_cast<float> (releaseTimeSecs);
        }
    }

    /** Destructor. */
    ~AudioBufferSamplerSound() override { };

    //==============================================================================
    /** Returns the sample's name */
    const String& getName() const noexcept                  { return name; }

    /** Returns the audio sample data.
        This could return nullptr if there was a problem loading the data.
    */
    AudioBuffer<float>* getAudioData() const noexcept       { return data.get(); }

    //==============================================================================
    /** Changes the parameters of the ADSR envelope which will be applied to the sample. */
    void setEnvelopeParameters (ADSR::Parameters parametersToUse)    { params = parametersToUse; }

    //==============================================================================
    bool appliesToNote (int midiNoteNumber) override { return midiNotes[midiNoteNumber]; }
    bool appliesToChannel (int midiChannel) override { return true; };

private:
    //==============================================================================
    friend class AudioBufferSamplerVoice;

    String name;
    std::unique_ptr<AudioBuffer<float>> data;
    double sourceSampleRate;
    BigInteger midiNotes;
    int length = 0, midiRootNote = 0;

    ADSR::Parameters params;

    JUCE_LEAK_DETECTOR (AudioBufferSamplerSound)
};

class AudioBufferSamplerVoice : public SynthesiserVoice
{
public:
    //==============================================================================
    /** Creates a AudioBufferSamplerVoice. */
    AudioBufferSamplerVoice() {}

    /** Destructor. */
    ~AudioBufferSamplerVoice() override {}

    //==============================================================================
    bool canPlaySound (SynthesiserSound* sound) override
    {
        return dynamic_cast<const AudioBufferSamplerSound*> (sound) != nullptr;
    }

    void startNote (int midiNoteNumber, float velocity, SynthesiserSound* s, int pitchWheel) override
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
    
    void stopNote (float velocity, bool allowTailOff) override
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

    void pitchWheelMoved (int newValue) override {}
    void controllerMoved (int controllerNumber, int newValue) override {}

    void renderNextBlock (AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
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
    
    using SynthesiserVoice::renderNextBlock;

private:
    //==============================================================================
    double pitchRatio = 0;
    double sourceSamplePosition = 0;
    float lgain = 0, rgain = 0;

    ADSR adsr;

    JUCE_LEAK_DETECTOR (AudioBufferSamplerVoice)
};
