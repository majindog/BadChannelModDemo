#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BadChannelModDemoAudioProcessor::BadChannelModDemoAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ) , apvts (*this, &mUndoManager, "Parameters", createParameters())
#endif
{
    apvts.addParameterListener("Delay", this);
    apvts.addParameterListener("Width", this);
    apvts.addParameterListener("Depth", this);
    apvts.addParameterListener("Rate", this);
    apvts.addParameterListener("Feedback", this);
}

BadChannelModDemoAudioProcessor::~BadChannelModDemoAudioProcessor()
{
}

//==============================================================================
void BadChannelModDemoAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // dsp spec
    dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumInputChannels();
    
    // mod
    mMod1.reset();
    mMod2.reset();
    mMod3.reset();
    mMod1.prepare(spec);
    mMod2.prepare(spec);
    mMod3.prepare(spec);
    
    lfoPhase = 0.0f;
    feedbackLeft = 0.0f;
    feedbackRight = 0.0f;
    inverseSampleRate = 1.0f / sampleRate;
    
    // smooth initialize
    delaySmoothed.prepare (delayParam.get());
    widthSmoothed.prepare (widthParam.get());
    depthSmoothed.prepare (depthParam.get());
    rateSmoothed.prepare (rateParam.get());
    feedbackSmoothed.prepare (feedbackParam.get());
}

void BadChannelModDemoAudioProcessor::releaseResources()
{
}

void BadChannelModDemoAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    auto numSamples = buffer.getNumSamples();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, numSamples);

    //======================================
    
    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = buffer.getWritePointer(1);
    
    // smooth set
    delaySmoothed.setParam(numSamples);
    widthSmoothed.setParam(numSamples);
    depthSmoothed.setParam(numSamples);
    rateSmoothed.setParam(numSamples);
    feedbackSmoothed.setParam(numSamples);
    
    for (int sample = 0; sample < numSamples; ++sample) {
        
        delaySmoothed.loopCheck();
        widthSmoothed.loopCheck();
        depthSmoothed.loopCheck();
        rateSmoothed.loopCheck();
        feedbackSmoothed.loopCheck();
        
        float leftSample = leftChannel[sample];
        float rightSample = rightChannel[sample];
        
        float outL = leftChannel[sample];
        float outR = rightChannel[sample];
        
        float phaseOffset = 0.0f;
        
        processMod (&mMod1, 0, &phaseOffset, leftSample, rightSample, &outL, &outR);
        processMod (&mMod2, 1, &phaseOffset, leftSample, rightSample, &outL, &outR);
        processMod (&mMod3, 2, &phaseOffset, leftSample, rightSample, &outL, &outR);
        
        leftChannel[sample] = outL;
        rightChannel[sample] = outR;
        
        lfoPhase += rateSmoothed.getCurrent() * inverseSampleRate;
        if (lfoPhase >= 1.0f)
            lfoPhase -= 1.0f;
    }
    
    delaySmoothed.outCheck();
    widthSmoothed.outCheck();
    depthSmoothed.outCheck();
    rateSmoothed.outCheck();
    feedbackSmoothed.outCheck();
}

float BadChannelModDemoAudioProcessor::lfo (float phase)
{
    return 0.5f + 0.5f * sinf (twoPi * phase);
}

void BadChannelModDemoAudioProcessor::processMod (dsp::DelayLine<float, dsp::DelayLineInterpolationTypes::Lagrange3rd> *dLine, int lineNumber,
                                                  float *phaseOffset, float leftSample, float rightSample, float *outL, float *outR)
{
    float delayMs = delaySmoothed.getCurrent() * 0.001f;
    float widthMs = widthSmoothed.getCurrent() * 0.001f;
    float lfoOut = lfo (lfoPhase + *phaseOffset);
    
    float delayTimeSamples = (delayMs + widthMs * lfoOut) * (float)getSampleRate();
    
    float dSampleL = dLine->popSample(0, delayTimeSamples);
    float dSampleR = dLine->popSample(1, delayTimeSamples);
    
    float weightL = (float)lineNumber / 2.0f;
    float weightR = 1.0f - weightL;
    
    *outL += dSampleL * depthSmoothed.getCurrent() * weightL;
    *outR += dSampleR * depthSmoothed.getCurrent() * weightR;
    
    feedbackLeft = feedbackSmoothed.getCurrent() * dSampleL;
    feedbackRight = feedbackSmoothed.getCurrent() * dSampleR;
    
    dLine->pushSample(0, leftSample + feedbackLeft);
    dLine->pushSample(1, rightSample + feedbackRight);
    
    *phaseOffset += 1.0f / 3.0f;
}

void BadChannelModDemoAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    if (parameterID == "Delay") {
        delayParam = newValue;
        delaySmoothed.updateTarget(newValue);
    }
    else if (parameterID == "Width") {
        widthParam = newValue;
        widthSmoothed.updateTarget(newValue);
    }
    else if (parameterID == "Depth") {
        depthParam = newValue;
        depthSmoothed.updateTarget(newValue);
    }
    else if (parameterID == "Frequency") {
        rateParam = newValue;
        rateSmoothed.updateTarget(newValue);
    }
    else if (parameterID == "Feedback") {
        feedbackParam = newValue;
        feedbackSmoothed.updateTarget(newValue);
    }
}

AudioProcessorValueTreeState::ParameterLayout BadChannelModDemoAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<RangedAudioParameter>> params;
    
    params.push_back(std::make_unique<AudioParameterFloat>("Delay", "Delay", NormalisableRange<float>(1.0f, 50.0f, 0.01f), 5.0f, "ms"));
    params.push_back(std::make_unique<AudioParameterFloat>("Width", "Width", NormalisableRange<float>(1.0f, 50.0f, 0.01), 20.0f, "Hz"));
    params.push_back(std::make_unique<AudioParameterFloat>("Depth", "Depth", 0.0f, 1.0f, 1.0f));
    params.push_back(std::make_unique<AudioParameterFloat>("Rate", "Rate", NormalisableRange<float>(0.03f, 10.0f, 0.01f), 0.2f, "Hz"));
    params.push_back(std::make_unique<AudioParameterFloat>("Feedback", "Feedback", 0.0f, 0.99f, 0.0f));
    params.push_back(std::make_unique<AudioParameterFloat>("Mix", "Mix", 0.0f, 1.0f, 0.5f));
    
    return { params.begin(), params.end() };
}

AudioProcessorValueTreeState& BadChannelModDemoAudioProcessor::getValueTreeState()
{
    return apvts;
}

void BadChannelModDemoAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    MemoryOutputStream stream(destData, false);
    apvts.state.writeToStream (stream);
}

void BadChannelModDemoAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    ValueTree tree = ValueTree::readFromData (data, sizeInBytes);
    if (tree.isValid()) {
        apvts.state = tree;
    }
}

//==============================================================================
bool BadChannelModDemoAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* BadChannelModDemoAudioProcessor::createEditor()
{
    return new BadChannelModDemoAudioProcessorEditor (*this);
}

//==============================================================================
#ifndef JucePlugin_PreferredChannelConfigurations
bool BadChannelModDemoAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
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

//==============================================================================
const juce::String BadChannelModDemoAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BadChannelModDemoAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool BadChannelModDemoAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool BadChannelModDemoAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double BadChannelModDemoAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BadChannelModDemoAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int BadChannelModDemoAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BadChannelModDemoAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BadChannelModDemoAudioProcessor::getProgramName (int index)
{
    return {};
}

void BadChannelModDemoAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BadChannelModDemoAudioProcessor();
}
