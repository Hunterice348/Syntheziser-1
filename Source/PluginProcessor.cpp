#include "PluginProcessor.h"
#include "PluginEditor.h"

HuntericeAbyssAudioProcessor::HuntericeAbyssAudioProcessor()
     : AudioProcessor (BusesProperties()
                 #if ! JucePlugin_IsMidiEffect
                  #if ! JucePlugin_IsSynth
                   .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                  #endif
                   .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                 #endif
                   ),
       parameters (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    adsrParams.attack = 0.01f;
    adsrParams.decay = 0.1f;
    adsrParams.sustain = 0.5f;
    adsrParams.release = 0.1f;
    adsr.setParameters (adsrParams);
}

juce::AudioProcessorValueTreeState::ParameterLayout HuntericeAbyssAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> ("attack", "Attack", 0.001f, 2.0f, 0.01f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("decay", "Decay", 0.001f, 2.0f, 0.1f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("sustain", "Sustain", 0.0f, 1.0f, 0.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("release", "Release", 0.001f, 3.0f, 0.1f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("volume", "Volume", 0.0f, 1.0f, 0.8f));

    return { params.begin(), params.end() };
}

void HuntericeAbyssAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    adsr.setSampleRate (sampleRate);
    currentPhase = 0.0;
}

void HuntericeAbyssAudioProcessor::releaseResources() {}

void HuntericeAbyssAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Очистка буфера
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Обновление параметров из GUI в реальном времени
    adsrParams.attack = *parameters.getRawParameterValue ("attack");
    adsrParams.decay = *parameters.getRawParameterValue ("decay");
    adsrParams.sustain = *parameters.getRawParameterValue ("sustain");
    adsrParams.release = *parameters.getRawParameterValue ("release");
    adsr.setParameters (adsrParams);

    auto volume = *parameters.getRawParameterValue ("volume");

    // Обработка MIDI
    for (const auto metadata : midiMessages)
    {
        auto msg = metadata.getMessage();
        if (msg.isNoteOn())
        {
            currentNoteFreq = (float) juce::MidiMessage::getMidiNoteInHertz (msg.getNoteNumber());
            phaseIncrement = currentNoteFreq / currentSampleRate;
            currentPhase = 0.0; // Сброс фазы для новой ноты
            adsr.noteOn();
        }
        else if (msg.isNoteOff())
        {
            adsr.noteOff();
        }
    }
    midiMessages.clear();

    // Генерация звука (поканально)
    for (int channel = 0; channel < totalNumOutputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float envelopeValue = adsr.getNextSample();

            // Простой синусоидальный осциллятор с отслеживанием фазы
            float oscillatorOutput = std::sin (2.0f * juce::MathConstants<float>::pi * (float)currentPhase);
            
            currentPhase += phaseIncrement;
            if (currentPhase >= 1.0)
                currentPhase -= 1.0;

            channelData[sample] = oscillatorOutput * envelopeValue * volume;
        }
    }
}

juce::AudioProcessorEditor* HuntericeAbyssAudioProcessor::createEditor()
{
    return new HuntericeAbyssAudioProcessorEditor (*this, parameters);
}

void HuntericeAbyssAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void HuntericeAbyssAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

// Точка входа для VST3
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HuntericeAbyssAudioProcessor();
}
