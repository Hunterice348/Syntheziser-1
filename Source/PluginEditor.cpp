#include "PluginProcessor.h"
#include "PluginEditor.h"

HuntericeAbyssAudioProcessorEditor::HuntericeAbyssAudioProcessorEditor (HuntericeAbyssAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), parameters (vts)
{
    setSize (850, 420);

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

    waveform1Box.addItemList ({ "Sine", "Saw", "Square", "Triangle" }, 1);
    waveform2Box.addItemList ({ "Sine", "Saw", "Square", "Triangle" }, 1);
    addAndMakeVisible (waveform1Box);
    addAndMakeVisible (waveform2Box);

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
    
    wf1Att = std::make_unique<ComboBoxAttachment> (parameters, "waveform1", waveform1Box);
    wf2Att = std::make_unique<ComboBoxAttachment> (parameters, "waveform2", waveform2Box);
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
    auto area = getLocalBounds().reduced (20).removeFromTop (250);
    
    auto oscArea = area.removeFromLeft (200);
    waveform1Box.setBounds (oscArea.removeFromTop (30).reduced (10));
    waveform2Box.setBounds (oscArea.removeFromTop (30).reduced (10));
    osc2LevelSlider.setBounds (oscArea.removeFromLeft (50));
    osc2DetuneSlider.setBounds (oscArea.removeFromLeft (50));
    subLevelSlider.setBounds (oscArea.removeFromLeft (50));
    noiseLevelSlider.setBounds (oscArea.removeFromLeft (50));
    
    auto envArea = area.removeFromLeft (300);
    attackSlider.setBounds (envArea.removeFromLeft (70));
    decaySlider.setBounds (envArea.removeFromLeft (70));
    sustainSlider.setBounds (envArea.removeFromLeft (70));
    releaseSlider.setBounds (envArea.removeFromLeft (70));
    
    auto analogArea = area.removeFromLeft (120);
    analogDriftSlider.setBounds (analogArea.removeFromLeft (60));
    masterDriveSlider.setBounds (analogArea.removeFromLeft (60));
    
    volumeSlider.setBounds (area.removeFromLeft (80));
}
