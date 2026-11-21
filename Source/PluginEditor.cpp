#include "PluginProcessor.h"
#include "PluginEditor.h"

AbyssLookAndFeel::AbyssLookAndFeel()
{
    setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xFFB41E28));
    setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xFF3A3A44));
    setColour (juce::Slider::thumbColourId, juce::Colours::white);
    setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0xFF1E1E24));
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xFF2A2A32));
    setColour (juce::ComboBox::textColourId, juce::Colours::white);
    setColour (juce::ComboBox::arrowColourId, juce::Colour (0xFFB41E28));
    setColour (juce::ToggleButton::textColourId, juce::Colours::white);
    setColour (juce::ToggleButton::tickColourId, juce::Colour (0xFFB41E28));
    setColour (juce::ToggleButton::tickDisabledColourId, juce::Colour (0xFF5A5A64));
}

void AbyssLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                         juce::Slider&)
{
    auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat();
    auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto centre = bounds.getCentre();

    g.setColour (juce::Colour (0x50000000));
    g.fillEllipse (bounds.reduced (3).translated (1.5f, 2.0f));

    juce::ColourGradient grad (
        juce::Colour (0xFF6A6A74), centre.x - radius * 0.7f, centre.y - radius * 0.7f,
        juce::Colour (0xFF1A1A22), centre.x + radius * 0.7f, centre.y + radius * 0.7f,
        true);
    grad.addColour (0.5, juce::Colour (0xFF3A3A44));
    g.setGradientFill (grad);
    g.fillEllipse (bounds.reduced (5));

    g.setColour (juce::Colour (0xFF0A0A12));
    g.drawEllipse (bounds.reduced (5), 1.5f);

    g.setColour (juce::Colour (0x30FFFFFF));
    g.fillEllipse (bounds.reduced (10).withTrimmedBottom (radius * 0.6f));

    float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    juce::Path arc;
    arc.addCentredArc (centre.x, centre.y, radius - 3, radius - 3,
                       0.0f, rotaryStartAngle, angle, true);
    g.setColour (juce::Colour (0xFFB41E28));
    g.strokePath (arc, juce::PathStrokeType (2.5f));

    juce::Point<float> indicatorPoint (
        centre.x + std::cos (angle) * (radius - 12),
        centre.y + std::sin (angle) * (radius - 12));
    g.setColour (juce::Colours::white);
    g.fillEllipse (juce::Rectangle<float> (5.0f, 5.0f).withCentre (indicatorPoint));
}

void AbyssLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                     int, int, int, int, juce::ComboBox&)
{
    juce::Rectangle<int> boxBounds (0, 0, width, height);
    juce::ColourGradient grad (
        juce::Colour (0xFF3A3A44), 0.0f, 0.0f,
        juce::Colour (0xFF1A1A22), 0.0f, (float) height,
        false);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (boxBounds.toFloat(), 4.0f);
    g.setColour (juce::Colour (0xFF5A5A64));
    g.drawRoundedRectangle (boxBounds.toFloat().reduced (0.5f), 4.0f, 1.0f);
    juce::Path arrow;
    auto arrowX = (float) width - 14.0f;
    auto arrowY = (float) height * 0.5f;
    arrow.addTriangle (arrowX - 4.0f, arrowY - 3.0f,
                       arrowX + 4.0f, arrowY - 3.0f,
                       arrowX, arrowY + 3.0f);
    g.setColour (juce::Colour (0xFFB41E28));
    g.fillPath (arrow);
}

HuntericeAbyssAudioProcessorEditor::HuntericeAbyssAudioProcessorEditor (HuntericeAbyssAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), parameters (vts)
{
    setLookAndFeel (&abyssLF);
    
    scaleFactor = detectOptimalScale();
    setSize ((int) (BASE_WIDTH * scaleFactor), (int) (BASE_HEIGHT * scaleFactor));
    setResizable (true, true);
    getConstrainer()->setMinimumSize ((int)(BASE_WIDTH * MIN_SCALE), (int)(BASE_HEIGHT * MIN_SCALE));

    auto setupSlider = [this](juce::Slider& s) {
        s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 18);
        s.setLookAndFeel (&abyssLF);
        addAndMakeVisible (s);
    };

    auto setupLabel = [](juce::Label& l, const juce::String& text, int fontSize = 11) {
        l.setText (text, juce::dontSendNotification);
        l.setJustificationType (juce::Justification::centred);
        l.setColour (juce::Label::textColourId, juce::Colours::white);
        l.setFont (juce::Font ((float) fontSize, juce::Font::bold));
    };

    auto setupSectionLabel = [](juce::Label& l, const juce::String& text) {
        l.setText (text, juce::dontSendNotification);
        l.setJustificationType (juce::Justification::centred);
        l.setColour (juce::Label::textColourId, juce::Colours::white);
        l.setFont (juce::Font (14.0f, juce::Font::bold));
    };

    auto setupCombo = [this](juce::ComboBox& c) {
        c.setLookAndFeel (&abyssLF);
        addAndMakeVisible (c);
    };

    setupSlider (osc2LevelSlider); setupSlider (osc2DetuneSlider);
    setupSlider (subLevelSlider); setupSlider (noiseLevelSlider);
    setupSlider (unisonSpreadSlider);
    setupLabel (osc2LevelLabel, "OSC2 LVL"); addAndMakeVisible (osc2LevelLabel);
    setupLabel (osc2DetuneLabel, "OSC2 DET"); addAndMakeVisible (osc2DetuneLabel);
    setupLabel (subLevelLabel, "SUB LVL"); addAndMakeVisible (subLevelLabel);
    setupLabel (noiseLevelLabel, "NOISE"); addAndMakeVisible (noiseLevelLabel);
    setupLabel (unisonSpreadLabel, "UNISON"); addAndMakeVisible (unisonSpreadLabel);

    waveform1Box.addItemList ({ "Sine", "Saw", "Square", "Triangle" }, 1);
    waveform2Box.addItemList ({ "Sine", "Saw", "Square", "Triangle" }, 1);
    setupCombo (waveform1Box); setupCombo (waveform2Box);
    setupLabel (waveform1Label, "OSC 1"); addAndMakeVisible (waveform1Label);
    setupLabel (waveform2Label, "OSC 2"); addAndMakeVisible (waveform2Label);

    setupSlider (filterCutoffSlider); setupSlider (filterResonanceSlider);
    setupSlider (filterDriveSlider); setupSlider (filterKeyTrackSlider);
    setupLabel (filterCutoffLabel, "CUTOFF"); addAndMakeVisible (filterCutoffLabel);
    setupLabel (filterResonanceLabel, "RESO"); addAndMakeVisible (filterResonanceLabel);
    setupLabel (filterDriveLabel, "DRV"); addAndMakeVisible (filterDriveLabel);
    setupLabel (filterKeyTrackLabel, "KEY TRK"); addAndMakeVisible (filterKeyTrackLabel);

    filterTypeBox.addItemList ({ "LP 12dB", "LP 24dB", "High Pass", "Band Pass" }, 1);
    setupCombo (filterTypeBox);
    setupLabel (filterTypeLabel, "FILTER TYPE"); addAndMakeVisible (filterTypeLabel);

    setupSlider (attackSlider); setupSlider (decaySlider);
    setupSlider (sustainSlider); setupSlider (releaseSlider);
    setupSlider (filterAttackSlider); setupSlider (filterDecaySlider);
    setupSlider (filterSustainSlider); setupSlider (filterReleaseSlider);
    setupSlider (filterEnvAmountSlider);
    setupLabel (attackLabel, "A"); addAndMakeVisible (attackLabel);
    setupLabel (decayLabel, "D"); addAndMakeVisible (decayLabel);
    setupLabel (sustainLabel, "S"); addAndMakeVisible (sustainLabel);
    setupLabel (releaseLabel, "R"); addAndMakeVisible (releaseLabel);
    setupLabel (filterAttackLabel, "F-A"); addAndMakeVisible (filterAttackLabel);
    setupLabel (filterDecayLabel, "F-D"); addAndMakeVisible (filterDecayLabel);
    setupLabel (filterSustainLabel, "F-S"); addAndMakeVisible (filterSustainLabel);
    setupLabel (filterReleaseLabel, "F-R"); addAndMakeVisible (filterReleaseLabel);
    setupLabel (filterEnvAmountLabel, "ENV AMT"); addAndMakeVisible (filterEnvAmountLabel);

    setupSlider (lfoRateSlider); setupSlider (lfoDepthSlider);
    setupLabel (lfoRateLabel, "RATE"); addAndMakeVisible (lfoRateLabel);
    setupLabel (lfoDepthLabel, "DEPTH"); addAndMakeVisible (lfoDepthLabel);

    lfoWaveformBox.addItemList ({ "Sine", "Triangle", "Saw", "Square", "S&H" }, 1);
    setupCombo (lfoWaveformBox);
    setupLabel (lfoWaveformLabel, "LFO WAVE"); addAndMakeVisible (lfoWaveformLabel);

    setupSlider (modLfoToPitchSlider); setupSlider (modLfoToCutoffSlider);
    setupSlider (modEnvToPitchSlider); setupSlider (modVelToCutoffSlider);
    setupLabel (modLfoToPitchLabel, "LFO>PIT"); addAndMakeVisible (modLfoToPitchLabel);
    setupLabel (modLfoToCutoffLabel, "LFO>CUT"); addAndMakeVisible (modLfoToCutoffLabel);
    setupLabel (modEnvToPitchLabel, "ENV>PIT"); addAndMakeVisible (modEnvToPitchLabel);
    setupLabel (modVelToCutoffLabel, "VEL>CUT"); addAndMakeVisible (modVelToCutoffLabel);

    setupCombo (patch1SrcBox); setupCombo (patch1DstBox); setupSlider (patch1AmtSlider);
    setupCombo (patch2SrcBox); setupCombo (patch2DstBox); setupSlider (patch2AmtSlider);
    setupCombo (patch3SrcBox); setupCombo (patch3DstBox); setupSlider (patch3AmtSlider);
    setupCombo (patch4SrcBox); setupCombo (patch4DstBox); setupSlider (patch4AmtSlider);

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

    setupSlider (distDriveSlider); setupSlider (distMixSlider);
    setupSlider (chorusRateSlider); setupSlider (chorusDepthSlider); setupSlider (chorusMixSlider);
    setupSlider (delayTimeSlider); setupSlider (delayFeedbackSlider); setupSlider (delayMixSlider);
    setupSlider (reverbDecaySlider); setupSlider (reverbMixSlider);

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
    addAndMakeVisible (fxBypassButton);

    setupSlider (analogDriftSlider); setupSlider (masterDriveSlider);
    setupSlider (volumeSlider); setupSlider (macroMorphSlider);
    setupLabel (analogDriftLabel, "DRIFT"); addAndMakeVisible (analogDriftLabel);
    setupLabel (masterDriveLabel, "DRV"); addAndMakeVisible (masterDriveLabel);
    setupLabel (volumeLabel, "VOLUME"); addAndMakeVisible (volumeLabel);
    setupLabel (macroMorphLabel, "CLEAN < > DIRTY"); addAndMakeVisible (macroMorphLabel);

    setupSlider (drumVolSlider); setupSlider (drumDriveSlider);
    setupLabel (drumVolLabel, "DRUM VOL"); addAndMakeVisible (drumVolLabel);
    setupLabel (drumDriveLabel, "DRUM DRV"); addAndMakeVisible (drumDriveLabel);

    setupSlider (kickLvlSlider); setupSlider (kickTuneSlider); setupSlider (kickDecaySlider); setupSlider (kickToneSlider);
    setupSlider (snareLvlSlider); setupSlider (snareTuneSlider); setupSlider (snareDecaySlider); setupSlider (snareToneSlider);
    setupSlider (hatLvlSlider); setupSlider (hatTuneSlider); setupSlider (hatDecaySlider); setupSlider (hatToneSlider);
    setupSlider (clapLvlSlider); setupSlider (clapTuneSlider); setupSlider (clapDecaySlider); setupSlider (clapToneSlider);
    setupSlider (tomLvlSlider); setupSlider (tomTuneSlider); setupSlider (tomDecaySlider); setupSlider (tomToneSlider);
    setupSlider (percLvlSlider); setupSlider (percTuneSlider); setupSlider (percDecaySlider); setupSlider (percToneSlider);

    setupLabel (kickLvlLabel, "KICK LVL"); addAndMakeVisible (kickLvlLabel);
    setupLabel (kickTuneLabel, "KICK TUNE"); addAndMakeVisible (kickTuneLabel);
    setupLabel (kickDecayLabel, "KICK DEC"); addAndMakeVisible (kickDecayLabel);
    setupLabel (kickToneLabel, "KICK TONE"); addAndMakeVisible (kickToneLabel);
    setupLabel (snareLvlLabel, "SNARE LVL"); addAndMakeVisible (snareLvlLabel);
    setupLabel (snareTuneLabel, "SNARE TUNE"); addAndMakeVisible (snareTuneLabel);
    setupLabel (snareDecayLabel, "SNARE DEC"); addAndMakeVisible (snareDecayLabel);
    setupLabel (snareToneLabel, "SNARE TONE"); addAndMakeVisible (snareToneLabel);
    setupLabel (hatLvlLabel, "HAT LVL"); addAndMakeVisible (hatLvlLabel);
    setupLabel (hatTuneLabel, "HAT TUNE"); addAndMakeVisible (hatTuneLabel);
    setupLabel (hatDecayLabel, "HAT DEC"); addAndMakeVisible (hatDecayLabel);
    setupLabel (hatToneLabel, "HAT TONE"); addAndMakeVisible (hatToneLabel);
    setupLabel (clapLvlLabel, "CLAP LVL"); addAndMakeVisible (clapLvlLabel);
    setupLabel (clapTuneLabel, "CLAP TUNE"); addAndMakeVisible (clapTuneLabel);
    setupLabel (clapDecayLabel, "CLAP DEC"); addAndMakeVisible (clapDecayLabel);
    setupLabel (clapToneLabel, "CLAP TONE"); addAndMakeVisible (clapToneLabel);
    setupLabel (tomLvlLabel, "TOM LVL"); addAndMakeVisible (tomLvlLabel);
    setupLabel (tomTuneLabel, "TOM TUNE"); addAndMakeVisible (tomTuneLabel);
    setupLabel (tomDecayLabel, "TOM DEC"); addAndMakeVisible (tomDecayLabel);
    setupLabel (tomToneLabel, "TOM TONE"); addAndMakeVisible (tomToneLabel);
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

HuntericeAbyssAudioProcessorEditor::~HuntericeAbyssAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

float HuntericeAbyssAudioProcessorEditor::detectOptimalScale()
{
    auto display = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay();
    if (display == nullptr) return 1.0f;
    
    float dpi = display->dpi;
    if (dpi < 120.0f) return 1.0f;
    if (dpi < 160.0f) return 1.25f;
    if (dpi < 200.0f) return 1.5f;
    return 2.0f;
}

void HuntericeAbyssAudioProcessorEditor::applyScale (float newScale)
{
    newScale = juce::jlimit (MIN_SCALE, MAX_SCALE, newScale);
    if (std::abs (newScale - scaleFactor) < 0.001f) return;
    
    scaleFactor = newScale;
    
    for (auto* s : {&attackSlider, &decaySlider, &sustainSlider, &releaseSlider, &volumeSlider,
                    &osc2LevelSlider, &osc2DetuneSlider, &subLevelSlider, &noiseLevelSlider,
                    &analogDriftSlider, &masterDriveSlider,
                    &filterCutoffSlider, &filterResonanceSlider, &filterDriveSlider, &filterKeyTrackSlider,
                    &filterAttackSlider, &filterDecaySlider, &filterSustainSlider, &filterReleaseSlider, &filterEnvAmountSlider,
                    &lfoRateSlider, &lfoDepthSlider,
                    &modLfoToPitchSlider, &modLfoToCutoffSlider, &modEnvToPitchSlider, &modVelToCutoffSlider,
                    &patch1AmtSlider, &patch2AmtSlider, &patch3AmtSlider, &patch4AmtSlider,
                    &distDriveSlider, &distMixSlider,
                    &chorusRateSlider, &chorusDepthSlider, &chorusMixSlider,
                    &delayTimeSlider, &delayFeedbackSlider, &delayMixSlider,
                    &reverbDecaySlider, &reverbMixSlider,
                    &unisonSpreadSlider, &macroMorphSlider,
                    &drumVolSlider, &drumDriveSlider,
                    &kickLvlSlider, &kickTuneSlider, &kickDecaySlider, &kickToneSlider,
                    &snareLvlSlider, &snareTuneSlider, &snareDecaySlider, &snareToneSlider,
                    &hatLvlSlider, &hatTuneSlider, &hatDecaySlider, &hatToneSlider,
                    &clapLvlSlider, &clapTuneSlider, &clapDecaySlider, &clapToneSlider,
                    &tomLvlSlider, &tomTuneSlider, &tomDecaySlider, &tomToneSlider,
                    &percLvlSlider, &percTuneSlider, &percDecaySlider, &percToneSlider})
    {
        s->setTextBoxStyle (juce::Slider::TextBoxBelow, false, (int)(60 * scaleFactor), (int)(18 * scaleFactor));
    }
    
    setSize ((int) (BASE_WIDTH * scaleFactor), (int) (BASE_HEIGHT * scaleFactor));
    resized();
}

void HuntericeAbyssAudioProcessorEditor::showScaleMenu()
{
    juce::PopupMenu menu;
    menu.setLookAndFeel (&abyssLF);
    
    auto addItem = [&](float scale, const juce::String& label) {
        menu.addItem (juce::roundToInt (scale * 100), label, true, std::abs (scaleFactor - scale) < 0.001f);
    };
    
    addItem (0.5f,  "50%");
    addItem (0.75f, "75%");
    addItem (1.0f,  "100%  (default)");
    addItem (1.25f, "125%");
    addItem (1.5f,  "150%");
    addItem (2.0f,  "200%");
    
    menu.addSeparator();
    menu.addItem (999, "Auto-detect from screen");
    
    auto chosen = menu.show();
    if (chosen == 999) applyScale (detectOptimalScale());
    else if (chosen > 0) applyScale (chosen / 100.0f);
}

void HuntericeAbyssAudioProcessorEditor::mouseDown (const juce::MouseEvent& e)
{
    if (e.mods.isRightButtonDown() && e.y < 40)
    {
        showScaleMenu();
    }
}

void HuntericeAbyssAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xFF1E1E24));

    auto s = scaleFactor;
    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (22.0f * s, juce::Font::bold));
    g.drawText ("HUNTERICE ABYSS", getLocalBounds().removeFromTop ((int)(32 * s)), juce::Justification::centred);

    g.setColour (juce::Colour (0xFFB41E28));
    g.fillRect (0, (int)(32 * s), getWidth(), 2);

    g.setColour (juce::Colour (0x80FFFFFF));
    g.setFont (juce::Font (10.0f * s, juce::Font::italic));
    juce::String hint = "Right-click title for UI scale (" + juce::String (int (scaleFactor * 100)) + "%)";
    g.drawText (hint, getLocalBounds().removeFromTop ((int)(32 * s)).removeFromRight ((int)(300 * s)).reduced (10, 0),
                juce::Justification::centredRight);

    auto area = getLocalBounds().reduced ((int)(10 * s)).withTrimmedTop ((int)(36 * s));
    auto synthArea = area.removeFromTop ((int)(320 * s));
    auto drumArea = area.removeFromTop ((int)(200 * s));
    auto fxArea = area.removeFromTop ((int)(180 * s));

    auto oscArea = synthArea.removeFromLeft ((int)(180 * s));
    g.setColour (juce::Colour (0xFF2A1E1E)); g.fillRect (oscArea);
    g.setColour (juce::Colour (0xFFB41E28)); g.drawRect (oscArea, 1);

    auto filterArea = synthArea.removeFromLeft ((int)(180 * s));
    g.setColour (juce::Colour (0xFF1E1E2A)); g.fillRect (filterArea);
    g.setColour (juce::Colour (0xFF4169E1)); g.drawRect (filterArea, 1);

    auto envArea = synthArea.removeFromLeft ((int)(280 * s));
    g.setColour (juce::Colour (0xFF1E2A1E)); g.fillRect (envArea);
    g.setColour (juce::Colour (0xFF3CB371)); g.drawRect (envArea, 1);

    auto lfoArea = synthArea.removeFromLeft ((int)(140 * s));
    g.setColour (juce::Colour (0xFF2A2A1E)); g.fillRect (lfoArea);
    g.setColour (juce::Colour (0xFFDAA520)); g.drawRect (lfoArea, 1);

    auto modArea = synthArea.removeFromLeft ((int)(160 * s));
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
    const float s = scaleFactor;
    auto area = getLocalBounds().reduced ((int)(10 * s)).withTrimmedTop ((int)(36 * s));
    auto synthArea = area.removeFromTop ((int)(320 * s));
    auto drumArea = area.removeFromTop ((int)(200 * s));
    auto fxArea = area.removeFromTop ((int)(180 * s));
    auto bottomArea = area;

    auto oscArea = synthArea.removeFromLeft ((int)(180 * s));
    sectionOsc.setBounds (oscArea.removeFromTop ((int)(22 * s)));
    waveform1Label.setBounds (oscArea.removeFromTop ((int)(14 * s)));
    waveform1Box.setBounds (oscArea.removeFromTop ((int)(22 * s)).reduced ((int)(10 * s), (int)(2 * s)));
    waveform2Label.setBounds (oscArea.removeFromTop ((int)(14 * s)));
    waveform2Box.setBounds (oscArea.removeFromTop ((int)(22 * s)).reduced ((int)(10 * s), (int)(2 * s)));
    oscArea.removeFromTop ((int)(4 * s));
    osc2LevelLabel.setBounds (oscArea.removeFromTop ((int)(12 * s))); osc2LevelSlider.setBounds (oscArea.removeFromTop ((int)(46 * s)));
    osc2DetuneLabel.setBounds (oscArea.removeFromTop ((int)(12 * s))); osc2DetuneSlider.setBounds (oscArea.removeFromTop ((int)(46 * s)));
    subLevelLabel.setBounds (oscArea.removeFromTop ((int)(12 * s))); subLevelSlider.setBounds (oscArea.removeFromTop ((int)(46 * s)));
    noiseLevelLabel.setBounds (oscArea.removeFromTop ((int)(12 * s))); noiseLevelSlider.setBounds (oscArea.removeFromTop ((int)(46 * s)));
    unisonSpreadLabel.setBounds (oscArea.removeFromTop ((int)(12 * s))); unisonSpreadSlider.setBounds (oscArea.removeFromTop ((int)(46 * s)));

    auto filterArea = synthArea.removeFromLeft ((int)(180 * s));
    sectionFilter.setBounds (filterArea.removeFromTop ((int)(22 * s)));
    filterTypeLabel.setBounds (filterArea.removeFromTop ((int)(14 * s)));
    filterTypeBox.setBounds (filterArea.removeFromTop ((int)(22 * s)).reduced ((int)(10 * s), (int)(2 * s)));
    filterArea.removeFromTop ((int)(8 * s));
    filterCutoffLabel.setBounds (filterArea.removeFromTop ((int)(12 * s))); filterCutoffSlider.setBounds (filterArea.removeFromTop ((int)(50 * s)));
    filterResonanceLabel.setBounds (filterArea.removeFromTop ((int)(12 * s))); filterResonanceSlider.setBounds (filterArea.removeFromTop ((int)(50 * s)));
    filterDriveLabel.setBounds (filterArea.removeFromTop ((int)(12 * s))); filterDriveSlider.setBounds (filterArea.removeFromTop ((int)(50 * s)));
    filterKeyTrackLabel.setBounds (filterArea.removeFromTop ((int)(12 * s))); filterKeyTrackSlider.setBounds (filterArea.removeFromTop ((int)(50 * s)));

    auto envArea = synthArea.removeFromLeft ((int)(280 * s));
    sectionEnv.setBounds (envArea.removeFromTop ((int)(22 * s)));
    auto ampLabelArea = envArea.removeFromTop ((int)(14 * s));
    attackLabel.setBounds (ampLabelArea.removeFromLeft ((int)(70 * s)));
    decayLabel.setBounds (ampLabelArea.removeFromLeft ((int)(70 * s)));
    sustainLabel.setBounds (ampLabelArea.removeFromLeft ((int)(70 * s)));
    releaseLabel.setBounds (ampLabelArea.removeFromLeft ((int)(70 * s)));
    auto ampSliderArea = envArea.removeFromTop ((int)(70 * s));
    attackSlider.setBounds (ampSliderArea.removeFromLeft ((int)(70 * s)));
    decaySlider.setBounds (ampSliderArea.removeFromLeft ((int)(70 * s)));
    sustainSlider.setBounds (ampSliderArea.removeFromLeft ((int)(70 * s)));
    releaseSlider.setBounds (ampSliderArea.removeFromLeft ((int)(70 * s)));
    envArea.removeFromTop ((int)(6 * s));
    auto fltLabelArea = envArea.removeFromTop ((int)(14 * s));
    filterAttackLabel.setBounds (fltLabelArea.removeFromLeft ((int)(56 * s)));
    filterDecayLabel.setBounds (fltLabelArea.removeFromLeft ((int)(56 * s)));
    filterSustainLabel.setBounds (fltLabelArea.removeFromLeft ((int)(56 * s)));
    filterReleaseLabel.setBounds (fltLabelArea.removeFromLeft ((int)(56 * s)));
    filterEnvAmountLabel.setBounds (fltLabelArea.removeFromLeft ((int)(56 * s)));
    auto fltSliderArea = envArea.removeFromTop ((int)(70 * s));
    filterAttackSlider.setBounds (fltSliderArea.removeFromLeft ((int)(56 * s)));
    filterDecaySlider.setBounds (fltSliderArea.removeFromLeft ((int)(56 * s)));
    filterSustainSlider.setBounds (fltSliderArea.removeFromLeft ((int)(56 * s)));
    filterReleaseSlider.setBounds (fltSliderArea.removeFromLeft ((int)(56 * s)));
    filterEnvAmountSlider.setBounds (fltSliderArea.removeFromLeft ((int)(56 * s)));

    auto lfoArea = synthArea.removeFromLeft ((int)(140 * s));
    sectionLfo.setBounds (lfoArea.removeFromTop ((int)(22 * s)));
    lfoWaveformLabel.setBounds (lfoArea.removeFromTop ((int)(14 * s)));
    lfoWaveformBox.setBounds (lfoArea.removeFromTop ((int)(22 * s)).reduced ((int)(10 * s), (int)(2 * s)));
    lfoArea.removeFromTop ((int)(8 * s));
    lfoRateLabel.setBounds (lfoArea.removeFromTop ((int)(12 * s))); lfoRateSlider.setBounds (lfoArea.removeFromTop ((int)(50 * s)));
    lfoDepthLabel.setBounds (lfoArea.removeFromTop ((int)(12 * s))); lfoDepthSlider.setBounds (lfoArea.removeFromTop ((int)(50 * s)));

    auto modArea = synthArea.removeFromLeft ((int)(160 * s));
    sectionMod.setBounds (modArea.removeFromTop ((int)(22 * s)));
    modLfoToPitchLabel.setBounds (modArea.removeFromTop ((int)(12 * s))); modLfoToPitchSlider.setBounds (modArea.removeFromTop ((int)(50 * s)));
    modLfoToCutoffLabel.setBounds (modArea.removeFromTop ((int)(12 * s))); modLfoToCutoffSlider.setBounds (modArea.removeFromTop ((int)(50 * s)));
    modEnvToPitchLabel.setBounds (modArea.removeFromTop ((int)(12 * s))); modEnvToPitchSlider.setBounds (modArea.removeFromTop ((int)(50 * s)));
    modVelToCutoffLabel.setBounds (modArea.removeFromTop ((int)(12 * s))); modVelToCutoffSlider.setBounds (modArea.removeFromTop ((int)(50 * s)));

    auto patchArea = synthArea;
    sectionPatch.setBounds (patchArea.removeFromTop ((int)(22 * s)));
    patch1SrcLabel.setBounds (patchArea.removeFromTop ((int)(12 * s))); patch1SrcBox.setBounds (patchArea.removeFromTop ((int)(22 * s)).reduced ((int)(5 * s), 0));
    patch1DstLabel.setBounds (patchArea.removeFromTop ((int)(12 * s))); patch1DstBox.setBounds (patchArea.removeFromTop ((int)(22 * s)).reduced ((int)(5 * s), 0));
    patch1AmtLabel.setBounds (patchArea.removeFromTop ((int)(12 * s))); patch1AmtSlider.setBounds (patchArea.removeFromTop ((int)(46 * s)).reduced ((int)(5 * s), 0));
    patch2SrcLabel.setBounds (patchArea.removeFromTop ((int)(12 * s))); patch2SrcBox.setBounds (patchArea.removeFromTop ((int)(22 * s)).reduced ((int)(5 * s), 0));
    patch2DstLabel.setBounds (patchArea.removeFromTop ((int)(12 * s))); patch2DstBox.setBounds (patchArea.removeFromTop ((int)(22 * s)).reduced ((int)(5 * s), 0));
    patch2AmtLabel.setBounds (patchArea.removeFromTop ((int)(12 * s))); patch2AmtSlider.setBounds (patchArea.removeFromTop ((int)(46 * s)).reduced ((int)(5 * s), 0));
    patch3SrcLabel.setBounds (patchArea.removeFromTop ((int)(12 * s))); patch3SrcBox.setBounds (patchArea.removeFromTop ((int)(22 * s)).reduced ((int)(5 * s), 0));
    patch3DstLabel.setBounds (patchArea.removeFromTop ((int)(12 * s))); patch3DstBox.setBounds (patchArea.removeFromTop ((int)(22 * s)).reduced ((int)(5 * s), 0));
    patch3AmtLabel.setBounds (patchArea.removeFromTop ((int)(12 * s))); patch3AmtSlider.setBounds (patchArea.removeFromTop ((int)(46 * s)).reduced ((int)(5 * s), 0));
    patch4SrcLabel.setBounds (patchArea.removeFromTop ((int)(12 * s))); patch4SrcBox.setBounds (patchArea.removeFromTop ((int)(22 * s)).reduced ((int)(5 * s), 0));
    patch4DstLabel.setBounds (patchArea.removeFromTop ((int)(12 * s))); patch4DstBox.setBounds (patchArea.removeFromTop ((int)(22 * s)).reduced ((int)(5 * s), 0));
    patch4AmtLabel.setBounds (patchArea.removeFromTop ((int)(12 * s))); patch4AmtSlider.setBounds (patchArea.removeFromTop ((int)(46 * s)).reduced ((int)(5 * s), 0));

    sectionDrum.setBounds (drumArea.removeFromTop ((int)(22 * s)));
    drumVolLabel.setBounds (drumArea.removeFromLeft ((int)(80 * s)).removeFromTop ((int)(12 * s)));
    drumVolSlider.setBounds (drumArea.removeFromLeft ((int)(80 * s)).removeFromTop ((int)(60 * s)));
    drumDriveLabel.setBounds (drumArea.removeFromLeft ((int)(80 * s)).removeFromTop ((int)(12 * s)));
    drumDriveSlider.setBounds (drumArea.removeFromLeft ((int)(80 * s)).removeFromTop ((int)(60 * s)));
    drumArea.removeFromLeft ((int)(10 * s));

    auto drumRow1 = drumArea.removeFromTop ((int)(85 * s));
    kickLvlLabel.setBounds (drumRow1.removeFromLeft ((int)(12 * s))); kickLvlSlider.setBounds (drumRow1.removeFromLeft ((int)(70 * s)));
    kickTuneLabel.setBounds (drumRow1.removeFromLeft ((int)(12 * s))); kickTuneSlider.setBounds (drumRow1.removeFromLeft ((int)(70 * s)));
    kickDecayLabel.setBounds (drumRow1.removeFromLeft ((int)(12 * s))); kickDecaySlider.setBounds (drumRow1.removeFromLeft ((int)(70 * s)));
    kickToneLabel.setBounds (drumRow1.removeFromLeft ((int)(12 * s))); kickToneSlider.setBounds (drumRow1.removeFromLeft ((int)(70 * s)));
    drumRow1.removeFromLeft ((int)(10 * s));
    snareLvlLabel.setBounds (drumRow1.removeFromLeft ((int)(12 * s))); snareLvlSlider.setBounds (drumRow1.removeFromLeft ((int)(70 * s)));
    snareTuneLabel.setBounds (drumRow1.removeFromLeft ((int)(12 * s))); snareTuneSlider.setBounds (drumRow1.removeFromLeft ((int)(70 * s)));
    snareDecayLabel.setBounds (drumRow1.removeFromLeft ((int)(12 * s))); snareDecaySlider.setBounds (drumRow1.removeFromLeft ((int)(70 * s)));
    snareToneLabel.setBounds (drumRow1.removeFromLeft ((int)(12 * s))); snareToneSlider.setBounds (drumRow1.removeFromLeft ((int)(70 * s)));
    drumRow1.removeFromLeft ((int)(10 * s));
    hatLvlLabel.setBounds (drumRow1.removeFromLeft ((int)(12 * s))); hatLvlSlider.setBounds (drumRow1.removeFromLeft ((int)(70 * s)));
    hatTuneLabel.setBounds (drumRow1.removeFromLeft ((int)(12 * s))); hatTuneSlider.setBounds (drumRow1.removeFromLeft ((int)(70 * s)));
    hatDecayLabel.setBounds (drumRow1.removeFromLeft ((int)(12 * s))); hatDecaySlider.setBounds (drumRow1.removeFromLeft ((int)(70 * s)));
    hatToneLabel.setBounds (drumRow1.removeFromLeft ((int)(12 * s))); hatToneSlider.setBounds (drumRow1.removeFromLeft ((int)(70 * s)));

    drumArea.removeFromTop ((int)(6 * s));
    auto drumRow2 = drumArea.removeFromTop ((int)(85 * s));
    clapLvlLabel.setBounds (drumRow2.removeFromLeft ((int)(12 * s))); clapLvlSlider.setBounds (drumRow2.removeFromLeft ((int)(70 * s)));
    clapTuneLabel.setBounds (drumRow2.removeFromLeft ((int)(12 * s))); clapTuneSlider.setBounds (drumRow2.removeFromLeft ((int)(70 * s)));
    clapDecayLabel.setBounds (drumRow2.removeFromLeft ((int)(12 * s))); clapDecaySlider.setBounds (drumRow2.removeFromLeft ((int)(70 * s)));
    clapToneLabel.setBounds (drumRow2.removeFromLeft ((int)(12 * s))); clapToneSlider.setBounds (drumRow2.removeFromLeft ((int)(70 * s)));
    drumRow2.removeFromLeft ((int)(10 * s));
    tomLvlLabel.setBounds (drumRow2.removeFromLeft ((int)(12 * s))); tomLvlSlider.setBounds (drumRow2.removeFromLeft ((int)(70 * s)));
    tomTuneLabel.setBounds (drumRow2.removeFromLeft ((int)(12 * s))); tomTuneSlider.setBounds (drumRow2.removeFromLeft ((int)(70 * s)));
    tomDecayLabel.setBounds (drumRow2.removeFromLeft ((int)(12 * s))); tomDecaySlider.setBounds (drumRow2.removeFromLeft ((int)(70 * s)));
    tomToneLabel.setBounds (drumRow2.removeFromLeft ((int)(12 * s))); tomToneSlider.setBounds (drumRow2.removeFromLeft ((int)(70 * s)));
    drumRow2.removeFromLeft ((int)(10 * s));
    percLvlLabel.setBounds (drumRow2.removeFromLeft ((int)(12 * s))); percLvlSlider.setBounds (drumRow2.removeFromLeft ((int)(70 * s)));
    percTuneLabel.setBounds (drumRow2.removeFromLeft ((int)(12 * s))); percTuneSlider.setBounds (drumRow2.removeFromLeft ((int)(70 * s)));
    percDecayLabel.setBounds (drumRow2.removeFromLeft ((int)(12 * s))); percDecaySlider.setBounds (drumRow2.removeFromLeft ((int)(70 * s)));
    percToneLabel.setBounds (drumRow2.removeFromLeft ((int)(12 * s))); percToneSlider.setBounds (drumRow2.removeFromLeft ((int)(70 * s)));

    sectionFx.setBounds (fxArea.removeFromTop ((int)(22 * s)));
    auto distArea = fxArea.removeFromLeft ((int)(120 * s));
    distDriveLabel.setBounds (distArea.removeFromTop ((int)(12 * s))); distDriveSlider.setBounds (distArea.removeFromTop ((int)(60 * s)));
    distMixLabel.setBounds (distArea.removeFromTop ((int)(12 * s))); distMixSlider.setBounds (distArea.removeFromTop ((int)(60 * s)));
    auto chorusArea = fxArea.removeFromLeft ((int)(180 * s));
    chorusRateLabel.setBounds (chorusArea.removeFromTop ((int)(12 * s))); chorusRateSlider.setBounds (chorusArea.removeFromTop ((int)(60 * s)));
    chorusDepthLabel.setBounds (chorusArea.removeFromTop ((int)(12 * s))); chorusDepthSlider.setBounds (chorusArea.removeFromTop ((int)(60 * s)));
    chorusMixLabel.setBounds (chorusArea.removeFromTop ((int)(12 * s))); chorusMixSlider.setBounds (chorusArea.removeFromTop ((int)(60 * s)));
    auto delayArea = fxArea.removeFromLeft ((int)(180 * s));
    delayTimeLabel.setBounds (delayArea.removeFromTop ((int)(12 * s))); delayTimeSlider.setBounds (delayArea.removeFromTop ((int)(60 * s)));
    delayFeedbackLabel.setBounds (delayArea.removeFromTop ((int)(12 * s))); delayFeedbackSlider.setBounds (delayArea.removeFromTop ((int)(60 * s)));
    delayMixLabel.setBounds (delayArea.removeFromTop ((int)(12 * s))); delayMixSlider.setBounds (delayArea.removeFromTop ((int)(60 * s)));
    auto reverbArea = fxArea.removeFromLeft ((int)(120 * s));
    reverbDecayLabel.setBounds (reverbArea.removeFromTop ((int)(12 * s))); reverbDecaySlider.setBounds (reverbArea.removeFromTop ((int)(60 * s)));
    reverbMixLabel.setBounds (reverbArea.removeFromTop ((int)(12 * s))); reverbMixSlider.setBounds (reverbArea.removeFromTop ((int)(60 * s)));
    fxBypassButton.setBounds (fxArea.reduced ((int)(30 * s), (int)(40 * s)));

    sectionMaster.setBounds (bottomArea.removeFromTop ((int)(22 * s)));
    macroMorphLabel.setBounds (bottomArea.removeFromTop ((int)(12 * s))); macroMorphSlider.setBounds (bottomArea.removeFromTop ((int)(50 * s)));
    analogDriftLabel.setBounds (bottomArea.removeFromTop ((int)(12 * s))); analogDriftSlider.setBounds (bottomArea.removeFromTop ((int)(50 * s)));
    masterDriveLabel.setBounds (bottomArea.removeFromTop ((int)(12 * s))); masterDriveSlider.setBounds (bottomArea.removeFromTop ((int)(50 * s)));
    volumeLabel.setBounds (bottomArea.removeFromTop ((int)(12 * s))); volumeSlider.setBounds (bottomArea.removeFromTop ((int)(50 * s)));
}
