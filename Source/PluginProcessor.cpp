#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
const juce::Identifier stateType { "BoneShiftState" };
const juce::Identifier languageProperty { "russianLanguage" };
}

ResampleCalcAudioProcessor::ResampleCalcAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

void ResampleCalcAudioProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    juce::ValueTree state(stateType);
    state.setProperty(languageProperty, russianLanguage.load(), nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destination);
}

void ResampleCalcAudioProcessor::setStateInformation(const void* data, int size)
{
    if (auto xml = getXmlFromBinary(data, size))
    {
        if (xml->hasTagName(stateType.toString()))
        {
            const auto state = juce::ValueTree::fromXml(*xml);
            russianLanguage.store((bool) state.getProperty(languageProperty, true));
        }
    }
}

void ResampleCalcAudioProcessor::setRussianLanguage(bool useRussian)
{
    if (russianLanguage.exchange(useRussian) == useRussian)
        return;

    updateHostDisplay(juce::AudioProcessorListener::ChangeDetails().withNonParameterStateChanged(true));
}

bool ResampleCalcAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet()
        && (layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono()
            || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo());
}

void ResampleCalcAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    // Analyzer/calculator only: transparent audio passthrough.
    juce::ignoreUnused(buffer);
}

juce::AudioProcessorEditor* ResampleCalcAudioProcessor::createEditor()
{
    return new ResampleCalcAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ResampleCalcAudioProcessor();
}
