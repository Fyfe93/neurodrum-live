/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NeuroDrumLiveAudioProcessorEditor::NeuroDrumLiveAudioProcessorEditor (NeuroDrumLiveAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    mLoadButton.onClick = [&]() { audioProcessor.loadFile(); };
    mPlayButton.onClick = [&]() { audioProcessor.play(); };
    mGenerateButton.onClick = [&]() { audioProcessor.generateSample(); };
    
    mLoadButton.setButtonText("Load Model");
    mPlayButton.setButtonText("Play");
    mGenerateButton.setButtonText("Generate");
    
    addAndMakeVisible(mLoadButton);
    addAndMakeVisible(mPlayButton);
    addAndMakeVisible(mGenerateButton);
    
    mLabels[params::attack].setText("Attack", NotificationType::dontSendNotification);
    mLabels[params::release].setText("Release", NotificationType::dontSendNotification);
    mLabels[params::brightness].setText("Brightness", NotificationType::dontSendNotification);
    mLabels[params::hardness].setText("Hardness", NotificationType::dontSendNotification);
    mLabels[params::depth].setText("Depth", NotificationType::dontSendNotification);
    mLabels[params::roughness].setText("Roughness", NotificationType::dontSendNotification);
    mLabels[params::boominess].setText("Boominess", NotificationType::dontSendNotification);
    mLabels[params::warmth].setText("Warmth", NotificationType::dontSendNotification);
    mLabels[params::sharpness].setText("Sharpness", NotificationType::dontSendNotification);
    
    for (int i = 0 ; i < params::totalParams; ++i) {
        mSliders[i].setSliderStyle(Slider::SliderStyle::LinearHorizontal);
        mSliders[i].setRange(Range<double> {0.0, 1.0}, 0.001);
        mSliders[i].setDoubleClickReturnValue(true, paramDefaultValues[i]);
        mSliders[i].setValue(paramDefaultValues[i]);
        addAndMakeVisible(mSliders[i]);
        addAndMakeVisible(mLabels[i]);
        mSliders[i].addListener(this);
    }
    
    
    setSize (600, 400);
}

NeuroDrumLiveAudioProcessorEditor::~NeuroDrumLiveAudioProcessorEditor()
{
}

//==============================================================================
void NeuroDrumLiveAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (30.0f);
    auto bounds = getLocalBounds();
    g.drawFittedText ("NEURO DRUM LIVE", bounds.removeFromTop(0.15f*getHeight()), juce::Justification::centred, 1);
}

void NeuroDrumLiveAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    mLoadButton.setBoundsRelative(0.075f, 0.8f, 0.25f, 0.125f);
    mGenerateButton.setBoundsRelative(0.375f, 0.8f, 0.25f, 0.125f);
    mPlayButton.setBoundsRelative(0.675f, 0.8f, 0.25f, 0.125f);
    
    Rectangle<float> boundsSliders { 0.075f, 0.125f, 0.75f, 0.8f };
    Rectangle<float> boundsLabels { 0.825f, 0.125f, 0.125f, 0.8f };
    
    for (int i = 0 ; i < params::totalParams; ++i) {
        Rectangle<float> sliderRect = boundsSliders.removeFromTop(0.07f);
        Rectangle<float> labelRect = boundsLabels.removeFromTop(0.07f);
        mSliders[i].setBoundsRelative(sliderRect);
        mLabels[i].setBoundsRelative(labelRect);
    }
}

void NeuroDrumLiveAudioProcessorEditor::sliderValueChanged (Slider* slider)
{
    if (slider == &mSliders[params::attack])
    {
        const float value = slider->getValue();
        audioProcessor.mAttackVal.store(value);
    }
    else if (slider == &mSliders[params::release])
    {
        const float value = slider->getValue();
        audioProcessor.mReleaseVal.store(value);
    }
    else if (slider == &mSliders[params::brightness])
    {
        const float value = slider->getValue();
        audioProcessor.mBrightnessVal.store(value);
    }
    else if (slider == &mSliders[params::hardness])
    {
        const float value = slider->getValue();
        audioProcessor.mHardnessVal.store(value);
    }
    else if (slider == &mSliders[params::depth])
    {
        const float value = slider->getValue();
        audioProcessor.mDepthVal.store(value);
    }
    else if (slider == &mSliders[params::roughness])
    {
        const float value = slider->getValue();
        audioProcessor.mRoughnessVal.store(value);
    }
    else if (slider == &mSliders[params::boominess])
    {
        const float value = slider->getValue();
        audioProcessor.mBoominessVal.store(value);
    }
    else if (slider == &mSliders[params::warmth])
    {
        const float value = slider->getValue();
        audioProcessor.mWarmthVal.store(value);
    }
    else if (slider == &mSliders[params::sharpness])
    {
        const float value = slider->getValue();
        audioProcessor.mSharpnessVal.store(value);
    }
    else
    {
        return;
    }
}
