#include "PluginProcessor.h"
#include "PluginEditor.h"

KinaVSTEditor::KinaVSTEditor(KinaVSTProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    // Set up module panels
    addAndMakeVisible(vcaGroup);
    vcaGroup.setText("VCA");
    addAndMakeVisible(vcfGroup);
    vcfGroup.setText("VCF");
    addAndMakeVisible(trasher1Group);
    trasher1Group.setText("Trasher 1");
    addAndMakeVisible(trasher2Group);
    trasher2Group.setText("Trasher 2");
    addAndMakeVisible(echoGroup);
    echoGroup.setText("Echo");
    addAndMakeVisible(reverbGroup);
    reverbGroup.setText("Reverb");
    addAndMakeVisible(globalGroup);
    globalGroup.setText("Global");

    // Set up VCA controls
    setupRotarySlider(vcaLfoRateSlider, "Hz");
    setupRotarySlider(vcaLfoAmountSlider, "%");
    setupRotarySlider(vcaAmountSlider, "%");
    vcaLfoShapeBox.addItemList({"Sine", "Triangle", "Saw", "Square", "Random"}, 1);
    vcaLfoSyncButton.setButtonText("Sync to BPM");
    addAndMakeVisible(vcaLfoRateSlider);
    addAndMakeVisible(vcaLfoAmountSlider);
    addAndMakeVisible(vcaAmountSlider);
    addAndMakeVisible(vcaLfoShapeBox);
    addAndMakeVisible(vcaLfoSyncButton);

    // Set up VCF controls
    setupRotarySlider(vcfCutoffSlider, "Hz");
    setupRotarySlider(vcfResonanceSlider);
    setupRotarySlider(vcfLfoRateSlider, "Hz");
    setupRotarySlider(vcfLfoAmountSlider, "%");
    vcfTypeBox.addItemList({"Low Pass", "Band Pass", "High Pass"}, 1);
    vcfLfoSyncButton.setButtonText("Sync to BPM");
    addAndMakeVisible(vcfCutoffSlider);
    addAndMakeVisible(vcfResonanceSlider);
    addAndMakeVisible(vcfLfoRateSlider);
    addAndMakeVisible(vcfLfoAmountSlider);
    addAndMakeVisible(vcfTypeBox);
    addAndMakeVisible(vcfLfoSyncButton);

    // Set up Trasher 1 controls
    trasher1ModeBox.addItemList({"Fuzz", "Scream"}, 1);
    setupRotarySlider(trasher1AmountSlider, "%");
    setupRotarySlider(trasher1ToneSlider, "%");
    addAndMakeVisible(trasher1ModeBox);
    addAndMakeVisible(trasher1AmountSlider);
    addAndMakeVisible(trasher1ToneSlider);

    // Set up Trasher 2 controls
    trasher2ModeBox.addItemList({"Fuzz", "Scream"}, 1);
    setupRotarySlider(trasher2AmountSlider, "%");
    setupRotarySlider(trasher2ToneSlider, "%");
    addAndMakeVisible(trasher2ModeBox);
    addAndMakeVisible(trasher2AmountSlider);
    addAndMakeVisible(trasher2ToneSlider);

    // Set up Echo controls
    setupRotarySlider(echoTimeSlider, "s");
    setupRotarySlider(echoFeedbackSlider, "%");
    setupRotarySlider(echoAmountSlider, "%");
    echoSyncButton.setButtonText("Sync to BPM");
    addAndMakeVisible(echoTimeSlider);
    addAndMakeVisible(echoFeedbackSlider);
    addAndMakeVisible(echoAmountSlider);
    addAndMakeVisible(echoSyncButton);

    // Set up Reverb controls
    setupRotarySlider(reverbSizeSlider, "%");
    setupRotarySlider(reverbDampingSlider, "%");
    setupRotarySlider(reverbWidthSlider, "%");
    setupRotarySlider(reverbAmountSlider, "%");
    addAndMakeVisible(reverbSizeSlider);
    addAndMakeVisible(reverbDampingSlider);
    addAndMakeVisible(reverbWidthSlider);
    addAndMakeVisible(reverbAmountSlider);

    // Set up Global controls
    setupSlider(dryWetSlider, "%");
    oversamplingBox.addItemList({"Off", "2x", "4x", "8x"}, 1);
    randomizeButton.setButtonText("Randomize");
    randomizeButton.onClick = [this] { processor.randomizeParameters(); };
    addAndMakeVisible(dryWetSlider);
    addAndMakeVisible(oversamplingBox);
    addAndMakeVisible(randomizeButton);

    // Create parameter attachments
    vcaLfoRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.parameters, KinaVSTProcessor::VCA_LFO_RATE_ID, vcaLfoRateSlider);
    vcaLfoAmountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.parameters, KinaVSTProcessor::VCA_LFO_AMOUNT_ID, vcaLfoAmountSlider);
    vcaAmountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.parameters, KinaVSTProcessor::VCA_AMOUNT_ID, vcaAmountSlider);
    vcaLfoShapeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        processor.parameters, KinaVSTProcessor::VCA_LFO_SHAPE_ID, vcaLfoShapeBox);
    vcaLfoSyncAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.parameters, KinaVSTProcessor::VCA_LFO_SYNC_ID, vcaLfoSyncButton);

    vcfCutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.parameters, KinaVSTProcessor::VCF_CUTOFF_ID, vcfCutoffSlider);
    vcfResonanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.parameters, KinaVSTProcessor::VCF_RESONANCE_ID, vcfResonanceSlider);
    vcfLfoRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.parameters, KinaVSTProcessor::VCF_LFO_RATE_ID, vcfLfoRateSlider);
    vcfLfoAmountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.parameters, KinaVSTProcessor::VCF_LFO_AMOUNT_ID, vcfLfoAmountSlider);
    vcfTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        processor.parameters, KinaVSTProcessor::VCF_TYPE_ID, vcfTypeBox);
    vcfLfoSyncAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.parameters, KinaVSTProcessor::VCF_LFO_SYNC_ID, vcfLfoSyncButton);

    trasher1ModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        processor.parameters, KinaVSTProcessor::TRASHER1_MODE_ID, trasher1ModeBox);
    trasher1AmountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.parameters, KinaVSTProcessor::TRASHER1_AMOUNT_ID, trasher1AmountSlider);
    trasher1ToneAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.parameters, KinaVSTProcessor::TRASHER1_TONE_ID, trasher1ToneSlider);

    trasher2ModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        processor.parameters, KinaVSTProcessor::TRASHER2_MODE_ID, trasher2ModeBox);
    trasher2AmountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.parameters, KinaVSTProcessor::TRASHER2_AMOUNT_ID, trasher2AmountSlider);
    trasher2ToneAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.parameters, KinaVSTProcessor::TRASHER2_TONE_ID, trasher2ToneSlider);

    echoTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.parameters, KinaVSTProcessor::ECHO_TIME_ID, echoTimeSlider);
    echoFeedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.parameters, KinaVSTProcessor::ECHO_FEEDBACK_ID, echoFeedbackSlider);
    echoAmountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.parameters, KinaVSTProcessor::ECHO_AMOUNT_ID, echoAmountSlider);
    echoSyncAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.parameters, KinaVSTProcessor::ECHO_SYNC_ID, echoSyncButton);

    reverbSizeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.parameters, KinaVSTProcessor::REVERB_SIZE_ID, reverbSizeSlider);
    reverbDampingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.parameters, KinaVSTProcessor::REVERB_DAMPING_ID, reverbDampingSlider);
    reverbWidthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.parameters, KinaVSTProcessor::REVERB_WIDTH_ID, reverbWidthSlider);
    reverbAmountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.parameters, KinaVSTProcessor::REVERB_AMOUNT_ID, reverbAmountSlider);

    dryWetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.parameters, KinaVSTProcessor::DRY_WET_ID, dryWetSlider);
    oversamplingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        processor.parameters, KinaVSTProcessor::OVERSAMPLING_ID, oversamplingBox);

    setSize(800, 600);
}

KinaVSTEditor::~KinaVSTEditor()
{
}

void KinaVSTEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void KinaVSTEditor::resized()
{
    auto area = getLocalBounds().reduced(10);
    auto topRow = area.removeFromTop(area.getHeight() / 2);
    auto bottomRow = area;

    // Top row layout
    vcaGroup.setBounds(topRow.removeFromLeft(topRow.getWidth() / 3).reduced(5));
    vcfGroup.setBounds(topRow.removeFromLeft(topRow.getWidth() / 2).reduced(5));
    globalGroup.setBounds(topRow.reduced(5));

    // Bottom row layout
    auto trasherWidth = bottomRow.getWidth() / 4;
    trasher1Group.setBounds(bottomRow.removeFromLeft(trasherWidth).reduced(5));
    trasher2Group.setBounds(bottomRow.removeFromLeft(trasherWidth).reduced(5));
    echoGroup.setBounds(bottomRow.removeFromLeft(trasherWidth).reduced(5));
    reverbGroup.setBounds(bottomRow.reduced(5));

    // Layout VCA controls
    auto vcaArea = vcaGroup.getBounds().reduced(10);
    auto vcaTopRow = vcaArea.removeFromTop(vcaArea.getHeight() / 2);
    vcaLfoRateSlider.setBounds(vcaTopRow.removeFromLeft(vcaTopRow.getWidth() / 2).reduced(5));
    vcaLfoAmountSlider.setBounds(vcaTopRow.reduced(5));
    vcaLfoShapeBox.setBounds(vcaArea.removeFromTop(20));
    vcaLfoSyncButton.setBounds(vcaArea.removeFromTop(20));
    vcaAmountSlider.setBounds(vcaArea.reduced(5));

    // Layout VCF controls
    auto vcfArea = vcfGroup.getBounds().reduced(10);
    auto vcfTopRow = vcfArea.removeFromTop(vcfArea.getHeight() / 2);
    vcfCutoffSlider.setBounds(vcfTopRow.removeFromLeft(vcfTopRow.getWidth() / 2).reduced(5));
    vcfResonanceSlider.setBounds(vcfTopRow.reduced(5));
    vcfTypeBox.setBounds(vcfArea.removeFromTop(20));
    auto vcfBottomRow = vcfArea.removeFromBottom(vcfArea.getHeight() / 2);
    vcfLfoRateSlider.setBounds(vcfBottomRow.removeFromLeft(vcfBottomRow.getWidth() / 2).reduced(5));
    vcfLfoAmountSlider.setBounds(vcfBottomRow.reduced(5));
    vcfLfoSyncButton.setBounds(vcfArea);

    // Layout Trasher 1 controls
    auto trasher1Area = trasher1Group.getBounds().reduced(10);
    trasher1ModeBox.setBounds(trasher1Area.removeFromTop(20));
    auto trasher1Bottom = trasher1Area.removeFromBottom(trasher1Area.getHeight() / 2);
    trasher1AmountSlider.setBounds(trasher1Area.reduced(5));
    trasher1ToneSlider.setBounds(trasher1Bottom.reduced(5));

    // Layout Trasher 2 controls
    auto trasher2Area = trasher2Group.getBounds().reduced(10);
    trasher2ModeBox.setBounds(trasher2Area.removeFromTop(20));
    auto trasher2Bottom = trasher2Area.removeFromBottom(trasher2Area.getHeight() / 2);
    trasher2AmountSlider.setBounds(trasher2Area.reduced(5));
    trasher2ToneSlider.setBounds(trasher2Bottom.reduced(5));

    // Layout Echo controls
    auto echoArea = echoGroup.getBounds().reduced(10);
    echoSyncButton.setBounds(echoArea.removeFromTop(20));
    auto echoGrid = echoArea.removeFromTop(echoArea.getHeight() * 2 / 3);
    echoTimeSlider.setBounds(echoGrid.removeFromLeft(echoGrid.getWidth() / 2).reduced(5));
    echoFeedbackSlider.setBounds(echoGrid.reduced(5));
    echoAmountSlider.setBounds(echoArea.reduced(5));

    // Layout Reverb controls
    auto reverbArea = reverbGroup.getBounds().reduced(10);
    auto reverbTopRow = reverbArea.removeFromTop(reverbArea.getHeight() / 2);
    reverbSizeSlider.setBounds(reverbTopRow.removeFromLeft(reverbTopRow.getWidth() / 2).reduced(5));
    reverbDampingSlider.setBounds(reverbTopRow.reduced(5));
    auto reverbBottomRow = reverbArea;
    reverbWidthSlider.setBounds(reverbBottomRow.removeFromLeft(reverbBottomRow.getWidth() / 2).reduced(5));
    reverbAmountSlider.setBounds(reverbBottomRow.reduced(5));

    // Layout Global controls
    auto globalArea = globalGroup.getBounds().reduced(10);
    dryWetSlider.setBounds(globalArea.removeFromTop(globalArea.getHeight() / 3).reduced(5));
    oversamplingBox.setBounds(globalArea.removeFromTop(20));
    randomizeButton.setBounds(globalArea.reduced(5));
}

void KinaVSTEditor::setupSlider(juce::Slider& slider, const juce::String& suffix)
{
    slider.setSliderStyle(juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    slider.setTextValueSuffix(suffix);
}

void KinaVSTEditor::setupRotarySlider(juce::Slider& slider, const juce::String& suffix)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    slider.setTextValueSuffix(suffix);
} 