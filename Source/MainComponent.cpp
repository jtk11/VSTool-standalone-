#include "MainComponent.h"
#include "AudioHandler.h"

MainComponent::MainComponent()
{
     // Adjust the size to add space for the toolbar
    int toolbarHeight = 50;
    setSize(600, 400 + toolbarHeight);

    // Set up the folderButton
    folderButton.setButtonText("Load");
    folderButton.addListener(this);
    addAndMakeVisible(folderButton);

    // Set up the diceButton
    diceButton.setButtonText("Dice");
    diceButton.addListener(this);
    diceButton.setEnabled(false); // Disable until a folder is selected
    addAndMakeVisible(diceButton);
    
    // Set up the loopButton
    loopButton.setButtonText("Loop");
    loopButton.addListener(this);
    loopButton.setToggleState(true, juce::dontSendNotification);
    addAndMakeVisible(loopButton);
    
    // Set up the rndMixButton
    rndMixButton.setButtonText("Rnd Mix");
    rndMixButton.addListener(this);
    rndMixButton.setEnabled(true); // Enable or disable as per your needs
    addAndMakeVisible(rndMixButton);
    startTimerHz(4);
    
    // Set up the stopButton
    stopButton.setButtonText("Stop");
    stopButton.addListener(this);
    addAndMakeVisible(stopButton);
    
    // Initialize the timerHzSlider
    timerHzSlider.setRange(30.0, 500.0, 0.1);
    timerHzSlider.setValue(200); // Starting value (you had startTimerHz(4) before)
    timerHzSlider.addListener(this);
    addAndMakeVisible(timerHzSlider);
    
    // Set up the detuneButton
    detuneButton.setButtonText("Pitch");
    detuneButton.addListener(this);
    addAndMakeVisible(detuneButton);
    
    recButton.setButtonText("R");
    recButton.addListener(this);
    addAndMakeVisible(recButton);

    stop2Button.setButtonText("S");
    stop2Button.addListener(this);
    addAndMakeVisible(stop2Button);

    playButton.setButtonText("P");
    playButton.addListener(this);
    addAndMakeVisible(playButton);
    
    ballPosition = { getWidth() * 0.5f, getHeight() * 0.5f };
    isDragging = false;
    
    setMouseClickGrabsKeyboardFocus(false);
    setWantsKeyboardFocus(false);
}

MainComponent::~MainComponent()
{
    audioHandler.stop(); // Stop audio playback
    // ... any other cleanup you may have
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::lightgrey);  
    
    // Draw the toolbar
    int toolbarHeight = 50;
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(0, 0, getWidth(), toolbarHeight);

    // Adjust quadrant lines
    g.setColour(juce::Colours::black);
    g.drawLine(getWidth() * 0.5, toolbarHeight, getWidth() * 0.5, getHeight(), 2.0f);

    int horizontalLineY = (getHeight() - toolbarHeight) * 0.5 + toolbarHeight;
    g.drawLine(0, horizontalLineY, getWidth(), horizontalLineY, 2.0f);

    // Adjust the blue ball
    g.setColour(juce::Colours::blue);
        g.fillEllipse(ballPosition.x - 15, ballPosition.y - 15, 30, 30);
    
    // Display the file names
    if (audioFiles.size() >= 4)
    {
        g.setColour(juce::Colours::black);
        g.drawText(audioFiles[0].getFileName(), 10, getHeight() * 0.25, getWidth() * 0.5 - 20, 20, juce::Justification::centred);
        g.drawText(audioFiles[1].getFileName(), getWidth() * 0.5 + 10, getHeight() * 0.25, getWidth() * 0.5 - 20, 20, juce::Justification::centred);
        g.drawText(audioFiles[2].getFileName(), 10, getHeight() * 0.75, getWidth() * 0.5 - 20, 20, juce::Justification::centred);
        g.drawText(audioFiles[3].getFileName(), getWidth() * 0.5 + 10, getHeight() * 0.75, getWidth() * 0.5 - 20, 20, juce::Justification::centred);
    }
}

void MainComponent::shuffleAudioFiles()
{
    juce::Random random; // Initialize a random number generator

    for (int i = audioFiles.size() - 1; i > 0; i--)
    {
        // Generate a random index between 0 and i
        int j = random.nextInt(i + 1);

        // Swap audioFiles[i] with the element at random index j
        audioFiles.swap(i, j);
    }
}
void MainComponent::timerCallback()
{
    if (shouldMoveBall) {
        // Calculate the vector from the current ball position to the target point
        juce::Point<float> direction = points[currentPointIndex] - ballPosition;

        // Check if we are close enough to the target point to snap to it
        const float snapThreshold = 0.5f;
        if (direction.getDistanceFromOrigin() < snapThreshold) {
            ballPosition = points[currentPointIndex]; // Snap to target point
            

            // Increment the current point index and wrap it around if it exceeds the number of points
            currentPointIndex = (currentPointIndex + 1) % 4;
            
        } else {
            // Normalize the direction vector to a unit vector
            direction = direction / direction.getDistanceFromOrigin();

            // Move the ball by a small step towards the target point
            const float stepSize = 1.0f;
            ballPosition += direction * stepSize;
            

        }

        // Update the mix levels based on new ball position
        float topLeft, topRight, bottomLeft, bottomRight;
        computeMixLevels(topLeft, topRight, bottomLeft, bottomRight);
        audioHandler.setMixLevels(topLeft, topRight, bottomLeft, bottomRight);

        // Repaint to show the ball's new position.
        repaint();
    }
    
    if (isPlayingBack)
    {
        if (playbackIndex < recordedPositions.size())
        {
            ballPosition = recordedPositions[playbackIndex];
            playbackIndex++;
            repaint();
            
            // Update mix levels during playback
            float topLeft, topRight, bottomLeft, bottomRight;
            computeMixLevels(topLeft, topRight, bottomLeft, bottomRight);
            audioHandler.setMixLevels(topLeft, topRight, bottomLeft, bottomRight);
            
        }
        else
        {
            playbackIndex = 0;  // loop playback
            // Log when playback loops
        }
    }
}


void MainComponent::computeMixLevels(float& topLeft, float& topRight, float& bottomLeft, float& bottomRight)
{
    float x = ballPosition.x / getWidth();
    float y = ballPosition.y / getHeight();

    topLeft = (1 - x) * (1 - y);
    topRight = x * (1 - y);
    bottomLeft = (1 - x) * y;
    bottomRight = x * y;
}

void MainComponent::resized()
{
    int toolbarHeight = 50; // Height of the toolbar

    // Let's space out the buttons a little bit within the toolbar
    int buttonYPosition = 10; // This will center them vertically in the toolbar (given their height of 30)
    int buttonSpacing = 10;  // Horizontal spacing between buttons

    folderButton.setBounds(buttonSpacing, buttonYPosition, 60, 30);
    
    // Place the diceButton to the right of the folderButton, with the defined spacing
    diceButton.setBounds(folderButton.getRight() + buttonSpacing, buttonYPosition, 60, 30);
    
    rndMixButton.setBounds(diceButton.getRight() + buttonSpacing, buttonYPosition, 60, 30);
    
    stopButton.setBounds(rndMixButton.getRight() + buttonSpacing, buttonYPosition, 40, 20);
    
    timerHzSlider.setBounds(stopButton.getRight() + buttonSpacing, buttonYPosition, 80, 20);
    
    detuneButton.setBounds(timerHzSlider.getRight() + buttonSpacing, buttonYPosition, 40, 20);
    
    recButton.setBounds(detuneButton.getRight() + buttonSpacing, buttonYPosition, 20, 20);
    
    stop2Button.setBounds(recButton.getRight() + buttonSpacing, buttonYPosition, 20, 20);
    
    playButton.setBounds(stop2Button.getRight() + buttonSpacing, buttonYPosition, 20, 20);
    
    loopButton.setBounds(playButton.getRight() + buttonSpacing, buttonYPosition, 50, 20);

}
void MainComponent::mouseDown(const juce::MouseEvent& e)
{
    // Check if the mouse down position is within the ball's region
    float distance = ballPosition.getDistanceFrom(e.position);
    if (distance <= 15.0f) // 15.0f is the ball's radius
    {
        isDragging = true;
    }
}

void MainComponent::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &timerHzSlider)
    {
        startTimerHz(timerHzSlider.getValue());
    }
}

void MainComponent::mouseDrag(const juce::MouseEvent& event)
{
    if (isDragging)
        
    {
        ballPosition = event.position;
    // Boundary checks
    if (ballPosition.x < 15) ballPosition.x = 15;
    if (ballPosition.x > getWidth() - 15) ballPosition.x = getWidth() - 15;
    if (ballPosition.y < 15 + 50) ballPosition.y = 15 + 50;  // 50 is the toolbar height
    if (ballPosition.y > getHeight() - 15) ballPosition.y = getHeight() - 15;

        if (isRecording)
        {
            recordedPositions.push_back(ballPosition);
            juce::Logger::writeToLog("Recording position: " + juce::String(ballPosition.x) + ", " + juce::String(ballPosition.y));
            if (recordedPositions.size() > 600) // Limit to 600 positions for 10 seconds (60 FPS)
            {
                recordedPositions.erase(recordedPositions.begin());
            }
        }
        
        float topLeft, topRight, bottomLeft, bottomRight;
        computeMixLevels(topLeft, topRight, bottomLeft, bottomRight);
        audioHandler.setMixLevels(topLeft, topRight, bottomLeft, bottomRight);
    DBG("Mix Levels: " + juce::String(topLeft) + ", " + juce::String(topRight) + ", " + juce::String(bottomLeft) + ", " + juce::String(bottomRight));
    repaint();

        repaint();  // Redraw with new ball position
    }
}

void MainComponent::mouseUp(const juce::MouseEvent& event)
{
    isDragging = false;
}

void MainComponent::buttonClicked(juce::Button* button)
{
    if (button == &folderButton)
    {
        juce::FileChooser chooser("Select a folder containing .wav or .aif files", juce::File::getSpecialLocation(juce::File::userHomeDirectory), "");

        if (chooser.browseForDirectory())
        {
            juce::File folder = chooser.getResult();

            audioFiles.clear(); // Clear the previous list

            // Fetch .wav files
            folder.findChildFiles(audioFiles, juce::File::findFiles, false, "*.wav");

            // Fetch .aif files
            folder.findChildFiles(audioFiles, juce::File::findFiles, false, "*.aif");

            // Log the results
            juce::String message = "Found " + juce::String(audioFiles.size()) + " audio files.";
            juce::Logger::writeToLog(message);
            
            if (audioFiles.size() > 0)
            {
                juce::Array<juce::File> filesToPlay;
                for (int i = 0; i < juce::jmin(4, audioFiles.size()); ++i)
                {
                    filesToPlay.add(audioFiles[i]);
                }
                audioHandler.loadFiles(filesToPlay);
                audioHandler.start();
            }

            diceButton.setEnabled(audioFiles.size() >= 4);
        }
    }
  else if (button == &diceButton)
{
    if (audioFiles.size() >= 4)
    {
        shuffleAudioFiles();
        juce::Array<juce::File> filesToPlay = { audioFiles[0], audioFiles[1], audioFiles[2], audioFiles[3] };
        audioHandler.loadFiles(filesToPlay);
        audioHandler.setLooping(loopButton.getToggleState());  // <-- ensure loop state is reapplied after loading new files
        audioHandler.start();
        repaint();
        diceButton.setEnabled(audioFiles.size() >= 4);

    }
    else
    {
        // If for some reason there are fewer than 4 audio files, you can add some error handling here.
        juce::Logger::writeToLog("Not enough audio files to shuffle and pick four.");
    }
}
    if (button == &loopButton)
    {
        audioHandler.setLooping(loopButton.getToggleState());
    }
    
    else if (button == &rndMixButton)
    {
        // Ensure no movement is happening while setting up new points
        shouldMoveBall = false;

        // Generate 4 random points for the joystick to move to
        for (int i = 0; i < 4; ++i)
        {
            points[i].x = juce::Random::getSystemRandom().nextFloat() * (getWidth() - 30) + 15; // 15 and 30 are ball radii and diameter
            points[i].y = juce::Random::getSystemRandom().nextFloat() * (getHeight() - 30 - 50) + 50 + 15; // 50 is the toolbar height
        }

        currentPointIndex = 0; // Reset the current point index

        // Set the initial ball position to the first point to avoid a visual jump
        ballPosition = points[0];
        
        int timerInterval = static_cast<int>(1000.0 / timerHzSlider.getValue());

        // Restart the timer here if it's not always running
        startTimer(timerInterval);

        // Now allow the ball to move
        shouldMoveBall = true;
    }

    
    else if (button == &stopButton)
    {
        shouldMoveBall = false; // This will stop the ball from moving
    }
    
    if (button == &recButton)
    {
        isRecording = true;
        recordedPositions.clear();
        juce::Logger::writeToLog("recButton clicked. Starting recording.");
    }
    else if (button == &stop2Button)
    {
        isRecording = false;
        isPlayingBack = false;
    }
    else if (button == &playButton)
    {
        isRecording = false; 
        isPlayingBack = true;
        playbackIndex = 0;
        juce::Logger::writeToLog("playButton clicked. Starting playback.");
    }
    
    else if (button == &detuneButton)
    {
        // Call the detune function from AudioHandler to change pitch of all sounds in the mixer
        audioHandler.detuneSounds();
    }

}
