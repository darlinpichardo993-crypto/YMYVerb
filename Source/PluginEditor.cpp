#include "PluginEditor.h"

namespace
{
const juce::Colour kInk      = juce::Colour::fromRGB (34, 43, 47);
const juce::Colour kMuted    = juce::Colour::fromRGB (105, 111, 109);
const juce::Colour kPaper    = juce::Colour::fromRGB (245, 242, 233);
const juce::Colour kPanel    = juce::Colour::fromRGB (252, 250, 244);
const juce::Colour kBorder   = juce::Colour::fromRGB (201, 193, 176);
const juce::Colour kGold     = juce::Colour::fromRGB (174, 126, 65);
const juce::Colour kGoldLite = juce::Colour::fromRGB (222, 184, 119);
const juce::Colour kDark     = juce::Colour::fromRGB (42, 48, 49);
}

juce::Colour YMYVerbAudioProcessorEditor::ink()      { return kInk; }
juce::Colour YMYVerbAudioProcessorEditor::paper()    { return kPaper; }
juce::Colour YMYVerbAudioProcessorEditor::gold()     { return kGold; }
juce::Colour YMYVerbAudioProcessorEditor::mutedInk() { return kMuted; }

void YMYVerbAudioProcessorEditor::HardwareLookAndFeel::drawRotarySlider (
    juce::Graphics& g, int x, int y, int width, int height,
    float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
    juce::Slider& slider)
{
    juce::ignoreUnused (slider);
    const float side = static_cast<float> (juce::jmin (width, height - 18));
    const float radius = side * 0.365f;
    const float cx = static_cast<float> (x) + static_cast<float> (width) * 0.5f;
    const float cy = static_cast<float> (y) + radius + 7.0f;
    const float start = rotaryStartAngle;
    const float end = rotaryEndAngle;
    const float angle = start + sliderPosProportional * (end - start);

    // Fine engraved scale.
    for (int i = 0; i <= 24; ++i)
    {
        const float t = static_cast<float> (i) / 24.0f;
        const float a = start + t * (end - start);
        const float r1 = radius + 7.0f;
        const float r2 = radius + (i % 3 == 0 ? 14.0f : 10.0f);
        g.setColour (i / 24.0f <= sliderPosProportional ? kGold : kBorder);
        g.drawLine (cx + std::cos (a) * r1, cy + std::sin (a) * r1,
                    cx + std::cos (a) * r2, cy + std::sin (a) * r2,
                    i % 3 == 0 ? 1.7f : 1.0f);
    }

    const juce::Rectangle<float> dial (cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);
    g.setColour (juce::Colours::black.withAlpha (0.08f));
    g.fillEllipse (dial.translated (0.0f, 2.0f));
    g.setGradientFill (juce::ColourGradient (juce::Colour::fromRGB (255, 255, 251), cx - radius, cy - radius,
                                              juce::Colour::fromRGB (202, 204, 199), cx + radius, cy + radius, false));
    g.fillEllipse (dial);
    g.setColour (juce::Colour::fromRGB (164, 166, 159));
    g.drawEllipse (dial, 1.0f);
    g.setColour (juce::Colours::white.withAlpha (0.72f));
    g.drawEllipse (dial.reduced (3.0f), 1.0f);

    const float pointerRadius = radius * 0.75f;
    g.setColour (kDark);
    g.drawLine (cx, cy,
                cx + std::cos (angle) * pointerRadius,
                cy + std::sin (angle) * pointerRadius, 4.2f);
    g.setColour (kGoldLite);
    g.fillEllipse (cx - 3.0f, cy - 3.0f, 6.0f, 6.0f);
    g.setColour (kDark.withAlpha (0.12f));
    g.drawEllipse (cx - radius * 0.48f, cy - radius * 0.48f, radius * 0.96f, radius * 0.96f, 0.8f);
}

YMYVerbAudioProcessorEditor::MicVisual::MicVisual (YMYVerbAudioProcessor& processorToUse)
    : processor (processorToUse)
{
    startTimerHz (30);
}

void YMYVerbAudioProcessorEditor::MicVisual::timerCallback()
{
    const float level = processor.getInputLevelDb();
    const float target = juce::jlimit (0.0f, 1.0f, (level + 58.0f) / 34.0f);
    glow += (target > glow ? 0.34f : 0.10f) * (target - glow);
    repaint();
}

void YMYVerbAudioProcessorEditor::MicVisual::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const float cx = bounds.getCentreX();
    const float top = 7.0f;
    const float glowAlpha = 0.10f + glow * 0.34f;

    // Quiet halo when idle; warm illuminated ring when vocal/input is present.
    g.setColour (kGoldLite.withAlpha (glowAlpha));
    g.fillEllipse (cx - 48.0f, 10.0f, 96.0f, 120.0f);
    g.setColour (kGold.withAlpha (0.11f + glow * 0.42f));
    g.drawEllipse (cx - 41.0f, 15.0f, 82.0f, 106.0f, 1.2f + glow * 1.8f);

    // Condenser microphone head, drawn as clean vector artwork (no stock image).
    juce::Path head;
    head.addRoundedRectangle (cx - 23.0f, top + 9.0f, 46.0f, 73.0f, 17.0f);
    g.setGradientFill (juce::ColourGradient (juce::Colour::fromRGB (247, 248, 244), cx - 24.0f, top + 11.0f,
                                              juce::Colour::fromRGB (122, 130, 128), cx + 25.0f, top + 70.0f, false));
    g.fillPath (head);
    g.setColour (juce::Colour::fromRGB (85, 94, 94));
    g.strokePath (head, juce::PathStrokeType (1.3f));

    g.saveState();
    g.reduceClipRegion (juce::Rectangle<int> (static_cast<int> (cx - 20.0f), static_cast<int> (top + 13.0f), 40, 63));
    for (int i = 0; i < 12; ++i)
    {
        const float yy = top + 18.0f + static_cast<float> (i) * 4.8f;
        g.setColour ((i % 2 == 0 ? juce::Colours::white : juce::Colour::fromRGB (72, 81, 82)).withAlpha (0.55f));
        g.drawLine (cx - 19.0f, yy, cx + 19.0f, yy, 1.0f);
    }
    for (int i = 0; i < 7; ++i)
    {
        const float xx = cx - 15.0f + static_cast<float> (i) * 5.0f;
        g.setColour (juce::Colour::fromRGB (60, 68, 68).withAlpha (0.45f));
        g.drawLine (xx, top + 15.0f, xx, top + 75.0f, 0.65f);
    }
    g.restoreState();

    // Capsule collar, shaft and compact desktop stand.
    g.setColour (juce::Colour::fromRGB (72, 80, 80));
    g.fillRoundedRectangle (cx - 26.0f, top + 78.0f, 52.0f, 8.0f, 2.4f);
    g.setGradientFill (juce::ColourGradient (juce::Colour::fromRGB (226, 228, 223), cx - 11.0f, top + 85.0f,
                                              juce::Colour::fromRGB (105, 112, 110), cx + 11.0f, top + 118.0f, false));
    g.fillRoundedRectangle (cx - 8.0f, top + 84.0f, 16.0f, 34.0f, 4.0f);
    g.setColour (juce::Colour::fromRGB (91, 98, 95));
    g.drawRoundedRectangle (cx - 8.0f, top + 84.0f, 16.0f, 34.0f, 4.0f, 0.9f);
    g.setColour (kGold.withAlpha (0.85f));
    g.fillRoundedRectangle (cx - 23.0f, top + 117.0f, 46.0f, 5.0f, 2.0f);
    g.setColour (juce::Colour::fromRGB (84, 91, 89));
    g.fillRoundedRectangle (cx - 37.0f, top + 127.0f, 74.0f, 7.0f, 3.0f);
    g.setColour (juce::Colours::white.withAlpha (0.68f));
    g.drawLine (cx - 30.0f, top + 129.0f, cx + 29.0f, top + 129.0f, 1.0f);

    // The lamp itself pulses only when the incoming channel contains a signal.
    const float lamp = 4.0f + glow * 2.0f;
    g.setColour (juce::Colour::fromRGB (80, 83, 75));
    g.fillEllipse (cx + 34.0f, top + 87.0f, 10.0f, 10.0f);
    g.setColour (juce::Colour::fromRGB (222, 176, 92).withAlpha (0.35f + glow * 0.65f));
    g.fillEllipse (cx + 36.0f, top + 89.0f, lamp, lamp);
}

YMYVerbAudioProcessorEditor::YMYVerbAudioProcessorEditor (YMYVerbAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p), micVisual (p)
{
    setLookAndFeel (&hardwareLookAndFeel);
    setSize (1000, 700);
    setResizable (true, false);
    setResizeLimits (900, 700, 1200, 700);

    for (size_t i = 0; i < sliders.size(); ++i)
    {
        auto& slider = sliders[i];
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 74, 18);
        slider.setTextBoxIsEditable (true);
        slider.setLookAndFeel (&hardwareLookAndFeel);
        slider.setColour (juce::Slider::textBoxTextColourId, kInk);
        slider.setColour (juce::Slider::textBoxBackgroundColourId, kPanel);
        slider.setColour (juce::Slider::textBoxOutlineColourId, kBorder);
        slider.setColour (juce::Slider::textBoxHighlightColourId, kGold.withAlpha (0.3f));
        static const std::array<const char*, 9> suffixes {{ " ms", " s", " %", " %", " %", " Hz", " Hz", " %", " %" }};
        slider.setTextValueSuffix (suffixes[i]);
        slider.setNumDecimalPlacesToDisplay (i == 1 ? 2 : 1);
        addAndMakeVisible (slider);

        auto& label = sliderLabels[i];
        label.setText (sliderNames[i], juce::dontSendNotification);
        label.setFont (juce::Font (11.5f, juce::Font::bold));
        label.setJustificationType (juce::Justification::centred);
        label.setColour (juce::Label::textColourId, kMuted);
        addAndMakeVisible (label);

        sliderAttachments[i] = std::make_unique<YMYVerbAudioProcessor::APVTS::SliderAttachment> (
            processor.parameters, sliderIDs[i], slider);
    }

    presetLabel.setText ("BACHATA REVERB PRESET", juce::dontSendNotification);
    presetLabel.setFont (juce::Font (11.5f, juce::Font::bold));
    presetLabel.setColour (juce::Label::textColourId, kMuted);
    addAndMakeVisible (presetLabel);

    presetBox.addItemList (YMYVerbAudioProcessor::getPresetNames(), 1);
    presetBox.setSelectedId (processor.getPresetIndex() + 1, juce::dontSendNotification);
    presetBox.setColour (juce::ComboBox::backgroundColourId, kPanel);
    presetBox.setColour (juce::ComboBox::textColourId, kInk);
    presetBox.setColour (juce::ComboBox::outlineColourId, kBorder);
    presetBox.setColour (juce::ComboBox::arrowColourId, kGold);
    presetBox.onChange = [this]
    {
        const int selected = presetBox.getSelectedId() - 1;
        if (selected >= 0)
        {
            processor.applyPreset (selected);
            lastShownPreset = selected;
        }
    };
    addAndMakeVisible (presetBox);

    rangeLabel.setText ("A close, sweet tail for bachata leads and backing vocals.\nUse on an insert, or enable AUX / WET ONLY on a send return.",
                        juce::dontSendNotification);
    rangeLabel.setFont (juce::Font (12.0f));
    rangeLabel.setColour (juce::Label::textColourId, kMuted);
    rangeLabel.setJustificationType (juce::Justification::topLeft);
    rangeLabel.setMinimumHorizontalScale (0.8f);
    addAndMakeVisible (rangeLabel);

    micStatusLabel.setText ("LISTENING FOR INPUT", juce::dontSendNotification);
    micStatusLabel.setFont (juce::Font (10.5f, juce::Font::bold));
    micStatusLabel.setColour (juce::Label::textColourId, kMuted);
    micStatusLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (micStatusLabel);
    addAndMakeVisible (micVisual);

    tempoLabel.setText ("HOST TEMPO  120 BPM", juce::dontSendNotification);
    tempoLabel.setFont (juce::Font (15.0f, juce::Font::bold));
    tempoLabel.setColour (juce::Label::textColourId, kInk);
    tempoLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (tempoLabel);

    detailLabel.setText ("PRE-DELAY FOLLOWS THE DAW WHEN SYNC IS ON", juce::dontSendNotification);
    detailLabel.setFont (juce::Font (10.5f));
    detailLabel.setColour (juce::Label::textColourId, kMuted);
    detailLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (detailLabel);

    syncButton.setClickingTogglesState (true);
    syncButton.setColour (juce::ToggleButton::textColourId, kInk);
    syncButton.setColour (juce::ToggleButton::tickColourId, kGold);
    syncButton.setColour (juce::ToggleButton::tickDisabledColourId, kBorder);
    syncAttachment = std::make_unique<YMYVerbAudioProcessor::APVTS::ButtonAttachment> (
        processor.parameters, "sync", syncButton);
    addAndMakeVisible (syncButton);

    divisionBox.addItemList ({ "1/128", "1/64T", "1/64", "1/32T", "1/32", "1/16T", "1/16", "1/8" }, 1);
    divisionBox.setColour (juce::ComboBox::backgroundColourId, kPanel);
    divisionBox.setColour (juce::ComboBox::textColourId, kInk);
    divisionBox.setColour (juce::ComboBox::outlineColourId, kBorder);
    divisionBox.setColour (juce::ComboBox::arrowColourId, kGold);
    divisionAttachment = std::make_unique<YMYVerbAudioProcessor::APVTS::ComboBoxAttachment> (
        processor.parameters, "division", divisionBox);
    addAndMakeVisible (divisionBox);

    wetOnlyButton.setClickingTogglesState (true);
    wetOnlyButton.setColour (juce::ToggleButton::textColourId, kInk);
    wetOnlyButton.setColour (juce::ToggleButton::tickColourId, kGold);
    wetOnlyButton.setColour (juce::ToggleButton::tickDisabledColourId, kBorder);
    wetOnlyAttachment = std::make_unique<YMYVerbAudioProcessor::APVTS::ButtonAttachment> (
        processor.parameters, "wetOnly", wetOnlyButton);
    addAndMakeVisible (wetOnlyButton);

    lastShownPreset = processor.getPresetIndex();
    updateTempoReadout();
    startTimerHz (10);
}

YMYVerbAudioProcessorEditor::~YMYVerbAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
    for (auto& slider : sliders)
        slider.setLookAndFeel (nullptr);
}

void YMYVerbAudioProcessorEditor::timerCallback()
{
    const int preset = processor.getPresetIndex();
    if (preset != lastShownPreset)
    {
        presetBox.setSelectedId (preset + 1, juce::dontSendNotification);
        lastShownPreset = preset;
    }

    const bool synced = processor.parameters.getRawParameterValue ("sync")->load() >= 0.5f;
    sliders[0].setEnabled (! synced);
    divisionBox.setEnabled (synced);
    updateTempoReadout();

    const bool signalPresent = processor.getInputLevelDb() > -45.0f;
    micStatusLabel.setText (signalPresent ? "INPUT DETECTED  •  MIC ACTIVE" : "LISTENING FOR INPUT",
                            juce::dontSendNotification);
    micStatusLabel.setColour (juce::Label::textColourId, signalPresent ? kGold : kMuted);
}

void YMYVerbAudioProcessorEditor::updateTempoReadout()
{
    const double bpm = processor.getTempoBpm();
    const float ms = processor.getEffectivePreDelayMs();
    tempoLabel.setText ("HOST TEMPO  " + juce::String (bpm, 1) + " BPM", juce::dontSendNotification);
    detailLabel.setText ("EFFECTIVE PRE-DELAY  " + juce::String (ms, 1) + " ms", juce::dontSendNotification);
}

void YMYVerbAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.fillAll (kPaper);

    // Thin outer line and restrained hardware-style panels.
    g.setColour (kBorder);
    g.drawRoundedRectangle (bounds.reduced (8.0f), 12.0f, 1.0f);

    g.setColour (kPanel);
    g.fillRoundedRectangle (20.0f, 70.0f, bounds.getWidth() - 40.0f, 185.0f, 10.0f);
    g.setColour (kBorder);
    g.drawRoundedRectangle (20.0f, 70.0f, bounds.getWidth() - 40.0f, 185.0f, 10.0f, 1.0f);

    g.setColour (kInk);
    g.setFont (juce::Font (29.0f, juce::Font::bold));
    g.drawText ("YMYVerb", 26, 13, 185, 36, juce::Justification::centredLeft);

    g.setColour (kGold);
    g.setFont (juce::Font (10.5f, juce::Font::bold));
    g.drawText ("BACHATA VOCAL SPACE  /  ANALOGUE-INSPIRED REVERB", 28, 47, 475, 16, juce::Justification::centredLeft);

    g.setColour (kMuted);
    g.setFont (juce::Font (11.0f, juce::Font::bold));
    g.drawText ("BY MADE ORYMY  •  YMY RECORDS", getWidth() - 290, 25, 258, 19, juce::Justification::centredRight);

    g.setColour (kBorder);
    g.drawLine (25.0f, 63.5f, bounds.getWidth() - 25.0f, 63.5f, 1.0f);

    g.setColour (kBorder);
    g.drawLine (350.0f, 88.0f, 350.0f, 237.0f, 1.0f);
    g.drawLine (650.0f, 88.0f, 650.0f, 237.0f, 1.0f);

    // Upper panel captions and meter rails.
    g.setColour (kMuted);
    g.setFont (juce::Font (9.5f, juce::Font::bold));
    g.drawText ("PRESET LIBRARY", 29, 83, 200, 15, juce::Justification::centredLeft);
    g.drawText ("VOICE / INPUT MONITOR", 389, 83, 222, 15, juce::Justification::centred);
    g.drawText ("TEMPO FOLLOWER", 690, 83, 250, 15, juce::Justification::centredRight);

    // A small fixed meter scale to make the status panel read like a studio unit.
    for (int i = 0; i < 14; ++i)
    {
        const float xx = 706.0f + static_cast<float> (i) * 17.0f;
        g.setColour (i < 10 ? kBorder : juce::Colour::fromRGB (222, 192, 146));
        g.fillRoundedRectangle (xx, 203.0f, 12.0f, 5.0f, 2.0f);
    }

    // Control sections are separated by understated rules, not glossy textures.
    g.setColour (kBorder);
    g.drawLine (25.0f, 420.0f, bounds.getWidth() - 25.0f, 420.0f, 1.0f);
    g.drawLine (25.0f, 580.0f, bounds.getWidth() - 25.0f, 580.0f, 1.0f);

    g.setColour (kMuted);
    g.setFont (juce::Font (9.5f, juce::Font::bold));
    g.drawText ("SPACE & TONE", 29, 588, 120, 13, juce::Justification::centredLeft);

    g.setColour (kMuted.withAlpha (0.85f));
    g.setFont (juce::Font (9.5f));
    g.drawText ("Designed for bachata leads, doubles and backing vocals. No vocal model or artist samples are used.",
                525, getHeight() - 27, getWidth() - 555, 14, juce::Justification::centredRight);
}

void YMYVerbAudioProcessorEditor::resized()
{
    const float scaleX = static_cast<float> (getWidth()) / 1000.0f;
    const auto scaled = [scaleX] (int x) { return juce::roundToInt (static_cast<float> (x) * scaleX); };

    presetLabel.setBounds (scaled (29), 98, scaled (285), 18);
    presetBox.setBounds (scaled (28), 119, scaled (295), 35);
    rangeLabel.setBounds (scaled (30), 166, scaled (298), 66);

    micVisual.setBounds (scaled (435), 87, scaled (130), 151);
    micStatusLabel.setBounds (scaled (382), 239, scaled (236), 16);

    tempoLabel.setBounds (scaled (673), 111, scaled (285), 25);
    detailLabel.setBounds (scaled (673), 145, scaled (285), 22);

    const std::array<int, 5> rowOneCentres {{ 105, 302, 500, 698, 895 }};
    for (size_t i = 0; i < 5; ++i)
    {
        const int cx = scaled (rowOneCentres[i]);
        sliderLabels[i].setBounds (cx - scaled (78), 269, scaled (156), 17);
        sliders[i].setBounds (cx - scaled (54), 285, scaled (108), 122);
    }

    const std::array<int, 4> rowTwoCentres {{ 200, 400, 600, 800 }};
    for (size_t i = 0; i < 4; ++i)
    {
        const size_t idx = i + 5;
        const int cx = scaled (rowTwoCentres[i]);
        sliderLabels[idx].setBounds (cx - scaled (78), 428, scaled (156), 17);
        sliders[idx].setBounds (cx - scaled (54), 444, scaled (108), 122);
    }

    syncButton.setBounds (scaled (29), 612, scaled (132), 30);
    divisionBox.setBounds (scaled (191), 610, scaled (130), 32);
    wetOnlyButton.setBounds (scaled (355), 612, scaled (170), 30);
}
