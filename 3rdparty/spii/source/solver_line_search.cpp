// Petter Strandmark 2012–2013.
#include <array>
#include <limits>

#include <Eigen/Dense>

#include <spii/solver.h>

namespace spii {

namespace
{
	// constexpr when supported.
	const double nan = std::numeric_limits<double>::quiet_NaN();
}

double
polynomial_interpolation(std::array<double, 2> x,
                         std::array<double, 2> f,
                         std::array<double, 2> df,
                         double xmin = nan, double xmax = nan)
{
	int minpos = 0;
	if (x[1] < x[0]) {
		minpos = 1;
	}
	int maxpos = 1 - minpos;

	if (xmin != xmin) {
		xmin = x[minpos];
	}
	if (xmax != xmax) {
		xmax = x[maxpos];
	}

	auto d1 = df[minpos] + df[maxpos] - 3*(f[minpos] - f[maxpos]) / (x[minpos] - x[maxpos]);
	auto inside_sqrt = d1*d1 - df[0]*df[1];
	if (inside_sqrt >= 0) {
		auto d2 = std::sqrt(inside_sqrt);
		auto t = x[maxpos] - (x[maxpos] - x[minpos])*((df[maxpos] + d2 - d1)/(df[maxpos] - df[minpos] + 2*d2));
		return std::min(std::max(t, xmin), xmax);
	}
	else {
		return (xmin + xmax) / 2;
	}
}

double perform_Wolfe_linesearch(const Solver& solver,
                                const Function& function,
                                const Eigen::VectorXd& x,
                                const double fval,
                                const Eigen::VectorXd& g,
                                const Eigen::VectorXd& p,
                                Eigen::VectorXd* scratch,
                                const double start_alpha)
{
	Eigen::VectorXd g_prev = g;
	auto f = function.evaluate(x);
	auto f_prev = f;
	double gtp = g.dot(p);
	auto gtp_prev = gtp;

	auto n = x.size();
	Eigen::VectorXd g_new(n);  // TODO: Somehow use scratch space.

	double alpha = start_alpha;
	double alpha_prev = 0;

	*scratch = x + alpha * p;
	double f_new = function.evaluate(*scratch, &g_new);
	double gtp_new  = g_new.dot(p);

	//
	auto c1 = solver.line_search_c;
	auto c2 = solver.line_search_c2;
	//

	std::array<double, 2> bracket;
	std::array<double, 2> bracket_fval;
	std::array<double, 2> bracket_gTpval;
	bool done = false;

	//
	// Bracketing phase.
	//
	int iterations = 0;
	const int max_iterations = 30;
	while (iterations <= max_iterations) {

		if (f_new > f + c1 * alpha * gtp || (iterations > 1 && f_new >= f_prev)) {
			// Double braces for GCC 4.7 compatibility. Remove later.
			bracket        = {{alpha_prev, alpha}};
			bracket_fval   = {{f_prev, f_new}};
			bracket_gTpval = {{ g_prev.dot(p), g_new.dot(p)}};
			break;
		}
		else if (std::abs(gtp_new) <= -c2 * gtp) {
			// We are done.
			return alpha;
		}
		else if (gtp_new >= 0) {
			// Double braces for GCC 4.7 compatibility. Remove later.
			bracket        = {{alpha_prev, alpha}};
			bracket_fval   = {{f_prev, f_new}};
			bracket_gTpval = {{g_prev.dot(p), g_new.dot(p)}};
			break;
		}

		double temp = alpha_prev;
		alpha_prev = alpha;
		double minStep = alpha + 0.01*(alpha - temp);
		double maxStep = 10 * alpha;

		if (solver.wolfe_interpolation_strategy == Solver::BISECTION) {
			alpha = maxStep;
		}
		else {
			// Double braces for GCC 4.7 compatibility. Remove later.
			alpha = polynomial_interpolation({{temp, alpha}},
			                                 {{f_prev, f_new}},
			                                 {{gtp_prev, gtp_new}},
			                                 minStep, maxStep);
		}

		f_prev = f_new;
		g_prev = g_new;
		gtp_prev = gtp_new;

		*scratch = x + alpha * p;
		f_new = function.evaluate(*scratch, &g_new);
		gtp_new  = g_new.dot(p);

		iterations++;
	}

	//
	// Zoom phase.
	//
	bool insufficient_progress = false;
	while (!done && iterations <= max_iterations) {

		// Compute new trial value.
		if (solver.wolfe_interpolation_strategy == Solver::BISECTION) {
			alpha = (bracket[0] + bracket[1]) / 2.0;
		}
		else {
			alpha = polynomial_interpolation(bracket,
			                                 bracket_fval,
			                                 bracket_gTpval);
		}	 

		auto max_bracket = std::max({bracket[0], bracket[1]});
		auto min_bracket = std::min({bracket[0], bracket[1]});

		if (std::min({max_bracket-alpha, alpha-min_bracket}) / (max_bracket - min_bracket) < 0.1) {
			if (insufficient_progress || alpha >= max_bracket || alpha <= min_bracket) {
				if (std::abs(alpha - max_bracket) < std::abs(alpha - min_bracket)) {
					alpha = max_bracket - 0.1*(max_bracket - min_bracket);
				}
				else {
					alpha = min_bracket + 0.1*(max_bracket - min_bracket);
				}
				insufficient_progress = false;
			}
			else {
				insufficient_progress = true;
			}
		}

		*scratch = x + alpha * p;
		f_new = function.evaluate(*scratch, &g_new);
		gtp_new  = g_new.dot(p);

		int lo_pos, hi_pos;
		double f_low;
		if (bracket_fval[0] < bracket_fval[1]) {
			f_low = bracket_fval[0];
			lo_pos = 0;
			hi_pos = 1;
		}
		else {
			f_low = bracket_fval[1];
			lo_pos = 1;
			hi_pos = 0;
		}

		bool armijo = f_new < f + c1 * alpha * gtp;

		if (!armijo || f_new >= f_low) {
			// Armijo condition not satisfied or not lower than lowest
			// point
			bracket[hi_pos]        = alpha;
			bracket_fval[hi_pos]   = f_new;
			bracket_gTpval[hi_pos] = g_new.dot(p);
		}
		else {
			if (std::abs(gtp_new) <= - c2*gtp) {
				// Wolfe conditions satisfied
				done = true;
			}
			else if (gtp_new * (bracket[hi_pos] - bracket[lo_pos]) >= 0) {
				// Old HI becomes new LO
				bracket[hi_pos]        = bracket[lo_pos];
				bracket_fval[hi_pos]   = bracket_fval[lo_pos];
				bracket_gTpval[hi_pos] = bracket_gTpval[lo_pos];
			}

			// New point becomes new LO
			bracket[lo_pos]        = alpha;
			bracket_fval[lo_pos]   = f_new;
			bracket_gTpval[lo_pos] = g_new.dot(p);
		}

		iterations++;
	}
	
	if (!done) {
		if (solver.log_function) {
			solver.log_function("Wolfe line search maximum iterations exceeded.");
		}
		return 0.0;
	}

	if (bracket_fval[0] < bracket_fval[1]) {
		alpha = bracket[0];
	}
	else {
		alpha = bracket[1];
	}

	return alpha;
}

double perform_Armijo_linesearch(const Solver& solver,
                                 const Function& function,
                                 const Eigen::VectorXd& x,
                                 const double fval,
                                 const Eigen::VectorXd& g,
                                 const Eigen::VectorXd& p,
                                 Eigen::VectorXd* scratch,
                                 const double start_alpha)
{
	//
	// Perform back-tracking line search.
	//

	// Starting value for alpha during line search. Newton and
	// quasi-Newton methods should choose 1.0.
	double alpha = start_alpha;
	double rho = solver.line_search_rho;
	double c = solver.line_search_c;
	double gTp = g.dot(p);
	if (gTp != gTp) {
		if (solver.log_function) {
			solver.log_function("Backtracking encountered NaN, returning zero step.");
		}
		return 0.0;
	}

	int backtracking_attempts = 0;
	while (true) {
		*scratch = x + alpha * p;
		double lhs = function.evaluate(*scratch);
		double rhs = fval + c * alpha * gTp;
		if (lhs <= rhs) {
			break;
		}
		alpha *= rho;

		backtracking_attempts++;
		if (backtracking_attempts > 1000) {
			if (solver.log_function) {
				solver.log_function("Backtracking failed, returning zero step.");
			}
			return 0.0;
		}
	}

	return alpha;
}

double Solver::perform_linesearch(const Function& function,
                                  const Eigen::VectorXd& x,
                                  const double fval,
                                  const Eigen::VectorXd& g,
                                  const Eigen::VectorXd& p,
                                  Eigen::VectorXd* scratch,
                                  const double start_alpha) const
{
	if (this->line_search_type == ARMIJO) {
		return perform_Armijo_linesearch(*this, function, x, fval, g, p, scratch, start_alpha);
	}
	else {
		return perform_Wolfe_linesearch(*this, function, x, fval, g, p, scratch, start_alpha);
	}
}

}  // namespace spii
