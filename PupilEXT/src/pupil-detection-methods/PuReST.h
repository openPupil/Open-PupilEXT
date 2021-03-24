#ifndef PUPILALGOSIMPLE_PUREST_H
#define PUPILALGOSIMPLE_PUREST_H

/*
 * Copyright (c) 2018, Thiago Santini
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for non-commercial purposes, without fee, and without a written
 * agreement is hereby granted, provided that:
 *
 * 1) the above copyright notice, this permission notice, and the subsequent
 * bibliographic references be included in all copies or substantial portions of
 * the software
 *
 * 2) the appropriate bibliographic references be made on related publications
 *
 * In this context, non-commercial means not intended for use towards commercial
 * advantage (e.g., as complement to or part of a product) or monetary
 * compensation. The copyright holder reserves the right to decide whether a
 * certain use classifies as commercial or not. For commercial use, please contact
 * the copyright holders.
 *
 * REFERENCES:
 *
 * Thiago Santini, Wolfgang Fuhl, Enkelejda Kasneci, PuRe: Robust pupil detection
 * for real-time pervasive eye tracking, Computer Vision and Image Understanding,
 * 2018, ISSN 1077-3142, https://doi.org/10.1016/j.cviu.2018.02.002.
 *
 *
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE TO ANY PARTY FOR DIRECT,
 * INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS,
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
 * THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE AUTHORS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE AUTHORS
 * HAVE NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
 * MODIFICATIONS.
 */

#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>
#include "Pupil.h"
#include "PupilDetectionMethod.h"
#include "PuRe.h"

class GreedyCandidate {

public:

    float maxGap;
    std::vector<cv::Point> points;
    std::vector<cv::Point> hull;
    cv::Point2f meanPoint;

    explicit GreedyCandidate(const std::vector<cv::Point> &points) : points(points) {
        cv::convexHull(points, hull);
        maxGap = 0;
        meanPoint = {0, 0};
        for (auto p1=hull.begin(); p1!=hull.end(); p1++) {
            meanPoint += cv::Point2f(*p1);
            for (auto p2=p1+1; p2!=hull.end(); p2++) {
                float gap = cv::norm(*p2-*p1);
                if (gap > maxGap)
                    maxGap = gap;
            }
        }
        meanPoint.x /= points.size();
        meanPoint.y /= points.size();
    }

};


class PuReST: public PuRe {

public:

    PuReST();
    ~PuReST() override;

    Pupil run(const cv::Mat &frame) override {
        Pupil pupil;
        run(frame, pupil);
        return pupil;
    }

    void reset();

    void run(const cv::Mat &frame, Pupil &pupil) override;
    void run(const cv::Mat &frame, const cv::Rect &roi, Pupil &pupil, const float &userMinPupilDiameterPx=-1, const float &userMaxPupilDiameterPx=-1) override;
    void runTracking(const cv::Mat &frame, Pupil &pupil, const float &userMinPupilDiameterPx, const float &userMaxPupilDiameterPx);

    bool hasPupilOutline() {
        return true;
    }

    bool hasConfidence() override {
        return true;
    }

    bool hasCoarseLocation() override {
        return false;
    }

    bool hasInliers() override {
        return false;
    }

private:

    cv::Mat dilateKernel;
    cv::Mat openKernel;
    Pupil outlineSeedPupil;
    Pupil previousPupil;

    void calculateHistogram(const cv::Mat &in, cv::Mat &histogram, const int &bins, const cv::Mat &mask = cv::Mat());
    void getThresholds(const cv::Mat &input, const cv::Mat &histogram, const Pupil &pupil, int &lowTh, int &highTh, cv::Mat &bright, cv::Mat &dark);

    bool greedySearch(const cv::Mat &greedyDetectorEdges, const Pupil &basePupil, const cv::Mat &dark, const cv::Mat &bright, Pupil &pupil, const float &localMinPupilDiameterPx);
    bool trackOutline(const cv::Mat &outlineTrackerEdges, const Pupil &basePupil, Pupil &pupil, const float &localScalingRatio, const float &minOutlineConfidence = 0.65f);
    static void generateCombinations(const std::vector<GreedyCandidate> &seeds, std::vector<GreedyCandidate> &candidates, int length);
    static float confidence(const cv::Mat &frame, const Pupil &pupil, const std::vector<cv::Point> &points);

};


#endif //PUPILALGOSIMPLE_PUREST_H
