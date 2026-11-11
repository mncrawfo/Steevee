#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class MIDISynthAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit MIDISynthAudioProcessorEditor (MIDISynthAudioProcessor&);
    ~MIDISynthAudioProcessorEditor() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    juce::Label waveformLabel;
    juce::ComboBox waveformSelector;
    juce::MidiKeyboardComponent keyboard;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveformAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MIDISynthAudioProcessorEditor)
};
