#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "SynthSound.h"

class SynthVoice : public juce::SynthesiserVoice
{
public:
    SynthVoice(juce::AudioProcessorValueTreeState& apvts);
    ~SynthVoice() override = default;

    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void startNote(int midiNoteNumber, float velocity,
                   juce::SynthesiserSound* sound,
                   int /*currentPitchWheelPosition*/) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void pitchWheelMoved(int /*newPitchWheelValue*/) override {}
    void controllerMoved(int /*controllerNumber*/, int /*newControllerValue*/) override {}
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                         int startSample,
                         int numSamples) override;

    void prepare(double sampleRate, int samplesPerBlock);
    void updateParams();

private:
    juce::AudioProcessorValueTreeState& apvts;

    // Параметры (atomic указатели)
    std::atomic<float>* waveformParam = nullptr;
    std::atomic<float>* attackParam = nullptr;
    std::atomic<float>* decayParam = nullptr;
    std::atomic<float>* sustainParam = nullptr;
    std::atomic<float>* releaseParam = nullptr;
    std::atomic<float>* cutoffParam = nullptr;
    std::atomic<float>* resonanceParam = nullptr;

    // DSP-модули
    juce::dsp::Oscillator<float> oscillator;
    juce::ADSR adsr;
    juce::dsp::StateVariableTPTFilter<float> filter;

    // Сглаженные значения
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> smoothedCutoff;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedResonance;

    // Состояние
    bool isPrepared = false;
    float velocityGain = 0.0f;

    void configureOscillator(int waveformIndex);
};