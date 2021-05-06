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
    apvts.addParameterListener("Mix", this);
    apvts.addParameterListener("Rate", this);
    apvts.addParameterListener("Feedback", this);
}

BadChannelModDemoAudioProcessor::~BadChannelModDemoAudioProcessor()
{
}

//==============================================================================
void BadChannelModDemoAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // delay line size set
    mMod1 = dsp::DelayLine<float, dsp::DelayLineInterpolationTypes::Lagrange3rd>(sampleRate);
    mMod2 = dsp::DelayLine<float, dsp::DelayLineInterpolationTypes::Lagrange3rd>(sampleRate);
    mMod3 = dsp::DelayLine<float, dsp::DelayLineInterpolationTypes::Lagrange3rd>(sampleRate);
    
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
    
    lfoPhaseL = 0.0f;
    lfoPhaseR = 0.0f;
    mFeedback = 0.0f;
    invSampleRate = 1.0f / sampleRate;
    
    // smooth initialize
    delaySmoothed.prepare (delayParam.get(), scalar);
    widthSmoothed.prepare (widthParam.get(), scalar);
    mixSmoothed.prepare (mixParam.get(), scalar);
    rateSmoothed.prepare (rateParam.get(), scalar);
    feedbackSmoothed.prepare (feedbackParam.get(), scalar);
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
    
    for (int channel = 0; channel < totalNumInputChannels; ++channel) {
        
        float* currentChan = buffer.getWritePointer (channel);
    
        for (int sample = 0; sample < numSamples; ++sample) {
            
            // smooth check
            delaySmoothed.loopCheck(channel);
            widthSmoothed.loopCheck(channel);
            mixSmoothed.loopCheck(channel);
            rateSmoothed.loopCheck(channel);
            feedbackSmoothed.loopCheck(channel);
        
            float currentSamp = currentChan[sample];
            float modOut = currentChan[sample];
            float phaseOffset = 0.0f;
        
            processMod (&mMod1, 0, channel, &phaseOffset, currentSamp, &modOut);
            processMod (&mMod2, 1, channel, &phaseOffset, currentSamp, &modOut);
            processMod (&mMod3, 2, channel, &phaseOffset, currentSamp, &modOut);
        
            currentChan[sample] = modOut;
            
            if (channel == 0) {
                lfoPhaseL += rateSmoothed.getCurrent(channel) * invSampleRate;
                if (lfoPhaseL >= 1.0f)
                    lfoPhaseL -= 1.0f;
            } else {
                lfoPhaseR += rateSmoothed.getCurrent(channel) * invSampleRate;
                if (lfoPhaseR >= 1.0f)
                    lfoPhaseR -= 1.0f;
            }
        }
    }
}

float BadChannelModDemoAudioProcessor::lfo (float phase)
{
    return 0.5f + 0.5f * sinf (twoPi * phase);
}

void BadChannelModDemoAudioProcessor::processMod (dsp::DelayLine<float, dsp::DelayLineInterpolationTypes::Lagrange3rd> *dLine, int lineNumber,
                                                  int channel, float *phaseOffset, float sample, float *modOut)
{
    float delayMs = delaySmoothed.getCurrent(channel) * 0.001f;
    float widthMs = widthSmoothed.getCurrent(channel) * 0.001f;
    float lfoOut;
    float weight;
    
    if (channel == 0)
        lfoOut = lfo (lfoPhaseL + *phaseOffset);
    else
        lfoOut = lfo (lfoPhaseR + *phaseOffset);
    
    float delayTimeSamples = (delayMs + widthMs * lfoOut) * (float)getSampleRate();
    float dSample = dLine->popSample(channel, delayTimeSamples);
    
    if (channel == 0)
        weight = (float)lineNumber / 2.0f;
    else
        weight = 1.0f - ((float)lineNumber / 2.0f);
    
    *modOut += dSample * mixSmoothed.getCurrent(channel) * weight;
    
    mFeedback = feedbackSmoothed.getCurrent(channel) * dSample;
    dLine->pushSample(channel, sample + mFeedback);
    
    *phaseOffset += 1.0f / 3.0f;
}

void BadChannelModDemoAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    if (parameterID == "Delay") {
        delayParam = newValue;
        delaySmoothed.update(newValue);
    }
    else if (parameterID == "Width") {
        widthParam = newValue;
        widthSmoothed.update(newValue);
    }
    else if (parameterID == "Mix") {
        mixParam = newValue;
        mixSmoothed.update(newValue);
    }
    else if (parameterID == "Rate") {
        rateParam = newValue;
        rateSmoothed.update(newValue);
    }
    else if (parameterID == "Feedback") {
        feedbackParam = newValue;
        feedbackSmoothed.update(newValue);
    }
}

AudioProcessorValueTreeState::ParameterLayout BadChannelModDemoAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<RangedAudioParameter>> params;
    
    NormalisableRange<float> rateRange (0.03f, 10.0f, 0.001f);
    rateRange.setSkewForCentre (sqrt (0.03f * 10.0f));
    
    params.push_back(std::make_unique<AudioParameterFloat>("Delay", "Delay", NormalisableRange<float>(1.0f, 50.0f, 0.001f), 5.0f, "ms"));
    params.push_back(std::make_unique<AudioParameterFloat>("Width", "Width", NormalisableRange<float>(1.0f, 50.0f, 0.001f), 20.0f, "Hz"));
    params.push_back(std::make_unique<AudioParameterFloat>("Mix", "Mix", 0.0f, 1.0f, 1.0f));
    params.push_back(std::make_unique<AudioParameterFloat>("Rate", "Rate", rateRange, 0.2f, "Hz"));
    params.push_back(std::make_unique<AudioParameterFloat>("Feedback", "Feedback", 0.0f, 0.99f, 0.0f));
    
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
