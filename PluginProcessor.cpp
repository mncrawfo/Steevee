/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MIDISynthAudioProcessor::MIDISynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#else
     :
#endif
      parameters (*this, nullptr, "Parameters",
                  { std::make_unique<juce::AudioParameterChoice> (
                        juce::ParameterID { "waveform", 1 }, "Waveform",
                        juce::StringArray { "Sine", "Square", "Sawtooth" }, 0) })
{
    oscillators.store (createOscillator (nullptr, true, 0.0f, 1.0f), std::memory_order_release);
    for (int i = 0; i < 8; ++i)
        synthesiser.addVoice (new SynthVoice (oscillators));

    synthesiser.addSound (new SynthSound());
}

OscillatorSettings* MIDISynthAudioProcessor::createOscillator (OscillatorSettings* previous,
                                                                bool first, float waveform,
                                                                float volume)
{
    auto oscillator = std::make_unique<OscillatorSettings> (
        previous, first ? parameters.getRawParameterValue ("waveform") : nullptr, waveform, volume);
    auto* result = oscillator.get();
    oscillatorStorage.push_back (std::move (oscillator));
    return result;
}

void MIDISynthAudioProcessor::addOscillator()
{
    {
        const juce::ScopedLock lock (oscillatorChanges);
        auto* current = oscillators.load (std::memory_order_acquire);
        oscillators.store (createOscillator (current, false, 0.0f, 0.5f),
                           std::memory_order_release);
    }
    updateHostDisplay (juce::AudioProcessorListener::ChangeDetails().withNonParameterStateChanged (true));
}

OscillatorSettings* MIDISynthAudioProcessor::getOscillators() const noexcept
{
    return oscillators.load (std::memory_order_acquire);
}

MIDISynthAudioProcessor::~MIDISynthAudioProcessor()
{
}

//==============================================================================
const juce::String MIDISynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MIDISynthAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MIDISynthAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MIDISynthAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MIDISynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.05;
}

int MIDISynthAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MIDISynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MIDISynthAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MIDISynthAudioProcessor::getProgramName (int index)
{
    return {};
}

void MIDISynthAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MIDISynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    synthesiser.setCurrentPlaybackSampleRate(sampleRate);
}

void MIDISynthAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MIDISynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void MIDISynthAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Clear the audio buffer
    buffer.clear();

    keyboardState.processNextMidiBuffer (midiMessages, 0, buffer.getNumSamples(), true);

    // Generate audio using incoming MIDI messages
    synthesiser.renderNextBlock(
        buffer,
        midiMessages,
        0,
        buffer.getNumSamples()
    );
}

//==============================================================================
bool MIDISynthAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MIDISynthAudioProcessor::createEditor()
{
    return new MIDISynthAudioProcessorEditor (*this);
}

//==============================================================================
void MIDISynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    juce::ValueTree layers ("Oscillators");
    std::vector<OscillatorSettings*> ordered;
    for (auto* layer = getOscillators(); layer != nullptr; layer = layer->next)
        ordered.push_back (layer);
    std::reverse (ordered.begin(), ordered.end());

    for (auto* layer : ordered)
    {
        juce::ValueTree item ("Oscillator");
        item.setProperty ("waveform", layer->waveform->load (std::memory_order_relaxed), nullptr);
        item.setProperty ("volume", layer->volume.load (std::memory_order_relaxed), nullptr);
        layers.addChild (item, -1, nullptr);
    }

    state.addChild (layers, -1, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void MIDISynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto state = getXmlFromBinary (data, sizeInBytes))
        if (state->hasTagName (parameters.state.getType()))
        {
            auto tree = juce::ValueTree::fromXml (*state);
            auto layers = tree.getChildWithName ("Oscillators");
            if (layers.isValid())
                tree.removeChild (layers, nullptr);

            const juce::ScopedLock lock (oscillatorChanges);
            parameters.replaceState (tree);
            OscillatorSettings* current = nullptr;

            if (layers.isValid())
            {
                for (const auto& layer : layers)
                {
                    if (! layer.hasType ("Oscillator"))
                        continue;
                    const auto shape = juce::jlimit (0.0f, 2.0f, static_cast<float> (layer.getProperty ("waveform", 0.0f)));
                    const auto volume = juce::jlimit (0.0f, 1.0f, static_cast<float> (layer.getProperty ("volume", 1.0f)));
                    current = createOscillator (current, current == nullptr, shape, volume);
                }
            }

            if (current == nullptr)
                current = createOscillator (nullptr, true, 0.0f, 1.0f);
            oscillators.store (current, std::memory_order_release);
        }
}



//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MIDISynthAudioProcessor();
}
