// Class for loading the data required for descibing a Fields of Experts (FoE)
// model. The Fields of Experts regularization consists of terms of the type
//
//   alpha * log(1 + (1/2)*sum(F .* X)^2),
//
// where F is a d-by-d image patch and alpha is a constant.
//
// [1] S. Roth and M.J. Black. “Fields of Experts.” International Journal of
//     Computer Vision, 82(2):205–229, 2009.
//
// Reads the file format defined by Ceres Solver – A fast non-linear least
// squares minimizer.

#ifndef FIELDS_OF_EXPERTS_H
#define FIELDS_OF_EXPERTS_H

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <spii/term.h>

#include "pgm_image.h"

// This class loads a set of filters and coefficients from file and
// then creates the correct Terms.
class FieldsOfExperts {
public:

	FieldsOfExperts(const std::string& filename);

	// Side length of a square filter in this FoE. They are all of the same size.
	int size() const
	{
		return size_;
	}

	// Total number of pixels the filter covers.
	int num_variables() const
	{
		return size_ * size_;
	}

	// Number of filters used by the FoE.
	int num_filters() const
	{
		return num_filters_;
	}

	// Creates a new cost function. The caller is responsible for deallocating the
	// memory. alpha_index specifies which filter is used in the cost function.
	std::shared_ptr<spii::Term> new_term(int alpha_index) const;

	// Gets the delta pixel indices for all pixels in a patch.
	const std::vector<int>& get_x_delta_indices() const
	{
		return x_delta_indices_;
	}

	const std::vector<int>& get_y_delta_indices() const
	{
		return y_delta_indices_;
	}

private:
	// The side length of a square filter.
	int size_;
	// The number of different filters used.
	int num_filters_;
	// Pixel offsets for all variables.
	std::vector<int> x_delta_indices_, y_delta_indices_;
	// The coefficients in front of each term.
	std::vector<double> alpha_;
	// The filters used for the dot product with image patches.
	std::vector<std::vector<double> > filters_;
};

class FoETerm
	: public spii::Term
{
public:
	FoETerm(double alpha_, const std::vector<double>& filter_)
	: alpha(alpha_), filter(filter_)
	{ }

	virtual int number_of_variables() const override
	{
		return int(filter.size());
	}

	virtual int variable_dimension(int var) const override
	{
		return 1;
	}

	virtual double evaluate(double * const * const variables) const
	{
		double dot_product = 0;
		for (size_t i = 0; i < filter.size(); ++i) {
			dot_product += variables[i][0] * filter[i];
		}

		return alpha * log(1 + 0.5 * dot_product * dot_product);
	}

	virtual double evaluate(double * const * const variables,
	                        std::vector<Eigen::VectorXd>* gradient) const override
	{
		double dot_product = 0;
		for (size_t i = 0; i < filter.size(); ++i) {
			dot_product += variables[i][0] * filter[i];
		}

		auto inside_log = 1 + 0.5 * dot_product * dot_product;
		auto alpha_inv_inside_log = alpha * 1.0 / inside_log;
		auto alpha_inv_inside_log_dot_product = alpha_inv_inside_log * dot_product;
		for (size_t i = 0; i < filter.size(); ++i) {
			(*gradient)[i](0) = alpha_inv_inside_log_dot_product * filter[i];
		}

		return alpha * std::log(inside_log);
	}

	virtual double evaluate(double * const * const variables,
	                        std::vector<Eigen::VectorXd>* gradient,
	                        std::vector< std::vector<Eigen::MatrixXd> >* hessian) const
	{
		double dot_product = 0;
		for (size_t i = 0; i < filter.size(); ++i) {
			dot_product += variables[i][0] * filter[i];
		}
		auto inside_log = 1 + 0.5 * dot_product * dot_product;
		auto dot_product2 = dot_product * dot_product;

		for (size_t i = 0; i < filter.size(); ++i) {
			auto num   =  2.0 * alpha * filter[i] * dot_product;
			auto denom =  dot_product2 + 2.0;
			(*gradient)[i](0) = num / denom;
		}

		auto expr = dot_product2 + 2.0;
		auto expr2 = expr * expr;
		for (size_t i = 0; i < filter.size(); ++i) {
			for (size_t j = 0; j < filter.size(); ++j) {
				auto first   = 2.0 * alpha * filter[i] * filter[j] / expr;
				auto second  = 4.0 * alpha * filter[i] * filter[j] * dot_product2 / expr2;
				(*hessian)[i][j](0,0) = first - second;
			}
		}

		return alpha * std::log(inside_log);
	}

private:
	double alpha;
	const std::vector<double>& filter;
};

FieldsOfExperts::FieldsOfExperts(const std::string& filename)
	:  size_{-1}, num_filters_{-1}
{ 
	std::ifstream foe_file(filename.c_str());
	foe_file >> size_;
	foe_file >> num_filters_;
	spii_assert(size_ >= 0);
	spii_assert(num_filters_ >= 0);

	x_delta_indices_.resize(num_variables());
	for (int i = 0; i < num_variables(); ++i) {
		foe_file >> x_delta_indices_[i];
	}

	y_delta_indices_.resize(num_variables());
	for (int i = 0; i < num_variables(); ++i) {
		foe_file >> y_delta_indices_[i];
	}

	alpha_.resize(num_filters_);
	for (int i = 0; i < num_filters_; ++i) {
		foe_file >> alpha_[i];
		// 0.5 to agree with Ceres.
		alpha_[i] *= 0.5;
	}

	filters_.resize(num_filters_);
	for (int i = 0; i < num_filters_; ++i) {
		filters_[i].resize(num_variables());
		for (int j = 0; j < num_variables(); ++j) {
			foe_file >> filters_[i][j];
		}
	}

	spii_assert(bool(foe_file));

	// There cannot be anything else in the file. Try reading another number and
	// return failure if that succeeded.
	double temp;
	foe_file >> temp;
	spii_assert(!foe_file);
}

std::shared_ptr<spii::Term> FieldsOfExperts::new_term(int alpha_index) const
{
	return std::make_shared<FoETerm>(alpha_[alpha_index], filters_[alpha_index]);
}

#endif
