#include "SynthVoice.h"

SynthVoice::SynthVoice(juce::AudioProcessorValueTreeState& state)
    : apvts(state)
{
    oscillator.initialise([](float phase) { return std::sin(phase); });
}

bool SynthVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<SynthSound*>(sound) != nullptr;
}

void SynthVoice::prepare(double sampleRate, int /*samplesPerBlock*/)
{
    isPrepared = true;

    oscillator.prepare({ sampleRate, 512, 1 });
    adsr.setSampleRate(sampleRate);
    filter.prepare({ sampleRate, 512, 1 });
    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);

    smoothedCutoff.reset(sampleRate, 0.02);   // 20 ms сглаживание
    smoothedResonance.reset(sampleRate, 0.02);

    smoothedCutoff.setCurrentAndTargetValue(1000.0f);
    smoothedResonance.setCurrentAndTargetValue(0.707f);

    // Кэширование указателей на параметры
    waveformParam  = apvts.getRawParameterValue("waveform");
    attackParam    = apvts.getRawParameterValue("attack");
    decayParam     = apvts.getRawParameterValue("decay");
    sustainParam   = apvts.getRawParameterValue("sustain");
    releaseParam   = apvts.getRawParameterValue("release");
    cutoffParam    = apvts.getRawParameterValue("cutoff");
    resonanceParam = apvts.getRawParameterValue("resonance");
}

void SynthVoice::startNote(int midiNoteNumber, float velocity,
                           juce::SynthesiserSound* /*sound*/,
                           int /*currentPitchWheelPosition*/)
{
    if (!isPrepared) return;

    const float frequency = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    oscillator.setFrequency(frequency, true);
    velocityGain = velocity;

    adsr.setParameters({
        attackParam->load(),
        decayParam->load(),
        sustainParam->load(),
        releaseParam->load()
    });
    adsr.noteOn();

    filter.reset();
    smoothedCutoff.setCurrentAndTargetValue(cutoffParam->load());
    smoothedResonance.setCurrentAndTargetValue(resonanceParam->load());
}

void SynthVoice::stopNote(float /*velocity*/, bool allowTailOff)
{
    if (!isPrepared) return;

    if (allowTailOff)
        adsr.noteOff();
    else
        adsr.reset();
}

void SynthVoice::updateParams()
{
    if (!isPrepared) return;

    // Форма волны (вызывается только при смене, но здесь обновляем безопасно)
    const int waveformIndex = static_cast<int>(waveformParam->load());
    configureOscillator(waveformIndex);

    // ADSR — обновляем только если параметры изменились
    adsr.setParameters({
        attackParam->load(),
        decayParam->load(),
        sustainParam->load(),
        releaseParam->load()
    });

    // Сглаженные цели для фильтра
    smoothedCutoff.setTargetValue(cutoffParam->load());
    smoothedResonance.setTargetValue(resonanceParam->load());
}

void SynthVoice::configureOscillator(int waveformIndex)
{
    switch (waveformIndex)
    {
        case 0: // Sine
            oscillator.initialise([](float phase) { return std::sin(phase); });
            break;
        case 1: // Saw
            oscillator.initialise([](float phase) { return phase / juce::MathConstants<float>::pi; });
            break;
        case 2: // Square
            oscillator.initialise([](float phase) {
                return phase < 0.0f ? -1.0f : 1.0f;
            });
            break;
        case 3: // Triangle
            oscillator.initialise([](float phase) {
                return 2.0f * std::abs(phase / juce::MathConstants<float>::pi) - 1.0f;
            });
            break;
        default:
            break;
    }
}

void SynthVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                                 int startSample,
                                 int numSamples)
{
    if (!isPrepared || !adsr.isActive())
        return;

    // Обновляем параметры один раз на блок (без аллокаций)
    updateParams();

    auto* leftChannel  = outputBuffer.getWritePointer(0, startSample);
    auto* rightChannel = outputBuffer.getNumChannels() > 1
                         ? outputBuffer.getWritePointer(1, startSample)
                         : nullptr;

    for (int i = 0; i < numSamples; ++i)
    {
        // Сглаживание фильтра по сэмплам
        const float currentCutoff    = smoothedCutoff.getNextValue();
        const float currentResonance = smoothedResonance.getNextValue();

        filter.setCutoffFrequency(currentCutoff);
        filter.setResonance(currentResonance);

        // Генерация осциллятора
        float sample = oscillator.processSample(0.0f);

        // Огибающая амплитуды
        const float envValue = adsr.getNextSample();
        sample *= envValue * velocityGain;

        // Фильтрация (левый канал)
        sample = filter.processSample(0, sample);

        // Запись в буфер
        leftChannel[i] += sample;
        if (rightChannel != nullptr)
            rightChannel[i] += sample;
    }

    // Если огибающая завершилась — освобождаем голос
    if (!adsr.isActive())
        clearCurrentNote();
}