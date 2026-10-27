#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <array>

class YMYVerbAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                          private juce::Timer
{
public:
    explicit YMYVerbAudioProcessorEditor (YMYVerbAudioProcessor&);
    ~YMYVerbAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    class HardwareLookAndFeel final : public juce::LookAndFeel_V4
    {
    public:
        void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                               float sliderPosProportional, float rotaryStartAngle,
                               float rotaryEndAngle, juce::Slider&) override;
    };

    class MicVisual final : public juce::Component, private juce::Timer
    {
    public:
        explicit MicVisual (YMYVerbAudioProcessor& processorToUse);
        void paint (juce::Graphics&) override;
    private:
        void timerCallback() override;
        YMYVerbAudioProcessor& processor;
        float glow = 0.0f;
    };

    void timerCallback() override;
    void updateTempoReadout();
    static juce::Colour ink();
    static juce::Colour paper();
    static juce::Colour gold();
    static juce::Colour mutedInk();

    YMYVerbAudioProcessor& processor;
    HardwareLookAndFeel hardwareLookAndFeel;
    std::array<juce::Slider, 9> sliders;
    std::array<juce::Label, 9> sliderLabels;
    std::array<std::unique_ptr<YMYVerbAudioProcessor::APVTS::SliderAttachment>, 9> sliderAttachments;
    const std::array<const char*, 9> sliderIDs {{ "preDelay", "decay", "damping", "width", "mix", "lowCut", "highCut", "warmth", "ducking" }};
    const std::array<const char*, 9> sliderNames {{ "PRE-DELAY", "DECAY", "DAMPING", "STEREO WIDTH", "WET MIX", "LOW CUT", "HIGH CUT", "WARMTH", "DUCKING" }};

    juce::ComboBox presetBox;
    juce::ComboBox divisionBox;
    juce::ToggleButton syncButton { "TEMPO SYNC" };
    juce::ToggleButton wetOnlyButton { "AUX / WET ONLY" };
    std::unique_ptr<YMYVerbAudioProcessor::APVTS::ButtonAttachment> syncAttachment;
    std::unique_ptr<YMYVerbAudioProcessor::APVTS::ButtonAttachment> wetOnlyAttachment;
    std::unique_ptr<YMYVerbAudioProcessor::APVTS::ComboBoxAttachment> divisionAttachment;

    juce::Label presetLabel;
    juce::Label rangeLabel;
    juce::Label micStatusLabel;
    juce::Label tempoLabel;
    juce::Label detailLabel;
    MicVisual micVisual;
    int lastShownPreset = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (YMYVerbAudioProcessorEditor)
};
