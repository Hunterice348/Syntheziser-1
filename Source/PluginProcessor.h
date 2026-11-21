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

    static constexpr int MAX_VOICES  = 16;
    static constexpr int NUM_PATCH   = 4;
    static constexpr int DELAY_MAX   = 192000;
    static constexpr int CHORUS_MAX  = 8192;

    double sr = 44100.0;
    juce::Random rng;

    // ---------- СГЛАЖИВАНИЕ ВСЕХ ПАРАМЕТРОВ ----------
    enum Sm { Volume, Osc2Level, Osc2Detune, SubLevel, NoiseLevel, MasterDrive,
              Cutoff, Resonance, FilterDrive, KeyTrack, FilterEnvAmt,
              LfoRate, LfoDepth, ModLfoPitch, ModLfoCutoff, ModEnvPitch, ModVelCutoff,
              DistDrive, DistMix, ChorusRate, ChorusDepth, ChorusMix,
              DelayTime, DelayFeedback, DelayMix, ReverbDecay, ReverbMix,
              Unison, Macro, Patch0, Patch1, Patch2, Patch3, NumSmooth };

    struct Smooth
    {
        juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> v;
        std::atomic<float>* p = nullptr;

        void prepare (double sampleRate, double seconds, std::atomic<float>* ptr)
        {
            p = ptr;
            v.reset (sampleRate, seconds);
            v.setCurrentAndTargetValue (p->load());
        }
        void update() { v.setTargetValue (p->load()); }
        float next()  { return v.getNextValue(); }
    };

    std::array<Smooth, NumSmooth> sm;

    // ---------- ФИЛЬТР (устойчивый SVF Simper, 2 каскада) ----------
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

            // LP24: второй каскад
            float w3 = low - ic2b;
            float w1 = a1 * ic1b + a2 * w3;
            float w2 = ic2b + a2 * ic1b + a3 * w3;
            ic1b = 2.0f * w1 - ic1b;
            ic2b = 2.0f * w2 - ic2b;
            return w2;
        }
    };

    // ---------- ГОЛОС ----------
    struct NoteParams
    {
        double sampleRate  = 44100.0;
        float  osc2Detune  = 0.0f;
        float  analogDrift = 0.0f;
        float  lfoRate     = 2.0f;
        float  velocity    = 1.0f;
        float  unison      = 0.0f;
    };

    struct Voice
    {
        bool  active = false, releasing = false, sustained = false;
        int   midiNote = -1;
        float velocity = 0.0f;
        juce::uint32 serial = 0;

        double phase1 = 0.0, phase2 = 0.0, phaseSub = 0.0;
        double inc1 = 0.0, inc2 = 0.0, incSub = 0.0;
        double lfoPhase = 0.0, lfoInc = 0.0;
        float shValue = 0.0f;
        float audioPitchMod = 0.0f;
        float keyTrackMul = 1.0f;
        float panL = 0.7071f, panR = 0.7071f;
        juce::uint32 noiseSeed = 12345u;

        juce::ADSR ampEnv, filterEnv;
        juce::ADSR::Parameters ampPars, fltPars;
        SVF filter;

        void start (int note, float freq, const NoteParams& p, juce::Random& r, juce::uint32 s)
        {
            active = true; releasing = false; sustained = false;
            midiNote = note; velocity = p.velocity; serial = s;

            const float drift = (r.nextFloat() - 0.5f) * 2.0f * p.analogDrift * 0.015f;
            const float uni   = (r.nextFloat() - 0.5f) * 2.0f * p.unison;
            const float ratio = 1.0f + drift + uni;

            const float f1 = freq * ratio;
            const float f2 = freq * ratio * std::pow (2.0f, p.osc2Detune / 12.0f);
            const float fs = freq * 0.5f * ratio;

            inc1   = f1 / p.sampleRate;
            inc2   = f2 / p.sampleRate;
            incSub = fs / p.sampleRate;
            lfoInc = p.lfoRate / p.sampleRate;

            phase1 = phase2 = phaseSub = 0.0;
            lfoPhase = 0.0;
            shValue = r.nextFloat() * 2.0f - 1.0f;
            audioPitchMod = 0.0f;
            noiseSeed = 12345u + s * 2654435761u;

            // equal-power панорама для ширины Unison
            const float pan = juce::jlimit (-1.0f, 1.0f, (r.nextFloat() * 2.0f - 1.0f) * p.unison * 20.0f);
            const float ang = (pan + 1.0f) * 0.7853981f;
            panL = std::cos (ang);
            panR = std::sin (ang);

            filter.reset();
            ampEnv.noteOn();
            filterEnv.noteOn();
        }

        void stop (bool sustainPedal)
        {
            if (sustainPedal) { sustained = true; return; }
            releasing = true;
            ampEnv.noteOff();
            filterEnv.noteOff();
        }
    };

    std::array<Voice, MAX_VOICES> voices;
    juce::uint32 noteSerial = 0;
    float normGain = 1.0f;

    // ---------- ЭФФЕКТЫ ----------
    struct Delay
    {
        std::array<float, DELAY_MAX> buf {};
        int wp = 0;
        void reset() { buf.fill (0.0f); wp = 0; }
        float process (float in, float len, float fb)
        {
            float rp = (float) wp - len;
            while (rp < 0.0f) rp += (float) DELAY_MAX;
            const int i0 = (int) rp;
            const int i1 = (i0 + 1) % DELAY_MAX;
            const float fr = rp - (float) i0;
            const float d = buf[(size_t) i0] + (buf[(size_t) i1] - buf[(size_t) i0]) * fr;
            buf[(size_t) wp] = in + d * fb;
            wp = (wp + 1) % DELAY_MAX;
            return d;
        }
    };

    struct Chorus
    {
        std::array<float, CHORUS_MAX> buf {};
        int wp = 0;
        double ph = 0.0;
        void reset() { buf.fill (0.0f); wp = 0; ph = 0.0; }
        float process (float in, float rate, float depth, float mix, double sampleRate, float phaseOffset)
        {
            ph += rate / sampleRate;
            if (ph >= 1.0) ph -= 1.0;
            double p = ph + phaseOffset;
            if (p >= 1.0) p -= 1.0;
            const float lfo = std::sin (6.2831853f * (float) p);
            float len = (0.008f + lfo * depth * 0.004f) * (float) sampleRate;
            len = juce::jlimit (1.0f, (float) (CHORUS_MAX - 2), len);
            float rp = (float) wp - len;
            while (rp < 0.0f) rp += (float) CHORUS_MAX;
            const int i0 = (int) rp;
            const int i1 = (i0 + 1) % CHORUS_MAX;
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
        std::array<float, CMAX> c[4] {};
        std::array<float, AMAX> a[2] {};
        int cl[4] { 1116, 1188, 1277, 1356 }, cp[4] { 0, 0, 0, 0 };
        int al[2] { 556, 441 }, ap[2] { 0, 0 };
        float cd[4] { 0, 0, 0, 0 };

        void prepare (double sampleRate)
        {
            const float s = (float) (sampleRate / 44100.0);
            const int base[4] { 1116, 1188, 1277, 1356 };
            const int ab[2] { 556, 441 };
            for (int i = 0; i < 4; ++i) cl[i] = juce::jlimit (1, CMAX - 1, (int) (base[i] * s));
            for (int i = 0; i < 2; ++i) al[i] = juce::jlimit (1, AMAX - 1, (int) (ab[i] * s));
            reset();
        }
        void reset()
        {
            for (auto& x : c) x.fill (0.0f);
            for (auto& x : a) x.fill (0.0f);
            for (int i = 0; i < 4; ++i) { cp[i] = 0; cd[i] = 0.0f; }
            for (int i = 0; i < 2; ++i) ap[i] = 0;
        }
        float process (float in, float decay, float mix)
        {
            const float fb = 0.55f + decay * 0.42f;
            const float damp = 0.35f;
            float acc = 0.0f;
            for (int i = 0; i < 4; ++i)
            {
                const float out = c[(size_t) i][(size_t) cp[i]];
                cd[i] = out * (1.0f - damp) + cd[i] * damp;
                c[(size_t) i][(size_t) cp[i]] = in + cd[i] * fb;
                cp[i] = (cp[i] + 1) % cl[i];
                acc += out;
            }
            acc *= 0.25f;
            for (int i = 0; i < 2; ++i)
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

    // ---------- MIDI-СОСТОЯНИЕ ----------
    float modWheel = 0.0f;
    bool  sustainPedal = false;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> bend;

    // ---------- ПАТЧ-БЕЙ ----------
    struct Cable { ModSource src = ModSource::None; ModDest dst = ModDest::None; bool on = false; };
    std::array<Cable, NUM_PATCH> cables;

    Waveform   wf1 = Waveform::Sine, wf2 = Waveform::Saw, lfoWf = Waveform::Sine;
    FilterType fType = FilterType::LP12;
    bool lfoSH = false;

    // ---------- УТИЛИТЫ ----------
    static inline float blep (float t, float dt)
    {
        if (dt <= 0.0f) return 0.0f;
        if (t < dt)      { t /= dt; return t + t - t * t - 1.0f; }
        if (t > 1.0f - dt) { t = (t - 1.0f) / dt; return t * t + t + t + 1.0f; }
        return 0.0f;
    }

    static inline float osc (float ph, float dt, Waveform w)
    {
        switch (w)
        {
            case Waveform::Sine: return std::sin (6.2831853f * ph);
            case Waveform::Saw:  return (2.0f * ph - 1.0f) - blep (ph, dt);
            case Waveform::Square:
            {
                float s = (ph < 0.5f) ? 1.0f : -1.0f;
                s += blep (ph, dt);
                float t2 = ph + 0.5f; if (t2 >= 1.0f) t2 -= 1.0f;
                return s - blep (t2, dt);
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
