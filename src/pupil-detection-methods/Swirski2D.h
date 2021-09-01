#ifndef PUPILALGOSIMPLE_SWIRSKI2D_H
#define PUPILALGOSIMPLE_SWIRSKI2D_H

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

struct TrackerParams
{
    int Radius_Min;
    int Radius_Max;

    double CannyBlur;
    double CannyThreshold1;
    double CannyThreshold2;
    int StarburstPoints;

    int PercentageInliers;
    int InlierIterations;
    bool ImageAwareSupport;
    int EarlyTerminationPercentage;
    bool EarlyRejection;
    int Seed;
};

class Swirski2D : public PupilDetectionMethod {

public:

    TrackerParams params;

    Swirski2D() {
        mDesc = "Swirski2D (Swirski et al.)";
        mTitle = "Swirski2D";

        params.Radius_Min = 40; // TODO this is very sensitive to resolution, one need to know the approx size of pupil in image
        params.Radius_Max = 80;

        params.CannyBlur = 1.6;
        params.CannyThreshold1 = 20; //20
        params.CannyThreshold2 = 40; //40
        params.StarburstPoints = 0;

        params.PercentageInliers = 20; // lower makes it slower
        params.InlierIterations = 2;
        params.ImageAwareSupport = true;
        params.EarlyTerminationPercentage = 95;
        params.EarlyRejection = true;

        params.Seed = -1;
    }

    Pupil run(const cv::Mat &frame) override {
        Pupil pupil;
        run(frame, pupil);
        return pupil;
    }

    void run(const cv::Mat &frame, Pupil &pupil) override;
    void run(const cv::Mat &frame, Pupil &pupil, std::vector<cv::Point2f> &inlierPts) override;
    void run(const cv::Mat &frame, const cv::Rect &roi, Pupil &pupil, const float &minPupilDiameterPx, const float &maxPupilDiameterPx) override;

    bool hasConfidence() override {
        return false;
    }

    bool hasCoarseLocation() override {
        return false;
    }

    bool hasInliers() override {
        return true;
    }

    cv::Rect findMaxHaarResponse(const cv::Mat &frame);

};

class HaarSurroundFeature {

public:

    HaarSurroundFeature(int r1, int r2) : r_inner(r1), r_outer(r2) {
        //  _________________
        // |        -ve      |
        // |     _______     |
        // |    |   +ve |    |
        // |    |   .   |    |
        // |    |_______|    |
        // |         <r1>    |
        // |_________<--r2-->|

        // Number of pixels in each part of the kernel
        int count_inner = r_inner*r_inner;
        int count_outer = r_outer*r_outer - r_inner*r_inner;

        // Frobenius normalized values
        //
        // Want norm = 1 where norm = sqrt(sum(pixelvals^2)), so:
        //  sqrt(count_inner*val_inner^2 + count_outer*val_outer^2) = 1
        //
        // Also want sum(pixelvals) = 0, so:
        //  count_inner*val_inner + count_outer*val_outer = 0
        //
        // Solving both of these gives:
        //val_inner = std::sqrt( (double)count_outer/(count_inner*count_outer + sq(count_inner)) );
        //val_outer = -std::sqrt( (double)count_inner/(count_inner*count_outer + sq(count_outer)) );

        // Square radius normalised values
        //
        // Want the response to be scale-invariant, so scale it by the number of pixels inside it:
        //  val_inner = 1/count = 1/r_outer^2
        //
        // Also want sum(pixelvals) = 0, so:
        //  count_inner*val_inner + count_outer*val_outer = 0
        //
        // Hence:
        val_inner = 1.0 / (r_inner*r_inner);
        val_outer = -val_inner*count_inner/count_outer;

    }

    double val_inner, val_outer;
    int r_inner, r_outer;
};

template<typename T>
class ConicSection_ {

public:

    T A,B,C,D,E,F;

    ConicSection_(cv::RotatedRect r)
    {
        cv::Point_<T> axis((T)std::cos(CV_PI/180.0 * r.angle), (T)std::sin(CV_PI/180.0 * r.angle));
        cv::Point_<T> centre(r.center);
        T a = r.size.width/2;
        T b = r.size.height/2;

        initFromEllipse(axis, centre, a, b);
    }

    T algebraicDistance(cv::Point_<T> p)
    {
        return A*p.x*p.x + B*p.x*p.y + C*p.y*p.y + D*p.x + E*p.y + F;
    }

    T distance(cv::Point_<T> p)
    {
        //    dist
        // -----------
        // |grad|^0.45

        T dist = algebraicDistance(p);
        cv::Point_<T> grad = algebraicGradient(p);

        T sqgrad = grad.dot(grad);

        return dist / std::pow(sqgrad, T(0.45/2));
    }

    cv::Point_<T> algebraicGradient(cv::Point_<T> p)
    {
        return cv::Point_<T>(2*A*p.x + B*p.y + D, B*p.x + 2*C*p.y + E);
    }

    cv::Point_<T> algebraicGradientDir(cv::Point_<T> p)
    {
        cv::Point_<T> grad = algebraicGradient(p);
        T len = std::sqrt(grad.ddot(grad));
        grad.x /= len;
        grad.y /= len;
        return grad;
    }

protected:

    void initFromEllipse(cv::Point_<T> axis, cv::Point_<T> centre, T a, T b)
    {
        T a2 = a * a;
        T b2 = b * b;

        A = axis.x*axis.x / a2 + axis.y*axis.y / b2;
        B = 2*axis.x*axis.y / a2 - 2*axis.x*axis.y / b2;
        C = axis.y*axis.y / a2 + axis.x*axis.x / b2;
        D = (-2*axis.x*axis.y*centre.y - 2*axis.x*axis.x*centre.x) / a2
            + (2*axis.x*axis.y*centre.y - 2*axis.y*axis.y*centre.x) / b2;
        E = (-2*axis.x*axis.y*centre.x - 2*axis.y*axis.y*centre.y) / a2
            + (2*axis.x*axis.y*centre.x - 2*axis.x*axis.x*centre.y) / b2;
        F = (2*axis.x*axis.y*centre.x*centre.y + axis.x*axis.x*centre.x*centre.x + axis.y*axis.y*centre.y*centre.y) / a2
            + (-2*axis.x*axis.y*centre.x*centre.y + axis.y*axis.y*centre.x*centre.x + axis.x*axis.x*centre.y*centre.y) / b2
            - 1;
    }

};

typedef ConicSection_<float> ConicSection;

#endif //PUPILALGOSIMPLE_SWIRSKI2D_H
