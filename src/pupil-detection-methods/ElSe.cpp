/*
  Version 1.0, 17.12.2015, Copyright University of Tübingen.

  The Code is created based on the method from the paper:
  "ElSe: Ellipse Selection for Robust Pupil Detection in Real-World Environments", W. Fuhl, T. C. Santini, T. C. Kübler, E. Kasneci
  ETRA 2016 : Eye Tracking Research and Application 2016

  The code and the algorithm are for non-comercial use only.
*/

#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>
#include <opencv2/opencv.hpp>
#include "ElSe.h"

using namespace cv;

float ElSe::minArea = 0;
float ElSe::maxArea = 0;

#define IMG_SIZE 640 //400
#define MAX_LINE 10000

static bool is_good_ellipse_eval(RotatedRect *ellipse, Mat *pic, int *erg)
{

    if (ellipse->center.x == 0 && ellipse->center.y == 0)
        return false;

    float x0 = ellipse->center.x;
    float y0 = ellipse->center.y;

    int st_x = (int)ceil(x0 - (ellipse->size.width / 4.0));
    int st_y = (int)ceil(y0 - (ellipse->size.height / 4.0));
    int en_x = (int)floor(x0 + (ellipse->size.width / 4.0));
    int en_y = (int)floor(y0 + (ellipse->size.height / 4.0));

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

    st_x = (int)(x0 - (ellipse->size.width * 0.75));
    st_y = (int)(y0 - (ellipse->size.height * 0.75));
    en_x = (int)(x0 + (ellipse->size.width * 0.75));
    en_y = (int)(y0 + (ellipse->size.height * 0.75));

    int in_st_x = (int)ceil(x0 - (ellipse->size.width / 2));
    int in_st_y = (int)ceil(y0 - (ellipse->size.height / 2));
    int in_en_x = (int)floor(x0 + (ellipse->size.width / 2));
    int in_en_y = (int)floor(y0 + (ellipse->size.height / 2));

    for (int i = st_x; i < en_x; i++)
        for (int j = st_y; j < en_y; j++)
        {
            if (!(i >= in_st_x && i <= in_en_x && j >= in_st_y && j <= in_en_y))
                if (i > 0 && i < pic->cols && j > 0 && j < pic->rows)
                {
                    ext_val += pic->data[(pic->cols * j) + i];
                    val_cnt++;
                }
        }

    if (val_cnt > 0)
        ext_val = ext_val / val_cnt;
    else
        return false;

    val = ext_val - val;

    *erg = (int)val;

    if (val > 10)
        return true;
    else
        return false;
}

static int calc_inner_gray(Mat *pic, std::vector<Point> curve, RotatedRect ellipse)
{

    int gray_val = 0;
    int gray_cnt = 0;

    Mat checkmap = Mat::zeros(pic->size(), CV_8U);

    for (unsigned int i = 0; i < curve.size(); i++)
    {

        int vec_x = (int)round(curve[i].x - ellipse.center.x);
        int vec_y = (int)round(curve[i].y - ellipse.center.y);

        for (float p = 0.95f; p > 0.80f; p -= 0.01f)
        { //0.75;-0.05
            int p_x = (int)round(ellipse.center.x + float((float(vec_x) * p) + 0.5));
            int p_y = (int)round(ellipse.center.y + float((float(vec_y) * p) + 0.5));

            if (p_x > 0 && p_x < pic->cols && p_y > 0 && p_y < pic->rows)
            {

                if (checkmap.data[(pic->cols * p_y) + p_x] == 0)
                {
                    checkmap.data[(pic->cols * p_y) + p_x] = 1;
                    gray_val += (unsigned int)pic->data[(pic->cols * p_y) + p_x];
                    gray_cnt++;
                }
            }
        }
    }

    if (gray_cnt > 0)
        gray_val = gray_val / gray_cnt;
    else
        gray_val = 1000;

    return gray_val;
}

static std::vector<std::vector<Point>> get_curves(Mat *pic, Mat *edge, Mat *magni, int start_x, int end_x, int start_y, int end_y, double mean_dist, int inner_color_range)
{

    (void)magni;
    std::vector<std::vector<Point>> all_lines;

    std::vector<std::vector<Point>> all_curves;
    std::vector<Point> curve;

    std::vector<Point> all_means;

    if (start_x < 2)
        start_x = 2;
    if (start_y < 2)
        start_y = 2;
    if (end_x > pic->cols - 2)
        end_x = pic->cols - 2;
    if (end_y > pic->rows - 2)
        end_y = pic->rows - 2;

    int curve_idx = 0;
    Point mean_p;
    bool add_curve;
    int mean_inner_gray;
    int mean_inner_gray_last = 1000000;

    all_curves.clear();
    all_means.clear();
    all_lines.clear();

    bool check[IMG_SIZE][IMG_SIZE];

    for (int i = 0; i < IMG_SIZE; i++)
        for (int j = 0; j < IMG_SIZE; j++)
            check[i][j] = 0;

    //get all lines
    for (int i = start_x; i < end_x; i++)
        for (int j = start_y; j < end_y; j++)
        {

            if (edge->data[(edge->cols * (j)) + (i)] > 0 && !check[i][j])
            {
                check[i][j] = 1;

                curve.clear();
                curve_idx = 0;

                curve.push_back(Point(i, j));
                mean_p.x = i;
                mean_p.y = j;
                curve_idx++;

                int akt_idx = 0;

                while (akt_idx < curve_idx)
                {

                    Point akt_pos = curve[akt_idx];
                    for (int k1 = -1; k1 < 2; k1++)
                        for (int k2 = -1; k2 < 2; k2++)
                        {

                            if (akt_pos.x + k1 >= start_x && akt_pos.x + k1 < end_x && akt_pos.y + k2 >= start_y && akt_pos.y + k2 < end_y)
                                if (!check[akt_pos.x + k1][akt_pos.y + k2])
                                    if (edge->data[(edge->cols * (akt_pos.y + k2)) + (akt_pos.x + k1)] > 0)
                                    {
                                        check[akt_pos.x + k1][akt_pos.y + k2] = 1;

                                        mean_p.x += akt_pos.x + k1;
                                        mean_p.y += akt_pos.y + k2;
                                        curve.push_back(Point(akt_pos.x + k1, akt_pos.y + k2));
                                        curve_idx++;
                                    }
                        }
                    akt_idx++;
                }

                if (curve_idx > 10 && curve.size() > 10)
                {

                    mean_p.x = (int)floor((double(mean_p.x) / double(curve_idx)) + 0.5);
                    mean_p.y = (int)floor((double(mean_p.y) / double(curve_idx)) + 0.5);

                    all_means.push_back(mean_p);
                    all_lines.push_back(curve);
                }
            }
        }

    RotatedRect selected_ellipse;

    for (unsigned int iii = 0; iii < all_lines.size(); iii++)
    {

        curve = all_lines.at(iii);
        mean_p = all_means.at(iii);

        int results = 0;
        add_curve = true;

        RotatedRect ellipse;

        for (unsigned int i = 0; i < curve.size(); i++)
            if (abs(mean_p.x - curve[i].x) <= mean_dist && abs(mean_p.y - curve[i].y) <= mean_dist)
                add_curve = false;

        //is ellipse fit possible
        if (add_curve)
        {

            ellipse = fitEllipse(Mat(curve));

            if (ellipse.center.x < 0 || ellipse.center.y < 0 ||
                ellipse.center.x > pic->cols || ellipse.center.y > pic->rows)
            {

                add_curve = false;
            }

            if (ellipse.size.height > 3 * ellipse.size.width ||
                ellipse.size.width > 3 * ellipse.size.height)
            {

                add_curve = false;
            }

            if (add_curve)
            { // pupil area
                if (ellipse.size.width * ellipse.size.height < ElSe::minArea ||
                    ellipse.size.width * ellipse.size.height > ElSe::maxArea)
                    add_curve = false;
            }

            if (add_curve)
            {
                if (!is_good_ellipse_eval(&ellipse, pic, &results))
                    add_curve = false;
            }
        }

        if (add_curve)
        {

            if (inner_color_range >= 0)
            {
                mean_inner_gray = 0;
                mean_inner_gray = calc_inner_gray(pic, curve, ellipse);
                mean_inner_gray = (int)(mean_inner_gray * (1 + abs(ellipse.size.height - ellipse.size.width)));

                if (mean_inner_gray_last > mean_inner_gray)
                {
                    mean_inner_gray_last = mean_inner_gray;
                    all_curves.clear();
                    all_curves.push_back(curve);
                }
                else if (mean_inner_gray_last == mean_inner_gray)
                {

                    if (curve.size() > all_curves[0].size())
                    {
                        mean_inner_gray_last = mean_inner_gray;
                        all_curves.clear();
                        all_curves.push_back(curve);
                        selected_ellipse = ellipse;
                    }
                }
            }
        }
    }

    return all_curves;
}

static RotatedRect find_best_edge(Mat *pic, Mat *edge, Mat *magni, int start_x, int end_x, int start_y, int end_y, double mean_dist, int inner_color_range)
{

    RotatedRect ellipse;
    ellipse.center.x = 0;
    ellipse.center.y = 0;
    ellipse.angle = 0.0;
    ellipse.size.height = 0.0;
    ellipse.size.width = 0.0;

    std::vector<std::vector<Point>> all_curves = get_curves(pic, edge, magni, start_x, end_x, start_y, end_y, mean_dist, inner_color_range);

    if (all_curves.size() == 1)
    {
        ellipse = fitEllipse(Mat(all_curves[0]));

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

static void filter_edges(Mat *edge, int start_xx, int end_xx, int start_yy, int end_yy)
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
            }
        }

    //too many neigbours
    for (int j = start_y; j < end_y; j++)
        for (int i = start_x; i < end_x; i++)
        {
            int neig = 0;

            for (int k1 = -1; k1 < 2; k1++)
                for (int k2 = -1; k2 < 2; k2++)
                {

                    if (edge->data[(edge->cols * (j + k1)) + (i + k2)] > 0)
                        neig++;
                }

            if (neig > 3)
                edge->data[(edge->cols * (j)) + (i)] = 0;
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

                if (box[0] && box[25] && box[2] && box[27])
                    edge->data[(edge->cols * (j)) + (i)] = 0;
                if (box[0] && box[25] && box[6] && box[28])
                    edge->data[(edge->cols * (j)) + (i)] = 0;
                if (box[8] && box[26] && box[2] && box[27])
                    edge->data[(edge->cols * (j)) + (i)] = 0;
                if (box[8] && box[26] && box[6] && box[28])
                    edge->data[(edge->cols * (j)) + (i)] = 0;

                int box2[18];
                box2[1] = (int)edge->data[(edge->cols * (j)) + (i - 1)];

                box2[2] = (int)edge->data[(edge->cols * (j - 1)) + (i - 2)];
                box2[3] = (int)edge->data[(edge->cols * (j - 2)) + (i - 3)];

                box2[4] = (int)edge->data[(edge->cols * (j - 1)) + (i + 1)];
                box2[5] = (int)edge->data[(edge->cols * (j - 2)) + (i + 2)];

                box2[6] = (int)edge->data[(edge->cols * (j + 1)) + (i - 2)];
                box2[7] = (int)edge->data[(edge->cols * (j + 2)) + (i - 3)];

                box2[8] = (int)edge->data[(edge->cols * (j + 1)) + (i + 1)];
                box2[9] = (int)edge->data[(edge->cols * (j + 2)) + (i + 2)];

                box2[10] = (int)edge->data[(edge->cols * (j + 1)) + (i)];

                box2[15] = (int)edge->data[(edge->cols * (j - 1)) + (i - 1)];
                box2[16] = (int)edge->data[(edge->cols * (j - 2)) + (i - 2)];

                box2[11] = (int)edge->data[(edge->cols * (j + 2)) + (i + 1)];
                box2[12] = (int)edge->data[(edge->cols * (j + 3)) + (i + 2)];

                box2[13] = (int)edge->data[(edge->cols * (j + 2)) + (i - 1)];
                box2[14] = (int)edge->data[(edge->cols * (j + 3)) + (i - 2)];

                if (box2[1] && box2[2] && box2[3] && box2[4] && box2[5])
                    edge->data[(edge->cols * (j)) + (i)] = 0;
                if (box2[1] && box2[6] && box2[7] && box2[8] && box2[9])
                    edge->data[(edge->cols * (j)) + (i)] = 0;
                if (box2[10] && box2[11] && box2[12] && box2[4] && box2[5])
                    edge->data[(edge->cols * (j)) + (i)] = 0;
                if (box2[10] && box2[13] && box2[14] && box2[15] && box2[16])
                    edge->data[(edge->cols * (j)) + (i)] = 0;
            }
        }
}

static void bwselect(Mat *strong, Mat *weak, Mat *check)
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

                check->data[idx + j] = 255;
                lines_idx = 1;
                lines[0] = idx + j;

                int akt_idx = 0;

                while (akt_idx < lines_idx && lines_idx < MAX_LINE - 1)
                {

                    int akt_pos = lines[akt_idx];

                    if (akt_pos - pic_x - 1 >= 0 && akt_pos + pic_x + 1 < pic_x * pic_y)
                    {
                        for (int k1 = -1; k1 < 2; k1++)
                        {
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

static Mat canny_impl(Mat *pic, Mat *magni)
{
    int k_sz = 16;

    float gau[16] = {0.000000220358050f, 0.000007297256405f, 0.000146569312970f, 0.001785579770079f,
                     0.013193749090229f, 0.059130281094460f, 0.160732768610747f, 0.265003534507060f, 0.265003534507060f,
                     0.160732768610747f, 0.059130281094460f, 0.013193749090229f, 0.001785579770079f, 0.000146569312970f,
                     0.000007297256405f, 0.000000220358050f};
    float deriv_gau[16] = {-0.000026704586264f, -0.000276122963398f, -0.003355163265098f, -0.024616683775044f, -0.108194751875585f,
                           -0.278368310241814f, -0.388430056419619f, -0.196732206873178f, 0.196732206873178f, 0.388430056419619f,
                           0.278368310241814f, 0.108194751875585f, 0.024616683775044f, 0.003355163265098f, 0.000276122963398f, 0.000026704586264f};

    Point anchor = Point(-1, -1);
    float delta = 0;
    int ddepth = -1;

    pic->convertTo(*pic, CV_32FC1);

    Mat gau_x = Mat(1, k_sz, CV_32FC1, &gau);
    Mat deriv_gau_x = Mat(1, k_sz, CV_32FC1, &deriv_gau);

    Mat res_x;
    Mat res_y;

    transpose(*pic, *pic);
    filter2D(*pic, res_x, ddepth, gau_x, anchor, delta, BORDER_REPLICATE);
    transpose(*pic, *pic);
    transpose(res_x, res_x);

    filter2D(res_x, res_x, ddepth, deriv_gau_x, anchor, delta, BORDER_REPLICATE);
    filter2D(*pic, res_y, ddepth, gau_x, anchor, delta, BORDER_REPLICATE);

    transpose(res_y, res_y);
    filter2D(res_y, res_y, ddepth, deriv_gau_x, anchor, delta, BORDER_REPLICATE);
    transpose(res_y, res_y);

    *magni = Mat::zeros(pic->rows, pic->cols, CV_32FC1);

    float *p_res, *p_x, *p_y;
    for (int i = 0; i < magni->rows; i++)
    {
        p_res = magni->ptr<float>(i);
        p_x = res_x.ptr<float>(i);
        p_y = res_y.ptr<float>(i);

        for (int j = 0; j < magni->cols; j++)
        {
            //res.at<float>(j, i)= sqrt( (res_x.at<float>(j, i)*res_x.at<float>(j, i)) + (res_y.at<float>(j, i)*res_y.at<float>(j, i)) );
            //res.at<float>(j, i)=robust_pytagoras_after_MOLAR_MORRIS(res_x.at<float>(j, i), res_y.at<float>(j, i));
            //res.at<float>(j, i)=hypot(res_x.at<float>(j, i), res_y.at<float>(j, i));

            //p_res[j]=__ieee754_hypot(p_x[j], p_y[j]);

            p_res[j] = hypot(p_x[j], p_y[j]);
        }
    }

    //th selection
    int PercentOfPixelsNotEdges = (int)round(0.7 * magni->cols * magni->rows);
    float ThresholdRatio = 0.4f;

    float high_th = 0;
    float low_th = 0;

    int h_sz = 64;
    int hist[64];
    for (int i = 0; i < h_sz; i++)
        hist[i] = 0;

    normalize(*magni, *magni, 0, 1, NORM_MINMAX, CV_32FC1);

    Mat res_idx = Mat::zeros(pic->rows, pic->cols, CV_8U);
    normalize(*magni, res_idx, 0, 63, NORM_MINMAX, CV_32S);

    int *p_res_idx = 0;
    for (int i = 0; i < magni->rows; i++)
    {
        p_res_idx = res_idx.ptr<int>(i);
        for (int j = 0; j < magni->cols; j++)
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
    Mat non_ms = Mat::zeros(pic->rows, pic->cols, CV_8U);
    Mat non_ms_hth = Mat::zeros(pic->rows, pic->cols, CV_8U);

    float ix, iy, grad1, grad2, d;

    char *p_non_ms, *p_non_ms_hth;
    float *p_res_t, *p_res_b;
    for (int i = 1; i < magni->rows - 1; i++)
    {
        p_non_ms = non_ms.ptr<char>(i);
        p_non_ms_hth = non_ms_hth.ptr<char>(i);

        p_res = magni->ptr<float>(i);
        p_res_t = magni->ptr<float>(i - 1);
        p_res_b = magni->ptr<float>(i + 1);

        p_x = res_x.ptr<float>(i);
        p_y = res_y.ptr<float>(i);

        for (int j = 1; j < magni->cols - 1; j++)
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

    //Mat res_lin = Mat::zeros(pic->rows, pic->cols, CV_8U);
    //bwselect(&non_ms_hth, &non_ms, &res_lin);

    Mat res_lin = cbwselect(non_ms_hth, non_ms);

    //res_lin.convertTo(res_lin, CV_8U);

    return res_lin;
}

static void mum(Mat *pic, Mat *result, int fak)
{

    int fak_ges = fak + 1;
    int sz_x = pic->cols / fak_ges;
    int sz_y = pic->rows / fak_ges;

    *result = Mat::zeros(sz_y, sz_x, CV_8U);

    int hist[256];
    int mean = 0;
    int cnt = 0;
    int mean_2 = 0;

    int idx = 0;
    int idy = 0;

    for (int i = 0; i < sz_y; i++)
    {
        idy += fak_ges;

        for (int j = 0; j < sz_x; j++)
        {
            idx += fak_ges;

            for (int k = 0; k < 256; k++)
                hist[k] = 0;

            mean = 0;
            cnt = 0;

            for (int ii = -fak; ii <= fak; ii++)
                for (int jj = -fak; jj <= fak; jj++)
                {

                    if (idy + ii > 0 && idy + ii < pic->rows && idx + jj > 0 && idx + jj < pic->cols)
                    {
                        if ((unsigned int)pic->data[(pic->cols * (idy + ii)) + (idx + jj)] > 255)
                            pic->data[(pic->cols * (idy + ii)) + (idx + jj)] = 255;

                        hist[pic->data[(pic->cols * (idy + ii)) + (idx + jj)]]++;
                        cnt++;
                        mean += pic->data[(pic->cols * (idy + ii)) + (idx + jj)];
                    }
                }
            mean = mean / cnt;

            mean_2 = 0;
            cnt = 0;
            for (int ii = 0; ii <= mean; ii++)
            {
                mean_2 += ii * hist[ii];
                cnt += hist[ii];
            }

            if (cnt == 0)
                mean_2 = mean;
            else
                mean_2 = mean_2 / cnt;

            result->data[(sz_x * (i)) + (j)] = mean_2;
        }
        idx = 0;
    }
}

static void gen_blob_neu(int rad, Mat *all_mat, Mat *all_mat_neg)
{

    int len = 1 + (4 * rad);
    int c0 = rad * 2;
    float negis = 0;
    float posis = 0;

    *all_mat = Mat::zeros(len, len, CV_32FC1);
    *all_mat_neg = Mat::zeros(len, len, CV_32FC1);

    float *p, *p_neg;
    for (int i = -rad * 2; i <= rad * 2; i++)
    { //height
        p = all_mat->ptr<float>(c0 + i);

        for (int j = -rad * 2; j <= rad * 2; j++)
        {

            if (i < -rad || i > rad)
            { //pos
                p[c0 + j] = 1;
                posis++;
            }
            else
            { //neg

                int sz_w = (int)sqrt(float(rad * rad) - float(i * i));

                if (abs(j) <= sz_w)
                {
                    p[c0 + j] = -1;
                    negis++;
                }
                else
                {
                    p[c0 + j] = 1;
                    posis++;
                }
            }
        }
    }

    for (int i = 0; i < len; i++)
    {
        p = all_mat->ptr<float>(i);
        p_neg = all_mat_neg->ptr<float>(i);

        for (int j = 0; j < len; j++)
        {

            if (p[j] > 0)
            {
                p[j] = (int)1.0 / posis;
                p_neg[j] = 0.0;
            }
            else
            {
                p[j] = (int)-1.0 / negis;
                p_neg[j] = (int)1.0 / negis;
            }
        }
    }
}

static bool is_good_ellipse_evaluation(RotatedRect *ellipse, Mat *pic)
{

    if (ellipse->center.x == 0 && ellipse->center.y == 0)
        return false;

    float x0 = ellipse->center.x;
    float y0 = ellipse->center.y;

    int st_x = (int)ceil(x0 - (ellipse->size.width / 4.0));
    int st_y = (int)ceil(y0 - (ellipse->size.height / 4.0));
    int en_x = (int)floor(x0 + (ellipse->size.width / 4.0));
    int en_y = (int)floor(y0 + (ellipse->size.height / 4.0));

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

    st_x = (int)ceil(x0 - (ellipse->size.width * 0.75));
    st_y = (int)ceil(y0 - (ellipse->size.height * 0.75));
    en_x = (int)floor(x0 + (ellipse->size.width * 0.75));
    en_y = (int)floor(y0 + (ellipse->size.height * 0.75));

    int in_st_x = (int)ceil(x0 - (ellipse->size.width / 2));
    int in_st_y = (int)ceil(y0 - (ellipse->size.height / 2));
    int in_en_x = (int)floor(x0 + (ellipse->size.width / 2));
    int in_en_y = (int)floor(y0 + (ellipse->size.height / 2));

    for (int i = st_x; i < en_x; i++)
        for (int j = st_y; j < en_y; j++)
        {
            if (!(i >= in_st_x && i <= in_en_x && j >= in_st_y && j <= in_en_y))
                if (i > 0 && i < pic->cols && j > 0 && j < pic->rows)
                {
                    ext_val += pic->data[(pic->cols * j) + i];
                    val_cnt++;
                }
        }

    if (val_cnt > 0)
        ext_val = ext_val / val_cnt;
    else
        return false;

    val = ext_val - val;

    if (val > 10)
        return true;
    else
        return false;
}

static RotatedRect blob_finder(Mat *pic)
{

    Point pos(0, 0);
    float abs_max = 0;

    float *p_erg;
    Mat blob_mat, blob_mat_neg;

    int fak_mum = 5;
    int fakk = pic->cols > pic->rows ? (pic->cols / 100) + 1 : (pic->rows / 100) + 1;

    Mat img;
    mum(pic, &img, fak_mum);
    Mat erg = Mat::zeros(img.rows, img.cols, CV_32FC1);

    Mat result, result_neg;

    gen_blob_neu(fakk, &blob_mat, &blob_mat_neg);

    img.convertTo(img, CV_32FC1);
    filter2D(img, result, -1, blob_mat, Point(-1, -1), 0, BORDER_REPLICATE);

    float *p_res, *p_neg_res;
    for (int i = 0; i < result.rows; i++)
    {
        p_res = result.ptr<float>(i);

        for (int j = 0; j < result.cols; j++)
        {
            if (p_res[j] < 0)
                p_res[j] = 0;
        }
    }

    filter2D(img, result_neg, -1, blob_mat_neg, Point(-1, -1), 0, BORDER_REPLICATE);

    for (int i = 0; i < result.rows; i++)
    {
        p_res = result.ptr<float>(i);
        p_neg_res = result_neg.ptr<float>(i);
        p_erg = erg.ptr<float>(i);

        for (int j = 0; j < result.cols; j++)
        {
            p_neg_res[j] = (255.0f - p_neg_res[j]);
            p_erg[j] = (p_neg_res[j]) * (p_res[j]);
        }
    }

    for (int i = 0; i < erg.rows; i++)
    {
        p_erg = erg.ptr<float>(i);

        for (int j = 0; j < erg.cols; j++)
        {
            if (abs_max < p_erg[j])
            {
                abs_max = p_erg[j];

                pos.x = (fak_mum + 1) + (j * (fak_mum + 1));
                pos.y = (fak_mum + 1) + (i * (fak_mum + 1));
            }
        }
    }

    if (pos.y > 0 && pos.y < pic->rows && pos.x > 0 && pos.x < pic->cols)
    {

        //calc th
        int opti_x = 0;
        int opti_y = 0;

        float mm = 0;
        float cnt = 0;
        for (int i = -(2); i < (2); i++)
        {
            for (int j = -(2); j < (2); j++)
            {
                if (pos.y + i > 0 && pos.y + i < pic->rows && pos.x + j > 0 && pos.x + j < pic->cols)
                {
                    mm += pic->data[(pic->cols * (pos.y + i)) + (pos.x + j)];
                    cnt++;
                }
            }
        }

        if (cnt > 0)
            mm = ceil(mm / cnt);

        int th_bot = 0;
        if (pos.y > 0 && pos.y < pic->rows && pos.x > 0 && pos.x < pic->cols)
            th_bot = (int)(pic->data[(pic->cols * (pos.y)) + (pos.x)] + abs(mm - pic->data[(pic->cols * (pos.y)) + (pos.x)]));
        cnt = 0;

        for (int i = -(fak_mum * fak_mum); i < (fak_mum * fak_mum); i++)
        {
            for (int j = -(fak_mum * fak_mum); j < (fak_mum * fak_mum); j++)
            {

                if (pos.y + i > 0 && pos.y + i < pic->rows && pos.x + j > 0 && pos.x + j < pic->cols)
                {

                    if (pic->data[(pic->cols * (pos.y + i)) + (pos.x + j)] <= th_bot)
                    {
                        opti_x += pos.x + j;
                        opti_y += pos.y + i;
                        cnt++;
                    }
                }
            }
        }

        if (cnt > 0)
        {
            opti_x = (int)(opti_x / cnt);
            opti_y = (int)(opti_y / cnt);
        }
        else
        {
            opti_x = pos.x;
            opti_y = pos.y;
        }

        pos.x = opti_x;
        pos.y = opti_y;
    }

    RotatedRect ellipse;

    if (pos.y > 0 && pos.y < pic->rows && pos.x > 0 && pos.x < pic->cols)
    {
        ellipse.center.x = (float)pos.x;
        ellipse.center.y = (float)pos.y;
        ellipse.angle = 0.0;
        ellipse.size.height = (float)((fak_mum * fak_mum * 2) + 1);
        ellipse.size.width = (float)((fak_mum * fak_mum * 2) + 1);

        if (!is_good_ellipse_evaluation(&ellipse, pic))
        {
            ellipse.center.x = 0;
            ellipse.center.y = 0;
            ellipse.angle = 0;
            ellipse.size.height = 0;
            ellipse.size.width = 0;
        }
    }
    else
    {
        ellipse.center.x = 0;
        ellipse.center.y = 0;
        ellipse.angle = 0;
        ellipse.size.height = 0;
        ellipse.size.width = 0;
    }

    return ellipse;
}

Pupil ElSe::run(const Mat &frame)
{

    RotatedRect ellipse;
    Point pos(0, 0);

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

    Mat pic;
    normalize(downscaled, pic, 0, 255, NORM_MINMAX, CV_8U);

    minArea = downscaled.cols * downscaled.rows * minAreaRatio;
    maxArea = downscaled.cols * downscaled.rows * maxAreaRatio;

    double border = 0.0; // ER takes care of setting an ROI
    double mean_dist = 3;
    int inner_color_range = 0;

    int start_x = (int)floor(double(pic.cols) * border);
    int start_y = (int)floor(double(pic.rows) * border);

    int end_x = pic.cols - start_x;
    int end_y = pic.rows - start_y;

    Mat picpic = Mat::zeros(end_y - start_y, end_x - start_x, CV_8U);
    Mat magni;

    for (int i = 0; i < picpic.cols; i++)
    {
        for (int j = 0; j < picpic.rows; j++)
        {
            picpic.data[(picpic.cols * j) + i] = pic.data[(pic.cols * (start_y + j)) + (start_x + i)];
        }
    }

    Mat detected_edges2 = canny_impl(&picpic, &magni);

    Mat detected_edges = Mat::zeros(pic.rows, pic.cols, CV_8U);
    for (int i = 0; i < detected_edges2.cols; i++)
        for (int j = 0; j < detected_edges2.rows; j++)
        {
            detected_edges.data[(detected_edges.cols * (start_y + j)) + (start_x + i)] = detected_edges2.data[(detected_edges2.cols * j) + i];
        }

    //cv::imwrite( "edge_image.jpg", detected_edges);

    filter_edges(&detected_edges, start_x, end_x, start_y, end_y);

    //cv::imwrite( "filtered_edge_image.jpg", detected_edges );

    ellipse = find_best_edge(&pic, &detected_edges, &magni, start_x, end_x, start_y, end_y, mean_dist, inner_color_range);

    if ((ellipse.center.x <= 0 && ellipse.center.y <= 0) || ellipse.center.x >= pic.cols || ellipse.center.y >= pic.rows)
    {

        ellipse = blob_finder(&pic);
        ellipse.angle = 0;
        ellipse.size = Size(0, 0);
    }

    cv::RotatedRect scaledEllipse(cv::Point2f(ellipse.center.x / scalingRatio, ellipse.center.y / scalingRatio), cv::Size2f(ellipse.size.width / scalingRatio, ellipse.size.height / scalingRatio), ellipse.angle);

    return Pupil(scaledEllipse);
}

void ElSe::run(const cv::Mat &frame, const cv::Rect &roi, Pupil &pupil, const float &minPupilDiameterPx = -1, const float &maxPupilDiameterPx = -1)
{

    if (roi.area() < 10)
    {
        std::cout << "Bad ROI: falling back to regular detection.";
        PupilDetectionMethod::run(frame, pupil);
        return;
    }

    if (minPupilDiameterPx > 0 && maxPupilDiameterPx > 0)
    {
        minArea = pow(minPupilDiameterPx, 2);
        maxArea = pow(maxPupilDiameterPx, 2);
    }
    else
    {
        minArea = frame.cols * frame.rows * 0.005;
        maxArea = frame.cols * frame.rows * 0.2;
    }

    pupil = run(frame(roi));
    if (pupil.center.x > 0 && pupil.center.y > 0)
        pupil.shift(roi.tl());
}
