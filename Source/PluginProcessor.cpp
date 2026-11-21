#include "PluginProcessor.h"
#include "PluginEditor.h"

const int HuntericeAbyssAudioProcessor::drumMidiNotes[NUM_DRUMS];

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

    // DRUM MACHINE (Stage 14)
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
        "kickLvl", "kickTune", "kickDecay", "kickTone",
        "snareLvl", "snareTune", "snareDecay", "snareTone",
        "hi_hatLvl", "hi_hatTune", "hi_hatDecay", "hi_hatTone",
        "clapLvl", "clapTune", "clapDecay", "clapTone",
        "tomLvl", "tomTune", "tomDecay", "tomTone",
        "percLvl", "percTune", "percDecay", "percTone"
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

                // === DRUM MACHINE TRIGGER ===
                bool isDrum = false;
                for (int d = 0; d < NUM_DRUMS; ++d)
                {
                    if (note == drumMidiNotes[d])
                    {
                        const juce::String prefix = juce::StringArray { "kick", "snare", "hi_hat", "clap", "tom", "perc" }[d];
                        const float lvl   = sm[DrumVol + 2 + d * 4].next();
                        const float tune  = sm[DrumVol + 3 + d * 4].next();
                        const float decay = sm[DrumVol + 4 + d * 4].next();
                        const float tone  = sm[DrumVol + 5 + d * 4].next();
                        drums[(size_t) d].trigger (vel, lvl, tune, decay, tone);
                        isDrum = true;
                        break;
                    }
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
                            + (v.velocity - 0.5f) * pv[ModVelCutoff] * 0.5f + modWheel * 0.3f;
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

            const float pf = (1.0f + pitchMod + v.audioPitchMod) * bendMul;
            const float dt1 = (float) juce::jlimit (0.0, 0.45, v.inc1 * (double) pf);
            const float dt2 = (float) juce::jlimit (0.0, 0.45, v.inc2 * (double) pf);
            const float dtS = (float) juce::jlimit (0.0, 0.45, v.incSub * (double) pf);

            const float o1 = osc ((float) v.phase1, dt1, wf1);
            const float o2 = (osc2Level > 0.0001f) ? osc ((float) v.phase2, dt2, wf2) : 0.0f;
            const float sb = (subLevel  > 0.0001f) ? ((v.phaseSub < 0.5) ? 1.0f : -1.0f) : 0.0f;
            const float nz = (noiseMix  > 0.0001f) ? noise (v.noiseSeed) : 0.0f;

            v.phase1 += dt1; if (v.phase1 >= 1.0) v.phase1 -= std::floor (v.phase1);
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
            const float s = filtered * env;
            mixL += s * v.panL;
            mixR += s * v.panR;
        }

        // === DRUM MACHINE RENDER ===
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
        mixL += drumL;
        mixR += drumR;

        const float target = 1.0f / std::sqrt ((float) juce::jmax (1, activeCount));
        normGain += 0.0015f * (target - normGain);
        const float gain = normGain * pv[Volume] * juce::jlimit (0.0f, 2.0f, 1.0f + volMod);

        float dryL = mixL * gain;
        float dryR = mixR * gain;

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
            const float dm = pv[DistMix];
            if (dm > 0.0001f)
            {
                const float d = 1.0f + pv[DistDrive] * 10.0f;
                const float comp = 1.0f / std::tanh (d);
                outL += (std::tanh (outL * d) * comp - outL) * dm;
                outR += (std::tanh (outR * d) * comp - outR) * dm;
            }
            const float cm = pv[ChorusMix];
            if (cm > 0.0001f)
            {
                outL = chorusL.process (outL, pv[ChorusRate], pv[ChorusDepth], cm, sr, 0.0f);
                outR = chorusR.process (outR, pv[ChorusRate], pv[ChorusDepth], cm, sr, 0.25f);
            }
            const float dlm = pv[DelayMix];
            if (dlm > 0.0001f)
            {
                const float len = juce::jlimit (1.0f, (float) (DELAY_MAX - 2), pv[DelayTime] * (float) sr);
                outL += delayL.process (outL, len, pv[DelayFeedback]) * dlm;
                outR += delayR.process (outR, len, pv[DelayFeedback]) * dlm;
            }
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
