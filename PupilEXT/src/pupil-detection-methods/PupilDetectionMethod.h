
#ifndef PUPILALGOSIMPLE_PUPILDETECTIONMETHOD_H
#define PUPILALGOSIMPLE_PUPILDETECTIONMETHOD_H

/*
Copyright (c) 2018, Thiago Santini / University of Tübingen

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software, source code, and associated documentation files (the "Software")
to use, copy, and modify the Software for academic use, subject to the following
conditions:

1) The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

2) Modifications to the source code should be made available under
free-for-academic-usage licenses.

For commercial use, please contact the authors.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
        WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include <opencv2/core/types.hpp>
#include "Pupil.h"
#include <iostream>

class PupilDetectionMethod {

public:
    PupilDetectionMethod() = default;

    virtual ~PupilDetectionMethod() = default;

    virtual Pupil run(const cv::Mat &frame) = 0;
    virtual bool hasConfidence() = 0;
    virtual bool hasCoarseLocation() = 0;
    virtual bool hasInliers() = 0;

    std::string title() {
        return mTitle;
    }

    std::string description() {
        return mDesc;
    }

    virtual void run(const cv::Mat &frame, Pupil &pupil) {
        pupil.clear();
        pupil = run(frame);
    }

    virtual void run(const cv::Mat &frame, Pupil &pupil, std::vector<cv::Point2f> &inlierPts) {
        pupil.clear();
        pupil = run(frame);
        inlierPts = std::vector<cv::Point2f>();
    }

    virtual void run(const cv::Mat &frame, const cv::Rect &roi, Pupil &pupil, const float &minPupilDiameterPx, const float &maxPupilDiameterPx) {
        (void) roi;
        (void) minPupilDiameterPx;
        (void) maxPupilDiameterPx;
        run(frame, pupil);
    }

    Pupil runWithConfidence(const cv::Mat &frame) {
        Pupil pupil;
        run(frame, pupil);
        pupil.outline_confidence = outlineContrastConfidence(frame, pupil);

        return pupil;
    }

    void runWithConfidence(const cv::Mat &frame, Pupil &pupil) {
        run(frame, pupil);
        pupil.outline_confidence = outlineContrastConfidence(frame, pupil);
    }

    // Pupil detection interface used in the tracking
    void runWithConfidence(const cv::Mat &frame, const cv::Rect &roi, Pupil &pupil, const float &minPupilDiameterPx=-1, const float &maxPupilDiameterPx=-1) {
        run(frame, roi, pupil, minPupilDiameterPx, maxPupilDiameterPx);
        pupil.outline_confidence = outlineContrastConfidence(frame, pupil);
    }

    virtual Pupil getNextCandidate() {
        return Pupil();
    }

    // Generic coarse pupil detection
    static cv::Rect coarsePupilDetection(const cv::Mat &frame, const float &minCoverage=0.5f, const int &workingWidth=60, const int &workingHeight=40);

    // Generic confidence metrics
    static float outlineContrastConfidence(const cv::Mat &frame, const Pupil &pupil, const int &bias=5);
    static float edgeRatioConfidence(const cv::Mat &edgeImage, const Pupil &pupil, std::vector<cv::Point> &edgePoints, const int &band=5);
    static float angularSpreadConfidence(const std::vector<cv::Point> &points, const cv::Point2f &center);
    static float aspectRatioConfidence(const Pupil &pupil);

    static std::vector<cv::Point> ellipse2Points(const cv::RotatedRect &ellipse, const int &delta);

protected:

    std::string mTitle;
    std::string mDesc;
};


#endif //PUPILALGOSIMPLE_PUPILDETECTIONMETHOD_H
