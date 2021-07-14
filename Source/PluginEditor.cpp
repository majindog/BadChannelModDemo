#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BadChannelModDemoAudioProcessorEditor::BadChannelModDemoAudioProcessorEditor (BadChannelModDemoAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    // gui image
    auto guiImage = ImageCache::getFromMemory(BinaryData::bcModDemoGUI_png, BinaryData::bcModDemoGUI_pngSize);
    
    if(!guiImage.isNull())
        mGuiComponent.setImage(guiImage, RectanglePlacement::stretchToFit);
    else
        jassert(!guiImage.isNull());
    
    addAndMakeVisible(mGuiComponent);
    
    // glow set
    glow.setGlowProperties(1.5f, Colours::wheat);
    
    const Array<AudioProcessorParameter*> parameters = processor.getParameters();
    
    for (int i = 0; i < parameters.size(); ++i) {
        
        AudioProcessorParameterWithID* parameter = dynamic_cast<AudioProcessorParameterWithID*> (parameters[i]);
            
        Slider* aSlider;
        sliders.add (aSlider = new Slider());

        aSlider->setLookAndFeel(&mLookAndFeel);
        aSlider->setSliderStyle(Slider::SliderStyle::RotaryHorizontalVerticalDrag);
        aSlider->setTextValueSuffix (parameter->label);
        aSlider->setTextBoxStyle (Slider::TextBoxBelow, false, 60, 20);
        aSlider->setColour(Slider::textBoxBackgroundColourId, Colours::transparentBlack);

        SliderAttachment* aSliderAttachment;
        sliderAttachments.add (aSliderAttachment = new SliderAttachment (processor.apvts, parameter->paramID, *aSlider));
        
        // set display percision
        aSlider->textFromValueFunction = nullptr;
        aSlider->setNumDecimalPlacesToDisplay(2);
                
        components.add (aSlider);
                
        // label
        Label* aLabel;
        labels.add (aLabel = new Label (parameter->name, parameter->name));
                
        aLabel->attachToComponent (components.getLast(), false);
        aLabel->Label::setJustificationType(Justification::centred);
        aLabel->setComponentEffect(&glow);
        aLabel->setLookAndFeel(&mLookAndFeel);

        addAndMakeVisible (aLabel);
        
        components.getLast()->setName (parameter->name);
        components.getLast()->setComponentID (parameter->paramID);
        addAndMakeVisible (components.getLast());
    }
    
    setSize (editorWidth, editorHeight);
}

BadChannelModDemoAudioProcessorEditor::~BadChannelModDemoAudioProcessorEditor()
{
    myLookAndFeel::setDefaultLookAndFeel (nullptr);
    
    for (int i = 0; i < components.size(); i++) {
        components[i]->setLookAndFeel (nullptr);
        labels[i]->setLookAndFeel (nullptr);
    }
}

//==============================================================================
void BadChannelModDemoAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(Colours::black);
}

void BadChannelModDemoAudioProcessorEditor::resized()
{
    // gui image
    mGuiComponent.setBoundsRelative(0.0f, 0.0f, 1.0f, 1.0f);
    
    // param gui
    for (int i = 0; i < components.size(); ++i) {

        auto sliderName = components[i]->getName();
                    
        // mod sliders
        if(sliderName == "Rate")
            components[i]->setBoundsRelative(0.05f, 0.475f, paramSize, paramSize);
        else if(sliderName == "Width")
            components[i]->setBoundsRelative(0.22f, 0.475f, paramSize, paramSize);
        else if(sliderName == "Delay")
            components[i]->setBoundsRelative(0.39f, 0.475f, paramSize, paramSize);
        else if(sliderName == "Feedback")
            components[i]->setBoundsRelative(0.56f, 0.475f, paramSize, paramSize);
        else if(sliderName == "Mix")
            components[i]->setBoundsRelative(0.73f, 0.475f, paramSize, paramSize);

        // glow
        components[i]->setComponentEffect(&glow);
    }
}
