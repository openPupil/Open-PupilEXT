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

#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include "Starburst.h"

#define IMG_SIZE 640 //400


void locate_corneal_reflection(cv::Mat *image, int sx, int sy, int window_size, int biggest_crar, int &crx, int &cry, int &crar) {

    if (window_size%2 == 0) {
        printf("Error! window_size should be odd!\n");
    }

    int r = (window_size-1)/2;
    int startx = MAX(sx-r, 0);
    startx = MIN(startx, image->size().width-1);
    int endx = MIN(sx+r, image->size().width-1);
    int starty = MAX(sy-r, 0);
    starty = MIN(starty, image->size().height-1);
    int endy = MIN(sy+r, image->size().height-1);

    cv::Mat roiImage = (*image)(cv::Rect(startx, starty, endx-startx+1, endy-starty+1));
    cv::Mat roiThresholdImage;
    roiImage.copyTo(roiThresholdImage);

    double min_value, max_value;
    cv::Point min_loc, max_loc; //location
    cv::minMaxLoc(*image, &min_value, &max_value, &min_loc, &max_loc);

    int threshold;
    std::vector<std::vector<cv::Point> > contours;
    auto *scores = (double*)malloc(sizeof(double)*((int)max_value+1));
    memset(scores, 0, sizeof(double)*((int)max_value+1));
    int area, max_area, sum_area;
    for (threshold = (int)max_value; threshold >= 1; threshold--) {
        cv::threshold(roiImage, roiThresholdImage, threshold, 1, cv::THRESH_BINARY);
        cv::findContours(roiThresholdImage, contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);
        max_area = 0;
        sum_area = 0;
        std::vector<cv::Point> max_contour;
        for (auto & contour : contours) {
            area = contour.size() + (int)(fabs(cv::contourArea(contour)));
            sum_area += area;
            if (area > max_area) {
                max_area = area;
                max_contour = contour;
            }
        }
        if (sum_area-max_area > 0) {
            scores[threshold-1] = max_area / (sum_area-max_area);
        }
        else
            continue;

        if (scores[threshold-1] - scores[threshold] < 0) {
            //found the corneal reflection
            crar = (int)sqrt(max_area / CV_PI);
            int sum_x = 0;
            int sum_y = 0;
            for (auto & it : max_contour) {
                sum_x += it.x;
                sum_y += it.y;
            }
            crx = sum_x/max_contour.size();
            cry = sum_y/max_contour.size();
            break;
        }
    }
    free(scores);

    if (crar > biggest_crar) {
        //printf("(corneal) size too large! crx:%d, cry:%d, crar:%d (should be less than %d)\n", crx, cry, crar, biggest_crar);
        cry = crx = -1;
        crar = -1;
    }

    if (crx != -1 && cry != -1) {
        //printf("(corneal) startx:%d, starty:%d, crx:%d, cry:%d, crar:%d\n", startx, starty, crx, cry, crar);
        crx += startx;
        cry += starty;
    }
}

int fit_circle_radius_to_corneal_reflection(cv::Mat *image, int crx, int cry, int crar, int biggest_crar, double *sin_array, double *cos_array, int array_len) {
    if (crx == -1 || cry == -1 || crar == -1)
        return -1;

    double *ratio = (double*)malloc((biggest_crar-crar+1)*sizeof(double));
    int i, r, r_delta=1;
    int x, y, x2, y2;
    double sum, sum2;
    for (r = crar; r <= biggest_crar; r++) {
        sum = 0;
        sum2 = 0;
        for (i = 0; i < array_len; i++) {
            x = (int)(crx + (r+r_delta)*cos_array[i]);
            y = (int)(cry + (r+r_delta)*sin_array[i]);
            x2 = (int)(crx + (r-r_delta)*cos_array[i]);
            y2 = (int)(cry + (r+r_delta)*sin_array[i]);
            if ((x >= 0 && y >=0 && x < image->size().width && y < image->size().height) &&
                (x2 >= 0 && y2 >=0 && x2 < image->size().width && y2 < image->size().height)) {
                sum += *(image->data + y*image->size().width + x);
                sum2 += *(image->data + y2*image->size().width + x2);
            }
        }
        ratio[r-crar] = sum / sum2;
        if (r - crar >= 2) {
            if (ratio[r-crar-2] < ratio[r-crar-1] && ratio[r-crar] < ratio[r-crar-1]) {
                free(ratio);
                return r-1;
            }
        }
    }

    free(ratio);
    //printf("ATTN! fit_circle_radius_to_corneal_reflection() do not change the radius\n");
    return crar;
}

void interpolate_corneal_reflection(cv::Mat *image, int crx, int cry, int crr, double *sin_array, double *cos_array,
                                    int array_len) {
    if (crx == -1 || cry == -1 || crr == -1)
        return;

    if (crx-crr < 0 || crx+crr >= image->size().width || cry-crr < 0 || cry+crr >= image->size().height) {
        //printf("Error! Corneal reflection is too near the image border\n");
        return;
    }

    int i, r, r2,  x, y;
    UINT8 *perimeter_pixel = (UINT8*)malloc(array_len*sizeof(int));
    int sum=0;
    double avg;
    for (i = 0; i < array_len; i++) {
        x = (int)(crx + crr*cos_array[i]);
        y = (int)(cry + crr*sin_array[i]);
        perimeter_pixel[i] = (UINT8)(*(image->data + y*image->size().width + x));
        sum += perimeter_pixel[i];
    }
    avg = sum*1.0/array_len;

    for (r = 1; r < crr; r++) {
        r2 = crr-r;
        for (i = 0; i < array_len; i++) {
            x = (int)(crx + r*cos_array[i]);
            y = (int)(cry + r*sin_array[i]);
            *(image->data + y*image->size().width + x) = (UINT8)((r2*1.0/crr)*avg + (r*1.0/crr)*perimeter_pixel[i]);
        }
    }
    free(perimeter_pixel);
}

void remove_corneal_reflection(cv::Mat *image, int sx, int sy, int window_size, int biggest_crr, int &crx, int& cry, int& crr) {
    int crar = -1;	//corneal reflection approximate radius
    crx = cry = crar = -1;

    float angle_delta = 1*CV_PI/180;
    int angle_num = (int)(2*CV_PI/angle_delta);

    double *angle_array = (double*)malloc(angle_num*sizeof(double));
    double *sin_array = (double*)malloc(angle_num*sizeof(double));
    double *cos_array = (double*)malloc(angle_num*sizeof(double));
    for (int i = 0; i < angle_num; i++) {
        angle_array[i] = i*angle_delta;
        sin_array[i] = sin(angle_array[i]);
        cos_array[i] = cos(angle_array[i]);
    }

    locate_corneal_reflection(image, sx, sy, window_size, (int)(biggest_crr/2.5), crx, cry, crar);
    crr = fit_circle_radius_to_corneal_reflection(image, crx, cry, crar, (int)(biggest_crr/2.5),  sin_array, cos_array, angle_num);
    crr = (int)(2.5*crr);
    interpolate_corneal_reflection(image, crx, cry, crr, sin_array, cos_array, angle_num);

    free(angle_array);
    free(sin_array);
    free(cos_array);
}


int RansacEllipse::starburst_pupil_contour_detection(UINT8* pupil_image, const cv::Point2d &startPoint, int width, int height, int edge_thresh, int N, int minimum_cadidate_features) {
    // ML: added int return to signal that detection success

    int dis = 7;
    double angle_spread = 100*CV_PI/180;
    int loop_count = 0;
    double angle_step = 2*CV_PI/N;
    double new_angle_step;
    cv::Point2d *edge, edge_mean;
    double angle_normal;
    double cx = startPoint.x;
    double cy = startPoint.y;
    int first_ep_num;

    while (edge_thresh > 5 && loop_count <= 10) {
        this->edge_intensity_diff.clear();
        this->destroy_edge_point();
        while (this->edge_point.size() < minimum_cadidate_features && edge_thresh > 5) {
            this->edge_intensity_diff.clear();
            this->destroy_edge_point();
            this->locate_edge_points(pupil_image, width, height, cx, cy, dis, angle_step, 0, 2*CV_PI, edge_thresh);
            if (this->edge_point.size() < minimum_cadidate_features) {
                edge_thresh -= 1;
            }
        }
        if (edge_thresh <= 5) {
            break;
        }

        first_ep_num = this->edge_point.size();
        for (int i = 0; i < first_ep_num; i++) {
            edge = this->edge_point.at(i);
            angle_normal = atan2(cy-edge->y, cx-edge->x);
            new_angle_step = angle_step*(edge_thresh*1.0/edge_intensity_diff.at(i));
            this->locate_edge_points(pupil_image, width, height, edge->x, edge->y, dis, new_angle_step, angle_normal, angle_spread, edge_thresh);
        }

        loop_count += 1;
        edge_mean = this->get_edge_mean();
        if (fabs(edge_mean.x-cx) + fabs(edge_mean.y-cy) < 10)
            break;

        cx = edge_mean.x;
        cy = edge_mean.y;
    }

    if (loop_count > 10) {
        this->destroy_edge_point();
        //printf("Error! edge points did not converge in %d iterations!\n", loop_count);
        return 1;
    }

    if (edge_thresh <= 5) {
        this->destroy_edge_point();
        //printf("Error! Adaptive threshold is too low!\n");
        return 1;
    }

    return 0;
}

void RansacEllipse::locate_edge_points(const UINT8* image, int width, int height, double cx, double cy, int dis, double angle_step, double angle_normal, double angle_spread, int edge_thresh)
{
    double angle;
    cv::Point2d p, *edge;
    double dis_cos, dis_sin;
    int pixel_value1, pixel_value2;

    for (angle = angle_normal-angle_spread/2+0.0001; angle < angle_normal+angle_spread/2; angle += angle_step) {
        dis_cos = dis * cos(angle);
        dis_sin = dis * sin(angle);
        p.x = cx + dis_cos;
        p.y = cy + dis_sin;

        if (p.x < 0 || p.x >= width || p.y < 0 || p.y >= height)
            continue;

        pixel_value1 = image[(int)(p.y)*width+(int)(p.x)];
        while (1) {
            p.x += dis_cos;
            p.y += dis_sin;
            if (p.x < 0 || p.x >= width || p.y < 0 || p.y >= height)
                break;

            pixel_value2 = image[(int)(p.y)*width+(int)(p.x)];
            //printf("edge diff: %d\n", pixel_value2 - pixel_value1);
            if ((pixel_value2 - pixel_value1) > edge_thresh) {
                edge = new cv::Point2d();
                edge->x = p.x - dis_cos/2;
                edge->y = p.y - dis_sin/2;
                this->edge_point.push_back(edge);
                this->edge_intensity_diff.push_back(pixel_value2 - pixel_value1);
                break;
            }
            pixel_value1 = pixel_value2;
        }
    }
}

cv::Point2d RansacEllipse::get_edge_mean() {

    cv::Point2d *edge;
    int i;
    double sumx=0, sumy=0;
    cv::Point2d edge_mean;

    for (i = 0; i < this->edge_point.size(); i++) {
        edge = this->edge_point.at(i);
        sumx += edge->x;
        sumy += edge->y;
    }
    if (this->edge_point.size() != 0) {
        edge_mean.x = sumx / this->edge_point.size();
        edge_mean.y = sumy / this->edge_point.size();
    } else {
        edge_mean.x = -1;
        edge_mean.y = -1;
    }
    return edge_mean;
}

void RansacEllipse::destroy_edge_point() {
    std::vector<cv::Point2d*>::iterator iter;

    if (this->edge_point.size() != 0) {
        for (iter = this->edge_point.begin(); iter != this->edge_point.end( ) ; iter++ ) {
            free(*iter);
        }
        this->edge_point.clear();
    }
}

void RansacEllipse::get_random_num(int n, int max_num, int* rand_num) {
    int rand_index = 0;
    int r;
    int i;
    bool is_new = 1;

    if (max_num == n-1) {
        for (i = 0; i < n; i++) {
            rand_num[i] = i;
        }
        return;
    }

    while (rand_index < n) {
        is_new = 1;
        r = (int)((rand()*1.0/RAND_MAX) * max_num);
        for (i = 0; i < rand_index; i++) {
            if (r == rand_num[i]) {
                is_new = 0;
                break;
            }
        }
        if (is_new) {
            rand_num[rand_index] = r;
            rand_index++;
        }
    }
}

bool RansacEllipse::solve_ellipse(double* conic_param, double* ellipse_param) {
    double a = conic_param[0];
    double b = conic_param[1];
    double c = conic_param[2];
    double d = conic_param[3];
    double e = conic_param[4];
    double f = conic_param[5];
    //get ellipse orientation
    double theta = atan2(b, a-c)/2;

    //get scaled major/minor axes
    double ct = cos(theta);
    double st = sin(theta);
    double ap = a*ct*ct + b*ct*st + c*st*st;
    double cp = a*st*st - b*ct*st + c*ct*ct;

    //get translations
    double cx = (2*c*d - b*e) / (b*b - 4*a*c);
    double cy = (2*a*e - b*d) / (b*b - 4*a*c);

    //get scale factor
    double val = a*cx*cx + b*cx*cy + c*cy*cy;
    double scale_inv = val - f;

    if (scale_inv/ap <= 0 || scale_inv/cp <= 0) {
        //printf("Error! ellipse parameters are imaginary a=sqrt(%lf), b=sqrt(%lf)\n", scale_inv/ap, scale_inv/cp);
        memset(ellipse_param, 0, sizeof(double)*5);
        return 0;
    }

    ellipse_param[0] = sqrt(scale_inv / ap);
    ellipse_param[1] = sqrt(scale_inv / cp);
    ellipse_param[2] = cx;
    ellipse_param[3] = cy;
    ellipse_param[4] = theta;
    return 1;
}

cv::Point2d* RansacEllipse::normalize_edge_point(double &dis_scale, cv::Point2d &nor_center, int ep_num) {
    double sumx = 0, sumy = 0;
    double sumdis = 0;
    cv::Point2d *edge;
    int i;

    for (i = 0; i < ep_num; i++) {
        edge = this->edge_point.at(i);
        sumx += edge->x;
        sumy += edge->y;
        sumdis += sqrt((double)(edge->x*edge->x + edge->y*edge->y));
    }

    dis_scale = sqrt((double)2)*ep_num/sumdis;
    nor_center.x = sumx*1.0/ep_num;
    nor_center.y = sumy*1.0/ep_num;
    cv::Point2d *edge_point_nor = (cv::Point2d*)malloc(sizeof(cv::Point2d)*ep_num);

    for (i = 0; i < ep_num; i++) {
        edge = this->edge_point.at(i);
        edge_point_nor[i].x = (edge->x - nor_center.x)*dis_scale;
        edge_point_nor[i].y = (edge->y - nor_center.y)*dis_scale;
    }
    return edge_point_nor;
}

void RansacEllipse::denormalize_ellipse_param(double* par, double* normailized_par, double dis_scale, cv::Point2d nor_center) {
    par[0] = normailized_par[0] / dis_scale;	//major or minor axis
    par[1] = normailized_par[1] / dis_scale;
    par[2] = normailized_par[2] / dis_scale + nor_center.x;	//ellipse center
    par[3] = normailized_par[3] / dis_scale + nor_center.y;
}

void RansacEllipse::svd(int m, int n, double **a, double **p, double *d, double **q) {
    int             flag, i, its, j, jj, k, l, nm, nm1 = n - 1, mm1 = m - 1;
    double          c, f, h, s, x, y, z;
    double          anorm = 0, g = 0, scale = 0;
    //double         *r = tvector_alloc(0, n, double);
    double			*r = (double*)malloc(sizeof(double)*n);

    for (i = 0; i < m; i++)
        for (j = 0; j < n; j++)
            p[i][j] = a[i][j];
    //for (i = m; i < n; i++)
    //                p[i][j] = 0;

    /* Householder reduction to bidigonal form */
    for (i = 0; i < n; i++)
    {
        l = i + 1;
        r[i] = scale * g;
        g = s = scale = 0.0;
        if (i < m)
        {
            for (k = i; k < m; k++)
                scale += fabs(p[k][i]);
            if (scale)
            {
                for (k = i; k < m; k++)
                {
                    p[k][i] /= scale;
                    s += p[k][i] * p[k][i];
                }
                f = p[i][i];
                g = -SIGN(sqrt(s), f);
                h = f * g - s;
                p[i][i] = f - g;
                if (i != nm1)
                {
                    for (j = l; j < n; j++)
                    {
                        for (s = 0.0, k = i; k < m; k++)
                            s += p[k][i] * p[k][j];
                        f = s / h;
                        for (k = i; k < m; k++)
                            p[k][j] += f * p[k][i];
                    }
                }
                for (k = i; k < m; k++)
                    p[k][i] *= scale;
            }
        }
        d[i] = scale * g;
        g = s = scale = 0.0;
        if (i < m && i != nm1)
        {
            for (k = l; k < n; k++)
                scale += fabs(p[i][k]);
            if (scale)
            {
                for (k = l; k < n; k++)
                {
                    p[i][k] /= scale;
                    s += p[i][k] * p[i][k];
                }
                f = p[i][l];
                g = -SIGN(sqrt(s), f);
                h = f * g - s;
                p[i][l] = f - g;
                for (k = l; k < n; k++)
                    r[k] = p[i][k] / h;
                if (i != mm1)
                {
                    for (j = l; j < m; j++)
                    {
                        for (s = 0.0, k = l; k < n; k++)
                            s += p[j][k] * p[i][k];
                        for (k = l; k < n; k++)
                            p[j][k] += s * r[k];
                    }
                }
                for (k = l; k < n; k++)
                    p[i][k] *= scale;
            }
        }
        anorm = MAX(anorm, fabs(d[i]) + fabs(r[i]));
    }

    /* Accumulation of right-hand transformations */
    for (i = n - 1; i >= 0; i--)
    {
        if (i < nm1)
        {
            if (g)
            {
                for (j = l; j < n; j++)
                    q[j][i] = (p[i][j] / p[i][l]) / g;
                for (j = l; j < n; j++)
                {
                    for (s = 0.0, k = l; k < n; k++)
                        s += p[i][k] * q[k][j];
                    for (k = l; k < n; k++)
                        q[k][j] += s * q[k][i];
                }
            }
            for (j = l; j < n; j++)
                q[i][j] = q[j][i] = 0.0;
        }
        q[i][i] = 1.0;
        g = r[i];
        l = i;
    }
    /* Accumulation of left-hand transformations */
    for (i = n - 1; i >= 0; i--)
    {
        l = i + 1;
        g = d[i];
        if (i < nm1)
            for (j = l; j < n; j++)
                p[i][j] = 0.0;
        if (g)
        {
            g = 1.0 / g;
            if (i != nm1)
            {
                for (j = l; j < n; j++)
                {
                    for (s = 0.0, k = l; k < m; k++)
                        s += p[k][i] * p[k][j];
                    f = (s / p[i][i]) * g;
                    for (k = i; k < m; k++)
                        p[k][j] += f * p[k][i];
                }
            }
            for (j = i; j < m; j++)
                p[j][i] *= g;
        } else
            for (j = i; j < m; j++)
                p[j][i] = 0.0;
        ++p[i][i];
    }
    /* diagonalization of the bidigonal form */
    for (k = n - 1; k >= 0; k--)
    {                       /* loop over singlar values */
        for (its = 0; its < 30; its++)
        {               /* loop over allowed iterations */
            flag = 1;
            for (l = k; l >= 0; l--)
            {       /* test for splitting */
                nm = l - 1;     /* note that r[l] is always
                                                 * zero */
                if (fabs(r[l]) + anorm == anorm)
                {
                    flag = 0;
                    break;
                }
                if (fabs(d[nm]) + anorm == anorm)
                    break;
            }
            if (flag)
            {
                c = 0.0;        /* cancellation of r[l], if
                                                 * l>1 */
                s = 1.0;
                for (i = l; i <= k; i++)
                {
                    f = s * r[i];
                    if (fabs(f) + anorm != anorm)
                    {
                        g = d[i];
                        h = radius(f, g);
                        d[i] = h;
                        h = 1.0 / h;
                        c = g * h;
                        s = (-f * h);
                        for (j = 0; j < m; j++)
                        {
                            y = p[j][nm];
                            z = p[j][i];
                            p[j][nm] = y * c + z * s;
                            p[j][i] = z * c - y * s;
                        }
                    }
                }
            }
            z = d[k];
            if (l == k)
            {       /* convergence */
                if (z < 0.0)
                {
                    d[k] = -z;
                    for (j = 0; j < n; j++)
                        q[j][k] = (-q[j][k]);
                }
                break;
            }
            if (its == 30)
            {
                //error("svd: No convergence in 30 svd iterations", non_fatal);
                return;
            }
            x = d[l];       /* shift from bottom 2-by-2 minor */
            nm = k - 1;
            y = d[nm];
            g = r[nm];
            h = r[k];
            f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0 * h * y);
            g = radius(f, 1.0);
            /* next QR transformation */
            f = ((x - z) * (x + z) + h * ((y / (f + SIGN(g, f))) - h)) / x;
            c = s = 1.0;
            for (j = l; j <= nm; j++)
            {
                i = j + 1;
                g = r[i];
                y = d[i];
                h = s * g;
                g = c * g;
                z = radius(f, h);
                r[j] = z;
                c = f / z;
                s = h / z;
                f = x * c + g * s;
                g = g * c - x * s;
                h = y * s;
                y = y * c;
                for (jj = 0; jj < n; jj++)
                {
                    x = q[jj][j];
                    z = q[jj][i];
                    q[jj][j] = x * c + z * s;
                    q[jj][i] = z * c - x * s;
                }
                z = radius(f, h);
                d[j] = z;       /* rotation can be arbitrary
                                                 * id z=0 */
                if (z)
                {
                    z = 1.0 / z;
                    c = f * z;
                    s = h * z;
                }
                f = (c * g) + (s * y);
                x = (c * y) - (s * g);
                for (jj = 0; jj < m; jj++)
                {
                    y = p[jj][j];
                    z = p[jj][i];
                    p[jj][j] = y * c + z * s;
                    p[jj][i] = z * c - y * s;
                }
            }
            r[l] = 0.0;
            r[k] = f;
            d[k] = x;
        }
    }
    free(r);

    // dhli add: the original code does not sort the eigen value
    // should do that and change the eigen vector accordingly
}

int* RansacEllipse::pupil_fitting_inliers(UINT8* pupil_image, int width, int height,  int &return_max_inliers_num) {
    int i;
    int ep_num = this->edge_point.size();   //ep stands for edge point
    cv::Point2d nor_center;
    double dis_scale;

    const int ellipse_point_num = 5;	//number of point that needed to fit an ellipse
    if (ep_num < ellipse_point_num) {
        //printf("Error! %d points are not enough to fit ellipse\n", ep_num);
        memset(this->pupil_param, 0, sizeof(this->pupil_param));
        return_max_inliers_num = 0;
        return NULL;
    }

    //Normalization
    cv::Point2d *edge_point_nor = this->normalize_edge_point(dis_scale, nor_center, ep_num);

    //Ransac
    int *inliers_index = (int*)malloc(sizeof(int)*ep_num);
    int *max_inliers_index = (int*)malloc(sizeof(int)*ep_num);
    int ninliers = 0;
    int max_inliers = 0;
    int sample_num = 1000;	//number of sample
    int ransac_count = 0;
    double dis_threshold = sqrt(3.84)*dis_scale/10; // Works better with the /10
    double dis_error;

    memset(inliers_index, int(0), sizeof(int)*ep_num);
    memset(max_inliers_index, int(0), sizeof(int)*ep_num);
    int rand_index[ellipse_point_num];
    double A[ellipse_point_num + 1][6];
    int M = ellipse_point_num + 1, N = 6; //M is row; N is column
    for (i = 0; i < M; i++) {
        A[i][5] = 1;
    }
    for(i = 0; i < N; i++) {
        A[ellipse_point_num][i] = 0;
    }
    double **ppa = (double**)malloc(sizeof(double*)*M);
    double **ppu = (double**)malloc(sizeof(double*)*M);
    double **ppv = (double**)malloc(sizeof(double*)*N);
    for (i = 0; i < M; i++) {
        ppa[i] = A[i];
        ppu[i] = (double*)malloc(sizeof(double)*N);
    }
    for (i = 0; i < N; i++) {
        ppv[i] = (double*)malloc(sizeof(double)*N);
    }
    double pd[6];
    int min_d_index;
    double conic_par[6] = {0};
    double ellipse_par[5] = {0};
    double best_ellipse_par[5] = {0};
    double ratio;
    while (sample_num > ransac_count) {
        this->get_random_num(ellipse_point_num, (ep_num-1), rand_index);

        //svd decomposition to solve the ellipse parameter
        for (i = 0; i < ellipse_point_num; i++) {
            A[i][0] = edge_point_nor[rand_index[i]].x * edge_point_nor[rand_index[i]].x;
            A[i][1] = edge_point_nor[rand_index[i]].x * edge_point_nor[rand_index[i]].y;
            A[i][2] = edge_point_nor[rand_index[i]].y * edge_point_nor[rand_index[i]].y;
            A[i][3] = edge_point_nor[rand_index[i]].x;
            A[i][4] = edge_point_nor[rand_index[i]].y;
        }

        svd(M, N, ppa, ppu, pd, ppv);
        min_d_index = 0;
        for (i = 1; i < N; i++) {
            if (pd[i] < pd[min_d_index])
                min_d_index = i;
        }

        for (i = 0; i < N; i++)
            conic_par[i] = ppv[i][min_d_index];	//the column of v that corresponds to the smallest singular value,
        //which is the solution of the equations
        ninliers = 0;
        memset(inliers_index, 0, sizeof(int)*ep_num);
        for (i = 0; i < ep_num; i++) {
            dis_error = conic_par[0]*edge_point_nor[i].x*edge_point_nor[i].x +
                        conic_par[1]*edge_point_nor[i].x*edge_point_nor[i].y +
                        conic_par[2]*edge_point_nor[i].y*edge_point_nor[i].y +
                        conic_par[3]*edge_point_nor[i].x + conic_par[4]*edge_point_nor[i].y + conic_par[5];
            if (fabs(dis_error) < dis_threshold) {
                inliers_index[ninliers] = i;
                ninliers++;
            }
        }

        if (ninliers > max_inliers) {
            if (this->solve_ellipse(conic_par, ellipse_par)) {
                this->denormalize_ellipse_param(ellipse_par, ellipse_par, dis_scale, nor_center);
                ratio = ellipse_par[0] / ellipse_par[1];
                if (ellipse_par[2] > 0 && ellipse_par[2] <= width-1 && ellipse_par[3] > 0 && ellipse_par[3] <= height-1 &&
                    ratio > 0.5 && ratio < 2) {
                    memcpy(max_inliers_index, inliers_index, sizeof(int)*ep_num);
                    for (i = 0; i < 5; i++) {
                        best_ellipse_par[i] = ellipse_par[i];
                    }
                    max_inliers = ninliers;
                    sample_num = (int)(log((double)(1-0.99))/log(1.0-pow(ninliers*1.0/ep_num, 5)));
                }
            }
        }
        ransac_count++;
        if (ransac_count > 1500) {
            //printf("Error! ransac_count exceed! ransac break! sample_num=%d, ransac_count=%d\n", sample_num, ransac_count);
            break;
        }
    }
    //INFO("ransc end\n");
    if (best_ellipse_par[0] > 0 && best_ellipse_par[1] > 0) {
        for (i = 0; i < 5; i++) {
            this->pupil_param[i] = best_ellipse_par[i];
        }
    } else {
        memset(pupil_param, 0, sizeof(pupil_param));
        max_inliers = 0;
        free(max_inliers_index);
        max_inliers_index = NULL;
    }

    for (i = 0; i < M; i++) {
        free(ppu[i]);
    }
    for (i = 0; i < N; i++) {
        free(ppv[i]);
    }
    free(ppu);
    free(ppv);
    free(ppa);

    free(edge_point_nor);
    free(inliers_index);
    return_max_inliers_num = max_inliers;
    return max_inliers_index;
}


void Starburst::run(const cv::Mat &frame, Pupil &pupil) {

    pupil.clear();

//    cv::Mat downscaled = frame;
//    if (frame.rows > IMG_SIZE || frame.cols > IMG_SIZE) {
//        // return ellipse;
//        // Downscaling
//        float rw = IMG_SIZE / (float) frame.cols;
//        float rh = IMG_SIZE / (float) frame.rows;
//        float scalingRatio = std::min<float>( std::min<float>(rw, rh) , 1.0 );
//        cv::resize(frame, downscaled, cv::Size(), scalingRatio, scalingRatio, cv::INTER_LINEAR);
//    }

    cv::Mat eyeImg = frame.clone();

    if(imageSize != eyeImg.size()) {
        // ML: If we change the image size in-run i.e. ROI selection changed, we need to reset some fields that are image size depended
        imageSize = eyeImg.size();
        this->startPoint.x = eyeImg.size().width/2;
        this->startPoint.y = eyeImg.size().height/2;
    }

    int *inliers_index;
    cv::Size ellipse_axis;

    // ML: we dont have noise in our video, applying these actually worsens result dramatically
    //cv::GaussianBlur(eyeImg, eyeImg, cv::Size(5, 5), 0);
    //this->reduceLineNoise(eyeImg);

    // corneal reflection
    cv::Point corneal_reflection(0, 0); //coordinates of corneal reflection in tracker coordinate system
    int corneal_reflection_r = 0;       //the radius of corneal reflection
    remove_corneal_reflection(&eyeImg, (int) this->startPoint.x, (int) this->startPoint.y, this->crWindowSize,
                              (int) eyeImg.size().height/corneal_reflection_ratio_to_image_size, corneal_reflection.x, corneal_reflection.y, corneal_reflection_r);
    //std::cout<<"corneal reflection: "<<corneal_reflection.x<<" "<<corneal_reflection.y<<std::endl;

    //starburst pupil contour detection
    int detection_success = this->ransacEllipse.starburst_pupil_contour_detection((UINT8*)eyeImg.data, this->startPoint, eyeImg.cols, eyeImg.rows, edge_threshold, rays, min_feature_candidates);

    int inliers_num = 0;
    cv::Point pupilPoint(0,0); //coordinates of pupil in tracker coordinate system

    inliers_index = this->ransacEllipse.pupil_fitting_inliers((UINT8*)eyeImg.data, eyeImg.size().width, eyeImg.size().height, inliers_num);

    ellipse_axis.width = (int)2*this->ransacEllipse.pupil_param[0];
    ellipse_axis.height = (int)2*this->ransacEllipse.pupil_param[1];
    pupilPoint.x = (int)this->ransacEllipse.pupil_param[2];
    pupilPoint.y = (int)this->ransacEllipse.pupil_param[3];

    // Set pupil
    pupil = cv::RotatedRect(pupilPoint, ellipse_axis, (float)this->ransacEllipse.pupil_param[4]);

    //printf("ellipse a:%lf; b:%lf, cx:%lf, cy:%lf, theta:%lf; inliers_num:%d\n\n",
    //       this->ransacEllipse.pupil_param[0], this->ransacEllipse.pupil_param[1],
    //       this->ransacEllipse.pupil_param[2], this->ransacEllipse.pupil_param[3],
    //       this->ransacEllipse.pupil_param[4], inliers_num);

    free(inliers_index);

    if (ellipse_axis.width > 0 && ellipse_axis.height > 0) {
        this->startPoint.x = pupilPoint.x;
        this->startPoint.y = pupilPoint.y;

        this->lostFrameNum = 0;
    } else {
        this->lostFrameNum++;
    }

    if (this->lostFrameNum > 5) {
        this->startPoint.x = eyeImg.size().width/2;
        this->startPoint.y = eyeImg.size().height/2;
    }
}


//void Starburst::calculateAvgIntensityHori(cv::Mat &inImage) {
//    UINT8 *pixel = (UINT8*) inImage.data;
//    int sum;
//    int i, j;
//    this->recreateIntensityArrays(inImage);
//    for (j = 0; j < inImage.size().height; j++) {
//        sum = 0;
//        for (i = 0; i < inImage.size().width; i++) {
//            sum += *pixel;
//            pixel++;
//        }
//        this->avgIntensityHori[j] = (double)sum/inImage.size().width; // TODO transform in object attribute
//    }
//}
//
// not used as image are not noisy
//void Starburst::reduceLineNoise(cv::Mat &inImage) {
//    UINT8 *pixel = (UINT8*) inImage.data;
//    int i, j;
//    double beta2 = 1 - this->beta;
//    int adjustment;
//
//    this->recreateIntensityArrays(inImage);
//    this->calculateAvgIntensityHori(inImage);
//    for (j = 0; j < inImage.size().height; j++) {
//        this->intensityFactorHori[j] = this->avgIntensityHori[j]*this->beta + this->intensityFactorHori[j]*beta2;
//        adjustment = (int)(this->intensityFactorHori[j] - this->avgIntensityHori[j]);
//        for (i = 0; i < inImage.size().width; i++) {
//            *pixel = FIX_UINT8(*pixel+adjustment);
//            pixel++;
//        }
//    }
//}
//
//void Starburst::recreateIntensityArrays(cv::Mat &refImage) {
//    if(this->curH != refImage.size().height) {
//        this->curH = refImage.size().height;
//        this->deleteArrays();
//        this->avgIntensityHori = new double[this->curH];
//        this->intensityFactorHori = new double[this->curH];
//    }
//}

void Starburst::deleteArrays() {
    if(this->avgIntensityHori != 0) {
        delete[] this->avgIntensityHori;
    }
    if(this->intensityFactorHori != 0) {
        delete[] this->intensityFactorHori;
    }
}
