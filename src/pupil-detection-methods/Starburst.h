/*
 *
 * cvEyeTracker is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * cvEyeTracker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cvEyeTracker; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 * cvEyeTracker - Version 1.2.5
 * Part of the openEyes ToolKit -- http://hcvl.hci.iastate.edu/openEyes
 * Release Date:
 * Authors : Dongheng Li <dhli@iastate.edu>
 *           Derrick Parkhurst <derrick.parkhurst@hcvl.hci.iastate.edu>
 *           Jason Babcock <babcock@nyu.edu>
 *           David Winfield <dwinfiel@iastate.edu>
 * Copyright (c) 2004-2006
 * All Rights Reserved.
 *
 * Modified by: Andrew Kurauchi, Moritz Lode
 *
 */

/*
 * Starburst.h
 *
 * Based on the code from:
 * cvEyeTracker - Version 1.2.5
 * Part of the openEyes ToolKit -- http://hcvl.hci.iastate.edu/openEyes
 * Release Date:
 * Authors : Dongheng Li <dhli@iastate.edu>
 *           Derrick Parkhurst <derrick.parkhurst@hcvl.hci.iastate.edu>
 *           Jason Babcock <babcock@nyu.edu>
 *           David Winfield <dwinfiel@iastate.edu>
 * Copyright (c) 2004-2006
 * All Rights Reserved.
 *
 * Modified by Moritz Lode 2020
 *
 */

#ifndef PUPILALGOSIMPLE_STARBURST_H
#define PUPILALGOSIMPLE_STARBURST_H

#include "PupilDetectionMethod.h"

#define UINT8 unsigned char
//svd function
#define SIGN(u, v) ((v) >= 0.0 ? fabs(u) : -fabs(u))
#ifndef MAX
#define MAX(x, y) ((x) >= (y) ? (x) : (y))
#endif
#define FIX_UINT8(x) ((x) < 0 ? 0 : ((x) > 255 ? 255 : (x)))

class RansacEllipse
{

public:
    RansacEllipse()
    {
    }

    int starburst_pupil_contour_detection(UINT8 *pupil_image, const cv::Point2d &startPoint, int width, int height, int edge_thresh, int N, int minimum_cadidate_features);
    int *pupil_fitting_inliers(UINT8 *pupil_image, int, int, int &return_max_inliers);

    std::vector<cv::Point2d *> edge_point;
    double pupil_param[5];

private:
    static double radius(double u, double v)
    {
        double w;
        u = fabs(u);
        v = fabs(v);

        if (u > v)
        {
            w = v / u;
            return (u * sqrt(1. + w * w));
        }
        else
        {
            if (v)
            {
                w = u / v;
                return (v * sqrt(1. + w * w));
            }
            else
                return 0.0;
        }
    }

    void svd(int m, int n, double **a, double **p, double *d, double **q);
    void destroy_edge_point();
    void locate_edge_points(const UINT8 *image, int width, int height, double cx, double cy, int dis, double angle_step, double angle_normal, double angle_spread, int edge_thresh);
    cv::Point2d get_edge_mean();
    cv::Point2d *normalize_edge_point(double &dis_scale, cv::Point2d &nor_center, int ep_num);
    bool solve_ellipse(double *conic_param, double *ellipse_param);
    void denormalize_ellipse_param(double *par, double *normailized_par, double dis_scale, cv::Point2d nor_center);
    void get_random_num(int n, int max_num, int *rand_num);

    std::vector<int> edge_intensity_diff;
};

class Starburst : public PupilDetectionMethod
{

public:
    int edge_threshold = 16;                        //threshold of pupil edge points detection
    int rays = 18;                                  //number of rays to use to detect feature points
    int min_feature_candidates = 10;                //minimum number of pupil feature candidates
    int corneal_reflection_ratio_to_image_size = 2; // approx max size of the reflection relative to image height -> height/this
    int crWindowSize = 301;                         //corneal reflection search window size

    //const double beta = 0.2;           //hysteresis factor for noise reduction

    Starburst() : ransacEllipse(), avgIntensityHori(nullptr), intensityFactorHori(nullptr), curH(0), startPoint(0, 0), lostFrameNum(0), imageSize(0, 0)
    {
        mDesc = "Starburst ()";
        mTitle = "Starburst";
    }

    ~Starburst()
    {
        this->deleteArrays();
    }

    Pupil run(const cv::Mat &frame) override
    {
        Pupil pupil;
        run(frame, pupil);
        return pupil;
    }

    void run(const cv::Mat &frame, Pupil &pupil) override;

    bool hasConfidence() override
    {
        return false;
    }

    bool hasCoarseLocation() override
    {
        return false;
    }

    bool hasInliers() override
    {
        return false;
    }

private:
    RansacEllipse ransacEllipse;
    cv::Point2d startPoint;
    double *avgIntensityHori;    //horizontal average intensity
    double *intensityFactorHori; //horizontal intensity factor for noise reduction
    int curH;
    int lostFrameNum;

    cv::Size imageSize;

    //void reduceLineNoise(cv::Mat &inImage);
    //void calculateAvgIntensityHori(cv::Mat &inImage);
    //void recreateIntensityArrays(cv::Mat &refImage);
    void deleteArrays();
};

#endif //PUPILALGOSIMPLE_STARBURST_H
