/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Analyser.h"

static int clickRadius = 4;
static float maxDB = 24.0f;

//==============================================================================
Gainrev2AudioProcessorEditor::Gainrev2AudioProcessorEditor (Gainrev2AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    mTooltipWindow->setMillisecondsBeforeTipAppears(1000);

    for (size_t i = 0; i < audioProcessor.getNumBands(); ++i)
    {
        auto* bandEditor = mBandEditor.add(new BandEditor(i, audioProcessor));
        addAndMakeVisible(bandEditor);
    }

    mGainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    mGainSlider.setRange(-60.0f, 0.0f, 0.01f);
    mGainSlider.setValue(0.0f);
    mGainSlider.addListener(this);
    
    mGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);

    // addAndMakeVisible(mSpectrum);
    addAndMakeVisible(mFrame);
    addAndMakeVisible(mGainSlider);
    
    // updateFreqRespone();

    //audioProcessor.addChangeListener(this);
    setSize (1280, 720);

#ifdef JUCE_OPENGL
    openGLContext.attachTo(*getTopLevelComponent());
#endif // JUCE_OPENGL

    audioProcessor.addChangeListener(this);

    juce::Timer::startTimerHz(30);
    
}

Gainrev2AudioProcessorEditor::~Gainrev2AudioProcessorEditor()
{
    //juce::PopupMenu::dismissAllActiveMenus();
    //audioProcessor.removeChangeListener(this);
    juce::PopupMenu::dismissAllActiveMenus();

    audioProcessor.removeChangeListener(this);

    
#ifdef JUCE_OPENGL
    openGLContext.detach();
#endif // JUCE_OPENGL

    
}

//==============================================================================
void Gainrev2AudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(juce::Colours::black);

    
    
   // juce::Rectangle<float> house(300, 120, 200, 170);
   // g.fillCheckerBoard(house, 30, 10, juce::Colours::sandybrown, juce::Colours::saddlebrown);
    audioProcessor.createAnalyserPlot(mAnalyserPath, mPlotFrame, 20.0f, true);
    g.setColour(juce::Colours::white);
    g.strokePath(mAnalyserPath, juce::PathStrokeType(1.0));
    audioProcessor.createAnalyserPlot(mAnalyserPath, mPlotFrame, 20.0f, false);
    g.setColour(juce::Colours::grey);
    g.strokePath(mAnalyserPath, juce::PathStrokeType(1.0));
}

void Gainrev2AudioProcessorEditor::resized()
{
    audioProcessor.setSavedSize({ getWidth(), getHeight() });
    mPlotFrame = getLocalBounds();

    auto bandSpace = mPlotFrame.removeFromBottom(getHeight() / 2);
    auto width = juce::roundToInt(bandSpace.getWidth()) / (mBandEditor.size() + 1);
    
    for (auto* bandEditor : mBandEditor)
    {
        bandEditor->setBounds(bandSpace.removeFromLeft(width));
    }

    mFrame.setBounds(bandSpace.removeFromTop(bandSpace.getHeight() / 2));
    //mGainSlider.setBounds(getWidth() - 80, getHeight() - 80, 80, 80);
    mGainSlider.setBounds(mFrame.getBounds().reduced(8));

    mPlotFrame.reduce(3, 3);
    mBrandingFrame = bandSpace.reduced(5);

    updateFreqRespone();
    
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
   // mGainSlider.setBounds(getWidth() / 2 - 75, getHeight() / 2 - 75, 150, 150);
    //visualiser.setBounds(0, 110, getWidth() - 100, 200);
    //mSpectrum.setBounds(0, 0, getWidth(), getHeight());
    
    
}

void Gainrev2AudioProcessorEditor::timerCallback()
{
    if (audioProcessor.checkForNewAnalyserData())
        repaint(mPlotFrame);
}

void Gainrev2AudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* sender)
{
    juce::ignoreUnused(sender);
    updateFreqRespone();
    repaint();
}

void Gainrev2AudioProcessorEditor::mouseDown(const juce::MouseEvent& event)
{

}

void Gainrev2AudioProcessorEditor::mouseMove(const juce::MouseEvent& event)
{

}

void Gainrev2AudioProcessorEditor::mouseDrag(const juce::MouseEvent& event)
{

}

void Gainrev2AudioProcessorEditor::mouseDoubleClick(const juce::MouseEvent& event)
{

} 

void Gainrev2AudioProcessorEditor::updateFreqRespone()
{

}

float Gainrev2AudioProcessorEditor::getPosForFreq(float freq)
{
    return (std::log(freq / 20.0f) / std::log(2.0f)) / 10.0f;
}

float Gainrev2AudioProcessorEditor::getFreqPos(float pos)
{
    return 20.0f * std::pow(2.0f, pos * 10.0f);
}

float Gainrev2AudioProcessorEditor::getPosForGain(float gain, float top, float bottom)
{
    return juce::jmap(juce::Decibels::gainToDecibels(gain, -maxDB), -maxDB, maxDB, bottom, top);
}

float Gainrev2AudioProcessorEditor::getGainForPos(float pos, float top, float bottom)
{
    return juce::Decibels::decibelsToGain(juce::jmap(pos, bottom, top, -maxDB, maxDB), -maxDB);
}


void Gainrev2AudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &mGainSlider)
    {
        audioProcessor.mGain = mGainSlider.getValue();
    }
}