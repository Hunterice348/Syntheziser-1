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
    double getTailLengthSeconds() const override { return 2.0; }
    
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int index) override {}
    const juce::String getProgramName (int index) override { return {}; }
    void changeProgramName (int index, const juce::String& newName) override {}
    
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    enum class Waveform { Sine = 0, Saw, Square, Triangle };
    enum class FilterType { LP12 = 0, LP24, HP, BP };

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState parameters;

    static constexpr int MAX_VOICES = 16;
    
    struct Voice
    {
        bool active = false;
        int midiNote = -1;
        
        double phase1 = 0.0, phaseInc1 = 0.0;
        double phase2 = 0.0, phaseInc2 = 0.0;
        double phaseSub = 0.0, phaseIncSub = 0.0;
        float noiseState = 0.0f;
        float driftOffset = 0.0f;

        juce::ADSR adsr;
        juce::ADSR::Parameters adsrParams;

        void noteOn (float frequency, double sampleRate, float detuneSemitones, float analogDriftAmount)
        {
            active = true;
            driftOffset = (juce::Random::getSystemRandom().nextFloat() - 0.5f) * 2.0f * analogDriftAmount * 0.015f;
            
            float freq1 = frequency * (1.0f + driftOffset);
            float freq2 = frequency * std::pow(2.0f, (detuneSemitones + driftOffset) / 12.0f);
            float freqSub = (frequency * 0.5f) * (1.0f + driftOffset);

            phaseInc1 = freq1 / sampleRate;
            phaseInc2 = freq2 / sampleRate;
            phaseIncSub = freqSub / sampleRate;
            
            phase1 = 0.0; phase2 = 0.0; phaseSub = 0.0;
            noiseState = static_cast<float>(juce::Random::getSystemRandom().nextInt());
            
            adsr.noteOn();
        }

        void noteOff() { adsr.noteOff(); }
    };

    std::array<Voice, MAX_VOICES> voices;
    
    Waveform currentWaveform1 = Waveform::Sine;
    Waveform currentWaveform2 = Waveform::Saw;
    FilterType currentFilterType = FilterType::LP12;
    double currentSampleRate = 44100.0;
    int lastNotePlayed = 60; // Для key tracking

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedDrive;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedCutoff;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedResonance;

    // --- STATE VARIABLE FILTER (SVF) ---
    // Стерео фильтр (по одному на канал)
    struct SVFilter
    {
        float low = 0.0f, band = 0.0f, high = 0.0f;
        
        void reset() { low = band = high = 0.0f; }
        
        // Обработка одного сэмпла, возвращает выбранный тип выхода
        float process (float input, float cutoffNorm, float resonance, float drive, FilterType type)
        {
            // cutoffNorm: 0..1 -> преобразуем в частоту в радианах
            // Используем sin для стабильности при высоких частотах
            float freq = 2.0f * std::sin (juce::MathConstants<float>::pi * cutoffNorm * 0.499f);
            
            // Ограничение resonance для предотвращения нестабильности
            // 1.0 = нет резонанса, 0.01 = почти self-oscillation
            float q = 1.0f - juce::jlimit (0.01f, 0.99f, resonance);
            
            // Применение drive (мягкое насыщение внутри фильтра для analog feel)
            float inputWithDrive = std::tanh (input * (1.0f + drive * 3.0f));
            
            // Chamberlin SVF core
            low  += freq * band;
            high  = inputWithDrive - low - q * band;
            band += freq * high;
            
            // Нелинейное насыщение band-прохода для warmth
            band = std::tanh (band * (1.0f + drive * 0.5f));
            
            switch (type)
            {
                case FilterType::LP12:
                    return low;
                case FilterType::LP24:
                    // Каскадирование двух SVF для 24dB/oct
                    // Используем простой подход: применяем SVF дважды
                    return low * low * 4.0f; // Упрощённая эмуляция 24dB
                case FilterType::HP:
                    return high;
                case FilterType::BP:
                    return band;
                default:
                    return low;
            }
        }
    };
    
    SVFilter filterL, filterR;

    inline float generateNoise(float& state) const
    {
        state = state * 1664525.0f + 1013904223.0f;
        return (state * 1.0f / 4294967296.0f) * 2.0f - 1.0f;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HuntericeAbyssAudioProcessor)
};
