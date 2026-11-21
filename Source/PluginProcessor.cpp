#include "PluginProcessor.h"
#include "PluginEditor.h"

HuntericeAbyssAudioProcessor::HuntericeAbyssAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    drums[0].type = DrumVoice::Kick;
    drums[1].type = DrumVoice::Snare;
    drums[2].type = DrumVoice::Hat;
    drums[3].type = DrumVoice::Clap;
    drums[4].type = DrumVoice::Tom;
    drums[5].type = DrumVoice::Perc;
}

bool HuntericeAbyssAudioProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    const auto& out = l.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

juce::AudioProcessorValueTreeState::ParameterLayout HuntericeAbyssAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    const juce::StringArray waves  { "Sine", "Saw", "Square", "Triangle" };
    const juce::StringArray ftypes { "LP 12dB", "LP 24dB", "High Pass", "Band Pass" };
    const juce::StringArray lwaves { "Sine", "Triangle", "Saw", "Square", "S&H" };
    const juce::StringArray srcs   { "None", "LFO", "Filter Env", "Velocity", "Mod Wheel", "Osc 1 (Audio)", "Noise (Audio)" };
    const juce::StringArray dsts   { "None", "Filter Cutoff", "Osc 1 Pitch", "Osc 2 Pitch", "Master Vol", "Filter Res" };

    p.push_back (std::make_unique<juce::AudioParameterFloat> ("attack",  "Attack",  0.001f, 2.0f, 0.01f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("decay",   "Decay",   0.001f, 2.0f, 0.10f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("sustain", "Sustain", 0.0f,   1.0f, 0.50f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("release", "Release", 0.001f, 3.0f, 0.10f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("volume",      "Volume",       0.0f, 1.0f, 0.8f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("osc2Level",   "Osc 2 Level",  0.0f, 1.0f, 0.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("osc2Detune",  "Osc 2 Detune", -12.0f, 12.0f, 0.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("subLevel",    "Sub Level",    0.0f, 1.0f, 0.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("noiseLevel",  "Noise Level",  0.0f, 1.0f, 0.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("analogDrift", "Analog Drift", 0.0f, 1.0f, 0.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("masterDrive", "Master Drive", 0.0f, 1.0f, 0.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("filterCutoff",    "Filter Cutoff", 0.0f, 1.0f, 0.8f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("filterResonance", "Resonance",     0.0f, 1.0f, 0.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("filterDrive",     "Filter Drive",  0.0f, 1.0f, 0.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("filterKeyTrack",  "Key Track",     0.0f, 1.0f, 0.5f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("filterAttack",    "Filter Attack", 0.001f, 2.0f, 0.01f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("filterDecay",     "Filter Decay",  0.001f, 2.0f, 0.10f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("filterSustain",   "Filter Sustain",0.0f, 1.0f, 0.50f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("filterRelease",   "Filter Release",0.001f, 3.0f, 0.10f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("filterEnvAmount", "Filter Env Amt",-1.0f, 1.0f, 0.5f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("lfoRate",  "LFO Rate",  0.1f, 20.0f, 2.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("lfoDepth", "LFO Depth", 0.0f, 1.0f,  0.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("modLfoToPitch",  "LFO -> Pitch",  0.0f, 1.0f, 0.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("modLfoToCutoff", "LFO -> Cutoff", 0.0f, 1.0f, 0.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("modEnvToPitch",  "Env -> Pitch",  0.0f, 1.0f, 0.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("modVelToCutoff", "Vel -> Cutoff", 0.0f, 1.0f, 0.0f));

    for (int i = 0; i < NUM_PATCH; ++i)
    {
        p.push_back (std::make_unique<juce::AudioParameterChoice> ("patch" + juce::String (i) + "Src", "Patch " + juce::String (i + 1) + " Source", srcs, 0));
        p.push_back (std::make_unique<juce::AudioParameterChoice> ("patch" + juce::String (i) + "Dst", "Patch " + juce::String (i + 1) + " Dest",   dsts, 0));
        p.push_back (std::make_unique<juce::AudioParameterFloat>  ("patch" + juce::String (i) + "Amt", "Patch " + juce::String (i + 1) + " Amount", 0.0f, 1.0f, 0.5f));
    }

    p.push_back (std::make_unique<juce::AudioParameterFloat> ("distDrive",     "Dist Drive",     0.0f, 1.0f,  0.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("distMix",       "Dist Mix",       0.0f, 1.0f,  0.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("chorusRate",    "Chorus Rate",    0.1f, 5.0f,  1.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("chorusDepth",   "Chorus Depth",   0.0f, 1.0f,  0.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("chorusMix",     "Chorus Mix",     0.0f, 1.0f,  0.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("delayTime",     "Delay Time",     0.01f, 1.0f, 0.3f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("delayFeedback", "Delay Feedback", 0.0f, 0.9f,  0.3f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("delayMix",      "Delay Mix",      0.0f, 1.0f,  0.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("reverbDecay",   "Reverb Decay",   0.0f, 0.95f, 0.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("reverbMix",     "Reverb Mix",     0.0f, 1.0f,  0.0f));
    p.push_back (std::make_unique<juce::AudioParameterBool>  ("fxBypass",      "FX Bypass",      false));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("unisonSpread", "Unison Spread", 0.0f, 0.05f, 0.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("macroMorph",   "Clean <-> Dirty", 0.0f, 1.0f, 0.0f));
    p.push_back (std::make_unique<juce::AudioParameterChoice> ("waveform1",   "Osc 1 Waveform", waves,  0));
    p.push_back (std::make_unique<juce::AudioParameterChoice> ("waveform2",   "Osc 2 Waveform", waves,  1));
    p.push_back (std::make_unique<juce::AudioParameterChoice> ("filterType",  "Filter Type",    ftypes, 0));
    p.push_back (std::make_unique<juce::AudioParameterChoice> ("lfoWaveform", "LFO Waveform",   lwaves, 0));

    // === ЮНИСОН + PWM ===
    p.push_back (std::make_unique<juce::AudioParameterInt> ("unisonVoices", "Unison Voices", 1, 8, 1));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("pwmWidth", "PWM Width", 0.1f, 0.9f, 0.5f));

    // === НОВЫЕ ПАРАМЕТРЫ ===
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("glide",        "Glide",      0.0f, 1.0f, 0.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("velToAmp",     "Vel -> Amp", 0.0f, 1.0f, 0.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("kickBody",     "KICK BODY",  0.0f, 1.0f, 0.5f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("kickPitchAmt", "KICK PITCH", 0.0f, 2.0f, 1.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("snareSnap",    "SNARE SNAP", 0.0f, 1.0f, 0.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("snareBody",    "SNARE BODY", 0.0f, 1.0f, 0.5f));

    // DRUM MACHINE
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("drumVol",   "DRUM VOL", 0.0f, 1.0f, 0.8f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("drumDrive", "DRUM DRV", 0.0f, 1.0f, 0.0f));

    const juce::StringArray drumNames { "KICK", "SNARE", "HI-HAT", "CLAP", "TOM", "PERC" };
    for (int i = 0; i < NUM_DRUMS; ++i)
    {
        const juce::String prefix = drumNames[i].toLowerCase().replaceCharacter ('-', '_');
        p.push_back (std::make_unique<juce::AudioParameterFloat> (prefix + "Lvl",   drumNames[i] + " LVL",   0.0f, 1.0f, 0.8f));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (prefix + "Tune",  drumNames[i] + " TUNE", -12.0f, 12.0f, 0.0f));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (prefix + "Decay", drumNames[i] + " DECAY", 0.05f, 2.0f, 0.3f));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (prefix + "Tone",  drumNames[i] + " TONE",  0.0f, 1.0f, 0.5f));
    }

    return { p.begin(), p.end() };
}

void HuntericeAbyssAudioProcessor::resetDsp()
{
    for (auto& v : voices) { v.active = false; v.releasing = false; v.ampEnv.reset(); v.filterEnv.reset(); v.filter.reset(); }
    for (auto& d : drums) d.active = false;
    delayL.reset(); delayR.reset();
    chorusL.reset(); chorusR.reset();
    reverbL.reset(); reverbR.reset();
    normGain = 1.0f;
}

void HuntericeAbyssAudioProcessor::allNotesOff (bool force)
{
    for (auto& v : voices)
    {
        if (! v.active) continue;
        if (force) { v.active = false; v.ampEnv.reset(); v.filterEnv.reset(); }
        else       { v.releasing = true; v.sustained = false; v.ampEnv.noteOff(); v.filterEnv.noteOff(); }
    }
    for (auto& d : drums) d.active = false;
}

void HuntericeAbyssAudioProcessor::prepareToPlay (double sampleRate, int)
{
    sr = (sampleRate > 0.0 ? sampleRate : 44100.0);

    static const char* ids[NumSmooth] = {
        "volume", "osc2Level", "osc2Detune", "subLevel", "noiseLevel", "masterDrive",
        "filterCutoff", "filterResonance", "filterDrive", "filterKeyTrack", "filterEnvAmount",
        "lfoRate", "lfoDepth", "modLfoToPitch", "modLfoToCutoff", "modEnvToPitch", "modVelToCutoff",
        "distDrive", "distMix", "chorusRate", "chorusDepth", "chorusMix",
        "delayTime", "delayFeedback", "delayMix", "reverbDecay", "reverbMix",
        "unisonSpread", "macroMorph", "patch0Amt", "patch1Amt", "patch2Amt", "patch3Amt",
        "drumVol", "drumDrive",
        "kickLvl", "kickTune", "kickDecay", "kickTone", "kickBody", "kickPitchAmt",
        "snareLvl", "snareTune", "snareDecay", "snareTone", "snareSnap", "snareBody",
        "hi_hatLvl", "hi_hatTune", "hi_hatDecay", "hi_hatTone",
        "clapLvl", "clapTune", "clapDecay", "clapTone",
        "tomLvl", "tomTune", "tomDecay", "tomTone",
        "percLvl", "percTune", "percDecay", "percTone",
        "glide", "velToAmp"
    };

    for (int i = 0; i < NumSmooth; ++i)
    {
        const double ramp = (i == DelayTime ? 0.15 : 0.03);
        sm[(size_t) i].prepare (sr, ramp, parameters.getRawParameterValue (ids[i]));
    }

    bend.reset (sr, 0.01);
    bend.setCurrentAndTargetValue (0.0f);
    reverbL.prepare (sr);
    reverbR.prepare (sr);
    resetDsp();
    for (auto& v : voices) { v.ampEnv.setSampleRate (sr); v.filterEnv.setSampleRate (sr); }
    modWheel = 0.0f;
    sustainPedal = false;
}

void HuntericeAbyssAudioProcessor::releaseResources() {}

void HuntericeAbyssAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    const int numCh = buffer.getNumChannels();
    buffer.clear();
    if (numCh == 0 || numSamples == 0) { midi.clear(); return; }

    const float kt = parameters.getRawParameterValue ("filterKeyTrack")->load();
    wf1   = (Waveform)   juce::jlimit (0, 3, (int) parameters.getRawParameterValue ("waveform1")->load());
    wf2   = (Waveform)   juce::jlimit (0, 3, (int) parameters.getRawParameterValue ("waveform2")->load());
    fType = (FilterType) juce::jlimit (0, 3, (int) parameters.getRawParameterValue ("filterType")->load());
    const int lw = juce::jlimit (0, 4, (int) parameters.getRawParameterValue ("lfoWaveform")->load());
    lfoSH = (lw == 4);
    lfoWf = (Waveform) (lw == 1 ? 3 : (lw == 2 ? 1 : (lw == 3 ? 2 : 0)));
    const bool bypass = parameters.getRawParameterValue ("fxBypass")->load() > 0.5f;

    const int unisonVoices = juce::jlimit (1, MAX_UNISON, (int) parameters.getRawParameterValue ("unisonVoices")->load());
    const float pwmWidth = parameters.getRawParameterValue ("pwmWidth")->load();
    
    // === НОВЫЕ ПАРАМЕТРЫ ===
    const float glideAmt = parameters.getRawParameterValue ("glide")->load();
    const float velToAmt = parameters.getRawParameterValue ("velToAmp")->load();
    const double glideCoef = (glideAmt > 0.001f) ? std::exp (-1.0 / (sr * (0.005 + (double) glideAmt * 0.6))) : 0.0;

    juce::ADSR::Parameters ap, fp;
    ap.attack  = parameters.getRawParameterValue ("attack")->load();
    ap.decay   = parameters.getRawParameterValue ("decay")->load();
    ap.sustain = parameters.getRawParameterValue ("sustain")->load();
    ap.release = parameters.getRawParameterValue ("release")->load();
    fp.attack  = parameters.getRawParameterValue ("filterAttack")->load();
    fp.decay   = parameters.getRawParameterValue ("filterDecay")->load();
    fp.sustain = parameters.getRawParameterValue ("filterSustain")->load();
    fp.release = parameters.getRawParameterValue ("filterRelease")->load();

    for (auto& v : voices)
    {
        v.ampEnv.setParameters (ap);
        v.filterEnv.setParameters (fp);
        v.keyTrackMul = std::pow (2.0f, (float) (v.midiNote - 60) * 0.0833333f * kt);
    }

    for (int i = 0; i < NUM_PATCH; ++i)
    {
        cables[(size_t) i].src = (ModSource) (int) parameters.getRawParameterValue ("patch" + juce::String (i) + "Src")->load();
        cables[(size_t) i].dst = (ModDest)   (int) parameters.getRawParameterValue ("patch" + juce::String (i) + "Dst")->load();
        cables[(size_t) i].on  = cables[(size_t) i].src != ModSource::None && cables[(size_t) i].dst != ModDest::None;
    }

    for (auto& s : sm) s.update();

    juce::MidiBuffer::Iterator it (midi);
    juce::MidiMessage msg;
    int msgPos = 0;
    bool hasNext = it.getNextEvent (msg, msgPos);
    bool dspFault = false;
    const float nyq = (float) sr * 0.45f;

    for (int i = 0; i < numSamples; ++i)
    {
        while (hasNext && msgPos <= i)
        {
            if (msg.isController())
            {
                const int cc = msg.getControllerNumber();
                if (cc == 1)  modWheel = msg.getControllerValue() / 127.0f;
                if (cc == 64)
                {
                    sustainPedal = msg.getControllerValue() >= 64;
                    if (! sustainPedal)
                        for (auto& v : voices)
                            if (v.active && v.sustained) { v.sustained = false; v.releasing = true; v.ampEnv.noteOff(); v.filterEnv.noteOff(); }
                }
                if (cc == 120 || cc == 123) allNotesOff (cc == 120);
            }
            else if (msg.isPitchWheel())
            {
                bend.setTargetValue (((float) msg.getPitchWheelValue() - 8192.0f) / 8192.0f * 2.0f);
            }
            else if (msg.isNoteOn() && msg.getVelocity() > 0)
            {
                const int note = msg.getNoteNumber();
                const float vel = msg.getVelocity() / 127.0f;

                bool isDrum = false;
                for (int d = 0; d < NUM_DRUMS; ++d)
                {
                    if (note != drumMidiNotes[d]) continue;

                    const int base[6] = { KickLvl, SnareLvl, HatLvl, ClapLvl, TomLvl, PercLvl };
                    const int b = base[d];
                    const float lvl  = sm[b + 0].v.getCurrentValue();
                    const float tune = sm[b + 1].v.getCurrentValue();
                    const float dec  = sm[b + 2].v.getCurrentValue();
                    const float ton  = sm[b + 3].v.getCurrentValue();

                    float body = 0.5f, pitchAmt = 1.0f, snap = 0.0f;
                    if (d == 0) { body = sm[KickBody].v.getCurrentValue();  pitchAmt = sm[KickPitchAmt].v.getCurrentValue(); }
                    if (d == 1) { body = sm[SnareBody].v.getCurrentValue(); snap     = sm[SnareSnap].v.getCurrentValue(); }

                    drums[(size_t) d].trigger (vel, lvl, tune, dec, ton, body, pitchAmt, snap);
                    isDrum = true;
                    break;
                }

                if (! isDrum)
                {
                    const float osc2Detune = sm[Osc2Detune].v.getCurrentValue();
                    const float lfoRate    = sm[LfoRate].v.getCurrentValue();
                    const float analogDrift = parameters.getRawParameterValue ("analogDrift")->load();
                    const float unison     = sm[Unison].v.getCurrentValue();

                    int idx = -1;
                    for (int k = 0; k < MAX_VOICES; ++k)
                        if (voices[(size_t) k].active && voices[(size_t) k].midiNote == note && ! voices[(size_t) k].releasing) { idx = k; break; }
                    if (idx < 0)
                        for (int k = 0; k < MAX_VOICES; ++k)
                            if (! voices[(size_t) k].active) { idx = k; break; }
                    if (idx < 0)
                    {
                        juce::uint32 best = 0xffffffffu;
                        for (int k = 0; k < MAX_VOICES; ++k)
                            if (voices[(size_t) k].releasing && voices[(size_t) k].serial < best) { best = voices[(size_t) k].serial; idx = k; }
                        if (idx < 0)
                        {
                            best = 0xffffffffu;
                            for (int k = 0; k < MAX_VOICES; ++k)
                                if (voices[(size_t) k].serial < best) { best = voices[(size_t) k].serial; idx = k; }
                        }
                    }
                    if (idx < 0) idx = 0;
                    
                    voices[(size_t) idx].start (note, (float) juce::MidiMessage::getMidiNoteInHertz (note), sr, osc2Detune, analogDrift, lfoRate, vel, unison, rng, ++noteSerial);
                    voices[(size_t) idx].keyTrackMul = std::pow (2.0f, (float) (note - 60) * 0.0833333f * kt);
                }
            }
            else if (msg.isNoteOff() || (msg.isNoteOn() && msg.getVelocity() == 0))
            {
                const int note = msg.getNoteNumber();
                for (auto& v : voices)
                    if (v.active && v.midiNote == note && ! v.releasing) v.stop (sustainPedal);
            }
            hasNext = it.getNextEvent (msg, msgPos);
        }

        float pv[NumSmooth];
        for (int k = 0; k < NumSmooth; ++k) pv[k] = sm[(size_t) k].next();

        const float macro     = pv[Macro];
        const float osc2Level = pv[Osc2Level];
        const float subLevel  = pv[SubLevel];
        const float noiseMix  = juce::jlimit (0.0f, 1.0f, pv[NoiseLevel] + macro * 0.35f);
        const float baseCut   = juce::jlimit (0.0f, 1.0f, pv[Cutoff] + macro * 0.45f);
        const float res       = juce::jlimit (0.0f, 1.0f, pv[Resonance]);
        const float fDrive    = juce::jlimit (0.0f, 1.0f, pv[FilterDrive] + macro * 0.5f);
        const float mDrive    = juce::jlimit (0.0f, 1.0f, pv[MasterDrive] + macro * 0.6f);
        const float bendMul   = std::pow (2.0f, bend.getNextValue() / 12.0f);

        float mixL = 0.0f, mixR = 0.0f;
        float volMod = 0.0f;
        int activeCount = 0;

        for (auto& v : voices)
        {
            if (! v.active) continue;
            const float env = v.ampEnv.getNextSample();
            const float fe  = v.filterEnv.getNextSample();
            if (! v.ampEnv.isActive()) { v.active = false; continue; }
            ++activeCount;

            // === GLIDE И ЮНИСОН ===
            v.inc1   = v.targetInc1   + (v.inc1   - v.targetInc1)   * glideCoef;
            v.inc2   = v.targetInc2   + (v.inc2   - v.targetInc2)   * glideCoef;
            v.incSub = v.targetIncSub + (v.incSub - v.targetIncSub) * glideCoef;

            if (v.unisonCount != unisonVoices)
            {
                v.unisonCount = unisonVoices;
                for (int j = 0; j < unisonVoices; ++j)
                    v.unisonPhase[j] = v.phase1;
            }

            float lfo;
            v.lfoInc = (double) pv[LfoRate] / sr;
            v.lfoPhase += v.lfoInc;
            if (v.lfoPhase >= 1.0)
            {
                v.lfoPhase -= 1.0;
                if (lfoSH) v.shValue = rng.nextFloat() * 2.0f - 1.0f;
            }
            lfo = lfoSH ? v.shValue : osc ((float) v.lfoPhase, 0.0f, lfoWf);

            float pitchMod  = (fe - 0.5f) * pv[ModEnvPitch] * 0.05f + lfo * pv[ModLfoPitch] * 0.05f;
            float cutoffMod = fe * pv[FilterEnvAmt] + lfo * pv[LfoDepth] + lfo * pv[ModLfoCutoff]
                            + (v.velocity - 0.5f) * pv[ModVelToCutoff] * 0.5f + modWheel * 0.3f;
            float resMod = 0.0f;

            for (int c = 0; c < NUM_PATCH; ++c)
            {
                const auto& cb = cables[(size_t) c];
                if (! cb.on) continue;
                float s = 0.0f;
                switch (cb.src)
                {
                    case ModSource::LFO:       s = lfo; break;
                    case ModSource::FilterEnv: s = fe; break;
                    case ModSource::Velocity:  s = v.velocity; break;
                    case ModSource::ModWheel:  s = modWheel; break;
                    default: continue;
                }
                const float a = s * pv[Patch0 + c];
                switch (cb.dst)
                {
                    case ModDest::FilterCutoff: cutoffMod += a; break;
                    case ModDest::Osc1Pitch:
                    case ModDest::Osc2Pitch:    pitchMod  += a * 0.05f; break;
                    case ModDest::MasterVol:    volMod    += a; break;
                    case ModDest::FilterRes:    resMod    += a; break;
                    default: break;
                }
            }

            // === НАСТОЯЩИЙ ЮНИСОН ===
            float osc1Sum = 0.0f;
            for (int j = 0; j < v.unisonCount; ++j)
            {
                const float spread = (v.unisonCount > 1) ? ((0.002f + pv[Unison] * 0.6f) * (j - (v.unisonCount - 1) * 0.5f)) : 0.0f;
                float detuneFactor = 1.0f + spread + (pitchMod + v.audioPitchMod) * 0.1f;
                float dt = (float) juce::jlimit (0.0, 0.45, v.inc1 * (double) detuneFactor * (double) bendMul);
                
                osc1Sum += osc ((float)v.unisonPhase[j], dt, wf1, pwmWidth);
                
                v.unisonPhase[j] += dt;
                if (v.unisonPhase[j] >= 1.0) v.unisonPhase[j] -= std::floor (v.unisonPhase[j]);
            }
            const float o1 = osc1Sum / (float) v.unisonCount;
            v.phase1 = v.unisonPhase[0];

            const float dt2 = (float) juce::jlimit (0.0, 0.45, v.inc2 * (double) ((1.0f + pitchMod + v.audioPitchMod) * bendMul));
            const float dtS = (float) juce::jlimit (0.0, 0.45, v.incSub * (double) ((1.0f + pitchMod + v.audioPitchMod) * bendMul));

            const float o2 = (osc2Level > 0.0001f) ? osc ((float) v.phase2, dt2, wf2, pwmWidth) : 0.0f;
            const float sb = (subLevel  > 0.0001f) ? ((v.phaseSub < 0.5) ? 1.0f : -1.0f) : 0.0f;
            const float nz = (noiseMix  > 0.0001f) ? noise (v.noiseSeed) : 0.0f;

            v.phase2 += dt2; if (v.phase2 >= 1.0) v.phase2 -= std::floor (v.phase2);
            v.phaseSub += dtS; if (v.phaseSub >= 1.0) v.phaseSub -= std::floor (v.phaseSub);

            float audioCut = 0.0f;
            v.audioPitchMod = 0.0f;
            for (int c = 0; c < NUM_PATCH; ++c)
            {
                const auto& cb = cables[(size_t) c];
                if (! cb.on) continue;
                float s = 0.0f;
                if      (cb.src == ModSource::Osc1Audio)  s = o1;
                else if (cb.src == ModSource::NoiseAudio) s = nz;
                else continue;
                const float a = s * pv[Patch0 + c];
                if      (cb.dst == ModDest::FilterCutoff) audioCut += a * 0.5f;
                else if (cb.dst == ModDest::Osc1Pitch || cb.dst == ModDest::Osc2Pitch) v.audioPitchMod += a * 0.1f;
                else if (cb.dst == ModDest::FilterRes) resMod += a * 0.3f;
            }

            const float cn = juce::jlimit (0.0f, 1.0f, baseCut + cutoffMod + audioCut);
            float fc = 20.0f * std::pow (1000.0f, cn) * v.keyTrackMul;
            fc = juce::jlimit (20.0f, nyq, fc);
            const float g = std::tan (juce::MathConstants<float>::pi * fc / (float) sr);
            const float Q = 0.5f + std::pow (juce::jlimit (0.0f, 1.0f, res + resMod), 2.0f) * 24.5f;
            const float k = juce::jlimit (0.04f, 2.0f, 1.0f / Q);

            float sig = o1 + o2 * osc2Level + sb * subLevel * 0.8f + nz * noiseMix * 0.5f;
            sig = std::tanh (sig * (1.0f + fDrive * 3.0f)) / (1.0f + fDrive * 0.7f);

            const float filtered = v.filter.process (sig, g, k, fType);
            // === VEL -> AMP ===
            const float s = filtered * env * (1.0f - velToAmt * (1.0f - v.velocity));
            mixL += s * v.panL;
            mixR += s * v.panR;
        }

        // === DRUM MACHINE ===
        float drumL = 0.0f, drumR = 0.0f;
        for (int d = 0; d < NUM_DRUMS; ++d)
        {
            const float s = drums[(size_t) d].process (sr);
            drumL += s * 0.7071f;
            drumR += s * 0.7071f;
        }
        const float drumDriveVal = pv[DrumDrive];
        if (drumDriveVal > 0.0001f)
        {
            const float d = 1.0f + drumDriveVal * 3.0f;
            const float comp = 1.0f / std::tanh (d);
            drumL = std::tanh (drumL * d) * comp;
            drumR = std::tanh (drumR * d) * comp;
        }

        const float target = 1.0f / std::sqrt ((float) juce::jmax (1, activeCount));
        normGain += 0.0015f * (target - normGain);
        const float gain = normGain * pv[Volume] * juce::jlimit (0.0f, 2.0f, 1.0f + volMod);
        const float drumGain = pv[DrumVol] * pv[Volume];

        float dryL = mixL * gain + drumL * drumGain;
        float dryR = mixR * gain + drumR * drumGain;

        if (mDrive > 0.0001f)
        {
            const float d = 1.0f + mDrive * 4.0f;
            const float comp = 1.0f / std::tanh (d);
            dryL = std::tanh (dryL * d) * comp;
            dryR = std::tanh (dryR * d) * comp;
        }

        float outL = dryL, outR = dryR;
        if (! bypass)
        {
            // === УЛУЧШЕННЫЙ DISTORTION ===
            const float dm = pv[DistMix];
            if (dm > 0.0001f)
            {
                const float d = 1.0f + pv[DistDrive] * 15.0f;
                float dL = outL * d;
                float dR = outR * d;
                dL = std::tanh (dL) * 0.7f + dL / (1.0f + std::abs (dL)) * 0.3f;
                dR = std::tanh (dR) * 0.7f + dR / (1.0f + std::abs (dR)) * 0.3f;
                const float comp = 1.0f / std::sqrt (d);
                dL *= comp; dR *= comp;
                outL += (dL - outL) * dm;
                outR += (dR - outR) * dm;
            }
            // === УЛУЧШЕННЫЙ CHORUS ===
            const float cm = pv[ChorusMix];
            if (cm > 0.0001f)
            {
                outL = chorusL.process (outL, pv[ChorusRate], pv[ChorusDepth], cm, sr, 0.0f);
                outR = chorusR.process (outR, pv[ChorusRate], pv[ChorusDepth], cm, sr, 0.25f);
            }
            // === УЛУЧШЕННЫЙ DELAY ===
            const float dlm = pv[DelayMix];
            if (dlm > 0.0001f)
            {
                const float len = juce::jlimit (1.0f, (float) (DELAY_MAX - 2), pv[DelayTime] * (float) sr);
                outL += delayL.process (outL, len, pv[DelayFeedback], 0.5f) * dlm;
                outR += delayR.process (outR, len, pv[DelayFeedback], 0.5f) * dlm;
            }
            // === УЛУЧШЕННЫЙ REVERB ===
            const float rm = pv[ReverbMix];
            if (rm > 0.0001f)
            {
                outL = reverbL.process (outL, pv[ReverbDecay], rm);
                outR = reverbR.process (outR, pv[ReverbDecay], rm);
            }
        }

        if (! std::isfinite (outL) || ! std::isfinite (outR))
        {
            outL = outR = 0.0f;
            dspFault = true;
        }
        outL = juce::jlimit (-2.0f, 2.0f, outL);
        outR = juce::jlimit (-2.0f, 2.0f, outR);

        if (numCh == 1) buffer.setSample (0, i, (outL + outR) * 0.5f);
        else
        {
            buffer.setSample (0, i, outL);
            buffer.setSample (1, i, outR);
        }
    }

    if (dspFault) resetDsp();
    midi.clear();
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
    if (xmlState != nullptr && xmlState->hasTagName (parameters.state.getType()))
        parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HuntericeAbyssAudioProcessor();
}
