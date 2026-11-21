#include "PluginProcessor.h"
#include "PluginEditor.h"

HuntericeAbyssAudioProcessorEditor::HuntericeAbyssAudioProcessorEditor (HuntericeAbyssAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), parameters (vts)
{
    setSize (1400, 820);

    auto setupSlider = [](juce::Slider& s) {
        s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 18);
        s.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
        s.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0xFF1E1E24));
        s.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        s.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xFFB41E28));
        s.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xFF5A5A64));
        s.setColour (juce::Slider::thumbColourId, juce::Colours::white);
    };

    auto setupLabel = [](juce::Label& l, const juce::String& text, int fontSize = 11) {
        l.setText (text, juce::dontSendNotification);
        l.setJustificationType (juce::Justification::centred);
        l.setColour (juce::Label::textColourId, juce::Colours::white);
        l.setFont (juce::Font ((float) fontSize, juce::Font::plain));
    };

    auto setupSectionLabel = [](juce::Label& l, const juce::String& text) {
        l.setText (text, juce::dontSendNotification);
        l.setJustificationType (juce::Justification::centred);
        l.setColour (juce::Label::textColourId, juce::Colours::white);
        l.setFont (juce::Font (14.0f, juce::Font::bold));
    };

    auto setupCombo = [](juce::ComboBox& c) {
        c.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xFF2A2A32));
        c.setColour (juce::ComboBox::textColourId, juce::Colours::white);
    };

    // SYNTH SECTION
    setupSlider (osc2LevelSlider); addAndMakeVisible (osc2LevelSlider);
    setupSlider (osc2DetuneSlider); addAndMakeVisible (osc2DetuneSlider);
    setupSlider (subLevelSlider); addAndMakeVisible (subLevelSlider);
    setupSlider (noiseLevelSlider); addAndMakeVisible (noiseLevelSlider);
    setupSlider (unisonSpreadSlider); addAndMakeVisible (unisonSpreadSlider);
    setupLabel (osc2LevelLabel, "OSC2 LVL"); addAndMakeVisible (osc2LevelLabel);
    setupLabel (osc2DetuneLabel, "OSC2 DET"); addAndMakeVisible (osc2DetuneLabel);
    setupLabel (subLevelLabel, "SUB LVL"); addAndMakeVisible (subLevelLabel);
    setupLabel (noiseLevelLabel, "NOISE"); addAndMakeVisible (noiseLevelLabel);
    setupLabel (unisonSpreadLabel, "UNISON"); addAndMakeVisible (unisonSpreadLabel);

    waveform1Box.addItemList ({ "Sine", "Saw", "Square", "Triangle" }, 1);
    waveform2Box.addItemList ({ "Sine", "Saw", "Square", "Triangle" }, 1);
    addAndMakeVisible (waveform1Box); addAndMakeVisible (waveform2Box);
    setupLabel (waveform1Label, "OSC 1"); addAndMakeVisible (waveform1Label);
    setupLabel (waveform2Label, "OSC 2"); addAndMakeVisible (waveform2Label);

    setupSlider (filterCutoffSlider); addAndMakeVisible (filterCutoffSlider);
    setupSlider (filterResonanceSlider); addAndMakeVisible (filterResonanceSlider);
    setupSlider (filterDriveSlider); addAndMakeVisible (filterDriveSlider);
    setupSlider (filterKeyTrackSlider); addAndMakeVisible (filterKeyTrackSlider);
    setupLabel (filterCutoffLabel, "CUTOFF"); addAndMakeVisible (filterCutoffLabel);
    setupLabel (filterResonanceLabel, "RESO"); addAndMakeVisible (filterResonanceLabel);
    setupLabel (filterDriveLabel, "DRV"); addAndMakeVisible (filterDriveLabel);
    setupLabel (filterKeyTrackLabel, "KEY TRK"); addAndMakeVisible (filterKeyTrackLabel);

    filterTypeBox.addItemList ({ "LP 12dB", "LP 24dB", "High Pass", "Band Pass" }, 1);
    addAndMakeVisible (filterTypeBox);
    setupLabel (filterTypeLabel, "FILTER TYPE"); addAndMakeVisible (filterTypeLabel);

    setupSlider (attackSlider); addAndMakeVisible (attackSlider);
    setupSlider (decaySlider); addAndMakeVisible (decaySlider);
    setupSlider (sustainSlider); addAndMakeVisible (sustainSlider);
    setupSlider (releaseSlider); addAndMakeVisible (releaseSlider);
    setupSlider (filterAttackSlider); addAndMakeVisible (filterAttackSlider);
    setupSlider (filterDecaySlider); addAndMakeVisible (filterDecaySlider);
    setupSlider (filterSustainSlider); addAndMakeVisible (filterSustainSlider);
    setupSlider (filterReleaseSlider); addAndMakeVisible (filterReleaseSlider);
    setupSlider (filterEnvAmountSlider); addAndMakeVisible (filterEnvAmountSlider);
    setupLabel (attackLabel, "A"); addAndMakeVisible (attackLabel);
    setupLabel (decayLabel, "D"); addAndMakeVisible (decayLabel);
    setupLabel (sustainLabel, "S"); addAndMakeVisible (sustainLabel);
    setupLabel (releaseLabel, "R"); addAndMakeVisible (releaseLabel);
    setupLabel (filterAttackLabel, "F-A"); addAndMakeVisible (filterAttackLabel);
    setupLabel (filterDecayLabel, "F-D"); addAndMakeVisible (filterDecayLabel);
    setupLabel (filterSustainLabel, "F-S"); addAndMakeVisible (filterSustainLabel);
    setupLabel (filterReleaseLabel, "F-R"); addAndMakeVisible (filterReleaseLabel);
    setupLabel (filterEnvAmountLabel, "ENV AMT"); addAndMakeVisible (filterEnvAmountLabel);

    setupSlider (lfoRateSlider); addAndMakeVisible (lfoRateSlider);
    setupSlider (lfoDepthSlider); addAndMakeVisible (lfoDepthSlider);
    setupLabel (lfoRateLabel, "RATE"); addAndMakeVisible (lfoRateLabel);
    setupLabel (lfoDepthLabel, "DEPTH"); addAndMakeVisible (lfoDepthLabel);

    lfoWaveformBox.addItemList ({ "Sine", "Triangle", "Saw", "Square", "S&H" }, 1);
    addAndMakeVisible (lfoWaveformBox);
    setupLabel (lfoWaveformLabel, "LFO WAVE"); addAndMakeVisible (lfoWaveformLabel);

    setupSlider (modLfoToPitchSlider); addAndMakeVisible (modLfoToPitchSlider);
    setupSlider (modLfoToCutoffSlider); addAndMakeVisible (modLfoToCutoffSlider);
    setupSlider (modEnvToPitchSlider); addAndMakeVisible (modEnvToPitchSlider);
    setupSlider (modVelToCutoffSlider); addAndMakeVisible (modVelToCutoffSlider);
    setupLabel (modLfoToPitchLabel, "LFO>PIT"); addAndMakeVisible (modLfoToPitchLabel);
    setupLabel (modLfoToCutoffLabel, "LFO>CUT"); addAndMakeVisible (modLfoToCutoffLabel);
    setupLabel (modEnvToPitchLabel, "ENV>PIT"); addAndMakeVisible (modEnvToPitchLabel);
    setupLabel (modVelToCutoffLabel, "VEL>CUT"); addAndMakeVisible (modVelToCutoffLabel);

    setupCombo (patch1SrcBox); addAndMakeVisible (patch1SrcBox);
    setupCombo (patch1DstBox); addAndMakeVisible (patch1DstBox);
    setupSlider (patch1AmtSlider); addAndMakeVisible (patch1AmtSlider);
    setupCombo (patch2SrcBox); addAndMakeVisible (patch2SrcBox);
    setupCombo (patch2DstBox); addAndMakeVisible (patch2DstBox);
    setupSlider (patch2AmtSlider); addAndMakeVisible (patch2AmtSlider);
    setupCombo (patch3SrcBox); addAndMakeVisible (patch3SrcBox);
    setupCombo (patch3DstBox); addAndMakeVisible (patch3DstBox);
    setupSlider (patch3AmtSlider); addAndMakeVisible (patch3AmtSlider);
    setupCombo (patch4SrcBox); addAndMakeVisible (patch4SrcBox);
    setupCombo (patch4DstBox); addAndMakeVisible (patch4DstBox);
    setupSlider (patch4AmtSlider); addAndMakeVisible (patch4AmtSlider);

    setupLabel (patch1AmtLabel, "AMT 1"); addAndMakeVisible (patch1AmtLabel);
    setupLabel (patch2AmtLabel, "AMT 2"); addAndMakeVisible (patch2AmtLabel);
    setupLabel (patch3AmtLabel, "AMT 3"); addAndMakeVisible (patch3AmtLabel);
    setupLabel (patch4AmtLabel, "AMT 4"); addAndMakeVisible (patch4AmtLabel);
    setupLabel (patch1SrcLabel, "SRC 1"); addAndMakeVisible (patch1SrcLabel);
    setupLabel (patch1DstLabel, "DST 1"); addAndMakeVisible (patch1DstLabel);
    setupLabel (patch2SrcLabel, "SRC 2"); addAndMakeVisible (patch2SrcLabel);
    setupLabel (patch2DstLabel, "DST 2"); addAndMakeVisible (patch2DstLabel);
    setupLabel (patch3SrcLabel, "SRC 3"); addAndMakeVisible (patch3SrcLabel);
    setupLabel (patch3DstLabel, "DST 3"); addAndMakeVisible (patch3DstLabel);
    setupLabel (patch4SrcLabel, "SRC 4"); addAndMakeVisible (patch4SrcLabel);
    setupLabel (patch4DstLabel, "DST 4"); addAndMakeVisible (patch4DstLabel);

    auto srcList = juce::StringArray { "None", "LFO", "Filter Env", "Velocity", "Mod Wheel", "Osc 1 (Audio)", "Noise (Audio)" };
    auto dstList = juce::StringArray { "None", "Filter Cutoff", "Osc 1 Pitch", "Osc 2 Pitch", "Master Vol", "Filter Res" };
    patch1SrcBox.addItemList (srcList, 1); patch1DstBox.addItemList (dstList, 1);
    patch2SrcBox.addItemList (srcList, 1); patch2DstBox.addItemList (dstList, 1);
    patch3SrcBox.addItemList (srcList, 1); patch3DstBox.addItemList (dstList, 1);
    patch4SrcBox.addItemList (srcList, 1); patch4DstBox.addItemList (dstList, 1);

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

    setupLabel (distDriveLabel, "DRV"); addAndMakeVisible (distDriveLabel);
    setupLabel (distMixLabel, "MIX"); addAndMakeVisible (distMixLabel);
    setupLabel (chorusRateLabel, "RATE"); addAndMakeVisible (chorusRateLabel);
    setupLabel (chorusDepthLabel, "DEPTH"); addAndMakeVisible (chorusDepthLabel);
    setupLabel (chorusMixLabel, "MIX"); addAndMakeVisible (chorusMixLabel);
    setupLabel (delayTimeLabel, "TIME"); addAndMakeVisible (delayTimeLabel);
    setupLabel (delayFeedbackLabel, "FDBK"); addAndMakeVisible (delayFeedbackLabel);
    setupLabel (delayMixLabel, "MIX"); addAndMakeVisible (delayMixLabel);
    setupLabel (reverbDecayLabel, "DECAY"); addAndMakeVisible (reverbDecayLabel);
    setupLabel (reverbMixLabel, "MIX"); addAndMakeVisible (reverbMixLabel);

    fxBypassButton.setButtonText ("FX BYPASS");
    fxBypassButton.setColour (juce::ToggleButton::textColourId, juce::Colours::white);
    addAndMakeVisible (fxBypassButton);

    setupSlider (analogDriftSlider); addAndMakeVisible (analogDriftSlider);
    setupSlider (masterDriveSlider); addAndMakeVisible (masterDriveSlider);
    setupSlider (volumeSlider); addAndMakeVisible (volumeSlider);
    setupSlider (macroMorphSlider); addAndMakeVisible (macroMorphSlider);
    setupLabel (analogDriftLabel, "DRIFT"); addAndMakeVisible (analogDriftLabel);
    setupLabel (masterDriveLabel, "DRV"); addAndMakeVisible (masterDriveLabel);
    setupLabel (volumeLabel, "VOLUME"); addAndMakeVisible (volumeLabel);
    setupLabel (macroMorphLabel, "CLEAN < > DIRTY"); addAndMakeVisible (macroMorphLabel);

    // DRUM MACHINE (Stage 14)
    setupSlider (drumVolSlider); addAndMakeVisible (drumVolSlider);
    setupSlider (drumDriveSlider); addAndMakeVisible (drumDriveSlider);
    setupLabel (drumVolLabel, "DRUM VOL"); addAndMakeVisible (drumVolLabel);
    setupLabel (drumDriveLabel, "DRUM DRV"); addAndMakeVisible (drumDriveLabel);

    setupSlider (kickLvlSlider); addAndMakeVisible (kickLvlSlider);
    setupSlider (kickTuneSlider); addAndMakeVisible (kickTuneSlider);
    setupSlider (kickDecaySlider); addAndMakeVisible (kickDecaySlider);
    setupSlider (kickToneSlider); addAndMakeVisible (kickToneSlider);
    setupLabel (kickLvlLabel, "KICK LVL"); addAndMakeVisible (kickLvlLabel);
    setupLabel (kickTuneLabel, "KICK TUNE"); addAndMakeVisible (kickTuneLabel);
    setupLabel (kickDecayLabel, "KICK DEC"); addAndMakeVisible (kickDecayLabel);
    setupLabel (kickToneLabel, "KICK TONE"); addAndMakeVisible (kickToneLabel);

    setupSlider (snareLvlSlider); addAndMakeVisible (snareLvlSlider);
    setupSlider (snareTuneSlider); addAndMakeVisible (snareTuneSlider);
    setupSlider (snareDecaySlider); addAndMakeVisible (snareDecaySlider);
    setupSlider (snareToneSlider); addAndMakeVisible (snareToneSlider);
    setupLabel (snareLvlLabel, "SNARE LVL"); addAndMakeVisible (snareLvlLabel);
    setupLabel (snareTuneLabel, "SNARE TUNE"); addAndMakeVisible (snareTuneLabel);
    setupLabel (snareDecayLabel, "SNARE DEC"); addAndMakeVisible (snareDecayLabel);
    setupLabel (snareToneLabel, "SNARE TONE"); addAndMakeVisible (snareToneLabel);

    setupSlider (hatLvlSlider); addAndMakeVisible (hatLvlSlider);
    setupSlider (hatTuneSlider); addAndMakeVisible (hatTuneSlider);
    setupSlider (hatDecaySlider); addAndMakeVisible (hatDecaySlider);
    setupSlider (hatToneSlider); addAndMakeVisible (hatToneSlider);
    setupLabel (hatLvlLabel, "HAT LVL"); addAndMakeVisible (hatLvlLabel);
    setupLabel (hatTuneLabel, "HAT TUNE"); addAndMakeVisible (hatTuneLabel);
    setupLabel (hatDecayLabel, "HAT DEC"); addAndMakeVisible (hatDecayLabel);
    setupLabel (hatToneLabel, "HAT TONE"); addAndMakeVisible (hatToneLabel);

    setupSlider (clapLvlSlider); addAndMakeVisible (clapLvlSlider);
    setupSlider (clapTuneSlider); addAndMakeVisible (clapTuneSlider);
    setupSlider (clapDecaySlider); addAndMakeVisible (clapDecaySlider);
    setupSlider (clapToneSlider); addAndMakeVisible (clapToneSlider);
    setupLabel (clapLvlLabel, "CLAP LVL"); addAndMakeVisible (clapLvlLabel);
    setupLabel (clapTuneLabel, "CLAP TUNE"); addAndMakeVisible (clapTuneLabel);
    setupLabel (clapDecayLabel, "CLAP DEC"); addAndMakeVisible (clapDecayLabel);
    setupLabel (clapToneLabel, "CLAP TONE"); addAndMakeVisible (clapToneLabel);

    setupSlider (tomLvlSlider); addAndMakeVisible (tomLvlSlider);
    setupSlider (tomTuneSlider); addAndMakeVisible (tomTuneSlider);
    setupSlider (tomDecaySlider); addAndMakeVisible (tomDecaySlider);
    setupSlider (tomToneSlider); addAndMakeVisible (tomToneSlider);
    setupLabel (tomLvlLabel, "TOM LVL"); addAndMakeVisible (tomLvlLabel);
    setupLabel (tomTuneLabel, "TOM TUNE"); addAndMakeVisible (tomTuneLabel);
    setupLabel (tomDecayLabel, "TOM DEC"); addAndMakeVisible (tomDecayLabel);
    setupLabel (tomToneLabel, "TOM TONE"); addAndMakeVisible (tomToneLabel);

    setupSlider (percLvlSlider); addAndMakeVisible (percLvlSlider);
    setupSlider (percTuneSlider); addAndMakeVisible (percTuneSlider);
    setupSlider (percDecaySlider); addAndMakeVisible (percDecaySlider);
    setupSlider (percToneSlider); addAndMakeVisible (percToneSlider);
    setupLabel (percLvlLabel, "PERC LVL"); addAndMakeVisible (percLvlLabel);
    setupLabel (percTuneLabel, "PERC TUNE"); addAndMakeVisible (percTuneLabel);
    setupLabel (percDecayLabel, "PERC DEC"); addAndMakeVisible (percDecayLabel);
    setupLabel (percToneLabel, "PERC TONE"); addAndMakeVisible (percToneLabel);

    setupSectionLabel (sectionOsc, "OSCILLATORS"); addAndMakeVisible (sectionOsc);
    setupSectionLabel (sectionFilter, "FILTER"); addAndMakeVisible (sectionFilter);
    setupSectionLabel (sectionEnv, "ENVELOPE"); addAndMakeVisible (sectionEnv);
    setupSectionLabel (sectionLfo, "LFO"); addAndMakeVisible (sectionLfo);
    setupSectionLabel (sectionMod, "MODULATION"); addAndMakeVisible (sectionMod);
    setupSectionLabel (sectionPatch, "PATCH BAY"); addAndMakeVisible (sectionPatch);
    setupSectionLabel (sectionFx, "EFFECTS"); addAndMakeVisible (sectionFx);
    setupSectionLabel (sectionMaster, "MASTER"); addAndMakeVisible (sectionMaster);
    setupSectionLabel (sectionDrum, "DRUM MACHINE"); addAndMakeVisible (sectionDrum);

    // ATTACHMENTS
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
    unisonSpreadAtt = std::make_unique<Attachment> (parameters, "unisonSpread", unisonSpreadSlider);
    macroMorphAtt = std::make_unique<Attachment> (parameters, "macroMorph", macroMorphSlider);
    wf1Att = std::make_unique<ComboBoxAttachment> (parameters, "waveform1", waveform1Box);
    wf2Att = std::make_unique<ComboBoxAttachment> (parameters, "waveform2", waveform2Box);
    filterTypeAtt = std::make_unique<ComboBoxAttachment> (parameters, "filterType", filterTypeBox);
    lfoWaveformAtt = std::make_unique<ComboBoxAttachment> (parameters, "lfoWaveform", lfoWaveformBox);

    drumVolAtt = std::make_unique<Attachment> (parameters, "drumVol", drumVolSlider);
    drumDriveAtt = std::make_unique<Attachment> (parameters, "drumDrive", drumDriveSlider);
    kickLvlAtt = std::make_unique<Attachment> (parameters, "kickLvl", kickLvlSlider);
    kickTuneAtt = std::make_unique<Attachment> (parameters, "kickTune", kickTuneSlider);
    kickDecayAtt = std::make_unique<Attachment> (parameters, "kickDecay", kickDecaySlider);
    kickToneAtt = std::make_unique<Attachment> (parameters, "kickTone", kickToneSlider);
    snareLvlAtt = std::make_unique<Attachment> (parameters, "snareLvl", snareLvlSlider);
    snareTuneAtt = std::make_unique<Attachment> (parameters, "snareTune", snareTuneSlider);
    snareDecayAtt = std::make_unique<Attachment> (parameters, "snareDecay", snareDecaySlider);
    snareToneAtt = std::make_unique<Attachment> (parameters, "snareTone", snareToneSlider);
    hatLvlAtt = std::make_unique<Attachment> (parameters, "hi_hatLvl", hatLvlSlider);
    hatTuneAtt = std::make_unique<Attachment> (parameters, "hi_hatTune", hatTuneSlider);
    hatDecayAtt = std::make_unique<Attachment> (parameters, "hi_hatDecay", hatDecaySlider);
    hatToneAtt = std::make_unique<Attachment> (parameters, "hi_hatTone", hatToneSlider);
    clapLvlAtt = std::make_unique<Attachment> (parameters, "clapLvl", clapLvlSlider);
    clapTuneAtt = std::make_unique<Attachment> (parameters, "clapTune", clapTuneSlider);
    clapDecayAtt = std::make_unique<Attachment> (parameters, "clapDecay", clapDecaySlider);
    clapToneAtt = std::make_unique<Attachment> (parameters, "clapTone", clapToneSlider);
    tomLvlAtt = std::make_unique<Attachment> (parameters, "tomLvl", tomLvlSlider);
    tomTuneAtt = std::make_unique<Attachment> (parameters, "tomTune", tomTuneSlider);
    tomDecayAtt = std::make_unique<Attachment> (parameters, "tomDecay", tomDecaySlider);
    tomToneAtt = std::make_unique<Attachment> (parameters, "tomTone", tomToneSlider);
    percLvlAtt = std::make_unique<Attachment> (parameters, "percLvl", percLvlSlider);
    percTuneAtt = std::make_unique<Attachment> (parameters, "percTune", percTuneSlider);
    percDecayAtt = std::make_unique<Attachment> (parameters, "percDecay", percDecaySlider);
    percToneAtt = std::make_unique<Attachment> (parameters, "percTone", percToneSlider);
}

HuntericeAbyssAudioProcessorEditor::~HuntericeAbyssAudioProcessorEditor() {}

void HuntericeAbyssAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xFF1E1E24));
    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (22.0f, juce::Font::bold));
    g.drawText ("HUNTERICE ABYSS", getLocalBounds().removeFromTop (32), juce::Justification::centred);
    g.setColour (juce::Colour (0xFFB41E28));
    g.fillRect (0, 32, getWidth(), 2);

    auto area = getLocalBounds().reduced (10).withTrimmedTop (36);
    auto synthArea = area.removeFromTop (320);
    auto drumArea = area.removeFromTop (200);
    auto fxArea = area.removeFromTop (180);

    auto oscArea = synthArea.removeFromLeft (180);
    g.setColour (juce::Colour (0xFF2A1E1E)); g.fillRect (oscArea);
    g.setColour (juce::Colour (0xFFB41E28)); g.drawRect (oscArea, 1);

    auto filterArea = synthArea.removeFromLeft (180);
    g.setColour (juce::Colour (0xFF1E1E2A)); g.fillRect (filterArea);
    g.setColour (juce::Colour (0xFF4169E1)); g.drawRect (filterArea, 1);

    auto envArea = synthArea.removeFromLeft (280);
    g.setColour (juce::Colour (0xFF1E2A1E)); g.fillRect (envArea);
    g.setColour (juce::Colour (0xFF3CB371)); g.drawRect (envArea, 1);

    auto lfoArea = synthArea.removeFromLeft (140);
    g.setColour (juce::Colour (0xFF2A2A1E)); g.fillRect (lfoArea);
    g.setColour (juce::Colour (0xFFDAA520)); g.drawRect (lfoArea, 1);

    auto modArea = synthArea.removeFromLeft (160);
    g.setColour (juce::Colour (0xFF2A1E2A)); g.fillRect (modArea);
    g.setColour (juce::Colour (0xFF9370DB)); g.drawRect (modArea, 1);

    auto patchArea = synthArea;
    g.setColour (juce::Colour (0xFF2A2A1E)); g.fillRect (patchArea);
    g.setColour (juce::Colour (0xFFDAA520)); g.drawRect (patchArea, 1);

    g.setColour (juce::Colour (0xFF2A1A1A)); g.fillRect (drumArea);
    g.setColour (juce::Colour (0xFFFF4500)); g.drawRect (drumArea, 2);

    g.setColour (juce::Colour (0xFF2A1E1E)); g.fillRect (fxArea);
    g.setColour (juce::Colour (0xFFFF8C00)); g.drawRect (fxArea, 1);

    g.setColour (juce::Colour (0xFF1E2A2A)); g.fillRect (area);
    g.setColour (juce::Colour (0xFF00CED1)); g.drawRect (area, 1);
}

void HuntericeAbyssAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (10).withTrimmedTop (36);
    auto synthArea = area.removeFromTop (320);
    auto drumArea = area.removeFromTop (200);
    auto fxArea = area.removeFromTop (180);
    auto bottomArea = area;

    // OSC
    auto oscArea = synthArea.removeFromLeft (180);
    sectionOsc.setBounds (oscArea.removeFromTop (22));
    waveform1Label.setBounds (oscArea.removeFromTop (14));
    waveform1Box.setBounds (oscArea.removeFromTop (22).reduced (10, 2));
    waveform2Label.setBounds (oscArea.removeFromTop (14));
    waveform2Box.setBounds (oscArea.removeFromTop (22).reduced (10, 2));
    oscArea.removeFromTop (4);
    osc2LevelLabel.setBounds (oscArea.removeFromTop (12)); osc2LevelSlider.setBounds (oscArea.removeFromTop (46));
    osc2DetuneLabel.setBounds (oscArea.removeFromTop (12)); osc2DetuneSlider.setBounds (oscArea.removeFromTop (46));
    subLevelLabel.setBounds (oscArea.removeFromTop (12)); subLevelSlider.setBounds (oscArea.removeFromTop (46));
    noiseLevelLabel.setBounds (oscArea.removeFromTop (12)); noiseLevelSlider.setBounds (oscArea.removeFromTop (46));
    unisonSpreadLabel.setBounds (oscArea.removeFromTop (12)); unisonSpreadSlider.setBounds (oscArea.removeFromTop (46));

    // FILTER
    auto filterArea = synthArea.removeFromLeft (180);
    sectionFilter.setBounds (filterArea.removeFromTop (22));
    filterTypeLabel.setBounds (filterArea.removeFromTop (14));
    filterTypeBox.setBounds (filterArea.removeFromTop (22).reduced (10, 2));
    filterArea.removeFromTop (8);
    filterCutoffLabel.setBounds (filterArea.removeFromTop (12)); filterCutoffSlider.setBounds (filterArea.removeFromTop (50));
    filterResonanceLabel.setBounds (filterArea.removeFromTop (12)); filterResonanceSlider.setBounds (filterArea.removeFromTop (50));
    filterDriveLabel.setBounds (filterArea.removeFromTop (12)); filterDriveSlider.setBounds (filterArea.removeFromTop (50));
    filterKeyTrackLabel.setBounds (filterArea.removeFromTop (12)); filterKeyTrackSlider.setBounds (filterArea.removeFromTop (50));

    // ENV
    auto envArea = synthArea.removeFromLeft (280);
    sectionEnv.setBounds (envArea.removeFromTop (22));
    auto ampLabelArea = envArea.removeFromTop (14);
    attackLabel.setBounds (ampLabelArea.removeFromLeft (70));
    decayLabel.setBounds (ampLabelArea.removeFromLeft (70));
    sustainLabel.setBounds (ampLabelArea.removeFromLeft (70));
    releaseLabel.setBounds (ampLabelArea.removeFromLeft (70));
    auto ampSliderArea = envArea.removeFromTop (70);
    attackSlider.setBounds (ampSliderArea.removeFromLeft (70));
    decaySlider.setBounds (ampSliderArea.removeFromLeft (70));
    sustainSlider.setBounds (ampSliderArea.removeFromLeft (70));
    releaseSlider.setBounds (ampSliderArea.removeFromLeft (70));
    envArea.removeFromTop (6);
    auto fltLabelArea = envArea.removeFromTop (14);
    filterAttackLabel.setBounds (fltLabelArea.removeFromLeft (56));
    filterDecayLabel.setBounds (fltLabelArea.removeFromLeft (56));
    filterSustainLabel.setBounds (fltLabelArea.removeFromLeft (56));
    filterReleaseLabel.setBounds (fltLabelArea.removeFromLeft (56));
    filterEnvAmountLabel.setBounds (fltLabelArea.removeFromLeft (56));
    auto fltSliderArea = envArea.removeFromTop (70);
    filterAttackSlider.setBounds (fltSliderArea.removeFromLeft (56));
    filterDecaySlider.setBounds (fltSliderArea.removeFromLeft (56));
    filterSustainSlider.setBounds (fltSliderArea.removeFromLeft (56));
    filterReleaseSlider.setBounds (fltSliderArea.removeFromLeft (56));
    filterEnvAmountSlider.setBounds (fltSliderArea.removeFromLeft (56));

    // LFO
    auto lfoArea = synthArea.removeFromLeft (140);
    sectionLfo.setBounds (lfoArea.removeFromTop (22));
    lfoWaveformLabel.setBounds (lfoArea.removeFromTop (14));
    lfoWaveformBox.setBounds (lfoArea.removeFromTop (22).reduced (10, 2));
    lfoArea.removeFromTop (8);
    lfoRateLabel.setBounds (lfoArea.removeFromTop (12)); lfoRateSlider.setBounds (lfoArea.removeFromTop (50));
    lfoDepthLabel.setBounds (lfoArea.removeFromTop (12)); lfoDepthSlider.setBounds (lfoArea.removeFromTop (50));

    // MOD
    auto modArea = synthArea.removeFromLeft (160);
    sectionMod.setBounds (modArea.removeFromTop (22));
    modLfoToPitchLabel.setBounds (modArea.removeFromTop (12)); modLfoToPitchSlider.setBounds (modArea.removeFromTop (50));
    modLfoToCutoffLabel.setBounds (modArea.removeFromTop (12)); modLfoToCutoffSlider.setBounds (modArea.removeFromTop (50));
    modEnvToPitchLabel.setBounds (modArea.removeFromTop (12)); modEnvToPitchSlider.setBounds (modArea.removeFromTop (50));
    modVelToCutoffLabel.setBounds (modArea.removeFromTop (12)); modVelToCutoffSlider.setBounds (modArea.removeFromTop (50));

    // PATCH
    auto patchArea = synthArea;
    sectionPatch.setBounds (patchArea.removeFromTop (22));
    patch1SrcLabel.setBounds (patchArea.removeFromTop (12)); patch1SrcBox.setBounds (patchArea.removeFromTop (22).reduced (5, 0));
    patch1DstLabel.setBounds (patchArea.removeFromTop (12)); patch1DstBox.setBounds (patchArea.removeFromTop (22).reduced (5, 0));
    patch1AmtLabel.setBounds (patchArea.removeFromTop (12)); patch1AmtSlider.setBounds (patchArea.removeFromTop (46).reduced (5, 0));
    patch2SrcLabel.setBounds (patchArea.removeFromTop (12)); patch2SrcBox.setBounds (patchArea.removeFromTop (22).reduced (5, 0));
    patch2DstLabel.setBounds (patchArea.removeFromTop (12)); patch2DstBox.setBounds (patchArea.removeFromTop (22).reduced (5, 0));
    patch2AmtLabel.setBounds (patchArea.removeFromTop (12)); patch2AmtSlider.setBounds (patchArea.removeFromTop (46).reduced (5, 0));
    patch3SrcLabel.setBounds (patchArea.removeFromTop (12)); patch3SrcBox.setBounds (patchArea.removeFromTop (22).reduced (5, 0));
    patch3DstLabel.setBounds (patchArea.removeFromTop (12)); patch3DstBox.setBounds (patchArea.removeFromTop (22).reduced (5, 0));
    patch3AmtLabel.setBounds (patchArea.removeFromTop (12)); patch3AmtSlider.setBounds (patchArea.removeFromTop (46).reduced (5, 0));
    patch4SrcLabel.setBounds (patchArea.removeFromTop (12)); patch4SrcBox.setBounds (patchArea.removeFromTop (22).reduced (5, 0));
    patch4DstLabel.setBounds (patchArea.removeFromTop (12)); patch4DstBox.setBounds (patchArea.removeFromTop (22).reduced (5, 0));
    patch4AmtLabel.setBounds (patchArea.removeFromTop (12)); patch4AmtSlider.setBounds (patchArea.removeFromTop (46).reduced (5, 0));

    // === DRUM MACHINE ===
    sectionDrum.setBounds (drumArea.removeFromTop (22));
    drumVolLabel.setBounds (drumArea.removeFromLeft (80).removeFromTop (12));
    drumVolSlider.setBounds (drumArea.removeFromLeft (80).removeFromTop (60));
    drumDriveLabel.setBounds (drumArea.removeFromLeft (80).removeFromTop (12));
    drumDriveSlider.setBounds (drumArea.removeFromLeft (80).removeFromTop (60));
    drumArea.removeFromLeft (10);

    auto drumRow1 = drumArea.removeFromTop (85);
    kickLvlLabel.setBounds (drumRow1.removeFromLeft (12)); kickLvlSlider.setBounds (drumRow1.removeFromLeft (70));
    kickTuneLabel.setBounds (drumRow1.removeFromLeft (12)); kickTuneSlider.setBounds (drumRow1.removeFromLeft (70));
    kickDecayLabel.setBounds (drumRow1.removeFromLeft (12)); kickDecaySlider.setBounds (drumRow1.removeFromLeft (70));
    kickToneLabel.setBounds (drumRow1.removeFromLeft (12)); kickToneSlider.setBounds (drumRow1.removeFromLeft (70));
    drumRow1.removeFromLeft (10);
    snareLvlLabel.setBounds (drumRow1.removeFromLeft (12)); snareLvlSlider.setBounds (drumRow1.removeFromLeft (70));
    snareTuneLabel.setBounds (drumRow1.removeFromLeft (12)); snareTuneSlider.setBounds (drumRow1.removeFromLeft (70));
    snareDecayLabel.setBounds (drumRow1.removeFromLeft (12)); snareDecaySlider.setBounds (drumRow1.removeFromLeft (70));
    snareToneLabel.setBounds (drumRow1.removeFromLeft (12)); snareToneSlider.setBounds (drumRow1.removeFromLeft (70));
    drumRow1.removeFromLeft (10);
    hatLvlLabel.setBounds (drumRow1.removeFromLeft (12)); hatLvlSlider.setBounds (drumRow1.removeFromLeft (70));
    hatTuneLabel.setBounds (drumRow1.removeFromLeft (12)); hatTuneSlider.setBounds (drumRow1.removeFromLeft (70));
    hatDecayLabel.setBounds (drumRow1.removeFromLeft (12)); hatDecaySlider.setBounds (drumRow1.removeFromLeft (70));
    hatToneLabel.setBounds (drumRow1.removeFromLeft (12)); hatToneSlider.setBounds (drumRow1.removeFromLeft (70));

    drumArea.removeFromTop (6);
    auto drumRow2 = drumArea.removeFromTop (85);
    clapLvlLabel.setBounds (drumRow2.removeFromLeft (12)); clapLvlSlider.setBounds (drumRow2.removeFromLeft (70));
    clapTuneLabel.setBounds (drumRow2.removeFromLeft (12)); clapTuneSlider.setBounds (drumRow2.removeFromLeft (70));
    clapDecayLabel.setBounds (drumRow2.removeFromLeft (12)); clapDecaySlider.setBounds (drumRow2.removeFromLeft (70));
    clapToneLabel.setBounds (drumRow2.removeFromLeft (12)); clapToneSlider.setBounds (drumRow2.removeFromLeft (70));
    drumRow2.removeFromLeft (10);
    tomLvlLabel.setBounds (drumRow2.removeFromLeft (12)); tomLvlSlider.setBounds (drumRow2.removeFromLeft (70));
    tomTuneLabel.setBounds (drumRow2.removeFromLeft (12)); tomTuneSlider.setBounds (drumRow2.removeFromLeft (70));
    tomDecayLabel.setBounds (drumRow2.removeFromLeft (12)); tomDecaySlider.setBounds (drumRow2.removeFromLeft (70));
    tomToneLabel.setBounds (drumRow2.removeFromLeft (12)); tomToneSlider.setBounds (drumRow2.removeFromLeft (70));
    drumRow2.removeFromLeft (10);
    percLvlLabel.setBounds (drumRow2.removeFromLeft (12)); percLvlSlider.setBounds (drumRow2.removeFromLeft (70));
    percTuneLabel.setBounds (drumRow2.removeFromLeft (12)); percTuneSlider.setBounds (drumRow2.removeFromLeft (70));
    percDecayLabel.setBounds (drumRow2.removeFromLeft (12)); percDecaySlider.setBounds (drumRow2.removeFromLeft (70));
    percToneLabel.setBounds (drumRow2.removeFromLeft (12)); percToneSlider.setBounds (drumRow2.removeFromLeft (70));

    // FX
    sectionFx.setBounds (fxArea.removeFromTop (22));
    auto distArea = fxArea.removeFromLeft (120);
    distDriveLabel.setBounds (distArea.removeFromTop (12)); distDriveSlider.setBounds (distArea.removeFromTop (60));
    distMixLabel.setBounds (distArea.removeFromTop (12)); distMixSlider.setBounds (distArea.removeFromTop (60));
    auto chorusArea = fxArea.removeFromLeft (180);
    chorusRateLabel.setBounds (chorusArea.removeFromTop (12)); chorusRateSlider.setBounds (chorusArea.removeFromTop (60));
    chorusDepthLabel.setBounds (chorusArea.removeFromTop (12)); chorusDepthSlider.setBounds (chorusArea.removeFromTop (60));
    chorusMixLabel.setBounds (chorusArea.removeFromTop (12)); chorusMixSlider.setBounds (chorusArea.removeFromTop (60));
    auto delayArea = fxArea.removeFromLeft (180);
    delayTimeLabel.setBounds (delayArea.removeFromTop (12)); delayTimeSlider.setBounds (delayArea.removeFromTop (60));
    delayFeedbackLabel.setBounds (delayArea.removeFromTop (12)); delayFeedbackSlider.setBounds (delayArea.removeFromTop (60));
    delayMixLabel.setBounds (delayArea.removeFromTop (12)); delayMixSlider.setBounds (delayArea.removeFromTop (60));
    auto reverbArea = fxArea.removeFromLeft (120);
    reverbDecayLabel.setBounds (reverbArea.removeFromTop (12)); reverbDecaySlider.setBounds (reverbArea.removeFromTop (60));
    reverbMixLabel.setBounds (reverbArea.removeFromTop (12)); reverbMixSlider.setBounds (reverbArea.removeFromTop (60));
    fxBypassButton.setBounds (fxArea.reduced (30, 40));

    // MASTER
    sectionMaster.setBounds (bottomArea.removeFromTop (22));
    macroMorphLabel.setBounds (bottomArea.removeFromTop (12)); macroMorphSlider.setBounds (bottomArea.removeFromTop (50));
    analogDriftLabel.setBounds (bottomArea.removeFromTop (12)); analogDriftSlider.setBounds (bottomArea.removeFromTop (50));
    masterDriveLabel.setBounds (bottomArea.removeFromTop (12)); masterDriveSlider.setBounds (bottomArea.removeFromTop (50));
    volumeLabel.setBounds (bottomArea.removeFromTop (12)); volumeSlider.setBounds (bottomArea.removeFromTop (50));
}
