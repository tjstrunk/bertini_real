#include "solver.hpp"





void endgamedata_to_endpoint(post_process_t *endPoint, endgame_data_t *EG){
	
	
	//	printf("endgame2endpoint\n");
	int num_vars, ii;
	endPoint->path_num = EG->pathNum;
	
	endPoint->sol_prec = EG->prec;
	endPoint->cond_est = EG->condition_number;
	endPoint->final_t = EG->t_val_at_latest_sample_point_d;//???
	endPoint->first_increase = EG->first_increase;
	
	
	if (EG->prec==52) {
		num_vars = EG->PD_d.point->size;
		endPoint->sol_d  = (comp_d *)br_malloc(num_vars * sizeof(comp_d));
		endPoint->sol_mp = NULL;
		
		for (ii=0; ii<num_vars; ii++) {
			endPoint->sol_d[ii]->r = EG->PD_d.point->coord[ii].r;
			endPoint->sol_d[ii]->i = EG->PD_d.point->coord[ii].i;
		}
		
		endPoint->size_sol = num_vars;
		endPoint->function_resid_d = EG->function_residual_d;  // the function residual
		endPoint->newton_resid_d = EG->latest_newton_residual_d;
		endPoint->cycle_num = EG->PD_d.cycle_num;
		endPoint->accuracy_estimate = EG->error_at_latest_sample_point_d;//
		
	}
	else
	{
		num_vars = EG->PD_mp.point->size;
		
		endPoint->sol_d  = NULL;
		endPoint->sol_mp = (comp_mp *)br_malloc(num_vars * sizeof(comp_mp));
		for (ii=0; ii<num_vars; ii++) {
			init_mp2(endPoint->sol_mp[ii],EG->prec);
			mpf_set(endPoint->sol_mp[ii]->r,EG->PD_mp.point->coord[ii].r);
			mpf_set(endPoint->sol_mp[ii]->i,EG->PD_mp.point->coord[ii].i);
		}
		endPoint->size_sol = num_vars;
		mpf_init2(endPoint->function_resid_mp, EG->prec); mpf_init2(endPoint->newton_resid_mp, EG->prec);
		
		mpf_set(endPoint->function_resid_mp,EG->function_residual_mp); //this is undoubtedly incorrect
		mpf_set(endPoint->newton_resid_mp,EG->latest_newton_residual_mp);
		endPoint->cycle_num = EG->PD_mp.cycle_num;
		endPoint->accuracy_estimate = mpf_get_d(EG->error_at_latest_sample_point_mp);
	}
	
	
	if (EG->retVal==0) {
		endPoint->success = 1;
	}
	else if (EG->retVal == retVal_sharpening_failed){
		endPoint->success = retVal_sharpening_failed;
	}
	else if (EG->retVal == retVal_sharpening_singular_endpoint){
		endPoint->success = retVal_sharpening_singular_endpoint;
	}
	else{
		std::cout << "setting endPoint->success = -1 because retVal==" << EG->retVal << std::endl;
		endPoint->success = -1;
	}
	
	
	
	endPoint->sol_num = 0; // set up for post-processing
	endPoint->multiplicity = 1;
	endPoint->isFinite = 0;
	//	// the post_process_t structure is used in post-processing //
	//	typedef struct
	//	{
	//		int path_num;     // path number of the solution
	//		int sol_num;      // solution number
	//		comp_d  *sol_d;   // solution
	//		comp_mp *sol_mp;
	//		int sol_prec;     // precision of the solution
	//		int size_sol;     // the number of entries in sol
	//		double function_resid_d;  // the function residual
	//		mpf_t  function_resid_mp;
	//		double cond_est;  // the estimate of the condition number
	//		double newton_resid_d;    // the newton residual
	//		mpf_t  newton_resid_mp;
	//		double final_t;   // the final value of time
	//		double accuracy_estimate; // accuracy estimate between extrapolations
	//		double first_increase;    // time value of the first increase in precision
	//		int cycle_num;    // cycle number used in extrapolations
	//		int success;      // success flag
	//		int multiplicity; // multiplicity
	//		int isReal;       // real flag:  0 - not real, 1 - real
	//		int isFinite;     // finite flag: -1 - no finite/infinite distinction, 0 - infinite, 1 - finite
	//		int isSing;       // singular flag: 0 - non-sigular, 1 - singular
	//	} post_process_t;
	
	
	
	//	typedef struct
	//	{
	//		int prec;
	//		point_data_d PD_d;
	//		point_data_mp PD_mp;
	//
	//		int last_approx_prec;       // precision of the last approximation
	//		point_d last_approx_d;      // last approximation to the end point
	//		point_mp last_approx_mp;    // last approximation to the end point
	//
	//		int retVal;
	//		int pathNum;
	//		int codim;
	//		double first_increase;
	//		double condition_number;
	//		double function_residual_d;
	//		mpf_t  function_residual_mp;
	//		double latest_newton_residual_d;
	//		mpf_t  latest_newton_residual_mp;
	//		double t_val_at_latest_sample_point_d;
	//		mpf_t  t_val_at_latest_sample_point_mp;
	//		double error_at_latest_sample_point_d;
	//		mpf_t  error_at_latest_sample_point_mp;
	//	} endgame_data_t;
	
	
	
	
}

//void findSingSol(post_process_t *endPoints, point_d *dehomPoints_d, point_mp *dehomPoints_mp, int num_sols, int num_vars, preproc_data *PPD, double maxCondNum, double finalTol, int regenToggle)

int BRfindSingularSolns(post_process_t *endPoints, int num_sols, int num_vars,
												tracker_config_t *T ){
	int ii, sing_count=0;
	
	for (ii = 0; ii < num_sols; ii++){
		if ( (endPoints[ii].cond_est >  T->cond_num_threshold) || (endPoints[ii].cond_est < 0.0) )
			endPoints[ii].isSing = 1;
		else
			endPoints[ii].isSing = 0;
		
		if (endPoints[ii].isSing)
		{
			sing_count++;
		}
	}
	
	return sing_count;
}


int BRfindFiniteSolns(post_process_t *endPoints, int num_sols, int num_vars,
											tracker_config_t *T ){
	int ii, jj, finite_count=0;
	
	
	
	//	printf("BR find Finite\n");
	
	//initialize temp stuffs
	comp_d dehom_coord_recip_d;
	comp_mp dehom_coord_recip_mp; init_mp(dehom_coord_recip_mp);
	vec_d dehom_d;   init_vec_d(dehom_d,num_vars-1);   dehom_d->size = num_vars-1;
	vec_mp dehom_mp; init_vec_mp(dehom_mp,num_vars-1); dehom_mp->size = num_vars-1;
	
	
	
	for (ii = 0; ii < num_sols; ii++){
		if (endPoints[ii].sol_prec<64) {
			set_d(dehom_coord_recip_d,endPoints[ii].sol_d[0]);
			recip_d(dehom_coord_recip_d,dehom_coord_recip_d);
			for (jj=0; jj<num_vars-1; ++jj) {
				//do the division.
				mul_d(&dehom_d->coord[jj],dehom_coord_recip_d,endPoints[ii].sol_d[jj])
			}
			
			if (infNormVec_d(dehom_d) < T->finiteThreshold){
				endPoints[ii].isFinite = 1;
				finite_count++;
			}
			else{
				endPoints[ii].isFinite = 0;
			}
			
		}
		else // high precision, do mp
		{
			change_prec_point_mp(dehom_mp,endPoints[ii].sol_prec);
			setprec_mp(dehom_coord_recip_mp,endPoints[ii].sol_prec);
			set_mp(dehom_coord_recip_mp,endPoints[ii].sol_mp[0]);
			for (jj=0; jj<num_vars-1; ++jj) {
				//do the division.
				mul_mp(&dehom_mp->coord[jj],dehom_coord_recip_mp,endPoints[ii].sol_mp[jj])
			}
			
			if (infNormVec_mp(dehom_mp) < T->finiteThreshold){
				endPoints[ii].isFinite = 1;
				finite_count++;
			}
			else{
				endPoints[ii].isFinite = 0;
			}
			
		}
	}
	
	clear_vec_d(dehom_d);
	clear_vec_mp(dehom_mp);
	clear_mp(dehom_coord_recip_mp);
	
	return finite_count;
}




int is_acceptable_solution(post_process_t endPoint, solver_configuration & solve_options){
	int indicator = 1;
	
	if ( (endPoint.multiplicity!=1) && (solve_options.allow_multiplicity==0) ) {
		indicator = 0;
	}
	
	if ( (endPoint.isSing==1) && (solve_options.allow_singular==0) ) {
		indicator = 0;
	}
	
	if ( (endPoint.isFinite!=1) && (solve_options.allow_infinite==0) ) {
		indicator = 0;
	}
	
	if ( (endPoint.success!=1) && (solve_options.allow_unsuccess==0) ) {
		indicator = 0;
	}
	
	
	return indicator;
}


//assumes that W has the number of variables already set, and the pts NOT allocated yet.  should be NULL
void BRpostProcessing(post_process_t *endPoints, witness_set *W_new, int num_pts,
											preproc_data *preProcData, tracker_config_t *T,
											solver_configuration & solve_options)
/***************************************************************\
 * USAGE:                                                        *
 * ARGUMENTS:                                                    *
 * RETURN VALUES:                                                *
 * NOTES: does the actual post processing for a zero dim run     *
 \***************************************************************/
{
	//	printf("BR post processing\n");
	//	options->allow_multiplicity = 0;
	//	options->allow_singular = 0;
	//	options->allow_infinite = 0;
	//	options->allow_unsuccess = 0;
	
	int ii, jj;
	
	
	// sets the multiplicity and solution number in the endPoints data
	//direct from the bertini library:
	findMultSol(endPoints, num_pts, W_new->num_variables, preProcData, T->final_tol_times_mult);
	
	//	printf("post findMultSol\n");
	
	//sets the singularity flag in endPoints.
	//custom, derived from bertini's analagous call.
	int num_singular_solns = BRfindSingularSolns(endPoints, num_pts, W_new->num_variables, T);
	//	printf("post BRfindSingular\n");
	
	//sets the finite flag in endPoints.
	//custom, derived from bertini's analagous call.
	int num_finite_solns = BRfindFiniteSolns(endPoints, num_pts, W_new->num_variables, T);
	//	printf("post BRfindFinite\n");
	
	
	
	if (solve_options.show_status_summary==1) {
		printf("%d singular solutions\n",num_singular_solns);
		printf("%d finite solutions\n",num_finite_solns);
		
		for (ii=0; ii<num_pts; ++ii) {
			//		int success;      // success flag
			//		int multiplicity; // multiplicity
			//		int isReal;       // real flag:  0 - not real, 1 - real
			//		int isFinite;     // finite flag: -1 - no finite/infinite distinction, 0 - infinite, 1 - finite
			//		int isSing;       // singular flag: 0 - non-sigular, 1 - singular
			printf("solution %d, success %d, multi %d, isFinite %d, isSing %d\n",ii,endPoints[ii].success,endPoints[ii].multiplicity,endPoints[ii].isFinite,endPoints[ii].isSing);
		}
	}
	int num_actual_solns = 0;
	int *actual_solns_indices;
	actual_solns_indices = (int *)br_malloc(num_pts*sizeof(int));
	
	
	
	for (ii=0; ii<num_pts; ii++) {
		if ( is_acceptable_solution(endPoints[ii],solve_options) )//determine if acceptable based on current configuration
		{
			actual_solns_indices[num_actual_solns] = ii;
			num_actual_solns++;
		}
	}
	
	//initialize the structures for holding the produced data
	W_new->num_pts=num_actual_solns; W_new->num_pts=num_actual_solns;
	//  W_new->pts_d=(point_d *)br_malloc(num_actual_solns*sizeof(point_d));
  W_new->pts_mp=(point_mp *)br_malloc(num_actual_solns*sizeof(point_mp));
	
	
	
	for (ii=0; ii<num_actual_solns; ++ii) {
		
		//		init_vec_d(W_new->pts_d[ii],W_new->num_variables);
		init_vec_mp(W_new->pts_mp[ii],W_new->num_variables);
		//		W_new->pts_d[ii]->size =
		W_new->pts_mp[ii]->size = W_new->num_variables;
		
		if (endPoints[actual_solns_indices[ii]].sol_prec<64) {
			//copy out of the double structure.
			for (jj=0; jj<W_new->num_variables; jj++) {
				d_to_mp(&W_new->pts_mp[ii]->coord[jj],endPoints[actual_solns_indices[ii]].sol_d[jj]);
			}
			//			vec_d_to_mp(W_new->pts_mp[ii],W_new->pts_d[ii]);
		}
		else{
			//copy out of the mp structure.
			for (jj=0; jj<W_new->num_variables; jj++) {
				set_mp(&W_new->pts_mp[ii]->coord[jj],endPoints[actual_solns_indices[ii]].sol_mp[jj]);
			}
			//			vec_mp_to_d(W_new->pts_d[ii],W_new->pts_mp[ii]);
		}
	}
	
	free(actual_solns_indices);
	
	
	
	
  return;
}








int solver::send()
{
	
	
	int *buffer = new int[4];
	
	buffer[0] = this->num_variables;
	buffer[1] = this->num_steps;
	buffer[2] = this->verbose_level;
	buffer[3] = this->MPType;
	
	MPI_Bcast(buffer, 4, MPI_INT, 0, MPI_COMM_WORLD);
	
	send_preproc_data(&this->preProcData);
	
	int num_SLP;
	if (this->have_SLP) 
		num_SLP = 1;
	else
		num_SLP = 0;
	
	MPI_Bcast(&num_SLP, 1, MPI_INT, 0, MPI_COMM_WORLD);
	
	for (int ii=0; ii<num_SLP; ii++) {
		std::cout << "master bcasting the SLP, MPType" << this->MPType << std::endl;
		bcast_prog_t(this->SLP, MPType, 0, 0);
	}
	
	delete(buffer);
	return SUCCESSFUL;
}

int solver::receive()
{
	
	int *buffer = new int[4];
	

	MPI_Bcast(buffer, 4, MPI_INT, 0, MPI_COMM_WORLD);
	
	this->num_variables = buffer[0];
	this->num_steps = buffer[1];
	this->verbose_level = buffer[2];
	this->MPType = buffer[3];
	
	
	receive_preproc_data(&this->preProcData);
	
	if (this->have_SLP) {
		clearProg(this->SLP, this->MPType, 1);
		this->have_SLP = false;
	}
//	this->SLP = (prog_t *) br_malloc(sizeof(prog_t));
	
	
	int num_SLP = 1;
	MPI_Bcast(&num_SLP, 1, MPI_INT, 0, MPI_COMM_WORLD); // get the number of SLP's to receieve
	
	if (num_SLP>0) {
		prog_t * _SLP = (prog_t *) br_malloc(num_SLP*sizeof(prog_t));
		for (int ii=0; ii<num_SLP; ii++) {
			std::cout << "worker bcasting the SLP, MPType" << this->MPType << std::endl;
			bcast_prog_t(&_SLP[ii], this->MPType, 1, 0); // last two arguments are: myid, headnode
			std::cout << "worker copying the SLP" << std::endl;
			this->SLP = &_SLP[ii];
//			cp_prog_t(this->SLP, &_SLP[ii]);
			std::cout << "worker copied the SLP" << std::endl;
		}
		
		this->have_SLP = true;
		initEvalProg(this->MPType);
	}
	
	
	
	
	delete(buffer);
	return SUCCESSFUL;
}




//////////////
//
//	SOLVER MP
//
////////////

int solver_mp::send()
{
	solver::send();
	
	std::cout << "master sending patch " << std::endl;
	print_matrix_to_screen_matlab(this->patch.patchCoeff,"patchCoeff");
	std::cout << patch.num_patches << " " << patch.curr_prec << std::endl;
	send_patch_mp(&this->patch);
	
	std::cout << "master sent patch_mp" << std::endl;
	
	bcast_comp_mp(this->gamma, 0,0);
	
	int *buffer = new int[2];
	buffer[0] = randomizer_matrix->rows;
	buffer[1] = randomizer_matrix->cols;
	
	MPI_Bcast(buffer, 2, MPI_INT, 0, MPI_COMM_WORLD);
	if (this->MPType==2) {
		bcast_mat_mp(randomizer_matrix_full_prec, 0, 0);
	}
	else{
		print_matrix_to_screen_matlab(randomizer_matrix,"R");
		bcast_mat_mp(randomizer_matrix, 0, 0);
	}
	
	delete[] buffer;
	std::cout << "master sent randomizer matrix" << std::endl;
	buffer = new int[1];
	
	
	buffer[0] = this->curr_prec;
	
	
	MPI_Bcast(buffer, 1, MPI_INT, 0, MPI_COMM_WORLD);
	
	delete[] buffer;
	
	return SUCCESSFUL;
}


int solver_mp::receive()
{
	
	solver::receive();
	
	std::cout << "worker after basic receive" << std::endl;
	
	receive_patch_mp(&this->patch);
	
	print_matrix_to_screen_matlab(patch.patchCoeff,"P");
	
	bcast_comp_mp(this->gamma, 1,0);
	
	int *buffer = new int[2];
	MPI_Bcast(buffer, 2, MPI_INT, 0, MPI_COMM_WORLD);
	
	if (this->MPType==2) {
		init_mat_mp2(randomizer_matrix_full_prec,buffer[0],buffer[1],1024);
		randomizer_matrix_full_prec->rows = buffer[0];
		randomizer_matrix_full_prec->cols = buffer[1];
		
		bcast_mat_mp(randomizer_matrix_full_prec, 1, 0);
		
		init_mat_mp(randomizer_matrix,0,0);
		mat_cp_mp(randomizer_matrix, randomizer_matrix_full_prec);

	}
	else{
		init_mat_mp(randomizer_matrix,buffer[0],buffer[1]);
		randomizer_matrix->rows = buffer[0];
		randomizer_matrix->cols = buffer[1];
		bcast_mat_mp(randomizer_matrix, 1, 0);
	}
	
	delete[] buffer;
	
	
	buffer = new int[1];
	
	MPI_Bcast(buffer, 1, MPI_INT, 0, MPI_COMM_WORLD);
	this->curr_prec = buffer[0];
	
	delete[] buffer;
	return SUCCESSFUL;
}







//////////////
//
//	SOLVER D
//
////////////

int solver_d::send()
{
	solver::send();
	
	send_patch_d(&(this->patch));

	bcast_comp_d(this->gamma, 0,0);
	
	bcast_mat_d(randomizer_matrix, 0, 0);
	
	return SUCCESSFUL;
}


int solver_d::receive()
{
	
	solver::receive();

	receive_patch_d(&this->patch);
	
	bcast_comp_d(this->gamma, 1,0);
	
	bcast_mat_d(randomizer_matrix, 1, 0);
	
	return SUCCESSFUL;
}








//////





void get_tracker_config(solver_configuration & solve_options,int MPType)
{
	
	//necessary for the setupConfig call
	double intrinsicCutoffMultiplier;
	int userHom = 0, useRegen = 0, regenStartLevel = 0, maxCodim = 0, specificCodim = 0, pathMod = 0, reducedOnly = 0, supersetOnly = 0, paramHom = 0;
	//end necessaries for the setupConfig call.
	
	
  setupConfig(&solve_options.T, &solve_options.midpoint_tol, &userHom, &useRegen, &regenStartLevel, &maxCodim, &specificCodim, &pathMod, &intrinsicCutoffMultiplier, &reducedOnly, &supersetOnly, &paramHom, MPType);
	
	return;
}




void solver_init_config(solver_configuration & options){
	options.allow_multiplicity = 0;
	options.allow_singular = 0;
	options.allow_infinite = 0;
	options.allow_unsuccess = 0;
	options.use_midpoint_checker = 1;
	options.show_status_summary = 0;
	options.verbose_level = 0; // default to 0.  higher is more verbose
	options.use_gamma_trick = 0;
	
	options.complete_witness_set = 1;
}


void solver_clear_config(solver_configuration & options){
	//has no fields which require clearing.
	return;
}




/**
 sets the start_pts structure to hold all points in W
 
 \param startPts the value being set.  should be NULL input.
 \param W the witness_set input
 
 */
void generic_set_start_pts(point_data_d ** startPts,
													 witness_set & W)
{
	int ii; // counters
	
	*startPts = (point_data_d *)br_malloc(W.num_pts * sizeof(point_data_d));
	
	for (ii = 0; ii < W.num_pts; ii++)
	{ // setup startPts[ii]
		init_point_data_d(&(*startPts)[ii], W.num_variables); // also performs initialization on the point inside startPts
		change_size_vec_d((*startPts)[ii].point,W.num_variables);
		(*startPts)[ii].point->size = W.num_variables;
		
		//1 set the coordinates
		vec_mp_to_d((*startPts)[ii].point, W.pts_mp[ii]);
		
		//2 set the start time to 1.
		set_one_d((*startPts)[ii].time);
	}
}



void generic_set_start_pts(point_data_mp ** startPts,
													 witness_set & W)
{
	int ii; // counters
	
	(*startPts) = (point_data_mp *)br_malloc(W.num_pts * sizeof(point_data_mp));
	
	for (ii = 0; ii < W.num_pts; ii++)
	{ // setup startPts[ii]
		init_point_data_mp(&(*startPts)[ii], W.num_variables); // also performs initialization on the point inside startPts
		change_size_vec_mp((*startPts)[ii].point,W.num_variables);
		(*startPts)[ii].point->size = W.num_variables;
		
		//1 set the coordinates
		vec_cp_mp((*startPts)[ii].point, W.pts_mp[ii]);
		
		//2 set the start time to 1.
		set_one_mp((*startPts)[ii].time);
	}
}






void generic_tracker_loop(trackingStats *trackCount,
										 FILE * OUT, FILE * MIDOUT,
										 witness_set & W,  // was the startpts file pointer.
										 post_process_t *endPoints,
										 solver_d * ED_d, solver_mp * ED_mp,
										 solver_configuration & solve_options)
{
	
	
	
	

	
	int (*curr_eval_d)(point_d, point_d, vec_d, mat_d, mat_d, point_d, comp_d, void const *) = NULL;
  int (*curr_eval_mp)(point_mp, point_mp, vec_mp, mat_mp, mat_mp, point_mp, comp_mp, void const *) = NULL;
	int (*change_prec)(void const *, int) = NULL;
	int (*find_dehom)(point_d, point_mp, int *, point_d, point_mp, int, void const *, void const *) = NULL;
	
	switch (solve_options.T.MPType) {
		case 0:
			curr_eval_d = ED_d->evaluator_function_d;
			curr_eval_mp = ED_d->evaluator_function_mp;
			change_prec = ED_d->precision_changer;
			find_dehom = ED_d->dehomogenizer;
			break;

		default:
			curr_eval_d = ED_mp->evaluator_function_d;
			curr_eval_mp = ED_mp->evaluator_function_mp;
			change_prec = ED_mp->precision_changer;
			find_dehom = ED_mp->dehomogenizer;
			break;
			
	}

	
	
	
	point_data_d *startPts_d = NULL;
	generic_set_start_pts(&startPts_d, W);
	
	point_data_mp *startPts_mp = NULL;
	generic_set_start_pts(&startPts_mp, W);
	
	solve_options.T.endgameOnly = 0;
	
	
  // setup the rest of the structures
	endgame_data_t EG; //this will hold the temp solution data produced for each individual track
	init_endgame_data(&EG, solve_options.T.Precision);
	
	
	
	
	trackCount->numPoints = W.num_pts;
	int solution_counter = 0;
	
	
	
	// track each of the start points
	
	for (int ii = 0; ii < W.num_pts; ii++)
	{
		if (solve_options.verbose_level>=0)
			printf("tracking path %d of %d\n",ii,W.num_pts);
		
		
		
//		if (!check_isstart_nullspacejac_d(startPts[ii].point,
//																			T,
//																			ED_d))
//		{
//			std::cout << "trying to start from a non-start-point\n";
//			mypause();
//		}
		
		if (solve_options.T.MPType==2) {
			ED_mp->curr_prec = 64;
		}
		
		
		// track the path
		generic_track_path(solution_counter, &EG,
											 &startPts_d[ii], &startPts_mp[ii],
											 OUT, MIDOUT,
											 &solve_options.T, ED_d, ED_mp,
											 curr_eval_d, curr_eval_mp, change_prec, find_dehom);
		
		
		
		// check to see if it should be sharpened
		if (EG.retVal == 0 && solve_options.T.sharpenDigits > 0)
		{ // use the sharpener for after an endgame
			sharpen_endpoint_endgame(&EG, &solve_options.T, OUT, ED_d, ED_mp, curr_eval_d, curr_eval_mp, change_prec);
		}
		
		
		
		int issoln;
		
		switch (solve_options.T.MPType) {
			case 0:
					issoln = ED_d->is_solution_checker_d(&EG,  &solve_options.T, ED_d);

				break;
				
			default:
				
				if (EG.prec<64){
					issoln = ED_mp->is_solution_checker_d(&EG,  &solve_options.T, ED_d); }
				else {
					issoln = ED_mp->is_solution_checker_mp(&EG, &solve_options.T, ED_mp); }
				break;
		}
		
		
		
		//get the terminal time in double form
		comp_d time_to_compare;
		if (EG.prec < 64) {
			set_d(time_to_compare,EG.PD_d.time);}
		else {
			mp_to_d(time_to_compare, EG.PD_mp.time); }
		
		
		if ((EG.retVal != 0 && time_to_compare->r > solve_options.T.minTrackT) || !issoln) {  // <-- this is the real indicator of failure...
			
			trackCount->failures++;
			
			printf("\nthere was a path failure nullspace_left tracking witness point %d\nretVal = %d; issoln = %d\n",ii,EG.retVal, issoln);
			
			print_path_retVal_message(EG.retVal);
			
			if (solve_options.verbose_level > 0) {
				if (EG.prec < 64)
					print_point_to_screen_matlab(EG.PD_d.point,"bad_terminal_point");
				else
					print_point_to_screen_matlab(EG.PD_mp.point,"bad_terminal_point");
			}
			
		}
		else
		{
			//otherwise converged, but may have still had non-zero retval due to other reasons.
			endgamedata_to_endpoint(&endPoints[solution_counter], &EG);
			trackCount->successes++;
			solution_counter++; // probably this could be eliminated
		}
		
	}// re: for (ii=0; ii<W.num_pts ;ii++)
	
	
	
	//clear the data structures.
  for (int ii = 0; ii >W.num_pts; ii++)
  { // clear startPts[ii]
    clear_point_data_d(&startPts_d[ii]);
		clear_point_data_mp(&startPts_mp[ii]);
  }
  free(startPts_d);
	free(startPts_mp);

	
	
}



void generic_tracker_loop_master(trackingStats *trackCount,
																 FILE * OUT, FILE * MIDOUT,
																 witness_set & W,  // was the startpts file pointer.
																 post_process_t *endPoints,
																 solver_d * ED_d, solver_mp * ED_mp,
																 solver_configuration & solve_options)
{
	
	
	
	
	
	
	int (*curr_eval_d)(point_d, point_d, vec_d, mat_d, mat_d, point_d, comp_d, void const *) = NULL;
  int (*curr_eval_mp)(point_mp, point_mp, vec_mp, mat_mp, mat_mp, point_mp, comp_mp, void const *) = NULL;
	int (*change_prec)(void const *, int) = NULL;
	int (*find_dehom)(point_d, point_mp, int *, point_d, point_mp, int, void const *, void const *) = NULL;
	
	switch (solve_options.T.MPType) {
		case 0:
			curr_eval_d = ED_d->evaluator_function_d;
			curr_eval_mp = ED_d->evaluator_function_mp;
			change_prec = ED_d->precision_changer;
			find_dehom = ED_d->dehomogenizer;
			break;
			
		default:
			curr_eval_d = ED_mp->evaluator_function_d;
			curr_eval_mp = ED_mp->evaluator_function_mp;
			change_prec = ED_mp->precision_changer;
			find_dehom = ED_mp->dehomogenizer;
			break;
			
	}
	
	
	point_data_d *startPts_d = NULL;
	point_data_mp *startPts_mp = NULL;
	
	switch (solve_options.T.MPType) {
		case 1:
			generic_set_start_pts(&startPts_mp, W); 
			break;
			
		default:
			generic_set_start_pts(&startPts_d, W);
			break;
	}
	
	
	
	
	
	solve_options.T.endgameOnly = 0;
	
	
	// setup the rest of the structures
	endgame_data_t *EG_receives = (endgame_data_t *) br_malloc(1*sizeof(endgame_data_t)); //this will hold the temp solution data produced for each individual track
	init_endgame_data(&EG_receives[0], solve_options.T.Precision);
	
	
	int *indices_outgoing= (int *) br_malloc(sizeof(int));
	int max_outgoing = 1;
	int max_incoming = 1;
	
	
	
	trackCount->numPoints = W.num_pts;
	int solution_counter = 0;
	
	
	
	// track each of the start points

	
	int next_index = 0;
	while (next_index < W.num_pts)
	{
		
		int num_packets =1;
		
		int next_worker = solve_options.activate_next_worker();
		std::cout << "master sending " << num_packets << " packets." << std::endl;
		MPI_Send(&num_packets, 1, MPI_INT, next_worker, NUMPACKETS, MPI_COMM_WORLD);
		
		
		
		if (num_packets > max_outgoing) {
			indices_outgoing = (int *) br_realloc(indices_outgoing,num_packets*sizeof(int));
		}
		for (int ii=0; ii<num_packets; ii++) {
			indices_outgoing[ii] = next_index;
			next_index++;
		}
		
		std::cout << "master sending indices: ";
		for (int ii=0; ii<num_packets; ii++) {
			std::cout << indices_outgoing[ii] << " ";
		}
		std::cout << std::endl;
		MPI_Send(indices_outgoing, num_packets, MPI_INT, next_worker, INDICES, MPI_COMM_WORLD);
		
		
		
		for (int ii=indices_outgoing[0]; ii<=indices_outgoing[num_packets-1]; ii++) {
			if (solve_options.T.MPType==1) {
				send_vec_mp( startPts_mp[ii].point, next_worker);

			}
			else
			{
				send_vec_d( startPts_d[ii].point, next_worker);

			}
		}
		
		
		
		//now to receive data
		int num_incoming;
		MPI_Status statty_mc_gatty;
		std::cout << "master trying to receive from any source the number of packets" << std::endl;
		MPI_Recv(&num_incoming, 1, MPI_INT, MPI_ANY_SOURCE, NUMPACKETS, MPI_COMM_WORLD, &statty_mc_gatty);
		
		std::cout << "master will receive " << num_incoming << " max currently " << max_incoming << std::endl;
		if (num_incoming > max_incoming) {
			EG_receives = (endgame_data_t *) br_realloc(EG_receives, num_incoming * sizeof(endgame_data_t));
			for (int ii=max_incoming; ii<num_incoming; ii++) {
				init_endgame_data(&EG_receives[ii], solve_options.T.Precision);
			}
			max_incoming = num_incoming;
		}
		
		int incoming_id = send_recv_endgame_data_t(&EG_receives, &num_incoming, solve_options.T.MPType, MPI_ANY_SOURCE, 0); // the trailing 0 indicates receiving
		
		std::cout << "master receieved " << num_incoming << " packets." << std::endl;
		solve_options.deactivate(statty_mc_gatty.MPI_SOURCE);
		
		for (int ii=0; ii<num_incoming; ii++) {
			int issoln;
			
			switch (solve_options.T.MPType) {
				case 0:
					issoln = ED_d->is_solution_checker_d(&EG_receives[ii],  &solve_options.T, ED_d);
					
					break;
					
				default:
					
					if (EG_receives[ii].prec<64){
						issoln = ED_mp->is_solution_checker_d(&EG_receives[ii],  &solve_options.T, ED_d); } // this function call is a reference!
					else {
						issoln = ED_mp->is_solution_checker_mp(&EG_receives[ii], &solve_options.T, ED_mp); } // this function call is a reference!
					break;
			}
			
			
			
			//get the terminal time in double form
			comp_d time_to_compare;
			if (EG_receives[ii].prec < 64) {
				set_d(time_to_compare,EG_receives[ii].PD_d.time);}
			else {
				mp_to_d(time_to_compare, EG_receives[ii].PD_mp.time); }
			
			
			if ((EG_receives[ii].retVal != 0 && time_to_compare->r > solve_options.T.minTrackT) || !issoln) {  // <-- this is the real indicator of failure...
				
				trackCount->failures++;
				
				printf("\nthere was a path failure nullspace_left tracking witness point %d\nretVal = %d; issoln = %d\n",EG_receives[ii].pathNum, EG_receives[ii].retVal, issoln);
				
				print_path_retVal_message(EG_receives[ii].retVal);
				
				if (solve_options.verbose_level > 0) {
					if (EG_receives[ii].prec < 64)
						print_point_to_screen_matlab(EG_receives[ii].PD_d.point,"bad_terminal_point");
					else
						print_point_to_screen_matlab(EG_receives[ii].PD_mp.point,"bad_terminal_point");
				}
				
			}
			else
			{
				std::cout << "master converting point " << EG_receives[ii].pathNum << " to endpoint" << std::endl;
				//otherwise converged, but may have still had non-zero retval due to other reasons.
				endgamedata_to_endpoint(&endPoints[solution_counter], &EG_receives[ii]);
				trackCount->successes++;
				solution_counter++; // probably this could be eliminated
			}
		}
	}// re: for (ii=0; ii<W.num_pts ;ii++)
	
	solve_options.send_all_available(0);
	
	
	
	//clear the data structures.
	switch (solve_options.T.MPType) {
		case 1:
			for (int ii = 0; ii >W.num_pts; ii++)
				clear_point_data_mp(&startPts_mp[ii]);
			free(startPts_mp);
			break;
			
		default:
			for (int ii = 0; ii >W.num_pts; ii++)
				clear_point_data_d(&startPts_d[ii]);
			free(startPts_d);
			break;
	}
  
	

	
	
}


void generic_tracker_loop_worker(trackingStats *trackCount,
																 FILE * OUT, FILE * MIDOUT,
																 solver_d * ED_d, solver_mp * ED_mp,
																 solver_configuration & solve_options)
{
	
	
	
	
	
	
	int (*curr_eval_d)(point_d, point_d, vec_d, mat_d, mat_d, point_d, comp_d, void const *) = NULL;
  int (*curr_eval_mp)(point_mp, point_mp, vec_mp, mat_mp, mat_mp, point_mp, comp_mp, void const *) = NULL;
	int (*change_prec)(void const *, int) = NULL;
	int (*find_dehom)(point_d, point_mp, int *, point_d, point_mp, int, void const *, void const *) = NULL;
	
	switch (solve_options.T.MPType) {
		case 0:
			curr_eval_d = ED_d->evaluator_function_d;
			curr_eval_mp = ED_d->evaluator_function_mp;
			change_prec = ED_d->precision_changer;
			find_dehom = ED_d->dehomogenizer;
			break;
			
		default:
			curr_eval_d = ED_mp->evaluator_function_d;
			curr_eval_mp = ED_mp->evaluator_function_mp;
			change_prec = ED_mp->precision_changer;
			find_dehom = ED_mp->dehomogenizer;
			break;
			
	}
	
	
	
	
	point_data_d *startPts_d;
	point_data_mp *startPts_mp;
	
	switch (solve_options.T.MPType) {
		case 1:
			startPts_mp = (point_data_mp *) br_malloc(1*sizeof(point_data_mp));
			init_point_data_mp(&startPts_mp[0], ED_mp->num_variables);
			break;
			
		default:
			startPts_d = (point_data_d *) br_malloc(1*sizeof(point_data_d));
			init_point_data_d(&startPts_d[0], ED_d->num_variables);
			break;
	}
	
	
  // setup the rest of the structures
	endgame_data_t * EG = (endgame_data_t *) br_malloc(1*sizeof(endgame_data_t)); //this will hold the temp solution data produced for each individual track
	init_endgame_data(&EG[0], solve_options.T.Precision);
	
	int *indices_incoming = (int *) br_malloc(1*sizeof(int));
	
	MPI_Status statty_mc_gatty;
	int max_num_allocated = 1;
	
	int numStartPts = 1;
	
	
	while (1)
	{
		
		MPI_Recv(&numStartPts, 1, MPI_INT, solve_options.head(), NUMPACKETS, MPI_COMM_WORLD, &statty_mc_gatty);
		// recv next set of start points
		
		if (numStartPts==0) {
			break;
		}
		
		if (numStartPts<max_num_allocated) {
			switch (solve_options.T.MPType) {
				case 1:
					startPts_mp = (point_data_mp *) br_realloc(startPts_mp, numStartPts*sizeof(point_data_mp));
					break;
					
				default:
					startPts_d = (point_data_d *) br_realloc(startPts_d, numStartPts*sizeof(point_data_d));
					break;
			}
			
			indices_incoming = (int *) br_realloc(indices_incoming, numStartPts*sizeof(int));
			
			max_num_allocated = numStartPts;
		}
		
		MPI_Recv(indices_incoming, numStartPts, MPI_INT, solve_options.head(), INDICES, MPI_COMM_WORLD, &statty_mc_gatty);
		
		for (int ii=0; ii<numStartPts; ii++) {
			switch (solve_options.T.MPType) {
				case 1:
					receive_vec_mp(startPts_mp[ii].point, solve_options.head());
					set_one_mp(startPts_mp[ii].time);
					break;
					
				default:
					receive_vec_d(startPts_d[ii].point, solve_options.head());
					set_one_d(startPts_d[ii].time);
					break;
			}
		}

		
		// track each of the start points
		for (int ii = 0; ii < numStartPts; ii++)
		{
			int current_index = indices_incoming[ii];;
			
			if (solve_options.verbose_level>=0)
				printf("tracking path %d, worker %d\n", current_index, solve_options.id());
			
			
			if (solve_options.T.MPType==2) {
				ED_mp->curr_prec = 64;
			}
			
			
			// track the path
			generic_track_path(indices_incoming[ii], &EG[ii],
												 &startPts_d[ii], &startPts_mp[ii],
												 OUT, MIDOUT,
												 &solve_options.T, ED_d, ED_mp,
												 curr_eval_d, curr_eval_mp, change_prec, find_dehom);
			
			
			
			// check to see if it should be sharpened
			if (EG[ii].retVal == 0 && solve_options.T.sharpenDigits > 0)
			{ // use the sharpener for after an endgame
				sharpen_endpoint_endgame(&EG[ii], &solve_options.T, OUT, ED_d, ED_mp, curr_eval_d, curr_eval_mp, change_prec);
			}
			
		}// re: for (ii=0; ii<W.num_pts ;ii++)


		MPI_Send(&numStartPts, 1, MPI_INT, solve_options.head(), NUMPACKETS, MPI_COMM_WORLD);
		send_recv_endgame_data_t(&EG, &numStartPts, solve_options.T.MPType, solve_options.head(), 1);
		
		
	}

	
	switch (solve_options.T.MPType) {
		case 1:
			for (int ii=0; ii<max_num_allocated; ii++) {
				clear_point_data_mp(&startPts_mp[ii]);
				clear_endgame_data(&EG[ii]);
			}
			break;
			
		default:
			for (int ii=0; ii<max_num_allocated; ii++) {
				clear_point_data_d(&startPts_d[ii]);
				clear_endgame_data(&EG[ii]);
			}
			break;
	}
	
	free(indices_incoming);
}





void generic_track_path(int pathNum, endgame_data_t *EG_out,
												point_data_d *Pin, point_data_mp *Pin_mp,
												FILE *OUT, FILE *MIDOUT,
												tracker_config_t *T,
												void const *ED_d, void const *ED_mp,
												int (*eval_func_d)(point_d, point_d, vec_d, mat_d, mat_d, point_d, comp_d, void const *),
												int (*eval_func_mp)(point_mp, point_mp, vec_mp, mat_mp, mat_mp, point_mp, comp_mp, void const *),
												int (*change_prec)(void const *, int),
												int (*find_dehom)(point_d, point_mp, int *, point_d, point_mp, int, void const *, void const *))
{
	
	
	EG_out->pathNum = pathNum;
	EG_out->codim = 0; // this is ignored
	
	T->first_step_of_path = 1;
	
	if (T->MPType == 2)
	{ // track using AMP
		EG_out->prec = EG_out->last_approx_prec = 52;
		
		EG_out->retVal = endgame_amp(T->endgameNumber, EG_out->pathNum, &EG_out->prec, &EG_out->first_increase, &EG_out->PD_d, &EG_out->PD_mp, &EG_out->last_approx_prec, EG_out->last_approx_d, EG_out->last_approx_mp, Pin, T, OUT, MIDOUT, ED_d, ED_mp, eval_func_d, eval_func_mp, change_prec, find_dehom);
		
		if (EG_out->prec == 52)
		{ // copy over values in double precision
			EG_out->latest_newton_residual_d = T->latest_newton_residual_d;
			EG_out->t_val_at_latest_sample_point_d = T->t_val_at_latest_sample_point;
			EG_out->error_at_latest_sample_point_d = T->error_at_latest_sample_point;
			findFunctionResidual_conditionNumber_d(&EG_out->function_residual_d, &EG_out->condition_number, &EG_out->PD_d, ED_d, eval_func_d);
		}
		else
		{ // make sure that the other MP things are set to the correct precision
			mpf_clear(EG_out->function_residual_mp);
			mpf_init2(EG_out->function_residual_mp, EG_out->prec);
			
			mpf_clear(EG_out->latest_newton_residual_mp);
			mpf_init2(EG_out->latest_newton_residual_mp, EG_out->prec);
			
			mpf_clear(EG_out->t_val_at_latest_sample_point_mp);
			mpf_init2(EG_out->t_val_at_latest_sample_point_mp, EG_out->prec);
			
			mpf_clear(EG_out->error_at_latest_sample_point_mp);
			mpf_init2(EG_out->error_at_latest_sample_point_mp, EG_out->prec);
			
			// copy over the values
			mpf_set(EG_out->latest_newton_residual_mp, T->latest_newton_residual_mp);
			mpf_set_d(EG_out->t_val_at_latest_sample_point_mp, T->t_val_at_latest_sample_point);
			mpf_set_d(EG_out->error_at_latest_sample_point_mp, T->error_at_latest_sample_point);
			findFunctionResidual_conditionNumber_mp(EG_out->function_residual_mp, &EG_out->condition_number, &EG_out->PD_mp, ED_mp, eval_func_mp);
		}
	}
	else if (T->MPType == 0)
	{ // track using double precision
		EG_out->prec = EG_out->last_approx_prec = 52;
		
		EG_out->retVal = endgame_d(T->endgameNumber, EG_out->pathNum, &EG_out->PD_d, EG_out->last_approx_d, Pin, T, OUT, MIDOUT, ED_d, eval_func_d, find_dehom);  // WHERE THE ACTUAL TRACKING HAPPENS
		EG_out->first_increase = 0;
		// copy over values in double precision
		EG_out->latest_newton_residual_d = T->latest_newton_residual_d;
		EG_out->t_val_at_latest_sample_point_d = T->t_val_at_latest_sample_point;
		EG_out->error_at_latest_sample_point_d = T->error_at_latest_sample_point;
		findFunctionResidual_conditionNumber_d(&EG_out->function_residual_d, &EG_out->condition_number, &EG_out->PD_d, ED_d, eval_func_d);
	}
	else if (T->MPType == 1)
	{
		EG_out->pathNum = pathNum;
		EG_out->codim = 0; // zero dimensional - this is ignored
		
		T->first_step_of_path = 1;
		
		// track using MP
		EG_out->retVal = endgame_mp(T->endgameNumber, EG_out->pathNum, &EG_out->PD_mp, EG_out->last_approx_mp, Pin_mp, T, OUT, MIDOUT, ED_mp, eval_func_mp, find_dehom);
		
		
		EG_out->prec = EG_out->last_approx_prec = T->Precision;
		EG_out->first_increase = 0;
		
		// copy over the values
		mpf_set(EG_out->latest_newton_residual_mp, T->latest_newton_residual_mp);
		mpf_set_d(EG_out->t_val_at_latest_sample_point_mp, T->t_val_at_latest_sample_point);
		mpf_set_d(EG_out->error_at_latest_sample_point_mp, T->error_at_latest_sample_point);
		findFunctionResidual_conditionNumber_mp(EG_out->function_residual_mp, &EG_out->condition_number, &EG_out->PD_mp, ED_mp, eval_func_mp);
		
	}
	
	
	
	return;
}




void generic_track_path_d(int pathNum, endgame_data_t *EG_out,
													point_data_d *Pin,
													FILE *OUT, FILE *MIDOUT, tracker_config_t *T,
													void const *ED_d, void const *ED_mp,
													int (*eval_func_d)(point_d, point_d, vec_d, mat_d, mat_d, point_d, comp_d, void const *),
													int (*eval_func_mp)(point_mp, point_mp, vec_mp, mat_mp, mat_mp, point_mp, comp_mp, void const *),
													int (*change_prec)(void const *, int),
													int (*find_dehom)(point_d, point_mp, int *, point_d, point_mp, int, void const *, void const *))
{
	
	
	
	EG_out->pathNum = pathNum;
	EG_out->codim = 0; // this is ignored
	
	T->first_step_of_path = 1;
	
	if (T->MPType == 2)
	{ // track using AMP
		EG_out->prec = EG_out->last_approx_prec = 52;
		
		EG_out->retVal = endgame_amp(T->endgameNumber, EG_out->pathNum, &EG_out->prec, &EG_out->first_increase, &EG_out->PD_d, &EG_out->PD_mp, &EG_out->last_approx_prec, EG_out->last_approx_d, EG_out->last_approx_mp, Pin, T, OUT, MIDOUT, ED_d, ED_mp, eval_func_d, eval_func_mp, change_prec, find_dehom);
		
		if (EG_out->prec == 52)
		{ // copy over values in double precision
			EG_out->latest_newton_residual_d = T->latest_newton_residual_d;
			EG_out->t_val_at_latest_sample_point_d = T->t_val_at_latest_sample_point;
			EG_out->error_at_latest_sample_point_d = T->error_at_latest_sample_point;
			findFunctionResidual_conditionNumber_d(&EG_out->function_residual_d, &EG_out->condition_number, &EG_out->PD_d, ED_d, eval_func_d);
		}
		else
		{ // make sure that the other MP things are set to the correct precision
			mpf_clear(EG_out->function_residual_mp);
			mpf_init2(EG_out->function_residual_mp, EG_out->prec);
			
			mpf_clear(EG_out->latest_newton_residual_mp);
			mpf_init2(EG_out->latest_newton_residual_mp, EG_out->prec);
			
			mpf_clear(EG_out->t_val_at_latest_sample_point_mp);
			mpf_init2(EG_out->t_val_at_latest_sample_point_mp, EG_out->prec);
			
			mpf_clear(EG_out->error_at_latest_sample_point_mp);
			mpf_init2(EG_out->error_at_latest_sample_point_mp, EG_out->prec);
			
			// copy over the values
			mpf_set(EG_out->latest_newton_residual_mp, T->latest_newton_residual_mp);
			mpf_set_d(EG_out->t_val_at_latest_sample_point_mp, T->t_val_at_latest_sample_point);
			mpf_set_d(EG_out->error_at_latest_sample_point_mp, T->error_at_latest_sample_point);
			findFunctionResidual_conditionNumber_mp(EG_out->function_residual_mp, &EG_out->condition_number, &EG_out->PD_mp, ED_mp, eval_func_mp);
		}
	}
	else if (T->MPType == 0)
	{ // track using double precision
		EG_out->prec = EG_out->last_approx_prec = 52;
		
		EG_out->retVal = endgame_d(T->endgameNumber, EG_out->pathNum, &EG_out->PD_d, EG_out->last_approx_d, Pin, T, OUT, MIDOUT, ED_d, eval_func_d, find_dehom);  // WHERE THE ACTUAL TRACKING HAPPENS
		EG_out->first_increase = 0;
		// copy over values in double precision
		EG_out->latest_newton_residual_d = T->latest_newton_residual_d;
		EG_out->t_val_at_latest_sample_point_d = T->t_val_at_latest_sample_point;
		EG_out->error_at_latest_sample_point_d = T->error_at_latest_sample_point;
		findFunctionResidual_conditionNumber_d(&EG_out->function_residual_d, &EG_out->condition_number, &EG_out->PD_d, ED_d, eval_func_d);
	}
	
	
	
	return;
}




void generic_track_path_mp(int pathNum, endgame_data_t *EG_out,
													 point_data_mp *Pin,
													 FILE *OUT, FILE *MIDOUT, tracker_config_t *T,
													 void const *ED,
													 int (*eval_func_mp)(point_mp, point_mp, vec_mp, mat_mp, mat_mp, point_mp, comp_mp, void const *),
													 int (*change_prec)(void const *, int),
													 int (*find_dehom)(point_d, point_mp, int *, point_d, point_mp, int, void const *, void const *))
{
	
	EG_out->pathNum = pathNum;
  EG_out->codim = 0; // zero dimensional - this is ignored
	
  T->first_step_of_path = 1;
	
  // track using MP
  EG_out->retVal = endgame_mp(T->endgameNumber, EG_out->pathNum, &EG_out->PD_mp, EG_out->last_approx_mp, Pin, T, OUT, MIDOUT, ED, eval_func_mp, find_dehom);
	
	
  EG_out->prec = EG_out->last_approx_prec = T->Precision;
  EG_out->first_increase = 0;
	
  // copy over the values
  mpf_set(EG_out->latest_newton_residual_mp, T->latest_newton_residual_mp);
  mpf_set_d(EG_out->t_val_at_latest_sample_point_mp, T->t_val_at_latest_sample_point);
  mpf_set_d(EG_out->error_at_latest_sample_point_mp, T->error_at_latest_sample_point);
  findFunctionResidual_conditionNumber_mp(EG_out->function_residual_mp, &EG_out->condition_number, &EG_out->PD_mp, ED, eval_func_mp);
	
  return;
}





void generic_setup_patch(patch_eval_data_d *P, const witness_set & W)
{
	int ii;
	P->num_patches = W.num_patches;
	init_mat_d(P->patchCoeff, W.num_patches, W.num_variables);
	P->patchCoeff->rows = W.num_patches; P->patchCoeff->cols = W.num_variables;
	
	int varcounter = 0;
	for (int jj=0; jj<W.num_patches; jj++) {
		for (ii=0; ii<varcounter; ii++) {
			set_zero_d(&P->patchCoeff->entry[jj][ii]);
		}
		
		int offset = varcounter;
		for (ii = 0; ii < W.patch_mp[jj]->size ; ii++){
			mp_to_d(&P->patchCoeff->entry[jj][ii+offset],&W.patch_mp[jj]->coord[ii]);
			varcounter++;
		}
		
		for (ii=varcounter; ii<W.num_variables; ii++) {
			set_zero_d(&P->patchCoeff->entry[jj][ii]);
		}
	}
}


void generic_setup_patch(patch_eval_data_mp *P, const witness_set & W)
{
	int ii;
	init_mat_rat(P->patchCoeff_rat, W.num_patches, W.num_variables);
	
	init_mat_mp2(P->patchCoeff, W.num_patches, W.num_variables, mpf_get_default_prec());
	P->curr_prec = mpf_get_default_prec();
	P->num_patches = W.num_patches;
	P->patchCoeff->rows = W.num_patches;
	P->patchCoeff->cols = W.num_variables;
	

	
	
	if (W.num_patches==0) {
		std::cerr << "the number of patches in input W is 0.  this is not allowed, the number must be positive.\n" << std::endl;
		deliberate_segfault();
	}
	
	
	int varcounter = 0;
	for (int jj=0; jj<W.num_patches; jj++) {
		
		for (ii=0; ii<varcounter; ii++) {
			set_zero_mp(&P->patchCoeff->entry[jj][ii]);
			mp_to_rat(P->patchCoeff_rat[jj][ii], &P->patchCoeff->entry[jj][ii]);
		}
		
		int offset = varcounter;
		for (ii = 0; ii < W.patch_mp[jj]->size; ii++){
			set_mp(&P->patchCoeff->entry[jj][ii+offset],&W.patch_mp[jj]->coord[ii]);
			mp_to_rat(P->patchCoeff_rat[jj][ii+offset],
								&P->patchCoeff->entry[jj][ii+offset]);
			varcounter++;
		}
		
		for (ii=varcounter; ii<W.num_variables; ii++) {
			set_zero_mp(&P->patchCoeff->entry[jj][ii]);
			mp_to_rat(P->patchCoeff_rat[jj][ii], &P->patchCoeff->entry[jj][ii]);
		}
	}
}



int generic_setup_files(FILE ** OUT, boost::filesystem::path outname,
												FILE ** MIDOUT, boost::filesystem::path midname)
{
	
	*OUT = safe_fopen_write(outname);  // open the main output files.
  *MIDOUT = safe_fopen_write(midname);
	
	return SUCCESSFUL;
}

void get_projection(vec_mp *pi,
										BR_configuration program_options,
										solver_configuration solve_options,
										int num_vars,
										int num_projections)
{
	
	int ii,jj;
	for (ii=0; ii<num_projections; ii++) {
		change_size_vec_mp(pi[ii], num_vars);  pi[ii]->size = num_vars;
	}
	
	
	
	//assumes the vector pi is already initialized
	if (program_options.user_projection==1) {
		FILE *IN = safe_fopen_read(program_options.projection_filename.c_str()); // we are already assured this file exists, but safe fopen anyway.
		int tmp_num_vars;
		fscanf(IN,"%d",&tmp_num_vars); scanRestOfLine(IN);
		if (tmp_num_vars!=num_vars-1) {
			printf("the number of variables appearing in the projection\nis not equal to the number of non-homogeneous variables in the problem\n");
			printf("please modify file to have %d coordinate pairs.\n",num_vars-1);
			abort();
		}
		
		for (ii=0; ii<num_projections; ii++) {
			set_zero_mp(&pi[ii]->coord[0]);
			for (jj=1; jj<num_vars; jj++) {
				mpf_inp_str(pi[ii]->coord[jj].r, IN, 10);
				mpf_inp_str(pi[ii]->coord[jj].i, IN, 10);
				scanRestOfLine(IN);
			}
		}
		fclose(IN);
	}
	else{
		for (ii=0; ii<num_projections; ii++) {
			set_zero_mp(&pi[ii]->coord[0]);
			for (jj=1; jj<num_vars; jj++)
				get_comp_rand_real_mp(&pi[ii]->coord[jj]);//, &temp_getter->entry[ii][jj-1]);

		}
//		mat_mp temp_getter;
//		init_mat_mp2(temp_getter,0,0,1024);
//		make_matrix_random_real_mp(temp_getter,num_projections, num_vars-1, 1024); // this matrix is ~orthogonal
//		
//		for (ii=0; ii<num_projections; ii++) {
//			set_zero_mp(&pi[ii]->coord[0]);
//			for (jj=1; jj<num_vars; jj++)
//				set_mp(&pi[ii]->coord[jj], &temp_getter->entry[ii][jj-1]);
//			
//		}
		
//		clear_mat_mp(temp_getter);
	}
	
	return;
}






