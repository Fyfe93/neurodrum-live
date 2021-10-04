/*
  ==============================================================================

    InferenceThreadJob.h
    Created: 2 Oct 2021 1:54:57pm
    Author:  Andrew Fyfe

  ==============================================================================
*/

#pragma once

#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

using namespace juce;

class NeuroDrumLiveAudioProcessor; // forward declare

class InferenceThreadJob : public ThreadPoolJob
{
public:
    
    explicit InferenceThreadJob(NeuroDrumLiveAudioProcessor& gluProcessor);
    virtual ~InferenceThreadJob();
    virtual auto runJob() -> JobStatus;
    
private:
    NeuroDrumLiveAudioProcessor& mProcessor;
    
    // Prevent uncontrolled usage
    InferenceThreadJob(const InferenceThreadJob&);
    InferenceThreadJob& operator=(const InferenceThreadJob&);
};
