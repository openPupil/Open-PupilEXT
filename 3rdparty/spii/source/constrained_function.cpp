// Petter Strandmark 2013.
//
// Constrained minimization.
//
// See Nocedal & Wright, chapter 17.

#include <cstdio>
#include <iomanip>

#include <spii/constrained_function.h>
#include <spii/solver.h>

using namespace std;

namespace spii
{

class Constraint
	: protected Function
{
public:
	bool is_equality = false;
	double lambda = 0;

	using Function::add_term;

	double evaluate()
	{
		c_x = Function::evaluate();
		return c_x;
	}

	double get_cached_value() const
	{
		return c_x;
	}

	bool is_feasible(double tolerance) const
	{
		if (is_equality) {
			return std::abs(c_x) <= tolerance;
		}
		else {
			return c_x <= tolerance;
		}
	}

	double infeasibility() const
	{
		return c_x * lambda;
	}

	double violation() const
	{
		if (is_equality) {
			return std::abs(c_x);
		}
		else {
			return max(0.0, c_x);
		}
	}
	
	void update_lambda(double mu)
	{
		if (is_equality) {
			lambda = lambda - mu * c_x;
		}
		else {
			if (c_x + lambda / mu <= 0) {
				lambda = 0;
			}
			else {
				lambda = lambda + mu * c_x;
			}
		}
	}

private:
	double c_x = 0;
};

// Phi wrapper function. 
//
// Nocedal & Wright p. 524
// Equation 17.65.
//
// But this implementation uses c(x) ≤ 0 instead
// of c(x) ≥ 0 as in Nocedal & Wright.
class Phi
	: public Term
{
public:
	Phi(std::shared_ptr<const Term> term_, double* sigma_, double* mu_)
		: term(term_), sigma(sigma_), mu(mu_)
	{
	}

	int number_of_variables() const override
	{
		return term->number_of_variables();
	}

	int variable_dimension(int var) const override
	{
		return term->variable_dimension(var);
	}

	double evaluate(double * const * const variables) const override
	{
		double t = term->evaluate(variables);
		if (- t - *sigma / *mu <= 0) {
			return *sigma * t + (*mu / 2) * t*t; 
		}
		else {
			return - 1.0 / (2.0 * *mu) * (*sigma)*(*sigma);
		}
	}

	double evaluate(double * const * const variables,
	                std::vector<Eigen::VectorXd>* gradient) const override
	{
		double t = term->evaluate(variables, gradient);

		if (- t - *sigma / *mu <= 0) {
			for (int i = 0; i < number_of_variables(); ++i) {
				for (int j = 0; j < variable_dimension(i); ++j) {
					(*gradient)[i][j] = *sigma * (*gradient)[i][j] 
					                    + (*mu) * t * (*gradient)[i][j];
				}
			}
			return *sigma * t + (*mu / 2) * t*t; 
		}
		else {
			for (int i = 0; i < number_of_variables(); ++i) {
				for (int j = 0; j < variable_dimension(i); ++j) {
					(*gradient)[i][j] = 0;
				}
			}
			return - 1.0 / (2.0 * *mu) * (*sigma)*(*sigma);
		}
	}

	double evaluate(double * const * const variables,
	                std::vector<Eigen::VectorXd>* gradient,
	                std::vector< std::vector<Eigen::MatrixXd> >* hessian) const override
	{
		check(false, "Phi: hessian not supported.");
		return 0;
	}

private:
	const shared_ptr<const Term> term;
	double* const sigma;
	double* const mu;
};

// Theta wrapper function. 
//
// As Phi, but for constraints c(x) = 0.
//
class Theta
	: public Term
{
public:
	Theta(std::shared_ptr<const Term> term_, double* sigma_, double* mu_)
		: term(term_), sigma(sigma_), mu(mu_)
	{
	}

	int number_of_variables() const override
	{
		return term->number_of_variables();
	}

	int variable_dimension(int var) const override
	{
		return term->variable_dimension(var);
	}

	double evaluate(double * const * const variables) const override
	{
		double t = term->evaluate(variables);
		return - *sigma * t + (*mu / 2) * t*t; 
	}

	double evaluate(double * const * const variables,
	                std::vector<Eigen::VectorXd>* gradient) const override
	{
		double t = term->evaluate(variables, gradient);
		for (int i = 0; i < number_of_variables(); ++i) {
			for (int j = 0; j < variable_dimension(i); ++j) {
				(*gradient)[i][j] = - *sigma * (*gradient)[i][j] 
					                + (*mu) * t * (*gradient)[i][j];
			}
		}
		return - *sigma * t + (*mu / 2) * t*t; 
	}

	double evaluate(double * const * const variables,
	                std::vector<Eigen::VectorXd>* gradient,
	                std::vector< std::vector<Eigen::MatrixXd> >* hessian) const override
	{
		check(false, "Theta: hessian not supported.");
		return 0;
	}

private:
	const shared_ptr<const Term> term;
	double* const sigma;
	double* const mu;
};

class ConstrainedFunction::Implementation
{
public:
	double mu;

	Function augmented_lagrangian;
	Function objective;
	map<string, Constraint> constraints;
};

ConstrainedFunction::ConstrainedFunction()
	: impl{new Implementation}
{ 
}

ConstrainedFunction::~ConstrainedFunction()
{ 
	delete impl;
}

void ConstrainedFunction::add_term(shared_ptr<const Term> term,
                                   const vector<double*>& arguments)
{
	impl->objective.add_term(term, arguments);
	impl->augmented_lagrangian.add_term(term, arguments);
}

void ConstrainedFunction::add_constraint_term(const string& constraint_name,
                                              shared_ptr<const Term> term,
                                              const vector<double*>& arguments)
{
	check(impl->constraints.find(constraint_name) == impl->constraints.end(),
	      "add_constraint_term: Term already added.");
	auto& constraint = impl->constraints[constraint_name];

	constraint.add_term(term, arguments);
	
	auto phi = make_shared<Phi>(term, &constraint.lambda, &impl->mu);
	impl->augmented_lagrangian.add_term(move(phi), arguments);
}

void ConstrainedFunction::add_equality_constraint_term(const string& constraint_name,
                                                       shared_ptr<const Term> term,
                                                       const vector<double*>& arguments)
{
	check(impl->constraints.find(constraint_name) == impl->constraints.end(),
	      "add_equality_constraint_term: Term already added.");
	auto& constraint = impl->constraints[constraint_name];

	constraint.add_term(term, arguments);
	constraint.is_equality = true;
	
	auto theta = make_shared<Theta>(term, &constraint.lambda, &impl->mu);
	impl->augmented_lagrangian.add_term(move(theta), arguments);
}

const Function& ConstrainedFunction::objective() const
{
	return impl->objective;
}

bool ConstrainedFunction::is_feasible() const
{
	for (auto& itr: impl->constraints) {
		itr.second.evaluate();
		if (!itr.second.is_feasible(this->feasibility_tolerance)) {
			return false;
		}
	}

	return true;
}

void ConstrainedFunction::solve(const Solver& solver, SolverResults* results)
{
	results->exit_condition = SolverResults::INTERNAL_ERROR;

	if (impl->augmented_lagrangian.get_number_of_variables() == 0) {
		results->exit_condition = SolverResults::FUNCTION_TOLERANCE;
		return;
	}

	auto& mu = impl->mu;
	mu = 10;
	double nu = 1.0 / std::pow(mu, 0.1);

	auto log_function = solver.log_function;

	double f_prev = numeric_limits<double>::quiet_NaN();

	int iterations = 0;
	while (true) {

		// Minimize the smooth approximation of the Lagrangian.
		solver.solve(impl->augmented_lagrangian, results);
		double f = impl->objective.evaluate();

		// Update all lambdas.
		double infeasibility = - numeric_limits<double>::infinity();
		double max_violation = 0;
		for (auto& itr: impl->constraints) {
			itr.second.evaluate();

			infeasibility = max(infeasibility, itr.second.infeasibility());
			max_violation = max(max_violation, itr.second.violation());
		}

		if (log_function) {
			char str[1024];
			std::sprintf(str,
			             " ___________________________________________________________\n"
			             "|   mu   |   nu   |   objective   |   infeas.  |  max viol. |\n"
			             "+--------+--------+---------------+------------+------------+\n"
			             "|%7.1e |%7.1e | %+10.6e | %10.3e | %10.3e |\n"
			             "|________|________|_______________|____________|____________|",
			             mu, nu, f, infeasibility, max_violation);
			log_function(str);
		}

		if (std::abs(f - f_prev) / (std::abs(f) + this->function_improvement_tolerance) <
		                                          this->function_improvement_tolerance) {
			results->exit_condition = SolverResults::FUNCTION_TOLERANCE;
			break;
		}

		if (max_violation <= nu) {
			// The maximum constraint violation is small enough.
			// Update the dual variables (explicit formula).

			auto lagrangian_before = impl->augmented_lagrangian.evaluate();

			double max_change = 0;
			double max_lambda = 0;
			double delta_lambda = 0;
			for (auto& itr: impl->constraints) {
				const auto& lambda = itr.second.lambda;

				double prev = lambda;
				itr.second.update_lambda(mu);

				max_change = std::max(max_change, std::abs(prev - lambda));
				max_lambda = std::max(max_lambda, std::abs(prev));
				delta_lambda += (prev - lambda) * (prev - lambda);
			}
			delta_lambda = std::sqrt(delta_lambda);

			auto lagrangian_after = impl->augmented_lagrangian.evaluate();

			double dL_dd = 0.0;
			if (delta_lambda > 0) {
				dL_dd = (lagrangian_after - lagrangian_before) / delta_lambda;
			}

			auto relative_change = 
				max_change / (max_lambda + this->dual_change_tolerance);

			if (log_function) {
				stringstream sout;
				sout << "Updating dual variables. "
				     << "Max (relative) change: " << max_change << " ("
				     << relative_change << ").";
				log_function(sout.str());
				log_function(to_string("Delta lambda = ", delta_lambda));
				log_function(to_string("Relative aug. lagr. change: ", dL_dd));
			}

			if (std::abs(dL_dd) < this->dual_change_tolerance
			    && max_violation < this->feasibility_tolerance) {
				results->exit_condition = SolverResults::GRADIENT_TOLERANCE;
				break;
			}

			// Nocedal & Wright.
			nu = nu / std::pow(mu, 0.9);
		}
		else {
			// The maximum constraint violation too big.
			// Update the penalty variable to decrease it.

			if (log_function) {
				log_function("Updating penalty parameter.");
			}

			// Increase penalty parameter.
			mu *= 2;

			// Nocedal & Wright.
			nu = 1.0 / std::pow(mu, 0.1);
		}

		if (log_function) {
			log_function("");
			int num_printed = 0;
			for (auto& itr: impl->constraints) {
				auto c_x = itr.second.get_cached_value();
				auto& lambda = itr.second.lambda;
				if (lambda != 0) {
					stringstream sout_name, sout;
					sout_name << "lambda[" << itr.first << "]";
					sout << left << setfill('.') << setw(25) << sout_name.str()
					     << ": " << setfill(' ') << setw(10) << lambda;
					if (c_x > 0) {
						sout << " Violation : " << c_x;
					}
					log_function(sout.str());

					if (num_printed++ >= 10) {
						log_function("Not printing more dual variables.");
						break;
					}
				}
			}
			log_function("");
		}

		iterations++;
		if (iterations >= this->max_number_of_iterations) {
			results->exit_condition = SolverResults::NO_CONVERGENCE;
			break;
		}

		// Go to next iteration.
		f_prev = f;
	}
}

}  // namespace spii.
