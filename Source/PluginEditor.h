/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"



//==============================================================================
/**
*/
class Gainrev2AudioProcessorEditor  : public juce::AudioProcessorEditor, 
                                      public juce::Slider::Listener
{
public:
    Gainrev2AudioProcessorEditor (Gainrev2AudioProcessor&);
    ~Gainrev2AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void sliderValueChanged(juce::Slider* slider) override;

private:
    juce::Slider mGainSlider;
    

    Gainrev2AudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Gainrev2AudioProcessorEditor)
};
