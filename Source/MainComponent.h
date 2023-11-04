#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "AudioHandler.h"


class MainComponent : public juce::Component,
                      public juce::Button::Listener,
                      public juce::Timer,
                      public juce::Slider::Listener
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void buttonClicked(juce::Button* button) override;
    void timerCallback() override;
    void sliderValueChanged(juce::Slider* slider) override;


private:
    AudioHandler audioHandler;
    juce::TextButton folderButton; // Button to select folder
    juce::TextButton diceButton;   // Button to randomly select files
    juce::Array<juce::File> audioFiles; // Store the list of audio files
    void shuffleAudioFiles();
    juce::ToggleButton loopButton; // Button to toggle loop
    juce::Point<float> ballPosition;
    bool isDragging =false;
    void computeMixLevels(float& topLeft, float& topRight, float& bottomLeft, float& bottomRight);
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    juce::TextButton rndMixButton;
    juce::TextButton stopButton;
    bool shouldMoveBall = false;
    int currentPointIndex = 0; // Declare a variable to keep track of the current point
    juce::Point<float> points[4]; // Declare an array to hold the points
    juce::Slider timerHzSlider;
    juce::TextButton detuneButton;
    bool isRecording = false;
    bool isPlayingBack = false;
    std::vector<juce::Point<float>> recordedPositions;
    size_t playbackIndex = 0;
    juce::TextButton recButton;
    juce::TextButton stop2Button;
    juce::TextButton playButton;
    private:
    juce::TextButton stopButton1;
    juce::TextButton stopButton2;
    juce::TextButton stopButton3;
    juce::TextButton stopButton4;
    juce::TextButton allStopButton;
    juce::TextButton playButton1;
    juce::TextButton playButton2;
    juce::TextButton playButton3;
    juce::TextButton playButton4;
    juce::TextButton recAudioButton;
    juce::TextButton stopRecAudioButton;
    juce::TextButton bitcrushButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
