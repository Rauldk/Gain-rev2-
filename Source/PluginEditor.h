/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "BandEditor.h"



//==============================================================================
/**
*/
class Gainrev2AudioProcessorEditor  : public juce::AudioProcessorEditor, 
                                      public juce::Slider::Listener,
                                      public juce::ChangeListener,
                                      public juce::Timer
    
{
public:
    Gainrev2AudioProcessorEditor (Gainrev2AudioProcessor&);
    ~Gainrev2AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void sliderValueChanged(juce::Slider* slider) override;
    void changeListenerCallback(juce::ChangeBroadcaster* sender) override;
    void timerCallback() override;

private:

    void updateFreqRespone();

    static float getFreqPos(float freq);
    static float getPosForFreq(float pos);


    juce::Slider mGainSlider;
    //BandEditor mSpectrum;
    juce::GroupComponent mFrame;
    juce::Rectangle<int> mPlotFrame;
    juce::Rectangle<int> mBrandingFrame;

    juce::Path mFrequencyResponse;
    juce::Path mAnalyserPath;

    Gainrev2AudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Gainrev2AudioProcessorEditor)
};

