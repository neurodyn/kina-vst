#include "PluginProcessor.h"

// Parameter IDs
const juce::String KinaVSTProcessor::VCA_LFO_RATE_ID = "vca_lfo_rate";
const juce::String KinaVSTProcessor::VCA_LFO_AMOUNT_ID = "vca_lfo_amount";
const juce::String KinaVSTProcessor::VCA_LFO_SYNC_ID = "vca_lfo_sync";
const juce::String KinaVSTProcessor::VCA_LFO_SHAPE_ID = "vca_lfo_shape";
const juce::String KinaVSTProcessor::VCA_AMOUNT_ID = "vca_amount";

const juce::String KinaVSTProcessor::VCF_TYPE_ID = "vcf_type";
const juce::String KinaVSTProcessor::VCF_CUTOFF_ID = "vcf_cutoff";
const juce::String KinaVSTProcessor::VCF_RESONANCE_ID = "vcf_resonance";
const juce::String KinaVSTProcessor::VCF_LFO_RATE_ID = "vcf_lfo_rate";
const juce::String KinaVSTProcessor::VCF_LFO_AMOUNT_ID = "vcf_lfo_amount";
const juce::String KinaVSTProcessor::VCF_LFO_SYNC_ID = "vcf_lfo_sync";

const juce::String KinaVSTProcessor::TRASHER1_MODE_ID = "trasher1_mode";
const juce::String KinaVSTProcessor::TRASHER1_AMOUNT_ID = "trasher1_amount";
const juce::String KinaVSTProcessor::TRASHER1_TONE_ID = "trasher1_tone";

const juce::String KinaVSTProcessor::TRASHER2_MODE_ID = "trasher2_mode";
const juce::String KinaVSTProcessor::TRASHER2_AMOUNT_ID = "trasher2_amount";
const juce::String KinaVSTProcessor::TRASHER2_TONE_ID = "trasher2_tone";

const juce::String KinaVSTProcessor::ECHO_TIME_ID = "echo_time";
const juce::String KinaVSTProcessor::ECHO_FEEDBACK_ID = "echo_feedback";
const juce::String KinaVSTProcessor::ECHO_AMOUNT_ID = "echo_amount";
const juce::String KinaVSTProcessor::ECHO_SYNC_ID = "echo_sync";

const juce::String KinaVSTProcessor::REVERB_SIZE_ID = "reverb_size";
const juce::String KinaVSTProcessor::REVERB_DAMPING_ID = "reverb_damping";
const juce::String KinaVSTProcessor::REVERB_WIDTH_ID = "reverb_width";
const juce::String KinaVSTProcessor::REVERB_AMOUNT_ID = "reverb_amount";

const juce::String KinaVSTProcessor::DRY_WET_ID = "dry_wet";
const juce::String KinaVSTProcessor::OVERSAMPLING_ID = "oversampling";

KinaVSTProcessor::KinaVSTProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                    .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "Parameters", createParameterLayout())
{
    // Initialize all member variables to safe defaults
    currentSampleRate = 44100.0;
    currentBlockSize = 512;

    // Initialize all pointers to nullptr first
    vcaLfo = nullptr;
    vcfLfo = nullptr;
    oversampling = nullptr;

    try {
        // Initialize oscillators with basic sine wave
        vcaLfo = std::make_unique<juce::dsp::Oscillator<float>>();
        vcfLfo = std::make_unique<juce::dsp::Oscillator<float>>();
        
        if (vcaLfo && vcfLfo) {
            vcaLfo->initialise([](float x) { return std::sin(x); });
            vcfLfo->initialise([](float x) { return std::sin(x); });
        }

        // Set up basic processing specs
        juce::dsp::ProcessSpec spec{
            currentSampleRate,
            static_cast<juce::uint32>(currentBlockSize),
            static_cast<juce::uint32>(2) // Stereo
        };

        // Initialize VCA
        vca.reset();
        vca.prepare(spec);

        // Initialize VCF
        vcf.reset();
        vcf.prepare(spec);
        vcf.setType(juce::dsp::StateVariableTPTFilter<float>::Type::lowpass);

        // Initialize Trashers
        trasher1.reset();
        trasher2.reset();
        trasher1.prepare(spec);
        trasher2.prepare(spec);

        // Initialize Echo
        echo.reset();
        echo.prepare(spec);
        echo.setMaximumDelayInSamples(static_cast<int>(currentSampleRate * 4.0)); // 4 seconds maximum delay

        // Initialize Reverb
        reverb.reset();
        reverb.setSampleRate(currentSampleRate);
    }
    catch (const std::exception&) {
        // If initialization fails, ensure everything is in a safe state
        vcaLfo.reset();
        vcfLfo.reset();
        oversampling.reset();
        vca.reset();
        vcf.reset();
        trasher1.reset();
        trasher2.reset();
        echo.reset();
        reverb.reset();
    }

    // Add to class members
    smoothedEchoTime.reset(currentSampleRate, 0.05);
    smoothedEchoFeedback.reset(currentSampleRate, 0.05);
    smoothedEchoAmount.reset(currentSampleRate, 0.05);
}

KinaVSTProcessor::~KinaVSTProcessor()
{
    // Ensure clean shutdown
    const juce::ScopedLock sl(lock);
    
    // Reset and release all resources
    if (oversampling != nullptr)
        oversampling->reset();
    
    vcaLfo.reset();
    vcfLfo.reset();
    oversampling.reset();
    vca.reset();
    vcf.reset();
    trasher1.reset();
    trasher2.reset();
    echo.reset();
    reverb.reset();
}

juce::AudioProcessorValueTreeState::ParameterLayout KinaVSTProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    // VCA LFO parameters (up to 10kHz)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(VCA_LFO_RATE_ID, "VCA LFO Rate", 
        juce::NormalisableRange<float>(0.01f, 10000.0f, 0.01f, 0.3f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(VCA_LFO_AMOUNT_ID, "VCA LFO Amount", 
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(VCA_LFO_SYNC_ID, "VCA LFO Sync", false));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(VCA_LFO_SHAPE_ID, "VCA LFO Shape",
        juce::StringArray("Sine", "Triangle", "Saw", "Square", "Random"), 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(VCA_AMOUNT_ID, "VCA Amount", 
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
    
    // VCF parameters (LFO up to 1kHz)
    params.push_back(std::make_unique<juce::AudioParameterChoice>(VCF_TYPE_ID, "Filter Type",
        juce::StringArray("Low Pass", "Band Pass", "High Pass"), 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(VCF_CUTOFF_ID, "Filter Cutoff", 
        juce::NormalisableRange<float>(20.0f, 20000.0f, 0.1f, 0.3f), 1000.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(VCF_RESONANCE_ID, "Filter Resonance", 
        juce::NormalisableRange<float>(0.1f, 8.0f, 0.01f, 0.5f), 0.707f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(VCF_LFO_RATE_ID, "VCF LFO Rate", 
        juce::NormalisableRange<float>(0.01f, 1000.0f, 0.01f, 0.3f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(VCF_LFO_AMOUNT_ID, "VCF LFO Amount", 
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(VCF_LFO_SYNC_ID, "VCF LFO Sync", false));
    
    // Trasher parameters
    params.push_back(std::make_unique<juce::AudioParameterChoice>(TRASHER1_MODE_ID, "Trasher 1 Mode",
        juce::StringArray("Fuzz", "Scream"), 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(TRASHER1_AMOUNT_ID, "Trasher 1 Amount", 
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f, 0.5f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(TRASHER1_TONE_ID, "Trasher 1 Tone", 
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
    
    params.push_back(std::make_unique<juce::AudioParameterChoice>(TRASHER2_MODE_ID, "Trasher 2 Mode",
        juce::StringArray("Fuzz", "Scream"), 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(TRASHER2_AMOUNT_ID, "Trasher 2 Amount", 
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f, 0.5f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(TRASHER2_TONE_ID, "Trasher 2 Tone", 
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
    
    // Echo parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ECHO_TIME_ID, "Echo Time", 
        juce::NormalisableRange<float>(0.01f, 2.0f, 0.01f, 0.3f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ECHO_FEEDBACK_ID, "Echo Feedback", 
        juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ECHO_AMOUNT_ID, "Echo Amount", 
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.3f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(ECHO_SYNC_ID, "Echo Sync", false));
    
    // Reverb parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(REVERB_SIZE_ID, "Room Size", 
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(REVERB_DAMPING_ID, "Damping", 
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(REVERB_WIDTH_ID, "Width", 
        juce::NormalisableRange<float>(0.0f, 1.0f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(REVERB_AMOUNT_ID, "Reverb Amount", 
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.3f));
    
    // Global parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(DRY_WET_ID, "Dry/Wet", 
        juce::NormalisableRange<float>(0.0f, 1.0f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(OVERSAMPLING_ID, "Oversampling",
        juce::StringArray("Off", "2x", "4x", "8x"), 0));
    
    return { params.begin(), params.end() };
}

void KinaVSTProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Guard against invalid parameters
    if (sampleRate <= 0 || samplesPerBlock <= 0)
        return;

    const juce::ScopedLock sl(lock);
    
    try {
        // Update basic parameters
        currentSampleRate = sampleRate;
        currentBlockSize = samplesPerBlock;

        // Create processing spec
        juce::dsp::ProcessSpec spec{
            sampleRate,
            static_cast<juce::uint32>(samplesPerBlock),
            static_cast<juce::uint32>(getTotalNumOutputChannels())
        };

        // Reset and prepare all processors
        if (!vcaLfo || !vcfLfo) {
            vcaLfo = std::make_unique<juce::dsp::Oscillator<float>>();
            vcfLfo = std::make_unique<juce::dsp::Oscillator<float>>();
            vcaLfo->initialise([](float x) { return std::sin(x); });
            vcfLfo->initialise([](float x) { return std::sin(x); });
        }

        // Prepare oscillators
        vcaLfo->reset();
        vcfLfo->reset();
        vcaLfo->prepare(spec);
        vcfLfo->prepare(spec);
        updateLfoWaveforms();

        // Prepare VCA
        vca.reset();
        vca.prepare(spec);

        // Prepare VCF
        vcf.reset();
        vcf.prepare(spec);

        // Prepare Trashers
        trasher1.reset();
        trasher2.reset();
        trasher1.prepare(spec);
        trasher2.prepare(spec);
        setupWaveShapers();
        
        // Prepare Echo
        echo.reset();
        echo.prepare(spec);
        const auto maxDelaySamples = static_cast<int>(sampleRate * 4.0);
        if (maxDelaySamples > 0) {
            echo.setMaximumDelayInSamples(maxDelaySamples);
        }

        // Prepare Reverb
        reverb.reset();
        reverb.setSampleRate(sampleRate);

        // Initialize oversampling last
        initializeOversampling(samplesPerBlock);

        // Reset smoothed parameters
        smoothedEchoTime.reset(currentSampleRate, 0.05);
        smoothedEchoFeedback.reset(currentSampleRate, 0.05);
        smoothedEchoAmount.reset(currentSampleRate, 0.05);
    }
    catch (const std::exception&) {
        // If preparation fails, reset everything to a safe state
        reset();
    }
}

void KinaVSTProcessor::releaseResources()
{
    const juce::ScopedLock sl(lock);
    
    // Reset all processors
    if (oversampling != nullptr)
        oversampling->reset();
        
    vcaLfo.reset();
    vcfLfo.reset();
    vca.reset();
    vcf.reset();
    trasher1.reset();
    trasher2.reset();
    echo.reset();
    reverb.reset();
}

void KinaVSTProcessor::reset()
{
    const juce::ScopedLock sl(lock);
    
    if (vcaLfo) vcaLfo->reset();
    if (vcfLfo) vcfLfo->reset();
    vca.reset();
    vcf.reset();
    trasher1.reset();
    trasher2.reset();
    echo.reset();
    reverb.reset();
    if (oversampling) oversampling->reset();
}

void KinaVSTProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;
    const juce::ScopedLock sl(lock);
    
    // Safety checks
    if (!vcaLfo || !vcfLfo || buffer.getNumChannels() <= 0 || buffer.getNumSamples() <= 0) {
        buffer.clear();
        return;
    }
    
    try {
        // Get current playhead info for sync features
        auto playHead = getPlayHead();
        juce::Optional<juce::AudioPlayHead::PositionInfo> posInfo;
        if (playHead != nullptr) {
            posInfo = playHead->getPosition();
        }
        
        // Create audio block
        juce::dsp::AudioBlock<float> block(buffer);
        
        // Process with oversampling if enabled and properly initialized
        auto* oversamplingParam = dynamic_cast<juce::AudioParameterChoice*>(parameters.getParameter(OVERSAMPLING_ID));
        if (oversamplingParam != nullptr && oversamplingParam->getIndex() > 0 && oversampling != nullptr) {
            auto oversampledBlock = oversampling->processSamplesUp(block);
            processBlockInternal(oversampledBlock, posInfo);
            oversampling->processSamplesDown(block);
        }
        else {
            processBlockInternal(block, posInfo);
        }
    }
    catch (const std::exception&) {
        // If processing fails, output silence
        buffer.clear();
    }
}

void KinaVSTProcessor::processBlockInternal(juce::dsp::AudioBlock<float>& block,
    const juce::Optional<juce::AudioPlayHead::PositionInfo>& posInfo)
{
    const auto numChannels = block.getNumChannels();
    const auto numSamples = block.getNumSamples();
    
    // Create a dry copy
    juce::AudioBuffer<float> dryBuffer(static_cast<int>(numChannels), static_cast<int>(numSamples));
    for (size_t channel = 0; channel < numChannels; ++channel)
    {
        dryBuffer.copyFrom(static_cast<int>(channel), 0, block.getChannelPointer(channel), 
            static_cast<int>(numSamples));
    }
    
    // Get parameter values (thread-safe)
    const float vcaLfoRate = parameters.getRawParameterValue(VCA_LFO_RATE_ID)->load();
    const float vcaLfoAmount = parameters.getRawParameterValue(VCA_LFO_AMOUNT_ID)->load();
    const bool vcaLfoSync = static_cast<bool>(parameters.getRawParameterValue(VCA_LFO_SYNC_ID)->load());
    const float vcaAmount = parameters.getRawParameterValue(VCA_AMOUNT_ID)->load();
    
    const float vcfCutoff = parameters.getRawParameterValue(VCF_CUTOFF_ID)->load();
    const float vcfResonance = parameters.getRawParameterValue(VCF_RESONANCE_ID)->load();
    const float vcfLfoRate = parameters.getRawParameterValue(VCF_LFO_RATE_ID)->load();
    const float vcfLfoAmount = parameters.getRawParameterValue(VCF_LFO_AMOUNT_ID)->load();
    const bool vcfLfoSync = static_cast<bool>(parameters.getRawParameterValue(VCF_LFO_SYNC_ID)->load());
    
    // Get filter type
    const auto filterType = static_cast<FilterType>(
        static_cast<int>(parameters.getRawParameterValue(VCF_TYPE_ID)->load()));
    
    // Set filter type
    switch (filterType)
    {
        case FilterType::LowPass:
            vcf.setType(juce::dsp::StateVariableTPTFilter<float>::Type::lowpass);
            break;
        case FilterType::BandPass:
            vcf.setType(juce::dsp::StateVariableTPTFilter<float>::Type::bandpass);
            break;
        case FilterType::HighPass:
            vcf.setType(juce::dsp::StateVariableTPTFilter<float>::Type::highpass);
            break;
    }
    
    // Process each sample
    for (size_t channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = block.getChannelPointer(channel);
        
        for (size_t sample = 0; sample < numSamples; ++sample)
        {
            // Apply VCA modulation with improved scaling
            float vcaModulation = getLfoValue(*vcaLfo, vcaLfoSync, vcaLfoRate, posInfo);
            // Scale modulation to 0.5 to 2.0 range instead of 0.0 to 1.0
            float vcaGain = juce::jmap(vcaModulation * vcaLfoAmount + (1.0f - vcaLfoAmount), 0.5f, 2.0f);
            channelData[sample] *= vcaGain * juce::jlimit(0.0f, 1.0f, vcaAmount);
            
            // Add protection against extreme values
            channelData[sample] = juce::jlimit(-1.0f, 1.0f, channelData[sample]);
            
            // Apply VCF modulation
            float vcfModulation = getLfoValue(*vcfLfo, vcfLfoSync, vcfLfoRate, posInfo);

            // Map LFO from [-1,1] to [1/factor, factor] where factor depends on amount
            float modulationFactor = std::pow(2.0f, vcfLfoAmount * 4.0f); // 4.0f gives us 4 octaves range at amount=1.0
            float frequencyMultiplier = std::exp2(vcfModulation * std::log2(modulationFactor));

            // Apply the modulation multiplicatively to preserve musical frequency ratios
            float modCutoff = vcfCutoff * frequencyMultiplier;

            // Ensure we stay within safe frequency bounds
            modCutoff = juce::jlimit(20.0f, 20000.0f, modCutoff);
            vcf.setCutoffFrequency(modCutoff);
            vcf.setResonance(vcfResonance);
            channelData[sample] = vcf.processSample(static_cast<int>(channel), channelData[sample]);
            
            // Apply Trasher 1
            const float trasher1Amount = parameters.getRawParameterValue(TRASHER1_AMOUNT_ID)->load();
            const float trasher1Tone = parameters.getRawParameterValue(TRASHER1_TONE_ID)->load();
            const auto trasher1Mode = static_cast<TrasherMode>(static_cast<int>(parameters.getRawParameterValue(TRASHER1_MODE_ID)->load()));
            channelData[sample] = processDistortion(channelData[sample], trasher1Amount, trasher1Tone, trasher1Mode);
            
            // Apply Trasher 2
            const float trasher2Amount = parameters.getRawParameterValue(TRASHER2_AMOUNT_ID)->load();
            const float trasher2Tone = parameters.getRawParameterValue(TRASHER2_TONE_ID)->load();
            const auto trasher2Mode = static_cast<TrasherMode>(static_cast<int>(parameters.getRawParameterValue(TRASHER2_MODE_ID)->load()));
            channelData[sample] = processDistortion(channelData[sample], trasher2Amount, trasher2Tone, trasher2Mode);
            
            // Apply Echo
            const float echoTime = parameters.getRawParameterValue(ECHO_TIME_ID)->load();
            const float echoFeedback = parameters.getRawParameterValue(ECHO_FEEDBACK_ID)->load();
            const float echoAmount = parameters.getRawParameterValue(ECHO_AMOUNT_ID)->load();
            const bool echoSync = static_cast<bool>(parameters.getRawParameterValue(ECHO_SYNC_ID)->load());
            
            // Update smoothed values
            smoothedEchoTime.setTargetValue(echoTime);
            smoothedEchoFeedback.setTargetValue(echoFeedback);
            smoothedEchoAmount.setTargetValue(echoAmount);
            
            float delayInSamples;
            if (echoSync && posInfo && posInfo->getBpm().hasValue())
            {
                const double samplesPerBeat = (60.0 / *posInfo->getBpm()) * currentSampleRate;
                // Map time parameter to musical divisions (e.g., 1/4, 1/8, 1/16 notes)
                const float beatDivisions[] = { 0.25f, 0.375f, 0.5f, 0.75f, 1.0f, 1.5f, 2.0f };
                const float mappedTime = juce::jmap(smoothedEchoTime.getNextValue(), 0.01f, 2.0f, 
                                                  beatDivisions[0], beatDivisions[std::size(beatDivisions)-1]);
                delayInSamples = static_cast<float>(samplesPerBeat * mappedTime);
            }
            else
            {
                delayInSamples = static_cast<float>(currentSampleRate * smoothedEchoTime.getNextValue());
            }
            
            // Ensure delay time is within bounds
            delayInSamples = juce::jlimit(1.0f, static_cast<float>(echo.getMaximumDelayInSamples()), delayInSamples);
            echo.setDelay(delayInSamples);
            
            // Get the delayed sample
            const float delayedSample = echo.popSample(static_cast<int>(channel));
            
            // Calculate the feedback input with smoothed feedback
            const float feedbackInput = channelData[sample] + (delayedSample * smoothedEchoFeedback.getNextValue());
            
            // Push the feedback signal into the delay line
            echo.pushSample(static_cast<int>(channel), feedbackInput);
            
            // Mix the original signal with the delayed signal (don't replace it)
            const float dry = channelData[sample];
            const float wet = delayedSample;
            channelData[sample] = dry + (wet * smoothedEchoAmount.getNextValue());
        }
    }
    
    // Apply Reverb
    juce::Reverb::Parameters params;
    params.roomSize = parameters.getRawParameterValue(REVERB_SIZE_ID)->load();
    params.damping = parameters.getRawParameterValue(REVERB_DAMPING_ID)->load();
    params.width = parameters.getRawParameterValue(REVERB_WIDTH_ID)->load();
    params.wetLevel = parameters.getRawParameterValue(REVERB_AMOUNT_ID)->load();
    params.dryLevel = 1.0f - params.wetLevel;
    reverb.setParameters(params);
    
    // Process reverb
    if (numChannels > 1)
    {
        reverb.processStereo(block.getChannelPointer(0), block.getChannelPointer(1), 
            static_cast<int>(numSamples));
    }
    else
    {
        reverb.processMono(block.getChannelPointer(0), static_cast<int>(numSamples));
    }
    
    // Mix dry/wet
    const float dryWet = parameters.getRawParameterValue(DRY_WET_ID)->load();
    for (size_t channel = 0; channel < numChannels; ++channel)
    {
        auto* wetData = block.getChannelPointer(channel);
        const auto* dryData = dryBuffer.getReadPointer(static_cast<int>(channel));
        
        for (size_t sample = 0; sample < numSamples; ++sample)
        {
            wetData[sample] = dryData[sample] * (1.0f - dryWet) + wetData[sample] * dryWet;
        }
    }
    
    // Update LFO waveforms if changed
    updateLfoWaveforms();
}

float KinaVSTProcessor::getLfoValue(juce::dsp::Oscillator<float>& lfo, bool sync, float rate,
    const juce::Optional<juce::AudioPlayHead::PositionInfo>& posInfo)
{
    if (sync && posInfo && posInfo->getBpm().hasValue())
    {
        const double samplesPerBeat = (60.0 / *posInfo->getBpm()) * currentSampleRate;
        lfo.setFrequency(static_cast<float>(1.0 / (samplesPerBeat / currentSampleRate)));
    }
    else
    {
        lfo.setFrequency(rate);
    }
    
    return lfo.processSample(0.0f);
}

float KinaVSTProcessor::processDistortion(float sample, float amount, float tone, TrasherMode mode)
{
    if (amount <= 0.0f)
        return sample;
        
    float processed;
    /*

    switch (mode)
    {
        case TrasherMode::Fuzz:
            processed = std::tanh(sample * (1.0f + 40.0f * amount));
            break;
            
        case TrasherMode::Scream:
            processed = (sample >= 0.0f) ? 1.0f - std::exp(-sample * amount * 3.0f)
                                       : -1.0f + std::exp(sample * amount * 3.0f);
            break;
    }
    */
    // Configure the waveshaping function based on the mode
     // Use the already configured waveshapers
    switch (mode)
    {
        case TrasherMode::Fuzz:
            processed = trasher1.processSample(sample * (1.0f + 40.0f * amount));
            break;
            
        case TrasherMode::Scream:
            processed = trasher2.processSample(sample * amount * 3.0f);
            break;
    }
    
    return processed * (1.0f - tone) + sample * tone;
}

void KinaVSTProcessor::updateOversamplingSettings()
{
    auto* param = dynamic_cast<juce::AudioParameterChoice*>(parameters.getParameter(OVERSAMPLING_ID));
    if (param != nullptr)
    {
        // First, reset the existing oversampling
        oversampling.reset();
        
        int factor = 1 << param->getIndex(); // 2^index: 1, 2, 4, 8
        if (factor > 1)
        {
            try
            {
                // Create new oversampling object with new factor
                auto newOversampling = std::make_unique<juce::dsp::Oversampling<float>>(
                    2, // num channels
                    param->getIndex(), // order: 1=2x, 2=4x, 3=8x
                    juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
                    true, // true = better quality
                    false // don't process the extra channels in parallel
                );
                
                // Initialize the new oversampling object
                newOversampling->initProcessing(static_cast<size_t>(currentBlockSize));
                
                // Replace the old oversampling object
                oversampling = std::move(newOversampling);
            }
            catch (const std::exception& e)
            {
                // If initialization fails, reset oversampling and parameter
                oversampling.reset();
                param->setValueNotifyingHost(0.0f); // Set back to "Off"
            }
        }
    }
}

void KinaVSTProcessor::randomizeParameters()
{
    for (auto* param : getParameters())
    {
        if (param->getName(32) != "Dry/Wet") // Don't randomize dry/wet
        {
            if (auto* rangedParam = dynamic_cast<juce::RangedAudioParameter*>(param))
            {
                float randomValue = random.nextFloat();
                rangedParam->setValueNotifyingHost(randomValue);
            }
        }
    }
}

void KinaVSTProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void KinaVSTProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorEditor* KinaVSTProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
}

void KinaVSTProcessor::updateLfoWaveforms()
{
    if (auto* vcaLfoShapeParam = dynamic_cast<juce::AudioParameterChoice*>(parameters.getParameter(VCA_LFO_SHAPE_ID)))
    {
        switch (vcaLfoShapeParam->getIndex())
        {
            case 0: // Sine
                vcaLfo->initialise([](float x) { return std::sin(x); });
                break;
            case 1: // Triangle
                vcaLfo->initialise([](float x) 
                { 
                    x = std::fmod(x + juce::MathConstants<float>::pi, 2.0f * juce::MathConstants<float>::pi);
                    return 2.0f * std::abs(x / juce::MathConstants<float>::pi - 1.0f) - 1.0f;
                });
                break;
            case 2: // Saw
                vcaLfo->initialise([](float x)
                {
                    x = std::fmod(x + juce::MathConstants<float>::pi, 2.0f * juce::MathConstants<float>::pi);
                    return x / juce::MathConstants<float>::pi - 1.0f;
                });
                break;
            case 3: // Square
                vcaLfo->initialise([](float x) { return std::sin(x) >= 0.0f ? 1.0f : -1.0f; });
                break;
            case 4: // Random
                vcaLfo->initialise([this](float x)
                {
                    static float lastValue = 0.0f;
                    static float phase = 0.0f;
                    
                    x = std::fmod(x, 2.0f * juce::MathConstants<float>::pi);
                    if (x < phase)
                    {
                        lastValue = random.nextFloat() * 2.0f - 1.0f;
                    }
                    phase = x;
                    return lastValue;
                });
                break;
        }
    }
}

void KinaVSTProcessor::initializeOversampling(int samplesPerBlock)
{
    auto* oversamplingParam = dynamic_cast<juce::AudioParameterChoice*>(parameters.getParameter(OVERSAMPLING_ID));
    if (oversamplingParam != nullptr) {
        // Don't reset to "Off" - let the parameter keep its value
        oversampling.reset();
        
        try {
            const int factor = 1 << oversamplingParam->getIndex();
            if (factor > 1) {
                oversampling = std::make_unique<juce::dsp::Oversampling<float>>(
                    getTotalNumOutputChannels(),
                    oversamplingParam->getIndex(),
                    juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
                    true,  // Use maximum quality
                    true   // Use integer latency compensation
                );
                
                if (oversampling) {
                    oversampling->initProcessing(static_cast<size_t>(samplesPerBlock));
                }
            }
        }
        catch (const std::exception&) {
            oversampling.reset();
            oversamplingParam->setValueNotifyingHost(0.0f);
        }
    }
}

void KinaVSTProcessor::setupWaveShapers()
{
    // Set up Fuzz waveshaper
    trasher1.functionToUse = [](float x) {
        return std::tanh(x);
    };
    
    // Set up Scream waveshaper
    trasher2.functionToUse = [](float x) {
        return (x >= 0.0f) ? 1.0f - std::exp(-x)
                          : -1.0f + std::exp(x);
    };
}
//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KinaVSTProcessor();
} 