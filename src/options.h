#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#include "SpiceUsr.h"
#include "propagator.h"
#define N_SURFACES 100
#define N_STEPS 100

typedef struct {
  double et_final_epoch;
  double et_initial_epoch;
  int swpc_final_epoch_more_recent_than_45_days_from_current_day; // if the user chose swpc for the f107 and ap, and that the final epoch is beyong +45 days in the future (limit of swpc predictions) then values of ap and f107 beyong +45 days are set to constant, equal to the mean of the predicted values by SWPC (over the 45 days)
  char oldest_tle_epoch[300];
  double et_oldest_tle_epoch;
  int file_is_quaternion;
  double **quaternion;
  double density_mod; // factor to apply on density at position of satellite (calculated by NRLMSIS, gitm or from density file)
  //newstructure
  int new_cd;
  char filename_kalman_init[1000];
  char path_to_spice[256];
  //newstructure
  int use_kalman;
  
  int one_file_with_tle_of_all_sc;
  int first_run_to_find_tca_before_collision_assessment;
  char test_omniweb_or_external_file[256];
  int aaa_sigma;
  int aaa_mod;
  double min_dist_collision;
  double min_dist_close_approach;
  double ***covariance_matrix;
  double **eigenvalue_covariance_matrix;
  double ***inverse_rotation_matrix_for_diagonalization ;
  double ***rotation_matrix_for_diagonalization ;
  //  double ***inverse_rotation_matrix_for_diagonalization_copy ;
  char filename_input_collision[256];
  char filename_collision[256];
  double swpc_et_last_observation;
  double swpc_et_first_prediction;
  double use_ap_hist;
  char tle_constellation_gps_filename[150];
  int swpc_need_observations;
  int swpc_need_predictions;
  char dir_input[1000];
  char dir_input_geometry[1000];
  char dir_input_collision[1000];
  char dir_input_density[1000];
  char dir_input_coverage[1000];
  char dir_input_coverage_storm[1000];
  char dir_input_coverage_ground_station[1000];
  char dir_input_density_msis[1000];
  char dir_input_density_density_files[1000];
  char dir_input_density_gitm[1000];
  char dir_input_attitude[1000], dir_input_tle[1000], dir_input_tle_gps_tle[1000];
  char dir_output[1000], dir_output_run_name[1000], dir_output_run_name_sat_name[N_SATS][1000], dir_output_run_name_constellation_gps[1000], dir_output_run_name_coverage[1000],dir_output_run_name_coverage_storm[1000], dir_output_run_name_sat_name_ensemble[1000], dir_output_run_name_sat_name_ensemble_proc_files[1000],dir_output_run_name_sat_name_coverage[N_SATS][1000], dir_output_run_name_temp[1000], dir_output_run_name_collision[1000], dir_output_run_name_collision_dca[1000], dir_output_run_name_collision_tca[1000];
  int write_collision_files;
  char gitm_directory[100]; // Path of the pirectory that has the GITM files
   char array_gitm_file[700][70]; // Their should not be more than 700 files in the GITM directory you're looking at. 
  int nb_gitm_file;
  double array_gitm_date[700][2]; // column 0 is the time in Julian date, second column is the index in array_gitm_file of the file which date is this one. 
   
  int initialize_geo_with_bstar; // 1 if the geometry is initialized reading bstar from the tle and not from a geometry file; 0 othersize
  char            format_density_driver[256];
  double          *density;
  char            filename_output_ensemble[30][1000];
  int             nb_ensembles_output_files;
  int             nb_ensembles_min; // if you run ensembles on different parameters at the same time (800 ensembles on COE and 900 ensembles attitude for example) then this variable represents the minimum of the number of ensembles for each parameter (900 here)
  int             nb_ensemble_min_per_proc;
  int             nb_ensemble_per_proc;
  int             nb_ensemble_attitude_per_proc;
  int             nb_ensemble_cd_per_proc;
  int             nb_ensemble_density_per_proc;
  int             nb_ensembles; // for the COE
  int             nb_ensembles_attitude; // for the attitude
  int             nb_ensembles_cd; // for the drag coefficient
  int             nb_ensembles_density; // for the thermospheric density
  int             coe_to_ensemble[N_SATS][6];
  char            filename_output[N_SATS][1000];
  char            filename_surface[1000];
  char            initial_epoch[300];     // Epoch
  char            final_epoch[300];     // Epoch
  char            name_sat[N_SATS][1000]; // name of the satellite in the ORBIT configuration (matrix X * 7 in input.d)
  int             n_satellites;   // Number of satellites
  int             nb_satellites_not_including_gps;
  char            gps_file_name[32][1000]; // Names of GPS satellites output files 
  int             nb_gps;   // Number of GPS satellites to propagate
  SpiceDouble     epoch_gps[32]; // epoch of TLE of the GPS
  char            type_orbit_initialisation[256];
  char            tle_initialisation[1000][N_SATS];
  int             include_drag;   // include drag
  int             include_solar_pressure; // include solar pressure
  int             include_sun;    // include Sun perturbations
  int             include_moon;   // include Moon perturbations
  double          cd_modification[N_SATS]; // hack to allow us to change the drag coefficient on certain satellites
  double          inclination[N_SATS];    // Inclination
  double          apogee_alt[N_SATS];     // apogee altitude (CBV 07-23-15)
  double          w[N_SATS] ;             // Argument of perigee  (CBV 07-22-15)
  double          long_an[N_SATS];        // RAAN                 (CBV 07-22-15)
  double          f[N_SATS];              // True anomaly         (CBV 07-22-15)
  double          eccentricity[N_SATS];   // Eccentricity         (CBV 07-22-15)
  double          inclination_sigma[N_SATS];    // Inclination
  double          apogee_alt_sigma[N_SATS];     // apogee altitude (CBV 07-23-15)
  double          w_sigma[N_SATS] ;             // Argument of perigee  (CBV 07-22-15)
  double          long_an_sigma[N_SATS];        // RAAN                 (CBV 07-22-15)
  double          f_sigma[N_SATS];              // True anomaly         (CBV 07-22-15)
  double          eccentricity_sigma[N_SATS];   // Eccentricity         (CBV 07-22-15)
  double          deployment_module_orbital_elements[N_SATS][6]; // orbital elements of the deployment module that releases the satellites (oneset of orbital elements per satellite even if it is the same deployment module for all satellites (we do that because it could release the satellites at different positions))
  double          deployment_speed[N_SATS]; // speed at which the satellite is ejected from the deployment module, in the LVLH reference system of the deployement module
  double          deployment_angle[N_SATS]; // angle at which the satellite is ejected from the deployment module, in the LVLH reference system of the deployement module. BE CAREFUL: LVLH _Y points to the left and we are considering that an angle of for example 30 degrees is counted from the in-track direction to the right (clockwise). If the sc is ejected to the left with an angle of for example 90 degrees, then deployment_angle is equal to 270 degrees
  double          x_ecef[N_SATS];    // position in ECEF
  double          y_ecef[N_SATS];   
  double          z_ecef[N_SATS];   
  double          vx_ecef[N_SATS];   // velocity in ECEF
  double          vy_ecef[N_SATS];   
  double          vz_ecef[N_SATS];   
  double          x_eci[N_SATS];    // position in ECI
  double          y_eci[N_SATS];   
  double          z_eci[N_SATS];   
  double          vx_eci[N_SATS];   // velocity in ECI
  double          vy_eci[N_SATS];   
  double          vz_eci[N_SATS];   
  double          x_eci_sigma[N_SATS];    // position in ECI
  double          y_eci_sigma[N_SATS];   
  double          z_eci_sigma[N_SATS];   
  double          vx_eci_sigma[N_SATS];   // velocity in ECI
  double          vy_eci_sigma[N_SATS];   
  double          vz_eci_sigma[N_SATS];   

  char            earth_fixed_frame[100]; // Earth body-fixed rotating frame for conversion ECI to ECEF
  double          dt;             // Integrator step size (s)
  double          dt_output;             // time step to output the results
  int             nb_density_drivers;  // number of density drivers (ex: f10.7, Ap)
  double          Ap_static, f107_static, f107A_static;
  double          Ap_hist_static[7];
  double          *Ap ;             // magnetic index(daily)
  double          **Ap_hist ;             // magnetic index(historical)
  double          *f107;           // Daily average of F10.7 flux
  double          *f107A;          // 81 day average of F10.7 flux
  double          **Ap_ensemble;   
  double          **f107_ensemble;   
  double          **f107A_ensemble;   
  double          *et_interpo;     // for linear interpolation
  double          *et_sigma_f107_ap;     // for linear interpolation of the time on uncertainty in F10.7 and Ap
  double          *sigma_f107;     // uncertainty in the prediction of F10.7
  double          *sigma_ap;       // uncertainty in the prediction of Ap

  double          *et_mod_f107_ap;     // for linear interpolation of the time on modificaiotn of F10.7 and Ap
  double          *mod_f107;     // modification in the prediction of F10.7
  double          *mod_ap;       // modifcaiton in the prediction of Ap
  
  char            attitude_profile[256]; // nadir, sun_pointed, other...
  /* double          lvlh_alongtrack_in_body_cartesian[3]; // component of the y axis of the LVLH frame in the SC reference system under the cartesian representation */
  /* double          lvlh_crosstrack_in_body_cartesian[3]; // component of the z axis of the LVLH frame in the SC reference system under the cartesian representation */
  double          *pitch;
  double          *roll;
  double          *yaw;
 
  // variables below are if ensemble_angular_velocity is chosen in section #ATTTITUDE
  double          pitch_ini_ensemble;
  double          roll_ini_ensemble;
  double          yaw_ini_ensemble;
  double          pitch_mean_angular_velocity_ensemble;
  double          roll_mean_angular_velocity_ensemble;
  double          yaw_mean_angular_velocity_ensemble;
  double          pitch_sigma_angular_velocity_ensemble;
  double          roll_sigma_angular_velocity_ensemble;
  double          yaw_sigma_angular_velocity_ensemble;  

  // variables below are if ensemble_initial_attitude is chosen in section #ATTTITUDE
  double          pitch_mean_ensemble; // mean of the normal distribution on the pitch (angle, not angular velocity) 
  double          roll_mean_ensemble; // mean of the normal distribution on the roll (angle, not angular velocity) 
  double          yaw_mean_ensemble; // mean of the normal distribution on the yaw (angle, not angular velocity) 
  double          pitch_sigma_for_ensemble_initial_attitude; // std of the normal distribution on the pitch (angle, not angular velocity) 
  double          roll_sigma_for_ensemble_initial_attitude; // std of the normal distribution on the roll (angle, not angular velocity) 
  double          yaw_sigma_for_ensemble_initial_attitude; // std of the normal distribution on the yaw (angle, not angular velocity) 
  double          pitch_angular_velocity_constant; // constant pitch angular velocity
  double          roll_angular_velocity_constant; // constant roll angular velocity
  double          yaw_angular_velocity_constant;  // constant yaw angular velocity

  double          pitch_sigma_ensemble;
  double          roll_sigma_ensemble;
  double          yaw_sigma_ensemble;  
  double             attitude_reset_delay;
  int             *order_pitch, *order_roll, *order_yaw;
  double          mass;           // Mass of spacecraft
  double          solar_cell_efficiency; // Solar cell efficiency
  double             n_surfaces;     // number of surfaces on each satellite (has to be the same number for every satellite...) (CBV 07-29-2015)
  SURFACE_T       surface[N_SURFACES]; // Properties of the surface
  double          degree;         // Gravity degree
  double          order;          // Gravity order
  char            leap_sec_file[256]; // SPICE leap seconds file
  char            eop_file[256];      // SPICE EOP parameters file
  char            planet_ephem_file[256]; // SPICE Load planetary ephemerides
  char            earth_binary_pck[256]; // SPICE Earth binary PCK (used for ECI/ECEF conversion)
  int             nb_time_steps; // number of time steps in the simulation
  int             nb_storm; // number of storms
  char            filename_storm[N_STORM][256]; // filename for each storm
  char            filename_ground_station[256]; // filename for the ground stations (one file for all ground stations)
  char            name_ground_station[N_GROUND_STATION][256];
  double          latitude_ground_station[N_GROUND_STATION];
  double          longitude_ground_station[N_GROUND_STATION];
  double          altitude_ground_station[N_GROUND_STATION];
  double          min_elevation_angle_ground_station[N_GROUND_STATION];
  int             nb_ground_stations;
} OPTIONS_T;

#endif

// Prototype
int load_options( OPTIONS_T *OPTIONS, char filename[1000], int iProc, int nProcs, int iDebugLevel, char filename_input_no_path[256]);

int convert_angle_to_cartesian(double T_sc_to_lvlh[3][3], double v_angle[3], int order_rotation[3]);

int load_surface( OPTIONS_T *OPTIONS, char filename[1000], int nProcs);

int load_attitude( OPTIONS_T *OPTIONS, char attitude_profile[256], FILE *input_file, int nProcs, double ang_velo[3], int iDebugLevel, int iProc);

int lin_interpolate(double *f107_after_interpo,
		    double *f107A_after_interpo,
		    double *Ap_after_interpo,
		    double **Ap_hist_after_interpo,
		    double *x_after_interpo,	  
		    double *use_ap_hist,
		    char f107_filename[256],
		    char ap_filename[256],
		    char src_file[256],
		    int nb_time_steps_simu,
		    char initial_epoch[256],
		    double et_oldest_tle_epoch, 
		    char final_epoch[256], 
		    double dt,
		    double missing_data_value,
		    int iDebugLevel, int iProc);


int lin_interpolate_ap_hist( double **y_after_interpo, double *x_after_interpo, char filename[256], char src_file[256], int nb_time_steps_simu, char initial_epoch[256], double et_oldest_tle_epoch, double dt, double missing_data_value, int iDebugLevel, int iProc); 

int lin_interpolate_attitude(double **quaternion_after_interpo, double *pitch_after_interpo, double *roll_after_interpo, double *yaw_after_interpo, int *pitch_order_after_interpo, int *roll_order_after_interpo, int *yaw_order_after_interpo, double *x_after_interpo, char filename[256], int nb_time_steps_simu, char initial_epoch[256], double et_oldest_tle_epoch, double dt, int iProc, int *file_is_quaternion, int use_kalman);

int nb_elements_file(int *nb_element_in_file,char filename[256],char header_end[256], char file_end[256]);

int nb_time_steps(int *nb_time_steps_simu, double et_initial_epoch, char final_epoch[256], double dt);

int previous_index(int *x_min_index, double *x_before_interpo, double value, double size_x_before_interpo);

int read_gps_tle(char gps_tle_filename[1000], int *nb_gps, char gps_name[32][1000]);

double randn(double mu, double sigma);
double randn_iproc (int iProc, double mu, double sigma);
void RemoveSpaces(char* source);

int calculate_f107_average(char filename_f107_to_calculate_f107_average[256], char filename_f107A[256],  char initial_epoch_wget[8], char final_epoch_wget[8], char final_epoch_wget_plus40_5days[8], double et_initial_epoch, double et_final_epoch);

char *str_replace(char *orig, char *rep, char *with);

int lin_interpolate_swpc(double *f107_after_interpo,
		    double *f107A_after_interpo,
		    double *ap_after_interpo,
		    double *x_after_interpo,
			 double *sigma_f107_after_interpo, 
			 double *sigma_ap_after_interpo,
			 double *x_sigma_f107_ap_after_interpo,
		    double *use_ap_hist,
			 char **list_filename_obs_swpc_f107,
			 char **list_filename_obs_swpc_ap,
		    char filename_f107_ap_pred[256],
		    int nb_time_steps_simu,
		    char initial_epoch[256],
			 double et_oldest_tle_epoch, 
		    char final_epoch[256],
		    double dt,
		    double missing_data_value,
			 int iDebugLevel,
			 int nb_file_obs_swpc_f107,
			 int nb_file_obs_swpc_ap,
			 int swpc_need_observations,
			 int swpc_need_predictions,
			 int nb_ensembles_density,
			 char dir_input_density_msis[1000],
			 double *swpc_et_first_prediction, int iProc,
			 int swpc_final_epoch_more_recent_than_45_days_from_current_day
);

int lin_interpolate_swpc_mod(double *f107_after_interpo,
			 double *f107A_after_interpo,
			 double *ap_after_interpo,
			 double *x_after_interpo,
			 double *mod_f107_after_interpo, 
			 double *mod_ap_after_interpo,
			 double *x_mod_f107_ap_after_interpo,
			 double *use_ap_hist,
			 char **list_filename_obs_swpc_f107,
			 char **list_filename_obs_swpc_ap,
			 char filename_f107_ap_pred[256],
			 int nb_time_steps_simu,
			 char initial_epoch[256],
			 char final_epoch[256],
			 double dt,
			 double missing_data_value,
			 int iDebugLevel,
			 int nb_file_obs_swpc_f107,
			 int nb_file_obs_swpc_ap,
			 int swpc_need_observations,
			 int swpc_need_predictions,
			     double *swpc_et_first_prediction, int iProc, char filename_f107_ap_mod[1000]
			     );
int generate_ensemble_f107_ap(OPTIONS_T *OPTIONS, int iDebugLevel, int iProc);

void sort_asc_order( double *array_out, double *array_in, int n);



int ptd( double d, char str[256]);

int pti( int i, char str[256]);

int ini_collision( OPTIONS_T *OPTIONS, int iProc );

int print_error(int iProc, char *error_message);
int print_error_any_iproc( int iProc, char *error_message);

int exitall();
int sign(double x);
int propagate_spacecraft(   SPACECRAFT_T *SC,
                            PARAMS_T     *PARAMS,
			    double et_initial_epoch,
			    double          et_oldest_tle_epoch,
			    double *density,
			    GROUND_STATION_T *GROUND_STATION,
			    OPTIONS_T *OPTIONS,
			    CONSTELLATION_T *CONSTELLATION,
			    int iProc,
			    int iDebugLevel,
			    int *start_ensemble,
			    int *array_sc);


int compute_dxdt(   double          drdt[3],
                    double          dvdt[3],
                    double          *et,
                    double          r_i2cg_INRTL[3],
                    double          v_i2cg_INRTL[3],
                    PARAMS_T        *PARAMS,
                    INTEGRATOR_T    *INTEGRATOR,
		    double          et_initial_epoch, 
		    double          et_oldest_tle_epoch,
		    double          *density,
		    int             index_in_attitude_interpolated, 
		    int             index_in_driver_interpolated,
			CONSTELLATION_T *CONSTELLATION,
		    OPTIONS_T       *OPTIONS,
		    int iProc,
		    int iDebugLevel, SPACECRAFT_T *SC);

int compute_drag(       double          adrag_i2cg_INRTL[3],
                        double          r_i2cg_INRTL[3],
                        double          v_i2cg_INRTL[3],
                        double          et,
                        PARAMS_T        *PARAMS,
                        INTEGRATOR_T    *INTEGRATOR,
			double          et_initial_epoch,
			double          et_oldest_tle_epoch,
			double          *density,
			int             index_in_attitude_interpolated, 
			int             index_in_driver_interpolated,
			CONSTELLATION_T *CONSTELLATION,
			OPTIONS_T       *OPTIONS,
			int iProc,
			int iDebugLevel, SPACECRAFT_T *SC);

int set_attitude( ATTITUDE_T attitude, int index_in_attitude_interpolated, OPTIONS_T *OPTIONS, int file_is_quaternion );
int print_oe(ORBITAL_ELEMENTS_T OE, PARAMS_T *PARAMS);

int compute_time_interpo(OPTIONS_T *OPTIONS);
