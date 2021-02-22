#pragma once

#include <JuceHeader.h>
#include "ParamSmoother.h"

//==============================================================================
/**
*/
class BadChannelModDemoAudioProcessor  : public juce::AudioProcessor,
                                         public AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    BadChannelModDemoAudioProcessor();
    ~BadChannelModDemoAudioProcessor() override;

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
    
    //============================================================================== MODULATION
    
    dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> mMod1{96000};
    dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> mMod2{96000};
    dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> mMod3{96000};
    
    float lfoPhase;
    float feedbackLeft;
    float feedbackRight;
    float inverseSampleRate;
    float twoPi = 2.0f * M_PI;
    
    float lfo (float phase);
    void processMod (dsp::DelayLine<float, dsp::DelayLineInterpolationTypes::Lagrange3rd> *dLine, int lineNumber,
                     float *phaseOffset, float leftSample, float rightSample, float *outL, float *outR);
    
    AudioProcessorValueTreeState& getValueTreeState();
    void parameterChanged (const String &parameterID, float newValue) override;
    
    Atomic<float> delayParam { 5.0f };
    Atomic<float> widthParam { 20.0f };
    Atomic<float> depthParam { 1.0f };
    Atomic<float> rateParam { 0.2f };
    Atomic<float> feedbackParam { 0.0f };
    
    UndoManager mUndoManager;
    AudioProcessorValueTreeState apvts;
    
    ParamSmoother delaySmoothed;
    ParamSmoother widthSmoothed;
    ParamSmoother depthSmoothed;
    ParamSmoother rateSmoothed;
    ParamSmoother feedbackSmoothed;

private:
    
    AudioProcessorValueTreeState::ParameterLayout createParameters();
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BadChannelModDemoAudioProcessor)
};
