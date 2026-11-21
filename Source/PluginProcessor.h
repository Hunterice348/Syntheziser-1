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
        float driftOffset = 0.0f; // Для аналогового дрейфа

        juce::ADSR adsr;
        juce::ADSR::Parameters adsrParams;

        void noteOn (float frequency, double sampleRate, float detuneSemitones, float analogDriftAmount)
        {
            active = true;
            // Случайный дрейф высоты тона (до ±1.5 центов) для имитации аналоговой нестабильности
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
    double currentSampleRate = 44100.0;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedDrive;

    inline float generateNoise(float& state) const
    {
        state = state * 1664525.0f + 1013904223.0f;
        return (state * 1.0f / 4294967296.0f) * 2.0f - 1.0f;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HuntericeAbyssAudioProcessor)
};
