## This library has been merged into [monolith](https://github.com/PetterS/monolith) and will not be updated further here.

This is a library for unconstrained minimization of smooth functions with a large number of variables. I wrote this to get a better grasp of nonlinear optimization. I used the book ny Nocedal and Wright [1] as a reference.

Features
--------
* Newton's method 
    * Bunch-Kaufman-Parlett factorization and block-diagonal modification. This is a robust method of dealing with nonconvex functions.
    * The sparse solver uses repeated diagonal modification of the hessian for nonconvex problems. This simple method seems to work well, but can require several Cholesky factorizations per iteration and is not as robust as B-K-P.
    * The sparse solver can also use sym-ildl (if available) for sparse Bunch-Kaufman-Parlett.
* L-BFGS.
* Nelder-Mead for nondifferentiable problems.
* Global optimization with interval arithmetic (Moore-Skelboe algorithm).
* Automatic differentiation to compute gradient and hessian using FADBAD++ (included).
* Multi-threaded using OpenMP.
* The interface is very easy to use, while still allowing very high performance. Generic lambdas (C++14) can be used if the compiler supports it.

#### Experimental features
This repository also contains some experimental features. These features are not ready for production and do not have very extensive test coverage. They may change or be removed in the future.
* Constrained optimization (augmented lagrangian).

Benchmarks
----------
The solver comes with extensive benchmarks and tests.

* The NIST collection of non-linear least-squares problems. http://www.itl.nist.gov/div898/strd/nls/nls_main.shtml
* Test functions from More et al. [2].
* TEST_OPT http://people.sc.fsu.edu/~jburkardt/m_src/test_opt/test_opt.html

The Newton solver and the L-BFGS solver pass all of these tests. The NIST collection was very challenging and required block-diagonal robust factorization and handling of numerically hard problem instances. Note that non-linear least-squares problems have a special structure and are best solved with custom code, for example Ceres Solver.

Generic Lambdas
---------------
The interface supports generic lambdas:

``` C++
auto lambda =
	[](auto x, auto y)
	{
		// The Rosenbrock function.
		auto d0 =  y[0] - x[0]*x[0];
		auto d1 =  1 - x[0];
		return 100 * d0*d0 + d1*d1;
	};

// The lambda function will be copied and
// automatically differentiated. The derivatives
// are computed using templates, not numerically.
//
// No need to derive or compute derivatives
// manually!
auto term = make_differentiable<1, 1>(lambda);

double x=0, y=0;
function.add_term(term, &x, &y);
```
This interface is unit tested in `test_generic_lambdas.cpp`.

Compilation
-----------
Everything needed to compile the library, examples and tests using CMake should be included.
All tests pass with the following compilers:
* Visual Studio 2015
* GCC 4.9 (Cygwin)
* GCC 4.9 (Ubuntu)
* Clang 3.5 (Ubuntu)
Earlier compilers may not work.

You can check travis.yml for the commands used to build the library and run all tests on Ubuntu.
It is even easier on Windows. The status of the automatic builds using gcc and Clang is [![Build Status](https://travis-ci.org/PetterS/spii.png)](https://travis-ci.org/PetterS/spii).

References
----------
1. Nocedal and Wright, *Numerical Optimization*, Springer, 2006.
2. Jorge J. More, Burton S. Garbow and Kenneth E. Hillstrom, *Testing unconstrained optimization software*, Transactions on Mathematical Software 7(1):17-41, 1981.
