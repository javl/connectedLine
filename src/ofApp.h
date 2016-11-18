#pragma once

#include "ofMain.h"
#include "ofxNearestNeighbour.h"
#include "ofxGui.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		void addPoint(unsigned p1, unsigned p2);
		void addRandomPoint();

		bool paused;
		unsigned num;
		// Using ofVec3f so we can easily use ofPolyline
		vector <ofVec3f> points;

		ofxNearestNeighbour3D nn;
		vector<pair<NNIndex, float> > indices;

		ofVec3f mouse;
		bool mouseDown;
		vector <int> ages;

		void begin();

		// GUI
		bool bHideGui;
		ofxPanel gui;
		ofxLabel numLabel;
		ofxLabel fpsLabel;
		ofxLabel emptyLabel1;
		ofxLabel emptyLabel2;
		ofxButton resetBtn;
		ofxButton startBtn;
		ofxButton addBtn;
		ofxToggle debug;
		ofxToggle fill;
		ofxToggle stackFrames;
		ofxIntSlider startAmount;
		ofxIntSlider startRadius;
		ofxIntSlider near;
		ofxIntSlider far;
		ofxIntSlider splitDist;
		ofxFloatSlider attractForce;
		ofxFloatSlider repelForce;


};
