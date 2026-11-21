#pragma once
#include <JuceHeader.h>

class HuntericeAbyssAudioProcessor;

class HuntericeAbyssAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    HuntericeAbyssAudioProcessorEditor (HuntericeAbyssAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~HuntericeAbyssAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    HuntericeAbyssAudioProcessor& audioProcessor;
    juce::AudioProcessorValueTreeState& parameters;

    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider, volumeSlider;
    juce::Slider osc2LevelSlider, osc2DetuneSlider, subLevelSlider, noiseLevelSlider;
    juce::Slider analogDriftSlider, masterDriveSlider;
    juce::Slider filterCutoffSlider, filterResonanceSlider, filterDriveSlider, filterKeyTrackSlider;
    juce::Slider filterAttackSlider, filterDecaySlider, filterSustainSlider, filterReleaseSlider, filterEnvAmountSlider;
    juce::Slider lfoRateSlider, lfoDepthSlider;
    juce::Slider modLfoToPitchSlider, modLfoToCutoffSlider, modEnvToPitchSlider, modVelToCutoffSlider;
    juce::ComboBox waveform1Box, waveform2Box, filterTypeBox, lfoWaveformBox;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<Attachment> attackAtt, decayAtt, sustainAtt, releaseAtt, volumeAtt;
    std::unique_ptr<Attachment> osc2LevelAtt, osc2DetuneAtt, subLevelAtt, noiseLevelAtt;
    std::unique_ptr<Attachment> analogDriftAtt, masterDriveAtt;
    std::unique_ptr<Attachment> filterCutoffAtt, filterResonanceAtt, filterDriveAtt, filterKeyTrackAtt;
    std::unique_ptr<Attachment> filterAttackAtt, filterDecayAtt, filterSustainAtt, filterReleaseAtt, filterEnvAmountAtt;
    std::unique_ptr<Attachment> lfoRateAtt, lfoDepthAtt;
    std::unique_ptr<Attachment> modLfoToPitchAtt, modLfoToCutoffAtt, modEnvToPitchAtt, modVelToCutoffAtt;
    std::unique_ptr<ComboBoxAttachment> wf1Att, wf2Att, filterTypeAtt, lfoWaveformAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HuntericeAbyssAudioProcessorEditor)
};
