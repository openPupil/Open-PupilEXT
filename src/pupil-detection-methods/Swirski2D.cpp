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

#include "Swirski2D.h"
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>

#include <boost/foreach.hpp>
#include <boost/math/special_functions/fpclassify.hpp>
#include <tbb/tbb.h>

#include <iostream>
#include <set>
#include <random>
#include <algorithm>

const double SQRT_2 = std::sqrt(2.0);
const cv::Point2f UNKNOWN_POSITION = cv::Point2f(-1, -1);

template <typename T>
inline cv::Rect_<T> roiAround(T x, T y, T radius)
{
    return cv::Rect_<T>(x - radius, y - radius, 2 * radius + 1, 2 * radius + 1);
}

template <typename T>
inline cv::Rect_<T> roiAround(const cv::Point_<T> &centre, T radius)
{
    return roiAround(centre.x, centre.y, radius);
}

static std::mt19937 static_gen;

int random(int min, int max)
{
    std::uniform_int_distribution<> distribution(min, max);
    return distribution(static_gen);
}

int random(int min, int max, unsigned int seed)
{
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> distribution(min, max);
    return distribution(gen);
}

template <typename T>
std::vector<T> randomSubset(const std::vector<T> &src, typename std::vector<T>::size_type size)
{
    if (size > src.size())
        throw std::range_error("Subset size out of range");

    std::vector<T> ret;
    std::set<size_t> vals;

    for (size_t j = src.size() - size; j < src.size(); ++j)
    {
        size_t idx = random(0, j); // generate a random integer in range [0, j]

        if (vals.find(idx) != vals.end())
            idx = j;

        ret.push_back(src[idx]);
        vals.insert(idx);
    }

    return ret;
}

template <typename T>
std::vector<T> randomSubset(const std::vector<T> &src, typename std::vector<T>::size_type size, unsigned int seed)
{
    if (size > src.size())
        throw std::range_error("Subset size out of range");

    std::vector<T> ret;
    std::set<size_t> vals;

    for (size_t j = src.size() - size; j < src.size(); ++j)
    {
        size_t idx = random(0, j, seed + j); // generate a random integer in range [0, j]

        if (vals.find(idx) != vals.end())
            idx = j;

        ret.push_back(src[idx]);
        vals.insert(idx);
    }

    return ret;
}

cv::Vec2f majorAxis(const cv::RotatedRect &ellipse)
{
    return cv::Vec2f(ellipse.size.width * std::cos(CV_PI / 180 * ellipse.angle), ellipse.size.width * std::sin(CV_PI / 180 * ellipse.angle));
}

void getROI(const cv::Mat &src, cv::Mat &dst, const cv::Rect &roi, int borderType)
{
    cv::Rect bbSrc = cv::Rect(0, 0, src.cols, src.rows);
    cv::Rect validROI = roi & bbSrc;

    if (validROI == roi)
    {
        dst = cv::Mat(src, validROI);
    }
    else
    {
        // Figure out how much to add on for top, left, right and bottom
        cv::Point tl = roi.tl() - bbSrc.tl();
        cv::Point br = roi.br() - bbSrc.br();

        int top = std::max(-tl.y, 0);  // Top and left are negated because adding a border
        int left = std::max(-tl.x, 0); // goes "the wrong way"
        int right = std::max(br.x, 0);
        int bottom = std::max(br.y, 0);

        cv::Mat tmp(src, validROI);
        cv::copyMakeBorder(tmp, dst, top, bottom, left, right, borderType);
    }
}

cv::RotatedRect fitEllipse(const cv::Moments &m)
{
    cv::RotatedRect ret;

    ret.center.x = m.m10 / m.m00;
    ret.center.y = m.m01 / m.m00;

    double mu20 = m.m20 / m.m00 - ret.center.x * ret.center.x;
    double mu02 = m.m02 / m.m00 - ret.center.y * ret.center.y;
    double mu11 = m.m11 / m.m00 - ret.center.x * ret.center.y;

    double common = std::sqrt(std::pow(mu20 - mu02, 2) + 4 * std::pow(mu11, 2));

    ret.size.width = std::sqrt(2 * (mu20 + mu02 + common));
    ret.size.height = std::sqrt(2 * (mu20 + mu02 - common));

    double num, den;
    if (mu02 > mu20)
    {
        num = mu02 - mu20 + common;
        den = 2 * mu11;
    }
    else
    {
        num = 2 * mu11;
        den = mu20 - mu02 + common;
    }

    if (num == 0 && den == 0)
        ret.angle = 0;
    else
        ret.angle = (180 / CV_PI) * std::atan2(num, den);

    return ret;
}

float histKmeans(const cv::Mat_<float> &hist, int bin_min, int bin_max, int K, float init_centres[], cv::Mat_<uchar> &labels, cv::TermCriteria termCriteria)
{
    CV_Assert(hist.rows == 1 || hist.cols == 1 && K > 0);

    labels = cv::Mat_<uchar>::zeros(hist.size());
    int nbins = hist.total();
    float binWidth = (bin_max - bin_min) / nbins;
    float binStart = bin_min + binWidth / 2;

    cv::Mat_<float> centres(K, 1, init_centres, 4);

    int iters = 0;
    bool finalRun = false;
    while (true)
    {
        ++iters;
        cv::Mat_<float> old_centres = centres.clone();

        int i_bin;
        cv::Mat_<float>::const_iterator i_hist;
        cv::Mat_<uchar>::iterator i_labels;
        cv::Mat_<float>::iterator i_centres;
        uchar label;

        float sumDist = 0;
        int movedCount = 0;

        // Step 1. Assign each element a label
        for (i_bin = 0, i_labels = labels.begin(), i_hist = hist.begin();
             i_bin < nbins;
             ++i_bin, ++i_labels, ++i_hist)
        {
            float bin_val = binStart + i_bin * binWidth;
            float minDist = std::pow(bin_val - centres(*i_labels), 2);
            int curLabel = *i_labels;

            for (label = 0; label < K; ++label)
            {
                float dist = std::pow(bin_val - centres(label), 2);
                if (dist < minDist)
                {
                    minDist = dist;
                    *i_labels = label;
                }
            }

            if (*i_labels != curLabel)
                movedCount++;

            sumDist += (*i_hist) * std::sqrt(minDist);
        }

        if (finalRun)
            return sumDist;

        // Step 2. Recalculate centres
        cv::Mat_<float> counts(K, 1, 0.0f);
        for (i_bin = 0, i_labels = labels.begin(), i_hist = hist.begin();
             i_bin < nbins;
             ++i_bin, ++i_labels, ++i_hist)
        {
            float bin_val = binStart + i_bin * binWidth;

            centres(*i_labels) += (*i_hist) * bin_val;
            counts(*i_labels) += *i_hist;
        }

        for (label = 0; label < K; ++label)
        {
            if (counts(label) == 0)
                return std::numeric_limits<float>::infinity();

            centres(label) /= counts(label);
        }

        // Step 3. Detect termination criteria
        if (movedCount == 0)
            finalRun = true;
        else if (termCriteria.type | cv::TermCriteria::COUNT && iters >= termCriteria.maxCount)
            finalRun = true;
        else if (termCriteria.type | cv::TermCriteria::EPS)
        {
            float max_movement = 0;
            for (label = 0; label < K; ++label)
            {
                max_movement = std::max(max_movement, (float)std::pow(centres(label) - old_centres(label), 2));
            }
            if (sqrt(max_movement) < termCriteria.epsilon)
                finalRun = true;
        }
    }

    return std::numeric_limits<float>::infinity();
}

void Swirski2D::run(const cv::Mat &frame, Pupil &pupil)
{
    std::vector<cv::Point2f> inlierPts;
    pupil.clear();
    run(frame, pupil, inlierPts);
}

cv::Rect Swirski2D::findMaxHaarResponse(const cv::Mat &frame)
{

    cv::Mat_<uchar> mEye = frame;

    // -----------------------
    // Find best haar response
    // -----------------------

    //             _____________________
    //            |         Haar kernel |
    //            |                     |
    //  __________|______________       |
    // | Image    |      |       |      |
    // |    ______|______|___.-r-|--2r--|
    // |   |      |      |___|___|      |
    // |   |      |          |   |      |
    // |   |      |          |   |      |
    // |   |      |__________|___|______|
    // |   |    Search       |   |
    // |   |    region       |   |
    // |   |                 |   |
    // |   |_________________|   |
    // |                         |
    // |_________________________|
    //

    cv::Mat_<int32_t> mEyeIntegral;
    int padding = 2 * params.Radius_Max;

    cv::Mat mEyePad;
    // Need to pad by an additional 1 to get bottom & right edges.
    cv::copyMakeBorder(mEye, mEyePad, padding, padding, padding, padding, cv::BORDER_REPLICATE);
    cv::integral(mEyePad, mEyeIntegral);

    cv::Point2f pHaarPupil;
    int haarRadius = 0;

    const int rstep = 2;
    const int ystep = 4;
    const int xstep = 4;

    double minResponse = std::numeric_limits<double>::infinity();

    for (int r = params.Radius_Min; r < params.Radius_Max; r += rstep)
    {
        // Get Haar feature
        int r_inner = r;
        int r_outer = 3 * r;
        HaarSurroundFeature f(r_inner, r_outer);

        // Use TBB for rows
        std::pair<double, cv::Point2f> minRadiusResponse = tbb::parallel_reduce(
            tbb::blocked_range<int>(0, (mEye.rows - r - r - 1) / ystep + 1,
                                    ((mEye.rows - r - r - 1) / ystep + 1) / 8),
            std::make_pair(std::numeric_limits<double>::infinity(), UNKNOWN_POSITION),
            [&](tbb::blocked_range<int> range,
                const std::pair<double, cv::Point2f> &minValIn) -> std::pair<double, cv::Point2f>
            {
                std::pair<double, cv::Point2f> minValOut = minValIn;
                for (int i = range.begin(), y = r + range.begin() * ystep; i < range.end(); i++, y += ystep)
                {
                    //            �         �
                    // row1_outer.|         |  p00._____________________.p01
                    //            |         |     |         Haar kernel |
                    //            |         |     |                     |
                    // row1_inner.|         |     |   p00._______.p01   |
                    //            |-padding-|     |      |       |      |
                    //            |         |     |      | (x,y) |      |
                    // row2_inner.|         |     |      |_______|      |
                    //            |         |     |   p10'       'p11   |
                    //            |         |     |                     |
                    // row2_outer.|         |     |_____________________|
                    //            |         |  p10'                     'p11
                    //            �         �

                    int *row1_inner = mEyeIntegral[y + padding - r_inner];
                    int *row2_inner = mEyeIntegral[y + padding + r_inner + 1];
                    int *row1_outer = mEyeIntegral[y + padding - r_outer];
                    int *row2_outer = mEyeIntegral[y + padding + r_outer + 1];

                    int *p00_inner = row1_inner + r + padding - r_inner;
                    int *p01_inner = row1_inner + r + padding + r_inner + 1;
                    int *p10_inner = row2_inner + r + padding - r_inner;
                    int *p11_inner = row2_inner + r + padding + r_inner + 1;

                    int *p00_outer = row1_outer + r + padding - r_outer;
                    int *p01_outer = row1_outer + r + padding + r_outer + 1;
                    int *p10_outer = row2_outer + r + padding - r_outer;
                    int *p11_outer = row2_outer + r + padding + r_outer + 1;

                    for (int x = r; x < mEye.cols - r; x += xstep)
                    {
                        int sumInner = *p00_inner + *p11_inner - *p01_inner - *p10_inner;
                        int sumOuter = *p00_outer + *p11_outer - *p01_outer - *p10_outer - sumInner;

                        double response = f.val_inner * sumInner + f.val_outer * sumOuter;

                        if (response < minValOut.first)
                        {
                            minValOut.first = response;
                            minValOut.second = cv::Point(x, y);
                        }

                        p00_inner += xstep;
                        p01_inner += xstep;
                        p10_inner += xstep;
                        p11_inner += xstep;

                        p00_outer += xstep;
                        p01_outer += xstep;
                        p10_outer += xstep;
                        p11_outer += xstep;
                    }
                }
                return minValOut;
            },
            [](const std::pair<double, cv::Point2f> &x,
               const std::pair<double, cv::Point2f> &y) -> std::pair<double, cv::Point2f>
            {
                if (x.first < y.first)
                    return x;
                else
                    return y;
            });

        if (minRadiusResponse.first < minResponse)
        {
            minResponse = minRadiusResponse.first;
            // Set return values
            pHaarPupil = minRadiusResponse.second;
            haarRadius = r;
        }
    }

    // Paradoxically, a good Haar fit won't catch the entire pupil, so expand it a bit
    haarRadius = (int)(haarRadius * SQRT_2);

    // ---------------------------
    // Pupil ROI around Haar point
    // ---------------------------
    cv::Rect roiHaarPupil = roiAround(cv::Point(pHaarPupil.x, pHaarPupil.y), haarRadius);
    //cv::Mat_<uchar> mHaarPupil;
    //getROI(mEye, mHaarPupil, roiHaarPupil, cv::BORDER_REPLICATE);

    return roiHaarPupil;
}

void Swirski2D::run(const cv::Mat &frame, Pupil &pupil, std::vector<cv::Point2f> &inlierPts)
{

    cv::Mat_<uchar> mEye = frame;

    // Pick one channel if necessary, and crop it to get rid of borders
    //    if (frame.channels() == 1)
    //    {
    //        mEye = frame;
    //    }
    //    else if (frame.channels() == 3)
    //    {
    //        cv::cvtColor(frame, mEye, cv::COLOR_BGR2GRAY);
    //    }
    //    else if (frame.channels() == 4)
    //    {
    //        cv::cvtColor(frame, mEye, cv::COLOR_BGRA2GRAY);
    //    }
    //    else
    //    {
    //        throw std::runtime_error("Unsupported number of channels");
    //    }

    // -----------------------
    // Find best haar response
    // -----------------------

    //             _____________________
    //            |         Haar kernel |
    //            |                     |
    //  __________|______________       |
    // | Image    |      |       |      |
    // |    ______|______|___.-r-|--2r--|
    // |   |      |      |___|___|      |
    // |   |      |          |   |      |
    // |   |      |          |   |      |
    // |   |      |__________|___|______|
    // |   |    Search       |   |
    // |   |    region       |   |
    // |   |                 |   |
    // |   |_________________|   |
    // |                         |
    // |_________________________|
    //

    cv::Mat_<int32_t> mEyeIntegral;
    int padding = 2 * params.Radius_Max;

    cv::Mat mEyePad;
    // Need to pad by an additional 1 to get bottom & right edges.
    cv::copyMakeBorder(mEye, mEyePad, padding, padding, padding, padding, cv::BORDER_REPLICATE);
    cv::integral(mEyePad, mEyeIntegral);

    cv::Point2f pHaarPupil;
    int haarRadius = 0;

    const int rstep = 2;
    const int ystep = 4;
    const int xstep = 4;

    double minResponse = std::numeric_limits<double>::infinity();

    for (int r = params.Radius_Min; r < params.Radius_Max; r += rstep)
    {
        // Get Haar feature
        int r_inner = r;
        int r_outer = 3 * r;
        HaarSurroundFeature f(r_inner, r_outer);

        // Use TBB for rows
        std::pair<double, cv::Point2f> minRadiusResponse = tbb::parallel_reduce(
            tbb::blocked_range<int>(0, (mEye.rows - r - r - 1) / ystep + 1, ((mEye.rows - r - r - 1) / ystep + 1) / 8),
            std::make_pair(std::numeric_limits<double>::infinity(), UNKNOWN_POSITION),
            [&](tbb::blocked_range<int> range, const std::pair<double, cv::Point2f> &minValIn) -> std::pair<double, cv::Point2f>
            {
                std::pair<double, cv::Point2f> minValOut = minValIn;
                for (int i = range.begin(), y = r + range.begin() * ystep; i < range.end(); i++, y += ystep)
                {
                    //            �         �
                    // row1_outer.|         |  p00._____________________.p01
                    //            |         |     |         Haar kernel |
                    //            |         |     |                     |
                    // row1_inner.|         |     |   p00._______.p01   |
                    //            |-padding-|     |      |       |      |
                    //            |         |     |      | (x,y) |      |
                    // row2_inner.|         |     |      |_______|      |
                    //            |         |     |   p10'       'p11   |
                    //            |         |     |                     |
                    // row2_outer.|         |     |_____________________|
                    //            |         |  p10'                     'p11
                    //            �         �

                    int *row1_inner = mEyeIntegral[y + padding - r_inner];
                    int *row2_inner = mEyeIntegral[y + padding + r_inner + 1];
                    int *row1_outer = mEyeIntegral[y + padding - r_outer];
                    int *row2_outer = mEyeIntegral[y + padding + r_outer + 1];

                    int *p00_inner = row1_inner + r + padding - r_inner;
                    int *p01_inner = row1_inner + r + padding + r_inner + 1;
                    int *p10_inner = row2_inner + r + padding - r_inner;
                    int *p11_inner = row2_inner + r + padding + r_inner + 1;

                    int *p00_outer = row1_outer + r + padding - r_outer;
                    int *p01_outer = row1_outer + r + padding + r_outer + 1;
                    int *p10_outer = row2_outer + r + padding - r_outer;
                    int *p11_outer = row2_outer + r + padding + r_outer + 1;

                    for (int x = r; x < mEye.cols - r; x += xstep)
                    {
                        int sumInner = *p00_inner + *p11_inner - *p01_inner - *p10_inner;
                        int sumOuter = *p00_outer + *p11_outer - *p01_outer - *p10_outer - sumInner;

                        double response = f.val_inner * sumInner + f.val_outer * sumOuter;

                        if (response < minValOut.first)
                        {
                            minValOut.first = response;
                            minValOut.second = cv::Point(x, y);
                        }

                        p00_inner += xstep;
                        p01_inner += xstep;
                        p10_inner += xstep;
                        p11_inner += xstep;

                        p00_outer += xstep;
                        p01_outer += xstep;
                        p10_outer += xstep;
                        p11_outer += xstep;
                    }
                }
                return minValOut;
            },
            [](const std::pair<double, cv::Point2f> &x, const std::pair<double, cv::Point2f> &y) -> std::pair<double, cv::Point2f>
            {
                if (x.first < y.first)
                    return x;
                else
                    return y;
            });

        if (minRadiusResponse.first < minResponse)
        {
            minResponse = minRadiusResponse.first;
            // Set return values
            pHaarPupil = minRadiusResponse.second;
            haarRadius = r;
        }
    }

    // Paradoxically, a good Haar fit won't catch the entire pupil, so expand it a bit
    haarRadius = (int)(haarRadius * SQRT_2);

    // ---------------------------
    // Pupil ROI around Haar point
    // ---------------------------
    cv::Rect roiHaarPupil = roiAround(cv::Point(pHaarPupil.x, pHaarPupil.y), haarRadius);
    cv::Mat_<uchar> mHaarPupil;
    getROI(mEye, mHaarPupil, roiHaarPupil, cv::BORDER_REPLICATE);

    //out.roiHaarPupil = roiHaarPupil;
    //out.mHaarPupil = mHaarPupil;

    // --------------------------------------------------
    // Get histogram of pupil region, segment with KMeans
    // --------------------------------------------------
    const int bins = 256;
    cv::Mat_<float> hist;

    int channels[] = {0};
    int sizes[] = {bins};
    float range[2] = {0, 256};
    const float *ranges[] = {range};
    cv::calcHist(&mHaarPupil, 1, channels, cv::Mat(), hist, 1, sizes, ranges);

    //out.histPupil = hist;

    float threshold;

    // Try various candidate centres, return the one with minimal label distance
    float candidate0[2] = {0, 0};
    float candidate1[2] = {128, 255};
    float bestDist = std::numeric_limits<float>::infinity();
    float bestThreshold = std::numeric_limits<float>::quiet_NaN();

    for (int i = 0; i < 2; i++)
    {
        cv::Mat_<uchar> labels;
        float centres[2] = {candidate0[i], candidate1[i]};
        float dist = histKmeans(hist, 0, 256, 2, centres, labels, cv::TermCriteria(cv::TermCriteria::COUNT, 50, 0.0));

        float thisthreshold = (centres[0] + centres[1]) / 2;
        if (dist < bestDist && boost::math::isnormal(thisthreshold))
        {
            bestDist = dist;
            bestThreshold = thisthreshold;
        }
    }

    if (!boost::math::isnormal(bestThreshold))
    {
        // If kmeans gives a degenerate solution, exit early
        cv::RotatedRect ellipse;
        ellipse.center.x = 0;
        ellipse.center.y = 0;
        ellipse.angle = 0.0;
        ellipse.size.height = 0.0;
        ellipse.size.width = 0.0;

        pupil = ellipse;
        //return ellipse;
    }

    threshold = bestThreshold;

    cv::Mat_<uchar> mPupilThresh;
    cv::threshold(mHaarPupil, mPupilThresh, threshold, 255, cv::THRESH_BINARY_INV);

    //out.threshold = threshold;
    //out.mPupilThresh = mPupilThresh;

    // ---------------------------------------------
    // Find best region in the segmented pupil image
    // ---------------------------------------------

    cv::Rect bbPupilThresh;
    cv::RotatedRect elPupilThresh;

    cv::Mat_<uchar> mPupilContours = mPupilThresh.clone();
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mPupilContours, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

    if (contours.size() == 0)
    {
        cv::RotatedRect ellipse;
        ellipse.center.x = 0;
        ellipse.center.y = 0;
        ellipse.angle = 0.0;
        ellipse.size.height = 0.0;
        ellipse.size.width = 0.0;

        pupil = ellipse;
        return;
    }

    std::vector<cv::Point> &maxContour = contours[0];
    double maxContourArea = cv::contourArea(maxContour);
    BOOST_FOREACH (std::vector<cv::Point> &c, contours)
    {
        double area = cv::contourArea(c);
        if (area > maxContourArea)
        {
            maxContourArea = area;
            maxContour = c;
        }
    }

    cv::Moments momentsPupilThresh = cv::moments(maxContour);

    bbPupilThresh = cv::boundingRect(maxContour);
    elPupilThresh = fitEllipse(momentsPupilThresh);

    // Shift best region into eye coords (instead of pupil region coords), and get ROI
    bbPupilThresh.x += roiHaarPupil.x;
    bbPupilThresh.y += roiHaarPupil.y;
    elPupilThresh.center.x += roiHaarPupil.x;
    elPupilThresh.center.y += roiHaarPupil.y;

    //out.bbPupilThresh = bbPupilThresh;
    //out.elPupilThresh = elPupilThresh;

    // ------------------------------
    // Find edges in new pupil region
    // ------------------------------

    cv::Mat_<uchar> mPupil, mPupilOpened, mPupilBlurred, mPupilEdges;
    cv::Mat_<float> mPupilSobelX, mPupilSobelY;
    cv::Rect bbPupil;
    cv::Rect roiPupil = roiAround(cv::Point(elPupilThresh.center.x, elPupilThresh.center.y), haarRadius);

    padding = 3;

    cv::Rect roiPadded(roiPupil.x - padding, roiPupil.y - padding, roiPupil.width + 2 * padding, roiPupil.height + 2 * padding);
    // First get an ROI around the approximate pupil location
    getROI(mEye, mPupil, roiPadded, cv::BORDER_REPLICATE);

    cv::Mat morphologyDisk = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
    cv::morphologyEx(mPupil, mPupilOpened, cv::MORPH_OPEN, morphologyDisk, cv::Point(-1, -1), 2);

    if (params.CannyBlur > 0)
    {
        cv::GaussianBlur(mPupilOpened, mPupilBlurred, cv::Size(), params.CannyBlur);
    }
    else
    {
        mPupilBlurred = mPupilOpened;
    }

    cv::Sobel(mPupilBlurred, mPupilSobelX, CV_32F, 1, 0, 3);
    cv::Sobel(mPupilBlurred, mPupilSobelY, CV_32F, 0, 1, 3);

    cv::Canny(mPupilBlurred, mPupilEdges, params.CannyThreshold1, params.CannyThreshold2);

    cv::Rect roiUnpadded(padding, padding, roiPupil.width, roiPupil.height);
    mPupil = cv::Mat(mPupil, roiUnpadded);
    mPupilOpened = cv::Mat(mPupilOpened, roiUnpadded);
    mPupilBlurred = cv::Mat(mPupilBlurred, roiUnpadded);
    mPupilSobelX = cv::Mat(mPupilSobelX, roiUnpadded);
    mPupilSobelY = cv::Mat(mPupilSobelY, roiUnpadded);
    mPupilEdges = cv::Mat(mPupilEdges, roiUnpadded);

    bbPupil = cv::Rect(0, 0, mPupil.cols, mPupil.rows);

    //out.roiPupil = roiPupil;
    //out.mPupil = mPupil;
    //out.mPupilOpened = mPupilOpened;
    //out.mPupilBlurred = mPupilBlurred;
    //out.mPupilSobelX = mPupilSobelX;
    //out.mPupilSobelY = mPupilSobelY;
    //out.mPupilEdges = mPupilEdges;

    // -----------------------------------------------
    // Get points on edges, optionally using starburst
    // -----------------------------------------------

    std::vector<cv::Point2f> edgePoints;

    if (params.StarburstPoints > 0)
    {
        // Starburst from initial pupil approximation, stopping when an edge is hit.
        // Collect all edge points into a vector

        // The initial pupil approximations are:
        //    Centre of mass of thresholded region
        //    Halfway along the major axis (calculated form second moments) in each direction

        tbb::concurrent_vector<cv::Point2f> edgePointsConcurrent;

        cv::Vec2f elPupil_majorAxis = majorAxis(elPupilThresh);
        std::vector<cv::Point2f> centres;
        centres.push_back(elPupilThresh.center - cv::Point2f(roiPupil.tl().x, roiPupil.tl().y));
        centres.push_back(elPupilThresh.center - cv::Point2f(roiPupil.tl().x, roiPupil.tl().y) + cv::Point2f(elPupil_majorAxis));
        centres.push_back(elPupilThresh.center - cv::Point2f(roiPupil.tl().x, roiPupil.tl().y) - cv::Point2f(elPupil_majorAxis));

        BOOST_FOREACH (const cv::Point2f &centre, centres)
        {
            tbb::parallel_for(0, params.StarburstPoints, [&](int i)
                              {
                                  double theta = i * 2 * CV_PI / params.StarburstPoints;

                                  // Initialise centre and direction vector
                                  cv::Point2f pDir((float)std::cos(theta), (float)std::sin(theta));

                                  int t = 1;
                                  cv::Point p = centre + (t * pDir);
                                  while (p.inside(bbPupil))
                                  {
                                      uchar val = mPupilEdges(p);

                                      if (val > 0)
                                      {
                                          float dx = mPupilSobelX(p);
                                          float dy = mPupilSobelY(p);

                                          float cdirx = p.x - (elPupilThresh.center.x - roiPupil.x);
                                          float cdiry = p.y - (elPupilThresh.center.y - roiPupil.y);

                                          // Check edge direction
                                          double dirCheck = dx * cdirx + dy * cdiry;

                                          if (dirCheck > 0)
                                          {
                                              // We've hit an edge
                                              edgePointsConcurrent.push_back(cv::Point2f(p.x + 0.5f, p.y + 0.5f));
                                              break;
                                          }
                                      }

                                      ++t;
                                      p = centre + (t * pDir);
                                  }
                              });
        }

        edgePoints = std::vector<cv::Point2f>(edgePointsConcurrent.begin(), edgePointsConcurrent.end());

        // Remove duplicate edge points
        std::sort(edgePoints.begin(), edgePoints.end(), [](const cv::Point2f &p1, const cv::Point2f &p2) -> bool
                  {
                      if (p1.x == p2.x)
                          return p1.y < p2.y;
                      else
                          return p1.x < p2.x;
                  });
        edgePoints.erase(std::unique(edgePoints.begin(), edgePoints.end()), edgePoints.end());

        if (edgePoints.size() < params.StarburstPoints / 2)
        {
            cv::RotatedRect ellipse;
            ellipse.center.x = 0;
            ellipse.center.y = 0;
            ellipse.angle = 0.0;
            ellipse.size.height = 0.0;
            ellipse.size.width = 0.0;

            pupil = ellipse;
            return;
        }
    }
    else
    {
        for (int y = 0; y < mPupilEdges.rows; y++)
        {
            uchar *val = mPupilEdges[y];
            for (int x = 0; x < mPupilEdges.cols; x++, val++)
            {
                if (*val == 0)
                    continue;

                edgePoints.push_back(cv::Point2f(x + 0.5f, y + 0.5f));
            }
        }
    }

    // ---------------------------
    // Fit an ellipse to the edges
    // ---------------------------
    cv::RotatedRect elPupil;
    std::vector<cv::Point2f> inliers;

    // Desired probability that only inliers are selected
    const double p = 0.999;
    // Probability that a point is an inlier
    double w = params.PercentageInliers / 100.0;
    // Number of points needed for a model
    const int n = 5;

    if (params.PercentageInliers == 0)
    {
        cv::RotatedRect ellipse;
        ellipse.center.x = 0;
        ellipse.center.y = 0;
        ellipse.angle = 0.0;
        ellipse.size.height = 0.0;
        ellipse.size.width = 0.0;

        pupil = ellipse;
        return;
    }

    if (edgePoints.size() >= n) // Minimum points for ellipse
    {
        // RANSAC!!!
        double wToN = std::pow(w, n);
        int k = static_cast<int>(std::log(1 - p) / std::log(1 - wToN) + 2 * std::sqrt(1 - wToN) / wToN);

        //out.ransacIterations = k;

        //size_t threshold_inlierCount = std::max<size_t>(n, static_cast<size_t>(out.edgePoints.size() * 0.7));

        // Use TBB for RANSAC
        struct EllipseRansac_out
        {
            std::vector<cv::Point2f> bestInliers;
            cv::RotatedRect bestEllipse;
            double bestEllipseGoodness;
            int earlyRejections;
            bool earlyTermination;

            EllipseRansac_out() : bestEllipseGoodness(-std::numeric_limits<double>::infinity()), earlyTermination(false), earlyRejections(0) {}
        };

        struct EllipseRansac
        {
            const TrackerParams &params;
            const std::vector<cv::Point2f> &edgePoints;
            int n;
            const cv::Rect &bb;
            const cv::Mat_<float> &mDX;
            const cv::Mat_<float> &mDY;
            int earlyRejections;
            bool earlyTermination;

            EllipseRansac_out out;

            EllipseRansac(
                const TrackerParams &params,
                const std::vector<cv::Point2f> &edgePoints,
                int n,
                const cv::Rect &bb,
                const cv::Mat_<float> &mDX,
                const cv::Mat_<float> &mDY)
                : params(params), edgePoints(edgePoints), n(n), bb(bb), mDX(mDX), mDY(mDY), earlyTermination(false), earlyRejections(0)
            {
            }

            EllipseRansac(EllipseRansac &other, tbb::split)
                : params(other.params), edgePoints(other.edgePoints), n(other.n), bb(other.bb), mDX(other.mDX), mDY(other.mDY), earlyTermination(other.earlyTermination), earlyRejections(other.earlyRejections)
            {
                //std::cout << "Ransac split" << std::endl;
            }

            void operator()(const tbb::blocked_range<size_t> &r)
            {
                if (out.earlyTermination)
                    return;
                //std::cout << "Ransac start (" << (r.end()-r.begin()) << " elements)" << std::endl;
                for (size_t i = r.begin(); i != r.end(); ++i)
                {
                    // Ransac Iteration
                    // ----------------
                    std::vector<cv::Point2f> sample;
                    if (params.Seed >= 0)
                        sample = randomSubset(edgePoints, n, static_cast<unsigned int>(i + params.Seed));
                    else
                        sample = randomSubset(edgePoints, n);

                    cv::RotatedRect ellipseSampleFit = fitEllipse(sample);
                    // Normalise ellipse to have width as the major axis.
                    if (ellipseSampleFit.size.height > ellipseSampleFit.size.width)
                    {
                        ellipseSampleFit.angle = std::fmod(ellipseSampleFit.angle + 90, 180);
                        std::swap(ellipseSampleFit.size.height, ellipseSampleFit.size.width);
                    }

                    cv::Size s = ellipseSampleFit.size;
                    // Discard useless ellipses early
                    if (!ellipseSampleFit.center.inside(bb) || s.height > params.Radius_Max * 2 || s.width > params.Radius_Max * 2 || s.height < params.Radius_Min * 2 && s.width < params.Radius_Min * 2 || s.height > 4 * s.width || s.width > 4 * s.height)
                    {
                        // Bad ellipse! Go to your room!
                        continue;
                    }

                    // Use conic section's algebraic distance as an error measure
                    ConicSection conicSampleFit(ellipseSampleFit);

                    // Check if sample's gradients are correctly oriented
                    if (params.EarlyRejection)
                    {
                        bool gradientCorrect = true;
                        BOOST_FOREACH (const cv::Point2f &p, sample)
                        {
                            cv::Point2f grad = conicSampleFit.algebraicGradientDir(p);
                            float dx = mDX(cv::Point(p.x, p.y));
                            float dy = mDY(cv::Point(p.x, p.y));

                            float dotProd = dx * grad.x + dy * grad.y;

                            gradientCorrect &= dotProd > 0;
                        }
                        if (!gradientCorrect)
                        {
                            out.earlyRejections++;
                            continue;
                        }
                    }

                    // Assume that the sample is the only inliers

                    cv::RotatedRect ellipseInlierFit = ellipseSampleFit;
                    ConicSection conicInlierFit = conicSampleFit;
                    std::vector<cv::Point2f> inliers, prevInliers;

                    // Iteratively find inliers, and re-fit the ellipse
                    for (int i = 0; i < params.InlierIterations; ++i)
                    {
                        // Get error scale for 1px out on the minor axis
                        cv::Point2f minorAxis(-std::sin(CV_PI / 180.0 * ellipseInlierFit.angle), std::cos(CV_PI / 180.0 * ellipseInlierFit.angle));
                        cv::Point2f minorAxisPlus1px = ellipseInlierFit.center + (ellipseInlierFit.size.height / 2 + 1) * minorAxis;
                        float errOf1px = conicInlierFit.distance(minorAxisPlus1px);
                        float errorScale = 1.0f / errOf1px;

                        // Find inliers
                        inliers.reserve(edgePoints.size());
                        const float MAX_ERR = 2;
                        BOOST_FOREACH (const cv::Point2f &p, edgePoints)
                        {
                            float err = errorScale * conicInlierFit.distance(p);

                            if (err * err < MAX_ERR * MAX_ERR)
                                inliers.push_back(p);
                        }

                        if (inliers.size() < n)
                        {
                            inliers.clear();
                            continue;
                        }

                        // Refit ellipse to inliers
                        ellipseInlierFit = fitEllipse(inliers);
                        conicInlierFit = ConicSection(ellipseInlierFit);

                        // Normalise ellipse to have width as the major axis.
                        if (ellipseInlierFit.size.height > ellipseInlierFit.size.width)
                        {
                            ellipseInlierFit.angle = std::fmod(ellipseInlierFit.angle + 90, 180);
                            std::swap(ellipseInlierFit.size.height, ellipseInlierFit.size.width);
                        }
                    }
                    if (inliers.empty())
                        continue;

                    // Discard useless ellipses again
                    s = ellipseInlierFit.size;
                    if (!ellipseInlierFit.center.inside(bb) || s.height > params.Radius_Max * 2 || s.width > params.Radius_Max * 2 || (s.height < params.Radius_Min * 2 && s.width < params.Radius_Min * 2) || s.height > 4 * s.width || s.width > 4 * s.height)
                    {
                        // Bad ellipse! Go to your room!
                        continue;
                    }

                    // Calculate ellipse goodness
                    double ellipseGoodness = 0;
                    if (params.ImageAwareSupport)
                    {
                        BOOST_FOREACH (cv::Point2f &p, inliers)
                        {
                            cv::Point2f grad = conicInlierFit.algebraicGradientDir(p);
                            float dx = mDX(p);
                            float dy = mDY(p);

                            double edgeStrength = dx * grad.x + dy * grad.y;

                            ellipseGoodness += edgeStrength;
                        }
                    }
                    else
                    {
                        ellipseGoodness = inliers.size();
                    }

                    if (ellipseGoodness > out.bestEllipseGoodness)
                    {
                        std::swap(out.bestEllipseGoodness, ellipseGoodness);
                        std::swap(out.bestInliers, inliers);
                        std::swap(out.bestEllipse, ellipseInlierFit);

                        // Early termination, if 90% of points match
                        if (params.EarlyTerminationPercentage > 0 && out.bestInliers.size() > params.EarlyTerminationPercentage * edgePoints.size() / 100)
                        {
                            earlyTermination = true;
                            break;
                        }
                    }
                }
                //std::cout << "Ransac end" << std::endl;
            }

            void join(EllipseRansac &other)
            {
                //std::cout << "Ransac join" << std::endl;
                if (other.out.bestEllipseGoodness > out.bestEllipseGoodness)
                {
                    std::swap(out.bestEllipseGoodness, other.out.bestEllipseGoodness);
                    std::swap(out.bestInliers, other.out.bestInliers);
                    std::swap(out.bestEllipse, other.out.bestEllipse);
                }
                out.earlyRejections += other.out.earlyRejections;
                earlyTermination |= other.earlyTermination;

                out.earlyTermination = earlyTermination;
            }
        };

        EllipseRansac ransac(params, edgePoints, n, bbPupil, mPupilSobelX, mPupilSobelY);

        try
        {
            tbb::parallel_reduce(tbb::blocked_range<size_t>(0, k, k / 8), ransac);
        }
        catch (std::exception &e)
        {
            const char *c = e.what();
            std::cerr << e.what() << std::endl;
        }

        inliers = ransac.out.bestInliers;

        //out.earlyRejections = ransac.out.earlyRejections;
        //out.earlyTermination = ransac.out.earlyTermination;

        elPupil = ransac.out.bestEllipse;
        elPupil.center.x += roiPupil.x;
        elPupil.center.y += roiPupil.y;

        //        ConicSection conicBestFit(ellipseBestFit);
        //        BOOST_FOREACH(const cv::Point2f& p, edgePoints)
        //        {
        //            cv::Point2f grad = conicBestFit.algebraicGradientDir(p);
        //            float dx = mPupilSobelX(p);
        //            float dy = mPupilSobelY(p);
        //
        //            out.edgePoints.push_back(EdgePoint(p, dx*grad.x + dy*grad.y));
        //        }
    }

    if (inliers.size() == 0)
    {
        cv::RotatedRect ellipse;
        ellipse.center.x = 0;
        ellipse.center.y = 0;
        ellipse.angle = 0.0;
        ellipse.size.height = 0.0;
        ellipse.size.width = 0.0;

        inlierPts = inliers;

        pupil = ellipse;
        return;
    }

    //cv::Point2f pPupil = elPupil.center;
    //out.pPupil = pPupil;
    //out.elPupil = elPupil;

    inlierPts = inliers;
    pupil = elPupil;

    //return outPupil;
}

void Swirski2D::run(const cv::Mat &frame, const cv::Rect &roi, Pupil &pupil, const float &minPupilDiameterPx, const float &maxPupilDiameterPx)
{

    // TODO we could use minPupilDiameterPx for setting minRadius etc., but how reset?

    if (roi.area() < 10)
    {
        std::cout << "Bad ROI: falling back to regular detection.";
        PupilDetectionMethod::run(frame, pupil);
        return;
    }

    (void)minPupilDiameterPx;
    (void)maxPupilDiameterPx;

    pupil = run(frame(roi));
}
