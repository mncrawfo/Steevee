#include "PluginEditor.h"

MIDISynthAudioProcessorEditor::MIDISynthAudioProcessorEditor (MIDISynthAudioProcessor& processor)
    : AudioProcessorEditor (&processor),
      keyboard (processor.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    waveformLabel.setText ("Waveform", juce::dontSendNotification);
    waveformLabel.attachToComponent (&waveformSelector, true);
    addAndMakeVisible (waveformSelector);
    waveformSelector.addItemList ({ "Sine", "Square", "Sawtooth" }, 1);
    waveformAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        processor.parameters, "waveform", waveformSelector);
    addAndMakeVisible (keyboard);
    keyboard.setAvailableRange (48, 84);

    setSize (560, 220);
}

MIDISynthAudioProcessorEditor::~MIDISynthAudioProcessorEditor() = default;

void MIDISynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff202530));
    g.setColour (juce::Colours::white);
    g.setFont (20.0f);
    g.drawText ("MIDI Synth", 20, 12, getWidth() - 40, 30, juce::Justification::centredLeft);
    g.setFont (14.0f);
    g.drawText ("Play the keys below or send MIDI from your host.", 20, 90,
                getWidth() - 40, 24, juce::Justification::centredLeft);
}

void MIDISynthAudioProcessorEditor::resized()
{
    waveformSelector.setBounds (110, 52, 180, 28);
    keyboard.setBounds (20, 120, getWidth() - 40, 80);
}
