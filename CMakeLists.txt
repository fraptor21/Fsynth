cmake_minimum_required(VERSION 3.22)
project(PolySynth VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Подключение JUCE (предполагается, что JUCE лежит в подпапке JUCE/)
add_subdirectory(JUCE)

juce_add_plugin(PolySynth
    COMPANY_NAME "YourCompany"
    PLUGIN_MANUFACTURER_CODE "Ycom"
    PLUGIN_CODE "Psyn"
    FORMATS VST3 Standalone
    PRODUCT_NAME "PolySynth"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE
    VST3_CATEGORIES "Instrument"
)

target_sources(PolySynth PRIVATE
    source/PluginProcessor.cpp
    source/PluginEditor.cpp
    source/SynthVoice.cpp
)

target_compile_definitions(PolySynth PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(PolySynth PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
