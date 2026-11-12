#include "PluginProcessor.h"
#include "PluginEditor.h"

PolySynthProcessor::PolySynthProcessor()
    : AudioProcessor(BusesProperties()
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    // Добавляем голоса
    for (int i = 0; i < 8; ++i)
        synth.addVoice(new SynthVoice(apvts));

    synth.addSound(new SynthSound());

    masterGainParam = apvts.getRawParameterValue("masterGain");
    smoothedMasterGain.reset(44100.0, 0.02);
    smoothedMasterGain.setCurrentAndTargetValue(0.7f);
}

juce::AudioProcessorValueTreeState::ParameterLayout PolySynthProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Waveform (Choice)
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "waveform", "Waveform",
        juce::StringArray{ "Sine", "Saw", "Square", "Triangle" }, 0));

    // ADSR
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "attack", "Attack",
        juce::NormalisableRange<float>(0.001f, 2.0f, 0.001f, 0.3f), 0.01f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "decay", "Decay",
        juce::NormalisableRange<float>(0.001f, 2.0f, 0.001f, 0.3f), 0.1f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "sustain", "Sustain",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.7f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "release", "Release",
        juce::NormalisableRange<float>(0.001f, 3.0f, 0.001f, 0.3f), 0.3f));

    // Filter
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "cutoff", "Cutoff",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 0.1f, 0.3f), 2000.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "resonance", "Resonance",
        juce::NormalisableRange<float>(0.1f, 10.0f, 0.01f), 0.707f));

    // Master Gain
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "masterGain", "Master Gain",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.7f));

    return layout;
}

void PolySynthProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);

    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<SynthVoice*>(synth.getVoice(i)))
            voice->prepare(sampleRate, samplesPerBlock);
    }

    smoothedMasterGain.reset(sampleRate, 0.02);
    smoothedMasterGain.setCurrentAndTargetValue(masterGainParam->load());
}

void PolySynthProcessor::releaseResources() {}

void PolySynthProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                      juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    // Рендеринг синтезатора (внутри Synthesiser нет аллокаций после prepare)
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    // Применяем Master Gain со сглаживанием
    smoothedMasterGain.setTargetValue(masterGainParam->load());
    const int numSamples = buffer.getNumSamples();

    for (int i = 0; i < numSamples; ++i)
    {
        const float gain = smoothedMasterGain.getNextValue();
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            buffer.getWritePointer(ch)[i] *= gain;
    }
}

juce::AudioProcessorEditor* PolySynthProcessor::createEditor()
{
    return new PolySynthEditor(*this);
}

void PolySynthProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void PolySynthProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PolySynthProcessor();
}