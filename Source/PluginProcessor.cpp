#include "PluginProcessor.h"
#include "PluginEditor.h"

HuntericeAbyssAudioProcessor::HuntericeAbyssAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
    apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    // ИСПРАВЛЕНО: добавлено [](float x) для корректной лямбда-функции
    oscillator.initialise([](float x) { return x < 0.5f ? 2.0f * x - 1.0f : 2.0f * (1.0f - x); }, 128);
}

HuntericeAbyssAudioProcessor::~HuntericeAbyssAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout HuntericeAbyssAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>("oscLevel", "Oscillator Level", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("attack", "Attack", 0.001f, 2.0f, 0.01f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("decay", "Decay", 0.01f, 2.0f, 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("sustain", "Sustain", 0.0f, 1.0f, 0.7f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("release", "Release", 0.01f, 3.0f, 0.3f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("masterVol", "Master Volume", 0.0f, 1.0f, 0.8f));

    return { params.begin(), params.end() };
}

void HuntericeAbyssAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    oscillator.prepare(spec);
    adsr.setSampleRate(sampleRate);
    masterGain.prepare(spec);
    
    currentPhase = 0.0f;
}

void HuntericeAbyssAudioProcessor::releaseResources()
{
}

void HuntericeAbyssAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    auto* oscLevelParam = apvts.getRawParameterValue("oscLevel");
    auto* masterVolParam = apvts.getRawParameterValue("masterVol");
    
    masterGain.setGainLinear(*masterVolParam);

    for (const auto metadata : midiMessages)
    {
        auto msg = metadata.getMessage();
        if (msg.isNoteOn())
        {
            adsr.noteOn();
            oscillator.setFrequency(msg.getFrequencyInHertz());
            currentPhase = 0.0f;
            isVoiceActive = true;
        }
        else if (msg.isNoteOff())
        {
            adsr.noteOff();
        }
    }

    juce::dsp::ADSR::Parameters adsrParams;
    adsrParams.attack = *apvts.getRawParameterValue("attack");
    adsrParams.decay = *apvts.getRawParameterValue("decay");
    adsrParams.sustain = *apvts.getRawParameterValue("sustain");
    adsrParams.release = *apvts.getRawParameterValue("release");
    adsr.setParameters(adsrParams);

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float envelopeValue = adsr.getNextSample();
        
        currentPhase += 440.0f / (float) getSampleRate(); 
        if (currentPhase >= 1.0f) currentPhase -= 1.0f;
        float rawSaw = 2.0f * currentPhase - 1.0f;
        float finalSample = rawSaw * envelopeValue * (*oscLevelParam);

        for (int channel = 0; channel < totalNumOutputChannels; ++channel)
        {
            buffer.setSample(channel, sample, finalSample);
        }
    }

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    masterGain.process(context);

    midiMessages.clear();
}

juce::AudioProcessorEditor* HuntericeAbyssAudioProcessor::createEditor()
{
    return new HuntericeAbyssAudioProcessorEditor (*this, apvts);
}

void HuntericeAbyssAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void HuntericeAbyssAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

// ОБЯЗАТЕЛЬНАЯ функция для создания экземпляра плагина (VST3 требует её наличия)
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HuntericeAbyssAudioProcessor();
}
