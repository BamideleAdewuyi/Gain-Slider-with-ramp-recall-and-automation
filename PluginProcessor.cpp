/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// This is the initialiser
GainTutorialAudioProcessor::GainTutorialAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
        // Can set initial gainValue to -5.0. Leave commented out so we can save settings properly
//        gainValue(-5.0f),
        // Initialise treeState treeState(AudioProcessor &processorToConnectTo, UndoManager *undoManagerToUse, const juce::Identifier &valueTreeType, ParameterLayout parameterLayout)
        // In ParameterLayout, we put std::make_unique<juce::AudioParameterFloat>(GAIN_ID, GAIN_NAME, -48.0f, 0.0f, -15.0f)}. Can keep adding more in the same way

treeState(*this, nullptr, "PARAMETER", {std::make_unique<juce::AudioParameterFloat>(GAIN_ID, GAIN_NAME, -48.0f, 0.0f, -15.0f)})

// This is the constructor
#endif
{
    // This will allow us to save the state of the plugin. Create a value tree where we save the parameters
    treeState.state = juce::ValueTree("savedParams");
}

GainTutorialAudioProcessor::~GainTutorialAudioProcessor()
{
}

//==============================================================================
const juce::String GainTutorialAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool GainTutorialAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool GainTutorialAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool GainTutorialAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double GainTutorialAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int GainTutorialAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int GainTutorialAudioProcessor::getCurrentProgram()
{
    return 0;
}

void GainTutorialAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String GainTutorialAudioProcessor::getProgramName (int index)
{
    return {};
}

void GainTutorialAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void GainTutorialAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // This is for initialising any variables that we need before we play anything through
    // plugin. In this case, we have stored the initial value of the plugin when opened
    previousGain = pow (10, *treeState.getRawParameterValue(GAIN_ID) / 20);
}

void GainTutorialAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool GainTutorialAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void GainTutorialAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Create variable for current gain. It is same as previous gain. We make a comparison between what it was
    // before and what it is now. If they are the same, keep gain the same
    float currentGain = pow (10, *treeState.getRawParameterValue(GAIN_ID) / 20);
    
    // Instantiate rawVolume value
//    rawVolume = 0.015;
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    // If they are the same, simply apply gain
    if (currentGain == previousGain)
    {
        buffer.applyGain(currentGain);
    } else{
        // buffer.applyGainRamp(int startSample, int numSamples, float startGain, float endGain)
        // startSample is 0.0. numSamples is number of samples in buffer. startGain and endGain
        // we want to be the previous value to our new value. What this does is instead of instantly
        // going from one value to another, we do it using samples
        buffer.applyGainRamp(0, buffer.getNumSamples(), previousGain, currentGain);
        // Then change previous gain to current gain
        previousGain = currentGain;
    }
    
    

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
//    for (int channel = 0; channel < totalNumInputChannels; ++channel)
//    {
//        auto channelData = buffer.getWritePointer (channel);
//        auto sliderGainValue = treeState.getRawParameterValue(GAIN_ID)->load();
//
//        // ..do something to the data...
//        // We need to find a sample, take it and multiply it by our volume in order
//        // to change it. buffer.getNumSamples() returns the buffer size.
//        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
//        {
//            // We need to multiply all the samples in channelData by our volume variable
//            // we use buffer.getSample(int channel, int sampleIndex). We get the channel, which
//            // will be 0 or 1 for left and right. We then multiply it by gainValue. This will
//            // iterate over the channels and be applied to every single sample
//            // This equation makes plugin logarithmic
////            channelData[sample] = buffer.getSample(channel, sample) * (pow(10, gainValue / 20));
//            // This is a simpler way of converting to decibels
//            channelData[sample] = buffer.getSample(channel, sample) * juce::Decibels::decibelsToGain(sliderGainValue);
//        }
//    }
}

//==============================================================================
bool GainTutorialAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* GainTutorialAudioProcessor::createEditor()
{
    return new GainTutorialAudioProcessorEditor (*this);
}

//==============================================================================
void GainTutorialAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // This means looking at where the plugin values are at particular time and saving them to the XML
    // First create an XML from the tree. This XML object will have the name "PARAMETERS" as that is
    // how treeState was initialised with above
    auto state = treeState.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void GainTutorialAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // This is how the settings are restored
    std::unique_ptr<juce::XmlElement> theParams (getXmlFromBinary (data, sizeInBytes));
    
    // Use if statement for error checking.
    // If the xml element is not nothing ie if it is there
    if (theParams.get() != nullptr)
        // If xml element has the same tag name as one we created
        if (theParams->hasTagName (treeState.state.getType()))
            // Load the processor state from the value tree saved
            treeState.replaceState (juce::ValueTree::fromXml (*theParams));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GainTutorialAudioProcessor();
}
