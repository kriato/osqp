#ifndef OSQP_TYPES_H
# define OSQP_TYPES_H

# ifdef __cplusplus
extern "C" {
# endif // ifdef __cplusplus

# include "glob_opts.h"
# include "constants.h"
# include "lin_alg.h"


/******************
* Internal types *
******************/

/**
 *  Matrix in compressed sparse column format
 */
typedef struct {
  c_int    m;     ///< number of rows
  c_int    n;     ///< number of columns
  c_int   *p;     ///< column pointers (size n+1); col indices (size nzmax) start from 0 when using triplet format (direct KKT matrix formation)
  c_int   *i;     ///< row indices, size nzmax starting from 0
  c_float *x;     ///< numerical values, size nzmax
} csc;

/**
 * Linear system solver structure (sublevel objects initialize it differently)
 */

typedef struct linsys_solver LinSysSolver;

/**
 * OSQP Timer for statistics
 */
typedef struct OSQP_TIMER OSQPTimer;

/**
 * Problem scaling matrices stored as vectors
 */
typedef struct {
  c_float  c;         ///< cost function scaling
  OSQPVectorf *D;     ///< primal variable scaling
  OSQPVectorf *E;     ///< dual variable scaling
  c_float  cinv;      ///< cost function rescaling
  OSQPVectorf *Dinv;  ///< primal variable rescaling
  OSQPVectorf *Einv;  ///< dual variable rescaling
} OSQPScaling;

/**
 * Solution structure
 */
typedef struct {
  c_float *x; ///< primal solution
  c_float *y; ///< Lagrange multiplier associated to \f$l <= Ax <= u\f$
} OSQPSolution;


/**
 * Solver return information
 */
typedef struct {
  c_int iter;          ///< number of iterations taken
  char  status[32];    ///< status string, e.g. 'solved'
  c_int status_val;    ///< status as c_int, defined in constants.h

# ifndef EMBEDDED
  c_int status_polish; ///< polish status: successful (1), unperformed (0), (-1) unsuccessful
# endif // ifndef EMBEDDED

  c_float obj_val;     ///< primal objective
  c_float pri_res;     ///< norm of primal residual
  c_float dua_res;     ///< norm of dual residual

# ifdef PROFILING
  c_float setup_time;  ///< time taken for setup phase (seconds)
  c_float solve_time;  ///< time taken for solve phase (seconds)
  c_float update_time; ///< time taken for update phase (seconds)
  c_float polish_time; ///< time taken for polish phase (seconds)
  c_float run_time;    ///< total time  (seconds)
# endif // ifdef PROFILING

# if EMBEDDED != 1
  c_int   rho_updates;  ///< number of rho updates
  c_float rho_estimate; ///< best rho estimate so far from residuals
# endif // if EMBEDDED != 1
} OSQPInfo;


# ifndef EMBEDDED

/**
 * Polish structure
 */
typedef struct {
  csc *Ared;          ///< active rows of A
  ///<    Ared = vstack[Alow, Aupp]
  c_int    n_low;     ///< number of lower-active rows
  c_int    n_upp;     ///< number of upper-active rows
  OSQPVectori   *A_to_Alow; ///< Maps indices in A to indices in Alow
  OSQPVectori   *A_to_Aupp; ///< Maps indices in A to indices in Aupp
  OSQPVectori   *Alow_to_A; ///< Maps indices in Alow to indices in A
  OSQPVectori   *Aupp_to_A; ///< Maps indices in Aupp to indices in A
  OSQPVectorf *x;         ///< optimal x-solution obtained by polish
  OSQPVectorf *z;         ///< optimal z-solution obtained by polish
  OSQPVectorf *y;         ///< optimal y-solution obtained by polish
  c_float  obj_val;   ///< objective value at polished solution
  c_float  pri_res;   ///< primal residual at polished solution
  c_float  dua_res;   ///< dual residual at polished solution
} OSQPPolish;
# endif // ifndef EMBEDDED


/**********************************
* Main structures and Data Types *
**********************************/

/**
 * Data structure
 */
typedef struct {
  c_int    n; ///< number of variables n
  c_int    m; ///< number of constraints m
  csc     *P; ///< the upper triangular part of the quadratic cost matrix P in csc format (size n x n).
  csc     *A; ///< linear constraints matrix A in csc format (size m x n)
  OSQPVectorf *q; ///< dense array for linear part of cost function (size n)
  OSQPVectorf *l; ///< dense array for lower bound (size m)
  OSQPVectorf *u; ///< dense array for upper bound (size m)
} OSQPData;


/**
 * Settings struct
 */
typedef struct {
  c_float rho;                    ///< ADMM step rho
  c_float sigma;                  ///< ADMM step sigma
  c_int   scaling;                ///< heuristic data scaling iterations; if 0, then disabled.

# if EMBEDDED != 1
  c_int   adaptive_rho;           ///< boolean, is rho step size adaptive?
  c_int   adaptive_rho_interval;  ///< number of iterations between rho adaptations; if 0, then it is automatic
  c_float adaptive_rho_tolerance; ///< tolerance X for adapting rho. The new rho has to be X times larger or 1/X times smaller than the current one to trigger a new factorization.
#  ifdef PROFILING
  c_float adaptive_rho_fraction;  ///< interval for adapting rho (fraction of the setup time)
#  endif // Profiling
# endif // EMBEDDED != 1

  c_int                   max_iter;      ///< maximum number of iterations
  c_float                 eps_abs;       ///< absolute convergence tolerance
  c_float                 eps_rel;       ///< relative convergence tolerance
  c_float                 eps_prim_inf;  ///< primal infeasibility tolerance
  c_float                 eps_dual_inf;  ///< dual infeasibility tolerance
  c_float                 alpha;         ///< relaxation parameter
  enum linsys_solver_type linsys_solver; ///< linear system solver to use

# ifndef EMBEDDED
  c_float delta;                         ///< regularization parameter for polishing
  c_int   polish;                        ///< boolean, polish ADMM solution
  c_int   polish_refine_iter;            ///< number of iterative refinement steps in polishing

  c_int verbose;                         ///< boolean, write out progress
# endif // ifndef EMBEDDED

  c_int scaled_termination;              ///< boolean, use scaled termination criteria
  c_int check_termination;               ///< integer, check termination interval; if 0, then termination checking is disabled
  c_int warm_start;                      ///< boolean, warm start

# ifdef PROFILING
  c_float time_limit;                    ///< maximum number of seconds allowed to solve the problem; if 0, then disabled
# endif // ifdef PROFILING
} OSQPSettings;


/**
 * OSQP Workspace
 */
typedef struct {
  /// Problem data to work on (possibly scaled)
  OSQPData *data;

  /// Linear System solver structure
  LinSysSolver *linsys_solver;

# ifndef EMBEDDED
  /// Polish structure
  OSQPPolish *pol;
# endif // ifndef EMBEDDED

  /**
   * @name Vector used to store a vectorized rho parameter
   * @{
   */
  OSQPVectorf *rho_vec;     ///< vector of rho values
  OSQPVectorf *rho_inv_vec; ///< vector of inv rho values

  /** @} */

# if EMBEDDED != 1
  OSQPVectori *constr_type; ///< Type of constraints: loose (-1), equality (1), inequality (0)
# endif // if EMBEDDED != 1

  /**
   * @name Iterates
   * @{
   */
  OSQPVectorf *x;           ///< Iterate x
  OSQPVectorf *y;           ///< Iterate y
  OSQPVectorf *z;           ///< Iterate z
  OSQPVectorf *xz_tilde;    ///< Iterate xz_tilde
  OSQPVectorf *xtilde_view; ///< xtilde view into xz_tilde
  OSQPVectorf *ztilde_view; ///< ztilde view into xz_tilde

  OSQPVectorf *x_prev;   ///< Previous x

  /**< NB: Used also as workspace vector for dual residual */
  OSQPVectorf *z_prev;   ///< Previous z

  /**< NB: Used also as workspace vector for primal residual */

  /**
   * @name Primal and dual residuals workspace variables
   *
   * Needed for residuals computation, tolerances computation,
   * approximate tolerances computation and adapting rho
   * @{
   */
  OSQPVectorf *Ax;  ///< scaled A * x
  OSQPVectorf *Px;  ///< scaled P * x
  OSQPVectorf *Aty; ///< scaled A * x

  /** @} */

  /**
   * @name Primal infeasibility variables
   * @{
   */
  OSQPVectorf *delta_y;   ///< difference between consecutive dual iterates
  OSQPVectorf *Atdelta_y; ///< A' * delta_y

  /** @} */

  /**
   * @name Dual infeasibility variables
   * @{
   */
  OSQPVectorf *delta_x;  ///< difference between consecutive primal iterates
  OSQPVectorf *Pdelta_x; ///< P * delta_x
  OSQPVectorf *Adelta_x; ///< A * delta_x

  /** @} */

  /**
   * @name Temporary vectors used in scaling
   * @{
   */

  OSQPVectorf *D_temp;   ///< temporary primal variable scaling vectors
  OSQPVectorf *D_temp_A; ///< temporary primal variable scaling vectors storing norms of A columns
  OSQPVectorf *E_temp;   ///< temporary constraints scaling vectors storing norms of A' columns


  /** @} */

  OSQPSettings *settings; ///< problem settings
  OSQPScaling  *scaling;  ///< scaling vectors
  OSQPSolution *solution; ///< problem solution
  OSQPInfo     *info;     ///< solver information

# ifdef PROFILING
  OSQPTimer *timer;       ///< timer object

  /// flag indicating whether the solve function has been run before
  c_int first_run;

  /// flag indicating whether the update_time should be cleared
  c_int clear_update_time;

  /// flag indicating that osqp_update_rho is called from osqp_solve function
  c_int rho_update_from_solve;
# endif // ifdef PROFILING

# ifdef PRINTING
  c_int summary_printed; ///< Has last summary been printed? (true/false)
# endif // ifdef PRINTING

} OSQPWorkspace;


/**
 * Define linsys_solver prototype structure
 *
 * NB: The details are defined when the linear solver is initialized depending
 *      on the choice
 */
struct linsys_solver {
  enum linsys_solver_type type;                 ///< linear system solver type functions
  c_int (*solve)(LinSysSolver *self,
                 c_float      *b);              ///< solve linear system

# ifndef EMBEDDED
  void (*free)(LinSysSolver *self);             ///< free linear system solver (only in desktop version)
# endif // ifndef EMBEDDED

# if EMBEDDED != 1
  c_int (*update_matrices)(LinSysSolver *s,
                           const csc *P,            ///< update matrices P
                           const csc *A);           //   and A in the solver

  c_int (*update_rho_vec)(LinSysSolver  *s,
                          const c_float *rho_vec);  ///< Update rho_vec
# endif // if EMBEDDED != 1

# ifndef EMBEDDED
  c_int nthreads; ///< number of threads active
# endif // ifndef EMBEDDED
};


# ifdef __cplusplus
}
# endif // ifdef __cplusplus

#endif // ifndef OSQP_TYPES_H
