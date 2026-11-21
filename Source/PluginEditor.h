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
    
    juce::ComboBox patch1SrcBox, patch1DstBox; juce::Slider patch1AmtSlider;
    juce::ComboBox patch2SrcBox, patch2DstBox; juce::Slider patch2AmtSlider;
    juce::ComboBox patch3SrcBox, patch3DstBox; juce::Slider patch3AmtSlider;
    juce::ComboBox patch4SrcBox, patch4DstBox; juce::Slider patch4AmtSlider;

    juce::Slider distDriveSlider, distMixSlider;
    juce::Slider chorusRateSlider, chorusDepthSlider, chorusMixSlider;
    juce::Slider delayTimeSlider, delayFeedbackSlider, delayMixSlider;
    juce::Slider reverbDecaySlider, reverbMixSlider;
    juce::ToggleButton fxBypassButton;

    juce::Slider unisonSpreadSlider;
    juce::Slider macroMorphSlider;

    juce::ComboBox waveform1Box, waveform2Box, filterTypeBox, lfoWaveformBox;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<Attachment> attackAtt, decayAtt, sustainAtt, releaseAtt, volumeAtt;
    std::unique_ptr<Attachment> osc2LevelAtt, osc2DetuneAtt, subLevelAtt, noiseLevelAtt;
    std::unique_ptr<Attachment> analogDriftAtt, masterDriveAtt;
    std::unique_ptr<Attachment> filterCutoffAtt, filterResonanceAtt, filterDriveAtt, filterKeyTrackAtt;
    std::unique_ptr<Attachment> filterAttackAtt, filterDecayAtt, filterSustainAtt, filterReleaseAtt, filterEnvAmountAtt;
    std::unique_ptr<Attachment> lfoRateAtt, lfoDepthAtt;
    std::unique_ptr<Attachment> modLfoToPitchAtt, modLfoToCutoffAtt, modEnvToPitchAtt, modVelToCutoffAtt;
    
    std::unique_ptr<ComboBoxAttachment> patch1SrcAtt, patch1DstAtt; std::unique_ptr<Attachment> patch1AmtAtt;
    std::unique_ptr<ComboBoxAttachment> patch2SrcAtt, patch2DstAtt; std::unique_ptr<Attachment> patch2AmtAtt;
    std::unique_ptr<ComboBoxAttachment> patch3SrcAtt, patch3DstAtt; std::unique_ptr<Attachment> patch3AmtAtt;
    std::unique_ptr<ComboBoxAttachment> patch4SrcAtt, patch4DstAtt; std::unique_ptr<Attachment> patch4AmtAtt;

    std::unique_ptr<Attachment> distDriveAtt, distMixAtt;
    std::unique_ptr<Attachment> chorusRateAtt, chorusDepthAtt, chorusMixAtt;
    std::unique_ptr<Attachment> delayTimeAtt, delayFeedbackAtt, delayMixAtt;
    std::unique_ptr<Attachment> reverbDecayAtt, reverbMixAtt;
    std::unique_ptr<ButtonAttachment> fxBypassAtt;

    std::unique_ptr<Attachment> unisonSpreadAtt;
    std::unique_ptr<Attachment> macroMorphAtt;

    std::unique_ptr<ComboBoxAttachment> wf1Att, wf2Att, filterTypeAtt, lfoWaveformAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HuntericeAbyssAudioProcessorEditor)
};
