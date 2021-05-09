/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Analyser.h"


class Visualiser : public juce::AudioVisualiserComponent
{
public:
	Visualiser() : juce::AudioVisualiserComponent(2)
	{
		setBufferSize(512);
		setSamplesPerBlock(256);
		setColours(juce::Colours::black, juce::Colours::indianred);
	}

};

//==============================================================================
/**
*/
class Gainrev2AudioProcessor : public juce::AudioProcessor,
	public juce::AudioProcessorValueTreeState::Listener,
	public juce::ChangeBroadcaster
{
public:
	//==============================================================================
	Gainrev2AudioProcessor();
	~Gainrev2AudioProcessor() override;

	enum FilterType
	{
		NoFilter = 0,
		HighPass,
		HighPass1st,
		LowShelf,
		BandPass,
		AllPass,
		AllPass1st,
		Notch,
		Peak,
		HighShelf,
		LowPass1st,
		LowPass,
		LastFilterID
	};

	static juce::String paramOutput;
	static juce::String paramType;
	static juce::String paramFrequency;
	static juce::String paramQuality;
	static juce::String paramGain;
	static juce::String paramActive;

	static juce::String getBandID(size_t index);
	static juce::String getTypeParamName(size_t index);
	static juce::String getFrequencyParamName(size_t index);
	static juce::String getQualityParamName(size_t index);
	static juce::String getGainParamName(size_t index);
	static juce::String getActiveParamName(size_t index);


	void createFrequencyPlot(juce::Path& p, const std::vector<double>& mags, const juce::Rectangle<int> bounds, float pixelsPerDouble);
	void createAnalyserPlot(juce::Path& p, const juce::Rectangle<int> bounds, float minFreq, bool input);
	//==============================================================================
	void prepareToPlay(double sampleRate, int samplesPerBlock) override;
	void releaseResources() override;

	bool checkForNewAnalyserData();

#ifndef JucePlugin_PreferredChannelConfigurations
	bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

	void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

	void parameterChanged(const juce::String& parameter, float newValue) override;

	juce::AudioProcessorValueTreeState& getPluginState();

	size_t getNumBands() const;
	juce::String getBandName(size_t index) const;
	juce::Colour getBandColour(size_t index) const;

	void setBandSolo(int index);
	bool getBandSolo(int index) const;

	static juce::StringArray getFilterTypeNames();

	//==============================================================================
	juce::AudioProcessorEditor* createEditor() override;
	bool hasEditor() const override;

	const std::vector<double>& getMagnitudes();
	//==============================================================================
	const juce::String getName() const override;

	bool acceptsMidi() const override;
	bool producesMidi() const override;
	bool isMidiEffect() const override;
	double getTailLengthSeconds() const override;

	//==============================================================================
	int getNumPrograms() override;
	int getCurrentProgram() override;
	void setCurrentProgram(int index) override;
	const juce::String getProgramName(int index) override;
	void changeProgramName(int index, const juce::String& newName) override;

	//==============================================================================
	void getStateInformation(juce::MemoryBlock& destData) override;
	void setStateInformation(const void* data, int sizeInBytes) override;

	juce::Point<int> getSavedSize() const;
	void setSavedSize(const juce::Point<int>& size);

	float mGain{ 0.5f };
	Visualiser visualiser;

	struct Band {
		Band(const juce::String& nameToUse, juce::Colour colourToUse, enum FilterType typeToUse,
			float frequencyToUse, float qualityToUse, float gainToUse = 1.0f, bool shouldBeActive = true)
			: name(nameToUse),
			colour(colourToUse),
			type(typeToUse),
			frequency(frequencyToUse),
			quality(qualityToUse),
			gain(gainToUse),
			active(shouldBeActive)
		{}

		juce::String      name;
		juce::Colour      colour;
		FilterType  type = BandPass;
		float       frequency = 1000.0f;
		float       quality = 1.0f;
		float       gain = 1.0f;
		bool        active = true;
		std::vector<double> magnitudes;
	};

	Band* getBand(size_t index);
	int getBandIndexFromID(juce::String paramID);

private:
	
	Analyser<float> mAnalyserInput;
	Analyser<float> mAnalyserOutput;
	
	void updateBand(const size_t index);

	void updateBypassedStates();

	void updatePlots();

	juce::UndoManager mUndo;
	juce::AudioProcessorValueTreeState mState;

	std::vector<Band> mBands;

	std::vector<double> mFrequencies;
	std::vector<double> mMagnitudes;

	bool mWasBypassed = true;
	
	using filterBand = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;
	using gain = juce::dsp::Gain<float>;
	juce::dsp::ProcessorChain<filterBand, filterBand, filterBand, filterBand, filterBand, filterBand, gain> mFilter;

	double mSampleRate = 0;

	int mSolo = -1;

	juce::Point<int> mEditorSize = { 900, 500 };
	
	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Gainrev2AudioProcessor)
};
