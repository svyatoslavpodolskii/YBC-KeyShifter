cmake_minimum_required(VERSION 3.22)
project(ResampleCalc VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 9.0.3
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(ResampleCalc
    COMPANY_NAME "YoungBonesClub"
    BUNDLE_ID "kz.phosphorea.resamplecalc"
    PLUGIN_MANUFACTURER_CODE Phos
    PLUGIN_CODE RsCl
    FORMATS VST3 Standalone
    PRODUCT_NAME "YBC KeyShifter"
    PLUGIN_NAME "YBC:KeyShifter"
    DESCRIPTION "Sample pitch/BPM/key calculator for varispeed resampling"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

juce_generate_juce_header(ResampleCalc)

target_sources(ResampleCalc PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/SampleAnalyzer.cpp
    Source/SampleAnalyzer.h
)

target_compile_definitions(ResampleCalc PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_compile_options(ResampleCalc PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
)

target_link_libraries(ResampleCalc PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_formats
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
