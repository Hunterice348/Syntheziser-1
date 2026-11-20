#include "PluginProcessor.h"
#include "PluginEditor.h"

HuntericeAbyssAudioProcessorEditor::HuntericeAbyssAudioProcessorEditor (HuntericeAbyssAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), parameters (vts)
{
    setSize (600, 400);
    setLookAndFeel (&juce::LookAndFeel::getDefaultLookAndFeel());

    auto setupSlider = [](juce::Slider& s) {
        s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
    };

    setupSlider (attackSlider);
    setupSlider (decaySlider);
    setupSlider (sustainSlider);
    setupSlider (releaseSlider);
    setupSlider (volumeSlider);

    waveformBox.addItemList ({ "Sine", "Saw", "Square", "Triangle" }, 1);
    addAndMakeVisible (waveformBox);

    addAndMakeVisible (attackSlider);
    addAndMakeVisible (decaySlider);
    addAndMakeVisible (sustainSlider);
    addAndMakeVisible (releaseSlider);
    addAndMakeVisible (volumeSlider);

    attackAttachment   = std::make_unique<Attachment> (parameters, "attack",   attackSlider);
    decayAttachment    = std::make_unique<Attachment> (parameters, "decay",    decaySlider);
    sustainAttachment  = std::make_unique<Attachment> (parameters, "sustain",  sustainSlider);
    releaseAttachment  = std::make_unique<Attachment> (parameters, "release",  releaseSlider);
    volumeAttachment   = std::make_unique<Attachment> (parameters, "volume",   volumeSlider);
    waveformAttachment = std::make_unique<ComboBoxAttachment> (parameters, "waveform", waveformBox);
}

HuntericeAbyssAudioProcessorEditor::~HuntericeAbyssAudioProcessorEditor()
{
}

void HuntericeAbyssAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xFF1E1E24)); // Тёмный фон HUNTERICE

    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (24.0f, juce::Font::bold));
    g.drawText ("HUNTERICE ABYSS", getLocalBounds().removeFromTop (40), juce::Justification::centred);

    g.setColour (juce::Colour (0xFFB41E28)); // Тёмно-красный акцент
    g.fillRect (0, 40, getWidth(), 2);
}

void HuntericeAbyssAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (20).removeFromTop (250);
    
    auto oscArea = area.removeFromLeft (150);
    waveformBox.setBounds (oscArea.removeFromTop (30).reduced (10));
    
    auto envArea = area.removeFromLeft (300);
    attackSlider.setBounds (envArea.removeFromLeft (70));
    decaySlider.setBounds (envArea.removeFromLeft (70));
    sustainSlider.setBounds (envArea.removeFromLeft (70));
    releaseSlider.setBounds (envArea.removeFromLeft (70));
    
    volumeSlider.setBounds (area.removeFromLeft (80));
}
