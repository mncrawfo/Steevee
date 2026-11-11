#pragma once

#include <JuceHeader.h>

class SynthSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote (int) override    { return true; }
    bool appliesToChannel (int) override { return true; }
};

class SynthVoice : public juce::SynthesiserVoice
{
public:
    explicit SynthVoice (std::atomic<float>& waveformParameter)
        : waveform (waveformParameter) {}

    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<SynthSound*> (sound) != nullptr;
    }

    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int) override
    {
        phase = 0.0;
        phaseDelta = juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber) / getSampleRate();
        level = velocity * 0.1f;
        releaseDelta = 0.0f;
    }

    void stopNote (float, bool allowTailOff) override
    {
        if (allowTailOff && level > 0.0f)
        {
            if (releaseDelta == 0.0f)
                releaseDelta = level / static_cast<float> (getSampleRate() * 0.05);
        }
        else
        {
            clearCurrentNote();
            phaseDelta = 0.0;
            level = 0.0f;
            releaseDelta = 0.0f;
        }
    }

    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}

    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample,
                          int numSamples) override
    {
        if (phaseDelta == 0.0)
            return;

        const auto selectedWaveform = static_cast<int> (waveform.load());

        for (int sample = 0; sample < numSamples; ++sample)
        {
            float value = 0.0f;

            switch (selectedWaveform)
            {
                case 1: value = phase < 0.5 ? 1.0f : -1.0f; break;
                case 2: value = static_cast<float> (2.0 * phase - 1.0); break;
                default: value = static_cast<float> (std::sin (phase * juce::MathConstants<double>::twoPi)); break;
            }

            for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
                outputBuffer.addSample (channel, startSample + sample, value * level);

            phase += phaseDelta;
            phase -= std::floor (phase);

            if (releaseDelta > 0.0f)
            {
                level -= releaseDelta;
                if (level <= 0.0f)
                {
                    clearCurrentNote();
                    phaseDelta = 0.0;
                    level = 0.0f;
                    releaseDelta = 0.0f;
                    break;
                }
            }
        }
    }

private:
    std::atomic<float>& waveform;
    double phase = 0.0;
    double phaseDelta = 0.0;
    float level = 0.0f;
    float releaseDelta = 0.0f;
};

class MIDISynthAudioProcessor : public juce::AudioProcessor
{
public:
    MIDISynthAudioProcessor();
    ~MIDISynthAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState parameters;
    juce::MidiKeyboardState keyboardState;

private:
    juce::Synthesiser synthesiser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MIDISynthAudioProcessor)
};
