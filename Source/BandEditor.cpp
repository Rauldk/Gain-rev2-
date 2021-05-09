/*
  ==============================================================================

    BandEditor.cpp
    Created: 9 May 2021 11:24:38pm
    Author:  Raul

  ==============================================================================
*/

#include "BandEditor.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"


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
    mBoxAttachments.add(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(audioProcessor.getPluginState(), audioProcessor.getTypeParamName(index), bFilterType));

    addAndMakeVisible(bFrequency);
    mAttachments.add(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.getPluginState(), audioProcessor.getFrequencyParamName(index), bFrequency));
    bFrequency.setTooltip(TRANS("Filter's frequency"));

}