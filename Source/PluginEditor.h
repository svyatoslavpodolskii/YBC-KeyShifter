#pragma once
#include <JuceHeader.h>
#include <future>
#include <chrono>
#include "PluginProcessor.h"
#include "SampleAnalyzer.h"

class LanguageFlagButton final : public juce::Button
{
public:
    LanguageFlagButton() : juce::Button("Language") {}
    void paintButton(juce::Graphics&, bool highlighted, bool down) override;
};

class ResampleCalcAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                                public juce::FileDragAndDropTarget,
                                                private juce::Timer
{
public:
    explicit ResampleCalcAudioProcessorEditor(ResampleCalcAudioProcessor&);
    ~ResampleCalcAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

private:
    void timerCallback() override;
    void analyze(const juce::File& file);
    void recalcFromPitch();
    void recalcFromTargetBpm();
    void refreshResult(double exactSemitones);
    int getSelectedRoot() const;
    bool getSelectedMinor() const;
    void setKeyCombo(int root, bool minor);
    void setLanguage(bool useRussian);
    void commitSemitoneInput();
    void updateStatus(const juce::String& english, const juce::String& russian = {});
    void styleField(juce::TextEditor& editor, bool accentField = false);
    void styleCaption(juce::Label& label);
    void styleValue(juce::Label& label, bool accentValue = false);

    ResampleCalcAudioProcessor& processor;

    juce::Label title;
    juce::Label titleAccent;
    juce::Label subtitle;
    juce::Label dropZone;
    juce::Label status;
    juce::Label developerCredit;

    juce::Label sourceBpmLabel;
    juce::TextEditor sourceBpm;
    juce::Label sourceKeyLabel;
    juce::ComboBox sourceKey;

    juce::Label semitoneLabel;
    juce::Slider semitones;
    juce::Label pitchReadout;
    juce::Label pitchHint;
    juce::TextEditor semitoneInput;

    juce::Label targetBpmInputLabel;
    juce::TextEditor targetBpmInput;

    juce::Label targetKeyCaption;
    juce::Label targetKey;
    juce::Label centsCaption;
    juce::Label cents;
    juce::Label ratioCaption;
    juce::Label ratio;

    juce::TextButton chooseFile { "LOAD SAMPLE" };
    juce::TextButton resetButton { "RESET" };
    LanguageFlagButton languageButton;

    juce::FileChooser chooser { "Choose an audio sample" };
    std::unique_ptr<std::future<AnalysisResult>> pendingAnalysis;
    bool updatingFields = false;
    bool russianLanguage = false;
    juce::String englishStatus;
    juce::String russianStatus;
    int latchedSemitone = 99;

    float animationPhase = 0.0f;
    float glowAmount = 0.0f;
    float analysisSweep = 0.0f;
    float analysisSweepDirection = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ResampleCalcAudioProcessorEditor)
};
