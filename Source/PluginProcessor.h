/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

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
class Gainrev2AudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    Gainrev2AudioProcessor();
    ~Gainrev2AudioProcessor() override;


    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    float mGain{ 0.5 };
    Visualiser visualiser;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Gainrev2AudioProcessor)
};
