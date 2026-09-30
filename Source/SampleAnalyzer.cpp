#include "SampleAnalyzer.h"
#include <numeric>

namespace
{
constexpr double pi = juce::MathConstants<double>::pi;

static double dotNormalized(const std::array<double, 12>& a,
                            const std::array<double, 12>& b)
{
    double dot = 0.0, aa = 0.0, bb = 0.0;
    for (int i = 0; i < 12; ++i)
    {
        dot += a[(size_t) i] * b[(size_t) i];
        aa += a[(size_t) i] * a[(size_t) i];
        bb += b[(size_t) i] * b[(size_t) i];
    }
    return dot / (std::sqrt(aa * bb) + 1.0e-12);
}

static std::array<double, 12> rotateProfile(const std::array<double, 12>& p, int root)
{
    std::array<double, 12> out {};
    for (int i = 0; i < 12; ++i)
        out[(size_t) ((i + root) % 12)] = p[(size_t) i];
    return out;
}
}

AnalysisResult SampleAnalyzer::analyzeFile(const juce::File& file)
{
    AnalysisResult result;
    juce::AudioFormatManager fm;
    fm.registerBasicFormats();

    std::unique_ptr<juce::AudioFormatReader> reader(fm.createReaderFor(file));
    if (reader == nullptr)
    {
        result.error = "Unsupported audio file";
        return result;
    }

    const auto maxSamples = (juce::int64) std::min<double>(reader->lengthInSamples,
                                                           reader->sampleRate * 90.0);
    if (maxSamples < 4096)
    {
        result.error = "Audio file is too short";
        return result;
    }

    juce::AudioBuffer<float> input((int) reader->numChannels, (int) maxSamples);
    if (! reader->read(&input, 0, (int) maxSamples, 0, true, true))
    {
        result.error = "Could not read audio";
        return result;
    }

    juce::AudioBuffer<float> mono(1, (int) maxSamples);
    mono.clear();
    for (int ch = 0; ch < input.getNumChannels(); ++ch)
        mono.addFrom(0, 0, input, ch, 0, input.getNumSamples(), 1.0f / input.getNumChannels());

    result.bpm = detectBpm(mono, reader->sampleRate, result.bpmConfidence);
    detectKey(mono, reader->sampleRate, result.root, result.minor, result.keyConfidence);
    return result;
}

double SampleAnalyzer::detectBpm(const juce::AudioBuffer<float>& mono,
                                 double sampleRate,
                                 double& confidence)
{
    // Onset-envelope autocorrelation. Envelope is sampled at ~200 Hz.
    const int hop = juce::jmax(1, (int) std::round(sampleRate / 200.0));
    const int n = mono.getNumSamples();
    const float* x = mono.getReadPointer(0);

    std::vector<double> env;
    env.reserve((size_t) (n / hop + 1));

    double prevEnergy = 0.0;
    double smooth = 0.0;
    for (int pos = 0; pos < n; pos += hop)
    {
        const int end = juce::jmin(n, pos + hop);
        double e = 0.0;
        for (int i = pos; i < end; ++i)
            e += std::abs((double) x[i]);
        e /= juce::jmax(1, end - pos);

        smooth = 0.85 * smooth + 0.15 * e;
        const double onset = juce::jmax(0.0, e - prevEnergy * 0.92 - smooth * 0.05);
        env.push_back(onset);
        prevEnergy = e;
    }

    if (env.size() < 100)
        return 0.0;

    const double mean = std::accumulate(env.begin(), env.end(), 0.0) / env.size();
    for (auto& v : env)
        v = juce::jmax(0.0, v - mean * 0.55);

    const double envRate = sampleRate / hop;
    constexpr double minBpm = 55.0;
    constexpr double maxBpm = 210.0;
    const int minLag = (int) std::floor(envRate * 60.0 / maxBpm);
    const int maxLag = (int) std::ceil(envRate * 60.0 / minBpm);

    double bestScore = -1.0;
    int bestLag = 0;
    double scoreSum = 0.0;
    int scoreCount = 0;

    for (int lag = minLag; lag <= maxLag; ++lag)
    {
        double num = 0.0, a = 0.0, b = 0.0;
        for (size_t i = (size_t) lag; i < env.size(); ++i)
        {
            const double v1 = env[i];
            const double v2 = env[i - (size_t) lag];
            num += v1 * v2;
            a += v1 * v1;
            b += v2 * v2;
        }

        double s = num / (std::sqrt(a * b) + 1.0e-12);

        // Light harmonic reinforcement helps avoid half/double-time mistakes.
        if (lag * 2 <= maxLag)
        {
            double num2 = 0.0, a2 = 0.0, b2 = 0.0;
            for (size_t i = (size_t) (lag * 2); i < env.size(); ++i)
            {
                const double v1 = env[i];
                const double v2 = env[i - (size_t) (lag * 2)];
                num2 += v1 * v2;
                a2 += v1 * v1;
                b2 += v2 * v2;
            }
            s += 0.18 * num2 / (std::sqrt(a2 * b2) + 1.0e-12);
        }

        scoreSum += s;
        ++scoreCount;
        if (s > bestScore)
        {
            bestScore = s;
            bestLag = lag;
        }
    }

    if (bestLag <= 0)
        return 0.0;

    double bpm = 60.0 * envRate / bestLag;

    // Prefer musically common sample-tempo range when candidates are octave-related.
    while (bpm < 70.0) bpm *= 2.0;
    while (bpm > 180.0) bpm *= 0.5;

    const double avg = scoreCount > 0 ? scoreSum / scoreCount : 0.0;
    confidence = juce::jlimit(0.0, 1.0, (bestScore - avg) * 2.2);
    return bpm;
}

void SampleAnalyzer::detectKey(const juce::AudioBuffer<float>& mono,
                               double sampleRate,
                               int& root,
                               bool& minor,
                               double& confidence)
{
    constexpr int fftOrder = 12; // 4096
    constexpr int fftSize = 1 << fftOrder;
    constexpr int hop = 2048;

    juce::dsp::FFT fft(fftOrder);
    juce::dsp::WindowingFunction<float> window(fftSize,
                                               juce::dsp::WindowingFunction<float>::hann,
                                               true);

    std::array<double, 12> chroma {};
    const float* x = mono.getReadPointer(0);
    const int n = mono.getNumSamples();

    std::vector<float> block((size_t) fftSize * 2, 0.0f);
    int frames = 0;

    for (int pos = 0; pos + fftSize < n; pos += hop)
    {
        std::fill(block.begin(), block.end(), 0.0f);
        std::copy(x + pos, x + pos + fftSize, block.begin());
        window.multiplyWithWindowingTable(block.data(), fftSize);
        fft.performFrequencyOnlyForwardTransform(block.data());

        double frameEnergy = 0.0;
        for (int bin = 1; bin < fftSize / 2; ++bin)
        {
            const double freq = (double) bin * sampleRate / fftSize;
            if (freq < 55.0 || freq > 5000.0)
                continue;

            const double mag = std::sqrt(juce::jmax(0.0f, block[(size_t) bin]));
            if (mag <= 1.0e-8)
                continue;

            const double midi = 69.0 + 12.0 * std::log2(freq / 440.0);
            const int note = (int) std::lround(midi);
            const int pc = ((note % 12) + 12) % 12;

            // Mild low/mid emphasis: harmonic fundamentals matter more than fizz.
            const double weight = mag / std::sqrt(1.0 + freq / 900.0);
            chroma[(size_t) pc] += weight;
            frameEnergy += weight;
        }

        if (frameEnergy > 0.0)
            ++frames;
    }

    if (frames == 0)
    {
        root = 0;
        minor = true;
        confidence = 0.0;
        return;
    }

    // Krumhansl-Kessler tonal profiles, ordered C..B relative to tonic C.
    const std::array<double, 12> majorProfile {
        6.35, 2.23, 3.48, 2.33, 4.38, 4.09, 2.52, 5.19, 2.39, 3.66, 2.29, 2.88
    };
    const std::array<double, 12> minorProfile {
        6.33, 2.68, 3.52, 5.38, 2.60, 3.53, 2.54, 4.75, 3.98, 2.69, 3.34, 3.17
    };

    double best = -1.0, second = -1.0;
    int bestRoot = 0;
    bool bestMinor = false;

    for (int r = 0; r < 12; ++r)
    {
        const auto maj = rotateProfile(majorProfile, r);
        const auto min = rotateProfile(minorProfile, r);
        const double majScore = dotNormalized(chroma, maj);
        const double minScore = dotNormalized(chroma, min);

        for (auto pair : { std::pair<double, bool>{majScore, false},
                           std::pair<double, bool>{minScore, true} })
        {
            if (pair.first > best)
            {
                second = best;
                best = pair.first;
                bestRoot = r;
                bestMinor = pair.second;
            }
            else if (pair.first > second)
            {
                second = pair.first;
            }
        }
    }

    root = bestRoot;
    minor = bestMinor;
    confidence = juce::jlimit(0.0, 1.0, (best - second) * 8.0);
}

juce::String SampleAnalyzer::keyName(int root, bool minor)
{
    static const char* names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    root = ((root % 12) + 12) % 12;
    return juce::String(names[root]) + (minor ? "m" : "");
}
