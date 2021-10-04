/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NeuroDrumLiveAudioProcessor::NeuroDrumLiveAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    mSampler.addVoice(new AudioBufferSamplerVoice());
    mSampler.addVoice(new AudioBufferSamplerVoice());
    
    mThreadPool = std::make_unique<ThreadPool>(1);

}

NeuroDrumLiveAudioProcessor::~NeuroDrumLiveAudioProcessor()
{
}

//==============================================================================
const juce::String NeuroDrumLiveAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NeuroDrumLiveAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NeuroDrumLiveAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NeuroDrumLiveAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NeuroDrumLiveAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NeuroDrumLiveAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NeuroDrumLiveAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NeuroDrumLiveAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NeuroDrumLiveAudioProcessor::getProgramName (int index)
{
    return {};
}

void NeuroDrumLiveAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NeuroDrumLiveAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    mSampler.setCurrentPlaybackSampleRate(sampleRate);
}

void NeuroDrumLiveAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NeuroDrumLiveAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void NeuroDrumLiveAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    mSampler.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

}

//==============================================================================
bool NeuroDrumLiveAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NeuroDrumLiveAudioProcessor::createEditor()
{
    return new NeuroDrumLiveAudioProcessorEditor (*this);
}

//==============================================================================
void NeuroDrumLiveAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void NeuroDrumLiveAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NeuroDrumLiveAudioProcessor();
}

void NeuroDrumLiveAudioProcessor::play()
{
    mSampler.noteOn(1, 60, 1.f);
}

void NeuroDrumLiveAudioProcessor::loadFile()
{
    auto chooserFlags = juce::FileBrowserComponent::openMode
                      | juce::FileBrowserComponent::canSelectFiles;

    chooser.launchAsync (chooserFlags, [this] (const juce::FileChooser& fc)
    {
        auto file = fc.getResult();

        if (file == juce::File{})
            return;
        
        mModelFile = file;
        
        //generateSample();
    });
}

void NeuroDrumLiveAudioProcessor::generateSample()
{
    juce::ScopedLock irCalculationlock(mMutex);
    if (mThreadPool)
    {
        mThreadPool->removeAllJobs(true, 1000);
    }

    mThreadPool->addJob(new InferenceThreadJob(*this), true);
}

const File NeuroDrumLiveAudioProcessor::getModelFile()
{
    return mModelFile;
}
