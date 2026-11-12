#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "SynthVoice.h"
#include "SynthSound.h"

class PolySynthProcessor : public juce::AudioProcessor
{
public:
    PolySynthProcessor();
    ~PolySynthProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "PolySynth"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    juce::Synthesiser synth;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Master gain (сглаженный)
    std::atomic<float>* masterGainParam = nullptr;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedMasterGain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PolySynthProcessor)
};