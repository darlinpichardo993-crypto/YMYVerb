cmake_minimum_required(VERSION 3.22)
project(YMYVerb VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT DEFINED JUCE_PATH OR JUCE_PATH STREQUAL "")
    set(JUCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/.deps/JUCE" CACHE PATH "Path to JUCE source")
endif()
if (NOT EXISTS "${JUCE_PATH}/CMakeLists.txt")
    message(FATAL_ERROR "JUCE was not found at '${JUCE_PATH}'. Run BUILD-WINDOWS.bat or pass -DJUCE_PATH=...")
endif()

add_subdirectory("${JUCE_PATH}" JUCE)

juce_add_plugin(YMYVerb
    COMPANY_NAME "YMY Records"
    BUNDLE_ID "com.ymyrecords.ymyverb"
    PLUGIN_MANUFACTURER_CODE YMYR
    PLUGIN_CODE YMYV
    PRODUCT_NAME "YMYVerb"
    DESCRIPTION "Warm tempo-aware bachata vocal reverb by ORYMY"
    FORMATS VST3
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Fx Reverb
)

target_sources(YMYVerb PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)

target_compile_definitions(YMYVerb PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(YMYVerb PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

if (MSVC)
    target_compile_options(YMYVerb PRIVATE /utf-8)
endif()
