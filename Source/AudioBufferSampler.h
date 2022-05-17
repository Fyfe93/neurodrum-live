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
                             double maxSampleLengthSeconds);

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
    bool canPlaySound (SynthesiserSound* sound) override;

    void startNote (int midiNoteNumber, float velocity, SynthesiserSound* s, int pitchWheel) override;
    
    void stopNote (float velocity, bool allowTailOff) override;

    void pitchWheelMoved (int newValue) override {}
    void controllerMoved (int controllerNumber, int newValue) override {}

    void renderNextBlock (AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;
    
    using SynthesiserVoice::renderNextBlock;

private:
    //==============================================================================
    double pitchRatio = 0;
    double sourceSamplePosition = 0;
    float lgain = 0, rgain = 0;

    ADSR adsr;

    JUCE_LEAK_DETECTOR (AudioBufferSamplerVoice)
};
