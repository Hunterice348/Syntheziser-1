#pragma once
#include <JuceHeader.h>

class HuntericeAbyssAudioProcessor : public juce::AudioProcessor
{
public:
    HuntericeAbyssAudioProcessor();
    ~HuntericeAbyssAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }
    
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int index) override {}
    const juce::String getProgramName (int index) override { return {}; }
    void changeProgramName (int index, const juce::String& newName) override {}
    
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    enum class Waveform { Sine = 0, Saw, Square, Triangle };
    enum class FilterType { LP12 = 0, LP24, HP, BP };
    
    enum class ModSource { None = 0, LFO, FilterEnv, Velocity, ModWheel, Osc1Audio, NoiseAudio, NumSources };
    enum class ModDest { None = 0, FilterCutoff, Osc1Pitch, Osc2Pitch, MasterVol, FilterRes, NumDests };

    struct PatchCable {
        bool active = false;
        ModSource source = ModSource::None;
        ModDest destination = ModDest::None;
        float amount = 0.5f;
    };

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState parameters;

    static constexpr int MAX_VOICES = 16;
    static constexpr int NUM_PATCH_CABLES = 4;
    static constexpr int MAX_DELAY_SAMPLES = 192000;
    
    struct Voice
    {
        bool active = false;
        int midiNote = -1;
        float velocity = 0.0f;
        
        double phase1 = 0.0, phaseInc1 = 0.0;
        double phase2 = 0.0, phaseInc2 = 0.0;
        double phaseSub = 0.0, phaseIncSub = 0.0;
        float noiseState = 0.0f;
        float driftOffset = 0.0f;

        juce::ADSR adsr;
        juce::ADSR::Parameters adsrParams;
        
        juce::ADSR filterAdsr;
        juce::ADSR::Parameters filterAdsrParams;
        
        double lfoPhase = 0.0;
        double lfoPhaseInc = 0.0;
        float lfoSHValue = 0.0f;

        void noteOn (float frequency, double sampleRate, float detuneSemitones, float analogDriftAmount, float lfoRateHz, float vel)
        {
            active = true;
            velocity = vel;
            driftOffset = (juce::Random::getSystemRandom().nextFloat() - 0.5f) * 2.0f * analogDriftAmount * 0.015f;
            
            float freq1 = frequency * (1.0f + driftOffset);
            float freq2 = frequency * std::pow(2.0f, (detuneSemitones + driftOffset) / 12.0f);
            float freqSub = (frequency * 0.5f) * (1.0f + driftOffset);

            phaseInc1 = freq1 / sampleRate;
            phaseInc2 = freq2 / sampleRate;
            phaseIncSub = freqSub / sampleRate;
            
            phase1 = 0.0; phase2 = 0.0; phaseSub = 0.0;
            noiseState = static_cast<float>(juce::Random::getSystemRandom().nextInt());
            
            lfoPhaseInc = lfoRateHz / sampleRate;
            lfoPhase = 0.0;
            lfoSHValue = (juce::Random::getSystemRandom().nextFloat() * 2.0f) - 1.0f;
            
            adsr.noteOn();
            filterAdsr.noteOn();
        }

        void noteOff() { 
            adsr.noteOff(); 
            filterAdsr.noteOff(); 
        }
    };

    std::array<Voice, MAX_VOICES> voices;
    std::array<PatchCable, NUM_PATCH_CABLES> patchCables;
    
    Waveform currentWaveform1 = Waveform::Sine;
    Waveform currentWaveform2 = Waveform::Saw;
    FilterType currentFilterType = FilterType::LP12;
    double currentSampleRate = 44100.0;
    int lastNotePlayed = 60;
    float currentModWheel = 0.0f;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedDrive;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedCutoff;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedResonance;

    struct SVFilter
    {
        float low = 0.0f, band = 0.0f, high = 0.0f;
        void reset() { low = band = high = 0.0f; }
        
        float process (float input, float cutoffNorm, float resonance, float drive, FilterType type)
        {
            float freq = 2.0f * std::sin (juce::MathConstants<float>::pi * cutoffNorm * 0.499f);
            float q = 1.0f - juce::jlimit (0.01f, 0.99f, resonance);
            float inputWithDrive = std::tanh (input * (1.0f + drive * 3.0f));
            
            low  += freq * band;
            high  = inputWithDrive - low - q * band;
            band += freq * high;
            band = std::tanh (band * (1.0f + drive * 0.5f));
            
            switch (type)
            {
                case FilterType::LP12: return low;
                case FilterType::LP24: return low * low * 4.0f;
                case FilterType::HP:   return high;
                case FilterType::BP:   return band;
                default:               return low;
            }
        }
    };
    
    SVFilter filterL, filterR;

    // --- EFFECTS ENGINE (Stage 11) ---
    struct DelayLine
    {
        std::array<float, MAX_DELAY_SAMPLES> buffer {};
        int writePos = 0;
        
        void reset()
        {
            buffer.fill (0.0f);
            writePos = 0;
        }
        
        float process (float input, int delaySamples, float feedback)
        {
            int readPos = writePos - delaySamples;
            if (readPos < 0) readPos += MAX_DELAY_SAMPLES;
            
            float delayed = buffer[readPos];
            buffer[writePos] = input + delayed * feedback;
            
            writePos++;
            if (writePos >= MAX_DELAY_SAMPLES) writePos = 0;
            
            return delayed;
        }
    };
    
    // ⚠️ ИСПРАВЛЕНИЕ: sampleRate теперь передаётся как параметр
    struct ChorusEffect
    {
        std::array<float, MAX_DELAY_SAMPLES> buffer {};
        int writePos = 0;
        double lfoPhase = 0.0;
        
        void reset()
        {
            buffer.fill (0.0f);
            writePos = 0;
            lfoPhase = 0.0;
        }
        
        // ⚠️ ДОБАВЛЕН ПАРАМЕТР sampleRate
        float process (float input, float rate, float depth, float mix, double sampleRate)
        {
            float lfoValue = std::sin (2.0f * juce::MathConstants<float>::pi * (float)lfoPhase);
            lfoPhase += rate / sampleRate;
            if (lfoPhase >= 1.0) lfoPhase -= 1.0;
            
            float baseDelay = 0.01f * (float)sampleRate;
            float modDelay = baseDelay + lfoValue * depth * 0.005f * (float)sampleRate;
            
            int readPos = writePos - (int)modDelay;
            if (readPos < 0) readPos += MAX_DELAY_SAMPLES;
            
            buffer[writePos] = input;
            writePos++;
            if (writePos >= MAX_DELAY_SAMPLES) writePos = 0;
            
            float wet = buffer[readPos];
            return input * (1.0f - mix) + wet * mix;
        }
    };
    
    struct ReverbEffect
    {
        std::array<float, 1200> comb1 {};
        std::array<float, 1600> comb2 {};
        std::array<float, 2000> comb3 {};
        std::array<float, 2400> comb4 {};
        int pos1 = 0, pos2 = 0, pos3 = 0, pos4 = 0;
        
        void reset()
        {
            comb1.fill (0.0f); comb2.fill (0.0f);
            comb3.fill (0.0f); comb4.fill (0.0f);
            pos1 = pos2 = pos3 = pos4 = 0;
        }
        
        float process (float input, float decay, float mix)
        {
            float feedback = juce::jlimit (0.0f, 0.95f, decay);
            
            float out1 = comb1[pos1];
            comb1[pos1] = input + out1 * feedback;
            pos1 = (pos1 + 1) % comb1.size();
            
            float out2 = comb2[pos2];
            comb2[pos2] = input + out2 * feedback;
            pos2 = (pos2 + 1) % comb2.size();
            
            float out3 = comb3[pos3];
            comb3[pos3] = input + out3 * feedback;
            pos3 = (pos3 + 1) % comb3.size();
            
            float out4 = comb4[pos4];
            comb4[pos4] = input + out4 * feedback;
            pos4 = (pos4 + 1) % comb4.size();
            
            float wet = (out1 + out2 + out3 + out4) * 0.25f;
            return input * (1.0f - mix) + wet * mix;
        }
    };
    
    DelayLine delayL, delayR;
    ChorusEffect chorusL, chorusR;
    ReverbEffect reverbL, reverbR;

    inline float generateNoise(float& state) const
    {
        state = state * 1664525.0f + 1013904223.0f;
        return (state * 1.0f / 4294967296.0f) * 2.0f - 1.0f;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HuntericeAbyssAudioProcessor)
};
