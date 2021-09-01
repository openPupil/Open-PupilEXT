#ifndef EXCUSE_H
#define EXCUSE_H

/*
  Version 1.0, 08.06.2015, Copyright University of Tübingen.

  The Code is created based on the method from the paper:
  "ExCuSe: Robust Pupil Detection in Real-World Scenarios", W. Fuhl, T. C. Kübler, K. Sippel, W. Rosenstiel, E. Kasneci
  CAIP 2015 : Computer Analysis of Images and Patterns

  The code and the algorithm are for non-comercial use only.
*/

#include "PupilDetectionMethod.h"

class ExCuSe : public PupilDetectionMethod {

public:

    static std::string desc;

    int max_ellipse_radi = 50;
    int good_ellipse_threshold = 15;

    ExCuSe() {
        mDesc = "ExCuSe (Fuhl et al. 2015)";
        mTitle = "ExCuSe";
    }

    Pupil run(const cv::Mat &frame);

	void run(const cv::Mat &frame, const cv::Rect &roi, Pupil &pupil, const float &minPupilDiameterPx=-1, const float &maxPupilDiameterPx=-1);

	bool hasConfidence() {
	    return false;
	}

	bool hasCoarseLocation() {
	    return false;
	}

    bool hasInliers() override {
        return false;
    }

};

#endif // EXCUSE_H
