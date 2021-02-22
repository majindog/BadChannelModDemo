#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "myLookAndFeel.h"

//==============================================================================
/**
*/
class BadChannelModDemoAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    BadChannelModDemoAudioProcessorEditor (BadChannelModDemoAudioProcessor&);
    ~BadChannelModDemoAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    
    BadChannelModDemoAudioProcessor& processor;

    int editorWidth = 600;
    int editorHeight = 300;
    
    float paramSize = 0.23f;

    //======================================
    
    OwnedArray<Slider> sliders;
    OwnedArray<Label> labels;
    Array<Component*> components;
    
    typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    OwnedArray<SliderAttachment> sliderAttachments;
    
    //======================================
    
    ImageComponent mGuiComponent;
    myLookAndFeel mLookAndFeel;
    GlowEffect glow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BadChannelModDemoAudioProcessorEditor)
};
