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


    //mGainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    /*mGainSlider.setRange(-60.0f, 0.0f, 0.01f);
    mGainSlider.setValue(0.0f);
    
    mGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);*/
    //mGainSlider.addListener(this);

    mFrame.setText("Output");
    mFrame.setTextLabelPosition(juce::Justification::centred);
    addAndMakeVisible(mFrame);
    addAndMakeVisible(mGainSlider);

    mAttachments.add(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.getPluginState(), Gainrev2AudioProcessor::paramOutput, mGainSlider));
    mGainSlider.setTooltip("Gain");

    auto size = audioProcessor.getSavedSize();
    setResizable(true, true);
    setSize(size.x, size.y);
    setResizeLimits(1000, 680, 2560, 1440);
    
    updateFreqRespone();

    //setSize (1280, 720);

#ifdef JUCE_OPENGL
    openGLContext.attachTo(*getTopLevelComponent());
#endif // JUCE_OPENGL

    audioProcessor.addChangeListener(this);

    juce::Timer::startTimerHz(30);
    
}

Gainrev2AudioProcessorEditor::~Gainrev2AudioProcessorEditor()
{
    juce::PopupMenu::dismissAllActiveMenus();

    audioProcessor.removeChangeListener(this);
    
#ifdef JUCE_OPENGL
    openGLContext.detach();
#endif // JUCE_OPENGL

    
}

//==============================================================================
//void Gainrev2AudioProcessorEditor::paint (juce::Graphics& g)
//{
//    // (Our component is opaque, so we must completely fill the background with a solid colour)
//    g.fillAll(juce::Colours::black);
//    
//    
//    
//   // juce::Rectangle<float> house(300, 120, 200, 170);
//   // g.fillCheckerBoard(house, 30, 10, juce::Colours::sandybrown, juce::Colours::saddlebrown);
//    audioProcessor.createAnalyserPlot(mAnalyserPath, mPlotFrame, 20.0f, true);
//    g.setColour(juce::Colours::white);
//    g.strokePath(mAnalyserPath, juce::PathStrokeType(1.0));
//    audioProcessor.createAnalyserPlot(mAnalyserPath, mPlotFrame, 20.0f, false);
//    g.setColour(juce::Colours::grey);
//    g.strokePath(mAnalyserPath, juce::PathStrokeType(1.0));
//}

void Gainrev2AudioProcessorEditor::paint(juce::Graphics& g)
{
    const auto inputColour = juce::Colours::greenyellow;
    const auto outputColour = juce::Colours::indianred;

    juce::Graphics::ScopedSaveState state(g);

    //g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.fillAll(juce::Colours::black);

    g.setFont(12.0f);
    g.setColour(juce::Colours::silver);
    g.drawRoundedRectangle(mPlotFrame.toFloat(), 5, 2);
    for (int i = 0; i < 10; ++i)
    {
        g.setColour(juce::Colours::silver.withAlpha(0.3f));
        auto x = mPlotFrame.getX() + mPlotFrame.getWidth() * i * 0.1f;
        if(i>0)
            g.drawVerticalLine(juce::roundToInt(x), (float)mPlotFrame.getY(), (float)mPlotFrame.getBottom());

            g.setColour(juce::Colours::silver);
            auto freq = getFreqPos(i * 0.1f);
            g.drawFittedText((freq < 1000) ? juce::String(freq) + " Hz" : juce::String(freq / 1000, 1) + " kHz",
                juce::roundToInt(x + 3), mPlotFrame.getBottom() - 18, 50, 15, juce::Justification::left, 1);
    }

    g.setColour(juce::Colours::silver.withAlpha(0.3f));
    g.drawHorizontalLine(juce::roundToInt(mPlotFrame.getY() + 0.25 * mPlotFrame.getHeight()), (float)mPlotFrame.getX(), (float)mPlotFrame.getRight());
    g.drawHorizontalLine(juce::roundToInt(mPlotFrame.getY() + 0.75 * mPlotFrame.getHeight()), (float)mPlotFrame.getX(), (float)mPlotFrame.getRight());

    g.setColour(juce::Colours::silver);
    g.drawFittedText(juce::String(maxDB) + " dB", mPlotFrame.getX() + 3, mPlotFrame.getY() + 2, 50, 14, juce::Justification::left, 1);
    g.drawFittedText(juce::String(maxDB / 2) + " dB", mPlotFrame.getX() + 3, juce::roundToInt(mPlotFrame.getY() + 2 + 0.25 * mPlotFrame.getHeight()),
        50, 14, juce::Justification::left, 1);
    g.drawFittedText(" 0 dB", mPlotFrame.getX() + 3, juce::roundToInt(mPlotFrame.getY() + 2 + 0.5 * mPlotFrame.getHeight()), 
        50, 14, juce::Justification::left, 1);
    g.drawFittedText(juce::String(-maxDB / 2) + " dB", mPlotFrame.getX() + 3, juce::roundToInt(mPlotFrame.getY() + 2 + 0.75 * mPlotFrame.getHeight()),
        50, 14, juce::Justification::left, 1);

    g.reduceClipRegion(mPlotFrame);

    g.setFont(16.0f);

    audioProcessor.createAnalyserPlot(mAnalyserPath, mPlotFrame, 20.0f, true);
    g.setColour(inputColour);
    g.drawFittedText("Input", mPlotFrame.reduced(8), juce::Justification::topRight, 1);
    g.strokePath(mAnalyserPath, juce::PathStrokeType(1.0));

    audioProcessor.createAnalyserPlot(mAnalyserPath, mPlotFrame, 20.0f, false);
    g.setColour(outputColour);
    g.drawFittedText("Output", mPlotFrame.reduced(8, 28), juce::Justification::topRight, 1);
    g.strokePath(mAnalyserPath, juce::PathStrokeType(1.0));

    for (size_t i = 0; i < audioProcessor.getNumBands(); ++i)
    {
        auto* bandEditor = mBandEditor.getUnchecked(int(i));
        auto* band = audioProcessor.getBand(i);

        g.setColour(band->active ? band->colour : band->colour.withAlpha(0.3f));
        g.strokePath(bandEditor->bFrequencyResponse, juce::PathStrokeType(1.0f));
        g.setColour(mDraggingBand == (int)i ? band->colour : band->colour.withAlpha(0.6f));

        auto x = juce::roundToInt(mPlotFrame.getX() + mPlotFrame.getWidth() * getPosForFreq((float)band->frequency));
        auto y = juce::roundToInt(getPosForGain((float)band->gain, (float)mPlotFrame.getY(), (float)mPlotFrame.getBottom()));

        g.drawVerticalLine(x, (float)mPlotFrame.getY(), (float)y - 5);
        g.drawVerticalLine(x, (float)y + 5, (float)mPlotFrame.getBottom());
        g.fillEllipse((float)x - 3, (float)y - 3, 6.0f, 6.0f);
    }

    g.setColour(juce::Colours::silver);
    g.strokePath(mFrequencyResponse, juce::PathStrokeType(1.0f));
}

void Gainrev2AudioProcessorEditor::resized()
{
    audioProcessor.setSavedSize({ getWidth(), getHeight() });
    mPlotFrame = getLocalBounds();

    auto bandSpace = mPlotFrame.removeFromBottom(getHeight() / 3);
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
    if (!event.mods.isPopupMenu() || !mPlotFrame.contains(event.x, event.y))
        return;

    for (int i = 0; i < mBandEditor.size(); i++)
    {
        if (auto* band = audioProcessor.getBand((size_t) i))
        {
            if (std::abs(mPlotFrame.getX() + getPosForFreq(std::floor(band->frequency) * mPlotFrame.getWidth()) - event.position.getX()) < clickRadius)
            {
                mContextMenu.clear();
                const auto& names = Gainrev2AudioProcessor::getFilterTypeNames();
                for (int t = 0; t < names.size(); t++)
                    mContextMenu.addItem(t + 1, names[t], true, band->type == t);

                mContextMenu.showMenuAsync(juce::PopupMenu::Options().
                    withTargetComponent(this).withTargetScreenArea({ event.getScreenX(), event.getScreenY(), 1, 1 }),
                    [this, i](int selected)
                    {
                        if (selected > 0)
                            mBandEditor.getUnchecked(i)->setType(selected - 1);
                    });
                return;

            }
        }
    }
}

void Gainrev2AudioProcessorEditor::mouseMove(const juce::MouseEvent& event)
{
    if (mPlotFrame.contains(event.x, event.y))
    {
        for (int i = 0; i < mBandEditor.size(); ++i)
        {
            if (auto* band = audioProcessor.getBand((size_t) i))
            {
                auto pos = mPlotFrame.getX() + getPosForFreq((float) band->frequency) * mPlotFrame.getWidth();

                if (std::abs(pos - event.position.getX()) < clickRadius)
                {
                    if (std::abs(getPosForGain((float)band->gain, (float)mPlotFrame.getY(), (float)mPlotFrame.getBottom()) - event.position.getY()) < clickRadius)
                    {
                        mDraggingGain = audioProcessor.getPluginState().getParameter(audioProcessor.getGainParamName((size_t)i));
                        setMouseCursor(juce::MouseCursor(juce::MouseCursor::UpDownLeftRightResizeCursor));
                    }
                    else
                    {
                        setMouseCursor(juce::MouseCursor(juce::MouseCursor::LeftRightResizeCursor));
                    }

                    if (i != mDraggingBand)
                    {
                        mDraggingBand = i;
                        repaint(mPlotFrame);
                    }
                    return;
                }
            }
        }
    }

    mDraggingBand = -1;
    mDraggingGain = false;
    setMouseCursor(juce::MouseCursor(juce::MouseCursor::NormalCursor));
    repaint(mPlotFrame);
}

void Gainrev2AudioProcessorEditor::mouseDrag(const juce::MouseEvent& event)
{
    if (juce::isPositiveAndBelow(mDraggingBand, mBandEditor.size()))
    {
        auto pos = (event.position.getX() - mPlotFrame.getX()) / mPlotFrame.getWidth();

        mBandEditor[mDraggingBand]->setFrequency(getFreqPos(pos));

        if (mDraggingGain)
            mBandEditor[mDraggingBand]->setGain(getGainForPos(event.position.getY(), (float)mPlotFrame.getY(), (float)mPlotFrame.getBottom()));
    }
}

void Gainrev2AudioProcessorEditor::mouseDoubleClick(const juce::MouseEvent& event)
{
    if (mPlotFrame.contains(event.x, event.y))
    {
        for (size_t i = 0; i < (size_t)mBandEditor.size(); ++i)
        {
            if (auto* band = audioProcessor.getBand(i))
            {
                if (std::abs(mPlotFrame.getX() + getPosForFreq((float)band->frequency) * mPlotFrame.getWidth() - event.position.getX()) < clickRadius)
                {
                    if (auto* param = audioProcessor.getPluginState().getParameter(audioProcessor.getActiveParamName(i)))
                        param->setValueNotifyingHost(param->getValue() < 0.5f ? 1.0f : 0.0f);
                }
            }
        }
    }
} 

void Gainrev2AudioProcessorEditor::updateFreqRespone()
{
    auto pixelsPerDouble = 2.0f * mPlotFrame.getHeight() / juce::Decibels::decibelsToGain(maxDB);

    for (int i = 0; i < mBandEditor.size(); ++i)
    {
        auto* bandEditor = mBandEditor.getUnchecked(i);

        if (auto* band = audioProcessor.getBand((size_t)i))
        {
            bandEditor->updateControls(band->type);
            bandEditor->bFrequencyResponse.clear();
            audioProcessor.createFrequencyPlot(bandEditor->bFrequencyResponse, band->magnitudes, mPlotFrame.withX(mPlotFrame.getX() + 1), pixelsPerDouble);
        }
        bandEditor->updateSoloState(audioProcessor.getBandSolo(i));
    }
    mFrequencyResponse.clear();
    audioProcessor.createFrequencyPlot(mFrequencyResponse, audioProcessor.getMagnitudes(), mPlotFrame, pixelsPerDouble);
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


//void Gainrev2AudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
//{
//    if (slider == &mGainSlider)
//    {
//        audioProcessor.mGain = mGainSlider.getValue();
//    }
//}