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
    double getTailLengthSeconds() const override { return 2.0; } // Увеличено для корректного Release
    
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int index) override {}
    const juce::String getProgramName (int index) override { return {}; }
    void changeProgramName (int index, const juce::String& newName) override {}
    
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::AudioProcessorValueTreeState parameters;

    // --- POLYPHONIC VOICE ENGINE ---
    static constexpr int MAX_VOICES = 16;
    
    struct Voice
    {
        bool active = false;
        int midiNote = -1;
        double phase = 0.0;
        double phaseIncrement = 0.0;
        juce::ADSR adsr;
        juce::ADSR::Parameters adsrParams;

        void noteOn (float frequency, double sampleRate)
        {
            active = true;
            phaseIncrement = frequency / sampleRate;
            phase = 0.0;
            adsr.noteOn();
        }

        void noteOff()
        {
            adsr.noteOff();
        }
    };

    std::array<Voice, MAX_VOICES> voices;
    enum class Waveform { Sine = 0, Saw, Square, Triangle };
    Waveform currentWaveform = Waveform::Sine;
    double currentSampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HuntericeAbyssAudioProcessor)
};
