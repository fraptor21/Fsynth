#include "PluginEditor.h"

PolySynthEditor::PolySynthEditor(PolySynthProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(500, 400);

    // Waveform ComboBox
    addAndMakeVisible(waveformBox);
    waveformBox.addItemList({ "Sine", "Saw", "Square", "Triangle" }, 1);
    waveformAttachment = std::make_unique<ComboBoxAttachment>(
        processor.apvts, "waveform", waveformBox);
    setupLabel(waveformLabel, "Waveform");
    addAndMakeVisible(waveformLabel);

    // ADSR Sliders
    setupSlider(attackSlider, attackLabel, "Attack");
    setupSlider(decaySlider, decayLabel, "Decay");
    setupSlider(sustainSlider, sustainLabel, "Sustain");
    setupSlider(releaseSlider, releaseLabel, "Release");

    attackAttachment = std::make_unique<SliderAttachment>(
        processor.apvts, "attack", attackSlider);
    decayAttachment = std::make_unique<SliderAttachment>(
        processor.apvts, "decay", decaySlider);
    sustainAttachment = std::make_unique<SliderAttachment>(
        processor.apvts, "sustain", sustainSlider);
    releaseAttachment = std::make_unique<SliderAttachment>(
        processor.apvts, "release", releaseSlider);

    // Filter
    setupSlider(cutoffSlider, cutoffLabel, "Cutoff");
    setupSlider(resonanceSlider, resonanceLabel, "Resonance");

    cutoffAttachment = std::make_unique<SliderAttachment>(
        processor.apvts, "cutoff", cutoffSlider);
    resonanceAttachment = std::make_unique<SliderAttachment>(
        processor.apvts, "resonance", resonanceSlider);

    // Master Gain
    setupSlider(masterGainSlider, masterGainLabel, "Master");
    masterGainAttachment = std::make_unique<SliderAttachment>(
        processor.apvts, "masterGain", masterGainSlider);
}

void PolySynthEditor::setupSlider(juce::Slider& slider, juce::Label& label,
                                  const juce::String& text)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(slider);
    setupLabel(label, text);
    addAndMakeVisible(label);
}

void PolySynthEditor::setupLabel(juce::Label& label, const juce::String& text)
{
    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::Font(14.0f));
}

void PolySynthEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1e1e2e));
    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    g.drawFittedText("PolySynth", getLocalBounds().removeFromTop(30),
                     juce::Justification::centred, 1);
}

void PolySynthEditor::resized()
{
    auto area = getLocalBounds().reduced(10);
    area.removeFromTop(30); // заголовок

    // Waveform
    auto waveformArea = area.removeFromTop(60);
    waveformLabel.setBounds(waveformArea.removeFromLeft(80));
    waveformBox.setBounds(waveformArea.removeFromLeft(150).reduced(5));

    area.removeFromTop(10);

    // ADSR (4 ручки в ряд)
    auto adsrArea = area.removeFromTop(120);
    const int sliderWidth = adsrArea.getWidth() / 4;

    auto layoutSlider = [&](juce::Slider& s, juce::Label& l, juce::Rectangle<int> r) {
        l.setBounds(r.removeFromTop(20));
        s.setBounds(r.reduced(5));
    };

    layoutSlider(attackSlider, attackLabel, adsrArea.removeFromLeft(sliderWidth));
    layoutSlider(decaySlider, decayLabel, adsrArea.removeFromLeft(sliderWidth));
    layoutSlider(sustainSlider, sustainLabel, adsrArea.removeFromLeft(sliderWidth));
    layoutSlider(releaseSlider, releaseLabel, adsrArea.removeFromLeft(sliderWidth));

    area.removeFromTop(10);

    // Filter + Master (3 ручки)
    auto filterArea = area.removeFromTop(120);
    const int filterWidth = filterArea.getWidth() / 3;

    layoutSlider(cutoffSlider, cutoffLabel, filterArea.removeFromLeft(filterWidth));
    layoutSlider(resonanceSlider, resonanceLabel, filterArea.removeFromLeft(filterWidth));
    layoutSlider(masterGainSlider, masterGainLabel, filterArea.removeFromLeft(filterWidth));
}