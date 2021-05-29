/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/
#pragma warning(disable: 4100)
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Analyser.h"
#include "BandEditor.h"

juce::String Gainrev2AudioProcessor::paramOutput("output");
juce::String Gainrev2AudioProcessor::paramType("type");
juce::String Gainrev2AudioProcessor::paramFrequency("frequency");
juce::String Gainrev2AudioProcessor::paramQuality("quality");
juce::String Gainrev2AudioProcessor::paramGain("gain");
juce::String Gainrev2AudioProcessor::paramActive("active");

namespace IDs {
	juce::String editor{ "editor" };
	juce::String sizeX{ "size-x" };
	juce::String sizeY{ "size-y" };
}

juce::String Gainrev2AudioProcessor::getBandID(size_t index)
{
	switch (index)
	{
	case 0: return "Lowest";
	case 1: return "Low";
	case 2: return "Low Mids";
	case 3: return "High Mids";
	case 4: return "High";
	case 5: return "Highest";
	default: break;
	}
	return "unknown";
}

juce::StringArray Gainrev2AudioProcessor::getFilterTypeNames()
{
	return {
		"No Filter",
		"High Pass",
		"1st High Pass",
		"Low Shelf",
		"Band Pass",
		"All Pass",
		"1st All Pass",
		"Notch",
		"Peak",
		"High Shelf",
		"1st Low Pass",
		"Low Pass"
	};
}

std::vector<Gainrev2AudioProcessor::Band> createDefaultBands() 
{
	std::vector<Gainrev2AudioProcessor::Band> defaults;

	defaults.push_back(Gainrev2AudioProcessor::Band("Lowest", juce::Colours::yellow, Gainrev2AudioProcessor::LowShelf, 30.0f));
	defaults.push_back(Gainrev2AudioProcessor::Band("Low", juce::Colours::brown, Gainrev2AudioProcessor::Peak, 250.0f));
	defaults.push_back(Gainrev2AudioProcessor::Band("Low Mids", juce::Colours::green, Gainrev2AudioProcessor::Peak, 500.0f));
	defaults.push_back(Gainrev2AudioProcessor::Band("High Mids", juce::Colours::coral, Gainrev2AudioProcessor::Peak, 1000.0f));
	defaults.push_back(Gainrev2AudioProcessor::Band("High", juce::Colours::orange, Gainrev2AudioProcessor::Peak, 5000.0f));
	defaults.push_back(Gainrev2AudioProcessor::Band("Highest", juce::Colours::red, Gainrev2AudioProcessor::HighShelf, 12000.0f));

	return defaults;
}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
	std::vector<std::unique_ptr<juce::AudioProcessorParameterGroup>> params;

	const float maxGain = juce::Decibels::decibelsToGain(24.0f);
	auto defaults = createDefaultBands();

	{
		auto param = std::make_unique<juce::AudioParameterFloat>(Gainrev2AudioProcessor::paramOutput,
			"Output",
			juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f),
			1.0f,
			"Output Level",
			juce::AudioProcessorParameter::genericParameter,
			[](float value, int) {return juce::String(juce::Decibels::gainToDecibels(value), 1) + "dB"; }, //lambda function
			[](juce::String text) {return juce::Decibels::decibelsToGain(text.dropLastCharacters(3).getFloatValue()); } //lambda function
		);

		auto group = std::make_unique<juce::AudioProcessorParameterGroup>("global", "Globals", "|", std::move(param));
		params.push_back(std::move(group));
	}

	for (size_t i = 0; i < defaults.size(); ++i)
	{
		auto prefix = "Q" + juce::String(i + 1) + ": ";

		auto typeParameter = std::make_unique<juce::AudioParameterChoice>(Gainrev2AudioProcessor::getTypeParamName(i),
			prefix + "Filter Type",
			Gainrev2AudioProcessor::getFilterTypeNames(),
			defaults[i].type);

		auto freqParameter = std::make_unique<juce::AudioParameterFloat>(Gainrev2AudioProcessor::getFrequencyParamName(i),
			prefix + "Frequency",
			juce::NormalisableRange<float> {20.0f, 20000.0f, 1.0f, std::log(0.5f) / std::log(980.0f / 19980.0f)},
			defaults[i].frequency,
			juce::String(),
			juce::AudioProcessorParameter::genericParameter,
			[](float value, int) { return (value < 1000.0f) ?
			juce::String(value, 0) + "Hz" :
			juce::String(value / 1000.0f, 2) + " kHz"; },
			[](juce::String text) { return text.endsWith(" kHz") ?
			text.dropLastCharacters(4).getFloatValue() * 1000.0f :
			text.dropLastCharacters(3).getFloatValue(); }
		);

		auto qltyParameter = std::make_unique<juce::AudioParameterFloat>(Gainrev2AudioProcessor::getQualityParamName(i),
			prefix + "Quality",
			juce::NormalisableRange<float> {0.1f, 10.0f, 0.1f, std::log(0.1f) / std::log(0.9f / 9.9f)},
			defaults[i].quality,
			juce::String(),
			juce::AudioProcessorParameter::genericParameter,
			[](float value, int) {return juce::String(value, 1); },
			[](const juce::String& text) {return text.getFloatValue(); }
		);

		auto gainParameter = std::make_unique<juce::AudioParameterFloat>(Gainrev2AudioProcessor::getGainParamName(i),
			prefix + "Gain",
			juce::NormalisableRange<float> {1.0f / maxGain, maxGain, 0.001f, std::log(0.5f) / std::log((1.0f - (1.0f / maxGain)) / (maxGain - (1.0f / maxGain)))},
			defaults[i].gain,
			juce::String(),
			juce::AudioProcessorParameter::genericParameter,
			[](float value, int) { return juce::String(juce::Decibels::gainToDecibels(value), 1) + " dB"; },
			[](juce::String text) { return juce::Decibels::decibelsToGain(text.dropLastCharacters(3).getFloatValue()); }
		);

		auto actvParameter = std::make_unique<juce::AudioParameterBool>(Gainrev2AudioProcessor::getActiveParamName(i),
			prefix + "Active",
			defaults[i].active,
			juce::String(),
			[](float value, int) {return value > 0.5f ? "active" : "bypassed"; },
			[](juce::String text) {return text == "active"; }
		);

		auto group = std::make_unique<juce::AudioProcessorParameterGroup>("band" + juce::String(i), defaults[i].name, "|",
			std::move(typeParameter),
			std::move(freqParameter),
			std::move(qltyParameter),
			std::move(gainParameter),
			std::move(actvParameter)
			);

		params.push_back(std::move(group));
	}

	return { params.begin(), params.end() };
}

//==============================================================================
Gainrev2AudioProcessor::Gainrev2AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
	: AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
		.withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
		.withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
	),
#endif
	mState(*this, &mUndo, "PARAMS", createParameterLayout())
{
	mFrequencies.resize(300);
	for (size_t i = 0; i < mFrequencies.size(); ++i) {
		mFrequencies[i] = 20.0 * std::pow(2.0, i / 30.0f);
	}
	mMagnitudes.resize(mFrequencies.size());

	mBands = createDefaultBands();

	for (size_t i = 0; i < mBands.size(); ++i)
	{
		mBands[i].magnitudes.resize(mFrequencies.size(), 1.0); // check here if error, different magnitudes

		mState.addParameterListener(getTypeParamName(i), this); 
		mState.addParameterListener(getFrequencyParamName(i), this);
		mState.addParameterListener(getQualityParamName(i), this);
		mState.addParameterListener(getGainParamName(i), this);
		mState.addParameterListener(getActiveParamName(i), this);

	}

	mState.addParameterListener(paramOutput, this);

	mState.state = juce::ValueTree(JucePlugin_Name);
}

Gainrev2AudioProcessor::~Gainrev2AudioProcessor()
{
	mAnalyserInput.stopThread(1000);
	mAnalyserOutput.stopThread(1000);
}

//==============================================================================
const juce::String Gainrev2AudioProcessor::getName() const
{
	return JucePlugin_Name;
}

bool Gainrev2AudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
	return true;
#else
	return false;
#endif
}

bool Gainrev2AudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
	return true;
#else
	return false;
#endif
}

bool Gainrev2AudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
	return true;
#else
	return false;
#endif
}

double Gainrev2AudioProcessor::getTailLengthSeconds() const
{
	return 0.0;
}

int Gainrev2AudioProcessor::getNumPrograms()
{
	return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
				// so this should be at least 1, even if you're not really implementing programs.
}

int Gainrev2AudioProcessor::getCurrentProgram()
{
	return 0;
}

void Gainrev2AudioProcessor::setCurrentProgram(int index)
{
}

const juce::String Gainrev2AudioProcessor::getProgramName(int index)
{
	return {};
}

void Gainrev2AudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

void Gainrev2AudioProcessor::updateBand(const size_t index)
{
	if (mSampleRate > 0)
	{
		juce::dsp::IIR::Coefficients<float>::Ptr newCoefficients;
		switch (mBands[index].type)
		{
		case NoFilter:
			newCoefficients = new juce::dsp::IIR::Coefficients<float>(1, 0, 1, 0);
			break;
		case LowPass:
			newCoefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(mSampleRate, mBands[index].frequency, mBands[index].quality);
			break;
		case LowPass1st:
			newCoefficients = juce::dsp::IIR::Coefficients<float>::makeFirstOrderLowPass(mSampleRate, mBands[index].frequency);
			break;
		case LowShelf:
			newCoefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(mSampleRate, mBands[index].frequency, mBands[index].quality, mBands[index].gain);
			break;
		case BandPass:
			newCoefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass(mSampleRate, mBands[index].frequency, mBands[index].quality);
			break;
		case AllPass:
			newCoefficients = juce::dsp::IIR::Coefficients<float>::makeAllPass(mSampleRate, mBands[index].frequency, mBands[index].quality);
			break;
		case AllPass1st:
			newCoefficients = juce::dsp::IIR::Coefficients<float>::makeFirstOrderAllPass(mSampleRate, mBands[index].frequency);
			break;
		case Notch:
			newCoefficients = juce::dsp::IIR::Coefficients<float>::makeNotch(mSampleRate, mBands[index].frequency, mBands[index].quality);
			break;
		case Peak:
			newCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(mSampleRate, mBands[index].frequency, mBands[index].quality, mBands[index].gain);
			break;
		case HighShelf:
			newCoefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(mSampleRate, mBands[index].frequency, mBands[index].quality, mBands[index].gain);
			break;
		case HighPass1st:
			newCoefficients = juce::dsp::IIR::Coefficients<float>::makeFirstOrderHighPass(mSampleRate, mBands[index].frequency);
			break;
		case HighPass:
			newCoefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(mSampleRate, mBands[index].frequency, mBands[index].quality);
			break;
		case LastFilterID:
		default:
			break;
		}

		if (newCoefficients)
		{
			{
				juce::ScopedLock processLock(getCallbackLock());
				if (index == 0)
					*mFilter.get<0>().state = *newCoefficients;
				else if (index == 1)
					*mFilter.get<1>().state = *newCoefficients;
				else if (index == 2)
					*mFilter.get<2>().state = *newCoefficients;
				else if (index == 3)
					*mFilter.get<3>().state = *newCoefficients;
				else if (index == 4)
					*mFilter.get<4>().state = *newCoefficients;
				else if (index == 5)
					*mFilter.get<5>().state = *newCoefficients;
			}
			newCoefficients->getMagnitudeForFrequencyArray(mFrequencies.data(), mBands[index].magnitudes.data(), mFrequencies.size(), mSampleRate);
		}
		updateBypassedStates();
		updatePlots();
	}
}

void Gainrev2AudioProcessor::updatePlots()
{
	auto gain = mFilter.get<6>().getGainLinear();
	std::fill(mMagnitudes.begin(), mMagnitudes.end(), gain);

	if (juce::isPositiveAndBelow(mSolo, mBands.size()))
	{
		juce::FloatVectorOperations::multiply(mMagnitudes.data(), mBands[size_t(mSolo)].magnitudes.data(), static_cast<int>(mMagnitudes.size()));
	}
	else
	{
		for (size_t i = 0; i < mBands.size(); ++i)
			if (mBands[i].active)
				juce::FloatVectorOperations::multiply(mMagnitudes.data(), mBands[i].magnitudes.data(), static_cast<int>(mMagnitudes.size()));
	}

	sendChangeMessage();
}

//==============================================================================
void Gainrev2AudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	// Use this method as the place to do any pre-playback
	// initialisation that you need..
	mSampleRate = sampleRate;

	juce::dsp::ProcessSpec spec;

	spec.sampleRate = sampleRate;
	spec.maximumBlockSize = uint32_t(samplesPerBlock);
	spec.numChannels = uint32_t(getTotalNumOutputChannels());

	for (size_t i = 0; i < mBands.size(); ++i)
	{
		updateBand(i);
	}
	mFilter.get<6>().setGainLinear(*mState.getRawParameterValue(paramOutput));

	updatePlots();

	mFilter.prepare(spec);

	//visualiser.clear();

	mAnalyserInput.setupAnalyser(int(sampleRate), float(sampleRate));
	mAnalyserOutput.setupAnalyser(int(sampleRate), float(sampleRate));

}

void Gainrev2AudioProcessor::releaseResources()
{
	mAnalyserInput.stopThread(1000);
	mAnalyserOutput.stopThread(1000);
	// When playback stops, you can use this as an opportunity to free up any
	// spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Gainrev2AudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
	juce::ignoreUnused(layouts);
	return true;
#else
	// This is the place where you check if the layout is supported.
	// In this template code we only support mono or stereo.
	// Some plugin hosts, such as certain GarageBand versions, will only
	// load plugins that support stereo bus layouts.
	if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
		&& layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()) //did daniel intentionally remove this?
		return false;

	// This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
	if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
		return false;
#endif

	return true;
#endif
}
#endif

void Gainrev2AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;
	juce::ignoreUnused(midiMessages);
	auto totalNumInputChannels = getTotalNumInputChannels();
	auto totalNumOutputChannels = getTotalNumOutputChannels();

	if (getActiveEditor() != nullptr)
	{
		mAnalyserInput.addAudioData(buffer, 0, totalNumInputChannels);
	}

	// In case we have more outputs than inputs, this code clears any output
	// channels that didn't contain input data, (because these aren't
	// guaranteed to be empty - they may contain garbage).
	// This is here to avoid people getting screaming feedback
	// when they first compile a plugin, but obviously you don't need to keep
	// this code if your algorithm always overwrites all the output channels.

	if (mWasBypassed)
	{
		mFilter.reset();
		mWasBypassed = false;
	}


	for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
		buffer.clear(i, 0, buffer.getNumSamples());

	// This is the place where you'd normally do the guts of your plugin's
	// audio processing...
	// Make sure to reset the state if your inner loop is processing
	// the samples and the outer loop is handling the channels.
	// Alternatively, you can process the samples with the channels
	// interleaved by keeping the same state.

	for (int channel = 0; channel < totalNumInputChannels; ++channel)
	{
		auto* channelData = buffer.getWritePointer(channel);
		const auto readPointer = buffer.getReadPointer(channel);

		for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
		{
			//channelData[sample] = channelData[sample] * juce::Decibels::decibelsToGain(mGain);
		}

	}
	juce::dsp::AudioBlock<float> ioBuffer(buffer);
	juce::dsp::ProcessContextReplacing<float> context(ioBuffer);

	mFilter.process(context);
	if (getActiveEditor() != nullptr)
		mAnalyserOutput.addAudioData(buffer, 0, totalNumOutputChannels);
}

void Gainrev2AudioProcessor::createFrequencyPlot(juce::Path& p, const std::vector<double>& mags, const juce::Rectangle<int> bounds, float pixelsPerDouble)
{
	p.startNewSubPath(float(bounds.getX()), mags[0] > 0 ? float(bounds.getCentreY() - pixelsPerDouble * std::log(mags[0]) / std::log(2.0)) : bounds.getBottom());

	const auto xFactor = static_cast<double>(bounds.getWidth()) / mFrequencies.size();
	for (size_t i = 1; i < mFrequencies.size(); ++i) //change to cubicTo?
	{
		p.lineTo(float(bounds.getX() + i * xFactor), float(mags[i] > 0 ? bounds.getCentreY() - pixelsPerDouble * std::log(mags[i]) / std::log(2.0) : bounds.getBottom()));
	}
}

void Gainrev2AudioProcessor::createAnalyserPlot(juce::Path& p, const juce::Rectangle<int> bounds, float minFreq, bool input)
{
	if (input)
		mAnalyserInput.createPath(p, bounds.toFloat(), minFreq);
	else
		mAnalyserOutput.createPath(p, bounds.toFloat(), minFreq);
}

bool Gainrev2AudioProcessor::checkForNewAnalyserData()
{
	return mAnalyserInput.checkDataAvailable() || mAnalyserOutput.checkDataAvailable();
}

juce::AudioProcessorValueTreeState& Gainrev2AudioProcessor::getPluginState()
{
	return mState;
}

juce::String Gainrev2AudioProcessor::getTypeParamName(size_t index)
{
	return getBandID(index) + "-" + paramType;
}

juce::String Gainrev2AudioProcessor::getFrequencyParamName(size_t index)
{
	return getBandID(index) + "-" + paramFrequency;
}

juce::String Gainrev2AudioProcessor::getQualityParamName(size_t index)
{
	return getBandID(index) + "-" + paramQuality;
}

juce::String Gainrev2AudioProcessor::getGainParamName(size_t index)
{
	return getBandID(index) + "-" + paramGain;
}

juce::String Gainrev2AudioProcessor::getActiveParamName(size_t index)
{
	return getBandID(index) + "-" + paramActive;
}

int Gainrev2AudioProcessor::getBandIndexFromID(juce::String paramID)
{
	for (size_t i = 0; i < 6; ++i)
		if (paramID.startsWith(getBandID(i) + "-"))
			return int(i);
	return -1;
}

Gainrev2AudioProcessor::Band* Gainrev2AudioProcessor::getBand(size_t index) {
	if (juce::isPositiveAndBelow(index, mBands.size()))
		return &mBands[index];
	return nullptr;
}

void Gainrev2AudioProcessor::parameterChanged(const juce::String& parameter, float newValue)
{
	if (parameter == paramOutput)
	{
		mFilter.get<6>().setGainLinear(newValue);
		updatePlots();
		return;
	}

	int index = getBandIndexFromID(parameter);
	if (juce::isPositiveAndBelow(index, mBands.size()))
	{
		auto* band = getBand(size_t(index));
		if (parameter.endsWith(paramType))
		{
			band->type = static_cast<FilterType>(static_cast<int> (newValue));
		}
		else if (parameter.endsWith(paramFrequency))
		{
			band->frequency = newValue;
		}
		else if (parameter.endsWith(paramQuality))
		{
			band->quality = newValue;
		}
		else if (parameter.endsWith(paramGain))
		{
			band->gain = newValue;
		}
		else if (parameter.endsWith(paramActive))
		{
			band->active = newValue >= 0.5f;
		}

		updateBand(size_t(index));
	}
}

size_t Gainrev2AudioProcessor::getNumBands() const
{
	return mBands.size();
}

juce::String Gainrev2AudioProcessor::getBandName(size_t index) const
{
	if (juce::isPositiveAndBelow(index, mBands.size()))
		return mBands[size_t(index)].name;
	return "unknown";
}

juce::Colour Gainrev2AudioProcessor::getBandColour(size_t index) const
{
	if (juce::isPositiveAndBelow(index, mBands.size()))
		return mBands[size_t(index)].colour;
	return juce::Colours::silver;
}

bool Gainrev2AudioProcessor::getBandSolo(int index) const
{
	return index == mSolo;
}

void Gainrev2AudioProcessor::setBandSolo(int index)
{
	mSolo = index;
	updateBypassedStates();
}

void Gainrev2AudioProcessor::updateBypassedStates()
{
	if (juce::isPositiveAndBelow(mSolo, mBands.size()))
	{
		mFilter.setBypassed<0>(mSolo != 0);
		mFilter.setBypassed<1>(mSolo != 1);
		mFilter.setBypassed<2>(mSolo != 2);
		mFilter.setBypassed<3>(mSolo != 3);
		mFilter.setBypassed<4>(mSolo != 4);
		mFilter.setBypassed<5>(mSolo != 5);
	}
	else {
		mFilter.setBypassed<0>(!mBands[0].active);
		mFilter.setBypassed<1>(!mBands[1].active);
		mFilter.setBypassed<2>(!mBands[2].active);
		mFilter.setBypassed<3>(!mBands[3].active);
		mFilter.setBypassed<4>(!mBands[4].active);
		mFilter.setBypassed<5>(!mBands[5].active);
	}
	updatePlots();
}

const std::vector<double>& Gainrev2AudioProcessor::getMagnitudes()
{
	return mMagnitudes;
}

//==============================================================================
bool Gainrev2AudioProcessor::hasEditor() const
{
	return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Gainrev2AudioProcessor::createEditor()
{
	return new Gainrev2AudioProcessorEditor(*this);
}

//==============================================================================
void Gainrev2AudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
	auto editor = mState.state.getOrCreateChildWithName(IDs::editor, nullptr);
	editor.setProperty(IDs::sizeX, mEditorSize.x, nullptr);
	editor.setProperty(IDs::sizeY, mEditorSize.y, nullptr);
	// You should use this method to store your parameters in the memory block.
	// You could do that either as raw data, or use the XML or ValueTree classes
	// as intermediaries to make it easy to save and load complex data.
	juce::MemoryOutputStream stream(destData, false);
	mState.state.writeToStream(stream);
}

void Gainrev2AudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
	juce::ValueTree tree = juce::ValueTree::readFromData(data, size_t(sizeInBytes));
	if (tree.isValid())
	{
		mState.state = tree;

		auto editor = mState.state.getChildWithName(IDs::editor);

		if (editor.isValid())
		{
			mEditorSize.setX(editor.getProperty(IDs::sizeX, 900));
			mEditorSize.setY(editor.getProperty(IDs::sizeY, 500));

			if (auto* thisEditor = getActiveEditor())
				thisEditor->setSize(mEditorSize.x, mEditorSize.y);
		}
	}
	// You should use this method to restore your parameters from this memory block,
	// whose contents will have been created by the getStateInformation() call.
}

juce::Point<int> Gainrev2AudioProcessor::getSavedSize() const
{
	return mEditorSize;
}

void Gainrev2AudioProcessor::setSavedSize(const juce::Point<int>& size)
{
	mEditorSize = size;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new Gainrev2AudioProcessor();
}
