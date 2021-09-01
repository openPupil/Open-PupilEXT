// Denoising using Fields of Experts [1]. This example problem also comes
// with Ceres Solver, a solver for nonlinear least squares problems and
// can be used for comparing the two solvers.
//
// [1] S. Roth and M.J. Black. “Fields of Experts.” International Journal of
//     Computer Vision, 82(2):205–229, 2009.

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>
#include <sstream>
#include <string>

#include <spii/color.h>
#include <spii/solver.h>

// pgm_image uses Google Glog, specifically the
// CHECK macro.
#define CHECK(arg) spii_assert(arg)
#include "pgm_image.h"
using ceres::examples::PGMImage;

#include "fields_of_experts.h"

const double sigma = 20.0;

// This Term is used to build the data term.
//
//   f_i(x) = a · (x_i - b)^2
//
class QuadraticTerm
	: public spii::SizedTerm<1>
{
public:
	QuadraticTerm(double a_, double b_)
		  // 0.5 to agree with Ceres.
		: a{0.5 * a_}, b{b_}
	{ }

	double evaluate(double * const * const x) const override
	{
		return a * (x[0][0] - b) * (x[0][0] - b);
	}

	double evaluate(double * const * const x,
	                std::vector<Eigen::VectorXd>* gradient) const override
	{
		(*gradient)[0][0] = 2.0 * a * x[0][0] - 2.0 * a * b;
		return evaluate(x);
	}

	double evaluate(double * const * const x,
	                std::vector<Eigen::VectorXd>* gradient,
	                std::vector< std::vector<Eigen::MatrixXd> >* hessian) const override
	{
		(*hessian)[0][0](0, 0) = 2.0 * a;
		return evaluate(x, gradient);
	}

private:
	double a, b;
};

// Creates a Fields of Experts MAP function.
void create_functions(const FieldsOfExperts& foe,
                      const PGMImage<double>& image,
                      spii::Function* function,
                      PGMImage<double>* solution)
{
	// Create the data term
	const double coefficient = 1 / (2.0 * sigma * sigma);
	for (int index = 0; index < image.NumPixels(); ++index)
	{
		auto term =
			std::make_shared<QuadraticTerm>(coefficient,
			                                image.PixelFromLinearIndex(index));
		function->add_term(term, solution->MutablePixelFromLinearIndex(index));
	}

	// Create terms for regularization. One is needed for each filter.
	std::vector<std::shared_ptr<spii::Term>> foe_terms(foe.num_filters());
	for (int alpha_index = 0; alpha_index < foe.num_filters(); ++alpha_index) {
		foe_terms[alpha_index] = foe.new_term(alpha_index);
	}

	// Add FoE regularization for each patch in the image.
	for (int x = 0; x < image.width() - (foe.size() - 1); ++x) {
		for (int y = 0; y < image.height() - (foe.size() - 1); ++y) {
			// Build a vector with the pixel indices of this patch.
			std::vector<double*> pixels;
			const std::vector<int>& x_delta_indices = foe.get_x_delta_indices();
			const std::vector<int>& y_delta_indices = foe.get_y_delta_indices();
			for (int i = 0; i < foe.num_variables(); ++i) {
				double* pixel = solution->MutablePixel(x + x_delta_indices[i],
				                                       y + y_delta_indices[i]);
				pixels.push_back(pixel);
			}
			// For this patch with coordinates (x, y), we will add foe.NumFilters()
			// terms to the objective function.
			for (int alpha_index = 0; alpha_index < foe.num_filters(); ++alpha_index) {
				function->add_term(foe_terms[alpha_index],
				                   pixels);
			}
		}
	}
}

void minimize_function(const spii::Function& function, PGMImage<double>* solution)
{
	spii::LBFGSSolver solver;
	solver.function_improvement_tolerance = 1e-3;

	solver.maximum_iterations = 1000;

	int iteration = 0;
	solver.callback_function =
		[&](const spii::CallbackInformation& info) -> bool
		{
			if (++iteration % 10 == 0) {
			// Uncomment to save solution every 10 iterations.
				//function.copy_global_to_user(*info.x);
				//std::stringstream sout;
				//sout << "output-" << iteration << ".pgm";
				//spii::check(solution->WriteToFile(sout.str()), "Writing failed.");
			}
			return true;
		};

	solver.log_function = 
		[](const std::string& str)
		{
			std::cout << str << std::endl;
		};

	spii::SolverResults results;
	solver.solve(function, &results);

	function.print_timing_information(std::cout);
    std::cout << results << std::endl;

	// Make the solution stay in [0, 255].
	for (int x = 0; x < solution->width(); ++x) {
		for (int y = 0; y < solution->height(); ++y) {
			*solution->MutablePixel(x, y) =
				std::min(255.0, std::max(0.0, solution->Pixel(x, y)));
		}
	}
}

int main_function(int argc, char** argv)
{
	using namespace spii;
	using namespace std;

	auto foe_file_name = "2x2.foe";
	if (argc > 1) {
		foe_file_name = argv[1];
	}

	auto input_file_name = "ceres_noisy.pgm";
	if (argc > 2) {
		input_file_name = argv[2];
	}

	// Load the Fields of Experts filters from file.
	Timer foe_timer("Loading FoE file");
		FieldsOfExperts foe(foe_file_name);
	foe_timer.OK();

	// Read the images
	Timer image_timer("Loading images");
		PGMImage<double> image(input_file_name);
		check(image.width() > 0, "Reading input image failed.");
		PGMImage<double> solution(image.width(), image.height());
		solution.Set(0.0);
		solution += image;
	image_timer.OK();

	Timer function_timer("Creating function");
		spii::Function function;
		function.hessian_is_enabled = false;
		create_functions(foe, image, &function, &solution);
	function_timer.OK();

	cout << "Number of variables  : " << function.get_number_of_variables() << endl;
	cout << "Number of terms      : " << function.get_number_of_terms() << endl;
	cout << "Function value       : " << function.evaluate() << endl;

	minimize_function(function, &solution);

	cout << "Final function value : " << function.evaluate() << endl;

	Timer save_timer("Saving image");
		check(solution.WriteToFile("output-final.pgm"), "Writing failed.");
	save_timer.OK();

	return 0;
}

int main(int argc, char** argv)
{
	try {
		return main_function(argc, argv);
	}
	catch (std::exception& e) {
		std::cerr << "\nException: " << e.what() << std::endl;
	}
}
