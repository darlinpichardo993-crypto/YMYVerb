#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

namespace
{
constexpr const char* kPreset = "preset";
constexpr const char* kPreDelay = "preDelay";
constexpr const char* kSync = "sync";
constexpr const char* kDivision = "division";
constexpr const char* kDecay = "decay";
constexpr const char* kDamping = "damping";
constexpr const char* kWidth = "width";
constexpr const char* kMix = "mix";
constexpr const char* kLowCut = "lowCut";
constexpr const char* kHighCut = "highCut";
constexpr const char* kWarmth = "warmth";
constexpr const char* kDucking = "ducking";
constexpr const char* kWetOnly = "wetOnly";

struct PresetValues
{
    float preDelayMs;
    bool sync;
    int division;
    float decay;
    float damping;
    float width;
    float mix;
    float lowCut;
    float highCut;
    float warmth;
    float ducking;
    bool wetOnly;
};

// These are original starting points inspired by broad bachata eras and aesthetics,
// not measurements or claims about the artists' private studio settings.
const std::array<PresetValues, 7> kPresets {{
    { 35.0f, true, 2, 2.00f, 52.0f, 78.0f, 24.0f, 140.0f, 8200.0f, 30.0f, 18.0f, false }, // Custom reset
    { 38.0f, true, 3, 2.35f, 56.0f, 84.0f, 25.0f, 145.0f, 8800.0f, 34.0f, 22.0f, false }, // Romeo-style silky
    { 23.0f, true, 1, 1.45f, 64.0f, 67.0f, 20.0f, 125.0f, 6100.0f, 48.0f, 13.0f, false }, // Raulín-style warm
    { 29.0f, true, 1, 1.90f, 58.0f, 76.0f, 22.0f, 135.0f, 7400.0f, 40.0f, 17.0f, false }, // Joe Veras-style
    { 34.0f, true, 2, 1.75f, 61.0f, 75.0f, 21.0f, 135.0f, 7000.0f, 43.0f, 15.0f, false }, // 2000s
    { 18.0f, true, 0, 1.20f, 69.0f, 61.0f, 18.0f, 115.0f, 5700.0f, 52.0f, 10.0f, false }, // 90s
    { 72.0f, true, 4, 4.40f, 42.0f, 100.0f, 32.0f, 170.0f, 10000.0f, 24.0f, 28.0f, false } // long modern
}};

constexpr std::array<float, 8> kDivisionBeats {{
    1.0f / 32.0f,  // 1/128 note
    1.0f / 24.0f,  // 1/64 triplet
    1.0f / 16.0f,  // 1/64 note
    1.0f / 12.0f,  // 1/32 triplet
    1.0f / 8.0f,   // 1/32 note
    1.0f / 6.0f,   // 1/16 triplet
    1.0f / 4.0f,   // 1/16 note
    1.0f / 2.0f    // 1/8 note
}};
}

YMYVerbAudioProcessor::YMYVerbAudioProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "YMYVERB_STATE", createParameterLayout())
{
}

YMYVerbAudioProcessor::APVTS::ParameterLayout YMYVerbAudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { kPreset, 1 }, "Preset", getPresetNames(), 0));
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { kPreDelay, 1 }, "Pre-Delay",
        juce::NormalisableRange<float> (0.0f, 250.0f, 0.1f, 0.55f), 35.0f, "ms"));
    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { kSync, 1 }, "Tempo Sync", true));
    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { kDivision, 1 }, "Sync Division",
        juce::StringArray { "1/128", "1/64T", "1/64", "1/32T", "1/32", "1/16T", "1/16", "1/8" }, 2));
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { kDecay, 1 }, "Decay",
        juce::NormalisableRange<float> (0.5f, 5.5f, 0.01f, 0.65f), 2.0f, "s"));
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { kDamping, 1 }, "Damping", juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 52.0f, "%"));
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { kWidth, 1 }, "Stereo Width", juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 78.0f, "%"));
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { kMix, 1 }, "Wet Mix", juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 24.0f, "%"));
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { kLowCut, 1 }, "Low Cut", juce::NormalisableRange<float> (60.0f, 500.0f, 1.0f, 0.55f), 140.0f, "Hz"));
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { kHighCut, 1 }, "High Cut", juce::NormalisableRange<float> (2500.0f, 18000.0f, 1.0f, 0.55f), 8200.0f, "Hz"));
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { kWarmth, 1 }, "Warmth", juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 30.0f, "%"));
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { kDucking, 1 }, "Ducking", juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 18.0f, "%"));
    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { kWetOnly, 1 }, "Aux / Wet Only", false));

    return layout;
}

juce::StringArray YMYVerbAudioProcessor::getPresetNames()
{
    return { "CUSTOM / RESET", "ROMEO SANTOS - SILKY", "RAULIN RODRIGUEZ - WARM",
             "JOE VERAS - SWEET", "BACHATA REVERB 2000s", "BACHATA REVERB 90s",
             "BACHATA MODERNA - LONG" };
}

float YMYVerbAudioProcessor::getParameter (const char* parameterID) const noexcept
{
    if (const auto* value = parameters.getRawParameterValue (parameterID))
        return value->load (std::memory_order_relaxed);
    return 0.0f;
}

int YMYVerbAudioProcessor::getPresetIndex() const noexcept
{
    return juce::jlimit (0, static_cast<int> (kPresets.size()) - 1,
                         juce::roundToInt (getParameter (kPreset)));
}

void YMYVerbAudioProcessor::applyPreset (int presetIndex)
{
    const int index = juce::jlimit (0, static_cast<int> (kPresets.size()) - 1, presetIndex);
    const auto& preset = kPresets[static_cast<size_t> (index)];

    const auto setParam = [this] (const char* id, float value)
    {
        if (auto* parameter = parameters.getParameter (id))
            parameter->setValueNotifyingHost (parameter->convertTo0to1 (value));
    };

    setParam (kPreDelay, preset.preDelayMs);
    setParam (kSync, preset.sync ? 1.0f : 0.0f);
    setParam (kDivision, static_cast<float> (preset.division));
    setParam (kDecay, preset.decay);
    setParam (kDamping, preset.damping);
    setParam (kWidth, preset.width);
    setParam (kMix, preset.mix);
    setParam (kLowCut, preset.lowCut);
    setParam (kHighCut, preset.highCut);
    setParam (kWarmth, preset.warmth);
    setParam (kDucking, preset.ducking);
    setParam (kWetOnly, preset.wetOnly ? 1.0f : 0.0f);
    setParam (kPreset, static_cast<float> (index));
}

void YMYVerbAudioProcessor::prepareToPlay (double sampleRate, int maximumExpectedSamplesPerBlock)
{
    activeSampleRate = juce::jmax (8000.0, sampleRate);
    const int safeBlockSize = juce::jmax (1, maximumExpectedSamplesPerBlock);
    preDelayCapacity = juce::jmax (safeBlockSize + 2, static_cast<int> (std::ceil (activeSampleRate * 1.25)) + safeBlockSize + 4);
    preDelayBuffer.setSize (2, preDelayCapacity, false, true, true);
    preDelayBuffer.clear();
    dryBuffer.setSize (2, safeBlockSize, false, true, true);
    preDelayWriteIndex = 0;
    hpPreviousInput = { 0.0f, 0.0f };
    hpPreviousOutput = { 0.0f, 0.0f };
    lpState = { 0.0f, 0.0f };
    detectorEnvelope = 0.0f;
    detectorAttack = std::exp (-1.0f / static_cast<float> (activeSampleRate * 0.006));
    detectorRelease = std::exp (-1.0f / static_cast<float> (activeSampleRate * 0.100));
    reverb.reset();
    updateReverbParameters();
}

void YMYVerbAudioProcessor::releaseResources()
{
    reverb.reset();
}

bool YMYVerbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto input = layouts.getMainInputChannelSet();
    const auto output = layouts.getMainOutputChannelSet();
    if (input != output)
        return false;
    return input == juce::AudioChannelSet::mono() || input == juce::AudioChannelSet::stereo();
}

float YMYVerbAudioProcessor::getEffectivePreDelaySamples()
{
    double bpm = 120.0;
    if (auto* playHead = getPlayHead())
    {
        if (auto position = playHead->getPosition())
        {
            if (auto hostBpm = position->getBpm())
                bpm = juce::jlimit (30.0, 300.0, *hostBpm);
        }
    }
    currentTempoBpm.store (bpm, std::memory_order_relaxed);

    float delayMs = getParameter (kPreDelay);
    if (getParameter (kSync) >= 0.5f)
    {
        const int division = juce::jlimit (0, static_cast<int> (kDivisionBeats.size()) - 1,
                                           juce::roundToInt (getParameter (kDivision)));
        delayMs = static_cast<float> ((60.0 / bpm) * 1000.0 * kDivisionBeats[static_cast<size_t> (division)]);
    }
    delayMs = juce::jlimit (0.0f, 1200.0f, delayMs);
    effectivePreDelayMs.store (delayMs, std::memory_order_relaxed);
    return static_cast<float> (delayMs * 0.001 * activeSampleRate);
}

void YMYVerbAudioProcessor::updateReverbParameters()
{
    juce::Reverb::Parameters settings;
    const float decay = getParameter (kDecay);
    // Map requested decay to JUCE's room-size curve. This is a musical control,
    // not a claim that the internal algorithm has an exact measured RT60.
    settings.roomSize = juce::jmap (decay, 0.5f, 5.5f, 0.20f, 0.92f);
    settings.damping = juce::jlimit (0.0f, 1.0f, getParameter (kDamping) * 0.01f);
    settings.wetLevel = 1.0f;
    settings.dryLevel = 0.0f;
    settings.width = juce::jlimit (0.0f, 1.0f, getParameter (kWidth) * 0.01f);
    settings.freezeMode = 0.0f;
    reverb.setParameters (settings);
}

void YMYVerbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused (midiMessages);

    const int numSamples = buffer.getNumSamples();
    const int numChannels = juce::jmin (2, getTotalNumOutputChannels());
    if (numSamples <= 0 || numChannels <= 0)
        return;

    if (dryBuffer.getNumSamples() < numSamples)
        dryBuffer.setSize (2, numSamples, false, false, true);

    for (int ch = 0; ch < numChannels; ++ch)
        dryBuffer.copyFrom (ch, 0, buffer, ch, 0, numSamples);

    const float delaySamplesFloat = getEffectivePreDelaySamples();
    const int delaySamples = juce::jlimit (0, preDelayCapacity - 2,
                                           juce::roundToInt (delaySamplesFloat));

    const float lowCut = juce::jlimit (60.0f, 500.0f, getParameter (kLowCut));
    const float highCut = juce::jlimit (2500.0f, 18000.0f, getParameter (kHighCut));
    const float hpA = std::exp (-juce::MathConstants<float>::twoPi * lowCut / static_cast<float> (activeSampleRate));
    const float lpA = 1.0f - std::exp (-juce::MathConstants<float>::twoPi * highCut / static_cast<float> (activeSampleRate));
    const float warmth = juce::jlimit (0.0f, 1.0f, getParameter (kWarmth) * 0.01f);
    const float drive = 1.0f + warmth;
    const float inverseDrive = 1.0f / juce::jmax (1.0f, drive);

    double squareSum = 0.0;
    for (int i = 0; i < numSamples; ++i)
    {
        const int readPosition = (preDelayWriteIndex - delaySamples + preDelayCapacity) % preDelayCapacity;
        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float x = dryBuffer.getSample (ch, i);
            squareSum += static_cast<double> (x) * static_cast<double> (x);

            // One-pole input high-pass followed by low-pass: keeps the tail warm,
            // but prevents low-frequency build-up and brittle top-end splash.
            const float highPassed = hpA * (hpPreviousOutput[static_cast<size_t> (ch)] + x - hpPreviousInput[static_cast<size_t> (ch)]);
            hpPreviousInput[static_cast<size_t> (ch)] = x;
            hpPreviousOutput[static_cast<size_t> (ch)] = highPassed;
            lpState[static_cast<size_t> (ch)] += lpA * (highPassed - lpState[static_cast<size_t> (ch)]);

            const float filtered = lpState[static_cast<size_t> (ch)];
            const float warmed = std::tanh (filtered * drive) * inverseDrive;
            const float delayed = (delaySamples == 0)
                ? warmed
                : preDelayBuffer.getSample (ch, readPosition);
            buffer.setSample (ch, i, delayed);
            preDelayBuffer.setSample (ch, preDelayWriteIndex, warmed);
        }
        preDelayWriteIndex = (preDelayWriteIndex + 1) % preDelayCapacity;
    }

    updateReverbParameters();
    if (numChannels == 1)
        reverb.processMono (buffer.getWritePointer (0), numSamples);
    else
        reverb.processStereo (buffer.getWritePointer (0), buffer.getWritePointer (1), numSamples);

    const float inputRms = static_cast<float> (std::sqrt (squareSum / static_cast<double> (numSamples * numChannels)));
    inputLevelDb.store (inputRms > 1.0e-6f ? 20.0f * std::log10 (inputRms) : -120.0f,
                        std::memory_order_relaxed);

    const float mix = juce::jlimit (0.0f, 1.0f, getParameter (kMix) * 0.01f);
    const bool wetOnly = getParameter (kWetOnly) >= 0.5f;
    const float dryGain = wetOnly ? 0.0f : std::cos (mix * juce::MathConstants<float>::halfPi);
    const float wetGain = wetOnly ? 1.0f : std::sin (mix * juce::MathConstants<float>::halfPi);
    const float duckAmount = juce::jlimit (0.0f, 1.0f, getParameter (kDucking) * 0.01f) * 0.65f;

    for (int i = 0; i < numSamples; ++i)
    {
        float detector = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
            detector = juce::jmax (detector, std::abs (dryBuffer.getSample (ch, i)));

        const float coefficient = detector > detectorEnvelope ? detectorAttack : detectorRelease;
        detectorEnvelope = detector + coefficient * (detectorEnvelope - detector);
        const float activity = juce::jlimit (0.0f, 1.0f, (detectorEnvelope - 0.008f) * 24.0f);
        const float duckGain = 1.0f - duckAmount * activity;

        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float dry = dryBuffer.getSample (ch, i);
            const float wet = buffer.getSample (ch, i) * duckGain;
            buffer.setSample (ch, i, dry * dryGain + wet * wetGain);
        }
    }
}

juce::AudioProcessorEditor* YMYVerbAudioProcessor::createEditor()
{
    return new YMYVerbAudioProcessorEditor (*this);
}

void YMYVerbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = parameters.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void YMYVerbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new YMYVerbAudioProcessor();
}
