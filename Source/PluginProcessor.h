#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_audio_utils/juce_audio_utils.h>

enum class FilterType
{
    LowPass,
    BandPass,
    HighPass
};

enum class LfoShape
{
    Sine,
    Triangle,
    Saw,
    Square,
    Random
};

enum class TrasherMode
{
    Fuzz,
    Scream
};

enum class OversamplingFactor
{
    None = 1,
    X2 = 2,
    X4 = 4,
    X8 = 8
};

class KinaVSTProcessor : public juce::AudioProcessor
{
public:
    KinaVSTProcessor();
    ~KinaVSTProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void reset() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Audio Parameters
    juce::AudioProcessorValueTreeState parameters;
    
    // Parameter IDs
    static const juce::String VCA_LFO_RATE_ID;
    static const juce::String VCA_LFO_AMOUNT_ID;
    static const juce::String VCA_LFO_SYNC_ID;
    static const juce::String VCA_LFO_SHAPE_ID;
    static const juce::String VCA_AMOUNT_ID;
    
    static const juce::String VCF_TYPE_ID;
    static const juce::String VCF_CUTOFF_ID;
    static const juce::String VCF_RESONANCE_ID;
    static const juce::String VCF_LFO_RATE_ID;
    static const juce::String VCF_LFO_AMOUNT_ID;
    static const juce::String VCF_LFO_SYNC_ID;
    
    static const juce::String TRASHER1_MODE_ID;
    static const juce::String TRASHER1_AMOUNT_ID;
    static const juce::String TRASHER1_TONE_ID;
    
    static const juce::String TRASHER2_MODE_ID;
    static const juce::String TRASHER2_AMOUNT_ID;
    static const juce::String TRASHER2_TONE_ID;
    
    static const juce::String ECHO_TIME_ID;
    static const juce::String ECHO_FEEDBACK_ID;
    static const juce::String ECHO_AMOUNT_ID;
    static const juce::String ECHO_SYNC_ID;
    
    static const juce::String REVERB_SIZE_ID;
    static const juce::String REVERB_DAMPING_ID;
    static const juce::String REVERB_WIDTH_ID;
    static const juce::String REVERB_AMOUNT_ID;
    
    static const juce::String DRY_WET_ID;
    static const juce::String OVERSAMPLING_ID;

    void randomizeParameters();
    
private:
    std::unique_ptr<juce::dsp::Oscillator<float>> vcaLfo;
    std::unique_ptr<juce::dsp::Oscillator<float>> vcfLfo;
    juce::dsp::Gain<float> vca;
    
    juce::dsp::StateVariableTPTFilter<float> vcf;
    
    juce::dsp::WaveShaper<float> trasher1;
    juce::dsp::WaveShaper<float> trasher2;
    
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> echo { 192000 };
    juce::Reverb reverb;
    juce::Reverb::Parameters reverbParameters;
    
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling;
    
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
    
    juce::Random random;
    juce::CriticalSection lock;

    // Add smoothed parameters for echo
    juce::SmoothedValue<float> smoothedEchoTime;
    juce::SmoothedValue<float> smoothedEchoFeedback;
    juce::SmoothedValue<float> smoothedEchoAmount;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void processBlockInternal(juce::dsp::AudioBlock<float>& block, const juce::Optional<juce::AudioPlayHead::PositionInfo>& posInfo);
    void updateOversamplingSettings();
    void updateLfoWaveforms();
    float processDistortion(float sample, float amount, float tone, TrasherMode mode);
    float getLfoValue(juce::dsp::Oscillator<float>& lfo, bool sync, float rate, const juce::Optional<juce::AudioPlayHead::PositionInfo>& posInfo);
    void initializeOversampling(int samplesPerBlock);
    void setupWaveShapers();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KinaVSTProcessor)
}; 