#pragma once
#include <JuceHeader.h>

struct AnalysisResult
{
    double bpm = 0.0;
    int root = 0;          // 0=C ... 11=B
    bool minor = true;
    double keyConfidence = 0.0;
    double bpmConfidence = 0.0;
    juce::String error;
};

class SampleAnalyzer
{
public:
    static AnalysisResult analyzeFile(const juce::File& file);
    static juce::String keyName(int root, bool minor);

private:
    static double detectBpm(const juce::AudioBuffer<float>& mono,
                            double sampleRate,
                            double& confidence);
    static void detectKey(const juce::AudioBuffer<float>& mono,
                          double sampleRate,
                          int& root,
                          bool& minor,
                          double& confidence);
};
