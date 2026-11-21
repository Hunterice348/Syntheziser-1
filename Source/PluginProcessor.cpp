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
        voice.adsrParams.attack = 0.01f; voice.adsrParams.decay = 0.1f;
        voice.adsrParams.sustain = 0.5f; voice.adsrParams.release = 0.1f;
        voice.adsr.setParameters (voice.adsrParams);
        voice.filterAdsrParams.attack = 0.01f; voice.filterAdsrParams.decay = 0.1f;
        voice.filterAdsrParams.sustain = 0.5f; voice.filterAdsrParams.release = 0.1f;
        voice.filterAdsr.setParameters (voice.filterAdsrParams);
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
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("osc2Level", "Osc 2 Level", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("osc2Detune", "Osc 2 Detune", -12.0f, 12.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("subLevel", "Sub Level", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("noiseLevel", "Noise Level", 0.0f, 1.0f, 0.0f));
    
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("analogDrift", "Analog Drift", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("masterDrive", "Master Drive", 0.0f, 1.0f, 0.0f));
    
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("filterCutoff", "Filter Cutoff", 0.0f, 1.0f, 0.8f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("filterResonance", "Filter Resonance", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("filterDrive", "Filter Drive", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("filterKeyTrack", "Key Track", 0.0f, 1.0f, 0.5f));
    
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("filterAttack", "Filter Attack", 0.001f, 2.0f, 0.01f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("filterDecay", "Filter Decay", 0.001f, 2.0f, 0.1f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("filterSustain", "Filter Sustain", 0.0f, 1.0f, 0.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("filterRelease", "Filter Release", 0.001f, 3.0f, 0.1f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("filterEnvAmount", "Filter Env Amt", -1.0f, 1.0f, 0.5f));
    
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lfoRate", "LFO Rate", 0.1f, 20.0f, 2.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lfoDepth", "LFO Depth", 0.0f, 1.0f, 0.0f));
    
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("modLfoToPitch", "LFO -> Pitch", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("modLfoToCutoff", "LFO -> Cutoff", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("modEnvToPitch", "Env -> Pitch", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("modVelToCutoff", "Vel -> Cutoff", 0.0f, 1.0f, 0.0f));
    
    for (int i = 0; i < NUM_PATCH_CABLES; ++i)
    {
        params.push_back (std::make_unique<juce::AudioParameterChoice> ("patch" + juce::String(i) + "Src", "Patch " + juce::String(i+1) + " Source", juce::StringArray { "None", "LFO", "Filter Env", "Velocity", "Mod Wheel", "Osc 1 (Audio)", "Noise (Audio)" }, 0));
        params.push_back (std::make_unique<juce::AudioParameterChoice> ("patch" + juce::String(i) + "Dst", "Patch " + juce::String(i+1) + " Dest", juce::StringArray { "None", "Filter Cutoff", "Osc 1 Pitch", "Osc 2 Pitch", "Master Vol", "Filter Res" }, 0));
        params.push_back (std::make_unique<juce::AudioParameterFloat> ("patch" + juce::String(i) + "Amt", "Patch " + juce::String(i+1) + " Amount", 0.0f, 1.0f, 0.5f));
    }
    
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("distDrive", "Dist Drive", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("distMix", "Dist Mix", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("chorusRate", "Chorus Rate", 0.1f, 5.0f, 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("chorusDepth", "Chorus Depth", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("chorusMix", "Chorus Mix", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("delayTime", "Delay Time", 0.01f, 1.0f, 0.3f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("delayFeedback", "Delay Feedback", 0.0f, 0.9f, 0.3f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("delayMix", "Delay Mix", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("reverbDecay", "Reverb Decay", 0.0f, 0.95f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("reverbMix", "Reverb Mix", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("fxBypass", "FX Bypass", false));

    // STAGE 12: Unison & Macro
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("unisonSpread", "Unison Spread", 0.0f, 0.05f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("macroMorph", "Macro: Clean <-> Dirty", 0.0f, 1.0f, 0.0f));
    
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
    filterL.reset(); filterR.reset(); delayL.reset(); delayR.reset();
    chorusL.reset(); chorusR.reset(); reverbL.reset(); reverbR.reset();
    currentModWheel = 0.0f;
    for (auto& voice : voices) { voice.adsr.setSampleRate (sampleRate); voice.filterAdsr.setSampleRate (sampleRate); voice.active = false; }
}

void HuntericeAbyssAudioProcessor::releaseResources() {}

inline float generateWaveform (float phase, HuntericeAbyssAudioProcessor::Waveform wf)
{
    switch (wf)
    {
        case HuntericeAbyssAudioProcessor::Waveform::Sine: return std::sin (2.0f * juce::MathConstants<float>::pi * phase);
        case HuntericeAbyssAudioProcessor::Waveform::Saw: return 2.0f * phase - 1.0f;
        case HuntericeAbyssAudioProcessor::Waveform::Square: return (phase < 0.5f) ? 1.0f : -1.0f;
        case HuntericeAbyssAudioProcessor::Waveform::Triangle:
            if (phase < 0.25f) return 4.0f * phase;
            else if (phase < 0.75f) return 2.0f - 4.0f * phase;
            else return 4.0f * phase - 4.0f;
        default: return 0.0f;
    }
}

void HuntericeAbyssAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) buffer.clear (i, 0, buffer.getNumSamples());

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
    
    float modLfoToPitch = parameters.getRawParameterValue ("modLfoToPitch")->load();
    float modLfoToCutoff = parameters.getRawParameterValue ("modLfoToCutoff")->load();
    float modEnvToPitch = parameters.getRawParameterValue ("modEnvToPitch")->load();
    float modVelToCutoff = parameters.getRawParameterValue ("modVelToCutoff")->load();
    
    float distDrive = parameters.getRawParameterValue ("distDrive")->load();
    float distMix = parameters.getRawParameterValue ("distMix")->load();
    float chorusRate = parameters.getRawParameterValue ("chorusRate")->load();
    float chorusDepth = parameters.getRawParameterValue ("chorusDepth")->load();
    float chorusMix = parameters.getRawParameterValue ("chorusMix")->load();
    float delayTime = parameters.getRawParameterValue ("delayTime")->load();
    float delayFeedback = parameters.getRawParameterValue ("delayFeedback")->load();
    float delayMix = parameters.getRawParameterValue ("delayMix")->load();
    float reverbDecay = parameters.getRawParameterValue ("reverbDecay")->load();
    float reverbMix = parameters.getRawParameterValue ("reverbMix")->load();
    bool fxBypass = parameters.getRawParameterValue ("fxBypass")->load();

    float unisonSpread = parameters.getRawParameterValue ("unisonSpread")->load();
    float macroMorph = parameters.getRawParameterValue ("macroMorph")->load();
    
    for (int i = 0; i < NUM_PATCH_CABLES; ++i)
    {
        patchCables[i].source = static_cast<ModSource> (static_cast<int> (parameters.getRawParameterValue ("patch" + juce::String(i) + "Src")->load()));
        patchCables[i].destination = static_cast<ModDest> (static_cast<int> (parameters.getRawParameterValue ("patch" + juce::String(i) + "Dst")->load()));
        patchCables[i].amount = parameters.getRawParameterValue ("patch" + juce::String(i) + "Amt")->load();
        patchCables[i].active = (patchCables[i].source != ModSource::None && patchCables[i].destination != ModDest::None);
    }
    
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
        voice.adsrParams.attack = attack; voice.adsrParams.decay = decay;
        voice.adsrParams.sustain = sustain; voice.adsrParams.release = release;
        voice.adsr.setParameters (voice.adsrParams);
        voice.filterAdsrParams.attack = fAttack; voice.filterAdsrParams.decay = fDecay;
        voice.filterAdsrParams.sustain = fSustain; voice.filterAdsrParams.release = fRelease;
        voice.filterAdsr.setParameters (voice.filterAdsrParams);
    }

    for (const auto metadata : midiMessages)
    {
        auto msg = metadata.getMessage();
        if (msg.isController() && msg.getControllerNumber() == 1) currentModWheel = msg.getControllerValue() / 127.0f;
        else if (msg.isNoteOn() && msg.getVelocity() > 0)
        {
            int note = msg.getNoteNumber();
            float freq = (float) juce::MidiMessage::getMidiNoteInHertz (note);
            float vel = msg.getVelocity() / 127.0f;
            int voiceIndex = -1;
            for (int i = 0; i < MAX_VOICES; ++i) if (!voices[i].active) { voiceIndex = i; break; }
            if (voiceIndex == -1) voiceIndex = 0;
            voices[voiceIndex].noteOn (freq, currentSampleRate, osc2Detune, analogDrift, lfoRate, vel, unisonSpread);
            voices[voiceIndex].midiNote = note;
            lastNotePlayed = note;
        }
        else if (msg.isNoteOff() || (msg.isNoteOn() && msg.getVelocity() == 0))
        {
            int note = msg.getNoteNumber();
            for (int i = 0; i < MAX_VOICES; ++i) if (voices[i].active && voices[i].midiNote == note) voices[i].noteOff();
        }
    }
    midiMessages.clear();

    float keyTrackOffset = ((float)lastNotePlayed - 60.0f) / 120.0f * filterKeyTrack;
    int delaySamples = (int)(delayTime * (float)currentSampleRate);
    delaySamples = juce::jlimit (1, MAX_DELAY_SAMPLES - 1, delaySamples);

    for (int channel = 0; channel < totalNumOutputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        bool isLeft = (channel == 0);
        SVFilter& filter = isLeft ? filterL : filterR;
        DelayLine& delay = isLeft ? delayL : delayR;
        ChorusEffect& chorus = isLeft ? chorusL : chorusR;
        ReverbEffect& reverb = isLeft ? reverbL : reverbR;

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float mix = 0.0f;
            float currentDrive = smoothedDrive.getNextValue();
            
            float morphedCutoff = juce::jlimit (0.0f, 1.0f, smoothedCutoff.getNextValue() + keyTrackOffset + (macroMorph * 0.6f));
            float morphedDistDrive = distDrive + (macroMorph * 0.8f);
            float morphedNoise = noiseLevel + (macroMorph * 0.5f);
            
            float currentResonance = smoothedResonance.getNextValue();
            float masterVolMod = 0.0f;

            for (int v = 0; v < MAX_VOICES; ++v)
            {
                if (!voices[v].active) continue;
                float env = voices[v].adsr.getNextSample();
                float fEnv = voices[v].filterAdsr.getNextSample();
                if (env <= 0.0f && !voices[v].adsr.isActive() && !voices[v].filterAdsr.isActive()) { voices[v].active = false; continue; }

                float lfoValue = 0.0f;
                if (lfoIsSH)
                {
                    voices[v].lfoPhase += voices[v].lfoPhaseInc;
                    if (voices[v].lfoPhase >= 1.0f) { voices[v].lfoPhase -= 1.0f; voices[v].lfoSHValue = (juce::Random::getSystemRandom().nextFloat() * 2.0f) - 1.0f; }
                    lfoValue = voices[v].lfoSHValue;
                }
                else if (lfoDepth > 0.0f || modLfoToPitch > 0.0f || modLfoToCutoff > 0.0f)
                {
                    lfoValue = generateWaveform ((float)voices[v].lfoPhase, currentLfoWaveform);
                    voices[v].lfoPhase += voices[v].lfoPhaseInc;
                    if (voices[v].lfoPhase >= 1.0f) voices[v].lfoPhase -= 1.0f;
                }

                float voicePitchMod = (fEnv - 0.5f) * modEnvToPitch * 0.05f + lfoValue * modLfoToPitch * 0.05f;
                float voiceCutoffMod = (fEnv * fEnvAmount) + (lfoValue * lfoDepth) + (lfoValue * modLfoToCutoff) + ((voices[v].velocity - 0.5f) * modVelToCutoff * 0.5f) + ((currentModWheel - 0.5f) * 0.5f);

                for (int c = 0; c < NUM_PATCH_CABLES; ++c)
                {
                    if (!patchCables[c].active) continue;
                    float srcVal = 0.0f;
                    switch (patchCables[c].source)
                    {
                        case ModSource::LFO: srcVal = lfoValue; break;
                        case ModSource::FilterEnv: srcVal = fEnv; break;
                        case ModSource::Velocity: srcVal = voices[v].velocity; break;
                        case ModSource::ModWheel: srcVal = currentModWheel; break;
                        default: break;
                    }
                    float modAmount = srcVal * patchCables[c].amount;
                    switch (patchCables[c].destination)
                    {
                        case ModDest::FilterCutoff: voiceCutoffMod += modAmount; break;
                        case ModDest::Osc1Pitch: voicePitchMod += modAmount * 0.05f; break;
                        case ModDest::Osc2Pitch: voicePitchMod += modAmount * 0.05f; break;
                        case ModDest::MasterVol: masterVolMod += modAmount; break;
                        case ModDest::FilterRes: currentResonance += modAmount; break;
                        default: break;
                    }
                }

                float modulatedPhaseInc1 = voices[v].phaseInc1 * (1.0f + voicePitchMod);
                float modulatedPhaseInc2 = voices[v].phaseInc2 * (1.0f + voicePitchMod);
                float modulatedPhaseIncSub = voices[v].phaseIncSub * (1.0f + voicePitchMod);

                float osc1 = generateWaveform ((float)voices[v].phase1, currentWaveform1);
                voices[v].phase1 += modulatedPhaseInc1;
                if (voices[v].phase1 >= 1.0) voices[v].phase1 -= 1.0;

                float osc2 = 0.0f;
                if (osc2Level > 0.0f)
                {
                    osc2 = generateWaveform ((float)voices[v].phase2, currentWaveform2);
                    voices[v].phase2 += modulatedPhaseInc2;
                    if (voices[v].phase2 >= 1.0) voices[v].phase2 -= 1.0;
                }

                float sub = 0.0f;
                if (subLevel > 0.0f)
                {
                    sub = (voices[v].phaseSub < 0.5f) ? 1.0f : -1.0f;
                    voices[v].phaseSub += modulatedPhaseIncSub;
                    if (voices[v].phaseSub >= 1.0) voices[v].phaseSub -= 1.0;
                }

                float noise = 0.0f;
                if (morphedNoise > 0.0f) noise = generateNoise (voices[v].noiseState);

                for (int c = 0; c < NUM_PATCH_CABLES; ++c)
                {
                    if (!patchCables[c].active) continue;
                    float srcVal = 0.0f;
                    if (patchCables[c].source == ModSource::Osc1Audio) srcVal = osc1;
                    else if (patchCables[c].source == ModSource::NoiseAudio) srcVal = noise;
                    else continue;
                    float modAmount = srcVal * patchCables[c].amount;
                    if (patchCables[c].destination == ModDest::FilterCutoff) voiceCutoffMod += modAmount * 0.5f;
                }

                float modulatedCutoff = juce::jlimit (0.0f, 1.0f, morphedCutoff + voiceCutoffMod);
                float oscMix = (osc1 + (osc2 * osc2Level) + (sub * subLevel) + (noise * morphedNoise)) * env;
                
                float filtered = filter.process (oscMix, modulatedCutoff, currentResonance, filterDrive, currentFilterType);
                float driveMultiplier = 1.0f + (morphedDistDrive * 4.0f);
                float saturatedMix = std::tanh (filtered * driveMultiplier);
                float compensation = 1.0f / std::tanh (driveMultiplier);
                if (driveMultiplier > 1.0f) saturatedMix *= compensation;

                mix += saturatedMix;
            }

            float drySignal = mix * volume * (1.0f + masterVolMod);
            float fxSignal = drySignal;

            if (!fxBypass)
            {
                if (distMix > 0.0f)
                {
                    float distMultiplier = 1.0f + (distDrive * 10.0f);
                    float distorted = std::tanh (fxSignal * distMultiplier);
                    float compDist = 1.0f / std::tanh (distMultiplier);
                    if (distMultiplier > 1.0f) distorted *= compDist;
                    fxSignal = fxSignal * (1.0f - distMix) + distorted * distMix;
                }
                if (chorusMix > 0.0f) fxSignal = chorus.process (fxSignal, chorusRate, chorusDepth, chorusMix, currentSampleRate);
                if (delayMix > 0.0f)
                {
                    float delayed = delay.process (fxSignal, delaySamples, delayFeedback);
                    fxSignal = fxSignal * (1.0f - delayMix) + delayed * delayMix;
                }
                if (reverbMix > 0.0f) fxSignal = reverb.process (fxSignal, reverbDecay, reverbMix);
            }
            channelData[sample] = fxSignal;
        }
    }
}

juce::AudioProcessorEditor* HuntericeAbyssAudioProcessor::createEditor() { return new HuntericeAbyssAudioProcessorEditor (*this, parameters); }
void HuntericeAbyssAudioProcessor::getStateInformation (juce::MemoryBlock& destData) { auto state = parameters.copyState(); std::unique_ptr<juce::XmlElement> xml (state.createXml()); copyXmlToBinary (*xml, destData); }
void HuntericeAbyssAudioProcessor::setStateInformation (const void* data, int sizeInBytes) { std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes)); if (xmlState.get() != nullptr) if (xmlState->hasTagName (parameters.state.getType())) parameters.replaceState (juce::ValueTree::fromXml (*xmlState)); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new HuntericeAbyssAudioProcessor(); }
