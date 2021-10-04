/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <onnxruntime_cxx_api.h>
#include "AudioBufferSampler.h"
#include "InferenceThreadJob.h"


using namespace juce;

//==============================================================================
/**
*/
class NeuroDrumLiveAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    NeuroDrumLiveAudioProcessor();
    ~NeuroDrumLiveAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    void loadFile();
    void generateSample();
    void play();
    const File getModelFile();
    
    juce::Synthesiser mSampler;
    
    std::atomic<float> mAttackVal { 0.1f };
    std::atomic<float> mReleaseVal { 0.9f };
    std::atomic<float> mBrightnessVal { 0.46533436f };
    std::atomic<float> mHardnessVal { 0.6132435f };
    std::atomic<float> mDepthVal { 0.6906892f };
    std::atomic<float> mRoughnessVal { 0.5227648f };
    std::atomic<float> mBoominessVal { 0.6955591f };
    std::atomic<float> mWarmthVal { 0.733622f };
    std::atomic<float> mSharpnessVal { 0.4321724f };

private:
    
    std::unique_ptr<ThreadPool> mThreadPool;
    mutable CriticalSection mMutex;
    
    juce::FileChooser chooser { " Please Load a Model ", juce::File{}, "*.onnx" };
    File mModelFile;

    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NeuroDrumLiveAudioProcessor)
};
