#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    // set framerate 
    ofSetFrameRate(60);

	// setup the serial port
	_serial.setup("COM3", 115200);
	_serial.startContinuousRead();
	ofAddListener(_serial.NEW_MESSAGE, this, &ofApp::parseSerial);
	_requestRead = false;

	// initialize gyro values
	_pitch	= 0;
	_roll	= 0;
 
    //Initialize a fake previous point
    DrawPoint previousPoint(0.0f, 0.0f, 0.0f);
}

//--------------------------------------------------------------
void ofApp::update() {
	if (_requestRead) {
		_serial.sendRequest();
		_requestRead = false;
	}

    // Calculate the cursor position
    float cursorX = ofMap(_pitch, 100, 899, 0, ofGetViewportWidth());
    float cursorY = ofMap(_roll, 100, 899, 0, ofGetViewportHeight());

    // update the cursor
    _cursor = DrawPoint(cursorX, cursorY, _brushRadius);
    
    // If the game is not paused, add the current cursor position to an array
    if (!_isPaused) {
        // calculate the current interpolated point
        float interpolatedX = ofLerp(_previousPoint.x, _cursor.x, 0.1);
        float interpolatedY = ofLerp(_previousPoint.y, _cursor.y, 0.1);
        DrawPoint interpolatedPoint = DrawPoint(interpolatedX, interpolatedY, _brushRadius);

        // Only add this current point if it differentiates enough from the previous point
        if (abs(interpolatedPoint.x - _previousPoint.x) >= _previousPoint.radius
                || abs(interpolatedPoint.y - _previousPoint.y) >= _previousPoint.radius) {
            // Add current point to points array
            _pointsArray.push_back(interpolatedPoint);
            
            // Make this point the previous point
            _previousPoint = interpolatedPoint;
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw() {

    // draw cursor
    ofNoFill();
    ofSetLineWidth(4);
    ofDrawCircle(_cursor.x, _cursor.y, 20);
    
    ofFill();

    // If the game is not set on pause, draw the points stored in the array
    if (!_isPaused) {
        for (int i = 0 ; i < _pointsArray.size() ; i++) {
            // don't lerp on the first point
            if (i > 0) {
                // Draw the next point in the array
                ofDrawCircle(_pointsArray[i].x, _pointsArray[i].y, _pointsArray[i].radius);
            }
        }
    }

    // Debug
	ofDrawBitmapString("Pitch: " + ofToString(_pitch), 5, 20);
	ofDrawBitmapString("Roll: " + ofToString(_roll), 5, 40);
	ofDrawBitmapString("Radius: " + ofToString(_brushRadius), 5, 60);
    ofDrawBitmapString("IsPaused: " + ofToString(_isPaused), 5, 80);
    ofDrawBitmapString("IsClearing: " + ofToString(_button2Value), 5, 100);
    ofDrawBitmapString("IsSliderActive: " + ofToString(_isSliderActive), 5, 120);
}

// Called right before program shuts down
void ofApp::exit() {
}

void ofApp::parseSerial(string & message) {
	try {
        // Define what each value we get from serial is
        // Each value is separated by a comma

        if (message != "Started.") {
		    int pitch	            = ofToInt(ofSplitString(message, ",")[0]);
		    int roll	            = ofToInt(ofSplitString(message, ",")[1]);
            bool currentButton1Val  = ofToBool(ofSplitString(message, ",")[2]);
            bool currentButton2Val  = ofToBool(ofSplitString(message, ",")[3]);
            bool currentButton3Val  = ofToBool(ofSplitString(message, ",")[4]);
            int currentSliderVal    = ofToInt(ofSplitString(message, ",")[5]);

            // set pitch and roll
		    _pitch	            = pitch;
		    _roll	            = roll;

            // check if button 3 (enable slider button) was just pressed
            if (currentButton3Val && !_button3Value) {
                // toggle activate slider
                _isSliderActive     = !_isSliderActive;
            }
            // update button 3 value
            _button3Value = currentButton3Val;

            // update slider value
            _sliderValue = currentSliderVal;

            // update brush radius if the slider is currently enabled
            if (_isSliderActive) {
                _brushRadius = _sliderValue;
            }

            // check if button 1 (play/pause button) was just pressed
            if (currentButton1Val && !_button1Value) {
                // toggle play
                _isPaused = !_isPaused;
            }
            // update button 1 value
            _button1Value = currentButton1Val;
            
            // check if button 2 (clear canvas) was just pressed
            if (currentButton2Val && !_button2Value) {
                // clear the canvas
                clearCanvas();
            }
        }
	}
	catch (exception e) {
		logError(e.what());
	}
}

void ofApp::logError(string message) {
	cout << message << "\n";
}

//Linear Interpolation formula for "easing" between points (or smoothing data)
float ofApp::lerp(float previous, float current, float percent)
{
	return (previous + percent * (current - previous));
}

// void clearCanvas()
// Called when the user either presses the clear button
// or uses the GUI
// Clears the pointsArray
void ofApp::clearCanvas() {
    _pointsArray.clear();
}


