/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// Defines a sound that our synthesizer can play
class SineWaveSound : public juce::SynthesiserSound
{
public:

    bool appliesToNote(int midiNoteNumber) override
    {
        return true;
    }

    bool appliesToChannel(int midiChannel) override
    {
        return true;
    }
};


// Generates an individual sine wave note
class SineWaveVoice : public juce::SynthesiserVoice
{
public:

    bool canPlaySound(juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<SineWaveSound*>(sound) != nullptr;
    }

    void startNote(int midiNoteNumber,
                   float velocity,
                   juce::SynthesiserSound* sound,
                   int currentPitchWheelPosition) override
    {
        // Convert MIDI note number into frequency
        auto frequency = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);

        // Calculate how much the oscillator's phase changes per sample
        angleDelta = juce::MathConstants<double>::twoPi
                     * frequency / getSampleRate();

        level = velocity;
        currentAngle = 0.0;
    }

    void stopNote(float velocity, bool allowTailOff) override
    {
        clearCurrentNote();
        angleDelta = 0.0;
    }

    void pitchWheelMoved(int newPitchWheelValue) override
    {
    }

    void controllerMoved(int controllerNumber, int newControllerValue) override
    {
    }

    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                         int startSample,
                         int numSamples) override
    {
        if (angleDelta == 0.0)
            return;

        for (int sample = 0; sample < numSamples; ++sample)
        {
            auto currentSample = (float) std::sin(currentAngle) * level;

            for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
            {
                outputBuffer.addSample(channel, startSample + sample, currentSample);
            }

            currentAngle += angleDelta;
        }
    }

private:

    double currentAngle = 0.0;
    double angleDelta = 0.0;

    float level = 0.0f;
};

//==============================================================================
/**
*/
class MIDISynthAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    MIDISynthAudioProcessor();
    ~MIDISynthAudioProcessor() override;

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

private:
    juce::Synthesiser synthesiser;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MIDISynthAudioProcessor)
  
};
