#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class PolySynthEditor : public juce::AudioProcessorEditor
{
public:
    explicit PolySynthEditor(PolySynthProcessor&);
    ~PolySynthEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    PolySynthProcessor& processor;

    juce::ComboBox waveformBox;
    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;
    juce::Slider cutoffSlider, resonanceSlider;
    juce::Slider masterGainSlider;

    juce::Label waveformLabel, attackLabel, decayLabel, sustainLabel, releaseLabel;
    juce::Label cutoffLabel, resonanceLabel, masterGainLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<ComboBoxAttachment> waveformAttachment;
    std::unique_ptr<SliderAttachment> attackAttachment, decayAttachment, sustainAttachment, releaseAttachment;
    std::unique_ptr<SliderAttachment> cutoffAttachment, resonanceAttachment, masterGainAttachment;

    void setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& text);
    void setupLabel(juce::Label& label, const juce::String& text);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PolySynthEditor)
};