#ifndef PUPILALGOSIMPLE_PUPIL_H
#define PUPILALGOSIMPLE_PUPIL_H

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

 Modified 01.01.2010: Moritz Lode

*/

#include <opencv2/core/types.hpp>

#define NO_CONFIDENCE -1.0


class Pupil : public cv::RotatedRect {

public:

    Pupil(const RotatedRect &outline, const float &confidence) :
            RotatedRect(outline), confidence(confidence), outline_confidence(NO_CONFIDENCE), eyelid(0), physicalDiameter(-1.0), undistortedDiameter(-1.0), algorithmName("") {
    }

    Pupil(const RotatedRect &outline, const float &confidence, const float &outline_confidence, const float &eyelid, const float &physicalDiameter, const float &undistortedDiameter) :
            RotatedRect(outline), confidence(confidence), outline_confidence(outline_confidence), eyelid(eyelid), physicalDiameter(physicalDiameter), undistortedDiameter(undistortedDiameter), algorithmName("") {
    }

    Pupil(const Pupil &other) :
            RotatedRect(other), confidence(other.confidence), outline_confidence(other.outline_confidence), eyelid(other.eyelid), physicalDiameter(other.physicalDiameter), undistortedDiameter(other.undistortedDiameter), algorithmName(other.algorithmName) {
    }

    Pupil(const RotatedRect &outline) :
            RotatedRect(outline), confidence(NO_CONFIDENCE), outline_confidence(NO_CONFIDENCE), eyelid(0), physicalDiameter(-1.0), undistortedDiameter(-1.0), algorithmName("") {
    }

    Pupil() {
        clear();
    }

    ~Pupil() = default;

    float confidence;

    float outline_confidence;

    float eyelid;

    float physicalDiameter;
    float undistortedDiameter;

    std::string algorithmName;

    void clear() {
        angle = -1.0;
        center = { -1.0, -1.0 };
        size = { -1.0, -1.0 };
        confidence = NO_CONFIDENCE;
        outline_confidence = NO_CONFIDENCE;
        eyelid=0;
        physicalDiameter=-1.0;
        undistortedDiameter=-1.0;
        algorithmName="";
    }

    void resize(const float &xf, const float &yf) {
        center.x *= xf;
        center.y *= yf;
        size.width *= xf;
        size.height *= yf;
    }

    void resize(const float &f) {
        center *= f;
        size *= f;
    }

    void shift( cv::Point2f p ) {
        center += p;
    }

    std::vector<cv::Point2f> rectPoints() {
        cv::Point2f pointsArr[4];
        points(pointsArr);

        std::vector<cv::Point2f> v(std::begin(pointsArr), std::end(pointsArr));

        return v;
    }

    bool valid(const double &confidenceThreshold=NO_CONFIDENCE) const {
        return center.x > 0 &&
               center.y > 0 &&
               size.width > 0 &&
               size.height > 0 &&
               (confidence > confidenceThreshold || outline_confidence > confidenceThreshold);
    }

    bool hasOutline() const {
        return size.width > 0 && size.height > 0;
    }

    int width() const {
        return (int)size.width;
    }

    int height() const {
        return (int)size.height;
    }

    int majorAxis() const {
        return std::max<int>(size.width, size.height);
    }

    int minorAxis() const {
        return std::min<int>(size.width, size.height);
    }

    int diameter() const {
        return majorAxis();
    }

    float circumference() const {
        if(size.width==-1 || size.height==-1) return -1.0;

        float a = 0.5*majorAxis();
        float b = 0.5*minorAxis();
        return CV_PI * abs( 3*(a+b) - sqrt( 10*a*b + 3*( pow(a,2) + pow(b,2) ) ) );
    }
};


#endif //PUPILALGOSIMPLE_PUPIL_H
