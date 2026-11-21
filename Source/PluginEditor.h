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

    // DRUM MACHINE (Stage 14)
    juce::Slider drumVolSlider, drumDriveSlider;
    juce::Slider kickLvlSlider, kickTuneSlider, kickDecaySlider, kickToneSlider;
    juce::Slider snareLvlSlider, snareTuneSlider, snareDecaySlider, snareToneSlider;
    juce::Slider hatLvlSlider, hatTuneSlider, hatDecaySlider, hatToneSlider;
    juce::Slider clapLvlSlider, clapTuneSlider, clapDecaySlider, clapToneSlider;
    juce::Slider tomLvlSlider, tomTuneSlider, tomDecaySlider, tomToneSlider;
    juce::Slider percLvlSlider, percTuneSlider, percDecaySlider, percToneSlider;

    // LABELS
    juce::Label sectionOsc, sectionFilter, sectionEnv, sectionLfo;
    juce::Label sectionMod, sectionPatch, sectionFx, sectionMaster, sectionDrum;
    
    juce::Label attackLabel, decayLabel, sustainLabel, releaseLabel, volumeLabel;
    juce::Label osc2LevelLabel, osc2DetuneLabel, subLevelLabel, noiseLevelLabel;
    juce::Label analogDriftLabel, masterDriveLabel;
    juce::Label filterCutoffLabel, filterResonanceLabel, filterDriveLabel, filterKeyTrackLabel;
    juce::Label filterAttackLabel, filterDecayLabel, filterSustainLabel, filterReleaseLabel, filterEnvAmountLabel;
    juce::Label lfoRateLabel, lfoDepthLabel;
    juce::Label modLfoToPitchLabel, modLfoToCutoffLabel, modEnvToPitchLabel, modVelToCutoffLabel;
    juce::Label patch1AmtLabel, patch2AmtLabel, patch3AmtLabel, patch4AmtLabel;
    juce::Label distDriveLabel, distMixLabel;
    juce::Label chorusRateLabel, chorusDepthLabel, chorusMixLabel;
    juce::Label delayTimeLabel, delayFeedbackLabel, delayMixLabel;
    juce::Label reverbDecayLabel, reverbMixLabel;
    juce::Label unisonSpreadLabel, macroMorphLabel;
    juce::Label waveform1Label, waveform2Label, filterTypeLabel, lfoWaveformLabel;
    juce::Label patch1SrcLabel, patch1DstLabel;
    juce::Label patch2SrcLabel, patch2DstLabel;
    juce::Label patch3SrcLabel, patch3DstLabel;
    juce::Label patch4SrcLabel, patch4DstLabel;
    
    juce::Label drumVolLabel, drumDriveLabel;
    juce::Label kickLvlLabel, kickTuneLabel, kickDecayLabel, kickToneLabel;
    juce::Label snareLvlLabel, snareTuneLabel, snareDecayLabel, snareToneLabel;
    juce::Label hatLvlLabel, hatTuneLabel, hatDecayLabel, hatToneLabel;
    juce::Label clapLvlLabel, clapTuneLabel, clapDecayLabel, clapToneLabel;
    juce::Label tomLvlLabel, tomTuneLabel, tomDecayLabel, tomToneLabel;
    juce::Label percLvlLabel, percTuneLabel, percDecayLabel, percToneLabel;

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

    // DRUM ATTACHMENTS
    std::unique_ptr<Attachment> drumVolAtt, drumDriveAtt;
    std::unique_ptr<Attachment> kickLvlAtt, kickTuneAtt, kickDecayAtt, kickToneAtt;
    std::unique_ptr<Attachment> snareLvlAtt, snareTuneAtt, snareDecayAtt, snareToneAtt;
    std::unique_ptr<Attachment> hatLvlAtt, hatTuneAtt, hatDecayAtt, hatToneAtt;
    std::unique_ptr<Attachment> clapLvlAtt, clapTuneAtt, clapDecayAtt, clapToneAtt;
    std::unique_ptr<Attachment> tomLvlAtt, tomTuneAtt, tomDecayAtt, tomToneAtt;
    std::unique_ptr<Attachment> percLvlAtt, percTuneAtt, percDecayAtt, percToneAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HuntericeAbyssAudioProcessorEditor)
};
