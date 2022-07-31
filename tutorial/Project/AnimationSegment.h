#pragma once
#ifndef PROJECT_ANIMATION_SEGMENT_H
#define PROJECT_ANIMATION_SEGMENT_H

class AnimationSegment{
public:
	AnimationSegment(int cameraIndex, double duration)
		:
		cameraIndex{cameraIndex},
		duration{duration}
	{}

	double GetDuration() { return duration; }
	int GetCameraIndex() { return cameraIndex; }

	void SetDuration(double newDuration) { duration = newDuration; }
	void SetCameraIndex(int newCameraIndex) { cameraIndex = newCameraIndex; }

	int cameraIndex;
	double duration;
};

#endif
