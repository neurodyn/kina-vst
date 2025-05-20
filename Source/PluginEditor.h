#pragma once

#include "../JUCE/modules/juce_audio_processors/juce_audio_processors.h"
#include "../JUCE/modules/juce_gui_basics/juce_gui_basics.h"
#include "PluginProcessor.h"

class KinaVSTEditor : public juce::AudioProcessorEditor
{
public:
    explicit KinaVSTEditor(KinaVSTProcessor&);
    ~KinaVSTEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    KinaVSTProcessor& processor;
    
    // Module panels
    juce::GroupComponent vcaGroup, vcfGroup, trasher1Group, trasher2Group, echoGroup, reverbGroup, globalGroup;
    
    // VCA controls
    juce::Slider vcaLfoRateSlider, vcaLfoAmountSlider, vcaAmountSlider;
    juce::ComboBox vcaLfoShapeBox;
    juce::ToggleButton vcaLfoSyncButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vcaLfoRateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vcaLfoAmountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vcaAmountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> vcaLfoShapeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> vcaLfoSyncAttachment;
    
    // VCF controls
    juce::Slider vcfCutoffSlider, vcfResonanceSlider, vcfLfoRateSlider, vcfLfoAmountSlider;
    juce::ComboBox vcfTypeBox;
    juce::ToggleButton vcfLfoSyncButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vcfCutoffAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vcfResonanceAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vcfLfoRateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vcfLfoAmountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> vcfTypeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> vcfLfoSyncAttachment;
    
    // Trasher 1 controls
    juce::ComboBox trasher1ModeBox;
    juce::Slider trasher1AmountSlider, trasher1ToneSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> trasher1ModeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> trasher1AmountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> trasher1ToneAttachment;
    
    // Trasher 2 controls
    juce::ComboBox trasher2ModeBox;
    juce::Slider trasher2AmountSlider, trasher2ToneSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> trasher2ModeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> trasher2AmountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> trasher2ToneAttachment;
    
    // Echo controls
    juce::Slider echoTimeSlider, echoFeedbackSlider, echoAmountSlider;
    juce::ToggleButton echoSyncButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> echoTimeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> echoFeedbackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> echoAmountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> echoSyncAttachment;
    
    // Reverb controls
    juce::Slider reverbSizeSlider, reverbDampingSlider, reverbWidthSlider, reverbAmountSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbSizeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbDampingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbWidthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbAmountAttachment;
    
    // Global controls
    juce::Slider dryWetSlider;
    juce::ComboBox oversamplingBox;
    juce::TextButton randomizeButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dryWetAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> oversamplingAttachment;
    
    void setupSlider(juce::Slider& slider, const juce::String& suffix = "");
    void setupRotarySlider(juce::Slider& slider, const juce::String& suffix = "");
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KinaVSTEditor)
}; 