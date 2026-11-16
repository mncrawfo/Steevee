#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class OscillatorRow : public juce::Component
{
public:
    OscillatorRow (MIDISynthAudioProcessor&, OscillatorSettings&, int index, bool canRemove);
    void resized() override;
    void refresh();

private:
    MIDISynthAudioProcessor& processor;
    OscillatorSettings& settings;
    juce::Label title;
    juce::Label volumeLabel;
    juce::Slider volume;
    juce::ComboBox waveform;
    juce::TextButton removeButton { "Remove" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveformAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OscillatorRow)
};

class MIDISynthAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     private juce::Timer
{
public:
    explicit MIDISynthAudioProcessorEditor (MIDISynthAudioProcessor&);
    ~MIDISynthAudioProcessorEditor() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    bool keyPressed (const juce::KeyPress& key) override;
    bool keyStateChanged (bool isKeyDown) override;
    void focusOfChildComponentChanged (FocusChangeType cause) override;
    void mouseDown (const juce::MouseEvent&) override;

private:
    void timerCallback() override;
    void rebuildRows();
    void layoutRows();

    MIDISynthAudioProcessor& processor;
    juce::Viewport viewport;
    juce::Component oscillatorContent;
    juce::OwnedArray<OscillatorRow> rows;
    juce::TextButton addButton { "+" };
    juce::Label warning;
    juce::MidiKeyboardComponent keyboard;
    OscillatorSettings* displayedOscillators = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MIDISynthAudioProcessorEditor)
};
