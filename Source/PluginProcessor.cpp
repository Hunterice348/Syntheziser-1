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
        
        voice.filterAdsrParams.attack = 0.01f;
        voice.filterAdsrParams.decay = 0.1f;
        voice.filterAdsrParams.sustain = 0.5f;
        voice.filterAdsrParams.release = 0.1f;
        voice.filterAdsr.setParameters (voice.filterAdsrParams);
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout HuntericeAbyssAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Amp Envelope
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
    
    // Analog Modeling
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("analogDrift", "Analog Drift", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("masterDrive", "Master Drive", 0.0f, 1.0f, 0.0f));
    
    // Filter Engine
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("filterCutoff", "Filter Cutoff", 0.0f, 1.0f, 0.8f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("filterResonance", "Filter Resonance", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("filterDrive", "Filter Drive", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("filterKeyTrack", "Key Track", 0.0f, 1.0f, 0.5f));
    
    // Modulation Engine (Stage 8)
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("filterAttack", "Filter Attack", 0.001f, 2.0f, 0.01f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("filterDecay", "Filter Decay", 0.001f, 2.0f, 0.1f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("filterSustain", "Filter Sustain", 0.0f, 1.0f, 0.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("filterRelease", "Filter Release", 0.001f, 3.0f, 0.1f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("filterEnvAmount", "Filter Env Amt", -1.0f, 1.0f, 0.5f));
    
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lfoRate", "LFO Rate", 0.1f, 20.0f, 2.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lfoDepth", "LFO Depth", 0.0f, 1.0f, 0.0f));
    
    juce::StringArray waveforms = { "Sine", "Saw", "Square", "Triangle" };
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("waveform1", "Osc 1 Waveform", waveforms, 0));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("waveform2", "Osc 2 Waveform", waveforms, 1));
    
    juce::StringArray filterTypes = { "LP 12dB", "LP 24dB", "High Pass", "Band Pass" };
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("filterType", "Filter Type", filterTypes, 0));
    
    juce::StringArray lfoWaveforms = { "Sine", "Triangle", "Saw", "Square", "S&H" };
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("lfoWaveform", "LFO Waveform", lfoWaveforms, 0));

    return { params.begin(), params.end() };
}

void HuntericeAbyssAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    smoothedDrive.reset (sampleRate, 0.05);
    smoothedCutoff.reset (sampleRate, 0.02);
    smoothedResonance.reset (sampleRate, 0.02);
    
    filterL.reset();
    filterR.reset();
    
    for (auto& voice : voices)
    {
        voice.adsr.setSampleRate (sampleRate);
        voice.filterAdsr.setSampleRate (sampleRate);
        voice.active = false;
    }
}

void HuntericeAbyssAudioProcessor::releaseResources() {}

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
        default:
            return 0.0f;
    }
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
    float analogDrift = parameters.getRawParameterValue ("analogDrift")->load();
    float masterDrive = parameters.getRawParameterValue ("masterDrive")->load();
    
    float filterCutoff = parameters.getRawParameterValue ("filterCutoff")->load();
    float filterResonance = parameters.getRawParameterValue ("filterResonance")->load();
    float filterDrive = parameters.getRawParameterValue ("filterDrive")->load();
    float filterKeyTrack = parameters.getRawParameterValue ("filterKeyTrack")->load();
    int filterTypeIdx = static_cast<int> (parameters.getRawParameterValue ("filterType")->load());
    
    float fAttack = parameters.getRawParameterValue ("filterAttack")->load();
    float fDecay = parameters.getRawParameterValue ("filterDecay")->load();
    float fSustain = parameters.getRawParameterValue ("filterSustain")->load();
    float fRelease = parameters.getRawParameterValue ("filterRelease")->load();
    float fEnvAmount = parameters.getRawParameterValue ("filterEnvAmount")->load();
    
    float lfoRate = parameters.getRawParameterValue ("lfoRate")->load();
    float lfoDepth = parameters.getRawParameterValue ("lfoDepth")->load();
    int lfoWfIdx = static_cast<int> (parameters.getRawParameterValue ("lfoWaveform")->load());
    Waveform currentLfoWaveform = static_cast<Waveform> (juce::jlimit (0, 3, lfoWfIdx));
    bool lfoIsSH = (lfoWfIdx == 4);
    
    smoothedDrive.setTargetValue (masterDrive);
    smoothedCutoff.setTargetValue (filterCutoff);
    smoothedResonance.setTargetValue (filterResonance);
    
    currentFilterType = static_cast<FilterType> (juce::jlimit (0, 3, filterTypeIdx));
    
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
        
        voice.filterAdsrParams.attack = fAttack;
        voice.filterAdsrParams.decay = fDecay;
        voice.filterAdsrParams.sustain = fSustain;
        voice.filterAdsrParams.release = fRelease;
        voice.filterAdsr.setParameters (voice.filterAdsrParams);
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
            
            if (voiceIndex == -1) voiceIndex = 0;
                
            // ⚠️ ЗДЕСЬ ДОЛЖНО БЫТЬ 5 АРГУМЕНТОВ:
            voices[voiceIndex].noteOn (freq, currentSampleRate, osc2Detune, analogDrift, lfoRate);
            voices[voiceIndex].midiNote = note;
            lastNotePlayed = note;
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

    float keyTrackOffset = ((float)lastNotePlayed - 60.0f) / 120.0f * filterKeyTrack;

    for (int channel = 0; channel < totalNumOutputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        bool isLeft = (channel == 0);
        SVFilter& filter = isLeft ? filterL : filterR;

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float mix = 0.0f;
            float currentDrive = smoothedDrive.getNextValue();
            float baseCutoff = juce::jlimit (0.0f, 1.0f, smoothedCutoff.getNextValue() + keyTrackOffset);
            float currentResonance = smoothedResonance.getNextValue();

            for (int v = 0; v < MAX_VOICES; ++v)
            {
                if (!voices[v].active) continue;

                float env = voices[v].adsr.getNextSample();
                float fEnv = voices[v].filterAdsr.getNextSample();
                
                if (env <= 0.0f && !voices[v].adsr.isActive() && !voices[v].filterAdsr.isActive())
                {
                    voices[v].active = false;
                    continue;
                }

                float osc1 = generateWaveform ((float)voices[v].phase1, currentWaveform1);
                voices[v].phase1 += voices[v].phaseInc1;
                if (voices[v].phase1 >= 1.0) voices[v].phase1 -= 1.0;

                float osc2 = 0.0f;
                if (osc2Level > 0.0f)
                {
                    osc2 = generateWaveform ((float)voices[v].phase2, currentWaveform2);
                    voices[v].phase2 += voices[v].phaseInc2;
                    if (voices[v].phase2 >= 1.0) voices[v].phase2 -= 1.0;
                }

                float sub = 0.0f;
                if (subLevel > 0.0f)
                {
                    sub = (voices[v].phaseSub < 0.5f) ? 1.0f : -1.0f;
                    voices[v].phaseSub += voices[v].phaseIncSub;
                    if (voices[v].phaseSub >= 1.0) voices[v].phaseSub -= 1.0;
                }

                float noise = 0.0f;
                if (noiseLevel > 0.0f)
                {
                    noise = generateNoise (voices[v].noiseState);
                }

                // LFO calculation
                float lfoValue = 0.0f;
                if (lfoIsSH)
                {
                    voices[v].lfoPhase += voices[v].lfoPhaseInc;
                    if (voices[v].lfoPhase >= 1.0f)
                    {
                        voices[v].lfoPhase -= 1.0f;
                        voices[v].lfoSHValue = (juce::Random::getSystemRandom().nextFloat() * 2.0f) - 1.0f;
                    }
                    lfoValue = voices[v].lfoSHValue;
                }
                else if (lfoDepth > 0.0f)
                {
                    lfoValue = generateWaveform ((float)voices[v].lfoPhase, currentLfoWaveform);
                    voices[v].lfoPhase += voices[v].lfoPhaseInc;
                    if (voices[v].lfoPhase >= 1.0f) voices[v].lfoPhase -= 1.0f;
                }

                // Modulate cutoff
                float modAmount = (fEnv * fEnvAmount) + (lfoValue * lfoDepth);
                float modulatedCutoff = juce::jlimit (0.0f, 1.0f, baseCutoff + modAmount);

                float oscMix = (osc1 + (osc2 * osc2Level) + (sub * subLevel) + (noise * noiseLevel)) * env;
                
                // --- ФИЛЬТР ---
                float filtered = filter.process (oscMix, modulatedCutoff, currentResonance, filterDrive, currentFilterType);

                // Master Saturation
                float driveMultiplier = 1.0f + (currentDrive * 4.0f);
                float saturatedMix = std::tanh (filtered * driveMultiplier);

                float compensation = 1.0f / std::tanh (driveMultiplier);
                if (driveMultiplier > 1.0f)
                {
                    saturatedMix *= compensation;
                }

                mix += saturatedMix;
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
