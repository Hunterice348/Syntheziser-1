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
    for (auto& voice : voices)
    {
        voice.adsrParams.attack = 0.01f;
        voice.adsrParams.decay = 0.1f;
        voice.adsrParams.sustain = 0.5f;
        voice.adsrParams.release = 0.1f;
        voice.adsr.setParameters (voice.adsrParams);
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout HuntericeAbyssAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> ("attack", "Attack", 0.001f, 2.0f, 0.01f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("decay", "Decay", 0.001f, 2.0f, 0.1f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("sustain", "Sustain", 0.0f, 1.0f, 0.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("release", "Release", 0.001f, 3.0f, 0.1f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("volume", "Volume", 0.0f, 1.0f, 0.8f));
    
    juce::StringArray waveforms = { "Sine", "Saw", "Square", "Triangle" };
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("waveform", "Waveform", waveforms, 0));

    return { params.begin(), params.end() };
}

void HuntericeAbyssAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    for (auto& voice : voices)
    {
        voice.adsr.setSampleRate (sampleRate);
        voice.active = false;
        voice.phase = 0.0;
    }
}

void HuntericeAbyssAudioProcessor::releaseResources() {}

void HuntericeAbyssAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Чтение параметров (сглаживание можно добавить позже, пока читаем напрямую)
    float attack = parameters.getRawParameterValue ("attack")->load();
    float decay = parameters.getRawParameterValue ("decay")->load();
    float sustain = parameters.getRawParameterValue ("sustain")->load();
    float release = parameters.getRawParameterValue ("release")->load();
    float volume = parameters.getRawParameterValue ("volume")->load();
    int waveformIndex = static_cast<int> (parameters.getRawParameterValue ("waveform")->load());
    currentWaveform = static_cast<Waveform> (juce::jlimit (0, 3, waveformIndex));

    // Обновляем параметры ADSR для всех голосов
    for (auto& voice : voices)
    {
        voice.adsrParams.attack = attack;
        voice.adsrParams.decay = decay;
        voice.adsrParams.sustain = sustain;
        voice.adsrParams.release = release;
        voice.adsr.setParameters (voice.adsrParams);
    }

    // 1. Обработка MIDI
    for (const auto metadata : midiMessages)
    {
        auto msg = metadata.getMessage();
        
        if (msg.isNoteOn() && msg.getVelocity() > 0)
        {
            int note = msg.getNoteNumber();
            float freq = (float) juce::MidiMessage::getMidiNoteInHertz (note);
            
            // Поиск свободного голоса
            int voiceIndex = -1;
            for (int i = 0; i < MAX_VOICES; ++i)
            {
                if (!voices[i].active)
                {
                    voiceIndex = i;
                    break;
                }
            }
            
            // Voice Stealing: если все заняты, берем первый (можно улучшить до oldest)
            if (voiceIndex == -1)
                voiceIndex = 0;
                
            voices[voiceIndex].noteOn (freq, currentSampleRate);
            voices[voiceIndex].midiNote = note;
        }
        else if (msg.isNoteOff() || (msg.isNoteOn() && msg.getVelocity() == 0))
        {
            int note = msg.getNoteNumber();
            // Ищем голос с этой нотой и отправляем Note Off
            for (int i = 0; i < MAX_VOICES; ++i)
            {
                if (voices[i].active && voices[i].midiNote == note)
                {
                    voices[i].noteOff();
                }
            }
        }
    }
    midiMessages.clear();

    // 2. Генерация звука (полифоническое суммирование)
    for (int channel = 0; channel < totalNumOutputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float mix = 0.0f;

            for (int v = 0; v < MAX_VOICES; ++v)
            {
                if (!voices[v].active)
                    continue;

                float env = voices[v].adsr.getNextSample();
                
                // Если огибающая закончилась, деактивируем голос
                if (env <= 0.0f && voices[v].adsr.getState() == juce::ADSR::State::idle)
                {
                    voices[v].active = false;
                    continue;
                }

                float osc = 0.0f;
                switch (currentWaveform)
                {
                    case Waveform::Sine:
                        osc = std::sin (2.0f * juce::MathConstants<float>::pi * (float)voices[v].phase);
                        break;
                    case Waveform::Saw:
                        osc = 2.0f * (float)voices[v].phase - 1.0f;
                        break;
                    case Waveform::Square:
                        osc = ((float)voices[v].phase < 0.5f) ? 1.0f : -1.0f;
                        break;
                    case Waveform::Triangle:
                        if (voices[v].phase < 0.25)      osc = 4.0f * (float)voices[v].phase;
                        else if (voices[v].phase < 0.75) osc = 2.0f - 4.0f * (float)voices[v].phase;
                        else                             osc = 4.0f * (float)voices[v].phase - 4.0f;
                        break;
                }
                
                mix += osc * env;
                
                voices[v].phase += voices[v].phaseIncrement;
                if (voices[v].phase >= 1.0)
                    voices[v].phase -= 1.0;
            }

            channelData[sample] = mix * volume;
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
