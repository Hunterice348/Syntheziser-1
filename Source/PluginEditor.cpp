void HuntericeAbyssAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (10).withTrimmedTop (36);
    auto topArea = area.removeFromTop (320);
    auto fxArea = area.removeFromTop (180);
    auto bottomArea = area;
    
    // --- OSCILLATORS ---
    auto oscArea = topArea.removeFromLeft (180);
    sectionOsc.setBounds (oscArea.removeFromTop (22));
    waveform1Label.setBounds (oscArea.removeFromTop (14));
    waveform1Box.setBounds (oscArea.removeFromTop (22).reduced (10, 2));
    waveform2Label.setBounds (oscArea.removeFromTop (14));
    waveform2Box.setBounds (oscArea.removeFromTop (22).reduced (10, 2));
    oscArea.removeFromTop (6);
    osc2LevelLabel.setBounds (oscArea.removeFromTop (12));
    osc2LevelSlider.setBounds (oscArea.removeFromTop (50));
    osc2DetuneLabel.setBounds (oscArea.removeFromTop (12));
    osc2DetuneSlider.setBounds (oscArea.removeFromTop (50));
    subLevelLabel.setBounds (oscArea.removeFromTop (12));
    subLevelSlider.setBounds (oscArea.removeFromTop (50));
    noiseLevelLabel.setBounds (oscArea.removeFromTop (12));
    noiseLevelSlider.setBounds (oscArea.removeFromTop (50));
    unisonSpreadLabel.setBounds (oscArea.removeFromTop (12));
    unisonSpreadSlider.setBounds (oscArea.removeFromTop (50));
    
    // --- FILTER ---
    auto filterArea = topArea.removeFromLeft (180);
    sectionFilter.setBounds (filterArea.removeFromTop (22));
    filterTypeLabel.setBounds (filterArea.removeFromTop (14));
    filterTypeBox.setBounds (filterArea.removeFromTop (22).reduced (10, 2));
    filterArea.removeFromTop (8);
    filterCutoffLabel.setBounds (filterArea.removeFromTop (12));
    filterCutoffSlider.setBounds (filterArea.removeFromTop (50));
    filterResonanceLabel.setBounds (filterArea.removeFromTop (12));
    filterResonanceSlider.setBounds (filterArea.removeFromTop (50));
    filterDriveLabel.setBounds (filterArea.removeFromTop (12));
    filterDriveSlider.setBounds (filterArea.removeFromTop (50));
    filterKeyTrackLabel.setBounds (filterArea.removeFromTop (12));
    filterKeyTrackSlider.setBounds (filterArea.removeFromTop (50));
    
    // --- ENVELOPE ---
    auto envArea = topArea.removeFromLeft (280);
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
    
    // --- LFO ---
    auto lfoArea = topArea.removeFromLeft (140);
    sectionLfo.setBounds (lfoArea.removeFromTop (22));
    lfoWaveformLabel.setBounds (lfoArea.removeFromTop (14));
    lfoWaveformBox.setBounds (lfoArea.removeFromTop (22).reduced (10, 2));
    lfoArea.removeFromTop (8);
    lfoRateLabel.setBounds (lfoArea.removeFromTop (12));
    lfoRateSlider.setBounds (lfoArea.removeFromTop (50));
    lfoDepthLabel.setBounds (lfoArea.removeFromTop (12));
    lfoDepthSlider.setBounds (lfoArea.removeFromTop (50));
    
    // --- MODULATION ---
    auto modArea = topArea.removeFromLeft (160);
    sectionMod.setBounds (modArea.removeFromTop (22));
    modLfoToPitchLabel.setBounds (modArea.removeFromTop (12));
    modLfoToPitchSlider.setBounds (modArea.removeFromTop (50));
    modLfoToCutoffLabel.setBounds (modArea.removeFromTop (12));
    modLfoToCutoffSlider.setBounds (modArea.removeFromTop (50));
    modEnvToPitchLabel.setBounds (modArea.removeFromTop (12));
    modEnvToPitchSlider.setBounds (modArea.removeFromTop (50));
    modVelToCutoffLabel.setBounds (modArea.removeFromTop (12));
    modVelToCutoffSlider.setBounds (modArea.removeFromTop (50));
    
    // --- PATCH BAY ---
    auto patchArea = topArea;
    sectionPatch.setBounds (patchArea.removeFromTop (22));
    auto rowH = 50;
    
    patch1SrcLabel.setBounds (patchArea.removeFromTop (12));
    patch1SrcBox.setBounds (patchArea.removeFromTop (22).reduced (5, 0));
    patch1DstLabel.setBounds (patchArea.removeFromTop (12));
    patch1DstBox.setBounds (patchArea.removeFromTop (22).reduced (5, 0));
    patch1AmtLabel.setBounds (patchArea.removeFromTop (12));
    patch1AmtSlider.setBounds (patchArea.removeFromTop (rowH).reduced (5, 0));
    
    patch2SrcLabel.setBounds (patchArea.removeFromTop (12));
    patch2SrcBox.setBounds (patchArea.removeFromTop (22).reduced (5, 0));
    patch2DstLabel.setBounds (patchArea.removeFromTop (12));
    patch2DstBox.setBounds (patchArea.removeFromTop (22).reduced (5, 0));
    patch2AmtLabel.setBounds (patchArea.removeFromTop (12));
    patch2AmtSlider.setBounds (patchArea.removeFromTop (rowH).reduced (5, 0));
    
    patch3SrcLabel.setBounds (patchArea.removeFromTop (12));
    patch3SrcBox.setBounds (patchArea.removeFromTop (22).reduced (5, 0));
    patch3DstLabel.setBounds (patchArea.removeFromTop (12));
    patch3DstBox.setBounds (patchArea.removeFromTop (22).reduced (5, 0));
    patch3AmtLabel.setBounds (patchArea.removeFromTop (12));
    patch3AmtSlider.setBounds (patchArea.removeFromTop (rowH).reduced (5, 0));
    
    patch4SrcLabel.setBounds (patchArea.removeFromTop (12));
    patch4SrcBox.setBounds (patchArea.removeFromTop (22).reduced (5, 0));
    patch4DstLabel.setBounds (patchArea.removeFromTop (12));
    patch4DstBox.setBounds (patchArea.removeFromTop (22).reduced (5, 0));
    patch4AmtLabel.setBounds (patchArea.removeFromTop (12));
    patch4AmtSlider.setBounds (patchArea.removeFromTop (rowH).reduced (5, 0));
    
    // --- EFFECTS ---
    sectionFx.setBounds (fxArea.removeFromTop (22));
    
    auto distArea = fxArea.removeFromLeft (120);
    distDriveLabel.setBounds (distArea.removeFromTop (12));
    distDriveSlider.setBounds (distArea.removeFromTop (60));
    distMixLabel.setBounds (distArea.removeFromTop (12));
    distMixSlider.setBounds (distArea.removeFromTop (60));
    
    auto chorusArea = fxArea.removeFromLeft (180);
    chorusRateLabel.setBounds (chorusArea.removeFromTop (12));
    chorusRateSlider.setBounds (chorusArea.removeFromTop (60));
    chorusDepthLabel.setBounds (chorusArea.removeFromTop (12));
    chorusDepthSlider.setBounds (chorusArea.removeFromTop (60));
    chorusMixLabel.setBounds (chorusArea.removeFromTop (12));
    chorusMixSlider.setBounds (chorusArea.removeFromTop (60));
    
    auto delayArea = fxArea.removeFromLeft (180);
    delayTimeLabel.setBounds (delayArea.removeFromTop (12));
    delayTimeSlider.setBounds (delayArea.removeFromTop (60));
    delayFeedbackLabel.setBounds (delayArea.removeFromTop (12));
    delayFeedbackSlider.setBounds (delayArea.removeFromTop (60));
    delayMixLabel.setBounds (delayArea.removeFromTop (12));
    delayMixSlider.setBounds (delayArea.removeFromTop (60));
    
    auto reverbArea = fxArea.removeFromLeft (120);
    reverbDecayLabel.setBounds (reverbArea.removeFromTop (12));
    reverbDecaySlider.setBounds (reverbArea.removeFromTop (60));
    reverbMixLabel.setBounds (reverbArea.removeFromTop (12));
    reverbMixSlider.setBounds (reverbArea.removeFromTop (60));
    
    fxBypassButton.setBounds (fxArea.reduced (30, 40));
    
    // --- MASTER ---
    sectionMaster.setBounds (bottomArea.removeFromTop (22));
    macroMorphLabel.setBounds (bottomArea.removeFromTop (12));
    macroMorphSlider.setBounds (bottomArea.removeFromTop (50));
    analogDriftLabel.setBounds (bottomArea.removeFromTop (12));
    analogDriftSlider.setBounds (bottomArea.removeFromTop (50));
    masterDriveLabel.setBounds (bottomArea.removeFromTop (12));
    masterDriveSlider.setBounds (bottomArea.removeFromTop (50));
    volumeLabel.setBounds (bottomArea.removeFromTop (12));
    volumeSlider.setBounds (bottomArea.removeFromTop (50));
}
