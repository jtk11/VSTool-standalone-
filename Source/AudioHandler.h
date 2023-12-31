#pragma once
#include "../JuceLibraryCode/JuceHeader.h"

class AudioHandler
{
public:
    AudioHandler();
   virtual ~AudioHandler();

    void loadFiles(const juce::Array<juce::File>& files);  // Load multiple files
    void initializeAudio();
    void start();
    void stop();
    void setLooping(bool shouldLoop);
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);
    void setMixLevels(float topLeft, float topRight, float bottomLeft, float bottomRight);
    void startPlaying();
    void detuneSounds();
    void stopSample(int index);
    void stopAllSounds();
    void startSample(int index);
    void startRecording(const juce::File& file);
    void stopRecording();



private:
    juce::AudioDeviceManager deviceManager;
    juce::AudioFormatManager formatManager;
    juce::MixerAudioSource mixerSource;
    std::unique_ptr<juce::AudioFormatWriter> writer;
    juce::File outputFile;
    //bool isRecording = false;
    juce::CriticalSection writerLock;
    std::atomic<bool> isRecording{ false };
    
    
    // We'll now have an array of AudioTransportSource objects and associated AudioFormatReaderSource objects
    juce::OwnedArray<juce::AudioTransportSource> transportSources;
    juce::OwnedArray<juce::AudioSourcePlayer> audioSourcePlayers;
    juce::OwnedArray<juce::AudioFormatReaderSource> audioSources;
    std::array<float, 4> mixLevels = {1.0f, 1.0f, 1.0f, 1.0f};
    juce::AudioSourcePlayer audioSourcePlayer;
    bool isLooping = false;
    juce::OwnedArray<juce::ResamplingAudioSource> resamplingSources; 
    
};
