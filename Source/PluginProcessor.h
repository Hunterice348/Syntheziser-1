#pragma once
#include <JuceHeader.h>

class HuntericeAbyssAudioProcessor : public juce::AudioProcessor
{
public:
    HuntericeAbyssAudioProcessor();
    ~HuntericeAbyssAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    enum class Waveform   { Sine = 0, Saw, Square, Triangle };
    enum class FilterType { LP12 = 0, LP24, HP, BP };
    enum class ModSource  { None = 0, LFO, FilterEnv, Velocity, ModWheel, Osc1Audio, NoiseAudio, NumSources };
    enum class ModDest    { None = 0, FilterCutoff, Osc1Pitch, Osc2Pitch, MasterVol, FilterRes, NumDests };

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState parameters;

    static constexpr int MAX_VOICES = 16;
    static constexpr int MAX_UNISON = 8;
    static constexpr int NUM_PATCH  = 4;
    static constexpr int DELAY_MAX  = 192000;
    static constexpr int CHORUS_MAX = 8192;
    static constexpr int NUM_DRUMS  = 6;

    double sr = 44100.0;
    juce::Random rng;

    enum Sm { Volume, Osc2Level, Osc2Detune, SubLevel, NoiseLevel, MasterDrive,
              Cutoff, Resonance, FilterDrive, KeyTrack, FilterEnvAmt,
              LfoRate, LfoDepth, ModLfoPitch, ModLfoCutoff, ModEnvPitch, ModVelCutoff,
              DistDrive, DistMix, ChorusRate, ChorusDepth, ChorusMix,
              DelayTime, DelayFeedback, DelayMix, ReverbDecay, ReverbMix,
              Unison, Macro, Patch0, Patch1, Patch2, Patch3,
              DrumVol, DrumDrive,
              KickLvl, KickTune, KickDecay, KickTone, KickBody, KickPitchAmt,
              SnareLvl, SnareTune, SnareDecay, SnareTone, SnareSnap, SnareBody,
              HatLvl, HatTune, HatDecay, HatTone,
              ClapLvl, ClapTune, ClapDecay, ClapTone,
              TomLvl, TomTune, TomDecay, TomTone,
              PercLvl, PercTune, PercDecay, PercTone,
              Glide, VelToAmp,
              NumSmooth };

    struct Smooth
    {
        juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> v;
        std::atomic<float>* p = nullptr;
        void prepare (double sampleRate, double seconds, std::atomic<float>* ptr)
        {
            p = ptr; v.reset (sampleRate, seconds); v.setCurrentAndTargetValue (p->load());
        }
        void update() { v.setTargetValue (p->load()); }
        float next()  { return v.getNextValue(); }
    };

    std::array<Smooth, NumSmooth> sm;

    struct SVF
    {
        float ic1a = 0.f, ic2a = 0.f, ic1b = 0.f, ic2b = 0.f;
        void reset() { ic1a = ic2a = ic1b = ic2b = 0.f; }
        float process (float in, float g, float k, FilterType type)
        {
            const float a1 = 1.0f / (1.0f + g * (g + k));
            const float a2 = g * a1;
            const float a3 = g * a2;
            float v3 = in - ic2a;
            float v1 = a1 * ic1a + a2 * v3;
            float v2 = ic2a + a2 * ic1a + a3 * v3;
            ic1a = 2.0f * v1 - ic1a;
            ic2a = 2.0f * v2 - ic2a;
            const float low  = v2;
            const float band = v1;
            const float high = in - k * v1 - v2;
            if (type == FilterType::HP) return high;
            if (type == FilterType::BP) return band;
            if (type == FilterType::LP12) return low;
            float w3 = low - ic2b;
            float w1 = a1 * ic1b + a2 * w3;
            float w2 = ic2b + a2 * ic1b + a3 * w3;
            ic1b = 2.0f * w1 - ic1b;
            ic2b = 2.0f * w2 - ic2b;
            return w2;
        }
    };

    struct Voice
    {
        bool active = false, releasing = false, sustained = false;
        int midiNote = -1;
        float velocity = 0.0f;
        juce::uint32 serial = 0;
        double phase1 = 0.0, phase2 = 0.0, phaseSub = 0.0;
        double inc1 = 0.0, inc2 = 0.0, incSub = 0.0;
        double targetInc1 = 0.0, targetInc2 = 0.0, targetIncSub = 0.0;
        double lfoPhase = 0.0, lfoInc = 0.0;
        float shValue = 0.0f;
        float audioPitchMod = 0.0f;
        float keyTrackMul = 1.0f;
        float panL = 0.7071f, panR = 0.7071f;
        juce::uint32 noiseSeed = 12345u;
        juce::ADSR ampEnv, filterEnv;
        juce::ADSR::Parameters ampPars, fltPars;
        SVF filter;

        int unisonCount = 1;
        float unisonDetune[MAX_UNISON] = {0};
        double unisonPhase[MAX_UNISON] = {0};

        void start (int note, float freq, double sampleRate, float osc2Detune, float analogDrift,
                    float lfoRate, float vel, float unison, juce::Random& r, juce::uint32 s)
        {
            active = true; releasing = false; sustained = false;
            midiNote = note; velocity = vel; serial = s;
            const float drift = (r.nextFloat() - 0.5f) * 2.0f * analogDrift * 0.015f;
            const float uni   = (r.nextFloat() - 0.5f) * 2.0f * unison;
            const float ratio = 1.0f + drift + uni;
            const float f1 = freq * ratio;
            const float f2 = freq * ratio * std::pow (2.0f, osc2Detune / 12.0f);
            const float fs = freq * 0.5f * ratio;
            targetInc1 = f1 / sampleRate; targetInc2 = f2 / sampleRate; targetIncSub = fs / sampleRate;
            inc1 = targetInc1; inc2 = targetInc2; incSub = targetIncSub;
            lfoInc = lfoRate / sampleRate;
            phase1 = phase2 = phaseSub = 0.0; lfoPhase = 0.0;
            shValue = r.nextFloat() * 2.0f - 1.0f; audioPitchMod = 0.0f;
            noiseSeed = 12345u + s * 2654435761u;
            const float pan = juce::jlimit (-1.0f, 1.0f, (r.nextFloat() * 2.0f - 1.0f) * unison * 20.0f);
            const float ang = (pan + 1.0f) * 0.7853981f;
            panL = std::cos (ang); panR = std::sin (ang);
            filter.reset(); ampEnv.noteOn(); filterEnv.noteOn();
            
            for (int i = 0; i < MAX_UNISON; ++i)
                unisonPhase[i] = phase1;
        }
        void stop (bool sustainPedal)
        {
            if (sustainPedal) { sustained = true; return; }
            releasing = true; ampEnv.noteOff(); filterEnv.noteOff();
        }
    };

    std::array<Voice, MAX_VOICES> voices;
    juce::uint32 noteSerial = 0;
    float normGain = 1.0f;

    struct DrumVoice
    {
        enum Type { Kick = 0, Snare, Hat, Clap, Tom, Perc };
        Type type = Kick;
        bool active = false;
        float level = 1.0f, tune = 0.0f, decay = 0.3f, tone = 0.5f;
        float bodyTone = 0.5f, pitchEnvAmt = 1.0f, snap = 0.0f;

        double phase = 0.0;
        double pitchEnv = 0.0;
        double ampEnvPhase = 0.0;
        float ampEnvVal = 0.0f;
        juce::uint32 noiseSeed = 0xABCD1234u;
        float clapBurstPhase = 0.0f;
        int clapBurstCount = 0;
        float hpState = 0.0f;
        float hpIn = 0.0f;
        float lpState = 0.0f;

        void trigger (float vel, float lvl, float tn, float dec, float ton, float body, float pitch, float snp)
        {
            active = true;
            level = vel * lvl;
            tune = tn;
            decay = dec;
            tone = ton;
            bodyTone = body;
            pitchEnvAmt = pitch;
            snap = snp;
            phase = 0.0;
            pitchEnv = 1.0;
            ampEnvPhase = 0.0;
            ampEnvVal = 1.0f;
            clapBurstPhase = 0.0f;
            clapBurstCount = 0;
            hpState = 0.0f;
            hpIn = 0.0f;
            lpState = 0.0f;
        }

        float process (double sampleRate)
        {
            if (! active) return 0.0f;

            float out = 0.0f;
            const float decayRate = 1.0f / (float) (sampleRate * juce::jlimit (0.05, 3.0, (double) decay));
            const float pitchDecayRate = 1.0f / (float) (sampleRate * 0.05);

            switch (type)
            {
                case Kick:
                {
                    const float baseFreq = 55.0f * std::pow (2.0f, tune / 12.0f) * (0.5f + bodyTone);
                    const float freq = baseFreq * (1.0f + (float) pitchEnv * 3.5f * pitchEnvAmt);
                    phase += freq / sampleRate;
                    if (phase >= 1.0) phase -= 1.0;
                    out = std::sin (6.2831853f * (float) phase);
                    float clickPhase = phase * 12.0f;
                    if (clickPhase >= 1.0f) clickPhase -= std::floor(clickPhase);
                    float clickEnv = std::exp (-150.0f * (float) ampEnvPhase);
                    float click = std::sin (6.2831853f * clickPhase) * clickEnv;
                    out += click * 0.5f;
                    pitchEnv -= pitchDecayRate;
                    if (pitchEnv < 0.0) pitchEnv = 0.0;
                    break;
                }
                case Snare:
                {
                    const float baseFreq = 200.0f * std::pow (2.0f, tune / 12.0f) * (0.5f + bodyTone);
                    phase += baseFreq / sampleRate;
                    if (phase >= 1.0) phase -= 1.0;
                    float tri = ((float) phase < 0.5f) ? (4.0f * (float) phase - 1.0f) : (3.0f - 4.0f * (float) phase);
                    float nz = noise (noiseSeed);
                    out = tri * 0.4f + nz * 0.8f;
                    if (snap > 0.0f)
                    {
                        float snapEnv = std::exp (-50.0f * (float) ampEnvPhase);
                        out += snapEnv * snap * 0.5f;
                    }
                    break;
                }
                case Hat:
                {
                    float nz = noise (noiseSeed);
                    const float cutoff = 4000.0f * std::pow (10.0f, (tone - 0.5f) * 2.0f);
                    const float cutoffClamped = juce::jlimit (200.0f, (float)(sampleRate * 0.45), cutoff);
                    const float g = std::tan (juce::MathConstants<float>::pi * cutoffClamped / (float) sampleRate);
                    const float a1 = g / (1.0f + g);
                    float y = a1 * (hpState + nz - hpIn);
                    hpState = y;
                    hpIn = nz;
                    out = y;
                    break;
                }
                case Clap:
                {
                    float nz = noise (noiseSeed);
                    const float burstSpacing = 0.003f;
                    clapBurstPhase += (float) (1.0 / sampleRate);
                    if (clapBurstPhase >= burstSpacing && clapBurstCount < 3)
                    {
                        clapBurstPhase = 0.0f;
                        clapBurstCount++;
                        ampEnvVal = 1.0f;
                    }
                    out = nz * ampEnvVal;
                    break;
                }
                case Tom:
                {
                    const float baseFreq = 120.0f * std::pow (2.0f, tune / 12.0f);
                    const float freq = baseFreq * (1.0f + (float) pitchEnv * 1.0f);
                    phase += freq / sampleRate;
                    if (phase >= 1.0) phase -= 1.0;
                    out = std::sin (6.2831853f * (float) phase);
                    pitchEnv -= pitchDecayRate * 0.7;
                    if (pitchEnv < 0.0) pitchEnv = 0.0;
                    break;
                }
                case Perc:
                {
                    const float baseFreq = 800.0f * std::pow (2.0f, tune / 12.0f);
                    phase += baseFreq / sampleRate;
                    if (phase >= 1.0) phase -= 1.0;
                    float bell = std::sin (6.2831853f * (float) phase) * 0.6f + noise (noiseSeed) * 0.4f;
                    out = bell;
                    break;
                }
            }

            ampEnvPhase += decayRate;
            ampEnvVal = std::exp (-5.0f * (float) ampEnvPhase);
            if (ampEnvPhase >= 1.0 || ampEnvVal < 0.001f)
            {
                active = false;
                ampEnvVal = 0.0f;
            }

            float toneNorm = juce::jlimit (0.0f, 1.0f, tone);
            float alpha = toneNorm * toneNorm;
            lpState = lpState + alpha * (out - lpState);
            out = lpState;

            if (toneNorm > 0.65f)
            {
                float drive = (toneNorm - 0.65f) * 4.0f;
                out = std::tanh (out * (1.0f + drive));
            }

            return out * level * ampEnvVal;
        }

        static inline float noise (juce::uint32& s)
        {
            s = s * 1664525u + 1013904223u;
            return (float) (juce::int32 (s)) * (1.0f / 2147483648.0f);
        }
    };

    std::array<DrumVoice, NUM_DRUMS> drums;
    static constexpr int drumMidiNotes[NUM_DRUMS] = { 36, 38, 42, 45, 48, 51 };

    struct Delay
    {
        std::array<float, DELAY_MAX> buf {};
        int wp = 0;
        float lpState = 0.0f;
        void reset() { buf.fill (0.0f); wp = 0; lpState = 0.0f; }
        float process (float in, float len, float fb, float tone = 0.5f)
        {
            float rp = (float) wp - len;
            while (rp < 0.0f) rp += (float) DELAY_MAX;
            const int i0 = (int) rp; const int i1 = (i0 + 1) % DELAY_MAX;
            const float fr = rp - (float) i0;
            const float d = buf[(size_t) i0] + (buf[(size_t) i1] - buf[(size_t) i0]) * fr;
            const float alpha = 0.1f + tone * 0.8f;
            lpState = lpState + alpha * (d - lpState);
            buf[(size_t) wp] = in + lpState * fb;
            wp = (wp + 1) % DELAY_MAX;
            return d;
        }
    };

    struct Chorus
    {
        std::array<float, CHORUS_MAX> buf {};
        int wp = 0;
        double ph1 = 0.0, ph2 = 0.0;
        void reset() { buf.fill (0.0f); wp = 0; ph1 = ph2 = 0.0; }
        float process (float in, float rate, float depth, float mix, double sampleRate, float phaseOffset)
        {
            ph1 += rate / sampleRate; if (ph1 >= 1.0) ph1 -= 1.0;
            ph2 += (rate * 0.71) / sampleRate; if (ph2 >= 1.0) ph2 -= 1.0;
            double p1 = ph1 + phaseOffset; if (p1 >= 1.0) p1 -= 1.0;
            double p2 = ph2 + phaseOffset * 1.3; if (p2 >= 1.0) p2 -= 1.0;
            const float lfo1 = std::sin (6.2831853f * (float) p1);
            const float lfo2 = std::sin (6.2831853f * (float) p2);
            const float lfo = lfo1 * 0.6f + lfo2 * 0.4f;
            float len = (0.008f + lfo * depth * 0.004f) * (float) sampleRate;
            len = juce::jlimit (1.0f, (float) (CHORUS_MAX - 2), len);
            float rp = (float) wp - len;
            while (rp < 0.0f) rp += (float) CHORUS_MAX;
            const int i0 = (int) rp; const int i1 = (i0 + 1) % CHORUS_MAX;
            const float fr = rp - (float) i0;
            const float wet = buf[(size_t) i0] + (buf[(size_t) i1] - buf[(size_t) i0]) * fr;
            buf[(size_t) wp] = in;
            wp = (wp + 1) % CHORUS_MAX;
            return in + (wet - in) * mix;
        }
    };

    struct Reverb
    {
        static constexpr int CMAX = 4096, AMAX = 2048;
        std::array<float, CMAX> c[8] {};
        std::array<float, AMAX> a[4] {};
        int cl[8] { 1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617 };
        int cp[8] { 0, 0, 0, 0, 0, 0, 0, 0 };
        int al[4] { 225, 556, 441, 341 };
        int ap[4] { 0, 0, 0, 0 };
        float cd[8] { 0, 0, 0, 0, 0, 0, 0, 0 };
        float lp[8] { 0, 0, 0, 0, 0, 0, 0, 0 };

        void prepare (double sampleRate)
        {
            const float s = (float) (sampleRate / 44100.0);
            const int baseC[8] { 1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617 };
            const int baseA[4] { 225, 556, 441, 341 };
            for (int i = 0; i < 8; ++i) cl[i] = juce::jlimit (1, CMAX - 1, (int) (baseC[i] * s));
            for (int i = 0; i < 4; ++i) al[i] = juce::jlimit (1, AMAX - 1, (int) (baseA[i] * s));
            reset();
        }
        void reset()
        {
            for (auto& x : c) x.fill (0.0f);
            for (auto& x : a) x.fill (0.0f);
            for (int i = 0; i < 8; ++i) { cp[i] = 0; cd[i] = 0.0f; lp[i] = 0.0f; }
            for (int i = 0; i < 4; ++i) ap[i] = 0;
        }
        float process (float in, float decay, float mix)
        {
            const float fb = 0.55f + decay * 0.42f;
            const float damp = 0.2f + decay * 0.5f;
            float acc = 0.0f;
            for (int i = 0; i < 8; ++i)
            {
                const float out = c[(size_t) i][(size_t) cp[i]];
                lp[i] = out * (1.0f - damp) + lp[i] * damp;
                c[(size_t) i][(size_t) cp[i]] = in + lp[i] * fb;
                cp[i] = (cp[i] + 1) % cl[i];
                acc += out;
            }
            acc *= 0.125f;
            for (int i = 0; i < 4; ++i)
            {
                const float bo = a[(size_t) i][(size_t) ap[i]];
                const float v = acc - bo * 0.5f;
                a[(size_t) i][(size_t) ap[i]] = v;
                acc = bo + v * 0.5f;
                ap[i] = (ap[i] + 1) % al[i];
            }
            return in + (acc - in) * mix;
        }
    };

    Delay  delayL, delayR;
    Chorus chorusL, chorusR;
    Reverb reverbL, reverbR;

    float modWheel = 0.0f;
    bool  sustainPedal = false;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> bend;

    struct Cable { ModSource src = ModSource::None; ModDest dst = ModDest::None; bool on = false; };
    std::array<Cable, NUM_PATCH> cables;

    Waveform   wf1 = Waveform::Sine, wf2 = Waveform::Saw, lfoWf = Waveform::Sine;
    FilterType fType = FilterType::LP12;
    bool lfoSH = false;

    static inline float blep (float t, float dt)
    {
        if (dt <= 0.0f) return 0.0f;
        if (t < dt)      { t /= dt; return t + t - t * t - 1.0f; }
        if (t > 1.0f - dt) { t = (t - 1.0f) / dt; return t * t + t + t + 1.0f; }
        return 0.0f;
    }

    static inline float osc (float ph, float dt, Waveform w, float pwm = 0.5f)
    {
        switch (w)
        {
            case Waveform::Sine: return std::sin (6.2831853f * ph);
            case Waveform::Saw:  return (2.0f * ph - 1.0f) - blep (ph, dt);
            case Waveform::Square:
            {
                float s = (ph < pwm) ? 1.0f : -1.0f;
                s += blep (ph, dt);
                float t2 = ph - pwm; if (t2 < 0.0f) t2 += 1.0f;
                s -= blep (t2, dt);
                return s;
            }
            case Waveform::Triangle:
            default:
                if (ph < 0.25f) return 4.0f * ph;
                if (ph < 0.75f) return 2.0f - 4.0f * ph;
                return 4.0f * ph - 4.0f;
        }
    }

    static inline float noise (juce::uint32& s)
    {
        s = s * 1664525u + 1013904223u;
        return (float) (juce::int32 (s)) * (1.0f / 2147483648.0f);
    }

    void allNotesOff (bool force);
    void resetDsp();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HuntericeAbyssAudioProcessor)
};
