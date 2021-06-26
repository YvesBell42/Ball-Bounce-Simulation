#include <iostream>
#include <stdlib.h>
#include <cmath>
#include <SFML/Graphics.hpp>
#define SFML_STATIC
#pragma comment(lib,"sfml-graphics-s.lib")
#pragma comment(lib,"sfml-system-s.lib")
#pragma comment(lib,"sfml-window-s.lib")
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"freetype.lib")
//#pragma comment(lib,"openal32.lib")
//#pragma comment(lib,"sfml-audio-s.lib")
//#pragma comment(lib,"sfml-network-s.lib")
//#pragma comment(lib,"sfml-main.lib")

using namespace sf;
using namespace std;

// Constants
string WindowTitle = "Tennis Ball Bounce Simulation";
int WindowWidth = 300;
int WindowHeight = 700;
int MaxFrameRate = 60;
int FrameInterval = 1000 / MaxFrameRate;
float TimeMultiplier = 0.25;


// Temperature - 20 C
// 60.3 pixels - 60 / 64 = 0.9375
// 2.54m to 1.41m (1.35m to 1.47m)
double StartHeight = 1; // 2.54
double G = 6.674e-11;
double EarthMass = 5.97237e24;
double EarthRadius = 6378100;
float Pi = 3.141593;
float Radius = 0.0335; // 3.27cm to 3.43cm
float Area = Pi * Radius * Radius;
float Mass = 0.0577; // 56.0g to 59.4g
float AirViscosity = 1.204;
float RestitutionCoefficient = 0.77265; // 0.74514
float DragCoefficient = 0.51; // 0.45 to 0.57

float getHeight(float Height, float TimeSlice, float Velocity, float Acceleration) {
	return Height - ((Velocity * TimeSlice) + 0.5 * Acceleration * TimeSlice * TimeSlice);
}

float getVelocity(float TimeSlice, float Velocity, float Acceleration) {
	return Velocity + Acceleration * TimeSlice;
}

float getAcceleration(float Height, float Velocity) {

	return ((Mass * (G * EarthMass / ((EarthRadius + Height) * (EarthRadius + Height)))) - (0.5 * abs(Velocity) * Velocity * AirViscosity * Area * DragCoefficient)) / Mass;
}

float getCollisionTime(float Height, float TimeSlice, float Velocity, float Acceleration) {
	if (getHeight(Height, TimeSlice, Velocity, Acceleration) < 0) {
		return (-Velocity + sqrt((Velocity * Velocity) + (2 * Acceleration * Height))) / Acceleration;
	}
	else return 0;
}

float getPeakHeight(float Height, float TimeSlice, float Velocity, float Acceleration) {
	return (Velocity * Velocity) / (2 * Acceleration);
}

float getScaleRatio(float Height, float PeakHeight) {
	if (PeakHeight > 25) {
		return 25;
	}
	else if (PeakHeight < 1) {
		return 1;
	}
		else {
			return PeakHeight;
		}
}

int main(int argc, char** argv) {
	Texture textureTennisBall;
	Sprite spriteTennisBall;
	Font fontLucidaFax;
	Text textHeight;
	Text textHeightValue;
	Text textPeakHeight;
	Text textPeakHeightValue;
	Text textTime;
	Text textTimeValue;
	Text textVelocity;
	Text textVelocityValue;
	Text textAcceleration;
	Text textAccelerationValue;
	Text textBounces;
	Text textBouncesValue;
	float Height = StartHeight;
	float PeakHeight = Height;	
	float ScaleRatio = 0.9375 / (1 + StartHeight);
	float HeightRatio = (WindowHeight - (28 * ScaleRatio)) / (StartHeight);
	float SimTime;
	int LastEventTime;
	float TimeSlice;
	float CollisionTime;
	float Velocity = 0;
	float Acceleration = getAcceleration(Height, Velocity);
	int Bounces = 0;
	textureTennisBall.loadFromFile("Tennis Ball.png");
	textureTennisBall.setSmooth(true);
	spriteTennisBall.setTexture(textureTennisBall);
	fontLucidaFax.loadFromFile("LFAX.TTF");
	textTime.setFont(fontLucidaFax);
	textTime.setCharacterSize(12);
	textTime.setPosition(0, 0);
	textTime.setString("           Time: ");
	textTimeValue.setFont(fontLucidaFax);
	textTimeValue.setCharacterSize(12);
	textTimeValue.setPosition(90, 0);
	textPeakHeight.setFont(fontLucidaFax);
	textPeakHeight.setCharacterSize(12);
	textPeakHeight.setPosition(0, 14);
	textPeakHeight.setString(" Peak Height: ");
	textPeakHeightValue.setFont(fontLucidaFax);
	textPeakHeightValue.setCharacterSize(12);
	textPeakHeightValue.setPosition(90, 14);
	textHeight.setFont(fontLucidaFax);
	textHeight.setCharacterSize(12);
	textHeight.setPosition(0, 28);
	textHeight.setString("         Height: ");
	textHeightValue.setFont(fontLucidaFax);
	textHeightValue.setCharacterSize(12);
	textHeightValue.setPosition(90, 28);
	textVelocity.setFont(fontLucidaFax);
	textVelocity.setCharacterSize(12);
	textVelocity.setPosition(0, 42);
	textVelocity.setString("       Velocity: ");
	textVelocityValue.setFont(fontLucidaFax);
	textVelocityValue.setCharacterSize(12);
	textVelocityValue.setPosition(90, 42);
	textAcceleration.setFont(fontLucidaFax);
	textAcceleration.setCharacterSize(12);
	textAcceleration.setPosition(0, 56);
	textAcceleration.setString("Acceleration: ");
	textAccelerationValue.setFont(fontLucidaFax);
	textAccelerationValue.setCharacterSize(12);
	textAccelerationValue.setPosition(90, 56);
	textBounces.setFont(fontLucidaFax);
	textBounces.setCharacterSize(12);
	textBounces.setPosition(0, 70);
	textBounces.setString("       Bounces: ");
	textBouncesValue.setFont(fontLucidaFax);
	textBouncesValue.setCharacterSize(12);
	textBouncesValue.setPosition(90, 70);

	Clock SimClock;
	LastEventTime = SimClock.getElapsedTime().asMilliseconds();
	SimTime = 0;
	RenderWindow renderWindow(VideoMode(WindowWidth, WindowHeight), WindowTitle);
	while (renderWindow.isOpen()) {
		Event event;
		while (renderWindow.pollEvent(event)) {
			if (event.type == Event::EventType::Closed)
			{
				renderWindow.close();
			}
		}

		if (SimClock.getElapsedTime().asMilliseconds() - LastEventTime >= FrameInterval) {
			LastEventTime = SimClock.getElapsedTime().asMilliseconds();
			TimeSlice = (float)FrameInterval / 1000 * TimeMultiplier;

			textHeightValue.setString(to_string(Height) + " m");
			textPeakHeightValue.setString(to_string(PeakHeight) + " m");
			textTimeValue.setString(to_string(SimTime) + " s");
			textVelocityValue.setString(to_string(Velocity) + " m/s");
			textAccelerationValue.setString(to_string(Acceleration) + " m/s");
			textBouncesValue.setString(to_string(Bounces));
			
			CollisionTime = getCollisionTime(Height, TimeSlice, Velocity, Acceleration);
			if (CollisionTime != 0) { // Collision
				TimeSlice = CollisionTime;				
				Height = getHeight(Height, TimeSlice, Velocity, Acceleration);
				Velocity = getVelocity(TimeSlice, Velocity, Acceleration) * -RestitutionCoefficient;
				Acceleration = getAcceleration(Height, Velocity);
				Bounces = Bounces + 1;

				ScaleRatio = 0.9375 / (1 + PeakHeight);
			}
			else {
				Height = getHeight(Height, TimeSlice, Velocity, Acceleration);
				if (Velocity < 0 && getVelocity(TimeSlice, Velocity, Acceleration) > 0) {
					PeakHeight = Height - getPeakHeight(Height, TimeSlice, Velocity, Acceleration);
				}
				Velocity = getVelocity(TimeSlice, Velocity, Acceleration);
				Acceleration = getAcceleration(Height, Velocity);
			}

			spriteTennisBall.setScale(ScaleRatio, ScaleRatio);
			spriteTennisBall.setOrigin(0, 32);
		
			spriteTennisBall.setPosition((WindowWidth / 2) - 30 * ScaleRatio, (StartHeight - Height) * HeightRatio);
			renderWindow.clear(Color::Black);
			renderWindow.draw(spriteTennisBall);
			renderWindow.draw(textHeight);
			renderWindow.draw(textHeightValue);
			renderWindow.draw(textPeakHeight);
			renderWindow.draw(textPeakHeightValue);
			renderWindow.draw(textTime);
			renderWindow.draw(textTimeValue);
			renderWindow.draw(textVelocity);
			renderWindow.draw(textVelocityValue);
			renderWindow.draw(textAcceleration);
			renderWindow.draw(textAccelerationValue);
			renderWindow.draw(textBounces);
			renderWindow.draw(textBouncesValue);
			renderWindow.display();
			
			SimTime = SimTime + TimeSlice;
		} 
	}
}