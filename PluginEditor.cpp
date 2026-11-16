#include "PluginEditor.h"

OscillatorRow::OscillatorRow (MIDISynthAudioProcessor& owner, OscillatorSettings& layer, int index,
                              bool canRemove)
    : processor (owner), settings (layer)
{
    title.setText ("Oscillator " + juce::String (index + 1), juce::dontSendNotification);
    addAndMakeVisible (title);

    volumeLabel.setText ("Volume", juce::dontSendNotification);
    volumeLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (volumeLabel);

    volume.setName ("Oscillator " + juce::String (index + 1) + " volume");
    volume.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    volume.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 58, 18);
    volume.setTextBoxIsEditable (false);
    volume.setWantsKeyboardFocus (false);
    volume.setRange (0.0, 1.0, 0.01);
    volume.textFromValueFunction = [] (double value) { return juce::String (juce::roundToInt (value * 100.0)) + "%"; };
    volume.valueFromTextFunction = [] (const juce::String& text) { return text.getDoubleValue() / 100.0; };
    volume.setValue (settings.volume.load(), juce::dontSendNotification);
    volume.onValueChange = [this]
    {
        settings.volume.store (static_cast<float> (volume.getValue()), std::memory_order_relaxed);
        processor.updateHostDisplay (juce::AudioProcessorListener::ChangeDetails().withNonParameterStateChanged (true));
    };
    addAndMakeVisible (volume);

    waveform.setName ("Oscillator " + juce::String (index + 1) + " waveform");
    waveform.setWantsKeyboardFocus (false);
    waveform.addItemList ({ "Sine", "Square", "Sawtooth" }, 1);
    addAndMakeVisible (waveform);
    if (index == 0)
        waveformAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
            processor.parameters, "waveform", waveform);
    else
    {
        waveform.setSelectedId (static_cast<int> (settings.waveform->load()) + 1, juce::dontSendNotification);
        waveform.onChange = [this]
        {
            settings.waveform->store (static_cast<float> (waveform.getSelectedItemIndex()), std::memory_order_relaxed);
            processor.updateHostDisplay (juce::AudioProcessorListener::ChangeDetails().withNonParameterStateChanged (true));
        };
    }

    removeButton.setName ("Remove oscillator " + juce::String (index + 1));
    removeButton.setEnabled (canRemove);
    removeButton.setWantsKeyboardFocus (false);
    removeButton.onClick = [this] { processor.removeOscillator (&settings); };
    addAndMakeVisible (removeButton);
}

void OscillatorRow::refresh()
{
    volume.setValue (settings.volume.load (std::memory_order_relaxed), juce::dontSendNotification);
    if (waveformAttachment == nullptr)
        waveform.setSelectedId (static_cast<int> (settings.waveform->load (std::memory_order_relaxed)) + 1,
                                juce::dontSendNotification);
}

void OscillatorRow::resized()
{
    title.setBounds (10, 4, getWidth() - 100, 20);
    removeButton.setBounds (getWidth() - 90, 4, 80, 26);
    volumeLabel.setBounds (10, 25, 70, 18);
    volume.setBounds (10, 42, 70, 65);
    waveform.setBounds (100, 54, juce::jmax (0, getWidth() - 120), 28);
}

MIDISynthAudioProcessorEditor::MIDISynthAudioProcessorEditor (MIDISynthAudioProcessor& owner)
    : AudioProcessorEditor (&owner), processor (owner),
      keyboard (owner.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setWantsKeyboardFocus (true);
    addMouseListener (this, true);
    viewport.setViewedComponent (&oscillatorContent, false);
    viewport.setScrollBarsShown (true, false);
    addAndMakeVisible (viewport);
    addButton.setTooltip ("Add an oscillator");
    addButton.setWantsKeyboardFocus (false);
    addButton.onClick = [this]
    {
        processor.addOscillator();
        rebuildRows();
        viewport.setViewPosition (0, juce::jmax (0, oscillatorContent.getHeight() - viewport.getHeight()));
    };
    oscillatorContent.addAndMakeVisible (addButton);

    warning.setText ("More than 4 oscillators may increase CPU use and output level.",
                     juce::dontSendNotification);
    warning.setColour (juce::Label::textColourId, juce::Colours::orange);
    addAndMakeVisible (warning);
    addAndMakeVisible (keyboard);
    keyboard.setAvailableRange (48, 84);

    setResizable (true, true);
    setResizeLimits (560, 500, 1600, 1200);
    setSize (900, 720);
    rebuildRows();
    startTimerHz (15);
}

MIDISynthAudioProcessorEditor::~MIDISynthAudioProcessorEditor()
{
    stopTimer();
    removeMouseListener (this);
    keyboard.focusLost (focusChangedDirectly);
    viewport.setViewedComponent (nullptr, false);
}

bool MIDISynthAudioProcessorEditor::keyPressed (const juce::KeyPress& key)
{
    return keyboard.keyPressed (key);
}

bool MIDISynthAudioProcessorEditor::keyStateChanged (bool isKeyDown)
{
    return keyboard.keyStateChanged (isKeyDown);
}

void MIDISynthAudioProcessorEditor::focusOfChildComponentChanged (FocusChangeType cause)
{
    if (! hasKeyboardFocus (true))
        keyboard.focusLost (cause);
}

void MIDISynthAudioProcessorEditor::mouseDown (const juce::MouseEvent&)
{
    if (! hasKeyboardFocus (true))
        grabKeyboardFocus();
}

void MIDISynthAudioProcessorEditor::timerCallback()
{
    if (displayedOscillators != processor.getOscillators())
        rebuildRows();
    else
        for (auto* row : rows)
            row->refresh();
}

void MIDISynthAudioProcessorEditor::rebuildRows()
{
    displayedOscillators = processor.getOscillators();
    std::vector<OscillatorSettings*> ordered;
    for (auto* layer = displayedOscillators; layer != nullptr; layer = layer->next)
        ordered.push_back (layer);
    std::reverse (ordered.begin(), ordered.end());

    rows.clear();
    for (size_t i = 0; i < ordered.size(); ++i)
    {
        auto* row = rows.add (new OscillatorRow (processor, *ordered[i], static_cast<int> (i),
                                                ordered.size() > 1));
        oscillatorContent.addAndMakeVisible (row);
    }
    warning.setVisible (rows.size() > 4);
    layoutRows();
}

void MIDISynthAudioProcessorEditor::layoutRows()
{
    const auto width = juce::jmax (0, viewport.getWidth() - viewport.getScrollBarThickness());
    int y = 0;
    for (auto* row : rows)
    {
        row->setBounds (0, y, width, 112);
        y += 112;
    }
    addButton.setBounds (10, y + 4, 48, 30);
    oscillatorContent.setSize (width, y + 44);
}

void MIDISynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff202530));
    g.setColour (juce::Colours::white);
    g.setFont (20.0f);
    g.drawText ("MIDI Synth", 20, 12, getWidth() - 40, 30, juce::Justification::centredLeft);
    g.setFont (14.0f);
    g.drawText ("Play with the computer keyboard, on-screen keys, or MIDI from your host.",
                20, getHeight() - 125, getWidth() - 40, 24, juce::Justification::centredLeft);
}

void MIDISynthAudioProcessorEditor::resized()
{
    viewport.setBounds (20, 48, getWidth() - 40, getHeight() - 215);
    warning.setBounds (20, getHeight() - 160, getWidth() - 40, 28);
    keyboard.setBounds (20, getHeight() - 95, getWidth() - 40, 75);
    layoutRows();
}
