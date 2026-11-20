#include "PluginProcessor.h"
#include "PluginEditor.h"

HuntericeAbyssAudioProcessorEditor::HuntericeAbyssAudioProcessorEditor (HuntericeAbyssAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (600, 400);
    setResizable(true, true);
    setResizeLimits(400, 300, 1200, 800);

    // Заголовок
    titleLabel.setText("HUNTERICE ABYSS", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);

    // Мастер-громкость
    masterVolSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    masterVolSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    masterVolAttachment.reset(new SliderAttachment(vts, "masterVol", masterVolSlider));
    addAndMakeVisible(masterVolSlider);

    // Слайдеры огибающей ADSR
    attackSlider.setSliderStyle(juce::Slider::LinearVertical);
    attackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 20);
    attackAttachment.reset(new SliderAttachment(vts, "attack", attackSlider));
    addAndMakeVisible(attackSlider);

    decaySlider.setSliderStyle(juce::Slider::LinearVertical);
    decaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 20);
    decayAttachment.reset(new SliderAttachment(vts, "decay", decaySlider));
    addAndMakeVisible(decaySlider);

    sustainSlider.setSliderStyle(juce::Slider::LinearVertical);
    sustainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 20);
    sustainAttachment.reset(new SliderAttachment(vts, "sustain", sustainSlider));
    addAndMakeVisible(sustainSlider);

    releaseSlider.setSliderStyle(juce::Slider::LinearVertical);
    releaseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 20);
    releaseAttachment.reset(new SliderAttachment(vts, "release", releaseSlider));
    addAndMakeVisible(releaseSlider);
}

HuntericeAbyssAudioProcessorEditor::~HuntericeAbyssAudioProcessorEditor()
{
}

void HuntericeAbyssAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Промышленный темно-графитовый фон
    g.fillAll(juce::Colour::fromRGB(25, 25, 28)); 
    
    // Тонкая верхняя граница (темно-красная подсветка)
    g.setColour(juce::Colour::fromRGB(139, 0, 0));
    g.fillRect(0, 0, getWidth(), 4);

    g.setColour(juce::Colours::white);
}

void HuntericeAbyssAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);
    
    titleLabel.setBounds(area.removeFromTop(40));
    
    auto controlsArea = area.removeFromTop(200);
    auto sliderArea = controlsArea.removeFromLeft(60);
    
    attackSlider.setBounds(sliderArea);
    sliderArea.setX(sliderArea.getRight() + 10);
    decaySlider.setBounds(sliderArea);
    sliderArea.setX(sliderArea.getRight() + 10);
    sustainSlider.setBounds(sliderArea);
    sliderArea.setX(sliderArea.getRight() + 10);
    releaseSlider.setBounds(sliderArea);

    masterVolSlider.setBounds(area.removeFromRight(100).removeFromTop(100));
}
