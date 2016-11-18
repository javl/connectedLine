#include "ofApp.h"

#define MAXNUM 10000 // total amount of points allowed

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetVerticalSync(false);
	ofSetFrameRate(500);
	gui.setup(); // most of the time you don't need a name

	resetBtn.addListener(this, &ofApp::setup);
	gui.add(resetBtn.setup("reset settings"));

	startBtn.addListener(this, &ofApp::begin);
	gui.add(startBtn.setup("restart (keep settings)"));

	addBtn.addListener(this, &ofApp::addRandomPoint);
	gui.add(addBtn.setup("add random point"));

	gui.add(debug.setup("enable debug view", false));
	gui.add(stackFrames.setup("stack frames", false));
	gui.add(fill.setup("fill", false));
	gui.add(emptyLabel1.setup("", ""));

	gui.add(startAmount.setup("start amount", 10, 2, 180));
	gui.add(startRadius.setup("start radius", 20, 0, 100));
	gui.add(near.setup("distance to neighbours", 14, 0, 100));
	gui.add(far.setup("distance to others", 30, 0, 100));
	gui.add(splitDist.setup("max edge size", 24, 0, 100));

	gui.add(attractForce.setup("attract force", 0.1, 0.0, 2.0));
	gui.add(repelForce.setup("repel force", 0.05, 0.0, 2.0));

	gui.add(emptyLabel2.setup("", ""));

	gui.add(numLabel.setup("Points", "0/" + ofToString(MAXNUM)));
	gui.add(fpsLabel.setup("FPS", ofToString(ofGetFrameRate())));

	begin();
}

void  ofApp::begin() {
	// ofBackground(255);
	ofBackgroundGradient(ofColor::white, ofColor::gray);
	ofSetBackgroundAuto(false);
	num = startAmount;
	points.clear();
	points.reserve(MAXNUM);

	// used for debug view
	ages.clear();
	ages.reserve(MAXNUM);

	for (unsigned i = 0; i < num; i++) {
		points.push_back(ofVec3f(ofGetWidth() / 2.0 + startRadius * cos((TWO_PI / num)*i), ofGetHeight() / 2.0 + startRadius * sin((TWO_PI / num)*i)));
		ages.push_back(0);
	}
	nn.buildIndex(points); // build our index for quick radius lookup
	numLabel.setup("Points", ofToString(num) + "/" + ofToString(MAXNUM));
}
//--------------------------------------------------------------
void ofApp::update() {
	// We want to update every position at the same time, so first we store their
	// new positions in a temporary list. Afterwards we apply them all at once
	vector <ofVec3f> newPos;
	// We don't want to introduce new point halfway through our loop, so we make a
	// list of where to insert the new points and add them afterwards
	vector <ofVec2f> newPoints;
	newPos.reserve(MAXNUM);

	for (unsigned i = 0; i < num; i++) {
		newPos[i].set(0.0, 0.0);
		if (!paused) {
			unsigned n1 = (num + i - 1) % num; // left neighbour
			unsigned n2 = (num + i + 1) % num; // right neighbour

			// check distance to neigbour one
			float dist = points[i].distance(points[n1]);
			if (dist > near) {
				if (dist > splitDist && ofRandom(0.0, 1.0) < 0.2) {
					newPoints.push_back(ofVec2f(n1, i));
				} else {
					newPos[i].x -= ((points[i].x - points[n1].x) / points[i].distance(points[n1]));
					newPos[i].y -= ((points[i].y - points[n1].y) / points[i].distance(points[n1]));
				}
			} else {
				newPos[i].x += ((points[i].x - points[n1].x) / points[i].distance(points[n1]));
				newPos[i].y += ((points[i].y - points[n1].y) / points[i].distance(points[n1]));
			}

			// same for neighbour two
			if (points[i].distance(points[n2]) > near) {
				if (dist > splitDist && ofRandom(0.0, 1.0) < 0.2) {
					newPoints.push_back(ofVec2f(i, n2));
				} else {
					newPos[i].x -= ((points[i].x - points[n2].x) / points[i].distance(points[n2]));
					newPos[i].y -= ((points[i].y - points[n2].y) / points[i].distance(points[n2]));
				}
			} else {
				newPos[i].x += ((points[i].x - points[n2].x) / points[i].distance(points[n2]));
				newPos[i].y += ((points[i].y - points[n2].y) / points[i].distance(points[n2]));
			}
			newPos[i] *= attractForce;

			// move away from any other point within <far>pixels
			nn.findPointsWithinRadius(points[i], far, indices);
			for (unsigned k = 0; k < indices.size(); k++) {
				if (indices[k].first != i && indices[k].first != n1 && indices[k].first != n2) {
					if (points[i].distance(points[indices[k].first]) < far) {
						newPos[i].x += (((points[i].x - points[indices[k].first].x) / points[i].distance(points[indices[k].first]))) * repelForce;
						newPos[i].y += (((points[i].y - points[indices[k].first].y) / points[i].distance(points[indices[k].first]))) * repelForce;
					}
				}
			}
			// also move away from the mouse if it is pressed
			if(mouseDown){
				if (points[i].distance(mouse) < far / 2.0) {
					newPos[i].x += (((points[i].x - mouse.x) / points[i].distance(mouse))) * repelForce * 10;
					newPos[i].y += (((points[i].y - mouse.y) / points[i].distance(mouse))) * repelForce * 20;
				}
			}
		}
	}

	// decrease the age of our points, used to draw larger circles for new points in debug view
	for (unsigned i = 0; i < num; i++) {
		points[i] += newPos[i];
		ages[i]--;
		if (ages[i] < 0) ages[i] = 0;
	}
	// Add our new points
	for (unsigned i = 0; i < newPoints.size(); i++) {
		addPoint((int)newPoints[i][0], (int)newPoints[i][1]);
	}

	// Sometimes add a random point as well
	if (ofRandom(0.0, 1.0) < 0.05) {
		addRandomPoint();
	}

	// rebuild our index to reflect the changes
	nn.buildIndex(points);
}

//--------------------------------------------------------------
void ofApp::draw() {
	// check if we want to overwrite each frame or stack them
	ofSetBackgroundAuto(!stackFrames);
	ofSetLineWidth(2);
	if (debug) {
		for (unsigned i = 0; i < num; i++) {
			if (!paused) {
				unsigned n1 = (num + i - 1) % num;
				unsigned n2 = (num + i + 1) % num;

				// draw a line to points we try to move away from
				nn.findPointsWithinRadius(points[i], far, indices);
				for (unsigned k = 0; k < indices.size(); k++) {
					if (indices[k].first != i && indices[k].first != n1 && indices[k].first != n2) {
						if (points[i].distance(points[indices[k].first]) < far) {
							ofSetColor(255, 0, 0, 50);
							ofDrawLine(points[i], points[indices[k].first]);
						}
					}
				}
			}
			// do the same for the mouse
			if(mouseDown){
				if (points[i].distance(mouse) < far) {
					ofSetColor(255, 0, 0, 50);
					ofDrawLine(points[i], mouse);
				}
			}
			// draw our points, make new point larger (checked using <age>)
			ofSetColor(0, 200);
			if (ages[i] > 0) {
				ofDrawCircle(points[i], 8);
			} else {
				ofDrawCircle(points[i], 5);
			}
		}

	}

	// If we are stacking frames, we want to use transparent colours
	if (stackFrames) {
		ofSetColor(0, 1);
	} else {
		ofSetColor(0);
	}

	// Fill or draw outline
	if (fill) {
		ofSetPolyMode(OF_POLY_WINDING_ODD);
		ofFill();
		ofBeginShape();
		for (unsigned i = 0; i < points.size(); i++) {
			ofVertex(points[i].x, points[i].y);
		}
		ofEndShape();
	} else {
		ofPolyline line(points);
		line.close();
		line.draw();
	}

	// check if we want to draw the gui
	if (!bHideGui) {
		gui.draw();
	}

}

//--------------------------------------------------------------
void ofApp::addRandomPoint() {
	unsigned p1 = (int)ofRandom(0, num);
	unsigned p2 = (num + p1 + 1) % num;
	addPoint(p1, p2);
}

//--------------------------------------------------------------
void ofApp::addPoint(unsigned p1, unsigned p2) {
	if (num >= MAXNUM) return;
	if (points[p1].distance(points[p2]) < near) {
		return; // don't add a point if these are already too close
	}
	points.insert(points.begin() + p2, points[p2].getMiddle(points[p1]));
	ages.insert(ages.begin() + p2, 100);
	num++;
	numLabel.setup("Points", ofToString(num) + "/" + ofToString(MAXNUM));
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
	switch (key) {
	case 'a':
		addRandomPoint();
		break;
	case 'h':
		bHideGui = !bHideGui;
		break;
	case 'p':
		paused = !paused;
		break;
	case 'r':
		begin();
		break;
	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
	mouse.set(x, y, 0);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
	mouse.set(x, y, 0);
	mouseDown = true;

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	mouseDown = false;
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}
