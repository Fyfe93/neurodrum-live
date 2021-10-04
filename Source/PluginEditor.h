/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class NeuroDrumLiveAudioProcessorEditor  : public juce::AudioProcessorEditor, public Slider::Listener
{
public:
    NeuroDrumLiveAudioProcessorEditor (NeuroDrumLiveAudioProcessor&);
    ~NeuroDrumLiveAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void sliderValueChanged (Slider* slider) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    NeuroDrumLiveAudioProcessor& audioProcessor;
    
    juce::TextButton mLoadButton;
    juce::TextButton mPlayButton;
    juce::TextButton mGenerateButton;
    
    std::vector<float> paramDefaultValues { 0.1f, 0.9f, 0.46533436f, 0.6132435f, 0.6906892f, 0.5227648f, 0.6955591f, 0.733622f, 0.4321724f };
    
    enum params
    {
        attack = 0,
        release,
        brightness,
        hardness,
        depth,
        roughness,
        boominess,
        warmth,
        sharpness,
        totalParams
    };
    
    Slider mSliders[params::totalParams];
    Label mLabels[params::totalParams];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NeuroDrumLiveAudioProcessorEditor)
};
