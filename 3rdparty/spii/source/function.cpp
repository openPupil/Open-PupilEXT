// Petter Strandmark 2012–2013.

#include <algorithm>
#include <exception>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <typeinfo>
#include <unordered_set>

#ifdef USE_OPENMP
	#include <omp.h>
#endif

#include <spii/function.h>
#include <spii/spii.h>

namespace spii {

// These two structs are used by Function to store added
// variables and terms.
struct AddedVariable
{
	int user_dimension;   // The dimension the Term object sees for evaluation.
	int solver_dimension; // The dimension of the variables the solver sees.
	double* user_data;    // The pointer provided by the user.
	size_t global_index;  // Global index into a vector of all scalars.
	bool is_constant;     // Whether this variable is (currently) constant.
	std::shared_ptr<ChangeOfVariables> change_of_variables;
	mutable std::vector<double>  temp_space; // Used internally during evaluation.
};

struct IntPairHash
{
	size_t operator()(const std::pair<int, int>& p) const
	{
		// http://stackoverflow.com/questions/738054/hash-function-for-a-pair-of-long-long
		std::hash<int> hash;
		size_t seed = hash(p.first);
		return hash(p.second) + 0x9e3779b9 + (seed<<6) + (seed>>2);
	}
};

namespace
{
	bool is_real(double value)
	{
		if (value != value) {
			return false;
		}
		else if (value == std::numeric_limits<double>::infinity()) {
			return false;
		}
		else if (value == -std::numeric_limits<double>::infinity()) {
			return false;
		}
		else {
			return true;
		}
	}
}

class Function::Implementation
{
public:
	Implementation(Function* function_interface);

	// Implemenations of functions in the public interface.
	double evaluate(const Eigen::VectorXd& x,
	                Eigen::VectorXd* gradient,
	                Eigen::MatrixXd* hessian) const;
	double evaluate(const Eigen::VectorXd& x,
	                Eigen::VectorXd* gradient,
	                Eigen::SparseMatrix<double>* hessian) const;
	Interval<double> evaluate(const std::vector<Interval<double>>& x) const;

	// Adds a variable to the function. All variables must be added
	// before any terms containing them are added.
	void add_variable_internal(double* variable,
	                           int dimension,
	                           std::shared_ptr<ChangeOfVariables> change_of_variables = 0);

	void set_constant(double* variable, bool is_constant);

	// Copies variables from a global vector x to the Function's
	// local storage.
	void copy_global_to_local(const Eigen::VectorXd& x) const;
	// Copies variables from a global vector x to the storage
	// provided by the user.
	void copy_global_to_user(const Eigen::VectorXd& x) const;
	// Copies variables from a the storage provided by the user
	// to a global vector x.
	void copy_user_to_global(Eigen::VectorXd* x) const;
	// Copies variables from a the storage provided by the user
	// to the Function's local storage.
	void copy_user_to_local() const;

	// Evaluates the function at the point in the local storage.
	double evaluate_from_local_storage() const;

	// Clears the function to the empty function.
	void clear();

	// All variables added to the function.
	std::vector<AddedVariable> variables;
	std::map<double*, std::size_t> variables_map;

	// Each variable can have several dimensions. This member
	// keeps track of the total number of scalars.
	size_t number_of_scalars;
	size_t number_of_constants;

	// Constant term (default: 0)
	double constant;

	// All terms added to the function.
	std::vector<AddedTerm> terms;

	// Number of threads used for evaluation.
	int number_of_threads;

	// Allocates temporary storage for gradient evaluations.
	// Should be called automatically at first evaluate()
	void allocate_local_storage() const;

	// If finalize has been called.
	mutable bool local_storage_allocated;
	// Has to be mutable because the temporary storage
	// needs to be written to.
	mutable std::vector< std::vector<Eigen::VectorXd> >
		thread_gradient_scratch;
	mutable std::vector<Eigen::VectorXd>
		thread_gradient_storage;
	// Temporary storage for the hessian.
	typedef std::vector<std::vector<Eigen::MatrixXd>> HessianStorage;
	mutable std::vector<HessianStorage> thread_hessian_scratch;
	mutable std::vector<Eigen::MatrixXd> thread_dense_hessian_storage;

	typedef std::vector<Eigen::Triplet<double>> SparseHessianStorage;
	mutable std::vector<SparseHessianStorage> thread_sparse_hessian_storage;

	// Stored how many element were used the last time the Hessian
	// was created.
	mutable size_t number_of_hessian_elements;

	Function* interface;
};

Function::Function() 
	: impl{new Function::Implementation{this}}
{ }

Function::Function(const Function& org)
	: impl{new Function::Implementation{this}}
{
	*this = org;
}

Function::Implementation::Implementation(Function* function_interface) 
	: interface{function_interface}
{
	clear();
} 

Function::~Function()
{
	delete impl;
}

void Function::Implementation::clear()
{
	constant = 0;

	terms.clear();
	variables.clear();
	variables_map.clear();
	number_of_scalars = 0;
	number_of_constants = 0;

	thread_gradient_scratch.clear();
	thread_gradient_storage.clear();
	local_storage_allocated = false;

	number_of_hessian_elements = 0;

	#ifdef USE_OPENMP
		number_of_threads = omp_get_max_threads();
	#else
		number_of_threads = 1;
	#endif
}

Function& Function::operator = (const Function& org)
{
	if (this == &org) {
		return *this;
	}
	impl->clear();

	this->hessian_is_enabled = org.hessian_is_enabled;
	impl->constant = org.impl->constant;

	// TODO: respect global order.
	std::map<size_t, double*> user_variables;
	for (const auto& added_variable: org.impl->variables_map) {
		AddedVariable& var_info = org.impl->variables[added_variable.second];

		impl->add_variable_internal(added_variable.first,
		                            var_info.user_dimension,
		                            var_info.change_of_variables);
		user_variables[var_info.global_index] = added_variable.first;
	}
	spii_assert(org.impl->variables.size() == user_variables.size());
	spii_assert(get_number_of_variables() == org.get_number_of_variables());
	spii_assert(get_number_of_scalars() == org.get_number_of_scalars());

	for (const auto& added_term: org.impl->terms) {
		std::vector<double*> vars;
		for (auto var: added_term.added_variables_indices) {
			auto index = org.impl->variables[var].global_index;
			spii_assert(user_variables.find(index) != user_variables.end());
			vars.push_back(user_variables[index]);
		}
		this->add_term(added_term.term, vars);
	}
	spii_assert(org.impl->variables.size() == user_variables.size());

	return *this;
}

Function& Function::operator += (const Function& org)
{
	impl->constant += org.impl->constant;

	// Check that there are no change of variables involved.
	for (const auto& added_variable: org.impl->variables) {
		spii_assert(!added_variable.change_of_variables);
	}
	for (const auto& added_variable: impl->variables) {
		spii_assert(!added_variable.change_of_variables);
	}

	// TODO: respect global order.
	std::map<size_t, double*> user_variables;
	for (const auto& added_variable: org.impl->variables_map) {
		AddedVariable& var_info = org.impl->variables[added_variable.second];

		// No-op if the variable already exists.
		impl->add_variable_internal(added_variable.first,
		                            var_info.user_dimension,
		                            var_info.change_of_variables);
		user_variables[var_info.global_index] = added_variable.first;
	}

	for (auto const& added_term: org.impl->terms) {
		std::vector<double*> vars;
		for (auto var: added_term.added_variables_indices) {
			auto index = org.impl->variables[var].global_index;
			spii_assert(user_variables.find(index) != user_variables.end());
			vars.push_back(user_variables[index]);
		}
		this->add_term(added_term.term, vars);
	}

	return *this;
}

Function& Function::operator += (double constant_value)
{
	impl->constant += constant_value;
	return *this;
}

void Function::add_variable(double* variable,
                            int dimension)
{
	impl->add_variable_internal(variable, dimension);
}

size_t Function::get_variable_global_index(double* variable) const
{
	// Find the variable. This has to succeed.
	auto itr = impl->variables_map.find(variable);
	check(itr != impl->variables_map.end(),
	      "Function::get_variable_global_index: variable not found.");

	return impl->variables[itr->second].global_index;
}

size_t Function::get_number_of_variables() const
{
	return impl->variables.size();
}

// Returns the current number of scalars the function contains.
// (each variable contains of one or several scalars.)
size_t Function::get_number_of_scalars() const
{
	return impl->number_of_scalars;
}

void Function::add_variable_internal(double* variable,
                                     int dimension,
                                     std::shared_ptr<ChangeOfVariables> change_of_variables)
{
	impl->add_variable_internal(variable, dimension, change_of_variables);
}


template<typename Map, typename T>
std::pair<typename Map::const_iterator,
          typename Map::const_iterator>
	find_surrounding_elements(const Map& map, const T& t)
{
	std::pair<typename Map::const_iterator,
		typename Map::const_iterator> output;
	if (map.empty()) {
		return std::make_pair(map.end(), map.end());
	}

	auto upper = map.upper_bound(t);
	output.second = upper;
	output.first = upper;
	if (upper != map.begin())
		output.first--;

	if (output.first->first >= t) {
		output.first = map.end();
	}
	if (output.second != map.end() && output.second->first <= t) {
		output.second = map.end();
	}
	return output;
}

void Function::Implementation::add_variable_internal(double* variable,
                                                    int dimension,
                                                    std::shared_ptr<ChangeOfVariables> change_of_variables)
{
	this->local_storage_allocated = false;

	// Check if variable already exists, and if it
	// does, that it still has the same dimensions.
	auto itr = variables_map.find(variable);
	if (itr != variables_map.end()) {
		AddedVariable& var_info = variables[itr->second];

		check(var_info.user_dimension == dimension,
		      "Function::add_variable: dimension mismatch "
			  "with previously added variable.");

		var_info.change_of_variables = change_of_variables;
		if (change_of_variables) {
			check(var_info.user_dimension == change_of_variables->x_dimension(),
			      "Function::add_variable: x_dimension can not change.");
			check(var_info.solver_dimension == change_of_variables->t_dimension(),
			      "Function::add_variable: t_dimension can not change.");
		}

		return;
	}
	else if (!variables_map.empty()) {
		// The variable does not yet exist, check that
		// it does not overlap any other.
		auto elems = find_surrounding_elements(variables_map, variable);

		auto succ = elems.second;
		if (succ != variables_map.end()) {
			check(variable + dimension <= variables[succ->second].user_data,
			      "Variables overlap.");
		}

		auto prev = elems.first;
		if (prev != variables_map.end()) {
			const auto& prev_var = variables[prev->second];
			check(prev_var.user_data + prev_var.user_dimension <= variable,
			      "Variables overlap.");
		}
	}

	variables.emplace_back();
	AddedVariable& var_info = variables.back();
	variables_map[variable] = variables.size() - 1;
	
	var_info.user_data = variable;
	var_info.is_constant = false;
	var_info.change_of_variables = change_of_variables;

	// Set the correct user_dimension and solver_dimension.
	if (change_of_variables){
		check(dimension == change_of_variables->x_dimension(), 
		      "Function::add_variable: dimension does not match the change of variables.");
		var_info.user_dimension   = change_of_variables->x_dimension();
		var_info.solver_dimension = change_of_variables->t_dimension();
	}
	else {
		var_info.user_dimension   = dimension;
		var_info.solver_dimension = dimension;
	}

	// Allocate local scratch spaces for evaluation.
	// We need as much space as the dimension of x.
	var_info.temp_space.resize(var_info.user_dimension);
	// Give this variable a global index into a global
	// state vector.
	var_info.global_index = number_of_scalars;
	number_of_scalars += var_info.solver_dimension;
}

void Function::Implementation::set_constant(double* variable, bool is_constant)
{
	// Find the variable. This has to succeed.
	auto itr = variables_map.find(variable);
	check(itr != variables_map.end(), 
	      "Function::set_constant: variable not found.");

	variables[itr->second].is_constant = is_constant;

	// Recompute all global indices. Expensive!
	this->number_of_scalars = 0;
	for (auto& variable: variables) {
		if (!variable.is_constant) {
			// Give this variable a global index into a global
			// state vector.
			variable.global_index = this->number_of_scalars;
			this->number_of_scalars += variable.solver_dimension;
		}
	}

	this->number_of_constants = 0;
	for (auto& variable : variables) {
		if (variable.is_constant) {
			// Give this variable a global index into a global
			// state vector.
			variable.global_index = this->number_of_scalars + this->number_of_constants;
			this->number_of_constants += variable.solver_dimension;
		}
	}

	this->local_storage_allocated = false;
}

void Function::set_constant(double* variable, bool is_constant)
{
	impl->set_constant(variable, is_constant);
}

void Function::add_term(std::shared_ptr<const Term> term, const std::vector<double*>& arguments)
{
	impl->local_storage_allocated = false;

	check(term->number_of_variables() == arguments.size(),
	      "Function::add_term: incorrect number of arguments.");

	impl->terms.emplace_back();
	auto& added_term = impl->terms.back();
	added_term.term = term;
	added_term.added_variables_indices.reserve(arguments.size());

	try {
		// Check whether the variables exist.
		for (int var = 0; var < term->number_of_variables(); ++var) {
			auto var_itr = impl->variables_map.find(arguments[var]);
			if (var_itr == impl->variables_map.end()) {
				add_variable(arguments[var], term->variable_dimension(var));
				var_itr = impl->variables_map.find(arguments[var]);
			}
			// The x-dimension of the variable must match what is expected by the term.
			else {
				spii_assert(impl->variables[var_itr->second].user_dimension == term->variable_dimension(var),
				            "Function::add_term: variable dimension does not match term.");
			}

			// Look up this variable.
			auto var_index = var_itr->second;
			added_term.added_variables_indices.emplace_back(var_index);
		}
	} catch(...) {
		impl->terms.pop_back();
		throw;
	}
}

size_t Function::get_number_of_terms() const
{
	return impl->terms.size();
}

const BeginEndProvider<AddedTerm> Function::terms() const
{
	return {impl->terms};
}

void Function::set_number_of_threads(int num)
{
	#ifdef USE_OPENMP
		spii_assert(num > 0, "Function::set_number_of_threads: "
		                     "invalid number of threads.");
		impl->local_storage_allocated = false;
		impl->number_of_threads = num;
	#endif
}

void Function::Implementation::allocate_local_storage() const
{
	auto start_time = wall_time();

	size_t max_arity = 1;
	int max_variable_dimension = 1;
	for (const auto& itr: variables) {
		max_variable_dimension = std::max(max_variable_dimension,
		                                  itr.user_dimension);
	}
	for (const auto& term: terms) {
		max_arity = std::max(max_arity, term.added_variables_indices.size());
	}

	this->thread_gradient_scratch.resize(this->number_of_threads);
	this->thread_gradient_storage.resize(this->number_of_threads);
	for (int t = 0; t < this->number_of_threads; ++t) {
		this->thread_gradient_storage[t].resize(number_of_scalars + number_of_constants);
		this->thread_gradient_scratch[t].resize(max_arity);
		for (int var = 0; var < max_arity; ++var) {
			this->thread_gradient_scratch[t][var].resize(max_variable_dimension);
		}
	}

	// Every term should have a pointer to the local space
	// used when evaluating.
	for (auto& added_term: terms) {
		for (auto ind: added_term.added_variables_indices) {
			// Look up this variable.
			auto& added_variable = variables[ind];
			// Stora a pointer to temporary storage for this variable.
			double* temp_space = &added_variable.temp_space[0];
			added_term.temp_variables.push_back(temp_space);
		}
	}

	if (interface->hessian_is_enabled) {
		this->thread_hessian_scratch.resize(this->number_of_threads);
		for (int t = 0; t < this->number_of_threads; ++t) {
			auto& hessian = this->thread_hessian_scratch[t];

			hessian.resize(max_arity);
			for (int var0 = 0; var0 < max_arity; ++var0) {
				hessian[var0].resize(max_arity);
				for (int var1 = 0; var1 < max_arity; ++var1) {
					hessian[var0][var1].resize(max_variable_dimension,
					                           max_variable_dimension);
				}
			}
		}
	}

	this->local_storage_allocated = true;

	interface->allocation_time += wall_time() - start_time;
}

void Function::print_timing_information(std::ostream& out) const
{
	out << "----------------------------------------------------\n";
	out << "Function evaluations without gradient : " << evaluations_without_gradient << '\n';
	out << "Function evaluations with gradient    : " << evaluations_with_gradient << '\n';
	out << "Function memory allocation time   : " << allocation_time << '\n';
	out << "Function evaluate time            : " << evaluate_time << '\n';
	out << "Function evaluate time (with g/H) : " << evaluate_with_hessian_time << '\n';
	out << "Function write g/H time           : " << write_gradient_hessian_time << '\n';
	out << "Function copy data time           : " << copy_time << '\n';
	out << "----------------------------------------------------\n";
}

double Function::Implementation::evaluate_from_local_storage() const
{
	spii_assert(this->local_storage_allocated);

	interface->evaluations_without_gradient++;
	double start_time = wall_time();

	double value = this->constant;
	// Go through and evaluate each term.
	// OpenMP requires a signed data type as the loop variable.
	#ifdef USE_OPENMP
		// Each thread needs to store a specific error.
		std::vector<std::exception_ptr> evaluation_errors(this->number_of_threads);

		#pragma omp parallel for reduction(+ : value) num_threads(this->number_of_threads) if (terms.size() > 1)
	#endif
	// For loop has to be int for OpenMP.
	for (int i = 0; i < terms.size(); ++i) {
		#ifdef USE_OPENMP
			// The thread number calling this iteration.
			int t = omp_get_thread_num();
			// We need to catch all exceptions before leaving
			// the loop body.
			try {
		#endif

		// Evaluate the term .
		value += terms[i].term->evaluate(&terms[i].temp_variables[0]);

		#ifdef USE_OPENMP
			// We need to catch all exceptions before leaving
			// the loop body.
			}
			catch (...) {
				evaluation_errors[t] = std::current_exception();
			}
		#endif
	}

	#ifdef USE_OPENMP
		// Now that we are outside the OpenMP block, we can
		// rethrow exceptions.
		for (auto itr = evaluation_errors.begin(); itr != evaluation_errors.end(); ++itr) {
			// VS 2010 does not have conversion to bool or
			// operator !=.
			if ( !(*itr == std::exception_ptr())) {
				std::rethrow_exception(*itr);
			}
		}
	#endif

	interface->evaluate_time += wall_time() - start_time;
	return value;
}

double Function::evaluate(const Eigen::VectorXd& x) const
{
	if (! impl->local_storage_allocated) {
		impl->allocate_local_storage();
	}

	// Copy values from the global vector x to the temporary storage
	// used for evaluating the term.
	impl->copy_global_to_local(x);

	return impl->evaluate_from_local_storage();
}

double Function::evaluate() const
{
	if (! impl->local_storage_allocated) {
		impl->allocate_local_storage();
	}

	// Copy the user state to the local storage
	// for evaluation.
	impl->copy_user_to_local();

	return impl->evaluate_from_local_storage();
}

void Function::create_sparse_hessian(Eigen::SparseMatrix<double>* H) const
{
	double start_time = wall_time();

	Implementation::SparseHessianStorage hessian_indices;
	//std::set<std::pair<int, int>> hessian_indices_set;
	std::unordered_set<std::pair<int, int>, IntPairHash> hessian_indices_set;
	impl->number_of_hessian_elements = 0;

	for (const auto& added_term: impl->terms) {
		auto& indices = added_term.added_variables_indices;
		auto& term    = added_term.term;

		// Put the hessian into the global hessian.
		for (int var0 = 0; var0 < term->number_of_variables(); ++var0) {
			if ( ! impl->variables[indices[var0]].is_constant) {

				size_t global_offset0 = impl->variables[indices[var0]].global_index;
				for (int var1 = 0; var1 < term->number_of_variables(); ++var1) {
					if ( ! impl->variables[indices[var1]].is_constant) {

						size_t global_offset1 = impl->variables[indices[var1]].global_index;
						for (size_t i = 0; i < term->variable_dimension(var0); ++i) {
							for (size_t j = 0; j < term->variable_dimension(var1); ++j) {
								int global_i = static_cast<int>(i + global_offset0);
								int global_j = static_cast<int>(j + global_offset1);
								
								// Fix for old versions of libstdc++ that do not have
								// emplace. Remove when continuous integration upgrades.
								#ifdef __GLIBCXX__
									#if __GLIBCXX__ <= 20120322
										hessian_indices_set.insert(std::make_pair(global_i, global_j));
									#else
										hessian_indices_set.emplace(global_i, global_j);
									#endif
								#else
									hessian_indices_set.emplace(global_i, global_j);
								#endif
							}
						}

					}
				}

			}
		}

	}

	hessian_indices.reserve(hessian_indices_set.size());
	for (const auto& ij: hessian_indices_set) {
		hessian_indices.emplace_back(ij.first, ij.second, 1.0);
	}

	impl->number_of_hessian_elements = hessian_indices.size();

	auto n = static_cast<int>(impl->number_of_scalars);
	H->resize(n, n);
	H->setFromTriplets(hessian_indices.begin(), hessian_indices.end());
	H->makeCompressed();

	this->allocation_time += wall_time() - start_time;
}

void Function::Implementation::copy_global_to_local(const Eigen::VectorXd& x) const
{
	double start_time = wall_time();

	#ifdef USE_OPENMP
		#pragma omp parallel for num_threads(this->number_of_threads) if (variables.size() > 1000)
	#endif
	for (std::ptrdiff_t i = 0; i < std::ptrdiff_t(variables.size()); ++i) {
		const auto& var = variables[i];

		if ( ! var.is_constant) {
			if (var.change_of_variables == nullptr) {
				for (int i = 0; i < var.user_dimension; ++i) {
					var.temp_space[i] = x[var.global_index + i];
				}
			}
			else {
				var.change_of_variables->t_to_x(
					&var.temp_space[0],
					&x[var.global_index]);
			}
		}
		else {
			// This variable is constant and is therefore not
			// present in the global vector x of variables.
			// Copy the constant from the user space instead.
			for (int i = 0; i < var.user_dimension; ++i) {
				var.temp_space[i] = var.user_data[i];
			}
		}
	}

	interface->copy_time += wall_time() - start_time;
}

void Function::copy_user_to_global(Eigen::VectorXd* x) const
{
	impl->copy_user_to_global(x);
}

void Function::Implementation::copy_user_to_global(Eigen::VectorXd* x) const
{
	double start_time = wall_time();

	x->resize(this->number_of_scalars);
	for (const auto& var: variables) {
		double* data = var.user_data;

		if ( ! var.is_constant) {
			if (var.change_of_variables == nullptr) {
				for (int i = 0; i < var.user_dimension; ++i) {
					check(is_real(data[i]), "User data is invalid (NaN or infinity).");

					(*x)[var.global_index + i] = data[i];
				}
			}
			else {
				var.change_of_variables->x_to_t(
					&(*x)[var.global_index],
					data);
			}
		}
	}

	interface->copy_time += wall_time() - start_time;
}

void Function::copy_global_to_user(const Eigen::VectorXd& x) const
{
	impl->copy_global_to_user(x);
}

void Function::Implementation::copy_global_to_user(const Eigen::VectorXd& x) const
{
	double start_time = wall_time();

	for (const auto& var: variables) {
		double* data = var.user_data;

		if ( ! var.is_constant) {
			if (var.change_of_variables == nullptr) {
				for (int i = 0; i < var.user_dimension; ++i) {
					data[i] = x[var.global_index + i];
				}
			}
			else {
				var.change_of_variables->t_to_x(
					data,
					&x[var.global_index]);
			}
		}
	}

	interface->copy_time += wall_time() - start_time;
}

void Function::Implementation::copy_user_to_local() const
{
	double start_time = wall_time();

	for (const auto& var: variables) {
		double* data = var.user_data;

		// Both variables and constants are copied here.

		for (int i = 0; i < var.user_dimension; ++i) {
			check(is_real(data[i]), "User data is invalid (NaN or infinity).");

			var.temp_space[i] = data[i];
		}
	}

	interface->copy_time += wall_time() - start_time;
}

double Function::evaluate(const Eigen::VectorXd& x,
                          Eigen::VectorXd* gradient) const
{
	return this->evaluate(x, gradient, reinterpret_cast<Eigen::MatrixXd*>(0));
}


double Function::evaluate(const Eigen::VectorXd& x,
                          Eigen::VectorXd* gradient,
						  Eigen::MatrixXd* hessian) const
{
	return impl->evaluate(x, gradient, hessian);
}

double Function::Implementation::evaluate(const Eigen::VectorXd& x,
                                          Eigen::VectorXd* gradient,
						                  Eigen::MatrixXd* hessian) const
{
	interface->evaluations_with_gradient++;

	spii_assert(!hessian || interface->hessian_is_enabled,
	            "Function::evaluate: Hessian computation is not enabled.");

	if (! this->local_storage_allocated) {
		this->allocate_local_storage();
	}

	double start_time = wall_time();
	if (hessian) {
		#ifdef USE_OPENMP
			thread_dense_hessian_storage.resize(this->number_of_threads);
		#else
			thread_dense_hessian_storage.resize(1);
		#endif
		for (int t = 0; t < this->number_of_threads; ++t) {
			thread_dense_hessian_storage[t].resize(static_cast<int>(this->number_of_scalars),
			                                       static_cast<int>(this->number_of_scalars));
			thread_dense_hessian_storage[t].setZero();
		}
	}
	interface->allocation_time += wall_time() - start_time;

	// Copy values from the global vector x to the temporary storage
	// used for evaluating the term.
	this->copy_global_to_local(x);

	start_time = wall_time();

	// Initialize each thread's global gradient.
	for (int t = 0; t < this->number_of_threads; ++t) {
		this->thread_gradient_storage[t].setZero();
	}

	double value = this->constant;

	// Go through and evaluate each term.
	// OpenMP requires a signed data type as the loop variable.
	#ifdef USE_OPENMP
		// Each thread needs to store a specific error.
		std::vector<std::exception_ptr> evaluation_errors(this->number_of_threads);

		#pragma omp parallel for reduction(+ : value) num_threads(this->number_of_threads)
	#endif
	for (int i = 0; i < terms.size(); ++i) {
		#ifdef USE_OPENMP
			// The thread number calling this iteration.
			int t = omp_get_thread_num();
			// We need to catch all exceptions before leaving
			// the loop body.
			try {
		#else
			int t = 0;
		#endif

		if (hessian) {
			// Evaluate the term and put its gradient and hessian
			// into local storage.
			value += terms[i].term->evaluate(&terms[i].temp_variables[0],
											 &this->thread_gradient_scratch[t],
											 &this->thread_hessian_scratch[t]);


			const auto& term = terms[i].term;
			const auto& indices = terms[i].added_variables_indices;
			// Put the hessian into the global hessian.
			for (int var0 = 0; var0 < term->number_of_variables(); ++var0) {

				if ( ! variables[indices[var0]].is_constant) {
					spii_assert(!variables[indices[var0]].change_of_variables,
					            "Change of variables not supported for Hessians");

					size_t global_offset0 = variables[indices[var0]].global_index;
					for (int var1 = 0; var1 < term->number_of_variables(); ++var1) {
						size_t global_offset1 = variables[indices[var1]].global_index;

						if ( ! variables[indices[var1]].is_constant) {

							const Eigen::MatrixXd& part_hessian = this->thread_hessian_scratch[t][var0][var1];
							for (int i = 0; i < term->variable_dimension(var0); ++i) {
								for (int j = 0; j < term->variable_dimension(var1); ++j) {
									thread_dense_hessian_storage[t]
										.coeffRef(i + global_offset0, j + global_offset1)
									+= part_hessian(i, j);
								}
							}

						}
					}
				}
			}


		}
		else {
			// Evaluate the term and put its gradient into local
			// storage.
			value += terms[i].term->evaluate(&terms[i].temp_variables[0],
											 &this->thread_gradient_scratch[t]);
		}

		// Put the gradient from the term into the thread's global gradient.
		const auto& indices = terms[i].added_variables_indices;
		for (int var = 0; var < indices.size(); ++var) {

			if ( ! variables[indices[var]].is_constant) {
				if (variables[indices[var]].change_of_variables == nullptr) {
					// No change of variables, just copy the gradient.
					size_t global_offset = variables[indices[var]].global_index;
					for (int i = 0; i < variables[indices[var]].user_dimension; ++i) {
						this->thread_gradient_storage[t][global_offset + i] +=
							this->thread_gradient_scratch[t][var][i];
					}
				}
				else {
					// Transform the gradient from user space to solver space.
					size_t global_offset = variables[indices[var]].global_index;
					if (global_offset < this->number_of_scalars) {
						variables[indices[var]].change_of_variables->update_gradient(
							&this->thread_gradient_storage[t][global_offset],
							&x[global_offset],
							&this->thread_gradient_scratch[t][var][0]);
					}
				}
			}
		}

		#ifdef USE_OPENMP
			// We need to catch all exceptions before leaving
			// the loop body.
			}
			catch (...) {
				evaluation_errors[t] = std::current_exception();
			}
		#endif
	}

	#ifdef USE_OPENMP
		// Now that we are outside the OpenMP block, we can
		// rethrow exceptions.
		for (auto itr = evaluation_errors.begin(); itr != evaluation_errors.end(); ++itr) {
			// VS 2010 does not have conversion to bool or
			// operator !=.
			if ( !(*itr == std::exception_ptr())) {
				std::rethrow_exception(*itr);
			}
		}
	#endif

	interface->evaluate_with_hessian_time += wall_time() - start_time;
	start_time = wall_time();

	// Create the global gradient.
	if (gradient->size() != this->number_of_scalars) {
		gradient->resize(this->number_of_scalars);
	}
	gradient->setZero();
	// Sum the gradients from all different terms.
	for (int t = 0; t < this->number_of_threads; ++t) {
		(*gradient) += this->thread_gradient_storage[t].segment(0, this->number_of_scalars);
	}

	if (hessian) {
		// Create the global (dense) hessian.
		hessian->resize( static_cast<int>(this->number_of_scalars),
						 static_cast<int>(this->number_of_scalars));
		hessian->setZero();
		for (int t = 0; t < this->number_of_threads; ++t) {
			(*hessian) += this->thread_dense_hessian_storage[t];
		}
	}

	interface->write_gradient_hessian_time += wall_time() - start_time;
	return value;
}

double Function::evaluate(const Eigen::VectorXd& x,
                          Eigen::VectorXd* gradient,
						  Eigen::SparseMatrix<double>* hessian) const
{
	return impl->evaluate(x, gradient, hessian);
}

double Function::Implementation::evaluate(const Eigen::VectorXd& x,
                                          Eigen::VectorXd* gradient,
						                  Eigen::SparseMatrix<double>* hessian) const
{
	interface->evaluations_with_gradient++;

	spii_assert(hessian);
	spii_assert(interface->hessian_is_enabled,
	            "Function::evaluate: Hessian computation is not enabled.");

	if (! this->local_storage_allocated) {
		this->allocate_local_storage();
	}

	double start_time = wall_time();
	#ifdef USE_OPENMP
		thread_sparse_hessian_storage.resize(this->number_of_threads);
	#else
		thread_sparse_hessian_storage.resize(1);
	#endif
	for (int t = 0; t < this->number_of_threads; ++t) {
		// TODO: Most likely too much space per thread here.
		thread_sparse_hessian_storage[t].reserve(this->number_of_hessian_elements);
		thread_sparse_hessian_storage[t].clear();
	}
	this->number_of_hessian_elements = 0;

	interface->allocation_time += wall_time() - start_time;

	// Copy values from the global vector x to the temporary storage
	// used for evaluating the term.
	this->copy_global_to_local(x);

	start_time = wall_time();

	interface->write_gradient_hessian_time += wall_time() - start_time;
	start_time = wall_time();

	// Initialize each thread's global gradient.
	for (int t = 0; t < this->number_of_threads; ++t) {
		this->thread_gradient_storage[t].setZero();
	}

	double value = this->constant;
	// Go through and evaluate each term.
	// OpenMP requires a signed data type as the loop variable.
	#ifdef USE_OPENMP
		// Each thread needs to store a specific error.
		std::vector<std::exception_ptr> evaluation_errors(this->number_of_threads);

		#pragma omp parallel for reduction(+ : value) num_threads(this->number_of_threads)
	#endif
	for (int i = 0; i < terms.size(); ++i) {
		#ifdef USE_OPENMP
			// The thread number calling this iteration.
			int t = omp_get_thread_num();
			// We need to catch all exceptions before leaving
			// the loop body.
			try {
		#else
			int t = 0;
		#endif

		// Evaluate the term and put its gradient and hessian
		// into local storage.
		value += terms[i].term->evaluate(&terms[i].temp_variables[0],
		                                 &this->thread_gradient_scratch[t],
		                                 &this->thread_hessian_scratch[t]);

		// Put the gradient from the term into the thread's global gradient.
		const auto& indices = terms[i].added_variables_indices;
		for (int var = 0; var < indices.size(); ++var) {

			if ( ! variables[indices[var]].is_constant) {
				spii_assert(!variables[indices[var]].change_of_variables,
				            "Change of variables not supported for sparse Hessian");

				size_t global_offset = variables[indices[var]].global_index;
				for (int i = 0; i < variables[indices[var]].user_dimension; ++i) {
					this->thread_gradient_storage[t][global_offset + i] +=
						this->thread_gradient_scratch[t][var][i];
				}
			}
		}

		// Put the hessian from the term into the thread's global hessian.
		const auto& term = terms[i].term;
		for (int var0 = 0; var0 < term->number_of_variables(); ++var0) {
			if ( ! variables[indices[var0]].is_constant) {

				size_t global_offset0 = variables[indices[var0]].global_index;
				for (int var1 = 0; var1 < term->number_of_variables(); ++var1) {
					if ( ! variables[indices[var1]].is_constant) {

						size_t global_offset1 = variables[indices[var1]].global_index;
						const Eigen::MatrixXd& part_hessian = this->thread_hessian_scratch[t][var0][var1];
						for (int i = 0; i < term->variable_dimension(var0); ++i) {
							for (int j = 0; j < term->variable_dimension(var1); ++j) {

								int global_i = static_cast<int>(i + global_offset0);
								int global_j = static_cast<int>(j + global_offset1);
								thread_sparse_hessian_storage[t].push_back(Eigen::Triplet<double>(global_i,
								                                                                  global_j,
								                                                                  part_hessian(i, j)));
							}
						}
					}

				}
			}
		}

		#ifdef USE_OPENMP
			// We need to catch all exceptions before leaving
			// the loop body.
			}
			catch (...) {
				evaluation_errors[t] = std::current_exception();
			}
		#endif
	}

	#ifdef USE_OPENMP
		// Now that we are outside the OpenMP block, we can
		// rethrow exceptions.
		for (auto itr = evaluation_errors.begin(); itr != evaluation_errors.end(); ++itr) {
			// VS 2010 does not have conversion to bool or
			// operator !=.
			if ( !(*itr == std::exception_ptr())) {
				std::rethrow_exception(*itr);
			}
		}
	#endif

	interface->evaluate_with_hessian_time += wall_time() - start_time;
	start_time = wall_time();

	// Create the global gradient.
	if (gradient->size() != this->number_of_scalars) {
		gradient->resize(this->number_of_scalars);
	}
	gradient->setZero();
	// Sum the gradients from all different terms.
	for (int t = 0; t < this->number_of_threads; ++t) {
		(*gradient) += this->thread_gradient_storage[t].segment(0, this->number_of_scalars);
	}

	for (int t = 1; t < thread_sparse_hessian_storage.size(); ++t) {
		for (const auto& triple: thread_sparse_hessian_storage[t]) {
			thread_sparse_hessian_storage[0].emplace_back(triple);
		}
	}

	hessian->setFromTriplets(thread_sparse_hessian_storage[0].begin(),
	                         thread_sparse_hessian_storage[0].end());
	//hessian->makeCompressed();

	interface->write_gradient_hessian_time += wall_time() - start_time;

	return value;
}

Interval<double> Function::evaluate(const std::vector<Interval<double>>& x) const
{
	return impl->evaluate(x);
}

Interval<double>  Function::Implementation::evaluate(const std::vector<Interval<double>>& x) const
{
	if (! this->local_storage_allocated) {
		this->allocate_local_storage();
	}
	// Copies constant variables.
	this->copy_user_to_local();

	spii_assert(x.size() == this->number_of_scalars);

	interface->evaluations_without_gradient++;
	double start_time = wall_time();

	std::vector<std::vector<const Interval<double> *>> scratch_space;
	std::vector<std::vector<std::vector<Interval<double>>>> temp_intervals;
	#ifdef USE_OPENMP
		scratch_space.resize(this->number_of_threads);
		temp_intervals.resize(this->number_of_threads);
	#else
		scratch_space.resize(1);
		temp_intervals.resize(1);
	#endif

	double lower = this->constant;
	double upper = this->constant;

	#ifdef USE_OPENMP
		// Each thread needs to store a specific error.
		std::vector<std::exception_ptr> evaluation_errors(this->number_of_threads);

		// Go through and evaluate each term.
		// reduction(+ : lower) reduction(+ : upper)
		#pragma omp parallel for reduction(+ : lower) reduction(+ : upper) num_threads(this->number_of_threads) if (terms.size() > 1)
	#endif
	for (int i = 0; i < terms.size(); ++i) {
		// Evaluate each term.

		#ifdef USE_OPENMP
			int t = omp_get_thread_num();

			// We need to catch all exceptions before leaving
			// the loop body.
			try {
		#else
			int t = 0;
		#endif

		temp_intervals[t].clear();
		scratch_space[t].clear();
		for (auto var: terms[i].added_variables_indices) {
			
			if (variables[var].is_constant) {
				// This code assumes that vectors that are part of another
				// vector are not reallocated when the outer vector is
				// resized. This is true if std::move of a vector is
				// implemented without reallocation.
				temp_intervals[t].emplace_back();
				for (auto value: variables[var].temp_space) {
					temp_intervals[t].back().emplace_back(value, value);
				}
				scratch_space[t].push_back(temp_intervals[t].back().data());
			}
			else {
				check(variables[var].change_of_variables == nullptr,
					"Global optimization does not support change of variables.");

				auto global_index = variables[var].global_index;
				scratch_space[t].push_back(&x.at(global_index));
			}
		}

		auto value = terms[i].term->evaluate_interval(scratch_space[t].data());
		lower += value.get_lower();
		upper += value.get_upper();

		#ifdef USE_OPENMP
				// We need to catch all exceptions before leaving
				// the loop body.
			}
			catch (...) {
				evaluation_errors[t] = std::current_exception();
			}
		#endif
	}

	#ifdef USE_OPENMP
		// Now that we are outside the OpenMP block, we can
		// rethrow exceptions.
		for (auto itr = evaluation_errors.begin(); itr != evaluation_errors.end(); ++itr) {
			// VS 2010 does not have conversion to bool or
			// operator !=.
			if (!(*itr == std::exception_ptr())) {
				std::rethrow_exception(*itr);
			}
		}
	#endif

	Interval<double> value(lower, upper);
	interface->evaluate_time += wall_time() - start_time;
	return value;
}

void Function::write_to_stream(std::ostream& out) const
{
	using namespace std;

	// Use high precision.
	out << setprecision(30);

	// Write version to stream;
	out << "spii::function" << endl;
	out << 1 << endl;
	// Write the representation of a reasonably complicated class to
	// the file. We can then check that the compiler-dependent format
	// matches.
	out << TermFactory::fix_name(typeid(std::vector<std::map<double,int>>).name()) << endl;

	out << impl->terms.size() << endl;
	out << impl->variables.size() << endl;
	out << impl->number_of_scalars << endl;
	out << impl->constant << endl;

	vector<pair<std::size_t, std::size_t>> variable_dimensions; 
	for (const auto& variable: impl->variables) {
		spii_assert(variable.change_of_variables == nullptr,
		            "Function::write_to_stream: Change of variables not allowed.");

		variable_dimensions.emplace_back(variable.global_index, variable.user_dimension);
	}
	sort(variable_dimensions.begin(), variable_dimensions.end());
	for (const auto& variable : variable_dimensions) {
		out << variable.second << " ";
	}
	out << endl;

	Eigen::VectorXd x;
	this->copy_user_to_global(&x);
	for (int i = 0; i < impl->number_of_scalars; ++i) {
		out << x[i] << " ";
	}
	out << endl;

	for (const auto& added_term : impl->terms) {
		string term_name = TermFactory::fix_name(typeid(*added_term.term).name());
		out << term_name << endl;
		out << added_term.added_variables_indices.size() << endl;
		for (auto global_index : added_term.added_variables_indices) {
			out << global_index << " ";
		}
		out << endl;
		out << *added_term.term << endl;
	}
}

void Function::read_from_stream(std::istream& in, std::vector<double>* user_space, const TermFactory& factory)
{
	using namespace std;

	impl->clear();

	auto check = [&in](const char* variable_name) 
	{ 
		spii_assert(in, "Function::read_from_stream: Reading ", variable_name, " failed.");
	};
	#define read_and_check(var) in >> var; check(#var); //cout << #var << " = " << var << endl;

 	// TODO: Clear f.

	string spii_function;
	read_and_check(spii_function);
	spii_assert(spii_function == "spii::function");

	int version;
	read_and_check(version);
	string compiler_type_format;
	read_and_check(compiler_type_format);
	if (compiler_type_format
	    != TermFactory::fix_name(typeid(std::vector<std::map<double,int>>).name()))
	{
		throw runtime_error("Function::read_from_stream: Type format does not match. "
		                    "Files can not be shared between compilers.");
	}

	unsigned number_of_terms;
	read_and_check(number_of_terms);
	unsigned number_of_variables;
	read_and_check(number_of_variables);
	unsigned number_of_scalars;
	read_and_check(number_of_scalars);
	read_and_check(impl->constant);

	user_space->resize(number_of_scalars);
	int current_var = 0;
	for (unsigned i = 0; i < number_of_variables; ++i) {
		int variable_dimension;
		read_and_check(variable_dimension);
		this->add_variable(&user_space->at(current_var), variable_dimension);
		current_var += variable_dimension;
	}
	spii_assert(current_var == number_of_scalars);

	for (unsigned i = 0; i < number_of_scalars; ++i) {
		read_and_check(user_space->at(i));
	}

	for (unsigned i = 0; i < number_of_terms; ++i) {
		std::string term_name;
		read_and_check(term_name);
		unsigned term_vars;
		read_and_check(term_vars);

		std::vector<double*> arguments;
		for (unsigned i = 0; i < term_vars; ++i) {
			int offset;
			read_and_check(offset);
			arguments.push_back(&user_space->at(offset));
		}

		auto term = std::shared_ptr<const Term>(factory.create(term_name, in));
		this->add_term(term, arguments);
	}

	#undef read_and_check
}

}  // namespace spii