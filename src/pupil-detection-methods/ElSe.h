#ifndef PUPILALGOSIMPLE_ELSE_H
#define PUPILALGOSIMPLE_ELSE_H

/*
  Version 1.0, 17.12.2015, Copyright University of Tübingen.

  The Code is created based on the method from the paper:
  "ElSe: Ellipse Selection for Robust Pupil Detection in Real-World Environments", W. Fuhl, T. C. Santini, T. C. Kübler, E. Kasneci
  ETRA 2016 : Eye Tracking Research and Application 2016

  The code and the algorithm are for non-comercial use only.
*/

#include "PupilDetectionMethod.h"

class ElSe : public PupilDetectionMethod {

public:

    static float minArea;
    static float maxArea;

    ElSe() {
        mDesc = "ElSe (Fuhl et al. 2016)";
        mTitle = "ElSe";
    }

    Pupil run(const cv::Mat &frame) override;
    void run(const cv::Mat &frame, const cv::Rect &roi, Pupil &pupil, const float &minPupilDiameterPx, const float &maxPupilDiameterPx) override;

    bool hasConfidence() override {
        return false;
    }

    bool hasCoarseLocation() override {
        return false;
    }

    bool hasInliers() override {
        return false;
    }

    float minAreaRatio = 0.005;
    float maxAreaRatio = 0.2;

};


#endif //PUPILALGOSIMPLE_ELSE_H
