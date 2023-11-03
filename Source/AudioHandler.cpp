#include "AudioHandler.h"

AudioHandler::AudioHandler()
{
    formatManager.registerBasicFormats();
    deviceManager.initialiseWithDefaultDevices(0, 2);  // 0 input channels, 2 output channels
    deviceManager.addAudioCallback(&audioSourcePlayer);
    audioSourcePlayer.setSource(&mixerSource);
}

AudioHandler::~AudioHandler()
{
    stop();
    
    for (auto* transportSource : transportSources)
    {
        transportSource->setSource(nullptr);
        transportSource->stop();
    }
    
    for (auto* audioSourcePlayer : audioSourcePlayers)
    {
        audioSourcePlayer->setSource(nullptr); 
        deviceManager.removeAudioCallback(audioSourcePlayer); 
    }
    
    transportSources.clear();
    audioSourcePlayers.clear();
}


void AudioHandler::initializeAudio()
{
    formatManager.registerBasicFormats();
    deviceManager.initialiseWithDefaultDevices(0, 2); // 0 in, 2 out

    // Assuming you have a fixed number of transportSources (like 4 for the quadrants),
    // initialize them all.
   for (int i = 0; i < 4; i++)
    {
        auto* newTransportSource = new juce::AudioTransportSource();
        transportSources.add(newTransportSource);

        // Wrap the newTransportSource with a ResamplingAudioSource
        auto* newResamplingSource = new juce::ResamplingAudioSource(newTransportSource, false);
        resamplingSources.add(newResamplingSource);

        auto* newAudioSourcePlayer = new juce::AudioSourcePlayer();
        newAudioSourcePlayer->setSource(newResamplingSource); // Set the ResamplingAudioSource
        audioSourcePlayers.add(newAudioSourcePlayer);

        // Add the newAudioSourcePlayer as the callback for the deviceManager
        deviceManager.addAudioCallback(newAudioSourcePlayer);
        
    }
}


void AudioHandler::loadFiles(const juce::Array<juce::File>& files)
{
    mixerSource.removeAllInputs();
    transportSources.clear();
    audioSources.clear();
    resamplingSources.clear(); // Clear the previous resampling sources

    for (auto& file : files)
    {
        auto* reader = formatManager.createReaderFor(file);
        if (reader)
        {
            auto* audioSource = new juce::AudioFormatReaderSource(reader, true);
            audioSources.add(audioSource);
            auto* transportSource = new juce::AudioTransportSource();
            transportSources.add(transportSource);
            transportSource->setSource(audioSource, 0, nullptr, reader->sampleRate);

            // Wrap the transportSource with a ResamplingAudioSource
            auto* newResamplingSource = new juce::ResamplingAudioSource(transportSource, false);
            resamplingSources.add(newResamplingSource); // Store the resampling source

            mixerSource.addInputSource(newResamplingSource, false); // Add the ResamplingAudioSource to the mixer
        }
    }
    setLooping(true);
}
void AudioHandler::startPlaying()
{
    for (auto* transportSource : transportSources)
    {
        transportSource->start();
    }
}

void AudioHandler::setMixLevels(float topLeft, float topRight, float bottomLeft, float bottomRight)
{
    mixLevels[0] = topLeft;
    mixLevels[1] = topRight;
    mixLevels[2] = bottomLeft;
    mixLevels[3] = bottomRight;

    if (transportSources[0]) transportSources[0]->setGain(mixLevels[0]);
    if (transportSources[1]) transportSources[1]->setGain(mixLevels[1]);
    if (transportSources[2]) transportSources[2]->setGain(mixLevels[2]);
    if (transportSources[3]) transportSources[3]->setGain(mixLevels[3]);
}



void AudioHandler::getNextAudioBlock(const juce::AudioSourceChannelInfo& buffer)
{
    buffer.clearActiveBufferRegion();  // Clear the buffer

    for (int i = 0; i < audioSources.size(); ++i)
    {
        juce::AudioBuffer<float> tempBuffer(buffer.buffer->getNumChannels(), buffer.numSamples);
        juce::AudioSourceChannelInfo tempInfo(&tempBuffer, buffer.startSample, buffer.numSamples);

        audioSources[i]->getNextAudioBlock(tempInfo);
        tempBuffer.applyGain(mixLevels[i]);

        buffer.buffer->addFrom(0, buffer.startSample, tempBuffer, 0, buffer.startSample, buffer.numSamples);
        if (buffer.buffer->getNumChannels() > 1)
            buffer.buffer->addFrom(1, buffer.startSample, tempBuffer, 1, buffer.startSample, buffer.numSamples);
    }
}




void AudioHandler::setLooping(bool shouldLoop)
{
    isLooping = shouldLoop;
    juce::Logger::writeToLog(isLooping ? "Looping ON" : "Looping OFF");
    
    for (auto* source : audioSources)
    {
        if (source)
            source->setLooping(isLooping);
    }
}

void AudioHandler::start()
{
    for (auto* transport : transportSources)
    {
        transport->start();
    }
}

void AudioHandler::stop()
{
    for (auto* transport : transportSources)
    {
        transport->stop();
    }
}

void AudioHandler::stopAllSounds() {
    for (auto* transportSource : transportSources) {
        transportSource->stop();
    }
}

void AudioHandler::stopSample(int index) {
    if (index >= 0 && index < transportSources.size()) {
        transportSources[index]->stop();
    }
}

void AudioHandler::detuneSounds()
{
    // Generate a random semitone
    int semitoneShift = juce::Random::getSystemRandom().nextInt(juce::Range<int>(-36, -1));

    // Calculate the new resampling ratio based on the semitone shift
    double newResamplingRatio = std::pow(2.0, static_cast<double>(semitoneShift) / 12.0);

    // Apply the new resampling ratio to all ResamplingAudioSources
    for (auto* resamplingSource : resamplingSources)
    {
        resamplingSource->setResamplingRatio(newResamplingRatio);
    }

    // Log the detuning
    juce::String message = "Detuned all sounds by " + juce::String(semitoneShift) + " semitones.";
    juce::Logger::writeToLog(message);
}

