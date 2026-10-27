#pragma once

#include <JuceHeader.h>
#include <array>
#include <atomic>

class YMYVerbAudioProcessor final : public juce::AudioProcessor
{
public:
    using APVTS = juce::AudioProcessorValueTreeState;

    YMYVerbAudioProcessor();
    ~YMYVerbAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "YMYVerb"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 8.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    APVTS parameters;
    void applyPreset (int presetIndex);
    int getPresetIndex() const noexcept;
    static juce::StringArray getPresetNames();
    float getInputLevelDb() const noexcept { return inputLevelDb.load (std::memory_order_relaxed); }
    double getTempoBpm() const noexcept { return currentTempoBpm.load (std::memory_order_relaxed); }
    float getEffectivePreDelayMs() const noexcept { return effectivePreDelayMs.load (std::memory_order_relaxed); }

private:
    static APVTS::ParameterLayout createParameterLayout();
    float getParameter (const char* parameterID) const noexcept;
    float getEffectivePreDelaySamples();
    void updateReverbParameters();

    juce::Reverb reverb;
    juce::AudioBuffer<float> preDelayBuffer;
    juce::AudioBuffer<float> dryBuffer;
    int preDelayWriteIndex = 0;
    int preDelayCapacity = 1;
    double activeSampleRate = 44100.0;

    std::array<float, 2> hpPreviousInput { 0.0f, 0.0f };
    std::array<float, 2> hpPreviousOutput { 0.0f, 0.0f };
    std::array<float, 2> lpState { 0.0f, 0.0f };
    float detectorEnvelope = 0.0f;
    float detectorAttack = 0.0f;
    float detectorRelease = 0.0f;

    std::atomic<float> inputLevelDb { -100.0f };
    std::atomic<double> currentTempoBpm { 120.0 };
    std::atomic<float> effectivePreDelayMs { 35.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (YMYVerbAudioProcessor)
};
