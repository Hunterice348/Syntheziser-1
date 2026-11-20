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
    
    // Добавляем параметр выбора формы волны
    juce::StringArray waveforms = { "Sine", "Saw", "Square", "Triangle" };
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("waveform", "Waveform", waveforms, 0));

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

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    adsrParams.attack = parameters.getRawParameterValue ("attack")->load();
    adsrParams.decay = parameters.getRawParameterValue ("decay")->load();
    adsrParams.sustain = parameters.getRawParameterValue ("sustain")->load();
    adsrParams.release = parameters.getRawParameterValue ("release")->load();
    adsr.setParameters (adsrParams);

    float volume = parameters.getRawParameterValue ("volume")->load();
    int waveformIndex = static_cast<int> (parameters.getRawParameterValue ("waveform")->load());
    currentWaveform = static_cast<Waveform> (juce::jlimit (0, 3, waveformIndex));

    for (const auto metadata : midiMessages)
    {
        auto msg = metadata.getMessage();
        if (msg.isNoteOn())
        {
            currentNoteFreq = (float) juce::MidiMessage::getMidiNoteInHertz (msg.getNoteNumber());
            phaseIncrement = currentNoteFreq / currentSampleRate;
            currentPhase = 0.0; 
            adsr.noteOn();
        }
        else if (msg.isNoteOff())
        {
            adsr.noteOff();
        }
    }
    midiMessages.clear();

    for (int channel = 0; channel < totalNumOutputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float envelopeValue = adsr.getNextSample();
            float oscillatorOutput = 0.0f;

            // DSP Генерация форм волны
            switch (currentWaveform)
            {
                case Waveform::Sine:
                    oscillatorOutput = std::sin (2.0f * juce::MathConstants<float>::pi * (float)currentPhase);
                    break;
                case Waveform::Saw:
                    oscillatorOutput = 2.0f * (float)currentPhase - 1.0f;
                    break;
                case Waveform::Square:
                    oscillatorOutput = ((float)currentPhase < 0.5f) ? 1.0f : -1.0f;
                    break;
                case Waveform::Triangle:
                    if (currentPhase < 0.25)      oscillatorOutput = 4.0f * (float)currentPhase;
                    else if (currentPhase < 0.75)  oscillatorOutput = 2.0f - 4.0f * (float)currentPhase;
                    else                           oscillatorOutput = 4.0f * (float)currentPhase - 4.0f;
                    break;
            }
            
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

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HuntericeAbyssAudioProcessor();
}
