#include "../../core/rt/string_type.h"
#include "../../core/os/file_dir.h"
#include "../../core/os/kernel.h"
#include "test.h"


#if defined(PLATFORM_INTEL_MKL_SUPPORT)

#include "../../core/ext/mkl/mkl_vector.h"
#include "../../core/ext/mkl/mkl_matrix.h"
#include "../../core/ext/mkl/mkl_linequ.h"

void rt::UnitTests::mkl_vector()
{
	srand(0);

	mkl::Vector32	vec;
	vec.SetSize(7);
	vec.Zero();
	vec.Perturb(10);
	_LOG(vec);
	vec.Normalize();
	_LOG(vec);
	vec *= (float)vec.GetSize();
	_LOG(vec);
	vec.vml_Reciprocal();
	_LOG(vec);

	mkl::Matrix32	mat;
	mat.SetSize(7, 6);
	mat.Zero();
	mat.Perturb(10);
	_LOG(mat);
	_LOG(!mat);

	mkl::Matrix32	core;
	core.SetSizeAsProductOf(!mat, mat);
	core.Product(!mat, mat);
	_LOG(core);

	if(core.SolveEigen_ByEnerge(vec))
	{	_LOG("Eigen Values:");
		_LOG(vec);
		_LOG("Eigen Vectors:");
		_LOG(core);
	}
	else
	{	_LOG_WARNING("Solving eigen failed");
	}
}

void rt::UnitTests::mkl_linequ()
{
	// y = a*x + b
	mkl::LinearEquationSolver<> equ;
	equ.DefineProblem(1000, 2, 2000);
	mkl::Vector32	B,X;
	B.SetSize(1000);

	srand(0);
	for(UINT i=0;i<1000;i++)
	{
		float x = rand()/100.0f;
		equ.AddCoefficient(i, 0, x/2)
		   .AddCoefficient(i, 1, 0.5f);

		equ.AddCoefficient(i, 0, x/2)
		   .AddCoefficient(i, 1, 0.5f);

		B(i) = 32.1f*x + 446.8f + 5*(rand()/(float)RAND_MAX - 0.5f);
	}

	equ.Solve(B,X);
	_LOG("Solved: "<<X);
}

#endif

