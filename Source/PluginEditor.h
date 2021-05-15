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
                                      //public juce::Slider::Listener,
                                      public juce::ChangeListener,
                                      public juce::Timer
    
{
public:
    Gainrev2AudioProcessorEditor (Gainrev2AudioProcessor&);
    ~Gainrev2AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    //void sliderValueChanged(juce::Slider* slider) override;

    void changeListenerCallback(juce::ChangeBroadcaster* sender) override;
    void timerCallback() override;

    void mouseDown(const juce::MouseEvent& event) override;

    void mouseMove(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;

    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

    void mouseDoubleClick(const juce::MouseEvent& event) override;
    

private:

    void updateFreqRespone();

    static float getFreqPos(float freq);
    static float getPosForFreq(float pos);

    static float getPosForGain(float gain, float top, float bottom);
    static float getGainForPos(float pos, float top, float bottom);


    juce::Slider mGainSlider{ juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow }; 

    juce::GroupComponent mFrame;
    juce::Rectangle<int> mPlotFrame;
    juce::GroupComponent mBrandingFrame;
    juce::Label mName;

    juce::Path mFrequencyResponse;
    juce::Path mAnalyserPath;

    juce::OwnedArray<BandEditor> mBandEditor;

    int mDraggingBand = -1;
    bool mDraggingGain = false;

    juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> mAttachments;
    juce::SharedResourcePointer<juce::TooltipWindow> mTooltipWindow;

    juce::PopupMenu mContextMenu;

    Gainrev2AudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Gainrev2AudioProcessorEditor)

#ifdef JUCE_OPENGL
        juce::OpenGLContext openGLContext;
#endif // JUCE_OPENGL

};

