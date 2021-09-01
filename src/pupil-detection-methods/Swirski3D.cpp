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

#include "Swirski3D.h"
#include "PupilDetectionMethod.h"

Pupil Swirski3D::run(const cv::Mat &frame)
{

    // first detect pupil using Swirski2D
    std::vector<cv::Point2f> inlier_pts;
    Pupil rr_pf;

    if (pupilDetector->hasInliers())
    {
        pupilDetector->run(frame, rr_pf, inlier_pts);
    }
    else
    {
        pupilDetector->run(frame, rr_pf);
        std::vector<cv::Point> inlierInt;
        inlierInt = PupilDetectionMethod::ellipse2Points(rr_pf, 4);
        inlier_pts = std::vector<cv::Point2f>(inlierInt.begin(), inlierInt.end());
    }

    bool pupil_found = rr_pf.valid(-2.0); // -2.0 because we ignore confidence for this check

    std::cout << "Pupil found: " << pupil_found << " " << inlier_pts.size() << std::endl;

    singleeyefitter::Ellipse2D<double> el = singleeyefitter::toEllipse<double>(toImgCoordInv(rr_pf, frame, 1.0));

    // found pupil is then feed to model, for generation

    // 3D eye pose estimation
    bool is_reliable = false;
    bool is_added = false;
    const bool force_add = false;

    const double kReliabilityThreshold = 0.8; // 0.96;
    double ellipse_reliability = 0.0;         /// Reliability of a detected 2D ellipse based on 3D eye model

    if (pupil_found)
    {
        if (is_model_built())
        {
            ellipse_reliability = compute_reliability(frame, el, inlier_pts);
            is_reliable = (ellipse_reliability > kReliabilityThreshold);
            std::cout << "Reliability: " << is_reliable << " " << ellipse_reliability << std::endl;
        }
        else
        {
            std::cout << "Sampling observation for building." << std::endl;
            add_observation(frame, el, inlier_pts, force_add);
        }
    }

    return rr_pf;
}

void Swirski3D::run(const cv::Mat &frame, const cv::Rect &roi, Pupil &pupil, const float &minPupilDiameterPx, const float &maxPupilDiameterPx)
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

bool Swirski3D::add_observation(const cv::Mat &image, sef::Ellipse2D<double> &pupil, std::vector<cv::Point2f> &pupil_inliers, bool force)
{

    if (!spaceBinSearcher.is_initialized())
    {
        spaceBinSearcher.initialize(image.cols, image.rows);
    }

    bool is_added = false;

    if (force || (!is_model_built_ && fitter_count < fitter_max_count))
    {
        cv::Vec2i pt;
        float dist;

        // Check if we already added a 2D ellipse close to the current 2D ellipse given
        if (force || spaceBinSearcher.search(
                         (int)(pupil.centre.x() + image.cols / 2),
                         (int)(pupil.centre.y() + image.rows / 2), pt, dist))
        {

            modelFitter.add_observation(image, pupil, pupil_inliers);
            fitter_count++;

            std::cout << "Observation added to model." << std::endl;

            if (fitter_count == fitter_max_count)
            {
                modelFitter.unproject_observations();
                modelFitter.initialise_model();
                is_model_built_ = true;
                std::cout << "Model finished building." << std::endl;
            }
            is_added = true;
        }
    }

    return is_added;
}

double Swirski3D::compute_reliability(const cv::Mat &img, sef::Ellipse2D<double> &el, std::vector<cv::Point2f> &inlier_pts)
{
    double realiabiliy = 0.0;

    if (modelFitter.eye)
    {

        // Unproject the current 2D ellipse observation to a 3D disk
        sef::EyeModelFitter::Circle curr_circle = unproject(img, el, inlier_pts);

        if (curr_circle && !isnan(curr_circle.normal(0, 0)))
        {
            const double displayscale = 1.0;

            sef::Ellipse2D<double> pupil_el(sef::project(curr_circle, focal_length));
            realiabiliy = el.similarity(pupil_el);

            //// 3D eyeball
            //cv::RotatedRect rr_eye = eye_tracker::toImgCoord(sef::toRotatedRect(sef::project(simple_fitter_.eye, focal_length_)), img, displayscale);

            //// 3D pupil
            //singleeyefitter::Ellipse2D<double> pupil_el(sef::project(curr_circle, focal_length_));
            //cv::RotatedRect rr_pupil = eye_tracker::toImgCoord(singleeyefitter::toRotatedRect(pupil_el), img, displayscale);

            //// 3D gaze vector
            //singleeyefitter::EyeModelFitter::Circle c_end = curr_circle;
            //c_end.centre = curr_circle.centre + (10.0)*curr_circle.normal; // Unit: mm
            //singleeyefitter::Ellipse2D<double> e_end(sef::project(c_end, focal_length_));
            //cv::RotatedRect rr_end = eye_tracker::toImgCoord(singleeyefitter::toRotatedRect(e_end), img, displayscale);
        }
    }

    return realiabiliy;
}

sef::EyeModelFitter::Circle Swirski3D::unproject(const cv::Mat &img, sef::Ellipse2D<double> &el, std::vector<cv::Point2f> &inlier_pts)
{

    if (modelFitter.eye)
    {
        // Unproject the current 2D ellipse observations
        sef::EyeModelFitter::Observation curr_obs(img, el, inlier_pts);
        sef::EyeModelFitter::Pupil curr_pupil(curr_obs);

        modelFitter.unproject_single_observation(curr_pupil, modelFitter.eye.radius);
        singleeyefitter::EyeModelFitter::Circle curr_circle = modelFitter.initialise_single_observation(curr_pupil);

        return curr_circle;
    }
    return singleeyefitter::EyeModelFitter::Circle::Null;
}

cv::Point2f Swirski3D::toImgCoord(const cv::Point2f &point, const cv::Mat &m, double scale, int shift)
{
    return cv::Point2f(static_cast<float>((m.cols / 2 + scale * point.x) * (1 << shift)),
                       static_cast<float>((m.rows / 2 + scale * point.y) * (1 << shift)));
}

cv::Point Swirski3D::toImgCoord(const cv::Point &point, const cv::Mat &m, double scale, int shift)
{
    return cv::Point(static_cast<int>((m.cols / 2 + scale * point.x) * (1 << shift)),
                     static_cast<int>((m.rows / 2 + scale * point.y) * (1 << shift)));
}

cv::RotatedRect Swirski3D::toImgCoord(const cv::RotatedRect &rect, const cv::Mat &m, float scale)
{
    return cv::RotatedRect(toImgCoord(rect.center, m, scale),
                           cv::Size2f(scale * rect.size.width,
                                      scale * rect.size.height),
                           rect.angle);
}

cv::Point2f Swirski3D::toImgCoordInv(const cv::Point2f &point, const cv::Mat &m, double scale, int shift)
{
    return cv::Point2f(
        static_cast<float>((point.x / (1 << shift) - m.cols / 2) / scale),
        static_cast<float>((point.y / (1 << shift) - m.rows / 2) / scale));
}

cv::Point Swirski3D::toImgCoordInv(const cv::Point &point, const cv::Mat &m, double scale, int shift)
{
    return cv::Point(
        static_cast<int>((point.x / (1 << shift) - m.cols / 2) / scale),
        static_cast<int>((point.y / (1 << shift) - m.rows / 2) / scale));
}

cv::RotatedRect Swirski3D::toImgCoordInv(const cv::RotatedRect &rect, const cv::Mat &m, float scale)
{
    return cv::RotatedRect(toImgCoordInv(rect.center, m, scale),
                           cv::Size2f(rect.size.width / scale,
                                      rect.size.height / scale),
                           rect.angle);
}

SpaceBinSearcher::SpaceBinSearcher() : kSearchGridSize_(16)
{
}

SpaceBinSearcher::SpaceBinSearcher(int w, int h) : kSearchGridSize_(16)
{
    initialize(w, h);
}

void SpaceBinSearcher::initialize(int w, int h)
{

    if (is_initialized_ == true)
    {
        std::cout << "SpaceBinSearcher::initialize: search tree is already initialized" << std::endl;
        return;
    }

    if (w < 0 || h < 0)
    {
        std::cout << "SpaceBinSearcher: Map size must be positive" << std::endl;
        throw;
    }
    const int w_num = w / kSearchGridSize_;
    const int h_num = h / kSearchGridSize_;

    // Create matrices
    ClusterMembers_.create(cv::Size(2, kN_), CV_32S); // The set A
    sample_num_ = (w_num + 1) * (h_num + 1);
    taken_flags_.resize(sample_num_);
    std::fill(taken_flags_.begin(), taken_flags_.end(), false);

    ClusterCenters_.create(cv::Size(2, sample_num_), CV_32S); // The set B
    int idx = 0;
    for (int r = 0; r < h; r += kSearchGridSize_)
    {
        for (int c = 0; c < w; c += kSearchGridSize_)
        {
            ClusterCenters_.at<int>(idx, 0) = c;
            ClusterCenters_.at<int>(idx, 1) = r;
            idx++;
        }
    }

    kdtrees = new cv::flann::GenericIndex<cv::flann::L2<int>>(ClusterCenters_, cvflann::KDTreeIndexParams(4)); // a 4 k-d tree
    is_initialized_ = true;
}

SpaceBinSearcher::~SpaceBinSearcher()
{
    if (is_initialized_)
    {
        delete kdtrees;
    }
}

void SpaceBinSearcher::render(cv::Mat &img)
{

    if (is_initialized_ == false)
    {
        std::cout << "SpaceBinSearcher::render: search tree is not initialized" << std::endl;
        return;
    }
    if (img.empty())
    {
        std::cout << "SpaceBinSearcher::render: input image is empty" << std::endl;
        return;
    }
    cv::Rect bb(cv::Point(), img.size());
    for (int idx = 0; idx < sample_num_; idx++)
    {
        cv::Vec2i center(ClusterCenters_.at<int>(idx, 0),
                         ClusterCenters_.at<int>(idx, 1));
        const int radius = 1;
        if (bb.contains(center))
        {
            if (taken_flags_[idx])
            {
                img.at<cv::Vec3b>(center[1], center[0]) = cv::Vec3b(0, 0, 255); // sample taken at least once
            }
            else
            {
                img.at<cv::Vec3b>(center[1], center[0]) = cv::Vec3b(0, 255, 0); // newly taken
            }
        }
    }
}
void SpaceBinSearcher::reset_indices()
{
    std::fill(taken_flags_.begin(), taken_flags_.end(), false);
}

bool SpaceBinSearcher::search(int x, int y, cv::Vec2i &pt, float &dist)
{
    if (!is_initialized_)
    {
        std::cout << "SpaceBinSearcher::search: search tree is not initialized" << std::endl;
        throw;
    }

    ClusterMembers_.at<int>(0, 0) = x;
    ClusterMembers_.at<int>(0, 1) = y;

    cv::Mat matches;   //This mat will contain the index of nearest neighbour as returned by Kd-tree
    cv::Mat distances; //In this mat Kd-Tree return the distances for each nearest neighbour
    matches.create(cv::Size(1, kN_), CV_32SC1);
    distances.create(cv::Size(1, kN_), CV_32FC1);

    // Search KdTree
    kdtrees->knnSearch(ClusterMembers_, matches, distances, 1, cvflann::SearchParams(8));
    int NN_index;
    for (int i = 0; i < kN_; i++)
    {
        NN_index = matches.at<int>(i, 0);
        dist = distances.at<float>(i, 0);
        pt = ClusterCenters_.row(NN_index);
        if (taken_flags_[NN_index])
        {
            return false; // sample is taken already
        }
        else
        {
            taken_flags_[NN_index] = true;
            return true; // newly searched point
        }
    }
}