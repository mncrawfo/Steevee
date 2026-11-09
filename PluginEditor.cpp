/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MIDISynthAudioProcessorEditor::MIDISynthAudioProcessorEditor (MIDISynthAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    midiVolume.addListener (this);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (200, 200);
    // these define the parameters of our slider object
    midiVolume.setSliderStyle (juce::Slider::LinearBarVertical);
    midiVolume.setRange (0.0, 127.0, 1.0);
    midiVolume.setTextBoxStyle (juce::Slider::NoTextBox, false, 90, 0);
    midiVolume.setPopupDisplayEnabled (true, false, this);
    midiVolume.setTextValueSuffix (" Volume");
    midiVolume.setValue (1.0);
    // this function adds the slider to the editor
    addAndMakeVisible (&midiVolume);
}

MIDISynthAudioProcessorEditor::~MIDISynthAudioProcessorEditor()
{
}

void MIDISynthAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
    //audioProcessor.noteOnVel = midiVolume.getValue();
}

//==============================================================================
void MIDISynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    // fill the whole window white
        g.fillAll (juce::Colours::white);
        // set the current drawing colour to black
        g.setColour (juce::Colours::black);
        // set the font size and draw text to the screen
        g.setFont (15.0f);
    g.drawFittedText ("Midi Volume", 0, 0, getWidth(), 30, juce::Justification::centred, 1);
}

void MIDISynthAudioProcessorEditor::resized()
{
    // sets the position and size of the slider with arguments (x, y, width, height)
        midiVolume.setBounds (40, 30, 20, getHeight() - 60);
}

