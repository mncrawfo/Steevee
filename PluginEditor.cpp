/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TestPluginAudioProcessorEditor::TestPluginAudioProcessorEditor (TestPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    frequencyLabel.setText ("Frequency", juce::dontSendNotification);
    frequencyLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (frequencyLabel);

    frequencySlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    frequencySlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 100, 24);
    frequencySlider.setTextValueSuffix (" Hz");
    addAndMakeVisible (frequencySlider);
    frequencyAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.getParameters(), "frequency", frequencySlider);

    waveformLabel.setText ("Waveform", juce::dontSendNotification);
    waveformLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (waveformLabel);

    waveformSelector.addItem ("Sine", 1);
    waveformSelector.addItem ("Square", 2);
    waveformSelector.addItem ("Sawtooth", 3);
    addAndMakeVisible (waveformSelector);
    waveformAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        audioProcessor.getParameters(), "waveform", waveformSelector);

    setSize (360, 240);
    setResizeLimits (360, 240, 360, 240);
}

TestPluginAudioProcessorEditor::~TestPluginAudioProcessorEditor()
{
}

//==============================================================================
void TestPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (22.0f));
    g.drawFittedText ("Simple Oscillator", getLocalBounds().removeFromTop (52),
                      juce::Justification::centred, 1);
}

void TestPluginAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (24);
    area.removeFromTop (44);

    auto controlRow = area.removeFromTop (36);
    waveformLabel.setBounds (controlRow.removeFromLeft (100));
    waveformSelector.setBounds (controlRow.removeFromLeft (180));

    area.removeFromTop (12);
    frequencyLabel.setBounds (area.removeFromTop (24));
    frequencySlider.setBounds (area);
}
