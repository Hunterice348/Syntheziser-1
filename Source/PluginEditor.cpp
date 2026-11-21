#include "PluginProcessor.h"
#include "PluginEditor.h"

HuntericeAbyssAudioProcessorEditor::HuntericeAbyssAudioProcessorEditor (HuntericeAbyssAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), parameters (vts)
{
    setSize (1200, 600);

    auto setupSlider = [](juce::Slider& s) {
        s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 20);
    };

    setupSlider (attackSlider); addAndMakeVisible (attackSlider);
    setupSlider (decaySlider); addAndMakeVisible (decaySlider);
    setupSlider (sustainSlider); addAndMakeVisible (sustainSlider);
    setupSlider (releaseSlider); addAndMakeVisible (releaseSlider);
    setupSlider (volumeSlider); addAndMakeVisible (volumeSlider);
    
    setupSlider (osc2LevelSlider); addAndMakeVisible (osc2LevelSlider);
    setupSlider (osc2DetuneSlider); addAndMakeVisible (osc2DetuneSlider);
    setupSlider (subLevelSlider); addAndMakeVisible (subLevelSlider);
    setupSlider (noiseLevelSlider); addAndMakeVisible (noiseLevelSlider);

    setupSlider (analogDriftSlider); addAndMakeVisible (analogDriftSlider);
    setupSlider (masterDriveSlider); addAndMakeVisible (masterDriveSlider);

    setupSlider (filterCutoffSlider); addAndMakeVisible (filterCutoffSlider);
    setupSlider (filterResonanceSlider); addAndMakeVisible (filterResonanceSlider);
    setupSlider (filterDriveSlider); addAndMakeVisible (filterDriveSlider);
    setupSlider (filterKeyTrackSlider); addAndMakeVisible (filterKeyTrackSlider);

    setupSlider (filterAttackSlider); addAndMakeVisible (filterAttackSlider);
    setupSlider (filterDecaySlider); addAndMakeVisible (filterDecaySlider);
    setupSlider (filterSustainSlider); addAndMakeVisible (filterSustainSlider);
    setupSlider (filterReleaseSlider); addAndMakeVisible (filterReleaseSlider);
    setupSlider (filterEnvAmountSlider); addAndMakeVisible (filterEnvAmountSlider);

    setupSlider (lfoRateSlider); addAndMakeVisible (lfoRateSlider);
    setupSlider (lfoDepthSlider); addAndMakeVisible (lfoDepthSlider);

    setupSlider (modLfoToPitchSlider); addAndMakeVisible (modLfoToPitchSlider);
    setupSlider (modLfoToCutoffSlider); addAndMakeVisible (modLfoToCutoffSlider);
    setupSlider (modEnvToPitchSlider); addAndMakeVisible (modEnvToPitchSlider);
    setupSlider (modVelToCutoffSlider); addAndMakeVisible (modVelToCutoffSlider);

    setupSlider (patch1AmtSlider); addAndMakeVisible (patch1AmtSlider);
    setupSlider (patch2AmtSlider); addAndMakeVisible (patch2AmtSlider);
    setupSlider (patch3AmtSlider); addAndMakeVisible (patch3AmtSlider);
    setupSlider (patch4AmtSlider); addAndMakeVisible (patch4AmtSlider);

    setupSlider (distDriveSlider); addAndMakeVisible (distDriveSlider);
    setupSlider (distMixSlider); addAndMakeVisible (distMixSlider);
    setupSlider (chorusRateSlider); addAndMakeVisible (chorusRateSlider);
    setupSlider (chorusDepthSlider); addAndMakeVisible (chorusDepthSlider);
    setupSlider (chorusMixSlider); addAndMakeVisible (chorusMixSlider);
    setupSlider (delayTimeSlider); addAndMakeVisible (delayTimeSlider);
    setupSlider (delayFeedbackSlider); addAndMakeVisible (delayFeedbackSlider);
    setupSlider (delayMixSlider); addAndMakeVisible (delayMixSlider);
    setupSlider (reverbDecaySlider); addAndMakeVisible (reverbDecaySlider);
    setupSlider (reverbMixSlider); addAndMakeVisible (reverbMixSlider);

    fxBypassButton.setButtonText ("FX BYPASS");
    fxBypassButton.setColour (juce::ToggleButton::textColourId, juce::Colours::white);
    addAndMakeVisible (fxBypassButton);

    auto setupCombo = [](juce::ComboBox& c) {
        c.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xFF2A2A32));
        c.setColour (juce::ComboBox::textColourId, juce::Colours::white);
    };
    setupCombo (patch1SrcBox); addAndMakeVisible (patch1SrcBox);
    setupCombo (patch1DstBox); addAndMakeVisible (patch1DstBox);
    setupCombo (patch2SrcBox); addAndMakeVisible (patch2SrcBox);
    setupCombo (patch2DstBox); addAndMakeVisible (patch2DstBox);
    setupCombo (patch3SrcBox); addAndMakeVisible (patch3SrcBox);
    setupCombo (patch3DstBox); addAndMakeVisible (patch3DstBox);
    setupCombo (patch4SrcBox); addAndMakeVisible (patch4SrcBox);
    setupCombo (patch4DstBox); addAndMakeVisible (patch4DstBox);

    waveform1Box.addItemList ({ "Sine", "Saw", "Square", "Triangle" }, 1);
    waveform2Box.addItemList ({ "Sine", "Saw", "Square", "Triangle" }, 1);
    filterTypeBox.addItemList ({ "LP 12dB", "LP 24dB", "High Pass", "Band Pass" }, 1);
    lfoWaveformBox.addItemList ({ "Sine", "Triangle", "Saw", "Square", "S&H" }, 1);
    
    auto srcList = juce::StringArray { "None", "LFO", "Filter Env", "Velocity", "Mod Wheel", "Osc 1 (Audio)", "Noise (Audio)" };
    auto dstList = juce::StringArray { "None", "Filter Cutoff", "Osc 1 Pitch", "Osc 2 Pitch", "Master Vol", "Filter Res" };
    
    patch1SrcBox.addItemList (srcList, 1); patch1DstBox.addItemList (dstList, 1);
    patch2SrcBox.addItemList (srcList, 1); patch2DstBox.addItemList (dstList, 1);
    patch3SrcBox.addItemList (srcList, 1); patch3DstBox.addItemList (dstList, 1);
    patch4SrcBox.addItemList (srcList, 1); patch4DstBox.addItemList (dstList, 1);

    addAndMakeVisible (waveform1Box); addAndMakeVisible (waveform2Box);
    addAndMakeVisible (filterTypeBox); addAndMakeVisible (lfoWaveformBox);

    attackAtt = std::make_unique<Attachment> (parameters, "attack", attackSlider);
    decayAtt = std::make_unique<Attachment> (parameters, "decay", decaySlider);
    sustainAtt = std::make_unique<Attachment> (parameters, "sustain", sustainSlider);
    releaseAtt = std::make_unique<Attachment> (parameters, "release", releaseSlider);
    volumeAtt = std::make_unique<Attachment> (parameters, "volume", volumeSlider);
    
    osc2LevelAtt = std::make_unique<Attachment> (parameters, "osc2Level", osc2LevelSlider);
    osc2DetuneAtt = std::make_unique<Attachment> (parameters, "osc2Detune", osc2DetuneSlider);
    subLevelAtt = std::make_unique<Attachment> (parameters, "subLevel", subLevelSlider);
    noiseLevelAtt = std::make_unique<Attachment> (parameters, "noiseLevel", noiseLevelSlider);
    
    analogDriftAtt = std::make_unique<Attachment> (parameters, "analogDrift", analogDriftSlider);
    masterDriveAtt = std::make_unique<Attachment> (parameters, "masterDrive", masterDriveSlider);
    
    filterCutoffAtt = std::make_unique<Attachment> (parameters, "filterCutoff", filterCutoffSlider);
    filterResonanceAtt = std::make_unique<Attachment> (parameters, "filterResonance", filterResonanceSlider);
    filterDriveAtt = std::make_unique<Attachment> (parameters, "filterDrive", filterDriveSlider);
    filterKeyTrackAtt = std::make_unique<Attachment> (parameters, "filterKeyTrack", filterKeyTrackSlider);
    
    filterAttackAtt = std::make_unique<Attachment> (parameters, "filterAttack", filterAttackSlider);
    filterDecayAtt = std::make_unique<Attachment> (parameters, "filterDecay", filterDecaySlider);
    filterSustainAtt = std::make_unique<Attachment> (parameters, "filterSustain", filterSustainSlider);
    filterReleaseAtt = std::make_unique<Attachment> (parameters, "filterRelease", filterReleaseSlider);
    filterEnvAmountAtt = std::make_unique<Attachment> (parameters, "filterEnvAmount", filterEnvAmountSlider);
    
    lfoRateAtt = std::make_unique<Attachment> (parameters, "lfoRate", lfoRateSlider);
    lfoDepthAtt = std::make_unique<Attachment> (parameters, "lfoDepth", lfoDepthSlider);
    
    modLfoToPitchAtt = std::make_unique<Attachment> (parameters, "modLfoToPitch", modLfoToPitchSlider);
    modLfoToCutoffAtt = std::make_unique<Attachment> (parameters, "modLfoToCutoff", modLfoToCutoffSlider);
    modEnvToPitchAtt = std::make_unique<Attachment> (parameters, "modEnvToPitch", modEnvToPitchSlider);
    modVelToCutoffAtt = std::make_unique<Attachment> (parameters, "modVelToCutoff", modVelToCutoffSlider);
    
    patch1SrcAtt = std::make_unique<ComboBoxAttachment> (parameters, "patch0Src", patch1SrcBox);
    patch1DstAtt = std::make_unique<ComboBoxAttachment> (parameters, "patch0Dst", patch1DstBox);
    patch1AmtAtt = std::make_unique<Attachment> (parameters, "patch0Amt", patch1AmtSlider);
    
    patch2SrcAtt = std::make_unique<ComboBoxAttachment> (parameters, "patch1Src", patch2SrcBox);
    patch2DstAtt = std::make_unique<ComboBoxAttachment> (parameters, "patch1Dst", patch2DstBox);
    patch2AmtAtt = std::make_unique<Attachment> (parameters, "patch1Amt", patch2AmtSlider);
    
    patch3SrcAtt = std::make_unique<ComboBoxAttachment> (parameters, "patch2Src", patch3SrcBox);
    patch3DstAtt = std::make_unique<ComboBoxAttachment> (parameters, "patch2Dst", patch3DstBox);
    patch3AmtAtt = std::make_unique<Attachment> (parameters, "patch2Amt", patch3AmtSlider);
    
    patch4SrcAtt = std::make_unique<ComboBoxAttachment> (parameters, "patch3Src", patch4SrcBox);
    patch4DstAtt = std::make_unique<ComboBoxAttachment> (parameters, "patch3Dst", patch4DstBox);
    patch4AmtAtt = std::make_unique<Attachment> (parameters, "patch3Amt", patch4AmtSlider);

    distDriveAtt = std::make_unique<Attachment> (parameters, "distDrive", distDriveSlider);
    distMixAtt = std::make_unique<Attachment> (parameters, "distMix", distMixSlider);
    chorusRateAtt = std::make_unique<Attachment> (parameters, "chorusRate", chorusRateSlider);
    chorusDepthAtt = std::make_unique<Attachment> (parameters, "chorusDepth", chorusDepthSlider);
    chorusMixAtt = std::make_unique<Attachment> (parameters, "chorusMix", chorusMixSlider);
    delayTimeAtt = std::make_unique<Attachment> (parameters, "delayTime", delayTimeSlider);
    delayFeedbackAtt = std::make_unique<Attachment> (parameters, "delayFeedback", delayFeedbackSlider);
    delayMixAtt = std::make_unique<Attachment> (parameters, "delayMix", delayMixSlider);
    reverbDecayAtt = std::make_unique<Attachment> (parameters, "reverbDecay", reverbDecaySlider);
    reverbMixAtt = std::make_unique<Attachment> (parameters, "reverbMix", reverbMixSlider);
    fxBypassAtt = std::make_unique<ButtonAttachment> (parameters, "fxBypass", fxBypassButton);
    
    wf1Att = std::make_unique<ComboBoxAttachment> (parameters, "waveform1", waveform1Box);
    wf2Att = std::make_unique<ComboBoxAttachment> (parameters, "waveform2", waveform2Box);
    filterTypeAtt = std::make_unique<ComboBoxAttachment> (parameters, "filterType", filterTypeBox);
    lfoWaveformAtt = std::make_unique<ComboBoxAttachment> (parameters, "lfoWaveform", lfoWaveformBox);
}

HuntericeAbyssAudioProcessorEditor::~HuntericeAbyssAudioProcessorEditor() {}

void HuntericeAbyssAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xFF1E1E24));
    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (24.0f, juce::Font::bold));
    g.drawText ("HUNTERICE ABYSS", getLocalBounds().removeFromTop (40), juce::Justification::centred);
    g.setColour (juce::Colour (0xFFB41E28));
    g.fillRect (0, 40, getWidth(), 2);
    
    // ✅ ИСПРАВЛЕНО: рисование секции эффектов перенесено сюда из resized()
    auto fxArea = getLocalBounds().reduced (15).removeFromTop (380).removeFromBottom (200);
    g.setColour (juce::Colour (0xFF2A2A32));
    g.fillRect (fxArea);
}

void HuntericeAbyssAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (15).removeFromTop (340);
    
    auto oscArea = area.removeFromLeft (150);
    waveform1Box.setBounds (oscArea.removeFromTop (30).reduced (10));
    waveform2Box.setBounds (oscArea.removeFromTop (30).reduced (10));
    osc2LevelSlider.setBounds (oscArea.removeFromLeft (37));
    osc2DetuneSlider.setBounds (oscArea.removeFromLeft (37));
    subLevelSlider.setBounds (oscArea.removeFromLeft (37));
    noiseLevelSlider.setBounds (oscArea.removeFromLeft (37));
    
    auto filterArea = area.removeFromLeft (170);
    filterTypeBox.setBounds (filterArea.removeFromTop (30).reduced (10));
    filterCutoffSlider.setBounds (filterArea.removeFromLeft (42));
    filterResonanceSlider.setBounds (filterArea.removeFromLeft (42));
    filterDriveSlider.setBounds (filterArea.removeFromLeft (42));
    filterKeyTrackSlider.setBounds (filterArea.removeFromLeft (42));
    
    auto modArea = area.removeFromLeft (150);
    lfoWaveformBox.setBounds (modArea.removeFromTop (30).reduced (10));
    lfoRateSlider.setBounds (modArea.removeFromLeft (37));
    lfoDepthSlider.setBounds (modArea.removeFromLeft (37));
    filterEnvAmountSlider.setBounds (modArea.removeFromLeft (37));
    filterAttackSlider.setBounds (modArea.removeFromLeft (37));
    
    auto matrixArea = area.removeFromLeft (150);
    matrixArea.removeFromTop (10);
    modLfoToPitchSlider.setBounds (matrixArea.removeFromLeft (37));
    modLfoToCutoffSlider.setBounds (matrixArea.removeFromLeft (37));
    modEnvToPitchSlider.setBounds (matrixArea.removeFromLeft (37));
    modVelToCutoffSlider.setBounds (matrixArea.removeFromLeft (37));
    
    auto patchArea = area.removeFromLeft (220);
    patchArea.removeFromTop (10);
    
    auto rowH = 60;
    patch1SrcBox.setBounds (patchArea.removeFromTop (22).reduced (3, 0));
    patch1DstBox.setBounds (patchArea.removeFromTop (22).reduced (3, 0));
    patch1AmtSlider.setBounds (patchArea.removeFromTop (rowH).reduced (3, 0));
    
    patch2SrcBox.setBounds (patchArea.removeFromTop (22).reduced (3, 0));
    patch2DstBox.setBounds (patchArea.removeFromTop (22).reduced (3, 0));
    patch2AmtSlider.setBounds (patchArea.removeFromTop (rowH).reduced (3, 0));
    
    patch3SrcBox.setBounds (patchArea.removeFromTop (22).reduced (3, 0));
    patch3DstBox.setBounds (patchArea.removeFromTop (22).reduced (3, 0));
    patch3AmtSlider.setBounds (patchArea.removeFromTop (rowH).reduced (3, 0));
    
    patch4SrcBox.setBounds (patchArea.removeFromTop (22).reduced (3, 0));
    patch4DstBox.setBounds (patchArea.removeFromTop (22).reduced (3, 0));
    patch4AmtSlider.setBounds (patchArea.removeFromTop (rowH).reduced (3, 0));
    
    auto envArea = area.removeFromLeft (180);
    attackSlider.setBounds (envArea.removeFromLeft (45));
    decaySlider.setBounds (envArea.removeFromLeft (45));
    sustainSlider.setBounds (envArea.removeFromLeft (45));
    releaseSlider.setBounds (envArea.removeFromLeft (45));
    
    auto masterArea = area.removeFromLeft (100);
    analogDriftSlider.setBounds (masterArea.removeFromLeft (33));
    masterDriveSlider.setBounds (masterArea.removeFromLeft (33));
    volumeSlider.setBounds (masterArea.removeFromLeft (33));
}
