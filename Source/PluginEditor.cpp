/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Gainrev2AudioProcessorEditor::Gainrev2AudioProcessorEditor (Gainrev2AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    mGainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    mGainSlider.setRange(-60.0f, 0.0f, 0.01f);
    mGainSlider.setValue(-20.0f);
    mGainSlider.addListener(this);
    addAndMakeVisible(mGainSlider);
    mGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);

    addAndMakeVisible(visualiser);


    setSize (1280, 720);
}

Gainrev2AudioProcessorEditor::~Gainrev2AudioProcessorEditor()
{
}

//==============================================================================
void Gainrev2AudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(juce::Colours::black);
}

void Gainrev2AudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
   // mGainSlider.setBounds(getWidth() / 2 - 75, getHeight() / 2 - 75, 150, 150);
    mGainSlider.setBounds(getWidth() - 80, getHeight() - 80, 80, 80);
}

void Gainrev2AudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &mGainSlider)
    {
        audioProcessor.mGain = mGainSlider.getValue();
    }
}