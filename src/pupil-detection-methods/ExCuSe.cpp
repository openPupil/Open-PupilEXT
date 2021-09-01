/*
  Version 1.0, 08.06.2015, Copyright University of Tübingen.

  The Code is created based on the method from the paper:
  "ExCuSe: Robust Pupil Detection in Real-World Scenarios", W. Fuhl, T. C. Kübler, K. Sippel, W. Rosenstiel, E. Kasneci
  CAIP 2015 : Computer Analysis of Images and Patterns

  The code and the algorithm are for non-comercial use only.
*/

#include <opencv2/core/mat.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include "ExCuSe.h"

using namespace std;
using namespace cv;

#define MAX_LINE 10000
#define IMG_SIZE 680 //400
#define DEF_SIZE 800 //800
//#define MAX_RADI 50

static void bwselect(cv::Mat *strong, cv::Mat *weak, cv::Mat *check)
{

    int pic_x = strong->cols;
    int pic_y = strong->rows;
    int lines[MAX_LINE];
    int lines_idx = 0;
    int idx = 0;

    for (int i = 1; i < pic_y - 1; i++)
    {
        for (int j = 1; j < pic_x - 1; j++)
        {

            if (strong->data[idx + j] != 0 && check->data[idx + j] == 0)
            {

                //check->data[idx+j]=255;
                lines_idx = 1;
                lines[0] = idx + j;

                int akt_idx = 0;

                while (akt_idx < lines_idx && lines_idx < MAX_LINE - 1)
                {
                    int akt_pos = lines[akt_idx];

                    if (akt_pos - pic_x - 1 >= 0 && akt_pos + pic_x + 1 < pic_x * pic_y)
                    {
                        for (int k1 = -1; k1 < 2; k1++)
                            for (int k2 = -1; k2 < 2; k2++)
                            {
                                if (check->data[(akt_pos + (k1 * pic_x)) + k2] == 0 && weak->data[(akt_pos + (k1 * pic_x)) + k2] != 0)
                                {
                                    check->data[(akt_pos + (k1 * pic_x)) + k2] = 255;
                                    lines_idx++;
                                    lines[lines_idx - 1] = (akt_pos + (k1 * pic_x)) + k2;
                                }
                            }
                    }
                    akt_idx++;
                }
            }
        }
        idx += pic_x;
    }
}

static Mat cbwselect(const Mat &strong, const Mat &weak)
{

    int pic_x = strong.cols;
    int pic_y = strong.rows;

    Mat check = Mat::zeros(pic_y, pic_x, CV_8U);

    int lines[MAX_LINE] = {0};
    int lines_idx = 0;

    int idx = 0;

    for (int i = 1; i < pic_y - 1; i++)
    {
        for (int j = 1; j < pic_x - 1; j++)
        {

            if (strong.at<uchar>(idx + j) != 0 && check.at<uchar>(idx + j) == 0)
            {

                check.at<uchar>(idx + j) = 255;
                lines_idx = 1;
                lines[0] = idx + j;

                int akt_idx = 0;

                while (akt_idx < lines_idx && lines_idx < MAX_LINE)
                {

                    int akt_pos = lines[akt_idx];

                    if (akt_pos - pic_x - 1 >= 0 && akt_pos + pic_x + 1 < pic_x * pic_y)
                    {
                        for (int k1 = -1; k1 < 2; k1++)
                        {
                            for (int k2 = -1; k2 < 2; k2++)
                            {

                                if (check.at<uchar>((akt_pos + (k1 * pic_x)) + k2) == 0 && weak.at<uchar>((akt_pos + (k1 * pic_x)) + k2) != 0)
                                {
                                    check.at<uchar>((akt_pos + (k1 * pic_x)) + k2) = 255;
                                    // ML 18.11.20: fixed array boundary access for lines array
                                    if (lines_idx < MAX_LINE)
                                    {
                                        lines[lines_idx] = (akt_pos + (k1 * pic_x)) + k2;
                                        lines_idx++;
                                    }
                                }
                            }
                        }
                    }
                    akt_idx++;
                }
            }
        }
        idx += pic_x;
    }

    return check;
}

static cv::Mat canny_impl(cv::Mat *pic)
{
    int k_sz = 16;

    float gau[16] = {0.000000220358050f, 0.000007297256405f, 0.000146569312970f, 0.001785579770079f,
                     0.013193749090229f, 0.059130281094460f, 0.160732768610747f, 0.265003534507060f, 0.265003534507060f,
                     0.160732768610747f, 0.059130281094460f, 0.013193749090229f, 0.001785579770079f, 0.000146569312970f,
                     0.000007297256405f, 0.000000220358050f};
    float deriv_gau[16] = {-0.000026704586264f, -0.000276122963398f, -0.003355163265098f, -0.024616683775044f, -0.108194751875585f,
                           -0.278368310241814f, -0.388430056419619f, -0.196732206873178f, 0.196732206873178f, 0.388430056419619f,
                           0.278368310241814f, 0.108194751875585f, 0.024616683775044f, 0.003355163265098f, 0.000276122963398f, 0.000026704586264f};

    cv::Point anchor = cv::Point(-1, -1);
    float delta = 0;
    int ddepth = -1;

    pic->convertTo(*pic, CV_32FC1);

    cv::Mat gau_x = cv::Mat(1, k_sz, CV_32FC1, &gau);
    cv::Mat deriv_gau_x = cv::Mat(1, k_sz, CV_32FC1, &deriv_gau);

    cv::Mat res_x;
    cv::Mat res_y;

    cv::transpose(*pic, *pic);
    filter2D(*pic, res_x, ddepth, gau_x, anchor, delta, cv::BORDER_REPLICATE);
    cv::transpose(*pic, *pic);
    cv::transpose(res_x, res_x);

    filter2D(res_x, res_x, ddepth, deriv_gau_x, anchor, delta, cv::BORDER_REPLICATE);

    filter2D(*pic, res_y, ddepth, gau_x, anchor, delta, cv::BORDER_REPLICATE);

    cv::transpose(res_y, res_y);
    filter2D(res_y, res_y, ddepth, deriv_gau_x, anchor, delta, cv::BORDER_REPLICATE);
    cv::transpose(res_y, res_y);

    cv::Mat res = cv::Mat::zeros(pic->rows, pic->cols, CV_32FC1);

    float *p_res, *p_x, *p_y;

    for (int i = 0; i < res.rows; i++)
    {
        p_res = res.ptr<float>(i);
        p_x = res_x.ptr<float>(i);
        p_y = res_y.ptr<float>(i);

        for (int j = 0; j < res.cols; j++)
        {
            //res.at<float>(j, i)= sqrt( (res_x.at<float>(j, i)*res_x.at<float>(j, i)) + (res_y.at<float>(j, i)*res_y.at<float>(j, i)) );
            //res.at<float>(j, i)=robust_pytagoras_after_MOLAR_MORRIS(res_x.at<float>(j, i), res_y.at<float>(j, i));
            //res.at<float>(j, i)=hypot(res_x.at<float>(j, i), res_y.at<float>(j, i));

            //p_res[j]=__ieee754_hypot(p_x[j], p_y[j]);

            p_res[j] = hypot(p_x[j], p_y[j]);
        }
    }

    //th selection
    int PercentOfPixelsNotEdges = 0.7 * res.cols * res.rows;
    float ThresholdRatio = 0.4f;

    float high_th = 0;
    float low_th = 0;

    int h_sz = 64;
    int hist[64];
    for (int i = 0; i < h_sz; i++)
        hist[i] = 0;

    cv::normalize(res, res, 0, 1, cv::NORM_MINMAX, CV_32FC1);
    cv::Mat res_idx = cv::Mat::zeros(pic->rows, pic->cols, CV_8U);
    cv::normalize(res, res_idx, 0, 63, cv::NORM_MINMAX, CV_32S);

    int *p_res_idx = 0;

    for (int i = 0; i < res.rows; i++)
    {
        p_res_idx = res_idx.ptr<int>(i);
        for (int j = 0; j < res.cols; j++)
        {
            hist[p_res_idx[j]]++;
        }
    }

    int sum = 0;

    for (int i = 0; i < h_sz; i++)
    {
        sum += hist[i];
        if (sum > PercentOfPixelsNotEdges)
        {
            high_th = float(i + 1) / float(h_sz);
            break;
        }
    }

    low_th = ThresholdRatio * high_th;

    //non maximum supression + interpolation
    cv::Mat non_ms = cv::Mat::zeros(pic->rows, pic->cols, CV_8U);
    cv::Mat non_ms_hth = cv::Mat::zeros(pic->rows, pic->cols, CV_8U);

    float ix, iy, grad1, grad2, d;
    char *p_non_ms, *p_non_ms_hth;
    float *p_res_t, *p_res_b;

    for (int i = 1; i < res.rows - 1; i++)
    {
        p_non_ms = non_ms.ptr<char>(i);
        p_non_ms_hth = non_ms_hth.ptr<char>(i);

        p_res = res.ptr<float>(i);
        p_res_t = res.ptr<float>(i - 1);
        p_res_b = res.ptr<float>(i + 1);

        p_x = res_x.ptr<float>(i);
        p_y = res_y.ptr<float>(i);

        for (int j = 1; j < res.cols - 1; j++)
        {

            iy = p_y[j];
            ix = p_x[j];

            if ((iy <= 0 && ix > -iy) || (iy >= 0 && ix < -iy))
            {

                d = abs(iy / ix);
                grad1 = (p_res[j + 1] * (1 - d)) + (p_res_t[j + 1] * d);
                grad2 = (p_res[j - 1] * (1 - d)) + (p_res_b[j - 1] * d);

                if (p_res[j] >= grad1 && p_res[j] >= grad2)
                {
                    p_non_ms[j] = (char)255;

                    if (p_res[j] > high_th)
                        p_non_ms_hth[j] = (char)255;
                }
            }

            if ((ix > 0 && -iy >= ix) || (ix < 0 && -iy <= ix))
            {
                d = abs(ix / iy);
                grad1 = (p_res_t[j] * (1 - d)) + (p_res_t[j + 1] * d);
                grad2 = (p_res_b[j] * (1 - d)) + (p_res_b[j - 1] * d);

                if (p_res[j] >= grad1 && p_res[j] >= grad2)
                {
                    p_non_ms[j] = (char)255;
                    if (p_res[j] > high_th)
                        p_non_ms_hth[j] = (char)255;
                }
            }

            if ((ix <= 0 && ix > iy) || (ix >= 0 && ix < iy))
            {
                d = abs(ix / iy);
                grad1 = (p_res_t[j] * (1 - d)) + (p_res_t[j - 1] * d);
                grad2 = (p_res_b[j] * (1 - d)) + (p_res_b[j + 1] * d);

                if (p_res[j] >= grad1 && p_res[j] >= grad2)
                {
                    p_non_ms[j] = (char)255;
                    if (p_res[j] > high_th)
                        p_non_ms_hth[j] = (char)255;
                }
            }

            if ((iy < 0 && ix <= iy) || (iy > 0 && ix >= iy))
            {
                d = abs(iy / ix);
                grad1 = (p_res[j - 1] * (1 - d)) + (p_res_t[j - 1] * d);
                grad2 = (p_res[j + 1] * (1 - d)) + (p_res_b[j + 1] * d);

                if (p_res[j] >= grad1 && p_res[j] >= grad2)
                {
                    p_non_ms[j] = (char)255;
                    if (p_res[j] > high_th)
                        p_non_ms_hth[j] = (char)255;
                }
            }
        }
    }

    ////bw select
    //cv::Mat res_lin=cv::Mat::zeros(pic->rows, pic->cols, CV_8U);
    //bwselect(&non_ms_hth, &non_ms,&res_lin);
    //pic->convertTo(*pic, CV_8U);

    Mat res_lin = cbwselect(non_ms_hth, non_ms);

    return res_lin;
}

static bool peek(cv::Mat *pic, double *stddev, int start_x, int end_x, int start_y, int end_y, int peek_detector_factor, int bright_region_th)
{

    int gray_hist[256];
    int max_gray = 0;
    int max_gray_pos = 0;
    int mean_gray = 0;
    int mean_gray_cnt = 0;

    for (int i = 0; i < 256; i++)
        gray_hist[i] = 0;

    double mean_feld[1000];
    double std_feld[1000];
    for (int i = start_x; i < end_x; i++)
    {
        mean_feld[i] = 0;
        std_feld[i] = 0;
    }

    for (int i = start_x; i < end_x; i++)
        for (int j = start_y; j < end_y; j++)
        {
            int idx = (int)pic->data[(pic->cols * j) + i];
            gray_hist[idx]++;
            mean_feld[i] += idx;
        }

    for (int i = start_x; i < end_x; i++)
        mean_feld[i] = (mean_feld[i] / double(end_y - start_y));

    for (int i = start_x; i < end_x; i++)
        for (int j = start_y; j < end_y; j++)
        {
            int idx = (int)pic->data[(pic->cols * j) + i];
            std_feld[i] += (mean_feld[i] - idx) * (mean_feld[i] - idx);
        }

    for (int i = start_x; i < end_x; i++)
        std_feld[i] = sqrt(std_feld[i] / double(end_y - start_y));

    *stddev = 0;
    for (int i = start_x; i < end_x; i++)
    {
        *stddev += std_feld[i];
    }

    *stddev = *stddev / ((end_x - start_x));

    for (int i = 0; i < 256; i++)
        if (gray_hist[i] > 0)
        {

            mean_gray += gray_hist[i];
            mean_gray_cnt++;

            if (max_gray < gray_hist[i])
            {
                max_gray = gray_hist[i];
                max_gray_pos = i;
            }
        }

    if (mean_gray_cnt < 1)
        mean_gray_cnt = 1;

    mean_gray = ceil((double)mean_gray / (double)mean_gray_cnt);

    if (max_gray > (mean_gray * peek_detector_factor) && max_gray_pos > bright_region_th)
        return true;
    else
        return false;
}

static void remove_points_with_low_angle(cv::Mat *edge, int start_xx, int end_xx, int start_yy, int end_yy)
{

    int start_x = start_xx + 5;
    int end_x = end_xx - 5;
    int start_y = start_yy + 5;
    int end_y = end_yy - 5;

    if (start_x < 5)
        start_x = 5;
    if (end_x > edge->cols - 5)
        end_x = edge->cols - 5;
    if (start_y < 5)
        start_y = 5;
    if (end_y > edge->rows - 5)
        end_y = edge->rows - 5;

    for (int j = start_y; j < end_y; j++)
        for (int i = start_x; i < end_x; i++)
        {

            if ((int)edge->data[(edge->cols * (j)) + (i)])
            {
                int box[8];

                box[0] = (int)edge->data[(edge->cols * (j - 1)) + (i - 1)];
                box[1] = (int)edge->data[(edge->cols * (j - 1)) + (i)];
                box[2] = (int)edge->data[(edge->cols * (j - 1)) + (i + 1)];
                box[3] = (int)edge->data[(edge->cols * (j)) + (i + 1)];
                box[4] = (int)edge->data[(edge->cols * (j + 1)) + (i + 1)];
                box[5] = (int)edge->data[(edge->cols * (j + 1)) + (i)];
                box[6] = (int)edge->data[(edge->cols * (j + 1)) + (i - 1)];
                box[7] = (int)edge->data[(edge->cols * (j)) + (i - 1)];

                bool valid = false;

                for (int k = 0; k < 8 && !valid; k++)
                    //if( box[k] && (box[(k+3)%8] || box[(k+4)%8] || box[(k+5)%8]) ) valid=true;
                    if (box[k] && (box[(k + 2) % 8] || box[(k + 3) % 8] || box[(k + 4) % 8] || box[(k + 5) % 8] || box[(k + 6) % 8]))
                        valid = true;

                if (!valid)
                    edge->data[(edge->cols * (j)) + (i)] = 0;
            }
        }

    for (int j = start_y; j < end_y; j++)
        for (int i = start_x; i < end_x; i++)
        {
            int box[9];

            box[4] = (int)edge->data[(edge->cols * (j)) + (i)];

            if (box[4])
            {
                box[1] = (int)edge->data[(edge->cols * (j - 1)) + (i)];
                box[3] = (int)edge->data[(edge->cols * (j)) + (i - 1)];
                box[5] = (int)edge->data[(edge->cols * (j)) + (i + 1)];
                box[7] = (int)edge->data[(edge->cols * (j + 1)) + (i)];

                if ((box[5] && box[7]))
                    edge->data[(edge->cols * (j)) + (i)] = 0;
                if ((box[5] && box[1]))
                    edge->data[(edge->cols * (j)) + (i)] = 0;
                if ((box[3] && box[7]))
                    edge->data[(edge->cols * (j)) + (i)] = 0;
                if ((box[3] && box[1]))
                    edge->data[(edge->cols * (j)) + (i)] = 0;

                //if( (box[1] && box[5]) || (box[1] && box[3]) || (box[3] && box[7]) || (box[5] && box[7]) )
                //		edge->data[(edge->cols*(j))+(i)]=0;
            }
        }

    for (int j = start_y; j < end_y; j++)
        for (int i = start_x; i < end_x; i++)
        {
            int box[17];

            box[4] = (int)edge->data[(edge->cols * (j)) + (i)];

            if (box[4])
            {
                box[0] = (int)edge->data[(edge->cols * (j - 1)) + (i - 1)];
                box[1] = (int)edge->data[(edge->cols * (j - 1)) + (i)];
                box[2] = (int)edge->data[(edge->cols * (j - 1)) + (i + 1)];

                box[3] = (int)edge->data[(edge->cols * (j)) + (i - 1)];
                box[5] = (int)edge->data[(edge->cols * (j)) + (i + 1)];

                box[6] = (int)edge->data[(edge->cols * (j + 1)) + (i - 1)];
                box[7] = (int)edge->data[(edge->cols * (j + 1)) + (i)];
                box[8] = (int)edge->data[(edge->cols * (j + 1)) + (i + 1)];

                //external
                box[9] = (int)edge->data[(edge->cols * (j)) + (i + 2)];
                box[10] = (int)edge->data[(edge->cols * (j + 2)) + (i)];

                box[11] = (int)edge->data[(edge->cols * (j)) + (i + 3)];
                box[12] = (int)edge->data[(edge->cols * (j - 1)) + (i + 2)];
                box[13] = (int)edge->data[(edge->cols * (j + 1)) + (i + 2)];

                box[14] = (int)edge->data[(edge->cols * (j + 3)) + (i)];
                box[15] = (int)edge->data[(edge->cols * (j + 2)) + (i - 1)];
                box[16] = (int)edge->data[(edge->cols * (j + 2)) + (i + 1)];

                if ((box[10] && !box[7]) && (box[8] || box[6]))
                {
                    edge->data[(edge->cols * (j + 1)) + (i - 1)] = 0;
                    edge->data[(edge->cols * (j + 1)) + (i + 1)] = 0;
                    edge->data[(edge->cols * (j + 1)) + (i)] = 255;
                }

                if ((box[14] && !box[7] && !box[10]) && ((box[8] || box[6]) && (box[16] || box[15])))
                {
                    edge->data[(edge->cols * (j + 1)) + (i + 1)] = 0;
                    edge->data[(edge->cols * (j + 1)) + (i - 1)] = 0;
                    edge->data[(edge->cols * (j + 2)) + (i + 1)] = 0;
                    edge->data[(edge->cols * (j + 2)) + (i - 1)] = 0;
                    edge->data[(edge->cols * (j + 1)) + (i)] = 255;
                    edge->data[(edge->cols * (j + 2)) + (i)] = 255;
                }

                if ((box[9] && !box[5]) && (box[8] || box[2]))
                {
                    edge->data[(edge->cols * (j + 1)) + (i + 1)] = 0;
                    edge->data[(edge->cols * (j - 1)) + (i + 1)] = 0;
                    edge->data[(edge->cols * (j)) + (i + 1)] = 255;
                }

                if ((box[11] && !box[5] && !box[9]) && ((box[8] || box[2]) && (box[13] || box[12])))
                {
                    edge->data[(edge->cols * (j + 1)) + (i + 1)] = 0;
                    edge->data[(edge->cols * (j - 1)) + (i + 1)] = 0;
                    edge->data[(edge->cols * (j + 1)) + (i + 2)] = 0;
                    edge->data[(edge->cols * (j - 1)) + (i + 2)] = 0;
                    edge->data[(edge->cols * (j)) + (i + 1)] = 255;
                    edge->data[(edge->cols * (j)) + (i + 2)] = 255;
                }
            }
        }

    for (int j = start_y; j < end_y; j++)
        for (int i = start_x; i < end_x; i++)
        {

            int box[33];

            box[4] = (int)edge->data[(edge->cols * (j)) + (i)];

            if (box[4])
            {
                box[0] = (int)edge->data[(edge->cols * (j - 1)) + (i - 1)];
                box[1] = (int)edge->data[(edge->cols * (j - 1)) + (i)];
                box[2] = (int)edge->data[(edge->cols * (j - 1)) + (i + 1)];

                box[3] = (int)edge->data[(edge->cols * (j)) + (i - 1)];
                box[5] = (int)edge->data[(edge->cols * (j)) + (i + 1)];

                box[6] = (int)edge->data[(edge->cols * (j + 1)) + (i - 1)];
                box[7] = (int)edge->data[(edge->cols * (j + 1)) + (i)];
                box[8] = (int)edge->data[(edge->cols * (j + 1)) + (i + 1)];

                box[9] = (int)edge->data[(edge->cols * (j - 1)) + (i + 2)];
                box[10] = (int)edge->data[(edge->cols * (j - 1)) + (i - 2)];
                box[11] = (int)edge->data[(edge->cols * (j + 1)) + (i + 2)];
                box[12] = (int)edge->data[(edge->cols * (j + 1)) + (i - 2)];

                box[13] = (int)edge->data[(edge->cols * (j - 2)) + (i - 1)];
                box[14] = (int)edge->data[(edge->cols * (j - 2)) + (i + 1)];
                box[15] = (int)edge->data[(edge->cols * (j + 2)) + (i - 1)];
                box[16] = (int)edge->data[(edge->cols * (j + 2)) + (i + 1)];

                box[17] = (int)edge->data[(edge->cols * (j - 3)) + (i - 1)];
                box[18] = (int)edge->data[(edge->cols * (j - 3)) + (i + 1)];
                box[19] = (int)edge->data[(edge->cols * (j + 3)) + (i - 1)];
                box[20] = (int)edge->data[(edge->cols * (j + 3)) + (i + 1)];

                box[21] = (int)edge->data[(edge->cols * (j + 1)) + (i + 3)];
                box[22] = (int)edge->data[(edge->cols * (j + 1)) + (i - 3)];
                box[23] = (int)edge->data[(edge->cols * (j - 1)) + (i + 3)];
                box[24] = (int)edge->data[(edge->cols * (j - 1)) + (i - 3)];

                box[25] = (int)edge->data[(edge->cols * (j - 2)) + (i - 2)];
                box[26] = (int)edge->data[(edge->cols * (j + 2)) + (i + 2)];
                box[27] = (int)edge->data[(edge->cols * (j - 2)) + (i + 2)];
                box[28] = (int)edge->data[(edge->cols * (j + 2)) + (i - 2)];

                box[29] = (int)edge->data[(edge->cols * (j - 3)) + (i - 3)];
                box[30] = (int)edge->data[(edge->cols * (j + 3)) + (i + 3)];
                box[31] = (int)edge->data[(edge->cols * (j - 3)) + (i + 3)];
                box[32] = (int)edge->data[(edge->cols * (j + 3)) + (i - 3)];

                if (box[7] && box[2] && box[9])
                    edge->data[(edge->cols * (j)) + (i)] = 0;
                if (box[7] && box[0] && box[10])
                    edge->data[(edge->cols * (j)) + (i)] = 0;
                if (box[1] && box[8] && box[11])
                    edge->data[(edge->cols * (j)) + (i)] = 0;
                if (box[1] && box[6] && box[12])
                    edge->data[(edge->cols * (j)) + (i)] = 0;

                if (box[0] && box[13] && box[17] && box[8] && box[11] && box[21])
                    edge->data[(edge->cols * (j)) + (i)] = 0;
                if (box[2] && box[14] && box[18] && box[6] && box[12] && box[22])
                    edge->data[(edge->cols * (j)) + (i)] = 0;
                if (box[6] && box[15] && box[19] && box[2] && box[9] && box[23])
                    edge->data[(edge->cols * (j)) + (i)] = 0;
                if (box[8] && box[16] && box[20] && box[0] && box[10] && box[24])
                    edge->data[(edge->cols * (j)) + (i)] = 0;

                if (box[0] && box[25] && box[29] && box[2] && box[27] && box[31])
                    edge->data[(edge->cols * (j)) + (i)] = 0;
                if (box[0] && box[25] && box[29] && box[6] && box[28] && box[32])
                    edge->data[(edge->cols * (j)) + (i)] = 0;
                if (box[8] && box[26] && box[30] && box[2] && box[27] && box[31])
                    edge->data[(edge->cols * (j)) + (i)] = 0;
                if (box[8] && box[26] && box[30] && box[6] && box[28] && box[32])
                    edge->data[(edge->cols * (j)) + (i)] = 0;
            }
        }
}

static std::vector<std::vector<cv::Point>> get_curves(cv::Mat *pic, cv::Mat *edge, int start_x, int end_x, int start_y, int end_y, double mean_dist, int inner_color_range)
{

    std::vector<std::vector<cv::Point>> all_curves;
    std::vector<cv::Point> curve;

    if (start_x < 2)
        start_x = 2;
    if (start_y < 2)
        start_y = 2;
    if (end_x > pic->cols - 2)
        end_x = pic->cols - 2;
    if (end_y > pic->rows - 2)
        end_y = pic->rows - 2;

    int curve_idx = 0;
    cv::Point mean_p;
    bool add_curve;
    int mean_inner_gray;
    int mean_inner_gray_last = 1000000;

    //curve.reserve(1000);
    //all_curves.reserve(1000);

    all_curves.clear();

    bool check[IMG_SIZE][IMG_SIZE];

    for (int i = 0; i < IMG_SIZE; i++)
        for (int j = 0; j < IMG_SIZE; j++)
            check[i][j] = 0;

    for (int i = start_x; i < end_x; i++)
        for (int j = start_y; j < end_y; j++)
        {

            if (edge->data[(edge->cols * (j)) + (i)] == 255 && !check[i][j])
            {
                check[i][j] = 1;

                curve.clear();
                curve_idx = 0;

                curve.push_back(cv::Point(i, j));
                mean_p.x = i;
                mean_p.y = j;
                curve_idx++;

                int akt_idx = 0;

                while (akt_idx < curve_idx)
                {

                    cv::Point akt_pos = curve[akt_idx];
                    for (int k1 = -1; k1 < 2; k1++)
                        for (int k2 = -1; k2 < 2; k2++)
                        {

                            if (akt_pos.x + k1 >= start_x && akt_pos.x + k1 < end_x && akt_pos.y + k2 >= start_y && akt_pos.y + k2 < end_y)
                                if (!check[akt_pos.x + k1][akt_pos.y + k2])
                                    if (edge->data[(edge->cols * (akt_pos.y + k2)) + (akt_pos.x + k1)] == 255)
                                    {
                                        check[akt_pos.x + k1][akt_pos.y + k2] = 1;

                                        mean_p.x += akt_pos.x + k1;
                                        mean_p.y += akt_pos.y + k2;
                                        curve.push_back(cv::Point(akt_pos.x + k1, akt_pos.y + k2));
                                        curve_idx++;
                                    }
                        }
                    akt_idx++;
                }

                if (curve_idx > 0 && curve.size() > 0)
                {
                    add_curve = true;
                    mean_p.x = floor((double(mean_p.x) / double(curve_idx)) + 0.5);
                    mean_p.y = floor((double(mean_p.y) / double(curve_idx)) + 0.5);

                    for (int i = 0; i < curve.size(); i++)
                        if (abs(mean_p.x - curve[i].x) <= mean_dist && abs(mean_p.y - curve[i].y) <= mean_dist)
                            add_curve = false;

                    //is ellipse fit possible
                    if (add_curve)
                    {
                        cv::RotatedRect ellipse = cv::fitEllipse(cv::Mat(curve));

                        if (ellipse.center.x < 0 || ellipse.center.y < 0 ||
                            ellipse.center.x > pic->cols || ellipse.center.y > pic->rows)
                        {

                            add_curve = false;
                        }

                        if (ellipse.size.height > 2.0 * ellipse.size.width ||
                            ellipse.size.width > 2.0 * ellipse.size.height)
                        {

                            add_curve = false;
                        }
                    }

                    if (add_curve)
                    {
                        if (inner_color_range > 0)
                        {
                            mean_inner_gray = 0;

                            //calc inner mean
                            for (int i = 0; i < curve.size(); i++)
                            {

                                if (pic->data[(pic->cols * (curve[i].y + 1)) + (curve[i].x)] != 0 || pic->data[(pic->cols * (curve[i].y - 1)) + (curve[i].x)] != 0)
                                    if (sqrt(pow(double(curve[i].y - mean_p.y), 2) + pow(double(curve[i].x - mean_p.x) + 2, 2)) <
                                        sqrt(pow(double(curve[i].y - mean_p.y), 2) + pow(double(curve[i].x - mean_p.x) - 2, 2)))

                                        mean_inner_gray += (unsigned char)pic->data[(pic->cols * (curve[i].y)) + (curve[i].x + 2)];
                                    else
                                        mean_inner_gray += (unsigned char)pic->data[(pic->cols * (curve[i].y)) + (curve[i].x - 2)];

                                else if (pic->data[(pic->cols * (curve[i].y)) + (curve[i].x + 1)] != 0 || pic->data[(pic->cols * (curve[i].y)) + (curve[i].x - 1)] != 0)
                                    if (sqrt(pow(double(curve[i].y - mean_p.y + 2), 2) + pow(double(curve[i].x - mean_p.x), 2)) <
                                        sqrt(pow(double(curve[i].y - mean_p.y - 2), 2) + pow(double(curve[i].x - mean_p.x), 2)))

                                        mean_inner_gray += (unsigned char)pic->data[(pic->cols * (curve[i].y + 2)) + (curve[i].x)];
                                    else
                                        mean_inner_gray += (unsigned char)pic->data[(pic->cols * (curve[i].y - 2)) + (curve[i].x)];

                                else if (pic->data[(pic->cols * (curve[i].y + 1)) + (curve[i].x + 1)] != 0 || pic->data[(pic->cols * (curve[i].y - 1)) + (curve[i].x - 1)] != 0)
                                    if (sqrt(pow(double(curve[i].y - mean_p.y - 2), 2) + pow(double(curve[i].x - mean_p.x + 2), 2)) <
                                        sqrt(pow(double(curve[i].y - mean_p.y + 2), 2) + pow(double(curve[i].x - mean_p.x - 2), 2)))

                                        mean_inner_gray += (unsigned char)pic->data[(pic->cols * (curve[i].y - 2)) + (curve[i].x + 2)];
                                    else
                                        mean_inner_gray += (unsigned char)pic->data[(pic->cols * (curve[i].y + 2)) + (curve[i].x - 2)];

                                else if (pic->data[(pic->cols * (curve[i].y - 1)) + (curve[i].x + 1)] != 0 || pic->data[(pic->cols * (curve[i].y + 1)) + (curve[i].x - 1)] != 0)
                                    if (sqrt(pow(double(curve[i].y - mean_p.y + 2), 2) + pow(double(curve[i].x - mean_p.x + 2), 2)) <
                                        sqrt(pow(double(curve[i].y - mean_p.y - 2), 2) + pow(double(curve[i].x - mean_p.x - 2), 2)))

                                        mean_inner_gray += (unsigned char)pic->data[(pic->cols * (curve[i].y + 2)) + (curve[i].x + 2)];
                                    else
                                        mean_inner_gray += (unsigned char)pic->data[(pic->cols * (curve[i].y - 2)) + (curve[i].x - 2)];

                                //mean_inner_gray+=pic->data[( pic->cols*( curve[i].y+((mean_p.y-curve[i].y)/2) ) ) + ( curve[i].x+((mean_p.x-curve[i].x)/2) )];
                            }

                            mean_inner_gray = floor((double(mean_inner_gray) / double(curve.size())) + 0.5);

                            if (mean_inner_gray_last > (mean_inner_gray + inner_color_range))
                            {
                                mean_inner_gray_last = mean_inner_gray;
                                all_curves.clear();
                                all_curves.push_back(curve);
                            }
                            else if (mean_inner_gray_last <= (mean_inner_gray + inner_color_range) && mean_inner_gray_last >= (mean_inner_gray - inner_color_range))
                            {

                                if (curve.size() > all_curves[0].size())
                                {
                                    mean_inner_gray_last = mean_inner_gray;
                                    all_curves.clear();
                                    all_curves.push_back(curve);
                                }
                            }
                        }
                        else
                            all_curves.push_back(curve);
                    }
                }
            }
        }

    /*
    std::cout<<all_curves.size()<<std::endl;

    for(int i=0;i<1;i++)
        for(int j=0;j<all_curves[i].size();j++)
         std::cout<<all_curves[i][j].x<<";"<<all_curves[i][j].y<<std::endl;


    cv::Mat m = cv::Mat::zeros(edge->rows, edge->cols, CV_8U);

    for(int i=0;i<all_curves.size();i++)
        for(int j=0;j<all_curves[i].size();j++)
            m.data[(edge->cols*all_curves[i][j].y)+(all_curves[i][j].x)]=255;

    imshow("ddd",m);
    */

    return all_curves;
}

static cv::RotatedRect find_best_edge(cv::Mat *pic, cv::Mat *edge, int start_x, int end_x, int start_y, int end_y, double mean_dist, int inner_color_range)
{

    cv::RotatedRect ellipse;
    ellipse.center.x = 0;
    ellipse.center.y = 0;
    ellipse.angle = 0.0;
    ellipse.size.height = 0.0;
    ellipse.size.width = 0.0;

    std::vector<std::vector<cv::Point>> all_curves = get_curves(pic, edge, start_x, end_x, start_y, end_y, mean_dist, inner_color_range);

    if (all_curves.size() == 1)
    {
        ellipse = cv::fitEllipse(cv::Mat(all_curves[0]));

        if (ellipse.center.x < 0 || ellipse.center.y < 0 || ellipse.center.x > pic->cols || ellipse.center.y > pic->rows)
        {
            ellipse.center.x = 0;
            ellipse.center.y = 0;
            ellipse.angle = 0.0;
            ellipse.size.height = 0.0;
            ellipse.size.width = 0.0;
        }
    }
    else
    {
        ellipse.center.x = 0;
        ellipse.center.y = 0;
        ellipse.angle = 0.0;
        ellipse.size.height = 0.0;
        ellipse.size.width = 0.0;
    }

    return ellipse;
}

static int calc_pos(int *hist, int mini, int max_region_hole, int min_region_size, int real_hist_sz)
{
    int pos = 0;

    int mean_pos = 0;
    int pos_hole = 0;
    int count = 0;
    int hole_size = 0;
    bool region_start = false;

    for (int i = 0; i < DEF_SIZE; i++)
    {
        if (hist[i] > mini && !region_start)
        {
            region_start = true;
            count++;
            mean_pos += i;
        }
        else if (hist[i] > mini && region_start)
        {
            count += 1 + hole_size;
            mean_pos += i + pos_hole;
            hole_size = 0;
            pos_hole = 0;
        }
        else if (hist[i] <= mini && region_start && hole_size < max_region_hole)
        {
            hole_size++;
            pos_hole += i;
        }
        else if (hist[i] <= mini && region_start && hole_size >= max_region_hole && count >= min_region_size)
        {

            if (count < 1)
                count = 1;
            mean_pos = mean_pos / count;
            if (pow(double((real_hist_sz / 2) - mean_pos), 2) < pow(double((real_hist_sz / 2) - pos), 2))
                pos = mean_pos;

            pos_hole = 0;
            hole_size = 0;
            region_start = 0;
            count = 0;
            mean_pos = 0;
        }
        else if (hist[i] <= mini && region_start && hole_size >= max_region_hole && count < min_region_size)
        {
            pos_hole = 0;
            hole_size = 0;
            region_start = 0;
            count = 0;
            mean_pos = 0;
        }
    }

    return pos;
}

static cv::Point th_angular_histo(cv::Mat *pic, cv::Mat *pic_th, int start_x, int end_x, int start_y, int end_y, int th, double th_histo, int max_region_hole, int min_region_size)
{
    cv::Point pos(0, 0);

    if (start_x < 0)
        start_x = 0;
    if (start_y < 0)
        start_y = 0;
    if (end_x > pic->cols)
        end_x = pic->cols;
    if (end_y > pic->rows)
        end_y = pic->rows;

    int max_l = 0;
    int max_lb = 0;
    int max_b = 0;
    int max_br = 0;

    int min_l, min_lb, min_b, min_br;
    int pos_l, pos_lb, pos_b, pos_br;

    int hist_l[DEF_SIZE];
    int hist_lb[DEF_SIZE];
    int hist_b[DEF_SIZE];
    int hist_br[DEF_SIZE];

    for (int i = 0; i < DEF_SIZE; i++)
    {
        hist_l[i] = 0;
        hist_lb[i] = 0;
        hist_b[i] = 0;
        hist_br[i] = 0;
    }

    int idx_lb = 0;
    int idx_br = 0;

    for (int i = start_x; i < end_x; i++)
    {
        for (int j = start_y; j < end_y; j++)
        {

            if (pic->data[(pic->cols * j) + i] < th)
            {

                pic_th->data[(pic->cols * j) + i] = 255;

                idx_lb = (pic->cols / 2) + (i - (pic->cols / 2)) + (j);
                idx_br = (pic->cols / 2) + (i - (pic->cols / 2)) + (pic->rows - j);

                if (j >= 0 && j < DEF_SIZE && i >= 0 && i < DEF_SIZE && idx_lb >= 0 && idx_lb < DEF_SIZE &&
                    idx_br >= 0 && idx_br < DEF_SIZE)
                {

                    if (++hist_l[j] > max_l)
                        max_l = hist_l[j];

                    if (++hist_b[i] > max_b)
                        max_b = hist_b[i];

                    if (++hist_lb[idx_lb] > max_lb)
                        max_lb = hist_lb[idx_lb];

                    if (++hist_br[idx_br] > max_br)
                        max_br = hist_br[idx_br];
                }
            }
        }
    }

    min_l = max_l - floor(max_l * th_histo);
    min_lb = max_lb - floor(max_lb * th_histo);
    min_b = max_b - floor(max_b * th_histo);
    min_br = max_br - floor(max_br * th_histo);

    pos_l = calc_pos(hist_l, min_l, max_region_hole, min_region_size, pic->rows);
    pos_lb = calc_pos(hist_lb, min_lb, max_region_hole, min_region_size, pic->cols + pic->rows);
    pos_b = calc_pos(hist_b, min_b, max_region_hole, min_region_size, pic->cols);
    pos_br = calc_pos(hist_br, min_br, max_region_hole, min_region_size, pic->cols + pic->rows);

    /*
    std::cout<<"min_l: "<<min_l<<" min_lb: "<<min_lb<<std::endl;
    std::cout<<"min_b: "<<min_b<<" min_br: "<<min_br<<std::endl;

    std::cout<<"l: "<<pos_l<<"    lb: "<<pos_lb<<std::endl;
    std::cout<<"b: "<<pos_b<<"    br: "<<pos_br<<std::endl;
    */

    if (pos_l > 0 && pos_lb > 0 && pos_b > 0 && pos_br > 0)
    {
        pos.x = floor(((pos_b + (floor((((pos_br + pic->rows) - pos_lb) / 2) + 0.5) + pos_lb - pic->rows)) / 2) + 0.5);
        pos.y = floor(((pos_l + (pic->rows - floor((((pos_br + pic->rows) - pos_lb) / 2) + 0.5))) / 2) + 0.5);
    }
    else if (pos_l > 0 && pos_b > 0)
    {
        pos.x = pos_b;
        pos.y = pos_l;
    }
    else if (pos_lb > 0 && pos_br > 0)
    {
        pos.x = floor((((pos_br + pic->rows) - pos_lb) / 2) + 0.5) + pos_lb - pic->rows;
        pos.y = pic->rows - floor((((pos_br + pic->rows) - pos_lb) / 2) + 0.5);
    }

    if (pos.x < 0)
        pos.x = 0;
    if (pos.y < 0)
        pos.y = 0;
    if (pos.x >= pic->cols)
        pos.x = 0;
    if (pos.y >= pic->rows)
        pos.y = 0;

    /*
    std::cout<<pos.x<<";"<<pos.y<<std::endl;
    imshow("th",*pic_th);
    cv::ellipse(*pic, cv::RotatedRect(pos, cv::Size2f(5,5),0), CV_RGB(255,255,255));
    imshow("angular",*pic);
    */

    return pos;
}

static void grow_region(cv::RotatedRect *ellipse, cv::Mat *pic, int max_ellipse_radi)
{

    float mean = 0.0;

    int x0 = ellipse->center.x;
    int y0 = ellipse->center.y;

    int mini = 1000;
    int maxi = 0;

    for (int i = -2; i < 3; i++)
        for (int j = -2; j < 3; j++)
        {
            if (y0 + j > 0 && y0 + j < pic->rows && x0 + i > 0 && x0 + i < pic->cols)
            {
                if (mini > pic->data[(pic->cols * (y0 + j)) + (x0 + i)])
                    mini = pic->data[(pic->cols * (y0 + j)) + (x0 + i)];

                if (maxi < pic->data[(pic->cols * (y0 + j)) + (x0 + i)])
                    maxi = pic->data[(pic->cols * (y0 + j)) + (x0 + i)];

                mean += pic->data[(pic->cols * (y0 + j)) + (x0 + i)];
            }
        }

    mean = mean / 25.0;

    float diff = abs(mean - pic->data[(pic->cols * (y0)) + (x0)]);

    int th_up = ceil(mean + diff) + 1;
    int th_down = floor(mean - diff) - 1;

    int radi = 0;

    for (int i = 1; i < max_ellipse_radi; i++)
    {
        radi = i;

        int left = 0;
        int right = 0;
        int top = 0;
        int bottom = 0;

        for (int j = -i; j <= 1 + (i * 2); j++)
        {

            //left
            if (y0 + j > 0 && y0 + j < pic->rows && x0 + i > 0 && x0 + i < pic->cols)
                if (pic->data[(pic->cols * (y0 + j)) + (x0 + i)] > th_down && pic->data[(pic->cols * (y0 + j)) + (x0 + i)] < th_up)
                {
                    left++;
                    //pic->data[(pic->cols*(y0+j))+(x0+i)]=255;
                }

            //right
            if (y0 + j > 0 && y0 + j < pic->rows && x0 - i > 0 && x0 - i < pic->cols)
                if (pic->data[(pic->cols * (y0 + j)) + (x0 - i)] > th_down && pic->data[(pic->cols * (y0 + j)) + (x0 - i)] < th_up)
                {
                    right++;
                    //pic->data[(pic->cols*(y0+j))+(x0-i)]=255;
                }

            //top
            if (y0 - i > 0 && y0 - i < pic->rows && x0 + j > 0 && x0 + j < pic->cols)
                if (pic->data[(pic->cols * (y0 - i)) + (x0 + j)] > th_down && pic->data[(pic->cols * (y0 - i)) + (x0 + j)] < th_up)
                {
                    top++;
                    //pic->data[(pic->cols*(y0-i))+(x0+j)]=255;
                }

            //bottom
            if (y0 + i > 0 && y0 + i < pic->rows && x0 + j > 0 && x0 + j < pic->cols)
                if (pic->data[(pic->cols * (y0 + i)) + (x0 + j)] > th_down && pic->data[(pic->cols * (y0 + i)) + (x0 + j)] < th_up)
                {
                    bottom++;
                    //pic->data[(pic->cols*(y0+i))+(x0+j)]=255;
                }
        }

        //if less than 25% stop
        float p_left = float(left) / float(1 + (2 * i));
        float p_right = float(right) / float(1 + (2 * i));
        float p_top = float(top) / float(1 + (2 * i));
        float p_bottom = float(bottom) / float(1 + (2 * i));

        if (p_top < 0.2 && p_bottom < 0.2)
            break;

        if (p_left < 0.2 && p_right < 0.2)
            break;
    }

    ellipse->size.height = radi;
    ellipse->size.width = radi;

    /*
    //collect points in threashold
    cv::Mat ch_mat=cv::Mat::zeros(pic->rows, pic->cols, CV_8UC1);
    cv::Point2i coor;
    std::vector<cv::Point2i> all_points;

    ch_mat.data[(pic->cols*(y0))+(x0)]=1;
    coor.x=x0;
    coor.y=y0;
    all_points.push_back(coor);

    int all_p_idx=0;

    while(all_p_idx<all_points.size()){
        cv::Point2i ak_p=all_points.at(all_p_idx);
        pic->data[(pic->cols*(ak_p.y))+(ak_p.x)]=255;

        for(int i=-1;i<2;i++)
            for(int j=-1;j<2;j++){

                if(ak_p.y+j>0 && ak_p.y+j<pic->rows && ak_p.x+i>0 && ak_p.x+i<pic->cols)
                if((int)ch_mat.data[(ch_mat.cols*(ak_p.y+j))+(ak_p.x+i)]==0 &&
                    (int)pic->data[(pic->cols*(ak_p.y+j))+(ak_p.x+i)]>th_down &&
                    (int)pic->data[(pic->cols*(ak_p.y+j))+(ak_p.x+i)]<th_up){

                        coor.x=ak_p.x+i;
                        coor.y=ak_p.y+j;
                        ch_mat.data[(pic->cols*(ak_p.y+j))+(ak_p.x+i)]=1;
                        all_points.push_back(coor);
                }
            }
        all_p_idx++;

        //std::cout<<all_points.size()<<":"<<all_p_idx<<std::endl;
    }
    */
}

static bool is_good_ellipse(cv::RotatedRect *ellipse, cv::Mat *pic, int good_ellipse_threshold, int max_ellipse_radi)
{

    if (ellipse->center.x == 0 && ellipse->center.y == 0)
        return false;

    if (ellipse->size.width == 0 || ellipse->size.height == 0)
        grow_region(ellipse, pic, max_ellipse_radi);

    float x0 = ellipse->center.x;
    float y0 = ellipse->center.y;

    int st_x = x0 - (ellipse->size.width / 4.0);
    int st_y = y0 - (ellipse->size.height / 4.0);
    int en_x = x0 + (ellipse->size.width / 4.0);
    int en_y = y0 + (ellipse->size.height / 4.0);

    float val = 0.0;
    float val_cnt = 0;
    float ext_val = 0.0;

    for (int i = st_x; i < en_x; i++)
        for (int j = st_y; j < en_y; j++)
        {

            if (i > 0 && i < pic->cols && j > 0 && j < pic->rows)
            {
                val += pic->data[(pic->cols * j) + i];
                val_cnt++;
            }
        }

    if (val_cnt > 0)
        val = val / val_cnt;
    else
        return false;

    val_cnt = 0;

    st_x = x0 - (ellipse->size.width * 0.75);
    st_y = y0 - (ellipse->size.height * 0.75);
    en_x = x0 + (ellipse->size.width * 0.75);
    en_y = y0 + (ellipse->size.height * 0.75);

    int in_st_x = x0 - (ellipse->size.width / 2);
    int in_st_y = y0 - (ellipse->size.height / 2);
    int in_en_x = x0 + (ellipse->size.width / 2);
    int in_en_y = y0 + (ellipse->size.height / 2);

    for (int i = st_x; i < en_x; i++)
        for (int j = st_y; j < en_y; j++)
        {
            if (!(i >= in_st_x && i <= in_en_x && j >= in_st_y && j <= in_en_y))
                if (i > 0 && i < pic->cols && j > 0 && j < pic->rows)
                {
                    ext_val += pic->data[(pic->cols * j) + i];
                    val_cnt++;
                    //pic->at<char>(j,i)=255;
                }
        }

    if (val_cnt > 0)
        ext_val = ext_val / val_cnt;
    else
        return false;

    val = ext_val - val;

    if (val > good_ellipse_threshold)
        return true;
    else
        return false;
}

static void rays(cv::Mat *th_edges, int end_x, int end_y, cv::Point *pos, int *ret)
{

    for (int i = 0; i < 8; i++)
        ret[i] = -1;
    for (int i = 0; i < end_x; i++)
        for (int j = 0; j < end_y; j++)
        {

            if (pos->x - i > 0 && pos->x + i < th_edges->cols && pos->y - j > 0 && pos->y + j < th_edges->rows)
            {

                if ((int)th_edges->data[(th_edges->cols * (pos->y)) + (pos->x + i)] != 0 && ret[0] == -1)
                {
                    ret[0] = th_edges->data[(th_edges->cols * (pos->y)) + (pos->x + i)] - 1;
                    //std::cout<<"val:"<<ret[0]<<" x:"<<pos->x+i<<" y:"<<pos->y<<std::endl;
                }
                if ((int)th_edges->data[(th_edges->cols * (pos->y)) + (pos->x - i)] != 0 && ret[1] == -1)
                {
                    ret[1] = th_edges->data[(th_edges->cols * (pos->y)) + (pos->x - i)] - 1;
                    //std::cout<<"val:"<<ret[0]<<" x:"<<pos->x-i<<" y:"<<pos->y<<std::endl;
                }
                if ((int)th_edges->data[(th_edges->cols * (pos->y + j)) + (pos->x)] != 0 && ret[2] == -1)
                {
                    ret[2] = th_edges->data[(th_edges->cols * (pos->y + j)) + (pos->x)] - 1;
                    //std::cout<<"val:"<<ret[0]<<" x:"<<pos->x<<" y:"<<pos->y+j<<std::endl;
                }
                if ((int)th_edges->data[(th_edges->cols * (pos->y - j)) + (pos->x)] != 0 && ret[3] == -1)
                {
                    ret[3] = th_edges->data[(th_edges->cols * (pos->y - j)) + (pos->x)] - 1;
                    //std::cout<<"val:"<<ret[0]<<" x:"<<pos->x<<" y:"<<pos->y-j<<std::endl;
                }

                if ((int)th_edges->data[(th_edges->cols * (pos->y + j)) + (pos->x + i)] != 0 && ret[4] == -1 && i == j)
                {
                    ret[4] = th_edges->data[(th_edges->cols * (pos->y + j)) + (pos->x + i)] - 1;
                    //std::cout<<"val:"<<ret[0]<<" x:"<<pos->x+i<<" y:"<<pos->y+j<<std::endl;
                }
                if ((int)th_edges->data[(th_edges->cols * (pos->y - j)) + (pos->x - i)] != 0 && ret[5] == -1 && i == j)
                {
                    ret[5] = th_edges->data[(th_edges->cols * (pos->y - j)) + (pos->x - i)] - 1;
                    //std::cout<<"val:"<<ret[0]<<" x:"<<pos->x-i<<" y:"<<pos->y-j<<std::endl;
                }
                if ((int)th_edges->data[(th_edges->cols * (pos->y - j)) + (pos->x + i)] != 0 && ret[6] == -1 && i == j)
                {
                    ret[6] = th_edges->data[(th_edges->cols * (pos->y - j)) + (pos->x + i)] - 1;
                    //std::cout<<"val:"<<ret[0]<<" x:"<<pos->x+i<<" y:"<<pos->y-j<<std::endl;
                }
                if ((int)th_edges->data[(th_edges->cols * (pos->y + j)) + (pos->x - i)] != 0 && ret[7] == -1 && i == j)
                {
                    ret[7] = th_edges->data[(th_edges->cols * (pos->y + j)) + (pos->x - i)] - 1;
                    //std::cout<<"val:"<<ret[0]<<" x:"<<pos->x-i<<" y:"<<pos->y+j<<std::endl;
                }
            }
        }
}

static void zero_around_region_th_border(cv::Mat *pic, cv::Mat *edges, cv::Mat *th_edges, int th, int edge_to_th, double mean_dist, double area, cv::RotatedRect *pos)
{

    int ret[8];
    std::vector<cv::Point> selected_points;
    cv::RotatedRect ellipse;

    int start_x = pos->center.x - (area * pic->cols);
    int end_x = pos->center.x + (area * pic->cols);
    int start_y = pos->center.y - (area * pic->rows);
    int end_y = pos->center.y + (area * pic->rows);

    if (start_x < 0)
        start_x = edge_to_th;
    if (start_y < 0)
        start_y = edge_to_th;
    if (end_x > pic->cols)
        end_x = pic->cols - (edge_to_th + 1);
    if (end_y > pic->rows)
        end_y = pic->rows - (edge_to_th + 1);

    th = th + th + 1;

    //std::cout<<"sx:"<<start_x<<" sy:"<<start_y<<" ex:"<<end_x<<" ey:"<<end_y<<" dist:"<<edge_to_th<<std::endl;
    for (int i = start_x; i < end_x; i++)
        for (int j = start_y; j < end_y; j++)
        {

            if (pic->data[(pic->cols * j) + (i)] < th)
            {

                for (int k1 = -edge_to_th; k1 < edge_to_th; k1++)
                    for (int k2 = -edge_to_th; k2 < edge_to_th; k2++)
                    {

                        if (i + k1 >= 0 && i + k1 < pic->cols && j + k2 > 0 && j + k2 < edges->rows)
                            if ((int)edges->data[(edges->cols * (j + k2)) + (i + k1)])
                                th_edges->data[(edges->cols * (j + k2)) + (i + k1)] = 255;
                    }
            }
        }

    //remove_points_with_low_angle(th_edges, start_x, end_x, start_y, end_y);
    std::vector<std::vector<cv::Point>> all_curves = get_curves(pic, th_edges, start_x, end_x, start_y, end_y, mean_dist, 0);

    //std::cout<<"all curves:"<<all_curves.size()<<std::endl;

    if (all_curves.size() > 0)
    {
        //zero th_edges
        /*
    for(int i=start_x-edge_to_th; i<end_x+edge_to_th; i++)
        for(int j=start_y-edge_to_th; j<end_y+edge_to_th; j++){
            th_edges->data[(th_edges->cols*(j))+(i)]=0;
        }
        */

        for (int i = 0; i < th_edges->cols; i++)
            for (int j = 0; j < th_edges->rows; j++)
            {
                th_edges->data[(th_edges->cols * (j)) + (i)] = 0;
            }

        //draw remaining edges
        for (int i = 0; i < all_curves.size(); i++)
        {
            //std::cout<<"written:"<<i+1<<std::endl;
            for (int j = 0; j < all_curves[i].size(); j++)
            {

                if (all_curves[i][j].x >= 0 && all_curves[i][j].x < th_edges->cols && all_curves[i][j].y >= 0 && all_curves[i][j].y < th_edges->rows)
                    th_edges->data[(th_edges->cols * (all_curves[i][j].y)) + (all_curves[i][j].x)] = i + 1; //+1 becouse of first is 0
            }
        }

        cv::Point st_pos;
        st_pos.x = pos->center.x;
        st_pos.y = pos->center.y;
        //send rays add edges to vector
        rays(th_edges, (end_x - start_x) / 2, (end_y - start_y) / 2, &st_pos, ret);

        //for(int i=0; i<8; i++) std::cout<<"ret:"<<ret[i]<<std::endl;
        //cv::imshow("akt", *th_edges);
        //cv::waitKey(0);

        //gather points
        for (int i = 0; i < 8; i++)
            if (ret[i] > -1 && ret[i] < all_curves.size())
            {
                //std::cout<<"size:"<<all_curves.size()<<std::endl;
                //std::cout<<"idx:"<<ret[i]<<std::endl;
                for (int j = 0; j < all_curves[ret[i]].size(); j++)
                {
                    selected_points.push_back(all_curves[ret[i]][j]);
                }
            }
        //ellipse fit if size>5

        if (selected_points.size() > 5)
        {

            *pos = cv::fitEllipse(cv::Mat(selected_points));
            /*
            cv::ellipse(*pic, cv::RotatedRect(ellipse.operator CvBox2D()),CV_RGB(255,255,255));
            cv::imshow("akt", *pic);
            */
        }
    }

    /*
    std::cout<<pos->x<<";"<<pos->y<<std::endl;
    cv::ellipse(*pic, cv::RotatedRect(*pos, cv::Size2f(5,5),0), CV_RGB(255,255,255));
    imshow("opt",*pic);
    */
}

static void optimize_pos(cv::Mat *pic, double area, cv::Point *pos)
{

    int start_x = pos->x - (area * pic->cols);
    int end_x = pos->x + (area * pic->cols);
    int start_y = pos->y - (area * pic->rows);
    int end_y = pos->y + (area * pic->rows);

    int val;
    int min_akt;
    int min_val = 1000000;

    int pos_x = 0;
    int pos_y = 0;
    int pos_count = 0;

    int reg_size = sqrt(sqrt(pow(double(area * pic->cols * 2), 2) + pow(double(area * pic->rows * 2), 2)));

    if (start_x < reg_size)
        start_x = reg_size;
    if (start_y < reg_size)
        start_y = reg_size;
    if (end_x > pic->cols)
        end_x = pic->cols - (reg_size + 1);
    if (end_y > pic->rows)
        end_y = pic->rows - (reg_size + 1);

    for (int i = start_x; i < end_x; i++)
        for (int j = start_y; j < end_y; j++)
        {

            min_akt = 0;

            for (int k1 = -reg_size; k1 < reg_size; k1++)
                for (int k2 = -reg_size; k2 < reg_size; k2++)
                {

                    if (i + k1 > 0 && i + k1 < pic->cols && j + k2 > 0 && j + k2 < pic->rows)
                    {
                        val = (pic->data[(pic->cols * j) + (i)] - pic->data[(pic->cols * (j + k2)) + (i + k1)]);
                        if (val > 0)
                            min_akt += val;
                    }
                }

            if (min_akt == min_val)
            {
                pos_x += i;
                pos_y += j;
                pos_count++;
            }

            if (min_akt < min_val)
            {
                min_val = min_akt;
                pos_x = i;
                pos_y = j;
                pos_count = 1;
            }
        }

    if (pos_count > 0)
    {
        pos->x = pos_x / pos_count;
        pos->y = pos_y / pos_count;
    }
}

static cv::RotatedRect runexcuse(cv::Mat *pic, cv::Mat *pic_th, cv::Mat *th_edges, int good_ellipse_threshold, int max_ellipse_radi)
{
    //mean under mean
    //mean_under_mean(pic, 5);
    cv::normalize(*pic, *pic, 0, 255, cv::NORM_MINMAX, CV_8U);

    double border = 0.1;
    int peek_detector_factor = 10;
    int bright_region_th = 199;
    double mean_dist = 3;
    int inner_color_range = 5;
    double th_histo = 0.5;
    int max_region_hole = 5;
    int min_region_size = 7;
    double area_opt = 0.1;
    double area_edges = 0.2;
    int edge_to_th = 5;

    cv::RotatedRect ellipse;
    cv::Point pos(0, 0);

    int start_x = floor(double(pic->cols) * border);
    int start_y = floor(double(pic->rows) * border);

    int end_x = pic->cols - start_x;
    int end_y = pic->rows - start_y;

    double stddev = 0;
    bool edges_only_tried = false;
    bool peek_found = false;
    int threshold_up = 0;

    //cv::Mat pic=rgb2gray(m); //m is input pointer to mat
    //cv::normalize(*m, *m, 0, 255, cv::NORM_MINMAX, CV_8U);
    //cv::Mat pic;
    //cvtColor(*m, pic, CV_RGB2GRAY);

    peek_found = peek(pic, &stddev, start_x, end_x, start_y, end_y, peek_detector_factor, bright_region_th);
    threshold_up = ceil(stddev / 2);
    threshold_up--;

    cv::Mat picpic = cv::Mat::zeros(end_y - start_y, end_x - start_x, CV_8U);

    for (int i = 0; i < picpic.cols; i++)
        for (int j = 0; j < picpic.rows; j++)
        {
            picpic.data[(picpic.cols * j) + i] = pic->data[(pic->cols * (start_y + j)) + (start_x + i)];
        }

    //cv::Mat detected_edges2;
    //cv::GaussianBlur(picpic,detected_edges2, cv::Size(15,15),sqrt(2.0));
    //Canny( detected_edges2, detected_edges2, stddev*0.4, stddev, 3 );
    cv::Mat detected_edges2 = canny_impl(&picpic);

    cv::Mat detected_edges = cv::Mat::zeros(pic->rows, pic->cols, CV_8U);
    for (int i = 0; i < detected_edges2.cols; i++)
        for (int j = 0; j < detected_edges2.rows; j++)
        {
            detected_edges.data[(detected_edges.cols * (start_y + j)) + (start_x + i)] = detected_edges2.data[(detected_edges2.cols * j) + i];
        }

    remove_points_with_low_angle(&detected_edges, start_x, end_x, start_y, end_y);

    //peek_found=1;
    if (peek_found)
    {
        edges_only_tried = true;
        ellipse = find_best_edge(pic, &detected_edges, start_x, end_x, start_y, end_y, mean_dist, inner_color_range);

        if (ellipse.center.x <= 0 || ellipse.center.x >= pic->cols || ellipse.center.y <= 0 || ellipse.center.y >= pic->rows)
        {
            ellipse.center.x = 0;
            ellipse.center.y = 0;
            ellipse.angle = 0.0;
            ellipse.size.height = 0.0;
            ellipse.size.width = 0.0;
            peek_found = false;
        }
    }

    if (!peek_found)
    {
        pos = th_angular_histo(pic, pic_th, start_x, end_x, start_y, end_y, threshold_up, th_histo, max_region_hole, min_region_size);

        ellipse.center.x = pos.x;
        ellipse.center.y = pos.y;
        ellipse.angle = 0.0;
        ellipse.size.height = 0.0;
        ellipse.size.width = 0.0;
    }

    if (pos.x == 0 && pos.y == 0 && !edges_only_tried)
    {
        ellipse = find_best_edge(pic, &detected_edges, start_x, end_x, start_y, end_y, mean_dist, inner_color_range);
        peek_found = true;
    }

    if (pos.x > 0 && pos.y > 0 && pos.x < pic->cols && pos.y < pic->rows && !peek_found)
    {
        optimize_pos(pic, area_opt, &pos);

        ellipse.center.x = pos.x;
        ellipse.center.y = pos.y;
        ellipse.angle = 0.0;
        ellipse.size.height = 0.0;
        ellipse.size.width = 0.0;
        zero_around_region_th_border(pic, &detected_edges, th_edges, threshold_up, edge_to_th, mean_dist, area_edges, &ellipse);
    }

    //if(ellipse.size.height>0 && ellipse.size.width>0.0){

    if (is_good_ellipse(&ellipse, pic, good_ellipse_threshold, max_ellipse_radi))
        return ellipse;
    else
    {
        ellipse.center.x = 0;
        ellipse.center.y = 0;
        ellipse.angle = 0.0;
        ellipse.size.height = 0.0;
        ellipse.size.width = 0.0;

        return ellipse;
    }

    //}

    return ellipse;
    /*
    if(is_possible_pupil(pic, ellipse))
        return ellipse;
    else{
        ellipse.center.x=0;
        ellipse.center.y=0;
        ellipse.angle=0.0;
        ellipse.size.height=0.0;
        ellipse.size.width=0.0;
        return ellipse;
    }
    */
}

Pupil ExCuSe::run(const Mat &frame)
{

    Mat downscaled = frame;
    float scalingRatio = 1.0;
    if (frame.rows > IMG_SIZE || frame.cols > IMG_SIZE)
    {
        // return ellipse;
        // Downscaling
        float rw = IMG_SIZE / (float)frame.cols;
        float rh = IMG_SIZE / (float)frame.rows;
        scalingRatio = min<float>(min<float>(rw, rh), 1.0);
        cv::resize(frame, downscaled, Size(), scalingRatio, scalingRatio, INTER_LINEAR);
    }

    Mat target;
    normalize(downscaled, target, 0, 255, NORM_MINMAX, CV_8U);

    Mat pic_th = Mat::zeros(target.rows, target.cols, CV_8U);
    Mat th_edges = Mat::zeros(target.rows, target.cols, CV_8U);

    cv::RotatedRect ellipse = runexcuse(&target, &pic_th, &th_edges, good_ellipse_threshold, max_ellipse_radi);
    cv::RotatedRect scaledEllipse(cv::Point2f(ellipse.center.x / scalingRatio, ellipse.center.y / scalingRatio), cv::Size2f(ellipse.size.width / scalingRatio, ellipse.size.height / scalingRatio), ellipse.angle);

    return Pupil(scaledEllipse);
}

void ExCuSe::run(const cv::Mat &frame, const Rect &roi, Pupil &pupil, const float &minPupilDiameterPx, const float &maxPupilDiameterPx)
{
    if (roi.area() < 10)
    {
        std::cout << "Bad ROI: falling back to regular detection.";
        PupilDetectionMethod::run(frame, pupil);
        return;
    }

    (void)minPupilDiameterPx;
    (void)maxPupilDiameterPx;

    pupil = run(frame(roi));
    if (pupil.center.x > 0 && pupil.center.y > 0)
        pupil.shift(roi.tl());
}
