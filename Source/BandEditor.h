/*
  ==============================================================================

  This is an automatically generated GUI class created by the Projucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Projucer version: 6.0.7

  ------------------------------------------------------------------------------

  The Projucer is part of the JUCE library.
  Copyright (c) 2020 - Raw Material Software Limited.

  ==============================================================================
*/

//#pragma once

//[Headers]     -- You can add your own extra header files here --
#include <JuceHeader.h>
#include "PluginProcessor.h"

//[/Headers]


//==============================================================================
/**
																	//[Comments]
	An auto-generated component, created by the Projucer.

	Describe your class and how it works here!
																	//[/Comments]
*/
class BandEditor : public juce::Component,
	public juce::Button::Listener
{
public:
	//==============================================================================

    BandEditor(size_t i, Gainrev2AudioProcessor& audioProcessor);

	~BandEditor() override;
	
	void resized() override;

	void updateControls(Gainrev2AudioProcessor::FilterType type);

	void updateSoloState(bool solo);

	void setFrequency(float frequency);

	void setGain(float gain);

	void setType(int type);

	void buttonClicked(juce::Button* b) override;

	juce::Path mFrequencyResponse;

	//==============================================================================
	//[UserMethods]     -- You can add your own custom methods in this section.
	//[/UserMethods]


	/*void paint(juce::Graphics& g) override
	{
		g.fillAll(juce::Colours::black);
		g.setColour(juce::Colours::grey);
		g.drawLine(20, 0.0, 20, getHeight(), 0.5f);
	}*/


private:
	//[UserVariables]   -- You can add your own custom variables in this section.
	//[/UserVariables]
	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BandEditor)
	
	size_t index;
	Gainrev2AudioProcessor& audioProcessor;

	juce::GroupComponent bFrame;
	juce::ComboBox bFilterType;

	juce::Slider bFrequency;
	juce::Slider bQuality;
	juce::Slider bGain;

	juce::TextButton bSolo;
	juce::TextButton bActivate;

	juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> mBoxAttachments;
	juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> mAttachments;
	juce::OwnedArray<juce::AudioProcessorValueTreeState::ButtonAttachment> mButtonAttachments;

	//==============================================================================

};

//[EndFile] You can add extra defines here...
//[/EndFile]

