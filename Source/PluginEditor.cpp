#include "PluginProcessor.h"
#include "PluginEditor.h"

HuntericeAbyssAudioProcessorEditor::HuntericeAbyssAudioProcessorEditor (HuntericeAbyssAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), parameters (vts)
{
    setSize (1050, 480);

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

    // Modulation Matrix Sliders (Stage 9)
    setupSlider (modLfoToPitchSlider); addAndMakeVisible (modLfoToPitchSlider);
    setupSlider (modLfoToCutoffSlider); addAndMakeVisible (modLfoToCutoffSlider);
    setupSlider (modEnvToPitchSlider); addAndMakeVisible (modEnvToPitchSlider);
    setupSlider (modVelToCutoffSlider); addAndMakeVisible (modVelToCutoffSlider);

    waveform1Box.addItemList ({ "Sine", "Saw", "Square", "Triangle" }, 1);
    waveform2Box.addItemList ({ "Sine", "Saw", "Square", "Triangle" }, 1);
    filterTypeBox.addItemList ({ "LP 12dB", "LP 24dB", "High Pass", "Band Pass" }, 1);
    lfoWaveformBox.addItemList ({ "Sine", "Triangle", "Saw", "Square", "S&H" }, 1);
    
    addAndMakeVisible (waveform1Box);
    addAndMakeVisible (waveform2Box);
    addAndMakeVisible (filterTypeBox);
    addAndMakeVisible (lfoWaveformBox);

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
}

void HuntericeAbyssAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (20).removeFromTop (320);
    
    auto oscArea = area.removeFromLeft (160);
    waveform1Box.setBounds (oscArea.removeFromTop (30).reduced (10));
    waveform2Box.setBounds (oscArea.removeFromTop (30).reduced (10));
    osc2LevelSlider.setBounds (oscArea.removeFromLeft (40));
    osc2DetuneSlider.setBounds (oscArea.removeFromLeft (40));
    subLevelSlider.setBounds (oscArea.removeFromLeft (40));
    noiseLevelSlider.setBounds (oscArea.removeFromLeft (40));
    
    auto filterArea = area.removeFromLeft (180);
    filterTypeBox.setBounds (filterArea.removeFromTop (30).reduced (10));
    filterCutoffSlider.setBounds (filterArea.removeFromLeft (45));
    filterResonanceSlider.setBounds (filterArea.removeFromLeft (45));
    filterDriveSlider.setBounds (filterArea.removeFromLeft (45));
    filterKeyTrackSlider.setBounds (filterArea.removeFromLeft (45));
    
    auto modArea = area.removeFromLeft (160);
    lfoWaveformBox.setBounds (modArea.removeFromTop (30).reduced (10));
    lfoRateSlider.setBounds (modArea.removeFromLeft (40));
    lfoDepthSlider.setBounds (modArea.removeFromLeft (40));
    filterEnvAmountSlider.setBounds (modArea.removeFromLeft (40));
    filterAttackSlider.setBounds (modArea.removeFromLeft (40));
    
    // MODULATION MATRIX SECTION (Stage 9)
    auto matrixArea = area.removeFromLeft (160);
    matrixArea.removeFromTop (10); // Spacer
    modLfoToPitchSlider.setBounds (matrixArea.removeFromLeft (40));
    modLfoToCutoffSlider.setBounds (matrixArea.removeFromLeft (40));
    modEnvToPitchSlider.setBounds (matrixArea.removeFromLeft (40));
    modVelToCutoffSlider.setBounds (matrixArea.removeFromLeft (40));
    
    auto envArea = area.removeFromLeft (200);
    attackSlider.setBounds (envArea.removeFromLeft (50));
    decaySlider.setBounds (envArea.removeFromLeft (50));
    sustainSlider.setBounds (envArea.removeFromLeft (50));
    releaseSlider.setBounds (envArea.removeFromLeft (50));
    filterDecaySlider.setBounds (envArea.removeFromLeft (50)); // Compact layout
    
    auto masterArea = area.removeFromLeft (120);
    analogDriftSlider.setBounds (masterArea.removeFromLeft (40));
    masterDriveSlider.setBounds (masterArea.removeFromLeft (40));
    volumeSlider.setBounds (masterArea.removeFromLeft (40));
}
