#pragma once

#include <JuceHeader.h>

class myLookAndFeel : public LookAndFeel_V4
{
public:
    
    myLookAndFeel()
    {
        myKnob = ImageCache::getFromMemory(BinaryData::badChannelKnob_png, BinaryData::badChannelKnob_pngSize);
    }
    
    ~myLookAndFeel()
    {
        setDefaultLookAndFeel (nullptr);
    }
    
    void drawRotarySlider (Graphics & g,
                           int x,
                           int y,
                           int width,
                           int height,
                           float sliderPosProportional,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           Slider & slider ) override
    {
        int radius = jmin(width, height);
        int centerX = int(width / 2);

        float mCenterX = float(myKnob.getWidth()) * .5f;
        float mCenterY = float(myKnob.getHeight()) * .5f;

        float rotation = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle) - 4.0f;
        
        myKnob = myKnob.rescaled(radius, radius, Graphics::ResamplingQuality::highResamplingQuality);
        
        AffineTransform transform;
        float centerImage = float(centerX) - mCenterX;
        
        transform = transform.rotation(rotation, mCenterX, mCenterY).translated(centerImage,0);
        g.drawImageTransformed(myKnob, transform, false);
    }
    
    Label* createSliderTextBox (Slider& slider) override
    {
        auto* l = LookAndFeel_V2::createSliderTextBox (slider);
        
        if (getCurrentColourScheme() == LookAndFeel_V4::getGreyColourScheme() && (slider.getSliderStyle() == Slider::LinearBar
                                                                                  || slider.getSliderStyle() == Slider::LinearBarVertical))
        {
            l->setColour (Label::textColourId, Colours::black.withAlpha (0.7f));
        }
        
        l->setColour (Label::outlineColourId, Colour());
        l->setFont(Font(12));
        
        return l;
    }

    Font getLabelFont (Label&) override
    {
        static auto typeface = Typeface::createSystemTypefaceFor (BinaryData::DogmagicfontRegular_ttf, BinaryData::DogmagicfontRegular_ttfSize);
        Font labelFont = Font(typeface);
        labelFont.setHeight(19.0f);
        return labelFont;
    }

    Font getSliderPopupFont (Slider&) override
    {
        static auto typeface = Typeface::createSystemTypefaceFor (BinaryData::DogmagicfontRegular_ttf, BinaryData::DogmagicfontRegular_ttfSize);
        return Font (typeface);
    }

private:
    
    Image myKnob;
    
};
