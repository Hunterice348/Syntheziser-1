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

    // ADSR
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("attack", "Attack", 0.001f, 2.0f, 0.01f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("decay", "Decay", 0.001f, 2.0f, 0.1f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("sustain", "Sustain", 0.0f, 1.0f, 0.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("release", "Release", 0.001f, 3.0f, 0.1f));
    
    // Mixer / Oscillators
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("volume", "Volume", 0.0f, 1.0f, 0.8f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("osc2Level", "Osc 2 Level", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("osc2Detune", "Osc 2 Detune", -12.0f, 12.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("subLevel", "Sub Level", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("noiseLevel", "Noise Level", 0.0f, 1.0f, 0.0f));
    
    // Waveforms
    juce::StringArray waveforms = { "Sine", "Saw", "Square", "Triangle" };
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("waveform1", "Osc 1 Waveform", waveforms, 0));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("waveform2", "Osc 2 Waveform", waveforms, 1));

    return { params.begin(), params.end() };
}

void HuntericeAbyssAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    for (auto& voice : voices)
    {
        voice.adsr.setSampleRate (sampleRate);
        voice.active = false;
    }
}

void HuntericeAbyssAudioProcessor::releaseResources() {}

// Вспомогательная функция для генерации волны
inline float generateWaveform (float phase, HuntericeAbyssAudioProcessor::Waveform wf)
{
    switch (wf)
    {
        case HuntericeAbyssAudioProcessor::Waveform::Sine:
            return std::sin (2.0f * juce::MathConstants<float>::pi * phase);
        case HuntericeAbyssAudioProcessor::Waveform::Saw:
            return 2.0f * phase - 1.0f;
        case HuntericeAbyssAudioProcessor::Waveform::Square:
            return (phase < 0.5f) ? 1.0f : -1.0f;
        case HuntericeAbyssAudioProcessor::Waveform::Triangle:
            if (phase < 0.25f)      return 4.0f * phase;
            else if (phase < 0.75f) return 2.0f - 4.0f * phase;
            else                    return 4.0f * phase - 4.0f;
    }
    return 0.0f;
}

void HuntericeAbyssAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    float attack = parameters.getRawParameterValue ("attack")->load();
    float decay = parameters.getRawParameterValue ("decay")->load();
    float sustain = parameters.getRawParameterValue ("sustain")->load();
    float release = parameters.getRawParameterValue ("release")->load();
    float volume = parameters.getRawParameterValue ("volume")->load();
    
    float osc2Level = parameters.getRawParameterValue ("osc2Level")->load();
    float osc2Detune = parameters.getRawParameterValue ("osc2Detune")->load();
    float subLevel = parameters.getRawParameterValue ("subLevel")->load();
    float noiseLevel = parameters.getRawParameterValue ("noiseLevel")->load();
    
    int wf1 = static_cast<int> (parameters.getRawParameterValue ("waveform1")->load());
    int wf2 = static_cast<int> (parameters.getRawParameterValue ("waveform2")->load());
    currentWaveform1 = static_cast<Waveform> (juce::jlimit (0, 3, wf1));
    currentWaveform2 = static_cast<Waveform> (juce::jlimit (0, 3, wf2));

    for (auto& voice : voices)
    {
        voice.adsrParams.attack = attack;
        voice.adsrParams.decay = decay;
        voice.adsrParams.sustain = sustain;
        voice.adsrParams.release = release;
        voice.adsr.setParameters (voice.adsrParams);
    }

    for (const auto metadata : midiMessages)
    {
        auto msg = metadata.getMessage();
        if (msg.isNoteOn() && msg.getVelocity() > 0)
        {
            int note = msg.getNoteNumber();
            float freq = (float) juce::MidiMessage::getMidiNoteInHertz (note);
            
            int voiceIndex = -1;
            for (int i = 0; i < MAX_VOICES; ++i)
                if (!voices[i].active) { voiceIndex = i; break; }
            
            if (voiceIndex == -1) voiceIndex = 0; // Voice stealing
                
            voices[voiceIndex].noteOn (freq, currentSampleRate, osc2Detune);
            voices[voiceIndex].midiNote = note;
        }
        else if (msg.isNoteOff() || (msg.isNoteOn() && msg.getVelocity() == 0))
        {
            int note = msg.getNoteNumber();
            for (int i = 0; i < MAX_VOICES; ++i)
                if (voices[i].active && voices[i].midiNote == note)
                    voices[i].noteOff();
        }
    }
    midiMessages.clear();

    for (int channel = 0; channel < totalNumOutputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float mix = 0.0f;

            for (int v = 0; v < MAX_VOICES; ++v)
            {
                if (!voices[v].active) continue;

                float env =
