/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

enum Slope_Choice
{
    slope_12,
    slope_24,
    slope_36,
    slope_48,
};

enum ChainPosition
{
    LowCut,
    Peak,
    HighCut
};

struct ChainSettings
{
    float peakFreq{ 0 }, peakGainInDecibels{ 0 }, peakQuality{ 1.f };
    float lowCutFreq{ 0 }, highCutFreq{ 0 };
    int lowCutSlope{ 0 }, highCutSlope{ 0 };
    float toneStrength{ 0 }, originalAttenuation{ 0 };
};

// ==== Aliases ====
using Filter = juce::dsp::IIR::Filter<float>;
using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>; // Make cut filter with 4 IIRs
using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;  // Low, Peak, High
using Coefficient = Filter::CoefficientsPtr;

// ==== Free Functions ====
void UpdateCoefficients(Coefficient& old, const Coefficient& replacement);

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);
Coefficient makePeakFilter(const ChainSettings& chain_settings, double sampleRate);
Coefficient makeToneFilter(const float frequency, double sampleRate);

static inline float distort(float v) 
{
    float abs_v = std::abs(v);
    return v / (1.0f + abs_v);
}

inline auto makeLowCutCoefficient(const ChainSettings& chain_settings, double sampleRate)
{
    return juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(
        chain_settings.lowCutFreq,
        sampleRate,
        (1 + chain_settings.lowCutSlope) * 2);
}
inline auto makeHighCutCoefficient(const ChainSettings& chain_settings, double sampleRate)
{
    return juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(
        chain_settings.highCutFreq,
        sampleRate,
        (1 + chain_settings.highCutSlope) * 2);
}
//==============================================================================
// Template Function. To make and update filters

// Enable one filter in the filter chain
template <int Index, typename ChainType, typename CoefficientType>
void EnableFilterInCutChain(ChainType& cutfilter, const CoefficientType& coefficient)
{
    UpdateCoefficients(cutfilter.template get<Index>().coefficients, coefficient[Index]);
    cutfilter.template setBypassed<Index>(false);
}

template <typename ChainType, typename CoefficientType>
void UpdateCutFilter(ChainType& cutfilter, const CoefficientType& coefficient,
    const int& slope)
{
    cutfilter.template setBypassed<0>(true);
    cutfilter.template setBypassed<1>(true);
    cutfilter.template setBypassed<2>(true);
    cutfilter.template setBypassed<3>(true);

    // no break here since we need to update all 4 filters in the filter chain
    switch (slope)
    {
    case slope_48:
        EnableFilterInCutChain<3>(cutfilter, coefficient);
    case slope_36:
        EnableFilterInCutChain<2>(cutfilter, coefficient);
    case slope_24:
        EnableFilterInCutChain<1>(cutfilter, coefficient);
    case slope_12:
        EnableFilterInCutChain<0>(cutfilter, coefficient);
        break;
    default:
        break;

    }
}

//==============================================================================
/**
*/
class Mixer561AudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    Mixer561AudioProcessor();
    ~Mixer561AudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout(); // Create apvts, for saving our filter parameters
    juce::AudioProcessorValueTreeState apvts{*this, nullptr, "AudioParameter", createParameterLayout()};
    //==============================================================================
    void SetToneFilterBypass(bool bypassed);
    float toneRMS[48];

private:
    //==============================================================================
    MonoChain leftChain, rightChain;
    void UpdatePeakFilter(const ChainSettings& chain_settings);
    void UpdateFilters();
    void UpdateLowCutFilters(ChainSettings& chain_settings);
    void UpdateHighCutFilters(ChainSettings& chain_settings);

    //=====================Tone Filter==============================================
    std::unique_ptr<Filter> ToneFilterArray[48]; // We average all the inputs to mono so only one group is needed here
    std::unique_ptr<juce::AudioBuffer<float>> ToneBuffers[48];
    std::unique_ptr<juce::AudioBuffer<float>> SideTrackBuffer; // Save the final mix result of this frame
    std::unique_ptr<juce::AudioBuffer<float>> TempTrackBuffer; // Used to save the original track averaged to 1 channel temporarily

    float runningPower[48]; // Collect energy distribution
    float running_total_power; // Current running power of the filters plus the original track (pre-attenuation)
    float tone_power_lerp = 0.00012f; // Will be adjusted later based on sample rate
    float total_tone_power_lerp = 0.0001f;
    bool tone_bypassed = false;

    // void ToneProcess(juce::AudioBuffer<float>& sideTrack, juce::AudioBlock<float>& lBuffer, juce::AudioBlock<float>& rBuffer);
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Mixer561AudioProcessor)
};
