/*
  ==============================================================================

    BandEditor.cpp
    Created: 9 May 2021 11:24:38pm
    Author:  

  ==============================================================================
*/

#include "BandEditor.h"

BandEditor::BandEditor(size_t i, Gainrev2AudioProcessor& p) : index(i), audioProcessor(p)
{
    bFrame.setText(audioProcessor.getBandName(index));
    bFrame.setTextLabelPosition(juce::Justification::centred);
    bFrame.setColour(juce::GroupComponent::textColourId, audioProcessor.getBandColour(index));
    bFrame.setColour(juce::GroupComponent::outlineColourId, audioProcessor.getBandColour(index));

    addAndMakeVisible(bFrame);

    if (auto* choiceParameter = dynamic_cast<juce::AudioParameterChoice*>(audioProcessor.getPluginState().getParameter(audioProcessor.getTypeParamName(index))))
        bFilterType.addItemList(choiceParameter->choices, 1);

    addAndMakeVisible(bFilterType);
    bBoxAttachments.add(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(audioProcessor.getPluginState(), audioProcessor.getTypeParamName(index), bFilterType));

    addAndMakeVisible(bFrequency);
    bAttachments.add(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.getPluginState(), audioProcessor.getFrequencyParamName(index), bFrequency));
    bFrequency.setTooltip("Filter's frequency");

    addAndMakeVisible(bQuality);
    bAttachments.add(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.getPluginState(), audioProcessor.getQualityParamName(index), bQuality));
    bQuality.setTooltip("Filter's steepness (Quality)");

    addAndMakeVisible(bGain);
    bAttachments.add(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.getPluginState(), audioProcessor.getGainParamName(index), bGain));
    bGain.setTooltip("Filter's gain");

    bSolo.setClickingTogglesState(true);
    bSolo.addListener(this);
    bSolo.setColour(juce::TextButton::buttonColourId, juce::Colours::yellow);
    addAndMakeVisible(bSolo);
    bSolo.setTooltip("Solo");

    bActivate.setClickingTogglesState(true);
    bActivate.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    bButtonAttachments.add(new juce::AudioProcessorValueTreeState::ButtonAttachment(audioProcessor.getPluginState(), audioProcessor.getActiveParamName(index), bActivate));
    addAndMakeVisible(bActivate);
    bActivate.setTooltip("Bypass");
}

BandEditor::~BandEditor()
{

}

void BandEditor::resized()
{
    auto bounds = getLocalBounds();
    bFrame.setBounds(bounds);

    bounds.reduce(10, 20);

    bFilterType.setBounds(bounds.removeFromTop(20));

    auto freqBounds = bounds.removeFromBottom(bounds.getHeight() * 2 / 3);
    bFrequency.setBounds(freqBounds.withTop(freqBounds.getY() + 20));

    auto buttons = freqBounds.reduced(5).withHeight(20);
    bSolo.setBounds(buttons.removeFromLeft(20));
    bActivate.setBounds(buttons.removeFromRight(20));

    bQuality.setBounds(bounds.removeFromLeft(bounds.getWidth() / 2));
    bGain.setBounds(bounds);
}

void BandEditor::updateControls(Gainrev2AudioProcessor::FilterType type)
{
    switch (type)
    {
    case Gainrev2AudioProcessor::NoFilter:
    case Gainrev2AudioProcessor::HighPass:
        bFrequency.setEnabled(true);
        bQuality.setEnabled(true);
        bGain.setEnabled(false);
        break;
    case Gainrev2AudioProcessor::HighPass1st:
        bFrequency.setEnabled(true);
        bQuality.setEnabled(false);
        bGain.setEnabled(false);
        break;
    case Gainrev2AudioProcessor::LowShelf:
        bFrequency.setEnabled(true);
        bQuality.setEnabled(true);
        bGain.setEnabled(true);
        break;
    case Gainrev2AudioProcessor::BandPass:
        bFrequency.setEnabled(true);
        bQuality.setEnabled(true);
        bGain.setEnabled(false);
        break;
    case Gainrev2AudioProcessor::AllPass:
        bFrequency.setEnabled(true);
        bQuality.setEnabled(false);
        bGain.setEnabled(false);
        break;
    case Gainrev2AudioProcessor::AllPass1st:
        bFrequency.setEnabled(true);
        bQuality.setEnabled(false);
        bGain.setEnabled(false);
        break;
    case Gainrev2AudioProcessor::Notch:
        bFrequency.setEnabled(true);
        bQuality.setEnabled(true);
        bGain.setEnabled(false);
        break;
    case Gainrev2AudioProcessor::Peak:
        bFrequency.setEnabled(true);
        bQuality.setEnabled(true);
        bGain.setEnabled(true);
        break;
    case Gainrev2AudioProcessor::HighShelf:
        bFrequency.setEnabled(true);
        bQuality.setEnabled(true);
        bGain.setEnabled(true);
        break;
    case Gainrev2AudioProcessor::LowPass1st:
        bFrequency.setEnabled(true);
        bQuality.setEnabled(false);
        bGain.setEnabled(false);
        break;
    case Gainrev2AudioProcessor::LowPass:
        bFrequency.setEnabled(true);
        bQuality.setEnabled(true);
        bGain.setEnabled(false);
        break;
    case Gainrev2AudioProcessor::LastFilterID:
    default:
        bFrequency.setEnabled(true);
        bQuality.setEnabled(true);
        bGain.setEnabled(true);
        break;
    }
}

void BandEditor::updateSoloState(bool isSolo)
{
    bSolo.setToggleState(isSolo, juce::dontSendNotification);
}

void BandEditor::setFrequency(float freq)
{
    bFrequency.setValue(freq, juce::sendNotification);
}

void BandEditor::setGain(float gainToUse)
{
    bGain.setValue(gainToUse, juce::sendNotification);
}

void BandEditor::setType(int type)
{
    bFilterType.setSelectedId(type + 1, juce::sendNotification);
}

void BandEditor::buttonClicked(juce::Button* b)
{
    if (b == &bSolo)
    {
        audioProcessor.setBandSolo(bSolo.getToggleState() ? int(index) : -1);
    }
}