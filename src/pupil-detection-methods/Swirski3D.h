#ifndef PUPILALGOSIMPLE_SWIRSKI3D_H
#define PUPILALGOSIMPLE_SWIRSKI3D_H

/*The MIT License (MIT)

Copyright (c) 2014 Lech Swirski

Permission is hereby granted, free of charge, to any person obtaining a copy
        of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
        to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
        copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
        copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
        AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Modified by Moritz Lode 2020
*/

#include "PupilDetectionMethod.h"
#include "Swirski2D.h"
#include "../../singleeyefitter/singleeyefitter/singleeyefitter.h"
#include "../../singleeyefitter/singleeyefitter/projection.h"
#include <opencv2/opencv.hpp>

// needs first training
// take first N frames for training then classify
// take N, then classify all again? -> open video again or save frames?

namespace sef = singleeyefitter;

class SpaceBinSearcher {

public:

    SpaceBinSearcher();
    SpaceBinSearcher(int w, int h);

    void initialize(int w, int h);
    ~SpaceBinSearcher();

    void render(cv::Mat &img);
    void reset_indices();

    bool search(int x, int y, cv::Vec2i &pt, float &dist);
    bool is_initialized(){ return is_initialized_; };

protected:

    // Local variables initialized at the constructor
    const int kSearchGridSize_;
    cv::Mat ClusterMembers_; //This Set A
    cv::Mat ClusterCenters_;  //This set B
    cv::flann::GenericIndex< cv::flann::L2<int> > *kdtrees; // The flann searching tree

    // Local variables
    bool is_initialized_ = false;
    const int kN_ = 1;
    int sample_num_;
    std::vector<bool> taken_flags_;

};


class Swirski3D: public PupilDetectionMethod {

public:

    PupilDetectionMethod *pupilDetector;
    sef::EyeModelFitter modelFitter;
    SpaceBinSearcher spaceBinSearcher;

    static const size_t kFitterMaxCountDefault = 30;// 100;
    size_t fitter_count = 0;
    size_t fitter_max_count = kFitterMaxCountDefault;// 100;


    Swirski3D(double focal_length, double region_band_width, double region_step_epsilon)
        : focal_length(focal_length), modelFitter(focal_length, region_band_width, region_step_epsilon), fitter_max_count(kFitterMaxCountDefault) {

        mDesc = "Swirski3D (Swirski et al.)";
        mTitle = "Swirski3D";

        pupilDetector = new Swirski2D();
    }

    Swirski3D(PupilDetectionMethod *pupilMethod, double focal_length, double region_band_width, double region_step_epsilon)
            : pupilDetector(pupilMethod), focal_length(focal_length), modelFitter(focal_length, region_band_width, region_step_epsilon), fitter_max_count(kFitterMaxCountDefault) {

        mDesc = "Swirski3D (Swirski et al.)";
        mTitle = "Swirski3D";
    }

    ~Swirski3D() override {
        delete pupilDetector;
    };

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

    bool is_model_built() {
        return is_model_built_;
    }


private:

    double focal_length;
    bool is_model_built_ = false;

    double compute_reliability(const cv::Mat &img, sef::Ellipse2D<double> &el, std::vector<cv::Point2f> &inlier_pts);

    sef::EyeModelFitter::Circle unproject(const cv::Mat &img, sef::Ellipse2D<double> &el, std::vector<cv::Point2f> &inlier_pts);

    bool add_observation(const cv::Mat &image, sef::Ellipse2D<double> &pupil, std::vector<cv::Point2f> &pupil_inliers, bool force);

    cv::RotatedRect toImgCoordInv(const cv::RotatedRect& rect, const cv::Mat& m, float scale=1);

    cv::Point2f toImgCoord(const cv::Point2f &point, const cv::Mat &m, double scale=1, int shift=0);

    cv::Point toImgCoord(const cv::Point &point, const cv::Mat &m, double scale=1, int shift=0);

    cv::RotatedRect toImgCoord(const cv::RotatedRect &rect, const cv::Mat &m, float scale=1);

    cv::Point2f toImgCoordInv(const cv::Point2f &point, const cv::Mat &m, double scale=1, int shift=0);

    cv::Point toImgCoordInv(const cv::Point &point, const cv::Mat &m, double scale=1, int shift=0);

};


#endif //PUPILALGOSIMPLE_SWIRSKI3D_H
