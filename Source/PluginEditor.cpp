#include "PluginEditor.h"

namespace
{
const auto bgTop       = juce::Colour(0xff0a0b0f);
const auto bgBottom    = juce::Colour(0xff11131a);
const auto surface     = juce::Colour(0xff151820);
const auto surfaceHi   = juce::Colour(0xff1b1f29);
const auto border      = juce::Colour(0xff2a303b);
const auto text        = juce::Colour(0xfff5f6f8);
const auto muted       = juce::Colour(0xff858d9b);
const auto muted2      = juce::Colour(0xff5e6674);
const auto accent      = juce::Colour(0xffbc73ff);
const auto accentBlue  = juce::Colour(0xff5577ff);
const auto accentSoft  = juce::Colour(0xff8594ff);
const auto warm        = juce::Colour(0xffffa95c);
const auto flagBlue    = juce::Colour(0xff3c3b6e);
const auto flagRed     = juce::Colour(0xffb22234);

juce::String signedNumber(double value, int decimals)
{
    return (value >= 0.0 ? "+" : "") + juce::String(value, decimals);
}

juce::String utf8(const char* value)
{
    return juce::String::fromUTF8(value);
}
}

void LanguageFlagButton::paintButton(juce::Graphics& g, bool highlighted, bool down)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);
    g.setColour(surfaceHi.brighter((highlighted || down) ? 0.12f : 0.0f));
    g.fillRoundedRectangle(bounds, 5.0f);

    auto flag = bounds.reduced(9.0f, 7.0f);
    if (getToggleState())
    {
        g.setColour(juce::Colours::white);
        g.fillRect(flag);
        g.setColour(juce::Colour(0xff2455a4));
        g.fillRect(flag.getX(), flag.getY() + flag.getHeight() / 3.0f, flag.getWidth(), flag.getHeight() / 3.0f);
        g.setColour(juce::Colour(0xffd52b1e));
        g.fillRect(flag.getX(), flag.getY() + flag.getHeight() * 2.0f / 3.0f, flag.getWidth(), flag.getHeight() / 3.0f);
    }
    else
    {
        const float stripeHeight = flag.getHeight() / 13.0f;
        g.saveState();
        g.reduceClipRegion(flag.toNearestInt());
        for (int stripe = 0; stripe < 13; ++stripe)
        {
            g.setColour((stripe % 2 == 0) ? flagRed : juce::Colours::white);
            g.fillRect(flag.getX(), flag.getY() + stripe * stripeHeight, flag.getWidth(), stripeHeight + 1.0f);
        }
        g.setColour(flagBlue);
        g.fillRect(flag.getX(), flag.getY(), flag.getWidth() * 0.46f, stripeHeight * 7.0f);
        for (int row = 0; row < 3; ++row)
            for (int column = 0; column < 3; ++column)
            {
                g.setColour(juce::Colours::white.withAlpha(0.9f));
                g.fillEllipse(flag.getX() + 2.0f + column * 3.2f,
                              flag.getY() + 2.0f + row * 3.0f, 1.3f, 1.3f);
            }
        g.restoreState();
    }
    g.setColour(border.brighter(highlighted ? 0.35f : 0.0f));
    g.drawRoundedRectangle(bounds, 5.0f, 1.0f);
}

ResampleCalcAudioProcessorEditor::ResampleCalcAudioProcessorEditor(ResampleCalcAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(820, 560);
    setResizable(false, false);

    title.setText("YBC:", juce::dontSendNotification);
    title.setFont(juce::Font(juce::FontOptions(25.0f).withStyle("Bold")));
    title.setColour(juce::Label::textColourId, text);
    addAndMakeVisible(title);

    titleAccent.setText("CALC", juce::dontSendNotification);
    titleAccent.setFont(juce::Font(juce::FontOptions(25.0f).withStyle("Bold")));
    titleAccent.setColour(juce::Label::textColourId, accent);
    addAndMakeVisible(titleAccent);

    subtitle.setText("VARISPEED / BPM / KEY", juce::dontSendNotification);
    subtitle.setFont(juce::Font(juce::FontOptions(10.5f).withStyle("Bold")));
    subtitle.setColour(juce::Label::textColourId, muted);
    addAndMakeVisible(subtitle);

    dropZone.setText("DROP SAMPLE HERE", juce::dontSendNotification);
    dropZone.setJustificationType(juce::Justification::centredLeft);
    dropZone.setFont(juce::Font(juce::FontOptions(12.5f).withStyle("Bold")));
    dropZone.setColour(juce::Label::textColourId, text.withAlpha(0.85f));
    addAndMakeVisible(dropZone);

    status.setText("Manual mode", juce::dontSendNotification);
    status.setFont(juce::Font(juce::FontOptions(11.0f)));
    status.setColour(juce::Label::textColourId, muted);
    addAndMakeVisible(status);

    developerCredit.setFont(juce::Font(juce::FontOptions(9.5f).withStyle("Bold")));
    developerCredit.setColour(juce::Label::textColourId, muted);
    addAndMakeVisible(developerCredit);

    sourceBpmLabel.setText("SOURCE BPM", juce::dontSendNotification);
    sourceKeyLabel.setText("SOURCE KEY", juce::dontSendNotification);
    semitoneLabel.setText("VARISPEED", juce::dontSendNotification);
    targetBpmInputLabel.setText("TARGET BPM", juce::dontSendNotification);
    targetKeyCaption.setText("RESULT KEY", juce::dontSendNotification);
    centsCaption.setText("TUNING", juce::dontSendNotification);
    ratioCaption.setText("SPEED", juce::dontSendNotification);

    for (auto* l : { &sourceBpmLabel, &sourceKeyLabel, &semitoneLabel, &targetBpmInputLabel,
                     &targetKeyCaption, &centsCaption, &ratioCaption })
        styleCaption(*l);

    sourceBpm.setText("80.00");
    sourceBpm.setInputRestrictions(8, "0123456789.");
    styleField(sourceBpm);
    sourceBpm.onTextChange = [this] { if (! updatingFields) recalcFromPitch(); };
    addAndMakeVisible(sourceBpm);

    static const char* names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    int id = 1;
    for (auto* n : names)
    {
        sourceKey.addItem(juce::String(n) + " major", id++);
        sourceKey.addItem(juce::String(n) + " minor", id++);
    }
    sourceKey.setSelectedId(6);
    sourceKey.setColour(juce::ComboBox::backgroundColourId, surfaceHi);
    sourceKey.setColour(juce::ComboBox::textColourId, text);
    sourceKey.setColour(juce::ComboBox::outlineColourId, border);
    sourceKey.setColour(juce::ComboBox::arrowColourId, muted);
    sourceKey.onChange = [this] { if (! updatingFields) recalcFromPitch(); };
    addAndMakeVisible(sourceKey);

    semitones.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    semitones.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    semitones.setRange(-12.0, 12.0, 0.01);
    semitones.setValue(8.0);
    semitones.setVelocityBasedMode(false);
    semitones.setDoubleClickReturnValue(true, 0.0);
    semitones.setRotaryParameters(juce::MathConstants<float>::pi * 1.20f,
                                  juce::MathConstants<float>::pi * 2.80f, true);
    semitones.setColour(juce::Slider::rotarySliderFillColourId, accent);
    semitones.setColour(juce::Slider::rotarySliderOutlineColourId, border);
    semitones.setColour(juce::Slider::thumbColourId, text);
    semitones.onValueChange = [this]
    {
        if (! updatingFields)
        {
            const double value = semitones.getValue();
            if (latchedSemitone != 99)
            {
                if (std::abs(value - latchedSemitone) <= 0.34)
                    semitones.setValue((double) latchedSemitone, juce::dontSendNotification);
                else
                    latchedSemitone = 99;
            }

            if (latchedSemitone == 99)
            {
                const int nearestWholeSemitone = (int) std::round(value);
                if (std::abs(value - nearestWholeSemitone) <= 0.20)
                {
                    latchedSemitone = nearestWholeSemitone;
                    semitones.setValue((double) latchedSemitone, juce::dontSendNotification);
                }
            }
            recalcFromPitch();
        }
        glowAmount = 1.0f;
        repaint();
    };
    addAndMakeVisible(semitones);

    pitchReadout.setJustificationType(juce::Justification::centred);
    pitchReadout.setFont(juce::Font(juce::FontOptions(31.0f).withStyle("Bold")));
    pitchReadout.setColour(juce::Label::textColourId, text);
    pitchReadout.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(pitchReadout);

    pitchHint.setText("SEMITONES", juce::dontSendNotification);
    pitchHint.setJustificationType(juce::Justification::centred);
    pitchHint.setFont(juce::Font(juce::FontOptions(9.5f).withStyle("Bold")));
    pitchHint.setColour(juce::Label::textColourId, muted2);
    pitchHint.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(pitchHint);

    targetBpmInput.setText("126.99");
    targetBpmInput.setInputRestrictions(8, "0123456789.");
    styleField(targetBpmInput, true);
    targetBpmInput.onTextChange = [this] { if (! updatingFields) recalcFromTargetBpm(); };
    addAndMakeVisible(targetBpmInput);

    for (auto* l : { &targetKey, &cents, &ratio })
        styleValue(*l);
    targetKey.setColour(juce::Label::textColourId, accent);

    chooseFile.setColour(juce::TextButton::buttonColourId, accent);
    chooseFile.setColour(juce::TextButton::buttonOnColourId, accentSoft);
    chooseFile.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff10130a));
    chooseFile.setColour(juce::TextButton::textColourOnId, juce::Colour(0xff10130a));
    chooseFile.onClick = [this]
    {
        auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
        chooser.launchAsync(flags, [this](const juce::FileChooser& fc)
        {
            const auto f = fc.getResult();
            if (f.existsAsFile()) analyze(f);
        });
    };
    addAndMakeVisible(chooseFile);

    resetButton.setColour(juce::TextButton::buttonColourId, surfaceHi);
    resetButton.setColour(juce::TextButton::textColourOffId, muted);
    resetButton.onClick = [this]
    {
        updatingFields = true;
        sourceBpm.setText("80.00", false);
        setKeyCombo(2, true);
        semitones.setValue(8.0, juce::dontSendNotification);
        updatingFields = false;
        recalcFromPitch();
        updateStatus("Manual mode", utf8("Ручной режим"));
    };
    addAndMakeVisible(resetButton);

    languageButton.setClickingTogglesState(true);
        languageButton.setTooltip(juce::String("Switch language / ") + utf8("Переключить язык"));
    languageButton.onClick = [this] { setLanguage(languageButton.getToggleState()); };
    addAndMakeVisible(languageButton);

    semitoneInput.setText("+8.00", false);
    semitoneInput.setInputRestrictions(7, "+-0123456789.");
    semitoneInput.setFont(juce::Font(juce::FontOptions(13.0f).withStyle("Bold")));
    semitoneInput.setJustification(juce::Justification::centred);
    semitoneInput.setColour(juce::TextEditor::backgroundColourId, surfaceHi);
    semitoneInput.setColour(juce::TextEditor::textColourId, accent);
    semitoneInput.setColour(juce::TextEditor::outlineColourId, border);
    semitoneInput.setColour(juce::TextEditor::focusedOutlineColourId, accent.withAlpha(0.75f));
    semitoneInput.onReturnKey = [this] { commitSemitoneInput(); };
    semitoneInput.onFocusLost = [this] { commitSemitoneInput(); };
    addAndMakeVisible(semitoneInput);

    setLanguage(processor.isRussianLanguage());
    updateStatus("Manual mode", utf8("Ручной режим"));

    startTimerHz(60);
    recalcFromPitch();
}

void ResampleCalcAudioProcessorEditor::styleField(juce::TextEditor& editor, bool accentField)
{
    editor.setFont(juce::Font(juce::FontOptions(21.0f).withStyle("Bold")));
    editor.setJustification(juce::Justification::centredLeft);
    editor.setColour(juce::TextEditor::backgroundColourId, surfaceHi);
    editor.setColour(juce::TextEditor::textColourId, accentField ? accent : text);
    editor.setColour(juce::TextEditor::outlineColourId, border);
    editor.setColour(juce::TextEditor::focusedOutlineColourId, accent.withAlpha(0.75f));
    editor.setColour(juce::TextEditor::highlightColourId, accent.withAlpha(0.22f));
    editor.setColour(juce::TextEditor::highlightedTextColourId, text);
    addAndMakeVisible(editor);
}

void ResampleCalcAudioProcessorEditor::styleCaption(juce::Label& label)
{
    label.setFont(juce::Font(juce::FontOptions(10.0f).withStyle("Bold")));
    label.setColour(juce::Label::textColourId, muted);
    addAndMakeVisible(label);
}

void ResampleCalcAudioProcessorEditor::styleValue(juce::Label& label, bool accentValue)
{
    label.setFont(juce::Font(juce::FontOptions(23.0f).withStyle("Bold")));
    label.setColour(juce::Label::textColourId, accentValue ? accent : text);
    addAndMakeVisible(label);
}

void ResampleCalcAudioProcessorEditor::paint(juce::Graphics& g)
{
    juce::ColourGradient bgGradient(bgTop, 0.0f, 0.0f, bgBottom, 0.0f, (float) getHeight(), false);
    g.setGradientFill(bgGradient);
    g.fillAll();

    const auto full = getLocalBounds().toFloat();

    // Layered bloom and orbiting meter accents keep the control visibly alive.
    const float pulse = 0.5f + 0.5f * std::sin(animationPhase);
    const float busy = pendingAnalysis != nullptr ? 1.0f : 0.0f;
    const float ambience = 0.055f + pulse * 0.055f + busy * 0.10f;
    const float glowX = 410.0f + std::sin(animationPhase) * 110.0f;
    const float glowY = 270.0f + std::cos(animationPhase * 2.0f) * 55.0f;
    juce::ColourGradient ambient(accent.withAlpha(ambience), glowX, glowY,
                                 accentBlue.withAlpha(ambience * 0.56f), 810.0f - glowX * 0.25f, 520.0f, true);
    g.setGradientFill(ambient);
    g.fillRect(full);

    juce::ColourGradient wash(accent.withAlpha(0.045f + pulse * 0.025f), 0.0f, 80.0f,
                              accentBlue.withAlpha(0.035f + (1.0f - pulse) * 0.025f), full.getWidth(), 460.0f, false);
    g.setGradientFill(wash);
    g.fillRect(full);

    // Header divider.
    g.setColour(border.withAlpha(0.55f));
    g.fillRect(28.0f, 73.0f, full.getWidth() - 56.0f, 1.0f);

    // Drop zone.
    auto drop = juce::Rectangle<float>(28.0f, 91.0f, full.getWidth() - 56.0f, 72.0f);
    g.setColour(surface.withAlpha(0.88f));
    g.fillRoundedRectangle(drop, 14.0f);
    g.setColour(border);
    g.drawRoundedRectangle(drop, 14.0f, 1.0f);

    // Main stage.
    auto stage = juce::Rectangle<float>(28.0f, 181.0f, full.getWidth() - 56.0f, 281.0f);
    g.setColour(surface.withAlpha(0.72f));
    g.fillRoundedRectangle(stage, 18.0f);
    g.setColour(border.withAlpha(0.78f));
    g.drawRoundedRectangle(stage, 18.0f, 1.0f);

    // Vertical separators keep the layout calm and plugin-like.
    g.setColour(border.withAlpha(0.7f));
    g.fillRect(273.0f, 205.0f, 1.0f, 229.0f);
    g.fillRect(548.0f, 205.0f, 1.0f, 229.0f);

    // Central reactive halo and rotating arc markers behind the knob.
    const auto knobBounds = semitones.getBounds().toFloat();
    const auto c = knobBounds.getCentre();
    const float magnitude = juce::jlimit(0.0f, 1.0f, (float) std::abs(semitones.getValue()) / 12.0f);
    const float reactive = 0.24f + magnitude * 0.24f + glowAmount * 0.28f;

    for (int i = 4; i >= 1; --i)
    {
        const float diameter = knobBounds.getWidth() + 18.0f * (float) i;
        const float alpha = reactive * (0.030f + 0.014f * (float) (5 - i));
        g.setColour(accent.withAlpha(alpha));
        g.fillEllipse(c.x - diameter * 0.5f, c.y - diameter * 0.5f, diameter, diameter);
    }

    juce::ColourGradient ringGradient(accent.withAlpha(0.28f + glowAmount * 0.22f),
                                      knobBounds.getX(), knobBounds.getY(),
                                      accentBlue.withAlpha(0.24f + pulse * 0.20f),
                                      knobBounds.getRight(), knobBounds.getBottom(), false);
    g.setGradientFill(ringGradient);
    g.drawEllipse(knobBounds.expanded(7.0f), 1.2f);

    for (int tick = 0; tick <= 24; ++tick)
    {
        const float angle = juce::MathConstants<float>::pi * 1.20f
                          + (float) tick / 24.0f * juce::MathConstants<float>::pi * 1.60f;
        const float outerRadius = knobBounds.getWidth() * 0.5f + 9.0f;
        const float innerRadius = outerRadius - (tick % 2 == 0 ? 7.0f : 4.0f);
        g.setColour(accent.interpolatedWith(accentBlue, (float) tick / 24.0f)
                 .withAlpha(tick % 2 == 0 ? 0.72f : 0.34f));
        g.drawLine(c.x + std::cos(angle) * innerRadius,
                   c.y + std::sin(angle) * innerRadius,
                   c.x + std::cos(angle) * outerRadius,
                   c.y + std::sin(angle) * outerRadius,
                   tick % 2 == 0 ? 1.5f : 1.0f);
    }

    for (int arc = 0; arc < 4; ++arc)
    {
        const float radius = knobBounds.getWidth() * 0.5f + 14.0f + arc * 8.0f;
        const float direction = arc % 2 == 0 ? 1.0f : -1.0f;
        const float speed = 1.0f + (float) (arc / 2);
        const float start = animationPhase * direction * speed + arc * 1.7f;
        juce::Path orbit;
        orbit.addCentredArc(c.x, c.y, radius, radius, 0.0f, start, start + 1.45f + pulse * 0.55f, true);
        juce::ColourGradient orbitGradient(accent.withAlpha(0.30f + pulse * 0.25f), c.x - radius, c.y,
                                           accentBlue.withAlpha(0.42f + pulse * 0.28f), c.x + radius, c.y, false);
        g.setGradientFill(orbitGradient);
        g.strokePath(orbit, juce::PathStrokeType(1.2f + arc * 0.25f));

        const float dotX = c.x + std::cos(start) * radius;
        const float dotY = c.y + std::sin(start) * radius;
        const auto dotColour = arc == 1 ? warm : accentBlue;
        g.setColour(dotColour.withAlpha(0.18f + pulse * 0.20f));
        g.fillEllipse(dotX - 6.0f, dotY - 6.0f, 12.0f, 12.0f);
        g.setColour(dotColour.withAlpha(0.72f + pulse * 0.25f));
        g.fillEllipse(dotX - 2.2f, dotY - 2.2f, 4.4f, 4.4f);
    }

    for (int particle = 0; particle < 18; ++particle)
    {
        const float orbit = knobBounds.getWidth() * 0.58f + (float) (particle % 4) * 12.0f;
        const float angle = animationPhase * (particle % 2 == 0 ? 1.0f : -2.0f)
                          + (float) particle * juce::MathConstants<float>::twoPi / 18.0f;
        const float x = c.x + std::cos(angle) * orbit;
        const float y = c.y + std::sin(angle) * orbit * 0.72f;
        const float twinkle = 0.5f + 0.5f * std::sin(animationPhase * 3.0f + particle * 2.3f);
        const auto particleColour = accent.interpolatedWith(accentBlue, (float) particle / 17.0f);
        g.setColour(particleColour.withAlpha(0.08f + twinkle * 0.16f));
        g.fillEllipse(x - 4.0f, y - 4.0f, 8.0f, 8.0f);
        g.setColour(particleColour.withAlpha(0.24f + twinkle * 0.55f));
        g.fillEllipse(x - 1.0f, y - 1.0f, 2.0f, 2.0f);
    }

    // Bottom info strip.
    auto strip = juce::Rectangle<float>(28.0f, 480.0f, full.getWidth() - 56.0f, 52.0f);
    g.setColour(surface.withAlpha(0.70f));
    g.fillRoundedRectangle(strip, 13.0f);
    g.setColour(border.withAlpha(0.75f));
    g.drawRoundedRectangle(strip, 13.0f, 1.0f);

    // Small accent marker in the footer, inspired by premium plugin metering without copying a brand UI.
    juce::ColourGradient footerGradient(accent.withAlpha(0.9f), 44.0f, 0.0f,
                                        accentBlue.withAlpha(0.9f), 180.0f, 0.0f, false);
    g.setGradientFill(footerGradient);
    g.fillRoundedRectangle(44.0f, 503.0f, 24.0f + 34.0f * magnitude, 3.0f, 1.5f);

    if (busy > 0.0f)
    {
        const float scanX = 36.0f + analysisSweep * (full.getWidth() - 72.0f);
        g.setColour(accent.withAlpha(0.7f));
        g.fillRoundedRectangle(scanX - 1.5f, 91.0f, 3.0f, 72.0f, 1.5f);
    }
}

void ResampleCalcAudioProcessorEditor::resized()
{
    title.setBounds(28, 19, 75, 32);
    titleAccent.setBounds(106, 19, 190, 32);
    subtitle.setBounds(29, 49, 440, 18);
    resetButton.setBounds(706, 25, 84, 28);
    languageButton.setBounds(650, 24, 42, 30);

    dropZone.setBounds(48, 103, 260, 24);
    status.setBounds(48, 127, 440, 22);
    developerCredit.setBounds(448, 488, 320, 28);
    developerCredit.setJustificationType(juce::Justification::centredRight);
    chooseFile.setBounds(638, 108, 132, 36);

    // Source column.
    sourceBpmLabel.setBounds(54, 213, 120, 18);
    sourceBpm.setBounds(54, 235, 178, 46);
    sourceKeyLabel.setBounds(54, 307, 178, 18);
    sourceKey.setBounds(54, 329, 178, 42);

    // Center column.
    semitoneLabel.setBounds(302, 210, 110, 18);
    semitoneLabel.setJustificationType(juce::Justification::centred);
    semitones.setBounds(315, 236, 190, 190);
    pitchReadout.setBounds(333, 294, 154, 48);
    pitchHint.setBounds(333, 337, 154, 18);
    semitoneInput.setBounds(362, 429, 96, 25);

    // Result column.
    targetBpmInputLabel.setBounds(581, 213, 140, 18);
    targetBpmInput.setBounds(581, 235, 180, 46);

    targetKeyCaption.setBounds(581, 306, 180, 18);
    targetKey.setBounds(581, 326, 180, 34);
    centsCaption.setBounds(581, 373, 85, 18);
    cents.setBounds(581, 393, 120, 32);
    ratioCaption.setBounds(704, 373, 60, 18);
    ratio.setBounds(704, 393, 72, 32);

    status.setJustificationType(juce::Justification::centredLeft);
}

bool ResampleCalcAudioProcessorEditor::isInterestedInFileDrag(const juce::StringArray& files)
{
    if (files.isEmpty()) return false;
    const auto ext = juce::File(files[0]).getFileExtension().toLowerCase();
    return ext == ".wav" || ext == ".mp3" || ext == ".aiff" || ext == ".aif" || ext == ".flac";
}

void ResampleCalcAudioProcessorEditor::filesDropped(const juce::StringArray& files, int, int)
{
    if (! files.isEmpty()) analyze(juce::File(files[0]));
}

void ResampleCalcAudioProcessorEditor::analyze(const juce::File& file)
{
    if (pendingAnalysis != nullptr)
        return;

    updateStatus("Analyzing  •  " + file.getFileName(), utf8("Анализ  •  ") + file.getFileName());
    pendingAnalysis = std::make_unique<std::future<AnalysisResult>>(
        std::async(std::launch::async, [file] { return SampleAnalyzer::analyzeFile(file); }));
}

void ResampleCalcAudioProcessorEditor::timerCallback()
{
    animationPhase += 0.022f;
    if (animationPhase > juce::MathConstants<float>::twoPi)
        animationPhase -= juce::MathConstants<float>::twoPi;

    glowAmount *= 0.982f;
    if (pendingAnalysis != nullptr)
    {
        analysisSweep += 0.003f * analysisSweepDirection;
        if (analysisSweep >= 1.0f || analysisSweep <= 0.0f)
        {
            analysisSweep = juce::jlimit(0.0f, 1.0f, analysisSweep);
            analysisSweepDirection = -analysisSweepDirection;
        }
    }
    repaint();

    if (pendingAnalysis == nullptr)
        return;

    using namespace std::chrono_literals;
    if (pendingAnalysis->wait_for(0ms) != std::future_status::ready)
        return;

    auto result = pendingAnalysis->get();
    pendingAnalysis.reset();

    if (result.error.isNotEmpty())
    {
        const auto russianError = result.error == "Unsupported audio file" ? utf8("Формат файла не поддерживается")
                : result.error == "Audio file is too short" ? utf8("Аудиофайл слишком короткий")
                : utf8("Не удалось прочитать аудиофайл");
        updateStatus(result.error, russianError);
        return;
    }

    updatingFields = true;
    if (result.bpm > 0.0)
        sourceBpm.setText(juce::String(result.bpm, 2), false);
    setKeyCombo(result.root, result.minor);
    updatingFields = false;

    const auto key = SampleAnalyzer::keyName(result.root, result.minor);
    const auto bpmConfidence = juce::String((int) std::round(result.bpmConfidence * 100.0)) + "%";
    const auto keyConfidence = juce::String((int) std::round(result.keyConfidence * 100.0)) + "%";
    updateStatus("Detected  •  " + juce::String(result.bpm, 1) + " BPM  •  " + key
                     + "  •  BPM " + bpmConfidence + "  •  KEY " + keyConfidence,
                 utf8("Найдено  •  ") + juce::String(result.bpm, 1) + " BPM  •  " + key
                     + "  •  BPM " + bpmConfidence + "  •  " + utf8("ТОНАЛЬНОСТЬ ") + keyConfidence);
    glowAmount = 1.0f;
    recalcFromPitch();
}

int ResampleCalcAudioProcessorEditor::getSelectedRoot() const
{
    const int idx = juce::jmax(0, sourceKey.getSelectedId() - 1);
    return idx / 2;
}

bool ResampleCalcAudioProcessorEditor::getSelectedMinor() const
{
    const int idx = juce::jmax(0, sourceKey.getSelectedId() - 1);
    return (idx % 2) == 1;
}

void ResampleCalcAudioProcessorEditor::setKeyCombo(int root, bool minor)
{
    root = ((root % 12) + 12) % 12;
    sourceKey.setSelectedId(root * 2 + (minor ? 2 : 1), juce::dontSendNotification);
}

void ResampleCalcAudioProcessorEditor::recalcFromPitch()
{
    const double bpm = sourceBpm.getText().getDoubleValue();
    const double st = semitones.getValue();

    if (bpm <= 0.0)
    {
        targetBpmInput.setText("", false);
        refreshResult(st);
        return;
    }

    const double speed = std::pow(2.0, st / 12.0);
    const double newBpm = bpm * speed;

    updatingFields = true;
    targetBpmInput.setText(juce::String(newBpm, 2), false);
    updatingFields = false;
    refreshResult(st);
}

void ResampleCalcAudioProcessorEditor::recalcFromTargetBpm()
{
    const double source = sourceBpm.getText().getDoubleValue();
    const double target = targetBpmInput.getText().getDoubleValue();

    if (source <= 0.0 || target <= 0.0)
        return;

    const double st = 12.0 * std::log2(target / source);
    const double clamped = juce::jlimit(-12.0, 12.0, st);

    updatingFields = true;
    semitones.setValue(clamped, juce::dontSendNotification);
    updatingFields = false;
    glowAmount = 1.0f;
    refreshResult(st);
}

void ResampleCalcAudioProcessorEditor::refreshResult(double exactSemitones)
{
    const double source = sourceBpm.getText().getDoubleValue();
    const double speed = std::pow(2.0, exactSemitones / 12.0);

    pitchReadout.setText(signedNumber(exactSemitones, 2), juce::dontSendNotification);
    if (! semitoneInput.hasKeyboardFocus(true))
        semitoneInput.setText(signedNumber(semitones.getValue(), 2), false);

    const int nearestSt = (int) std::round(exactSemitones);
    const double centOffset = (exactSemitones - nearestSt) * 100.0;
    const int newRoot = ((getSelectedRoot() + nearestSt) % 12 + 12) % 12;
    targetKey.setText(SampleAnalyzer::keyName(newRoot, getSelectedMinor()), juce::dontSendNotification);

    juce::String tuning;
    if (std::abs(centOffset) < 0.5)
        tuning = russianLanguage ? utf8("0 ц") : "0 ct";
    else
        tuning = signedNumber(centOffset, 0) + (russianLanguage ? utf8(" ц") : juce::String(" ct"));
    cents.setText(tuning, juce::dontSendNotification);

    ratio.setText(source > 0.0 ? "x" + juce::String(speed, 3) : "—", juce::dontSendNotification);
}

void ResampleCalcAudioProcessorEditor::setLanguage(bool useRussian)
{
    russianLanguage = useRussian;
    processor.setRussianLanguage(russianLanguage);
    languageButton.setToggleState(russianLanguage, juce::dontSendNotification);
    languageButton.setTooltip(russianLanguage ? utf8("Русский язык") : "English language");

    title.setText("YBC:", juce::dontSendNotification);
    titleAccent.setText("KeyShifter", juce::dontSendNotification);
    subtitle.setText(russianLanguage ? utf8("ВАРИСПИД / BPM / ТОНАЛЬНОСТЬ") : "VARISPEED / BPM / KEY", juce::dontSendNotification);
    dropZone.setText(russianLanguage ? utf8("ПЕРЕТАЩИТЕ СЭМПЛ СЮДА") : "DROP SAMPLE HERE", juce::dontSendNotification);
    developerCredit.setText(russianLanguage ? utf8("Разработано: YoungBonesClub:musiq") : "Developed by: YoungBonesClub:musiq", juce::dontSendNotification);
    sourceBpmLabel.setText(russianLanguage ? utf8("ИСХОДНЫЙ BPM") : "SOURCE BPM", juce::dontSendNotification);
    sourceKeyLabel.setText(russianLanguage ? utf8("ИСХОДНАЯ ТОНАЛЬНОСТЬ") : "SOURCE KEY", juce::dontSendNotification);
    semitoneLabel.setText(russianLanguage ? utf8("ВАРИСПИД") : "VARISPEED", juce::dontSendNotification);
    pitchHint.setText(russianLanguage ? utf8("ПОЛУТОНЫ") : "SEMITONES", juce::dontSendNotification);
    targetBpmInputLabel.setText(russianLanguage ? utf8("ЦЕЛЕВОЙ BPM") : "TARGET BPM", juce::dontSendNotification);
    targetKeyCaption.setText(russianLanguage ? utf8("НОВАЯ ТОНАЛЬНОСТЬ") : "RESULT KEY", juce::dontSendNotification);
    centsCaption.setText(russianLanguage ? utf8("СТРОЙ") : "TUNING", juce::dontSendNotification);
    ratioCaption.setText(russianLanguage ? utf8("СКОРОСТЬ") : "SPEED", juce::dontSendNotification);
    chooseFile.setButtonText(russianLanguage ? utf8("ЗАГРУЗИТЬ СЭМПЛ") : "LOAD SAMPLE");
    resetButton.setButtonText(russianLanguage ? utf8("СБРОС") : "RESET");
    titleAccent.setBounds(106, 19, 190, 32);

    const int selectedRoot = getSelectedRoot();
    const bool selectedMinor = getSelectedMinor();
    static const char* noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    sourceKey.clear(juce::dontSendNotification);
    int itemId = 1;
    for (const auto* note : noteNames)
    {
        sourceKey.addItem(juce::String(note) + (russianLanguage ? utf8(" мажор") : juce::String(" major")), itemId++);
        sourceKey.addItem(juce::String(note) + (russianLanguage ? utf8(" минор") : juce::String(" minor")), itemId++);
    }
    setKeyCombo(selectedRoot, selectedMinor);
    status.setText(russianLanguage ? russianStatus : englishStatus, juce::dontSendNotification);
    refreshResult(semitones.getValue());
}

void ResampleCalcAudioProcessorEditor::commitSemitoneInput()
{
    const auto enteredText = semitoneInput.getText().trim();
    if (enteredText.isEmpty())
    {
        semitoneInput.setText(signedNumber(semitones.getValue(), 2), false);
        return;
    }

    const double value = juce::jlimit(-12.0, 12.0, enteredText.getDoubleValue());
    updatingFields = true;
    semitones.setValue(value, juce::dontSendNotification);
    updatingFields = false;
    recalcFromPitch();
    semitoneInput.setText(signedNumber(semitones.getValue(), 2), false);
    glowAmount = 1.0f;
}

void ResampleCalcAudioProcessorEditor::updateStatus(const juce::String& english, const juce::String& russian)
{
    englishStatus = english;
    russianStatus = russian.isNotEmpty() ? russian : english;
    status.setText(russianLanguage ? russianStatus : englishStatus, juce::dontSendNotification);
}
