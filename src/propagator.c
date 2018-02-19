#include <mpi.h>
#include "propagator.h"
#include "options.h"
#include "gsl/gsl_rng.h"
#include "gsl/gsl_sf_legendre.h"

int eci2lla(double pos[3], double et,  double geodetic[3]  ){ // convert ECI xyz to lon/lat/alt. x,y,z in km. time in seconds past J2000.

  // convert time seconds past J2000 to julian date in ut1 days from 4713 bc   
  // // convert time seconds past J2000 to time string 
  char cu_time[290];
  et2utc_c(et, "ISOC" ,3 ,255 , cu_time); 
  // // convert time string to year month day hour minute seconds (all int, except seconds which is double)
  int year_current, month_current, day_current, hour_current, minute_current;
  double second_current;
  char year_current_str[10];	
  strcpy(year_current_str, ""); strncat(year_current_str, &cu_time[0], 4); year_current = atoi(year_current_str);
  char month_current_str[10];	
  strcpy(month_current_str, ""); strncat(month_current_str, &cu_time[5], 2); month_current = atoi(month_current_str);
  char day_current_str[10];	
  strcpy(day_current_str, ""); strncat(day_current_str, &cu_time[8], 2); day_current = atoi(day_current_str);
  char hour_current_str[10];	
  strcpy(hour_current_str, ""); strncat(hour_current_str, &cu_time[11], 2); hour_current = atoi(hour_current_str);
  char minute_current_str[10];	
  strcpy(minute_current_str, ""); strncat(minute_current_str, &cu_time[14], 2); minute_current = atoi(minute_current_str);
  char second_current_str[10];	
  int second_current_no_microsec;
  strcpy(second_current_str, ""); strncat(second_current_str, &cu_time[17], 2); second_current_no_microsec = atoi(second_current_str);
  char microsecond_current_str[10];	
  int microsecond;
  strcpy(microsecond_current_str, ""); strncat(microsecond_current_str, &cu_time[20], 3); microsecond = atoi(microsecond_current_str);
  second_current = second_current_no_microsec + microsecond/1000.;

  //  printf("%d %d %d %d %d %f\n", year_current, month_current, day_current, hour_current, minute_current, second_current);
  double time ;//= 2453101.82741;
  // // Convert year month day hhour min sec to Julian day (in ut1 days from 4713 bc)
  jday (year_current, month_current, day_current, hour_current, minute_current, second_current, &time );



  double f = 1.0/298.26;
  double xkmper = 6378.135;
  double theta = atan2(pos[1],pos[0]);
  double lon = fmod( theta - gstime( time ), 2*M_PI ) ; 
    double r = sqrt(pos[0]*pos[0] + pos[1]*pos[1]);
    double e2 = f*(2-f);
    double lat = atan2(pos[2],r);
    
    double phi = lat;
    double c = 1.0/sqrt( 1 - e2*sin(phi)*sin(phi) );
    lat = atan2(pos[2]+xkmper*c*e2*sin(phi),r);
    while (fabs(lat - phi) > 0.0000000001){ 
      phi = lat;
      c = 1.0/sqrt( 1 - e2*sin(phi)*sin(phi) );
      lat = atan2(pos[2]+xkmper*c*e2*sin(phi),r);
      //      printf("%e %d\n", fabs(lat - phi), lat == phi);
    }

    double   alt = r/cos(lat) - xkmper*c;
    geodetic[0] = lat;
      geodetic[1] = lon;
      geodetic[2] = alt;
      if (lon < -M_PI){
	printf("lon = %f\n", lon*180./M_PI);
      }
      

  return 0;
}

/* -----------------------------------------------------------------------------
*
*                           function gstime
*
*  this function finds the greenwich sidereal time.
*
*  author        : Charles Bussy-Virat, largely inspired from SGP4 by david vallado                  719-573-2600    1 mar 2001
*
*  inputs          description                    range / units
*    jdut1       - julian date in ut1             days from 4713 bc
*
*  outputs       :
*    gstime      - greenwich sidereal time        0 to 2pi rad
*
*  locals        :
*    temp        - temporary variable for doubles   rad
*    tut1        - julian centuries from the
*                  jan 1, 2000 12 h epoch (ut1)
*
*  coupling      :
*    none
*
*  references    :
*    vallado       2004, 191, eq 3-45
* --------------------------------------------------------------------------- */

double  gstime (double jdut1 )
   {
     const double twopi = 2.0 * M_PI;
     const double deg2rad = M_PI / 180.0;
     double       temp, tut1;

     tut1 = (jdut1 - 2451545.0) / 36525.0;

     temp = -6.2e-6* tut1 * tut1 * tut1 + 0.093104 * tut1 * tut1 +
             (876600.0*3600 + 8640184.812866) * tut1 + 67310.54841;  // sec
     temp = fmod(temp * deg2rad / 240.0, twopi); //360/86400 = 1/240, to deg, to rad

     // ------------------------ check quadrants ---------------------
     if (temp < 0.0)
         temp += twopi;

     return temp;
   }  // end gstime

/* -----------------------------------------------------------------------------
*
*                           procedure jday
*
*  this procedure finds the julian date given the year, month, day, and time.
*    the julian date is defined by each elapsed day since noon, jan 1, 4713 bc.
*
*  algorithm     : calculate the answer in one step for efficiency
*
*  author        : Charles Bussy-Virat, largely inspired from SGP4 by david vallado                  719-573-2600    1 mar 2001
*
*  inputs          description                    range / units
*    year        - year                           1900 .. 2100
*    mon         - month                          1 .. 12
*    day         - day                            1 .. 28,29,30,31
*    hr          - universal time hour            0 .. 23
*    min         - universal time min             0 .. 59
*    sec         - universal time sec             0.0 .. 59.999
*
*  outputs       :
*    jd          - julian date                    days from 4713 bc
*
*  locals        :
*    none.
*
*  coupling      :
*    none.
*
*  references    :
*    vallado       2007, 189, alg 14, ex 3-14
*
* --------------------------------------------------------------------------- */

int jday (int year, int mon, int day, int hr, int minute, double sec, double *jd )
   {
     *jd = 367.0 * year -
          floor((7 * (year + floor((mon + 9) / 12.0))) * 0.25) +
          floor( 275 * mon / 9.0 ) +
          day + 1721013.5 +
          ((sec / 60.0 + minute) / 60.0 + hr) / 24.0;  // ut in days
          // - 0.5*sgn(100.0*year + mon - 190002.5) + 0.5;
     return 0;
   }  // end jday





/////////////////////////////////////////////////////////////////////////////////////////
//
//  Name:           coverage_ground_station
//  Purpose:        Computes the coverage of a satellite above all ground stations and calculate a few parameters: elevation, azimuth, range in different two coordinate systems (ECEF, ENU) and in two different frames of reference (satellite and ground station)
//  Assumptions:    if the sc is a GPS that was initialized in the second line of section #SPACECRAFT of the main input file, its attitude is assumed to be nadir pointing
//  References:     /
//
//  Change Log:
//      |   Developer   |       Date    |   SCR     |   Notes
//      | --------------|---------------|-----------|-------------------------------
//      | C. Bussy-Virat| 06/01/2016    |   ---     | Initial Implementation
//
/////////////////////////////////////////////////////////////////////////////////////////
int coverage_ground_station(   SPACECRAFT_T *SC, // in: spacecraft (position and time). out: elevation, azimuth, range 
			       GROUND_STATION_T *GROUND_STATION, // in: list of ground stations
			       PARAMS_T *PARAMS, // in: parameters for the propagation (Earth flattening and radius here)
			       int index_in_attitude_interpolated, // in: index of the current time step (take into account RK4)
			       INTEGRATOR_T    *INTEGRATOR, // in: paramters for the propagation (attitude of the sc here)
			       double          et_initial_epoch, // in: time of the inital epoch of the constellation
			       double et_sc_initial,
			       double sc_ecef_previous_time_step[3], // in: ECEF position of the spacecraft at the previous time step of the propagation (used to linear interpolate the position between the previous position and the current position)
			       double sc_eci_previous_time_step[3], //  in: ECI position of the spacecraft at the previous time step of the propagation (used to linear interpolate the position between the previous position and the current position)
			       double sc_v_eci_previous_time_step[3], //  in: ECI velocity of the spacecraft at the previous time step of the propagation (used to linear interpolate the position between the previous position and the current position)
			       double time_step_interpolation // in: time step of the linear interpolation of the sc position (in seconds)
			       )

{

  /* Declaration */


  //      char time_et[256];
  double norm_unit_ground_station_to_sc_in_enu_projected_on_east_north_local_plane[3];
  double norm_unit_sc_to_ground_station_body_projected_on_body_xy[3];
  double unit_sc_to_ground_station_body[3];
  double sc_to_ground_station_dot_minus_z_axis_body;	
  double unit_sc_to_ground_station_body_projected_on_body_xy_dot_y_axis_body;
  double x_axis_body_dot_sc_to_ground_station_body_projected_on_body_xy;
  double unit_sc_to_ground_station_body_projected_on_body_xy[3];
  double minus_z_axis_body[3];
  int step_end_interpolation;
  int iground;
  double ground_station_to_sc_in_ecef[3];
  double opposite_ground_station[3];
  double T_ECEF_to_J2000[3][3], T_inrtl_2_lvlh[3][3], T_sc_to_lvlh[3][3], T_lvlh_to_sc[3][3];
  double ground_station_to_sc_eci[3], ground_station_to_sc_lvlh[3],ground_station_to_sc_body[3];
  double v_angle[3];
  int order_rotation[3];
  double x_axis_body[3], unit_ground_station_to_sc_body[3];
  double ground_station_to_sc_in_enu[3];
  double ground_station_to_sc_dot_local_vertical_in_enu;
  double T_enu_to_ecef[3][3], T_ecef_to_enu[3][3];
  double local_vertical_in_enu[3];
  double local_north_in_enu[3];
  double ground_station_to_sc_in_enu_projected_on_east_north_local_plane[3];
  double ground_station_to_sc_in_enu_projected_on_east_north_local_plane_dot_local_north_in_enu;
  char time_interpolated[256];
  double et_interpolated, et_previous_time_step;
  double ecef_sc[3], eci_sc[3], eci_v_sc[3];
  int in_sight_of_ground_station;
  double elevation_wtr_to_ground_station_in_sc_refce, azimuth_wtr_to_ground_station_in_sc_refce, range_wtr_to_ground_station, elevation_wtr_to_ground_station_in_ground_station_refce, azimuth_wtr_to_ground_station_in_ground_station_refce;
  int istep;
  double unit_ground_station_to_sc_in_enu[3];
  double y_axis_body[3];
  double unit_ground_station_to_sc_in_enu_projected_on_east_north_local_plane[3];
  double ground_station_to_sc_in_enu_projected_on_east_north_local_plane_dot_local_east_in_enu;
  double local_east_in_enu[3];

  et_previous_time_step = SC->et - INTEGRATOR->dt;

  /* Algorithm */
  for (iground = 0; iground < GROUND_STATION->nb_ground_stations; iground++){
    // Write the header of the coverage file
    if ( index_in_attitude_interpolated == INTEGRATOR->index_in_attitude_interpolated_first + 2 ){ // Write the header at the first time step of the propagation
      fprintf(SC->fp_coverage_ground_station[iground], "// ---------------------------------------------------------------------------------- \n");
      fprintf(SC->fp_coverage_ground_station[iground], "// \n");
      fprintf(SC->fp_coverage_ground_station[iground], "// Please note this file is auto generated.\n");
      fprintf(SC->fp_coverage_ground_station[iground], "// Coverage of Spacecraft %s for the ground station %s.\n", SC->name_sat,GROUND_STATION->name_ground_station[iground]);
      fprintf(SC->fp_coverage_ground_station[iground], "// \n");
      fprintf(SC->fp_coverage_ground_station[iground], "// Version control is under Joel Getchius' Mac and Charles Bussy-Virat University of Michigan's Mac \n");
      fprintf(SC->fp_coverage_ground_station[iground], "// \n");
      fprintf(SC->fp_coverage_ground_station[iground], "// ---------------------------------------------------------------------------------- \n");
      fprintf(SC->fp_coverage_ground_station[iground], "// Ground station %s ECEF: %f %f %f\n", GROUND_STATION->name_ground_station[iground],GROUND_STATION->ecef_ground_station[iground][0], GROUND_STATION->ecef_ground_station[iground][1], GROUND_STATION->ecef_ground_station[iground][2]);
      fprintf(SC->fp_coverage_ground_station[iground], "// Ground station %s longitude(degree) latitude(degree) altitude(m): %f %f %f\n", GROUND_STATION->name_ground_station[iground],GROUND_STATION->longitude_ground_station[iground]*RAD2DEG, GROUND_STATION->latitude_ground_station[iground]*RAD2DEG, GROUND_STATION->altitude_ground_station[iground]);
      fprintf(SC->fp_coverage_ground_station[iground], "TIME ECEF_X_SC ECEF_Y_SC ECEF_Z_SC ELEV_REFCE_SC AZI_REFCE_SC ELEV_REFCE_GROUND_STATION AZI_REFCE_GROUND_STATION RANGE_SC_TO_GROUND_STATION\n");
      fprintf(SC->fp_coverage_ground_station[iground], "#START \n");
    }

    if ( ( index_in_attitude_interpolated == INTEGRATOR->index_in_attitude_interpolated_first + 2 ) ) {
      step_end_interpolation = (int)(nearbyint(INTEGRATOR->dt / time_step_interpolation + 1));
      //      printf("%s: %d | %d - %d\n", SC->name_sat,step_end_interpolation, GROUND_STATION->nb_ground_stations, iground);
      et_interpolated = et_previous_time_step - 1;
    }
    else{
      step_end_interpolation = (int)(nearbyint(INTEGRATOR->dt / time_step_interpolation));
      //      printf("%s: %d | %d - %d\n", SC->name_sat,step_end_interpolation, GROUND_STATION->nb_ground_stations, iground);
      et_interpolated = et_previous_time_step;
    }
    for (istep = 0; istep < step_end_interpolation; istep++){

      // Linear interpolation of the sc position between the previous time step and the current time step
      et_interpolated = et_interpolated + time_step_interpolation;
      /* printf("%d\n", istep); */
      /* etprint(et_interpolated, ""); */
      
      // // ECEF position interpolation
      ecef_sc[0] = sc_ecef_previous_time_step[0] + ( ( et_interpolated - et_previous_time_step ) / INTEGRATOR->dt  ) * ( SC->r_ecef2cg_ECEF[0] - sc_ecef_previous_time_step[0] ) ;
      ecef_sc[1] = sc_ecef_previous_time_step[1] + ( ( et_interpolated - et_previous_time_step ) / INTEGRATOR->dt ) * ( SC->r_ecef2cg_ECEF[1] - sc_ecef_previous_time_step[1] ) ;
      ecef_sc[2] = sc_ecef_previous_time_step[2] + ( ( et_interpolated - et_previous_time_step ) / INTEGRATOR->dt ) * ( SC->r_ecef2cg_ECEF[2] - sc_ecef_previous_time_step[2] ) ;
      // // ECI position interpolation
      eci_sc[0] = sc_eci_previous_time_step[0] + ( ( et_interpolated - et_previous_time_step ) / INTEGRATOR->dt  ) * ( SC->r_i2cg_INRTL[0] - sc_eci_previous_time_step[0] ) ;
      eci_sc[1] = sc_eci_previous_time_step[1] + ( ( et_interpolated - et_previous_time_step ) / INTEGRATOR->dt ) * ( SC->r_i2cg_INRTL[1] - sc_eci_previous_time_step[1] ) ;
      eci_sc[2] = sc_eci_previous_time_step[2] + ( ( et_interpolated - et_previous_time_step ) / INTEGRATOR->dt ) * ( SC->r_i2cg_INRTL[2] - sc_eci_previous_time_step[2] ) ;
      // // ECI velocity interpolation
      eci_v_sc[0] = sc_v_eci_previous_time_step[0] + ( ( et_interpolated - et_previous_time_step ) / INTEGRATOR->dt  ) * ( SC->v_i2cg_INRTL[0] - sc_v_eci_previous_time_step[0] ) ;
      eci_v_sc[1] = sc_v_eci_previous_time_step[1] + ( ( et_interpolated - et_previous_time_step ) / INTEGRATOR->dt ) * ( SC->v_i2cg_INRTL[1] - sc_v_eci_previous_time_step[1] ) ;
      eci_v_sc[2] = sc_v_eci_previous_time_step[2] + ( ( et_interpolated - et_previous_time_step ) / INTEGRATOR->dt ) * ( SC->v_i2cg_INRTL[2] - sc_v_eci_previous_time_step[2] ) ;

      // Figures out if the satellite is above the local horizon of the ground station
      // // Local vertical in the ENU frame attached to the ground station  (ENU: East-North-Up)
      local_vertical_in_enu[0] = 0; local_vertical_in_enu[1] = 0; local_vertical_in_enu[2] = 1;
      // // vector ground station to sc
      v_scale(opposite_ground_station, GROUND_STATION->ecef_ground_station[iground], -1);
      v_add(ground_station_to_sc_in_ecef, opposite_ground_station, ecef_sc);
      // // convert vector ground station to sc from ECEF to ENU coordinates of the ground station
      compute_T_enu_to_ecef( T_enu_to_ecef, GROUND_STATION->latitude_ground_station[iground], GROUND_STATION->longitude_ground_station[iground], PARAMS->EARTH.flattening);
      m_trans(T_ecef_to_enu, T_enu_to_ecef);
      m_x_v(ground_station_to_sc_in_enu, T_ecef_to_enu, ground_station_to_sc_in_ecef);
      // // Dot product between the local vertical and the vector ground station to sc in ENU reference system
      v_norm(unit_ground_station_to_sc_in_enu, ground_station_to_sc_in_enu);
      v_dot(&ground_station_to_sc_dot_local_vertical_in_enu, unit_ground_station_to_sc_in_enu, local_vertical_in_enu);
      // Elevation angle of the sc with respect to the ground station in the reference frame of the ground station
      elevation_wtr_to_ground_station_in_ground_station_refce = M_PI/2. - acos( ground_station_to_sc_dot_local_vertical_in_enu );

      if ( elevation_wtr_to_ground_station_in_ground_station_refce >= GROUND_STATION->min_elevation_angle_ground_station[iground] ){ // if the sc is above the local horizon + min elevation angle of the ground station
    	in_sight_of_ground_station = 1;

    	/************************************************************************************************************/
    	/* COMPUTE ELVATION AND AZIMUTH ANGLES IN THE REFERENCE FRAME OF THE SC */
    	// Elevation angle of the sc with respect to the ground station in the reference frame of the sc: angle between the sc xy body plane and the vector sc to ground station (expressed in the sc body reference system). Counted negative if the ground station is bwlo the body xy plane, which is always the case in a nadir configuration and when the sc is in sight.
    	// // convert the ground station to sc vector in the sc body reference system
    	// // // ECEF to ECI
    	pxform_c(PARAMS->EARTH.earth_fixed_frame, "J2000", et_interpolated, T_ECEF_to_J2000);
    	m_x_v(ground_station_to_sc_eci, T_ECEF_to_J2000, ground_station_to_sc_in_ecef);
    	// // // ECI to LVLH
    	compute_T_inrtl_2_lvlh(T_inrtl_2_lvlh, eci_sc, eci_v_sc);
    	m_x_v(ground_station_to_sc_lvlh, T_inrtl_2_lvlh, ground_station_to_sc_eci);
    	// // // LVLH to BODY
    	// // // // Find the current attitude of the sc
	if (INTEGRATOR->isGPS == 0){ // if the sc is not a GPS that was initialized in the second line of section #SPACECRAFT of the main input file
	  if (strcmp(INTEGRATOR->attitude.attitude_profile, "ensemble_angular_velocity") == 0){
	    v_angle[0] = INTEGRATOR->attitude.pitch_angular_velocity_ensemble * ( et_interpolated - et_sc_initial ) + INTEGRATOR->attitude.pitch_ini_ensemble;
	    v_angle[1] = INTEGRATOR->attitude.roll_angular_velocity_ensemble * ( et_interpolated - et_sc_initial ) + INTEGRATOR->attitude.roll_ini_ensemble;
	    v_angle[2] = INTEGRATOR->attitude.yaw_angular_velocity_ensemble * ( et_interpolated - et_sc_initial ) + INTEGRATOR->attitude.yaw_ini_ensemble;
	    order_rotation[0] = 1; // !!!!!!!! we might want to change that in the future
	    order_rotation[1] = 2;
	    order_rotation[2] = 3;
	    INTEGRATOR->attitude.pitch_current = v_angle[0];
	    INTEGRATOR->attitude.roll_current = v_angle[1];
	    INTEGRATOR->attitude.yaw_current = v_angle[2];
	        INTEGRATOR->attitude.order_pitch_current = order_rotation[0];
    INTEGRATOR->attitude.order_roll_current = order_rotation[1];
    INTEGRATOR->attitude.order_yaw_current = order_rotation[2];


	  }

	  if (strcmp(INTEGRATOR->attitude.attitude_profile, "ensemble_initial_attitude") == 0) {
	      v_angle[0] =  INTEGRATOR->attitude.pitch_for_attitude_ensemble  +  INTEGRATOR->attitude.pitch_angular_velocity_constant * ( et_interpolated - et_sc_initial );
	      v_angle[1] =  INTEGRATOR->attitude.roll_for_attitude_ensemble  +  INTEGRATOR->attitude.roll_angular_velocity_constant * ( et_interpolated - et_sc_initial );
	      v_angle[2] =  INTEGRATOR->attitude.yaw_for_attitude_ensemble  +  INTEGRATOR->attitude.yaw_angular_velocity_constant * ( et_interpolated - et_sc_initial );

	      order_rotation[0]  = 1; order_rotation[1]  = 2; order_rotation[2]  = 3;
	   
	      INTEGRATOR->attitude.pitch_current = v_angle[0];
	      INTEGRATOR->attitude.roll_current = v_angle[1];
	      INTEGRATOR->attitude.yaw_current = v_angle[2];
	          INTEGRATOR->attitude.order_pitch_current = order_rotation[0];
    INTEGRATOR->attitude.order_roll_current = order_rotation[1];
    INTEGRATOR->attitude.order_yaw_current = order_rotation[2];


	  }
   
	  if ( (strcmp(INTEGRATOR->attitude.attitude_profile, "ensemble_angular_velocity") != 0) && (strcmp(INTEGRATOR->attitude.attitude_profile, "ensemble_initial_attitude") != 0) && (strcmp(INTEGRATOR->attitude.attitude_profile, "sun_pointed") != 0) ){ // otherwise (atittude is nadir, sun_pointed or manual (from an input file))  
	    if (INTEGRATOR->file_is_quaternion == 0){
	    v_angle[0] = INTEGRATOR->attitude.pitch[index_in_attitude_interpolated];
	    v_angle[1] = INTEGRATOR->attitude.roll[index_in_attitude_interpolated];
	    v_angle[2] = INTEGRATOR->attitude.yaw[index_in_attitude_interpolated];
	    order_rotation[0] = INTEGRATOR->attitude.order_pitch[index_in_attitude_interpolated];
	    order_rotation[1] = INTEGRATOR->attitude.order_roll[index_in_attitude_interpolated];
	    order_rotation[2] = INTEGRATOR->attitude.order_yaw[index_in_attitude_interpolated];
	    INTEGRATOR->attitude.pitch_current = v_angle[0];
	    INTEGRATOR->attitude.roll_current = v_angle[1];
	    INTEGRATOR->attitude.yaw_current = v_angle[2];
	        INTEGRATOR->attitude.order_pitch_current = order_rotation[0];
    INTEGRATOR->attitude.order_roll_current = order_rotation[1];
    INTEGRATOR->attitude.order_yaw_current = order_rotation[2];


	    }
	    else{
	      q_copy( INTEGRATOR->attitude.quaternion_current, INTEGRATOR->attitude.quaternion[index_in_attitude_interpolated]);
	    }
	    

	  }
	}
	else{ // if the sc is a GPS that was initialized in the second line of section #SPACECRAFT of the main input file: its attitude is assumed to be nadir pointing
	  v_angle[0] = 0;
	  v_angle[1] = 0;
	  v_angle[2] = 0;
	  order_rotation[0] = 1;
	  order_rotation[1] = 2;
	  order_rotation[2] = 3;
	    INTEGRATOR->attitude.pitch_current = v_angle[0];
	    INTEGRATOR->attitude.roll_current = v_angle[1];
	    INTEGRATOR->attitude.yaw_current = v_angle[2];
    INTEGRATOR->attitude.order_pitch_current = order_rotation[0];
    INTEGRATOR->attitude.order_roll_current = order_rotation[1];
    INTEGRATOR->attitude.order_yaw_current = order_rotation[2];


	}

    	// // // // Now we know the current attitude of the sc, convert the ground station to sc vector from LVLH to SC body reference frame
	//

	  compute_T_sc_to_lvlh( T_sc_to_lvlh, v_angle, order_rotation, INTEGRATOR->attitude.attitude_profile, &et_interpolated,  eci_sc, eci_v_sc, INTEGRATOR->file_is_quaternion, INTEGRATOR->attitude.quaternion_current);
    	m_trans(T_lvlh_to_sc, T_sc_to_lvlh);


    	m_x_v(ground_station_to_sc_body, T_lvlh_to_sc, ground_station_to_sc_lvlh);
	// // Calculate the elevation angle
    	v_norm( unit_ground_station_to_sc_body, ground_station_to_sc_body);
	v_scale(unit_sc_to_ground_station_body, unit_ground_station_to_sc_body, -1);
    	minus_z_axis_body[0] = 0, minus_z_axis_body[1] = 0, minus_z_axis_body[2] = -1; // coordinates of the -z_axis body in the sc body reference frame
    	v_dot( &sc_to_ground_station_dot_minus_z_axis_body, unit_sc_to_ground_station_body, minus_z_axis_body );
    	elevation_wtr_to_ground_station_in_sc_refce = -(M_PI/2- acos( sc_to_ground_station_dot_minus_z_axis_body )); // the elevation is counted from the sc xy body, negatively if the ground station is below the xy body plane
    	// Azimuth angle of the sc with respect to the ground station in the reference frame of the sc
    	// This is the angle between the x axis and the sc to ground station vector (expressed in the sc body reference system) projected on the body xy plane
    	// // Computes the angle, in the sc body system, between the body x axis and the sc to ground station vector projected on the body xy plane
    	x_axis_body[0] = 1, x_axis_body[1] = 0, x_axis_body[2] = 0; // coordinates of the x_axis body in the sc body reference frame
    	unit_sc_to_ground_station_body_projected_on_body_xy[0] = unit_sc_to_ground_station_body[0]; // project the unit_sc_to_ground_station_body on the body xy plane
    	unit_sc_to_ground_station_body_projected_on_body_xy[1] = unit_sc_to_ground_station_body[1]; // project the unit_sc_to_ground_station_body on the body xy plane
    	unit_sc_to_ground_station_body_projected_on_body_xy[2] = 0; // project the unit_sc_to_ground_station_body on the body xy plane
	v_norm(norm_unit_sc_to_ground_station_body_projected_on_body_xy, unit_sc_to_ground_station_body_projected_on_body_xy);
    	v_dot( &x_axis_body_dot_sc_to_ground_station_body_projected_on_body_xy, x_axis_body, norm_unit_sc_to_ground_station_body_projected_on_body_xy);
    	azimuth_wtr_to_ground_station_in_sc_refce = acos( x_axis_body_dot_sc_to_ground_station_body_projected_on_body_xy ) ;
    	// // Azimuth angle goes from 0 to 360 in the body -y axis direction (so to the right of the +x axis vector if sc seen from above) (ex: if azimuth = 90 then the ground station is in body -y axis direction of the sc, so starboard). This is to be conistent with the azimuth angle in the ground station reference frame where it is calculated in the +East direction (so the right of the North direction)
    	y_axis_body[0] = 0; y_axis_body[1] = 1; y_axis_body[2] = 0;
    	v_dot(&unit_sc_to_ground_station_body_projected_on_body_xy_dot_y_axis_body, y_axis_body, norm_unit_sc_to_ground_station_body_projected_on_body_xy); 
    	if ( unit_sc_to_ground_station_body_projected_on_body_xy_dot_y_axis_body > 0){ // if the ground station is in the positive body y plane (so port of the sc if sc nadir)
    	  azimuth_wtr_to_ground_station_in_sc_refce = 2*M_PI - azimuth_wtr_to_ground_station_in_sc_refce;
    	}
    	/************************************************************************************************************/
    	/* COMPUTE AZIMUTH ANGLES IN THE REFERENCE FRAME OF THE GROUND STATION */
    	// Azimuth angle of the sc with respect to the ground station in the reference frame of the ground station
    	// This is the angle between the local North of the ground station and the ground station to sc vector (expressed in the ENU reference system of the ground station)  projected on the East-North local plane (which is the xy plane of the ENU)
    	// // Local North in ENU reference frame
    	local_north_in_enu[0] = 0; local_north_in_enu[1] = 1; local_north_in_enu[2] = 0;
    	// // Projection of the ground station to sc vector (expressed in the ENU reference system of the ground station) on the East-North local plane
    	ground_station_to_sc_in_enu_projected_on_east_north_local_plane[0] = ground_station_to_sc_in_enu[0];
    	ground_station_to_sc_in_enu_projected_on_east_north_local_plane[1] = ground_station_to_sc_in_enu[1];
    	ground_station_to_sc_in_enu_projected_on_east_north_local_plane[2] = 0;
    	// // Dot product between the local North and the ground station to sc vector (expressed in the ENU reference system of the ground station) projected on the East-North local plane
	v_norm(unit_ground_station_to_sc_in_enu_projected_on_east_north_local_plane, ground_station_to_sc_in_enu_projected_on_east_north_local_plane);
	v_norm(norm_unit_ground_station_to_sc_in_enu_projected_on_east_north_local_plane, unit_ground_station_to_sc_in_enu_projected_on_east_north_local_plane);
    	v_dot(&ground_station_to_sc_in_enu_projected_on_east_north_local_plane_dot_local_north_in_enu, norm_unit_ground_station_to_sc_in_enu_projected_on_east_north_local_plane, local_north_in_enu);
    	azimuth_wtr_to_ground_station_in_ground_station_refce = acos( ground_station_to_sc_in_enu_projected_on_east_north_local_plane_dot_local_north_in_enu );
    	// // Azimuth angle goes from 0 to 360 in the East direction (ex: if azimuth = 90 then the sc is in the direction of the East wtr to the ground station)
    	local_east_in_enu[0] = 1; local_east_in_enu[1] = 0; local_east_in_enu[2] = 0;
    	v_dot(&ground_station_to_sc_in_enu_projected_on_east_north_local_plane_dot_local_east_in_enu, norm_unit_ground_station_to_sc_in_enu_projected_on_east_north_local_plane, local_east_in_enu);
    	if ( ground_station_to_sc_in_enu_projected_on_east_north_local_plane_dot_local_east_in_enu < 0){ // if the sc is in the West direction
    	  azimuth_wtr_to_ground_station_in_ground_station_refce = 2*M_PI - azimuth_wtr_to_ground_station_in_ground_station_refce;
    	}

    	/************************************************************************************************************/
    	// Range of the sc with respect to the ground station
    	v_mag(&range_wtr_to_ground_station,ground_station_to_sc_in_ecef);
      // Write the results in a file: Time ECEF_sc elevation_refce_sc azimuth_refce_sc elevation_refce_ground_station azimuth_refce_ground_station (in the header I indicate ECEF_ground_station and lla)
      et2utc_c(et_interpolated, "ISOC" ,0 ,255 , time_interpolated);
      //     printf("e_sc %f | a_sc %f | e_g %f | a_g %f\n",elevation_wtr_to_ground_station_in_sc_refce*RAD2DEG, azimuth_wtr_to_ground_station_in_sc_refce*RAD2DEG, elevation_wtr_to_ground_station_in_ground_station_refce*RAD2DEG, azimuth_wtr_to_ground_station_in_ground_station_refce*RAD2DEG);
      if (istep == step_end_interpolation-1){ // only print every dt_output
	//	      etprint(et_interpolated, "");
	//fprintf(SC->fp_coverage_ground_station[iground], "%s %f %f %f %f %f %f %f %f\n",time_interpolated, ecef_sc[0], ecef_sc[1], ecef_sc[2], elevation_wtr_to_ground_station_in_sc_refce*RAD2DEG, azimuth_wtr_to_ground_station_in_sc_refce*RAD2DEG, elevation_wtr_to_ground_station_in_ground_station_refce*RAD2DEG, azimuth_wtr_to_ground_station_in_ground_station_refce*RAD2DEG, range_wtr_to_ground_station); // !!!!!!!!!!!!! uncomment this line and comment line below
	fprintf(SC->fp_coverage_ground_station[iground], "%s\n",time_interpolated); // !!!!!!!!!!!!! comment this line and uncommnet line above

      }

      }
      else{
    	in_sight_of_ground_station = 0;
      et2utc_c(et_interpolated, "ISOC" ,0 ,255 , time_interpolated);
	elevation_wtr_to_ground_station_in_sc_refce = -999; azimuth_wtr_to_ground_station_in_sc_refce = -999; elevation_wtr_to_ground_station_in_ground_station_refce = -999; azimuth_wtr_to_ground_station_in_ground_station_refce = -999; range_wtr_to_ground_station = -999;
	//	fprintf(SC->fp_coverage_ground_station[iground], "%s %f %f %f %f %f %f %f %f\n",time_interpolated, ecef_sc[0], ecef_sc[1], ecef_sc[2], elevation_wtr_to_ground_station_in_sc_refce, azimuth_wtr_to_ground_station_in_sc_refce, elevation_wtr_to_ground_station_in_ground_station_refce, azimuth_wtr_to_ground_station_in_ground_station_refce, range_wtr_to_ground_station);
      }


    }
  }
  /* if ( index_in_attitude_interpolated == 16 ){ */
  /*   exit(0); */
  /* } */

  return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////
//
//  Name:           geodetic_to_geocentric
//  Purpose:        Computes planet fixed state based on lat/long/altitude FOR LOCATIONS ON THE EARTH'S SURFACE ONLY (NOT FOR SATELLITES) (although I don't know, seems it's working ok...)
//  Assumptions:    Works only FOR LOCATIONS ON THE EARTH'S SURFACE ONLY (NOT FOR SATELLITES) (although I don't know, seems it's working ok...). This assumes the Earth is an ellipsoid. To use a spherical Eart, set the flattening parameter to 0 
//  References      Valado 3rd edition section 3.2
//
//  Change Log:
//      |   Developer   |       Date    |   SCR     |   Notes
//      | --------------|---------------|-----------|-------------------------------
//      | C. Bussy-Virat| 10/01/2015    |   ---     | Initial Implementation
//
/////////////////////////////////////////////////////////////////////////////////////////
int geodetic_to_geocentric(double flattening,            /* IN:     flattening parameter     */
			   double h,                     /* IN:  M  height above ellipsoid  (km) */
			   double lat,                   /* IN:  r  geodetic latitude (rad)      */
			   double longitude,              /* IN:  r  longitude  (rad)             */
			   double equatorial_radius,       /* IN: equatorial radius  (km) */
			   double  R_ecef_2_cg_ECEF[3])   /* OUT:     vector in ECEF           */

{
  /* Declarations */
  double c_earth, s_earth, earth_eccentricity, deno, r_xy_mag;

  /* Algorithm */
  earth_eccentricity = sqrt( 1 - (1 - flattening)*(1-flattening) );
  deno = sqrt( 1-earth_eccentricity*earth_eccentricity*sin(lat)*sin(lat) );
  c_earth = equatorial_radius / deno;
  s_earth =  equatorial_radius * (1 - earth_eccentricity*earth_eccentricity) / deno;

  r_xy_mag = ( c_earth + h ) * cos(lat);
  R_ecef_2_cg_ECEF[0] = r_xy_mag * cos(longitude);
  R_ecef_2_cg_ECEF[1] = r_xy_mag * sin(longitude);

  R_ecef_2_cg_ECEF[2] = ( s_earth + h ) * sin(lat);

  // OHTER SOLUTION (SOURCE: http://www.mathworks.com/help/aeroblks/llatoecefposition.html)
  /* /\* Declaration *\/ */
  /* double geocentric_lat_mean_sea_level; */
  /* double radius_at_surface_point; */
  /* double term1, term2; */

  /* /\* Algorithm *\/ */
  /* geocentric_lat_mean_sea_level = atan( ( 1 - flattening ) * ( 1 - flattening ) * tan( lat ) ); */
  
  /* term1 = ( 1 - flattening ) * ( 1 - flattening ); */
  /* term2 = ( 1 / term1 ) - 1 ; */
  /* radius_at_surface_point = sqrt( equatorial_radius * equatorial_radius / ( 1 + term2 *  sin(geocentric_lat_mean_sea_level)  *  sin(geocentric_lat_mean_sea_level ) ) ); */

  /* R_ecef_2_cg_ECEF[0] = radius_at_surface_point * cos(geocentric_lat_mean_sea_level) * cos(longitude) + h * cos(lat) * cos(longitude); */
  /* R_ecef_2_cg_ECEF[1] = radius_at_surface_point * cos(geocentric_lat_mean_sea_level) * sin(longitude) + h * cos(lat) * sin(longitude); */
  /* R_ecef_2_cg_ECEF[2] = radius_at_surface_point * sin(geocentric_lat_mean_sea_level) + h * sin(lat); */

  /* v_print(R_ecef_2_cg_ECEF, "r_ecef"); */
  /* exit(0); */
  return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////
//
//  Name:           Geocentric_to_geodetic
//  Purpose:        Computes lat/long/altitude based on planet fixed state
//  Assumptions:    None.
//  References      Larson & Wertz
//
//  Change Log:
//      |   Developer   |       Date    |   SCR     |   Notes
//      | --------------|---------------|-----------|-------------------------------
//      | J. Getchius   | 05/09/2015    |   ---     | Initial Implementation
//
/////////////////////////////////////////////////////////////////////////////////////////
int geocentric_to_geodetic(
			   double  R_ecef_2_cg_ECEF[3],   /* IN:     vector in ECEF           */
			   double *semimajor_axis,        /* IN:     planetary semimajor axis */
			   double *flattening,            /* IN:     flattening parameter     */
			   double *h,                     /* OUT:  M  height above ellipsoid  */
			   double *lat,                   /* OUT:  r  geodetic latitude       */
			   double *longitude   )          /* OUT:  r  longitude               */
{

  /* Variable declarations */
  double b;
  double R;
  double E;
  double F;
  double P;
  double Q;
  double D;
  double v;
  double G;
  double t;
  double inv3;
  double D2;
  double Domain;
  double D_minus_Q;
  double D_plus_Q;
  double D_minus_Q_pow;
  double D_plus_Q_pow;


  /* Algorithm */
  inv3 = 1.0 / 3.0;

  b = -((*flattening * *semimajor_axis) - *semimajor_axis);

  if (R_ecef_2_cg_ECEF[2] < 0.0) {
    b = (-b);
  }

  R = sqrt( R_ecef_2_cg_ECEF[0]* R_ecef_2_cg_ECEF[0] +  R_ecef_2_cg_ECEF[1]* R_ecef_2_cg_ECEF[1] );

  if ((*semimajor_axis * R ) > SMALL_NUM) {
    E = ( (b*R_ecef_2_cg_ECEF[2] - (*semimajor_axis * *semimajor_axis - b*b))) / (*semimajor_axis *R);
    F = ( (b*R_ecef_2_cg_ECEF[2] + (*semimajor_axis * *semimajor_axis - b*b))) / (*semimajor_axis *R);
  } else {
    E = 0.0;
    F = 0.0;
  }

  P = 4*(E*F + 1.0 )/ 3.0;
  Q = 2*(E*E - F*F);

  /* Sqrt protection */
  D2 = P*P*P + Q*Q;

  if (D2 > 0.0 ) {
    D = sqrt(D2);
  } else {
    D = 0.0;
  }

  /* Pow protection */
  D_minus_Q = D - Q;
  D_plus_Q = D + Q;

  if (D_minus_Q < 0.0) {
    D_minus_Q_pow = pow(fabs(D_minus_Q ), inv3 );
    D_minus_Q_pow = (-D_minus_Q_pow);
  } else {
    D_minus_Q_pow = pow( D_minus_Q , inv3);
  }

  if (D_plus_Q < 0.0) {
    D_plus_Q_pow = pow(fabs(D_plus_Q ), inv3 );
    D_plus_Q_pow = (-D_plus_Q_pow);
  } else {
    D_plus_Q_pow = pow( D_plus_Q , inv3);
  }

  v = D_minus_Q_pow - D_plus_Q_pow;

  Domain = E*E + v;

  /* Sqrt protection */
  if ( Domain > 0.0 ) {
    G = 0.5*(sqrt( Domain ) + E);
  }  else {
    G = 0.0;
  }

  if (fabs(2*G - E) > SMALL_NUM) {
    t = sqrt(G*G + (F - v*G) / (2*G - E)) - G;
  } else {
    t = 0.0 ;
  }


  if (fabs(2.0*b*t) > SMALL_NUM ) {
    *lat = atan( *semimajor_axis * (1.0 - t*t) / (2.0*b*t));
  } else {
    *lat = 0.0;
  }

  if (fabs( R_ecef_2_cg_ECEF[0] ) > SMALL_NUM ) {
    *longitude = atan2( R_ecef_2_cg_ECEF[1] , R_ecef_2_cg_ECEF[0] ) ;

  } else {
    *longitude = M_PI / 2;
  }

  if ( *longitude < 0.0 ) {
    *longitude += (2.0 * M_PI);
  }

  *h = (R - *semimajor_axis * t)*cos( *lat ) + (R_ecef_2_cg_ECEF[2] - b)*sin( *lat );

  return 0;

} /* ---------- end of function geocentric_to_geodetic ----------*/

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Name:           cart2kep
//  Purpose:        Computes the Keplerian elements based on the cartesian inputs
//  Assumptions:    Oscullating elements only.
//  References      BMW  // Note by CBV: all the following equations can also be found in Vallado3 p. 104 to 108.
//
//  Change Log:
//      |   Developer   |       Date    |   SCR     |   Notes
//      | --------------|---------------|-----------|-------------------------------
//      | J. Getchius   | 05/09/2015    |   ---     | Initial Implementation
//
/////////////////////////////////////////////////////////////////////////////////////////
int cart2kep( ORBITAL_ELEMENTS_T   *OE,
              double                r[3],
              double                v[3],
              double                time,
              double                u)

{
  // Declarations
  double rrss;
  double vrss;
  double unit_r[3];
  double rdotv;
  double h[3];
  double mag_h;
  double K[3] = {0.0};
  double node[3];
  double mag_node;
  double unit_node[3];
  double specific_energy;
  double tempv1[3];
  double tempv2[3];
  double coeff;
  double e_vector[3];
  double unit_e[3];
  double e_vector_dot_r;
  double n;
    
  // Begin algorithm
  v_mag( &rrss, r);
  v_mag( &vrss, v);
  v_norm( unit_r, r);
  v_dot(&rdotv, r, v);

  //
  //  Solve for angular momentum (r x v)
  //
  v_cross(h,r,v);
  v_mag( &mag_h, h);

  //
  // Solve for node vector
  //
  K[2] = 1.0;
  v_cross(node, K,h);
  v_mag(&mag_node, node);
  v_norm( unit_node, node);

  //
  //  Solve for semi-major axis, sma
  //
  OE->sma = 1.0 / ( (2.0/rrss) - ( (vrss*vrss)/u ) );
    
  //
  //  Solve for ecentricity, e and the e vector
  //
  specific_energy = -1.0*(u/(2.0*OE->sma));
  v_scale(tempv1, v, rdotv);
    
  coeff = vrss*vrss - (u/rrss);
    
  v_scale(tempv2, r, coeff);
  v_sub( e_vector , tempv2, tempv1);
    
  coeff = 1.0/u;
  v_scale(e_vector, e_vector, (coeff));

  v_mag( &OE->eccentricity, e_vector );
  v_norm( unit_e, e_vector);

  //  Solve for inclination, i
  OE->inclination = acos(h[2]/mag_h);

  //  Solve for longitude of ascending node
  if (mag_node == 0.0) {
    
    OE->long_an = 0.0;
    
  } else if (node[1] >= 0){
    
    // TODO: Check this
    OE->long_an = acos(node[0]/mag_node);  // was checked by CBV (Vallado3 eq (2.82))
        
  }

  else if (node[1] < 0){
    OE->long_an = 2*M_PI - acos(node[0]/mag_node);
  }

  //
  //  Solve for argument of periapse
  //
  if (mag_node != 0.0) {
    v_dot(&coeff, unit_node, unit_e);
    OE->w = acos( coeff );
    if (e_vector[2] < 0.0) {
      OE->w = (2.0*M_PI - OE->w);
    }/*  else { */
    /*   OE->w = 0; */
    /* } */
  }

  //  Solve for true anomaly
  v_dot(&e_vector_dot_r, e_vector,r);
  if (OE->eccentricity != 0) {
    OE->f = acos(e_vector_dot_r/(OE->eccentricity*rrss));
    if (rdotv < 0) {
      OE->f = (2*M_PI) - fabs(OE->f);
      //OE->f = OE->f + M_PI;
    }
  } else {
    OE->f = 0;
  }
  //
  //  Solve for time of periapsis
  //
  OE->E = 2*atan(sqrt((1-OE->eccentricity)/(1+OE->eccentricity))*tan(OE->f/2));
  if (OE->E < 0) {
    OE->E = OE->E + (2*M_PI);
  }

  n = sqrt(u/(OE->sma*OE->sma*OE->sma)); // mean motion - Vallado3 eq (2.76) (CBV)
  OE->tp = -1 * ((OE->E/n) - ((OE->eccentricity*sin(OE->E))/n) - time);

  // Right ascension
  OE->ra =  atan2(r[1], r[0]);
  if ( OE->ra < 0){
    OE->ra = 2*M_PI + OE->ra ;
  }

  return 0;
    
} /* ---------- end of function cart2kep ----------*/

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Name:           kep2cart
//  Purpose:        Computes the ECI coordinates based on the Keplerian inputs
//  Assumptions:    None.
//  References      BMW
//
//  Change Log:
//      |   Developer   |       Date    |   SCR     |   Notes
//      | --------------|---------------|-----------|-------------------------------
//      | J. Getchius   | 05/09/2015    |   ---     | Initial Implementation
//
/////////////////////////////////////////////////////////////////////////////////////////
int kep2cart(   double              r_i2cg_INRTL[3],
                double              v_i2cg_INRTL[3],
                double             *mu,
                ORBITAL_ELEMENTS_T *OE)

{

  /* Declarations */
  double slr;
  double rm;
  double arglat;
  double sarglat;
  double carglat;
  double c4;
  double c5;
  double c6;
  double sinc;
  double cinc;
  double sraan;
  double craan;
    
  /* Algorithm */
  slr = OE->sma * (1 - OE->eccentricity * OE->eccentricity);

  rm = slr / (1 + OE->eccentricity * cos(OE->f));
   
  arglat = OE->w + OE->f;

  sarglat = sin(arglat);
  carglat = cos(arglat);
   
  c4 = sqrt(mu[0] / slr);
  c5 = OE->eccentricity * cos(OE->w) + carglat;
  c6 = OE->eccentricity * sin(OE->w) + sarglat;

  sinc = sin(OE->inclination);
  cinc = cos(OE->inclination);

  sraan = sin(OE->long_an);
  craan = cos(OE->long_an);

  // position vector
  r_i2cg_INRTL[0] = rm * (craan * carglat - sraan * cinc * sarglat);
  r_i2cg_INRTL[1] = rm * (sraan * carglat + cinc * sarglat * craan);
  r_i2cg_INRTL[2] = rm * sinc * sarglat;

  // velocity vector
  v_i2cg_INRTL[0] = -c4 * (craan * c6 + sraan * cinc * c5);
  v_i2cg_INRTL[1] = -c4 * (sraan * c6 - craan * cinc * c5);
  v_i2cg_INRTL[2] = c4 * c5 * sinc;

  return 0;
} /* ---------- end of function kep2cart ----------*/

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Name:           propagate_spacecraft
//  Purpose:        RK4
//  Assumptions:    None.
//  References      BMW
//
//  Change Log:
//      |   Developer   |       Date    |   SCR     |   Notes
//      | --------------|---------------|-----------|-------------------------------
//      | J. Getchius   | 05/09/2015    |   ---     | Initial Implementation
//
/////////////////////////////////////////////////////////////////////////////////////////
int propagate_spacecraft(   SPACECRAFT_T *SC,
                            PARAMS_T     *PARAMS,
			    double et_initial_epoch,
			    double et_sc_initial,
			    double *density,
			    GROUND_STATION_T *GROUND_STATION,
			    OPTIONS_T *OPTIONS,
			    CONSTELLATION_T *CONSTELLATION,
			    int iProc,
			    int iDebugLevel,
			    int *start_ensemble,
			    int *array_sc)

{
  //    etprint(SC->et, "before propagate");

  //      printf("\n");
  // Declarations
  //  int start_ensemble;
  double geodetic[3];
  SpiceDouble       xform[6][6];
  double estate[6], jstate[6];
  double k1r[3];
  double k1v[3];
  double k2r[3];
  double k2v[3];
  double k3r[3];
  double k3v[3];
  double k4r[3];
  double k4v[3];
  double dr[3];
  double dv[3];
  double etk;
  double rk[3];
  double vk[3];
  double sc_ecef_previous_time_step[3];  double sc_eci_previous_time_step[3];  double sc_v_eci_previous_time_step[3];

/*   /\*************************************************************************************\/ */
/*   /\*************************************************************************************\/ */
/*   /\*********************** SET THINGS UP FOR PARALLEL PROGRAMMING **********************\/ */
/*   /\*************************************************************************************\/ */
/*   /\*************************************************************************************\/ */
/*   /\*   Things we set up here: *\/ */
/*   /\*   - which iProc runs which main sc / which sc is run by which iProc *\/ */


/*   // For each iProc, set up the first main sc (iStart_save[iProcf]) and the last main sc (iEnd_save[iProcf]) that it's going to run. iStart_save and iEnd_save are two arrays that have the same values for all procs (so if you're iProc 0 or iProc 1, you have value recorded for iStart_save[0] and iEnd_save[0] and the same value recorded for iStart_save[1] and iEnd_save[1]) -> they are not "iProc-dependent" */
/*   int *iStart_save, *iEnd_save; */
/*   int nscEachPe, nscLeft; */
/*   int iProcf; */
/*   int i; */
/*   nscEachPe = (OPTIONS->n_satellites)/nProcs; */
/*   nscLeft = (OPTIONS->n_satellites) - (nscEachPe * nProcs); */

/*   iStart_save = malloc( nProcs * sizeof(int)); */
/*   iEnd_save = malloc( nProcs  * sizeof(int)); */
/*   for (iProcf = 0; iProcf < nProcs; iProcf++){ */
/*     iStart_save[iProcf] = 0; */
/*     iEnd_save[iProcf] = 0; */
/*   } */
/*   for (iProcf = 0; iProcf < nProcs; iProcf++){ */
/*     for (i=0; i<iProcf; i++) { */
/*       iStart_save[iProcf] += nscEachPe; */
/*       if (i < nscLeft && iProcf > 0) iStart_save[iProcf]++; */
/*     } */
/*     iEnd_save[iProcf] = iStart_save[iProcf]+nscEachPe; */
/*     if (iProcf  < nscLeft) iEnd_save[iProcf]++; */
/*     iStart_save[iProcf] = iStart_save[iProcf]; */
/*     iEnd_save[iProcf] = iEnd_save[iProcf]; */
/*   } */
    
/*   //    if (iProc == 0){ */
/*   /\*     for (iProcf = 0; iProcf < nProcs; iProcf++){ *\/ */
/*   /\*       printf("%d - %d\n", iStart_save[iProcf], iEnd_save[iProcf] - 1) ; *\/ */
/*   /\*     } *\/ */
/*   //} */

/*   // For each main sc, start_ensemble is 0 if the iProc runs this main sc. start_ensemble is 1 is the iProc does not run this main sc. (so each iProc has a different array start_ensemble -> start_ensemble is "iProc-dependent")  */
/*   int *start_ensemble; */
/*   start_ensemble = malloc(OPTIONS->n_satellites * sizeof(int)); */
/*   for (ii = 0; ii < OPTIONS->n_satellites; ii++){ */
/*     if ( (ii >= iStart_save[iProc]) & ( ii < iEnd_save[iProc]) ){ */
/*       start_ensemble[ii] = 0; */
/*     } */
/*     else{ */
/*       start_ensemble[ii] = 1; */
/*     } */
/*     //    printf("iProc %d | start_ensemble[%d] %d\n", iProc, ii, start_ensemble[ii]); */
/*   } */

/* /\*   if ( (iProc == 0) && (OPTIONS->first_run_to_find_tca_before_collision_assessment == 1)){ *\/ */
/* /\*     start_ensemble = 0; *\/ */
/* /\*   } *\/ */
/* /\*   else{ *\/ */
/* /\*     start_ensemble = 1; *\/ */
/* /\*   } *\/ */

/*   /\*************************************************************************************\/ */
/*   /\*************************************************************************************\/ */
/*   /\****************** end of SET THINGS UP FOR PARALLEL PROGRAMMING ********************\/ */
/*   /\*************************************************************************************\/ */
/*   /\*************************************************************************************\/ */

  SC->INTEGRATOR.write_given_output = 0;

  // Compute k
  v_copy(sc_ecef_previous_time_step, SC->r_ecef2cg_ECEF);
  v_copy(sc_eci_previous_time_step, SC->r_i2cg_INRTL);
  v_copy(sc_v_eci_previous_time_step, SC->v_i2cg_INRTL);

  // the only case where the attitute needs to be set is the only case where it has not been set initially in initialize_constellation, which is if there is no ensemble at all run on the attitude
  if ( ( strcmp(OPTIONS->attitude_profile, "ensemble_angular_velocity") != 0 ) && ( strcmp(OPTIONS->attitude_profile, "ensemble_initial_attitude") != 0 ) ) { // if we do not run ensembles on the initial angular velocity

    if (OPTIONS->nb_ensembles_attitude <= 0){ // if we dont' run ensembles of any kind on the attitude

      if ( (SC->INTEGRATOR.solar_cell_efficiency != -1) || (GROUND_STATION->nb_ground_stations > 0) || (SC->INTEGRATOR.include_solar_pressure == 1) || (SC->INTEGRATOR.include_drag == 1) ){ // these are the case where SpOCK uses the attitude

	if (SC->INTEGRATOR.isGPS == 0){ // no attitude to set up for GPS because we dont compute the drag, solar rariatin pressu,re, power, and gorund station coverage (attitude is needed for coverage because we calculate azimuth and levation angles form the spaceecraft reference system)
	  
	  set_attitude(SC->INTEGRATOR.attitude, SC->INTEGRATOR.index_in_attitude_interpolated, OPTIONS, SC->INTEGRATOR.file_is_quaternion);

	}
      }
    }
  }
  // end of if the only case where the attitute needs to be set is the only case where it has not been set initially in initialize_constellation, which is if there is no ensemble at all run on the attitude
  SC->INTEGRATOR.last_compute_dxdt = 0; // dont worry about it


  compute_dxdt( k1r, k1v, &SC->et, SC->r_i2cg_INRTL, SC->v_i2cg_INRTL, PARAMS, &SC->INTEGRATOR, et_initial_epoch, et_sc_initial, density, SC->INTEGRATOR.index_in_attitude_interpolated, SC->INTEGRATOR.index_in_driver_interpolated, CONSTELLATION, OPTIONS, iProc, iDebugLevel, SC);
  v_scale(k1r, k1r, SC->INTEGRATOR.dt);
  v_scale(k1v, k1v, SC->INTEGRATOR.dt);

  // Compute k2
  etk = SC->et + SC->INTEGRATOR.dt / 2.0;
    
  v_copy( dr, k1r);
  v_scale(dr, dr, 0.5);
  v_add(rk, SC->r_i2cg_INRTL, dr);
    
  v_copy( dv, k1v);

  v_scale(dv, dv, 0.5);
  v_add(vk, SC->v_i2cg_INRTL, dv);

  if ( ( SC->INTEGRATOR.include_drag != 0 ) || ( SC->INTEGRATOR.include_solar_pressure != 0 ) || ( SC->INTEGRATOR.solar_cell_efficiency != -1 ) || (GROUND_STATION->nb_ground_stations > 0) ){
    //    if (SC->et >= et_initial_epoch){ //  that's because if the initializtion of the orbit was made with a TLE, we don't want index_in_attitude_interpolated to be incremented for times between the TLE epoch and the inital epoch 
      SC->INTEGRATOR.index_in_attitude_interpolated =  SC->INTEGRATOR.index_in_attitude_interpolated + 1;
    
      if ( SC->INTEGRATOR.include_drag != 0 ){
	SC->INTEGRATOR.index_in_driver_interpolated =  SC->INTEGRATOR.index_in_driver_interpolated + 1;
      }
      //    }
  }

  if ( (OPTIONS->nb_ensembles_density > 0) && (OPTIONS->swpc_need_predictions) && ( SC->et >= OPTIONS->swpc_et_first_prediction ) ) { 
	if( ( (array_sc[start_ensemble[SC->INTEGRATOR.sc_main_nb]] >= 0) ) && (SC->INTEGRATOR.sc_ensemble_nb == array_sc[start_ensemble[SC->INTEGRATOR.sc_main_nb]] )){ // used to be	if( ( (array_sc[start_ensemble[SC->INTEGRATOR.sc_ensemble_nb]] >= 0) ) && (SC->INTEGRATOR.sc_ensemble_nb == array_sc[start_ensemble[SC->INTEGRATOR.sc_main_nb]] )){  but doesn't make senese because start_ensemble is defined only for the number of main sc so start_ensemble[xxx] doesnt exist for xxx = SC->INTEGRATOR.sc_ensemble_nb (imagine there's 2 main sc and 10 ensbles then start_ensemble is defined for xxx = 0 or 1, not 2, 3, ..., 10. The explanation right below was for this if that used to be here so not sure it's all correct either
      // if this iProc is running main sc SC->INTEGRATOR.sc_main_nb then start_ensemble[SC->INTEGRATOR.sc_main_nb] = 0. If this iProc is not running main sc SC->INTEGRATOR.sc_main_nb but is running an ensemble corresponding to main sc SC->INTEGRATOR.sc_main_nb, then start_ensemble[SC->INTEGRATOR.sc_main_nb] = 1. If none of these two, then start_ensemble[SC->INTEGRATOR.sc_main_nb] = -1. So here we don't want to get in this if the iProc does not run main sc or an ensemble corresponding to this main sc (if (array_sc[start_ensemble[SC->INTEGRATOR.sc_ensemble_nb]] >= 0)). And we want to get in only once, for the first sc that this iProc is running

      CONSTELLATION->aaa_sigma[SC->INTEGRATOR.sc_main_nb] = CONSTELLATION->aaa_sigma[SC->INTEGRATOR.sc_main_nb] + 1;
    }
  }
  if ( (strcmp(OPTIONS->test_omniweb_or_external_file, "swpc_mod") == 0) && (OPTIONS->swpc_need_predictions) && ( SC->et >= OPTIONS->swpc_et_first_prediction ) ){
	if( ( (array_sc[start_ensemble[SC->INTEGRATOR.sc_main_nb]] >= 0) ) && (SC->INTEGRATOR.sc_ensemble_nb == array_sc[start_ensemble[SC->INTEGRATOR.sc_main_nb]] )){ // used to be	if( ( (array_sc[start_ensemble[SC->INTEGRATOR.sc_ensemble_nb]] >= 0) ) && (SC->INTEGRATOR.sc_ensemble_nb == array_sc[start_ensemble[SC->INTEGRATOR.sc_main_nb]] )){  but doesn't make senese because start_ensemble is defined only for the number of main sc so start_ensemble[xxx] doesnt exist for xxx = SC->INTEGRATOR.sc_ensemble_nb (imagine there's 2 main sc and 10 ensbles then start_ensemble is defined for xxx = 0 or 1, not 2, 3, ..., 10. The explanation right below was for this if that used to be here so not sure it's all correct either
      // if this iProc is running main sc SC->INTEGRATOR.sc_main_nb then start_ensemble[SC->INTEGRATOR.sc_main_nb] = 0. If this iProc is not running main sc SC->INTEGRATOR.sc_main_nb but is running an ensemble corresponding to main sc SC->INTEGRATOR.sc_main_nb, then start_ensemble[SC->INTEGRATOR.sc_main_nb] = 1. If none of these two, then start_ensemble[SC->INTEGRATOR.sc_main_nb] = -1. So here we don't want to get in this if the iProc does not run main sc or an ensemble corresponding to this main sc (if (array_sc[start_ensemble[SC->INTEGRATOR.sc_ensemble_nb]] >= 0)). And we want to get in only once, for the first sc that this iProc is running

      CONSTELLATION->aaa_mod[SC->INTEGRATOR.sc_main_nb] = CONSTELLATION->aaa_mod[SC->INTEGRATOR.sc_main_nb] + 1;

    }
  }



  // the only case where the attitute needs to be set is the only case where it has not been set initially in initialize_constellation, which is if there is no ensemble at all run on the attitude
  if ( ( strcmp(OPTIONS->attitude_profile, "ensemble_angular_velocity") != 0 ) && ( strcmp(OPTIONS->attitude_profile, "ensemble_initial_attitude") != 0 ) ) { // if we do not run ensembles on the initial angular velocity
    if (OPTIONS->nb_ensembles_attitude <= 0){
      if ( (SC->INTEGRATOR.solar_cell_efficiency != -1) || (GROUND_STATION->nb_ground_stations > 0) || (SC->INTEGRATOR.include_solar_pressure == 1) || (SC->INTEGRATOR.include_drag == 1) ){ // these are the case where SpOCK uses the attitude
	if (SC->INTEGRATOR.isGPS == 0){ // no attitude to set up for GPS because we dont compute the drag, solar rariatin pressu,re, power, and gorund station coverage (attitude is needed for coverage because we calculate azimuth and levation angles form the spaceecraft reference system)
	  set_attitude(SC->INTEGRATOR.attitude, SC->INTEGRATOR.index_in_attitude_interpolated, OPTIONS, SC->INTEGRATOR.file_is_quaternion);
	}
      }
    }
  }
  // end of if the only case where the attitute needs to be set is the only case where it has not been set initially in initialize_constellation, which is if there is no ensemble at all run on the attitude

  compute_dxdt( k2r, k2v, &etk, rk, vk, PARAMS, &SC->INTEGRATOR, et_initial_epoch, et_sc_initial,density, SC->INTEGRATOR.index_in_attitude_interpolated, SC->INTEGRATOR.index_in_driver_interpolated, CONSTELLATION, OPTIONS, iProc,iDebugLevel, SC);


  v_scale(k2r, k2r, SC->INTEGRATOR.dt);
  v_scale(k2v, k2v, SC->INTEGRATOR.dt);

  // Compute k3
  etk = SC->et + SC->INTEGRATOR.dt / 2.0;
    
  v_copy( dr, k2r);
  v_scale(dr, dr, 0.5);
  v_add(rk, SC->r_i2cg_INRTL, dr);

  v_copy( dv, k2v);
  v_scale(dv, dv, 0.5);
  v_add(vk, SC->v_i2cg_INRTL, dv);


  compute_dxdt( k3r, k3v, &etk, rk, vk, PARAMS, &SC->INTEGRATOR, et_initial_epoch, et_sc_initial, density, SC->INTEGRATOR.index_in_attitude_interpolated, SC->INTEGRATOR.index_in_driver_interpolated, CONSTELLATION, OPTIONS, iProc, iDebugLevel, SC);

  v_scale(k3r, k3r, SC->INTEGRATOR.dt);
  v_scale(k3v, k3v, SC->INTEGRATOR.dt);
    
  // Compute k4
  etk = SC->et + SC->INTEGRATOR.dt;

  v_copy( dr, k3r);
  v_add(rk, SC->r_i2cg_INRTL, dr);
    
  v_copy( dv, k3v);
  v_add(vk, SC->v_i2cg_INRTL, dv);

  if ( ( SC->INTEGRATOR.include_drag != 0 ) || ( SC->INTEGRATOR.include_solar_pressure != 0 ) || ( SC->INTEGRATOR.solar_cell_efficiency != -1 )  || (GROUND_STATION->nb_ground_stations > 0) ){
    //    if (SC->et >= et_initial_epoch){ //  that's because if the initializtion of the orbit was made with a TLE, we don't want index_in_attitude_interpolated to be incremented for times between the TLE epoch and the inital epoch 
      SC->INTEGRATOR.index_in_attitude_interpolated =  SC->INTEGRATOR.index_in_attitude_interpolated + 1;
      if ( SC->INTEGRATOR.include_drag != 0 ){
	SC->INTEGRATOR.index_in_driver_interpolated =  SC->INTEGRATOR.index_in_driver_interpolated + 1;
      }
      //    }
  }
  if ( (OPTIONS->nb_ensembles_density > 0) && (OPTIONS->swpc_need_predictions) && ( SC->et >= OPTIONS->swpc_et_first_prediction ) ) { 
	if( ( (array_sc[start_ensemble[SC->INTEGRATOR.sc_main_nb]] >= 0) ) && (SC->INTEGRATOR.sc_ensemble_nb == array_sc[start_ensemble[SC->INTEGRATOR.sc_main_nb]] )){ // used to be	if( ( (array_sc[start_ensemble[SC->INTEGRATOR.sc_ensemble_nb]] >= 0) ) && (SC->INTEGRATOR.sc_ensemble_nb == array_sc[start_ensemble[SC->INTEGRATOR.sc_main_nb]] )){  but doesn't make senese because start_ensemble is defined only for the number of main sc so start_ensemble[xxx] doesnt exist for xxx = SC->INTEGRATOR.sc_ensemble_nb (imagine there's 2 main sc and 10 ensbles then start_ensemble is defined for xxx = 0 or 1, not 2, 3, ..., 10. The explanation right below was for this if that used to be here so not sure it's all correct either
      // if this iProc is running main sc SC->INTEGRATOR.sc_main_nb then start_ensemble[SC->INTEGRATOR.sc_main_nb] = 0. If this iProc is not running main sc SC->INTEGRATOR.sc_main_nb but is running an ensemble corresponding to main sc SC->INTEGRATOR.sc_main_nb, then start_ensemble[SC->INTEGRATOR.sc_main_nb] = 1. If none of these two, then start_ensemble[SC->INTEGRATOR.sc_main_nb] = -1. So here we don't want to get in this if the iProc does not run main sc or an ensemble corresponding to this main sc (if (array_sc[start_ensemble[SC->INTEGRATOR.sc_ensemble_nb]] >= 0)). And we want to get in only once, for the first sc that this iProc is running

      CONSTELLATION->aaa_sigma[SC->INTEGRATOR.sc_main_nb] = CONSTELLATION->aaa_sigma[SC->INTEGRATOR.sc_main_nb] + 1;
    }
  }
  if ( (strcmp(OPTIONS->test_omniweb_or_external_file, "swpc_mod") == 0) && (OPTIONS->swpc_need_predictions) && ( SC->et >= OPTIONS->swpc_et_first_prediction ) ){
	if( ( (array_sc[start_ensemble[SC->INTEGRATOR.sc_main_nb]] >= 0) ) && (SC->INTEGRATOR.sc_ensemble_nb == array_sc[start_ensemble[SC->INTEGRATOR.sc_main_nb]] )){ // used to be	if( ( (array_sc[start_ensemble[SC->INTEGRATOR.sc_ensemble_nb]] >= 0) ) && (SC->INTEGRATOR.sc_ensemble_nb == array_sc[start_ensemble[SC->INTEGRATOR.sc_main_nb]] )){  but doesn't make senese because start_ensemble is defined only for the number of main sc so start_ensemble[xxx] doesnt exist for xxx = SC->INTEGRATOR.sc_ensemble_nb (imagine there's 2 main sc and 10 ensbles then start_ensemble is defined for xxx = 0 or 1, not 2, 3, ..., 10. The explanation right below was for this if that used to be here so not sure it's all correct either
      // if this iProc is running main sc SC->INTEGRATOR.sc_main_nb then start_ensemble[SC->INTEGRATOR.sc_main_nb] = 0. If this iProc is not running main sc SC->INTEGRATOR.sc_main_nb but is running an ensemble corresponding to main sc SC->INTEGRATOR.sc_main_nb, then start_ensemble[SC->INTEGRATOR.sc_main_nb] = 1. If none of these two, then start_ensemble[SC->INTEGRATOR.sc_main_nb] = -1. So here we don't want to get in this if the iProc does not run main sc or an ensemble corresponding to this main sc (if (array_sc[start_ensemble[SC->INTEGRATOR.sc_ensemble_nb]] >= 0)). And we want to get in only once, for the first sc that this iProc is running

      CONSTELLATION->aaa_mod[SC->INTEGRATOR.sc_main_nb] = CONSTELLATION->aaa_mod[SC->INTEGRATOR.sc_main_nb] + 1;
    }
  }

  // the only case where the attitute needs to be set is the only case where it has not been set initially in initialize_constellation, which is if there is no ensemble at all run on the attitude
  if ( ( strcmp(OPTIONS->attitude_profile, "ensemble_angular_velocity") != 0 ) && ( strcmp(OPTIONS->attitude_profile, "ensemble_initial_attitude") != 0 ) ) { // if we do not run ensembles on the initial angular velocity
    if (OPTIONS->nb_ensembles_attitude <= 0){
      if ( (SC->INTEGRATOR.solar_cell_efficiency != -1) || (GROUND_STATION->nb_ground_stations > 0) || (SC->INTEGRATOR.include_solar_pressure == 1) || (SC->INTEGRATOR.include_drag == 1) ){ // these are the case where SpOCK uses the attitude
	if (SC->INTEGRATOR.isGPS == 0){ // no attitude to set up for GPS because we dont compute the drag, solar rariatin pressu,re, power, and gorund station coverage (attitude is needed for coverage because we calculate azimuth and levation angles form the spaceecraft reference system)
	  set_attitude(SC->INTEGRATOR.attitude, SC->INTEGRATOR.index_in_attitude_interpolated, OPTIONS, SC->INTEGRATOR.file_is_quaternion);
	}
      }
    }
  }
  // end of if the only case where the attitute needs to be set is the only case where it has not been set initially in initialize_constellation, which is if there is no ensemble at all run on the attitude
  //  printf("%f %f %f %d %d %d\n", SC->INTEGRATOR.attitude.pitch[SC->INTEGRATOR.index_in_attitude_interpolated], SC->INTEGRATOR.attitude.roll[SC->INTEGRATOR.index_in_attitude_interpolated], SC->INTEGRATOR.attitude.yaw[SC->INTEGRATOR.index_in_attitude_interpolated], SC->INTEGRATOR.attitude.order_pitch[SC->INTEGRATOR.index_in_attitude_interpolated], SC->INTEGRATOR.attitude.order_roll[SC->INTEGRATOR.index_in_attitude_interpolated], SC->INTEGRATOR.attitude.order_yaw[SC->INTEGRATOR.index_in_attitude_interpolated]);
  compute_dxdt( k4r, k4v, &etk, rk, vk, PARAMS, &SC->INTEGRATOR, et_initial_epoch, et_sc_initial, density, SC->INTEGRATOR.index_in_attitude_interpolated, SC->INTEGRATOR.index_in_driver_interpolated, CONSTELLATION, OPTIONS, iProc, iDebugLevel, SC);

  v_scale(k4r, k4r, SC->INTEGRATOR.dt);
  v_scale(k4v, k4v, SC->INTEGRATOR.dt);
  
  //  Compute the delta r, delta v
  v_copy( dr, k4r);
  v_scale( k3r, k3r, 2.0);
  v_add(dr, dr, k3r);
  v_scale( k2r, k2r, 2.0);
  v_add(dr, dr, k2r);
  v_add(dr, dr, k1r);
  v_scale( dr, dr, (1.0/6.0));
    
  v_copy( dv, k4v);
  v_scale( k3v, k3v, 2.0);
  v_add(dv, dv, k3v);
  v_scale( k2v, k2v, 2.0);
  v_add(dv, dv, k2v);
  v_add(dv, dv, k1v);
  v_scale( dv, dv, (1.0/6.0));

  // Update Inertial State
  SC->et = SC->et + SC->INTEGRATOR.dt;
  v_add( SC->r_i2cg_INRTL, SC->r_i2cg_INRTL, dr);
  v_add( SC->v_i2cg_INRTL, SC->v_i2cg_INRTL, dv);

  double starttime;
  str2et_c(OPTIONS->initial_epoch, &starttime);
  double min_end_time;
  str2et_c(OPTIONS->final_epoch, &min_end_time);

  double dvdt[3], drdt[3];
	    if ( ( fmod( SC->et - starttime, OPTIONS->dt_output ) < OPTIONS->dt / 2.) || ( fabs( fmod( SC->et - starttime, OPTIONS->dt_output ) - OPTIONS->dt_output ) < OPTIONS->dt / 2.) || ( SC->et > min_end_time - 0.01)  )  {

  SC->INTEGRATOR.write_given_output = 1;
	    }
	    //	    printf("\n\n");
	    SC->INTEGRATOR.last_compute_dxdt = 1;
	    compute_dxdt( drdt, dvdt, &SC->et,  SC->r_i2cg_INRTL, SC->v_i2cg_INRTL, PARAMS, &SC->INTEGRATOR, et_initial_epoch, et_sc_initial, density, SC->INTEGRATOR.index_in_attitude_interpolated, SC->INTEGRATOR.index_in_driver_interpolated, CONSTELLATION, OPTIONS, iProc, iDebugLevel, SC);
	    SC->INTEGRATOR.last_compute_dxdt = 0;
  v_copy( SC->a_i2cg_INRTL,  dvdt);

  
  // For Kalman: convert acceleration from inertial to lvlh
  double T_inrtl_2_lvlh[3][3];
  compute_T_inrtl_2_lvlh(T_inrtl_2_lvlh, SC->r_i2cg_INRTL, SC->v_i2cg_INRTL);
  m_x_v(SC->a_i2cg_LVLH, T_inrtl_2_lvlh, SC->a_i2cg_INRTL);
  m_x_v(SC->a_i2cg_LVLH_gravity, T_inrtl_2_lvlh, SC->a_i2cg_INRTL_gravity);
  m_x_v(SC->a_i2cg_LVLH_drag, T_inrtl_2_lvlh, SC->a_i2cg_INRTL_drag);


 
  // Update ECEF state
  estate[0] = SC->r_i2cg_INRTL[0];estate[1] = SC->r_i2cg_INRTL[1];estate[2] = SC->r_i2cg_INRTL[2];
  estate[3] = SC->v_i2cg_INRTL[0];estate[4] = SC->v_i2cg_INRTL[1];estate[5] = SC->v_i2cg_INRTL[2];
  sxform_c (  "J2000", PARAMS->EARTH.earth_fixed_frame,  SC->et,    xform  );

  mxvg_c   (  xform,       estate,   6,  6, jstate );
  SC->r_ecef2cg_ECEF[0] = jstate[0]; SC->r_ecef2cg_ECEF[1] = jstate[1]; SC->r_ecef2cg_ECEF[2] = jstate[2];
  SC->v_ecef2cg_ECEF[0] = jstate[3]; SC->v_ecef2cg_ECEF[1] = jstate[4]; SC->v_ecef2cg_ECEF[2] = jstate[5];

   // Update Geodetic State
  geocentric_to_geodetic( SC->r_ecef2cg_ECEF,
			  &PARAMS->EARTH.radius,
			  &PARAMS->EARTH.flattening,
			  &SC->GEODETIC.altitude,
			  &SC->GEODETIC.latitude,
			  &SC->GEODETIC.longitude);

/* /\*    // Update Geodetic State *\/ */
/* 	eci2lla(SC->r_i2cg_INRTL , SC->et, geodetic ); */
	
/* 	SC->GEODETIC.altitude = geodetic[2]; */
/* 	SC->GEODETIC.latitude = geodetic[0]; */
/* 	SC->GEODETIC.longitude = geodetic[1];  */



/* 	// update the planet fixed state		 */
/* 	  geodetic_to_geocentric(PARAMS->EARTH.flattening,             */
/* 				 SC->GEODETIC.altitude, */
/* 				 SC->GEODETIC.latitude, */
/* 				 SC->GEODETIC.longitude, */
/* 				 PARAMS->EARTH.radius,        */
/* 				 SC->r_ecef2cg_ECEF) ; */

  // Update Keplerian State
  cart2kep(&SC->OE, SC->r_i2cg_INRTL, SC->v_i2cg_INRTL, SC->et, PARAMS->EARTH.GRAVITY.mu);

  // Returns if the satellite is or not in the shadow of the Earth
  shadow_light( SC->INTEGRATOR.shadow, SC->r_i2cg_INRTL, SC->et, PARAMS);

  // Returns if the satellite is or not in the shadow of the Moon
  shadow_light_moon( SC->INTEGRATOR.shadow_moon, SC->r_i2cg_INRTL, SC->et, PARAMS);


  // Compute power from solar arrays

  if (SC->INTEGRATOR.solar_cell_efficiency != -1){
    compute_power(&SC->INTEGRATOR, SC->r_i2cg_INRTL, SC->v_i2cg_INRTL, &SC->et, PARAMS, et_initial_epoch, et_sc_initial,SC->INTEGRATOR.index_in_attitude_interpolated);
  }  


  if (SC->INTEGRATOR.isGPS == 0){
    if (GROUND_STATION->nb_ground_stations > 0){
      if (SC->INTEGRATOR.index_in_attitude_interpolated > 0){
	coverage_ground_station( SC, GROUND_STATION, PARAMS, SC->INTEGRATOR.index_in_attitude_interpolated, &SC->INTEGRATOR, et_initial_epoch, et_sc_initial,sc_ecef_previous_time_step, sc_eci_previous_time_step, sc_v_eci_previous_time_step, 1 ); // last argument is the time step in seconds for the linear interpolation of the sc position when computing the coverage of the ground stations by the sc
      }
    }
  }

  //    etprint(SC->et, "after propagate");
  return 0;
}  /* ---------- end of function propagate_spacecraft ----------*/

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Name:           compute_dxdt
//  Purpose:        Computes the dxdt for position / velocity in RK4 integrator
//  Assumptions:    None
//  References      BMW
//
//  Change Log:
//      |   Developer   |       Date    |   SCR     |   Notes
//      | --------------|---------------|-----------|-------------------------------
//      | J. Getchius   | 05/09/2015    |   ---     | Initial Implementation
//      | J. Getchius   | 05/20/2015    |   ---     | Update with MSIS and EGM96 gravity
//      | C. Bussy-Virat| 08/02/2015    |   ---     | Sun and Moon perturbations: correction of the direct effect (Vallado3 page 35 (eq (1-35)))
//
/////////////////////////////////////////////////////////////////////////////////////////
int compute_dxdt(   double          drdt[3],
                    double          dvdt[3],
                    double          *et,
                    double          r_i2cg_INRTL[3],
                    double          v_i2cg_INRTL[3],
                    PARAMS_T        *PARAMS,
                    INTEGRATOR_T    *INTEGRATOR,
		    double          et_initial_epoch, 
		    double          et_sc_initial, 
		    double          *density,
		    int             index_in_attitude_interpolated, 
		    int             index_in_driver_interpolated,
		    CONSTELLATION_T *CONSTELLATION,
		    OPTIONS_T       *OPTIONS,
		    int iProc,
		    int iDebugLevel,
		    SPACECRAFT_T *SC)


{

  // Declarations
  double r;
  double r3;
  double r_earth2moon_J2000[3];
  double x[6];
  double lt;
  double pert_direct[3];
  double pert_indirect[3];
  double pert_total[3];
  double coeff;
  double r_cg2moon_J2000[3];
  double r_cg2sun_J2000[3];
  double r_earth2sun_J2000[3];
  double a_solar_pressure_INRTL[3];
  double adrag_i2cg_INRTL[3];


  /* // drdt */
  v_copy( drdt, v_i2cg_INRTL);

  // dvdt

  // Gravity
  compute_gravity( dvdt, r_i2cg_INRTL, et[0], &PARAMS->EARTH.GRAVITY, INTEGRATOR->degree, PARAMS->EARTH.earth_fixed_frame, PARAMS->EARTH.flattening, PARAMS->EARTH.radius, SC);

  //  Moon Perturbations
  if (INTEGRATOR->include_moon == 1){

    spkez_c(301, et[0], "J2000", "NONE", 399, x, &lt);
    r_earth2moon_J2000[0] = x[0];
    r_earth2moon_J2000[1] = x[1];
    r_earth2moon_J2000[2] = x[2];
    
    v_mag( &r, r_earth2moon_J2000);
    r3 = pow( r, 3.0 );
    coeff = PARAMS->MOON.GRAVITY.mu / r3;
    v_copy(pert_indirect, r_earth2moon_J2000);
    v_scale(pert_indirect, pert_indirect, coeff);
    
    v_sub(r_cg2moon_J2000, r_earth2moon_J2000, r_i2cg_INRTL);
    v_mag( &r, r_cg2moon_J2000);
    r3 = pow( r, 3.0 );
    coeff = PARAMS->MOON.GRAVITY.mu / r3;
    v_copy(pert_direct, r_cg2moon_J2000); // used to be: v_copy(pert_direct, r_earth2moon_J2000) but corrected based on Vallado3 page 35 equation (1-35)
    v_scale(pert_direct, pert_direct, coeff);
    v_sub(pert_total, pert_direct , pert_indirect);
    v_add(dvdt, dvdt , pert_total);
  }

  /* Sun Perturbations */
  if (INTEGRATOR->include_sun == 1){

    spkez_c(10, et[0], "J2000", "NONE", 399, x, &lt); //   Return the state (position and velocity) of a target body relative to an observing body, optionally corrected for light time (planetary aberration) and stellar aberration.

    r_earth2sun_J2000[0] = x[0];
    r_earth2sun_J2000[1] = x[1];
    r_earth2sun_J2000[2] = x[2];
    
    v_mag( &r, r_earth2sun_J2000);
    r3 = pow( r, 3.0 );
    coeff = PARAMS->SUN.GRAVITY.mu / r3;
    v_copy(pert_indirect, r_earth2sun_J2000);
    v_scale(pert_indirect, pert_indirect, coeff);

    v_sub(r_cg2sun_J2000, r_earth2sun_J2000, r_i2cg_INRTL);
    v_mag( &r, r_cg2sun_J2000);
    r3 = pow( r, 3.0 );
    coeff = PARAMS->SUN.GRAVITY.mu / r3;
    v_copy(pert_direct, r_cg2sun_J2000); // used to be: v_copy(pert_direct, r_earth2sun_J2000) but corrected based on Vallado3 page 35 equation (1-35)
    v_scale(pert_direct, pert_direct, coeff);
    v_sub(pert_total, pert_direct , pert_indirect);
    v_add(dvdt, dvdt , pert_total);
  }


  /* Solar radiation pressure */
  if (INTEGRATOR->include_solar_pressure == 1){
    compute_solar_pressure(a_solar_pressure_INRTL, r_i2cg_INRTL, v_i2cg_INRTL, et[0], PARAMS, INTEGRATOR, et_initial_epoch, et_sc_initial, index_in_attitude_interpolated);
    v_add(dvdt, dvdt , a_solar_pressure_INRTL);
  }
  // Drag

  if (INTEGRATOR->include_drag == 1){

    compute_drag( adrag_i2cg_INRTL, r_i2cg_INRTL, v_i2cg_INRTL, et[0], PARAMS, INTEGRATOR, et_initial_epoch, et_sc_initial, density, index_in_attitude_interpolated, index_in_driver_interpolated, CONSTELLATION, OPTIONS, iProc, iDebugLevel, SC);
    v_add( dvdt, dvdt, adrag_i2cg_INRTL);
  }

  return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////
//
//  Name:           load_gravity
//  Purpose:        Loads EGM96 gravity coefficients
//  Assumptions:    This isn't agnositic to file format, don't change current file
//  References      BMW
//
//  Change Log:
//      |   Developer   |       Date    |   SCR     |   Notes
//      | --------------|---------------|-----------|-------------------------------
//      | J. Getchius   | 05/17/2015    |   ---     | Initial Implementation
//      | C. Bussy-Virat| 10/14/2015    |   ---     | Change the inputs
//
/////////////////////////////////////////////////////////////////////////////////////////
//newstructure
//int load_gravity( GRAVITY_T *GRAVITY, char main_directory_location[256])
int load_gravity( GRAVITY_T *GRAVITY,  char path_to_spice[256])
//newstructure
{
  FILE *fp = NULL;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  int l = 0;
  int m = 0;
  float C;
  float S;
  float Csig;
  float Ssig;
  char text_location[256];


  //  strcpy(text_location, main_directory_location);
  //newstructure
  //  egm96_to360_not_norm.txt is put with the SPICE files path_to_spice. ASSUMPTION: the path of spice specified in the main inoput file in the sectioN #SPICE must be the same as the path of SPICE installation in the Makefile, with /data at the end. So if in the Makefile the SPICE directory is hello/hi/ then the path of spice in the main input file in section #SPICE must be hello/hi/data/
  strcpy(text_location,path_to_spice);
  strcat(text_location, "egm96_to360_not_norm.txt");
    //  strcpy(text_location, "input/egm96_to360_not_norm.txt");
  //newstructure
  // !!!!!!!!!! to read normalized coefficients then put ./code/egm96_to360.txt
  fp = fopen(text_location, "r");
  if (fp) {
    while ( (read = getline(&line, &len, fp)) != -1 ) {

      sscanf(line, "%i %i %e %e %e %e" , &l, &m, &C, &S, &Csig, &Ssig);
      GRAVITY->Clm[l][m] = (double)C; // l is the degree (CBV)
      GRAVITY->Slm[l][m] = (double)S; // m is the order (CBV)

    }
    
    free(line);
    
    fclose(fp);

  } else {

    printf("The file input/egm96_to360_not_norm.txt has not been opened. The program will stop.\n");
    MPI_Finalize();
    exit(0);

  }

  return 0;
    
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Name:           compute_gravity
//  Purpose:        Computes gravity acceleration using EGM96
//  Assumptions:    Limited to 360x360
//  References      Vallado
//
//  Change Log:
//      |   Developer   |       Date    |   SCR     |   Notes
//      | --------------|---------------|-----------|---------------------------------------------
//      | J. Getchius   | 05/09/2015    |   ---     | Initial Implementatio
//      | C. Bussy-Virat| 07/19/2015    |   ---     | Corrections of term2 (from Vallado's theory (Vallado3 p. 548))
//
/////////////////////////////////////////////////////////////////////////////////////////
int compute_gravity(    double      a_i2cg_INRTL[3],
                        double      r_i2cg_INRTL[3],
                        double      et,
                        GRAVITY_T   *Gravity,
                        int         degree,
			char earth_fixed_frame[100],
			double earth_flattening, double earth_radius,
			SPACECRAFT_T *SC)
			//                        int         order)
{

  // Declarations
  double r_ecef2cg_ECEF[3];
  double a_ecef2cg_ECEF[3];
  double T_J2000_to_ECEF[3][3];
  double T_ECEF_to_J2000[3][3];
  double rmag;
  double rmag2;
  double rmag3;
  double lat_gc; // phi
  double slat_gc;
  double long_gc;
  double dUdr     = 0.0;
  double dUdlat   = 0.0;
  double dUdlong  = 0.0;
  double rad_over_r;
  double Plm;
  double Plm_plus1;
  double term;
  double term2;
  double coeff;
  double r_xy;
  int l;
  int m;
    
  // Begin algorithm
/*    // Update Geodetic State */
/*   double geodetic[3]; */
/* 	eci2lla(r_i2cg_INRTL , et, geodetic ); */
/* 	double altitude, latitude, longitude; */
/* 	altitude = geodetic[2]; */
/* 	latitude = geodetic[0]; */
/* 	longitude = geodetic[1];  */


/* 	// update the planet fixed state		 */
/* 	  geodetic_to_geocentric(earth_flattening,             */
/* 				 altitude, */
/* 				 latitude, */
/* 				 longitude, */
/* 				 earth_radius,        */
/* 				 r_ecef2cg_ECEF) ; */


  // Get Earth Fixed state
  pxform_c("J2000", earth_fixed_frame, et, T_J2000_to_ECEF);
  m_x_v(r_ecef2cg_ECEF, T_J2000_to_ECEF, r_i2cg_INRTL);

  // Compute geocentric latitude and longitude
  v_mag( &rmag, r_ecef2cg_ECEF);
  lat_gc  = asin( r_ecef2cg_ECEF[2] / rmag );
  long_gc = atan2(r_ecef2cg_ECEF[1], r_ecef2cg_ECEF[0]);
    
  // Some intermediate
  rad_over_r  = Gravity->radius / rmag;
  slat_gc     = sin( lat_gc );
  r_xy        = sqrt( r_ecef2cg_ECEF[0]*r_ecef2cg_ECEF[0] + r_ecef2cg_ECEF[1]*r_ecef2cg_ECEF[1]);
  rmag2       = pow( rmag, 2);
  rmag3       = pow( rmag, 3);

  //   Compute Partial of potential function wrt range, lat, long
  //  printf("degree = %d\n", degree);

  for (l = 2; l <= degree; l++) {

    for ( m = 0; m <= l; m++) { // !!!!!!!!!!!!! replace 0 with l

      Plm         = pow(-1,m) * gsl_sf_legendre_Plm( l, m, slat_gc ); // in ~/gsl-1.16/specfunc/legendre_poly.c // pow(-1,m) has been added to agree with Vallado's theory (the C library uses a pow(-1,m) that the theory does not))

      if ((m+1) > l) {
            
  	Plm_plus1 = 0.0;
            
      } else {
                
  	Plm_plus1   = pow(-1,m+1) * gsl_sf_legendre_Plm( l, (m+1), slat_gc ); // !!!!!!!!!!  pow(-1,m+1) has been added to agree with Vallado's theory (the C library uses a pow(-1,m+1) that the theory does not))
            
      }


      coeff       = pow(rad_over_r, l);
      //      printf("%d %d %e %e\n", l,m,Gravity->Clm[l][m], Gravity->Slm[l][m]);
      term        = (Gravity->Clm[l][m] * cos( m*long_gc ) + Gravity->Slm[l][m] * sin( m*long_gc ));
      term2       = (-Gravity->Clm[l][m] * sin( m*long_gc ) + Gravity->Slm[l][m] * cos( m*long_gc )); // Modif by CBV 07-19-2015 from Vallado3 p. 548
            
      dUdr    += coeff * (l + 1.0) * Plm * term;
      dUdlat  += coeff * ( Plm_plus1  - m * tan( lat_gc) * Plm ) * term;
      dUdlong += coeff * m * Plm * term2;
            
    }
  }

  //  exit(0);

  dUdr    = -dUdr     * Gravity->mu / rmag2;
  dUdlat  = dUdlat    * Gravity->mu / rmag;
  dUdlong = dUdlong   * Gravity->mu / rmag;
    
  //  // Compute the Earth fixed accels
  term                = (1/rmag) * dUdr - (r_ecef2cg_ECEF[2] / (rmag*rmag * r_xy)) * dUdlat;
  term2               = (1/(r_xy * r_xy)) * dUdlong;
  a_ecef2cg_ECEF[0]   = term * r_ecef2cg_ECEF[0] - term2 * r_ecef2cg_ECEF[1] - Gravity->mu * r_ecef2cg_ECEF[0] / (rmag3);
  a_ecef2cg_ECEF[1]   = term * r_ecef2cg_ECEF[1] + term2 * r_ecef2cg_ECEF[0] - Gravity->mu * r_ecef2cg_ECEF[1] / (rmag3);
  a_ecef2cg_ECEF[2]   = (1/rmag) * dUdr * r_ecef2cg_ECEF[2] + r_xy / ( rmag2) * dUdlat - Gravity->mu  * r_ecef2cg_ECEF[2] / (rmag3);



  // Rotate to Inertial accels
  pxform_c(earth_fixed_frame, "J2000", et, T_ECEF_to_J2000);
  m_x_v( a_i2cg_INRTL, T_ECEF_to_J2000, a_ecef2cg_ECEF);
  v_copy(SC->a_i2cg_INRTL_gravity, a_i2cg_INRTL);
   /* v_print(a_ecef2cg_ECEF, "a_ecef2cg_ECEF"); */


  /* // J2 only  */
  /* double R0 = 6378.1370; */
  /* double J2 = 1082.65e-6; */

  /* double j2pert = J2*(3.0/2.0)*(R0/rmag)*(R0/rmag); */
  /* double j2sub = 5.0*(r_i2cg_INRTL[2]*r_i2cg_INRTL[2]/R0/R0); */

  /* a_i2cg_INRTL[0]   = -Gravity->mu*r_i2cg_INRTL[0]/(rmag*rmag*rmag) * (1.0 - j2pert*(j2sub-1)); */
  /* a_i2cg_INRTL[1]   = -Gravity->mu*r_i2cg_INRTL[1]/(rmag*rmag*rmag) * (1.0 - j2pert*(j2sub-1)); */
  /* a_i2cg_INRTL[2]   = -Gravity->mu*r_i2cg_INRTL[2]/(rmag*rmag*rmag) * (1.0 - j2pert*(j2sub-3)); */
    

  return 0;
}



/////////////////////////////////////////////////////////////////////////////////////////
//
//  Name:           compute_power
//  Purpose:        Computes power from the solar arrays
//  Assumptions:    None
//  References      None
//
//  Change Log:
//      |   Developer   |       Date    |   SCR     |   Notes
//      | --------------|---------------|-----------|---------------------------------------------------------
//      | C. Bussy-Virat| 08/02/2015    |   ---     | Initial implementation
//
/////////////////////////////////////////////////////////////////////////////////////////

int compute_power(INTEGRATOR_T    *INTEGRATOR,
		  double          r_i2cg_INRTL[3],
		  double          v_i2cg_INRTL[3],
		  double          *et,
		  PARAMS_T        *PARAMS,
		  double          et_initial_epoch,
		  double          et_sc_initial,
		  int             index_in_attitude_interpolated)	
		
{

  // Declarations

  //  int index_in_attitude_interpolated;
  double cos_sun_elevation;
  //  double angle_normal_to_sun;
  double z_body[3];
  double solar_flux = 1358.;//1367.0; // solar flux is in W/m^2 (SI), which is in kg/s^3. So the fact that we express distances in km and not in m does not change the value of the solar_flux here.
  double x[6];
  double lt;
  double r_cg2sun_J2000[3];
  double r_cg2sun_J2000_normalized[3];
  double r_cg2sun_LVLH_normalized[3];
  double r_cg2sun_SC_normalized[3];
  double r_earth2sun_J2000[3];
  double T_inrtl_2_lvlh[3][3];
  double T_sc_to_lvlh[3][3];
  double T_lvlh_to_sc[3][3];
  double cos_phi; // phi is the angle between the satellite-Sun direction and the normal to the surface
  int sss;
  char shadow[256];
  double v_angle[3];
  int order_rotation[3];

  shadow_light( shadow, r_i2cg_INRTL, et[0], PARAMS);
  if (strcmp(INTEGRATOR->attitude.attitude_profile, "ensemble_angular_velocity") == 0){  // if we run ensembles on the attitude. This block was moved out of the condition "if ( strcmp(shadow, "light") == 0)" to compute the (pitch, roll, yaw) angles even in the shadow, so that we can write these in an output file to compare with STK. If you don't want to compare with STK, then you can put it in the the condition "if ( strcmp(shadow, "light") == 0)"
    v_angle[0] = INTEGRATOR->attitude.pitch_angular_velocity_ensemble * ( et[0] - et_sc_initial ) + INTEGRATOR->attitude.pitch_ini_ensemble;
    v_angle[1] = INTEGRATOR->attitude.roll_angular_velocity_ensemble * ( et[0] - et_sc_initial ) + INTEGRATOR->attitude.roll_ini_ensemble;
    v_angle[2] = INTEGRATOR->attitude.yaw_angular_velocity_ensemble * ( et[0] - et_sc_initial ) + INTEGRATOR->attitude.yaw_ini_ensemble;
    order_rotation[0] = 1; // !!!!!!!! we might want to change that in the future
    order_rotation[1] = 2;
    order_rotation[2] = 3;
    INTEGRATOR->attitude.pitch_current = v_angle[0];
    INTEGRATOR->attitude.roll_current = v_angle[1];
    INTEGRATOR->attitude.yaw_current = v_angle[2];
        INTEGRATOR->attitude.order_pitch_current = order_rotation[0];
    INTEGRATOR->attitude.order_roll_current = order_rotation[1];
    INTEGRATOR->attitude.order_yaw_current = order_rotation[2];


  }

	  if (strcmp(INTEGRATOR->attitude.attitude_profile, "ensemble_initial_attitude") == 0) {
	      v_angle[0] =  INTEGRATOR->attitude.pitch_for_attitude_ensemble  +  INTEGRATOR->attitude.pitch_angular_velocity_constant * ( et[0] - et_sc_initial );
	      v_angle[1] =  INTEGRATOR->attitude.roll_for_attitude_ensemble  +  INTEGRATOR->attitude.roll_angular_velocity_constant * ( et[0] - et_sc_initial );
	      v_angle[2] =  INTEGRATOR->attitude.yaw_for_attitude_ensemble  +  INTEGRATOR->attitude.yaw_angular_velocity_constant * ( et[0] - et_sc_initial );

	      order_rotation[0]  = 1; order_rotation[1]  = 2; order_rotation[2]  = 3;
	   
	      INTEGRATOR->attitude.pitch_current = v_angle[0];
	      INTEGRATOR->attitude.roll_current = v_angle[1];
	      INTEGRATOR->attitude.yaw_current = v_angle[2];
	          INTEGRATOR->attitude.order_pitch_current = order_rotation[0];
    INTEGRATOR->attitude.order_roll_current = order_rotation[1];
    INTEGRATOR->attitude.order_yaw_current = order_rotation[2];


	      
	  }

  //  char cu_time[256];
	  if ( (strcmp(INTEGRATOR->attitude.attitude_profile, "ensemble_angular_velocity") != 0) && (strcmp(INTEGRATOR->attitude.attitude_profile, "ensemble_initial_attitude") != 0) && (strcmp(INTEGRATOR->attitude.attitude_profile, "sun_pointed") != 0) ){ // otherwise (atittude is nadir, sun_pointed or manual (from an input file))  
    //    et2utc_c(et[0], "ISOC" ,0 ,255 , cu_time);
    //    index_in_attitude_interpolated = floor( ( et[0] - et_initial_epoch ) / ( INTEGRATOR->dt / 2.0) ) ; 
	    if (INTEGRATOR->file_is_quaternion == 0){
    v_angle[0] = INTEGRATOR->attitude.pitch[index_in_attitude_interpolated];
    v_angle[1] = INTEGRATOR->attitude.roll[index_in_attitude_interpolated];
    v_angle[2] = INTEGRATOR->attitude.yaw[index_in_attitude_interpolated];
    order_rotation[0] = INTEGRATOR->attitude.order_pitch[index_in_attitude_interpolated];
    order_rotation[1] = INTEGRATOR->attitude.order_roll[index_in_attitude_interpolated];
    order_rotation[2] = INTEGRATOR->attitude.order_yaw[index_in_attitude_interpolated];
    INTEGRATOR->attitude.pitch_current = v_angle[0];
    INTEGRATOR->attitude.roll_current = v_angle[1];
    INTEGRATOR->attitude.yaw_current = v_angle[2];
        INTEGRATOR->attitude.order_pitch_current = order_rotation[0];
    INTEGRATOR->attitude.order_roll_current = order_rotation[1];
    INTEGRATOR->attitude.order_yaw_current = order_rotation[2];


	    }
	    else{
	      q_copy( INTEGRATOR->attitude.quaternion_current, INTEGRATOR->attitude.quaternion[index_in_attitude_interpolated]);
	    }


  }



  if ( strcmp(shadow, "light") == 0) {

    spkez_c(10, et[0], "J2000", "NONE", 399, x, &lt); //   Return the state (position and velocity) of a target body relative to an observing body, optionally corrected for light time (planetary aberration) and stellar aberration.

    r_earth2sun_J2000[0] = x[0];
    r_earth2sun_J2000[1] = x[1];
    r_earth2sun_J2000[2] = x[2];
     
    v_sub(r_cg2sun_J2000, r_earth2sun_J2000, r_i2cg_INRTL);
    v_norm(r_cg2sun_J2000_normalized, r_cg2sun_J2000);


      /* r_cg2sun_J2000_normalized inertial to LVLH */
      compute_T_inrtl_2_lvlh(T_inrtl_2_lvlh, r_i2cg_INRTL, v_i2cg_INRTL);
      m_x_v(r_cg2sun_LVLH_normalized, T_inrtl_2_lvlh, r_cg2sun_J2000_normalized);
      /* r_cg2sun_J2000_normalized LVLH to body */
      compute_T_sc_to_lvlh( T_sc_to_lvlh, v_angle, order_rotation, INTEGRATOR->attitude.attitude_profile, et,  r_i2cg_INRTL, v_i2cg_INRTL, INTEGRATOR->file_is_quaternion, INTEGRATOR->attitude.quaternion_current);
      m_trans(T_lvlh_to_sc, T_sc_to_lvlh);
      m_x_v(r_cg2sun_SC_normalized, T_lvlh_to_sc, r_cg2sun_LVLH_normalized ); 


    z_body[0] = 0; z_body[1] = 0; z_body[2] = 1;
    v_dot(&cos_sun_elevation, r_cg2sun_SC_normalized, z_body);
    INTEGRATOR->sun_elevation =  M_PI/2 - acos(cos_sun_elevation)  ;
    for (sss = 0; sss < INTEGRATOR->nb_surfaces; sss++){
      INTEGRATOR->surface[sss].power_per_surface = 0.0;
      if (INTEGRATOR->surface[sss].area_solar_panel > 0){
      v_dot(&cos_phi, r_cg2sun_SC_normalized, INTEGRATOR->surface[sss].normal);
      // // BLOCK BELOW USELESS, SO ERASE IT (OR COMMENT IT)

      /* angle_normal_to_sun = acos(cos_phi) * 180 / M_PI; */
      
      /* //      printf("angle = %f\n", angle_normal_to_sun); */
      // // END OF BLOCK BELOW USELESS, SO ERASE IT (OR COMMENT IT)
      //      printf("%d: %f\n", sss,cos_phi);
      if (cos_phi > 0){
	//      printf("surface %d | cos = %f\n", sss, cos_phi);
	INTEGRATOR->surface[sss].power_per_surface = INTEGRATOR->surface[sss].area_solar_panel * solar_flux * cos_phi * INTEGRATOR->solar_cell_efficiency * 1e6; // "1e6" is here to convert the area of the solar panel from km^2 to m^2
	//      printf("power for surface %d: %f\n", sss, INTEGRATOR->surface[sss].power_per_surface);
      }
      else{
	INTEGRATOR->surface[sss].power_per_surface = 0.0;
      }
    }
      else{
	INTEGRATOR->surface[sss].power_per_surface = 0.0;
      }
    }

  }
  else{
    INTEGRATOR->sun_elevation = -999*DEG2RAD;
    for (sss = 0; sss < INTEGRATOR->nb_surfaces; sss++){
      INTEGRATOR->surface[sss].power_per_surface = 0.0;   
    } 
  }
  //  exit(0);
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Name:           compute_solar_pressure
//  Purpose:        Computes solar pressure acceleration
//  Assumptions:    VALLADO OR STK THEORY CAN BE USED, CHOOSE THE ONE YOU WANT (SEE COMMENTS IN THE CODE)
//  References      Vallado version 3 (section 8.6.4) AND STK (http://www.agi.com/resources/help/online/stk/10.1/index.html?page=source%2Fhpop%2Fhpop-05.htm)
//
//  Change Log:
//      |   Developer   |       Date    |   SCR     |   Notes
//      | --------------|---------------|-----------|---------------------------------------------------------
//      | C. Bussy-Virat| 08/02/2015    |   ---     | Initial implementation
//
/////////////////////////////////////////////////////////////////////////////////////////

int compute_solar_pressure(double          a_solar_pressure_INRTL[3],
			   double          r_i2cg_INRTL[3],
			   double          v_i2cg_INRTL[3],
			   double          et,
			   PARAMS_T        *PARAMS,
			   INTEGRATOR_T    *INTEGRATOR,
			   double          et_initial_epoch,
			   double          et_sc_initial,
			   int             index_in_attitude_interpolated)
		
{


  // Declarations
	  double k = 1; // in STK 'fraction of the solar disk visible at satellite location', set to 1 here
	  double solar_luminosity = 3.823e26; // in Watts

    double dist_sat_to_sun;
    //  double solar_flux = 1358.0; // solar flux is in W/m^2 (SI), which is in kg/s^3. So the fact that we express distances in km and not in m does not change the value of the solar_flux here.
  double a_solar_pressure_in_body[3]; // solar pressure acceleration in the SC reference system
  double x[6];
  double lt;
  double r_cg2sun_J2000[3];
  double r_cg2sun_J2000_normalized[3];
  double r_cg2sun_LVLH_normalized[3];
  double r_cg2sun_SC_normalized[3];
  double r_earth2sun_J2000[3];
  double T_inrtl_2_lvlh[3][3];
  double T_lvlh_2_inrtl[3][3];
  double T_sc_to_lvlh[3][3];
  double T_lvlh_to_sc[3][3];
  double cos_phi; // phi is the angle between the satellite-Sun direction and the normal to the surface
  double light_speed = 299792.458; // spped of light in km/s
  double a_solar_pressure_in_LVLH[3];
  //  double term1, term2;
  int sss;
  char shadow[256];
  //  int index_in_attitude_interpolated;
  double v_angle[3];
  int order_rotation[3];

  shadow_light( shadow, r_i2cg_INRTL, et, PARAMS);
  if (strcmp(INTEGRATOR->attitude.attitude_profile, "ensemble_angular_velocity") == 0){
    v_angle[0] = INTEGRATOR->attitude.pitch_angular_velocity_ensemble * ( et - et_sc_initial ) + INTEGRATOR->attitude.pitch_ini_ensemble;
    v_angle[1] = INTEGRATOR->attitude.roll_angular_velocity_ensemble * ( et - et_sc_initial ) + INTEGRATOR->attitude.roll_ini_ensemble;
    v_angle[2] = INTEGRATOR->attitude.yaw_angular_velocity_ensemble * ( et - et_sc_initial ) + INTEGRATOR->attitude.yaw_ini_ensemble;
    order_rotation[0] = 1; // !!!!!!!! we might want to change that in the future                                                                     
    order_rotation[1] = 2;
    order_rotation[2] = 3;
    INTEGRATOR->attitude.pitch_current = v_angle[0];
    INTEGRATOR->attitude.roll_current = v_angle[1];
    INTEGRATOR->attitude.yaw_current = v_angle[2];

  }

	  if (strcmp(INTEGRATOR->attitude.attitude_profile, "ensemble_initial_attitude") == 0) {
	      v_angle[0] =  INTEGRATOR->attitude.pitch_for_attitude_ensemble  +  INTEGRATOR->attitude.pitch_angular_velocity_constant * ( et - et_sc_initial );
	      v_angle[1] =  INTEGRATOR->attitude.roll_for_attitude_ensemble  +  INTEGRATOR->attitude.roll_angular_velocity_constant * ( et - et_sc_initial );
	      v_angle[2] =  INTEGRATOR->attitude.yaw_for_attitude_ensemble  +  INTEGRATOR->attitude.yaw_angular_velocity_constant * ( et - et_sc_initial );

	      order_rotation[0]  = 1; order_rotation[1]  = 2; order_rotation[2]  = 3;
	   
	      INTEGRATOR->attitude.pitch_current = v_angle[0];
	      INTEGRATOR->attitude.roll_current = v_angle[1];
	      INTEGRATOR->attitude.yaw_current = v_angle[2];
	          INTEGRATOR->attitude.order_pitch_current = order_rotation[0];
    INTEGRATOR->attitude.order_roll_current = order_rotation[1];
    INTEGRATOR->attitude.order_yaw_current = order_rotation[2];


	      
	  }


	  if ( (strcmp(INTEGRATOR->attitude.attitude_profile, "ensemble_angular_velocity") != 0) && (strcmp(INTEGRATOR->attitude.attitude_profile, "ensemble_initial_attitude") != 0) && (strcmp(INTEGRATOR->attitude.attitude_profile, "sun_pointed") != 0) ){ // otherwise (atittude is nadir, sun_pointed or manual (from an input file))    
    //    index_in_attitude_interpolated = floor( ( et - et_sc_initial ) / ( INTEGRATOR->dt / 2.0) ) ; 
	    if (INTEGRATOR->file_is_quaternion == 0){
    v_angle[0] = INTEGRATOR->attitude.pitch[index_in_attitude_interpolated];
    v_angle[1] = INTEGRATOR->attitude.roll[index_in_attitude_interpolated];
    v_angle[2] = INTEGRATOR->attitude.yaw[index_in_attitude_interpolated];
    order_rotation[0] = INTEGRATOR->attitude.order_pitch[index_in_attitude_interpolated];
    order_rotation[1] = INTEGRATOR->attitude.order_roll[index_in_attitude_interpolated];
    order_rotation[2] = INTEGRATOR->attitude.order_yaw[index_in_attitude_interpolated];
    INTEGRATOR->attitude.pitch_current = v_angle[0];
    INTEGRATOR->attitude.roll_current = v_angle[1];
    INTEGRATOR->attitude.yaw_current = v_angle[2];
        INTEGRATOR->attitude.order_pitch_current = order_rotation[0];
    INTEGRATOR->attitude.order_roll_current = order_rotation[1];
    INTEGRATOR->attitude.order_yaw_current = order_rotation[2];


	    }
	    else{
	      q_copy( INTEGRATOR->attitude.quaternion_current, INTEGRATOR->attitude.quaternion[index_in_attitude_interpolated]);
	    }
  }

  if ( strcmp(shadow, "light") == 0) {

    spkez_c(10, et, "J2000", "NONE", 399, x, &lt); //   Return the state (position and velocity) of a target body relative to an observing body, optionally corrected for light time (planetary aberration) and stellar aberration.

    r_earth2sun_J2000[0] = x[0];
    r_earth2sun_J2000[1] = x[1];
    r_earth2sun_J2000[2] = x[2];
     
    v_sub(r_cg2sun_J2000, r_earth2sun_J2000, r_i2cg_INRTL);
    v_mag( &dist_sat_to_sun, r_cg2sun_J2000 );
    v_norm(r_cg2sun_J2000_normalized, r_cg2sun_J2000);



    /* r_cg2sun_J2000_normalized inertial to LVLH */
    compute_T_inrtl_2_lvlh(T_inrtl_2_lvlh, r_i2cg_INRTL, v_i2cg_INRTL);
    m_x_v(r_cg2sun_LVLH_normalized, T_inrtl_2_lvlh, r_cg2sun_J2000_normalized);

    /* r_cg2sun_J2000_normalized LVLH to body */
    compute_T_sc_to_lvlh( T_sc_to_lvlh, v_angle, order_rotation, INTEGRATOR->attitude.attitude_profile, &et,  r_i2cg_INRTL, v_i2cg_INRTL, INTEGRATOR->file_is_quaternion, INTEGRATOR->attitude.quaternion_current);
    //  compute_T_sc_to_lvlh(T_sc_to_lvlh, INTEGRATOR->attitude.lvlh_alongtrack_in_body_cartesian, INTEGRATOR->attitude.lvlh_crosstrack_in_body_cartesian, &et, r_i2cg_INRTL, v_i2cg_INRTL, INTEGRATOR); 
    m_trans(T_lvlh_to_sc, T_sc_to_lvlh);
    m_x_v(r_cg2sun_SC_normalized, T_lvlh_to_sc, r_cg2sun_LVLH_normalized ); 
    


    for (sss = 0; sss < INTEGRATOR->nb_surfaces; sss++){
      if (sss == 0){
	
	v_dot(&cos_phi, r_cg2sun_SC_normalized, INTEGRATOR->surface[0].normal);
	if (cos_phi > 0){

	  // !!!!!!!!!!! THIS BLOCK IS USING THE EQUATION FROM STK (http://www.agi.com/resources/help/online/stk/10.1/index.html?page=source%2Fhpop%2Fhpop-05.htm). UNCOMMENT THE BLOCK BELOW THAT USES VALLADO AND COMMENT THIS STK BLOCK IF YOU WANT TO USE VALLADO'S EQUATIONS. ALSO NEED TO CHANGE initialize_constellation.c AND load_options.c TO READ THE SPECULAR AND DIFFUSE REFLECIVITIES IF YOU WANT TO USE VALLADO'S EQUATIONS (SEE COMMENTS IN THESE CODES)
	  a_solar_pressure_in_body[0] = - k * INTEGRATOR->surface[0].solar_radiation_coefficient * INTEGRATOR->surface[0].area * cos_phi / INTEGRATOR->mass * solar_luminosity / (4 * M_PI * light_speed * dist_sat_to_sun * dist_sat_to_sun * 1000000.) * r_cg2sun_SC_normalized[0];
	  a_solar_pressure_in_body[1] = - k * INTEGRATOR->surface[0].solar_radiation_coefficient * INTEGRATOR->surface[0].area * cos_phi / INTEGRATOR->mass * solar_luminosity / (4 * M_PI * light_speed * dist_sat_to_sun * dist_sat_to_sun * 1000000.) * r_cg2sun_SC_normalized[1];
	  a_solar_pressure_in_body[2] = - k * INTEGRATOR->surface[0].solar_radiation_coefficient * INTEGRATOR->surface[0].area * cos_phi / INTEGRATOR->mass * solar_luminosity / (4 * M_PI * light_speed * dist_sat_to_sun * dist_sat_to_sun * 1000000.) * r_cg2sun_SC_normalized[2];

	  // !!!!!!!!!!! END OF THIS BLOCK IS USING THE EQUATION FROM STK (http://www.agi.com/resources/help/online/stk/10.1/index.html?page=source%2Fhpop%2Fhpop-05.htm).  UNCOMMENT THE BLOCK BELOW THAT USES VALLADO AND COMMENT THIS STK BLOCK IF YOU WANT TO USE VALLADO'S EQUATIONS. ALSO NEED TO CHANGE initialize_constellation.c AND load_options.c TO READ THE SPECULAR AND DIFFUSE REFLECIVITIES IF YOU WANT TO USE VALLADO'S EQUATIONS (SEE COMMENTS IN THESE CODES)

	    // !!!!!!!!!! THIS BLOCK USES VALLADO'S EQUATIONS. COMMENT IT AND UNCOMMENT THE BLOCK ABOVE THAT USES STK IF YOU WANT TO USE STK'S EQUATIONS. ALSO NEED TO CHANGE initialize_constellation.c AND load_options.c TO READ THE SPECULAR AND DIFFUSE REFLECIVITIES IF YOU WANT TO USE VALLADO'S EQUATIONS (SEE COMMENTS IN THESE CODES)
	  /* term1 = INTEGRATOR->surface[0].diffuse_reflectivity / 3.0 + INTEGRATOR->surface[0].specular_reflectivity * cos_phi; */
	  /* term2 = 1 - INTEGRATOR->surface[0].specular_reflectivity; */

	  /* a_solar_pressure_in_body[0] = - solar_flux * INTEGRATOR->surface[0].area * cos_phi * ( 2 * term1 * INTEGRATOR->surface[0].normal[0] + term2 * r_cg2sun_SC_normalized[0] ) / (light_speed * INTEGRATOR->mass); */
	  /* a_solar_pressure_in_body[1] = - solar_flux * INTEGRATOR->surface[0].area * cos_phi * ( 2 * term1 * INTEGRATOR->surface[0].normal[1] + term2 * r_cg2sun_SC_normalized[1] ) / (light_speed * INTEGRATOR->mass); */
	  /* a_solar_pressure_in_body[2] = - solar_flux * INTEGRATOR->surface[0].area * cos_phi * ( 2 * term1 * INTEGRATOR->surface[0].normal[2] + term2 * r_cg2sun_SC_normalized[2] ) / (light_speed * INTEGRATOR->mass); */
	    // !!!!!!!!!! END OF THIS BLOCK USES VALLADO'S EQUATIONS. COMMENT IT AND UNCOMMENT THE BLOCK ABOVE THAT USES STK IF YOU WANT TO USE STK'S EQUATIONS. ALSO NEED TO CHANGE initialize_constellation.c AND load_options.c TO READ THE SPECULAR AND DIFFUSE REFLECIVITIES IF YOU WANT TO USE VALLADO'S EQUATIONS (SEE COMMENTS IN THESE CODES)
	}
	else {
	  a_solar_pressure_in_body[0] = 0.0;
	  a_solar_pressure_in_body[1] = 0.0;
	  a_solar_pressure_in_body[2] = 0.0;
	}
      }
      else{
      
	v_dot(&cos_phi, r_cg2sun_SC_normalized, INTEGRATOR->surface[sss].normal);
      
	if (cos_phi > 0){
	  // !!!!!!!!!!! THIS BLOCK IS USING THE EQUATION FROM STK (http://www.agi.com/resources/help/online/stk/10.1/index.html?page=source%2Fhpop%2Fhpop-05.htm). UNCOMMENT THE BLOCK BELOW THAT USES VALLADO AND COMMENT THIS STK BLOCK IF YOU WANT TO USE VALLADO'S EQUATIONS. ALSO NEED TO CHANGE initialize_constellation.c AND load_options.c TO READ THE SPECULAR AND DIFFUSE REFLECIVITIES IF YOU WANT TO USE VALLADO'S EQUATIONS (SEE COMMENTS IN THESE CODES)
	  a_solar_pressure_in_body[0] = a_solar_pressure_in_body[0]  - k * INTEGRATOR->surface[sss].solar_radiation_coefficient * INTEGRATOR->surface[sss].area * cos_phi / INTEGRATOR->mass * solar_luminosity / (4 * M_PI * light_speed * dist_sat_to_sun * dist_sat_to_sun * 1000000.) * r_cg2sun_SC_normalized[0];
	  a_solar_pressure_in_body[1] = a_solar_pressure_in_body[1] - k * INTEGRATOR->surface[sss].solar_radiation_coefficient * INTEGRATOR->surface[sss].area * cos_phi / INTEGRATOR->mass * solar_luminosity / (4 * M_PI * light_speed * dist_sat_to_sun * dist_sat_to_sun * 1000000.) * r_cg2sun_SC_normalized[1];
	  a_solar_pressure_in_body[2] = a_solar_pressure_in_body[2] - k * INTEGRATOR->surface[sss].solar_radiation_coefficient * INTEGRATOR->surface[sss].area * cos_phi / INTEGRATOR->mass * solar_luminosity / (4 * M_PI * light_speed * dist_sat_to_sun * dist_sat_to_sun * 1000000.) * r_cg2sun_SC_normalized[2];
	  // !!!!!!!!!!! END OF THIS BLOCK IS USING THE EQUATION FROM STK (http://www.agi.com/resources/help/online/stk/10.1/index.html?page=source%2Fhpop%2Fhpop-05.htm).  UNCOMMENT THE BLOCK BELOW THAT USES VALLADO AND COMMENT THIS STK BLOCK IF YOU WANT TO USE VALLADO'S EQUATIONS. ALSO NEED TO CHANGE initialize_constellation.c AND load_options.c TO READ THE SPECULAR AND DIFFUSE REFLECIVITIES IF YOU WANT TO USE VALLADO'S EQUATIONS (SEE COMMENTS IN THESE CODES)

	    // !!!!!!!!!! THIS BLOCK USES VALLADO'S EQUATIONS. COMMENT IT AND UNCOMMENT THE BLOCK ABOVE THAT USES STK IF YOU WANT TO USE STK'S EQUATIONS.ALSO NEED TO CHANGE initialize_constellation.c AND load_options.c TO READ THE SPECULAR AND DIFFUSE REFLECIVITIES IF YOU WANT TO USE VALLADO'S EQUATIONS (SEE COMMENTS IN THESE CODES)
	  /* term1 = INTEGRATOR->surface[sss].diffuse_reflectivity / 3.0 + INTEGRATOR->surface[sss].specular_reflectivity * cos_phi; */
	  /* term2 = 1 - INTEGRATOR->surface[sss].specular_reflectivity; */

	  /* a_solar_pressure_in_body[0] = a_solar_pressure_in_body[0] - solar_flux * INTEGRATOR->surface[sss].area * cos_phi * ( 2 * term1 * INTEGRATOR->surface[sss].normal[0] + term2 * r_cg2sun_SC_normalized[0] ) / (light_speed * INTEGRATOR->mass); */
	  /* a_solar_pressure_in_body[1] = a_solar_pressure_in_body[1] - solar_flux * INTEGRATOR->surface[sss].area * cos_phi * ( 2 * term1 * INTEGRATOR->surface[sss].normal[1] + term2 * r_cg2sun_SC_normalized[1] ) / (light_speed * INTEGRATOR->mass); */
	  /* a_solar_pressure_in_body[2] = a_solar_pressure_in_body[2] - solar_flux * INTEGRATOR->surface[sss].area * cos_phi * ( 2 * term1 * INTEGRATOR->surface[sss].normal[2] + term2 * r_cg2sun_SC_normalized[2] ) / (light_speed * INTEGRATOR->mass); */
	    // !!!!!!!!!! END OF THIS BLOCK USES VALLADO'S EQUATIONS. COMMENT IT AND UNCOMMENT THE BLOCK ABOVE THAT USES STK IF YOU WANT TO USE STK'S EQUATIONS. ALSO NEED TO CHANGE initialize_constellation.c AND load_options.c TO READ THE SPECULAR AND DIFFUSE REFLECIVITIES IF YOU WANT TO USE VALLADO'S EQUATIONS (SEE COMMENTS IN THESE CODES)
	}
   
      }
   
    }

    m_x_v(a_solar_pressure_in_LVLH, T_sc_to_lvlh, a_solar_pressure_in_body);
    m_trans(T_lvlh_2_inrtl, T_inrtl_2_lvlh);
    m_x_v(a_solar_pressure_INRTL, T_lvlh_2_inrtl, a_solar_pressure_in_LVLH);  



  }
  else{
    a_solar_pressure_INRTL[0] = 0.0;
    a_solar_pressure_INRTL[1] = 0.0;
    a_solar_pressure_INRTL[2] = 0.0;
  }

  return 0;

}



/////////////////////////////////////////////////////////////////////////////////////////
//
//  Name:           compute_drag
//  Purpose:        Computes acceleration acceleration using NRLMSIS 2000
//  Assumptions:    None.
//  References      NRL / GSFC
//
//  Change Log:
//      |   Developer   |       Date    |   SCR     |   Notes
//      | --------------|---------------|-----------|---------------------------------------------------------
//      | J. Getchius   | 05/20/2015    |   ---     | Initial Implementation
//      | C. Bussy-Virat| 07/19/2015    |   ---     | Corrections: add Cd as a factor in the drag acceleration
//      | C. Bussy-Virat| 07/28/2015    |   ---     | Add attiude dependence and multi surfaces + the third component of LVLH now points away from the Earth (and not towards the Earth anymore)
//
/////////////////////////////////////////////////////////////////////////////////////////

int compute_drag(       double          adrag_i2cg_INRTL[3],
                        double          r_i2cg_INRTL[3],
                        double          v_i2cg_INRTL[3],
                        double          et,
                        PARAMS_T        *PARAMS,
                        INTEGRATOR_T    *INTEGRATOR,
			double          et_initial_epoch,
			double          et_sc_initial,
			double          *density,
			int             index_in_attitude_interpolated, 
			int             index_in_driver_interpolated,
			CONSTELLATION_T *CONSTELLATION,
			OPTIONS_T       *OPTIONS,
			int iProc,
			int iDebugLevel,
			SPACECRAFT_T *SC)

{


  //  printf("%d %d\n",index_in_attitude_interpolated, index_in_driver_interpolated);

  double total_cross_area = 0;

  if (iDebugLevel >= 2){
    printf("--- (compute_drag) Just got in compute_drag ... (iProc %d | iii = %d, eee = %d)\n", iProc, INTEGRATOR->sc_main_nb, INTEGRATOR->sc_ensemble_nb);
  }

  // if set to 0 then don't write in the given files. if set to 1 the write in given files


  // Declarations

  int aaa_a;
  double nb_index_in_81_days;
  int hhh;
  //  int start_ensemble = 1;
  int eee;

  int ppp;
  double latitude_gitm,  longitude_gitm,  altitude_gitm;
  double r_ecef2cg_ECEF_gitm[3];
  double T_J2000_to_ECEF_gitm[3][3];
  double ballistic_coefficient = 0;
  double v_angle[3];
  int order_rotation[3];
  //      int index_in_attitude_interpolated;
  struct nrlmsise_output output;
  struct nrlmsise_input input;
  char   timestamp[36];
  char   doy_s[256];
  char   year_s[256];
  char   hour_s[256];
  char   min_s[256];
  char   sec_s[256];
  double hour = 0.0;
  double min = 0.0;
  double sec = 0.0;
  double r_ecef2cg_ECEF[3];
  double T_J2000_to_ECEF[3][3];
  double normal_in_lvlh[3];
  double normal_in_inertial[3];
  double normal_in_ntw[3];
  double v_i2cg_NTW_DOT_normal_to_the_surface_NTW;
  double altitude;
  double longitude;
  double latitude;
  double T_sc_to_lvlh[3][3];
  double T_inrtl_2_lvlh[3][3];
  double T_lvlh_2_inrtl[3][3];
  double T_inrtl_2_ntw[3][3];
  double T_ntw_2_inrtl[3][3];
  double v_i2cg_NTW[3];
  double a_i2cg_NTW[3];
  //  int ii;
  int sss;
  char timestamp_isoc[300];
  //   int index_in_driver_interpolated;

  //  double density;
  /////////////////// COMPUTE DENSITY ///////////////////
  if ( ( strcmp(INTEGRATOR->format_density_driver, "density_file") != 0 ) && ( strcmp(INTEGRATOR->format_density_driver, "gitm") != 0 ) ){ // the user chooses f107 and Ap for the density
    // Generate the time inputs
    
    et2utc_c(et, "D", 3, 35, timestamp);
    et2utc_c(et, "ISOC" ,6 ,255 , timestamp_isoc);



    // YEAR
    strcpy(year_s, "");
    strncat(year_s, &timestamp[0], 4);
    input.year = atoi( year_s );

    // DOY
    strcpy(doy_s, "");
    strncat(doy_s, &timestamp[5], 3);
    /* for (ii = 0; ii < 3; ii++ ) { */
    /*   doy_s[ii] = timestamp[ii+5]; */
    /* } */
    input.doy = atoi( doy_s );
    
    //  Hour
    strcpy(hour_s, "");
    strncat(hour_s, &timestamp[12], 2);
    /* for (ii = 0; ii < 2; ii++ ) { */
    /*   hour_s[ii] = timestamp[ii+12]; */
    /* } */
    hour = atof( hour_s );
    
    // Min
    strcpy(min_s, "");
    strncat(min_s, &timestamp[15], 2);
    /* for (ii = 0; ii < 2; ii++ ) { */
    
    /*   min_s[ii] = timestamp[ii+15]; */
    /* } */
    min = atof( min_s );

    // Sec
    strcpy(sec_s, "");
    strncat(sec_s, &timestamp[18], 6);
    /* for (ii = 0; ii < 6; ii++ ) { */
    
    /*   sec_s[ii] = timestamp[ii+18]; */
    
    /* } */
    sec = atof( sec_s );

    input.sec = hour * 3600.0 + min * 60.0 + sec;
    //    printf("%s: %d %d %f\n", timestamp, input.year, input.doy, input.sec);

/*   double geodetic[3]; */
/* 	double altitude, latitude, longitude; */
/* 	eci2lla(r_i2cg_INRTL , et, geodetic ); */
/* 	altitude = geodetic[2]; */
/* 	latitude = geodetic[0]; */
/* 	longitude = geodetic[1];  */

/* 	// update the planet fixed state		 */
/* 	  geodetic_to_geocentric(PARAMS->EARTH.flattening,             */
/* 				 altitude, */
/* 				 latitude, */
/* 				 longitude, */
/* 				 PARAMS->EARTH.radius,        */
/* 				 r_ecef2cg_ECEF) ; */
    // Geneterate the Geodetic inputs
    pxform_c("J2000", PARAMS->EARTH.earth_fixed_frame, et, T_J2000_to_ECEF);
    m_x_v(r_ecef2cg_ECEF, T_J2000_to_ECEF, r_i2cg_INRTL);
    geocentric_to_geodetic( r_ecef2cg_ECEF,
			    &PARAMS->EARTH.radius,
			    &PARAMS->EARTH.flattening,
			    &altitude,
			    &latitude,
			    &longitude);
    input.g_lat     = latitude * RAD2DEG;
    input.g_long    = longitude * RAD2DEG;
    input.alt       = altitude;
    input.lst       = input.sec/3600. + input.g_long/15.;
    //    printf("%f %f %f %f ||| ", input.g_lat ,input.g_long  ,input.alt , input.lst);
    // Solar / Geomagnetic Activity
    if ( strcmp(INTEGRATOR->format_density_driver, "static") == 0 ){ // if the user chooses a constant f107 and Ap for the density  
      if (iDebugLevel >= 3){
	printf("---- (compute_drag) Computing the static F10.7, F10.7A, and Ap to determine density for drag ... (iProc %d | iii = %d, eee = %d)\n", iProc, INTEGRATOR->sc_main_nb, INTEGRATOR->sc_ensemble_nb);
      }


      input.f107A = INTEGRATOR->f107A_static;
      input.f107  = INTEGRATOR->f107_static;
      //      printf("%s: %f %f", timestamp_isoc, input.f107,input.f107A);
      if (PARAMS->ATMOSPHERE.flags.switches[9] != -1){ // if daily ap
	
	input.ap    = INTEGRATOR->Ap_static;
	//	printf(" %f", INTEGRATOR->Ap_static);
      }
      else{ //if historical ap
	input.ap_a = malloc(7 * sizeof(double));
	for (ppp = 0; ppp < 7 ; ppp ++){
	  input.ap_a->a[ppp]    = INTEGRATOR->Ap_hist_static[ppp];
	}
	//	printf(" %f", input.ap_a->a[ppp]);
      }
      //      printf("\n");
      if (iDebugLevel >= 3){
	printf("---- (compute_drag) Done computing the static F10.7, F10.7A, and Ap to determine density for drag ... (iProc %d | iii = %d, eee = %d)\n", iProc, INTEGRATOR->sc_main_nb, INTEGRATOR->sc_ensemble_nb);
      }

    } // end of if the user chooses a constant f107 and Ap for the density  

    else{ // if the user chooses a time varying f107 and Ap for the density  
      //      index_in_driver_interpolated = floor( ( et - et_initial_epoch ) / ( INTEGRATOR->dt / 2.0 ) ) ; // "/ 2.0" because of the Runge Kunta orfer 4 method
      if (iDebugLevel >= 3){
	printf("---- (compute_drag) Computing F10.7, F10.7A, and Ap to determine density for drag ... (iProc %d | iii = %d, eee = %d)\n", iProc, INTEGRATOR->sc_main_nb, INTEGRATOR->sc_ensemble_nb);
      }

      if ( (OPTIONS->nb_ensembles_density > 0) && (OPTIONS->swpc_need_predictions) && ( et >= OPTIONS->swpc_et_first_prediction ) ) { 
	if (iDebugLevel >= 4){
	  printf("---- (compute_drag) Computing the ensembles on the predictions of F10.7, F10.7A, and Ap ...(iProc %d | iii = %d, eee = %d)\n", iProc, INTEGRATOR->sc_main_nb, INTEGRATOR->sc_ensemble_nb);
	}

	// if the user chose to run ensembles on the density using data from SWPC 
	// if future predictions of F10.7 and Ap (not only past observations) (past values (value before the current time at running) are perfectly known (result from observations, not predictions))
	// only overwrite predictions of ensembles (not observations). OPTIONS->et_interpo[aaa] corresponds to the first say of predictions
	//      printf("XXXXXXXXXXXXXX\nXXXXXXXXXXXXXXXXXXX\n");
	nb_index_in_81_days =  81. * 24 * 3600 / (OPTIONS->dt/2.) + 1 ;
	if (INTEGRATOR->sc_ensemble_nb == 1 + iProc * OPTIONS->nb_ensemble_min_per_proc){ // initialize array only once per iProc
	  // // Generate nb_ensembles_density normal random values
	  /* if (iProc == 0){ */
	  /* 	etprint(et, ""); */
	  /* 	printf("OPTIONS->sigma_ap[%d] at index %d: %f | %f\n",CONSTELLATION->aaa_sigma[INTEGRATOR->sc_main_nb], index_in_driver_interpolated, OPTIONS->sigma_ap[CONSTELLATION->aaa_sigma[INTEGRATOR->sc_main_nb]], OPTIONS->sigma_f107[CONSTELLATION->aaa_sigma[INTEGRATOR->sc_main_nb]]); */
	  /* 	} */
	  /* if (INTEGRATOR->sc_ensemble_nb == 1){ */
	  /* 	etprint(et, "time"); */
	  /* 	printf("%f %d\n", OPTIONS->sigma_ap[CONSTELLATION->aaa_sigma[INTEGRATOR->sc_main_nb]], INTEGRATOR->sc_main_nb); */
	  /* } */
	    

	  for ( eee = 0; eee < OPTIONS->nb_ensemble_min_per_proc; eee++){
	    CONSTELLATION->ensemble_array_per_iproc_f107_at_given_time[iProc][eee]   = randn( OPTIONS->f107[index_in_driver_interpolated], OPTIONS->sigma_f107[CONSTELLATION->aaa_sigma[INTEGRATOR->sc_main_nb]]);
	    CONSTELLATION->ensemble_array_per_iproc_ap_at_given_time[iProc][eee]   = randn(  OPTIONS->Ap[index_in_driver_interpolated], OPTIONS->sigma_ap[CONSTELLATION->aaa_sigma[INTEGRATOR->sc_main_nb]]);
	    if (CONSTELLATION->ensemble_array_per_iproc_ap_at_given_time[iProc][eee] < 0){ // just in case sigma_ap is too big...
	      CONSTELLATION->ensemble_array_per_iproc_ap_at_given_time[iProc][eee] =  0; // OPTIONS->Ap[index_in_driver_interpolated];
	    }
	    if (CONSTELLATION->ensemble_array_per_iproc_f107_at_given_time[iProc][eee] < 0){// just in case sigma_f107 is too big...   
	      CONSTELLATION->ensemble_array_per_iproc_f107_at_given_time[iProc][eee]  = 0; //OPTIONS->f107[index_in_driver_interpolated];
	    }
	    /* if (eee == 0){ */
	    /* etprint(OPTIONS->et_interpo[aaa], "time"); */
	    /* printf("Ap[%d]: %f | sigma_ap[%d]: %f \n",aaa, OPTIONS->Ap[aaa],CONSTELLATION->aaa_sigma[INTEGRATOR->sc_main_nb], OPTIONS->sigma_ap[CONSTELLATION->aaa_sigma[INTEGRATOR->sc_main_nb]]); */
	    /* } */
	  }

	  // // Order values in ascending order
	  sort_asc_order(CONSTELLATION->ensemble_array_per_iproc_f107_at_given_time_sorted[iProc],  CONSTELLATION->ensemble_array_per_iproc_f107_at_given_time[iProc], OPTIONS->nb_ensemble_min_per_proc);
	  sort_asc_order(CONSTELLATION->ensemble_array_per_iproc_ap_at_given_time_sorted[iProc],  CONSTELLATION->ensemble_array_per_iproc_ap_at_given_time[iProc], OPTIONS->nb_ensemble_min_per_proc);


	

	  // Initialization of swpc_et_first_prediction for the calculation of F10.7A for an ensemble
	  if ( et_sc_initial > OPTIONS->swpc_et_first_prediction){ // if the propagation starts in the future. Otherwise, sum_sigma_for_f107_average = 0 for all ensemble sc

	    if (  CONSTELLATION->sum_sigma_for_f107_average[INTEGRATOR->sc_main_nb][INTEGRATOR->sc_ensemble_nb] == 0 )  {// for the first time that variable et gets bigger than swpc_et_first_prediction 
	      // initialize sum_sigma_for_f107_average as the sum of all sigma on F10.7 from first prediction to inital epoch. There is no mathematical logic in here. The exact solution would be to sum all the deviations between f107_ensemble and f107_refce from first prediciton until intial epoch, and sum them up. This is KIND OF similar. The reason I don't do the correct approach is that I did not calculate f107_ensemble for times before initial epoch
	      for (aaa_a = 0; aaa_a < CONSTELLATION->aaa_sigma[INTEGRATOR->sc_main_nb]; aaa_a++){

		CONSTELLATION->sum_sigma_for_f107_average[INTEGRATOR->sc_main_nb][INTEGRATOR->sc_ensemble_nb] = CONSTELLATION->sum_sigma_for_f107_average[INTEGRATOR->sc_main_nb][INTEGRATOR->sc_ensemble_nb] + OPTIONS->sigma_f107[aaa_a];		    
		//		      ptd( OPTIONS->sigma_f107[aaa_a], "s");
	      }

	      //		      printf("eee: %d | index: %d | iProc: %d | sum: %f\n", INTEGRATOR->sc_ensemble_nb, index_in_driver_interpolated, iProc, CONSTELLATION->sum_sigma_for_f107_average[INTEGRATOR->sc_main_nb][INTEGRATOR->sc_ensemble_nb]);exit(0);
		   
	    } // end of for the first time that variable et gets bigger than swpc_et_first_prediction
	  }// end of if the propagation starts in the future  
	  // End of initialization of swpc_et_first_prediction for the calculation of F10.7A for an ensemble

	} // end of initialize array only once per iProc
	else if (INTEGRATOR->sc_ensemble_nb == 0){ // also need to calculate f107 and ap for reference sc (no perturbation so directly equal to the values in the prediction files)
	  INTEGRATOR->Ap[index_in_driver_interpolated]            = OPTIONS->Ap[index_in_driver_interpolated];           // magnetic index(daily)
	  if (OPTIONS->use_ap_hist == 1){
	    for (hhh = 0; hhh < 7; hhh++){
	      INTEGRATOR->Ap_hist[hhh][index_in_driver_interpolated]            = OPTIONS->Ap_hist[hhh][index_in_driver_interpolated];           // magnetic index(historical)
	    }
	  }

	  INTEGRATOR->f107[index_in_driver_interpolated]          = OPTIONS->f107[index_in_driver_interpolated];         // Daily average of F10.7 flux
	  /* print_test(); */
	  /* printf("INTEGRATOR->f107[%d]: %f\n", index_in_driver_interpolated, INTEGRATOR->f107[index_in_driver_interpolated]); */

	  INTEGRATOR->f107A[index_in_driver_interpolated]         = OPTIONS->f107A[index_in_driver_interpolated];        // 81 day average of F10.7 flux


	} // end of also need to alculate f107 and ap for reference sc (no perturbation so directly equal to the values in the prediction files)
	if (INTEGRATOR->sc_ensemble_nb != 0) { // don't overwrite previously written values of f107 and Ap for reference sc
	  INTEGRATOR->Ap[index_in_driver_interpolated]            = CONSTELLATION->ensemble_array_per_iproc_ap_at_given_time_sorted[iProc][INTEGRATOR->sc_ensemble_nb-1-iProc*OPTIONS->nb_ensemble_min_per_proc];
	  INTEGRATOR->f107[index_in_driver_interpolated]          = CONSTELLATION->ensemble_array_per_iproc_f107_at_given_time_sorted[iProc][INTEGRATOR->sc_ensemble_nb-1-iProc*OPTIONS->nb_ensemble_min_per_proc]; 
	
	  CONSTELLATION->sum_sigma_for_f107_average[INTEGRATOR->sc_main_nb][INTEGRATOR->sc_ensemble_nb] = CONSTELLATION->sum_sigma_for_f107_average[INTEGRATOR->sc_main_nb][INTEGRATOR->sc_ensemble_nb] +  ( INTEGRATOR->f107[index_in_driver_interpolated]  - OPTIONS->f107[index_in_driver_interpolated] );
	  INTEGRATOR->f107A[index_in_driver_interpolated]         = OPTIONS->f107A[index_in_driver_interpolated] + CONSTELLATION->sum_sigma_for_f107_average[INTEGRATOR->sc_main_nb][INTEGRATOR->sc_ensemble_nb] / nb_index_in_81_days; // derivation of F10.7A considering uncerainties in F10.7
	  //	    printf("eee: %d | index: %d | iProc: %d | sum: %f | 81: %f | opeion %f\n", INTEGRATOR->sc_ensemble_nb, index_in_driver_interpolated, iProc, CONSTELLATION->sum_sigma_for_f107_average[INTEGRATOR->sc_main_nb][INTEGRATOR->sc_ensemble_nb], nb_index_in_81_days, OPTIONS->dt);
	  //	    print_test();
	  /* if (iProc == 0){ */
	  /*   //	    if ((INTEGRATOR->sc_ensemble_nb == 1 + iProc * OPTIONS->nb_ensemble_min_per_proc + 1)){ */
	  /*   if (index_in_driver_interpolated == 4){ */
	  /*     printf("eee: %d | f107:  %f | index: %d | iProc: %d\n", INTEGRATOR->sc_ensemble_nb, INTEGRATOR->f107[index_in_driver_interpolated], index_in_driver_interpolated, iProc); */
	  /* } */
	  /* } */
	} // end of don't overwrite previously written values of f107 and Ap for reference sc
	if (iDebugLevel >= 4){
	  printf("---- (compute_drag) Done computing the ensembles on the predictions of F10.7, F10.7A, and Ap ...(iProc %d | iii = %d, eee = %d)\n", iProc, INTEGRATOR->sc_main_nb, INTEGRATOR->sc_ensemble_nb);
	}

      } // end of:
      // if the user chose to run ensembles on the density using data from SWPC AND
      // if future predictions of F10.7 and Ap (not only past observations) (past values (value before the current time at running) are perfectly known (result from observations, not predictions))
      // only overwrite predictions of ensembles (not observations). OPTIONS->et_interpo[aaa] corresponds to the first say of predictions
      else{ // if we don't run ensembles on F10.7/Ap or that we run ensemble on F10.7/Ap but that the current time is before the first prediction (so there is no uncertainty in F10.7/Ap because the time corresponds to an observation, not a prediction)
	    //	    printf("%d %d %d %d\n", index_in_driver_interpolated, iProc, INTEGRATOR->sc_ensemble_nb, INTEGRATOR->sc_main_nb);
	    //	    print_test();

	if ( (strcmp(OPTIONS->test_omniweb_or_external_file, "swpc_mod") == 0) && (OPTIONS->swpc_need_predictions) && ( et >= OPTIONS->swpc_et_first_prediction ) ){ //if the option "swpc_mod" (so nominal f107 and Ap minus a certain value), that predictions of f107 and ap are used, and that the current time is in the future
	  nb_index_in_81_days =  81. * 24 * 3600 / (OPTIONS->dt/2.) + 1 ;

	  
	  INTEGRATOR->f107[index_in_driver_interpolated]  = OPTIONS->f107[index_in_driver_interpolated] + OPTIONS->mod_f107[CONSTELLATION->aaa_mod[INTEGRATOR->sc_main_nb]];
	  INTEGRATOR->Ap[index_in_driver_interpolated]    = OPTIONS->Ap[index_in_driver_interpolated] + OPTIONS->mod_ap[CONSTELLATION->aaa_mod[INTEGRATOR->sc_main_nb]]; // note: never ap_hist option if spwc prediction


	  /* if (iProc == 0){ */
	  /* 	if (INTEGRATOR->sc_main_nb==0){ */
	  /* 	  etprint(et, ""); */
	  /* 	  printf("(%d %d) %f %f (%d)\n", INTEGRATOR->sc_main_nb, INTEGRATOR->sc_ensemble_nb,  INTEGRATOR->f107[index_in_driver_interpolated] , OPTIONS->mod_f107[CONSTELLATION->aaa_mod[INTEGRATOR->sc_main_nb]], CONSTELLATION->aaa_mod[INTEGRATOR->sc_main_nb] ); */

	  /* 	  } */
	  /* } */


	  /* if (iProc == 3){ */
	  /* 	etprint(et, ""); */
	  /* 	printf("%f\n",OPTIONS->mod_ap[CONSTELLATION->aaa_mod[INTEGRATOR->sc_main_nb]]); */
	  /* 	} */
	  // Initialization of swpc_et_first_prediction for the calculation of F10.7A for an ensemble
	  if ( et_sc_initial > OPTIONS->swpc_et_first_prediction){ // if the propagation starts in the future. Otherwise, sum_sigma_for_f107_average = 0 for all ensemble sc

	    if (  CONSTELLATION->sum_sigma_for_f107_average[INTEGRATOR->sc_main_nb][INTEGRATOR->sc_ensemble_nb] == 0 )  {// for the first time that variable et gets bigger than swpc_et_first_prediction 
	      // initialize sum_sigma_for_f107_average as the sum of all mod on F10.7 from first prediction to inital epoch.
	      for (aaa_a = 0; aaa_a < CONSTELLATION->aaa_mod[INTEGRATOR->sc_main_nb]; aaa_a++){

		CONSTELLATION->sum_sigma_for_f107_average[INTEGRATOR->sc_main_nb][INTEGRATOR->sc_ensemble_nb] = CONSTELLATION->sum_sigma_for_f107_average[INTEGRATOR->sc_main_nb][INTEGRATOR->sc_ensemble_nb] + OPTIONS->mod_f107[aaa_a];		    
		//		      ptd( OPTIONS->mod_f107[aaa_a], "s");
	      }

	      //		      printf("eee: %d | index: %d | iProc: %d | sum: %f\n", INTEGRATOR->sc_ensemble_nb, index_in_driver_interpolated, iProc, CONSTELLATION->sum_sigma_for_f107_average[INTEGRATOR->sc_main_nb][INTEGRATOR->sc_ensemble_nb]);exit(0);
		   
	    } // end of for the first time that variable et gets bigger than swpc_et_first_prediction
	  }// end of if the propagation starts in the future  

	
	  CONSTELLATION->sum_sigma_for_f107_average[INTEGRATOR->sc_main_nb][INTEGRATOR->sc_ensemble_nb] = CONSTELLATION->sum_sigma_for_f107_average[INTEGRATOR->sc_main_nb][INTEGRATOR->sc_ensemble_nb] +  ( INTEGRATOR->f107[index_in_driver_interpolated]  - OPTIONS->f107[index_in_driver_interpolated] );
	  INTEGRATOR->f107A[index_in_driver_interpolated]         = OPTIONS->f107A[index_in_driver_interpolated] + CONSTELLATION->sum_sigma_for_f107_average[INTEGRATOR->sc_main_nb][INTEGRATOR->sc_ensemble_nb] / nb_index_in_81_days; // derivation of F10.7A considerin

	  /* if (iProc == 3){ */
	  /* 	if (INTEGRATOR->sc_main_nb == 1){ */
	  /* 	etprint(et, ""); */
	  /* 	//		printf("%f (%d  %d) \n", INTEGRATOR->f107[index_in_driver_interpolated], index_in_driver_interpolated, INTEGRATOR->sc_ensemble_nb); */
	  /* 		      	printf("%f %f %f | %f  (%d %d)\n",OPTIONS->f107A[index_in_driver_interpolated], INTEGRATOR->f107[index_in_driver_interpolated], OPTIONS->f107[index_in_driver_interpolated] ,CONSTELLATION->sum_sigma_for_f107_average[INTEGRATOR->sc_main_nb][INTEGRATOR->sc_ensemble_nb] ,INTEGRATOR->sc_main_nb, INTEGRATOR->sc_ensemble_nb); */
	  /* } */
	  /* 	} */
	  //	 	  printf("%d %d\n",index_in_driver_interpolated,INTEGRATOR->sc_main_nb);

 
	}


	else{

	  INTEGRATOR->Ap[index_in_driver_interpolated]            = OPTIONS->Ap[index_in_driver_interpolated];           // magnetic index(daily)

	  if (OPTIONS->use_ap_hist == 1){
	    for (hhh = 0; hhh < 7; hhh++){
	      INTEGRATOR->Ap_hist[hhh][index_in_driver_interpolated]            = OPTIONS->Ap_hist[hhh][index_in_driver_interpolated];           // magnetic index(historical)
	    }
	  }

	  INTEGRATOR->f107[index_in_driver_interpolated]          = OPTIONS->f107[index_in_driver_interpolated];         // Daily average of F10.7 flux
	  INTEGRATOR->f107A[index_in_driver_interpolated]         = OPTIONS->f107A[index_in_driver_interpolated];        // 81 day average of F10.7 flux
	  //	  printf("[%d] %f\n", index_in_driver_interpolated, OPTIONS->f107A[index_in_driver_interpolated]);

	}



	//  	    printf("%d %f %d %f\n", INTEGRATOR->sc_ensemble_nb, INTEGRATOR->f107[index_in_driver_interpolated], index_in_driver_interpolated, OPTIONS->f107[index_in_driver_interpolated]);
      } // end of if we don't run ensembles on F10.7/Ap or that we run ensemble on F10.7/Ap but that the current time is before the first prediction (so there is no uncertainty in F10.7/Ap because the time corresponds to an observation, not a prediction)
      //	  if (iProc == 0){
      //if (index_in_driver_interpolated == 4){ 
      // printf("eee: %d | f107:  %f | f107A: %f | Ap: %f | index: %d | iProc: %d\n", INTEGRATOR->sc_ensemble_nb, INTEGRATOR->f107[index_in_driver_interpolated],INTEGRATOR->f107A[index_in_driver_interpolated], INTEGRATOR->Ap[index_in_driver_interpolated] , index_in_driver_interpolated, iProc);
      // }
      //	  	  }
      /* if (iProc == 0){ */
      /*   //	    if ((INTEGRATOR->sc_ensemble_nb == 1 + iProc * OPTIONS->nb_ensemble_min_per_proc + 1)){ */
      /*   if (index_in_driver_interpolated == 4){ */
      /*     printf("eee: %d | f107:  %f | index: %d | iProc: %d\n", INTEGRATOR->sc_ensemble_nb, INTEGRATOR->f107[index_in_driver_interpolated], index_in_driver_interpolated, iProc); */
      /* } */
      /* } */


      input.f107A = INTEGRATOR->f107A[index_in_driver_interpolated];
      input.f107  = INTEGRATOR->f107[index_in_driver_interpolated];
      //      printf(" %f | %f || ", input.f107, input.f107A);
      if (PARAMS->ATMOSPHERE.flags.switches[9] != -1){ // if daily ap
	if (INTEGRATOR->Ap[index_in_driver_interpolated] < 0){ // this can happen when running ensembles on Ap
	  INTEGRATOR->Ap[index_in_driver_interpolated] = 0;
	}

	input.ap    = INTEGRATOR->Ap[index_in_driver_interpolated];
	if ( INTEGRATOR->sc_ensemble_nb == 0 ){
	  if (INTEGRATOR->last_compute_dxdt == 1){
	  if (iDebugLevel >= 5){
	    printf("------ (compute_drag) Writing in file_given_output...(iProc %d | iii = %d, eee = %d)\n", iProc, INTEGRATOR->sc_main_nb, INTEGRATOR->sc_ensemble_nb);
	  }

	  	  fprintf(INTEGRATOR->file_given_output, "%s: %f %f %f\n", timestamp_isoc, input.f107,input.f107A, input.ap);
	  if (iDebugLevel >= 5){
	    printf("------ (compute_drag) Done writing in file_given_output...(iProc %d | iii = %d, eee = %d)\n", iProc, INTEGRATOR->sc_main_nb, INTEGRATOR->sc_ensemble_nb);
	  }
	  }
	}

      }

      else{ //if historical ap

	input.ap_a = malloc(7 * sizeof(double));
	//		printf("%s: ", timestamp);
	if ( INTEGRATOR->sc_ensemble_nb == 0 ){
	if (SC->INTEGRATOR.write_given_output == 1){
	  if (INTEGRATOR->last_compute_dxdt == 1){
	  	    fprintf(INTEGRATOR->file_given_output, "%s: %f %f", timestamp_isoc, input.f107,input.f107A);
		    
		    //		    printf("%s %d %d %f\n", timestamp_isoc,SC->INTEGRATOR.sc_main_nb,index_in_driver_interpolated);
	  }
	}
	}
	for (ppp = 0; ppp < 7 ; ppp ++){
	  if (INTEGRATOR->Ap_hist[ppp][index_in_driver_interpolated] < 0){ // this can happen when running ensembles on Ap
	    INTEGRATOR->Ap_hist[ppp][index_in_driver_interpolated] = 0;
	  }

	  input.ap_a->a[ppp]    = INTEGRATOR->Ap_hist[ppp][index_in_driver_interpolated];
	  if ( INTEGRATOR->sc_ensemble_nb == 0 ){
	if (SC->INTEGRATOR.write_given_output == 1){
	  if (INTEGRATOR->last_compute_dxdt == 1){
	  	        fprintf(INTEGRATOR->file_given_output, " %f", input.ap_a->a[ppp]);
	  }
	}
	  }
	  //	   	  	  printf("[%d] %f | ", ppp, input.ap_a->a[ppp]);
		    
	}
	if ( INTEGRATOR->sc_ensemble_nb == 0 ){
	  //	  printf("\n");
	if (SC->INTEGRATOR.write_given_output == 1){
	  if (INTEGRATOR->last_compute_dxdt == 1){
	  	    fprintf(INTEGRATOR->file_given_output, "\n");
	  }
	}
	}
      } // end of if historical ap    

      if (iDebugLevel >= 3){
	printf("---- (compute_drag) Done computing F10.7, F10.7A, and Ap to determine density for drag (iProc %d | iii = %d, eee = %d)\n", iProc, INTEGRATOR->sc_main_nb, INTEGRATOR->sc_ensemble_nb);
      }




    } // end of if the user chooses a time varying f107 and Ap for the density  
    //        printf("\n");
    //  printf("%f %f %f ||| %s\n", input.f107A, input.f107, input.ap, timestamp);
    /* if (input.f107A < 0){ */
    /*   print_test(); */
    /*   exit(0); */
    /* } */
    //  exit(0);


/*     // !!!!!!!!!!!! remove block below */
/*     double mean_earth_sun_distance = 149597870.700; */
/*     double earth_sun_distance; */
/*   double x[6]; */
/*   double lt; */
/*   double r_earth2sun_J2000[3]; */

/*     spkez_c(10, et, "J2000", "NONE", 399, x, &lt); //   Return the state (position and velocity) of a target body relative to an observing body, optionally corrected for light time (planetary aberration) and stellar aberration. */

/*     r_earth2sun_J2000[0] = x[0]; */
/*     r_earth2sun_J2000[1] = x[1]; */
/*     r_earth2sun_J2000[2] = x[2]; */
     
/*     v_mag(&earth_sun_distance, r_earth2sun_J2000); */
/* /\*     printf("%e %e\n", earth_sun_distance,mean_earth_sun_distance); *\/ */
/* /\*     printf("b %f %f\n", input.f107, input.f107A ); *\/ */
/*     input.f107 = input.f107 * ( mean_earth_sun_distance / earth_sun_distance ) * ( mean_earth_sun_distance / earth_sun_distance ); */
/*     input.f107A = input.f107A * ( mean_earth_sun_distance / earth_sun_distance ) * ( mean_earth_sun_distance / earth_sun_distance ); */
/* /\*     printf("a %f %f\n", input.f107, input.f107A ); *\/ */
/*     // !!!!!!!!!!!! end of remove block below */

    // Call the NRL-MSIS-00 Atmosphere Model
    gtd7d(&input, &PARAMS->ATMOSPHERE.flags, &output);
    //    printf("T: %f\n", output.t[1]);
    if (PARAMS->ATMOSPHERE.flags.switches[9] == -1){ // if we used hitorical ap
      free(input.ap_a);
    }
    output.d[5] = (output.d[5] / 1000) * (100 * 100 * 100 ) *1e9; // Convert from Gram/cm^3 to kg/m^3 to kg/km^3 (CBV for the kg/km^3)
    *density = output.d[5];
    //    printf("%f %f %e\n", input.f107, input.f107A, *density);
    SC->INTEGRATOR.Ta = output.t[1];
  } // end of the user chooses f107 and Ap for the density
  else if ( strcmp(INTEGRATOR->format_density_driver, "density_file") == 0 ){ // the user chooses to directly input the density from a file
    //    index_in_attitude_interpolated = floor( ( et - et_initial_epoch ) / ( INTEGRATOR->dt / 2.0) ) ;
    *density = INTEGRATOR->density[index_in_attitude_interpolated];
    SC->INTEGRATOR.Ta = 800.;
  } // end of the user chooses to directly input the density from a file

  else if ( strcmp(INTEGRATOR->format_density_driver, "gitm") == 0 ){ // the user chooses GITM
    // GITM outputs in longitude/latitude/altitude cordinates so the conversion ECI to longitude/latitude/altitude needs to be done before calling GITM
	
    // // ECI to ECEF
/*     double geodetic[3]; */
/* 	eci2lla(r_i2cg_INRTL , et, geodetic ); */
/* 	altitude_gitm = geodetic[2]; */
/* 	latitude_gitm = geodetic[0]; */
/* 	longitude_gitm = geodetic[1];  */

/* 	// update the planet fixed state		 */
/* 	  geodetic_to_geocentric(PARAMS->EARTH.flattening,             */
/* 				 altitude_gitm, */
/* 				 latitude_gitm, */
/* 				 longitude_gitm, */
/* 				 PARAMS->EARTH.radius,        */
/* 				 r_ecef2cg_ECEF_gitm) ; */

    pxform_c("J2000", PARAMS->EARTH.earth_fixed_frame, et, T_J2000_to_ECEF_gitm);
    m_x_v(r_ecef2cg_ECEF_gitm, T_J2000_to_ECEF_gitm, r_i2cg_INRTL);
    
    // // ECEF to longitude/latitude/altitude
    geocentric_to_geodetic( r_ecef2cg_ECEF_gitm,
			    &PARAMS->EARTH.radius,
			    &PARAMS->EARTH.flattening,
			    &altitude_gitm,
			    &latitude_gitm,
			    &longitude_gitm);

    gitm_density(density, et, altitude_gitm, latitude_gitm, longitude_gitm, PARAMS);
    *density = *density * 1e9;
    SC->INTEGRATOR.Ta = 800.;
  } // end of the user chooses GITM
  //				printf("density = %e\n", *density);
  /////////////////// END OF COMPUTE DENSITY ///////////////////

  // apply density_mod: factor to apply on density at position of satellite (calculated by NRLMSIS, gitm or from density file)
  *density = *density * SC->INTEGRATOR.density_mod;


  compute_T_inrtl_2_ntw(T_inrtl_2_ntw, r_i2cg_INRTL, v_i2cg_INRTL);

  // !!!!!!!!!!!!!!!! the code for the rotating atmo has not been checked yet
  // Take into account the rotating atmosphere: v_rel = v_sat - v_rot_atmo
/*   //  // Update ECEF state */
/*   SpiceDouble       xform[6][6]; */
/*   double estate[6], jstate[6]; */

/*   estate[0] = r_i2cg_INRTL[0];estate[1] = r_i2cg_INRTL[1];estate[2] = r_i2cg_INRTL[2]; */
/*   estate[3] = v_i2cg_INRTL[0];estate[4] = v_i2cg_INRTL[1];estate[5] = v_i2cg_INRTL[2]; */
/*   sxform_c (  "J2000", PARAMS->EARTH.earth_fixed_frame,  et,    xform  );  */
/*   mxvg_c   (  xform,       estate,   6,  6, jstate );  */
/*   double r_ecef2cg_ECEF_for_rot_atmo[3], v_ecef2cg_ECEF_for_rot_atmo[3]; */
/*   r_ecef2cg_ECEF_for_rot_atmo[0] = jstate[0]; r_ecef2cg_ECEF_for_rot_atmo[1] = jstate[1]; r_ecef2cg_ECEF_for_rot_atmo[2] = jstate[2]; */
/*   v_ecef2cg_ECEF_for_rot_atmo[0] = jstate[3]; v_ecef2cg_ECEF_for_rot_atmo[1] = jstate[4]; v_ecef2cg_ECEF_for_rot_atmo[2] = jstate[5]; */

/*   double v_rel[3]; */
/*   double v_rot_atmo[3]; */
/*   double omega_rot_atmo[3]; */
/*   omega_rot_atmo[0] = 0; omega_rot_atmo[1] = 0; omega_rot_atmo[2] =  0.0;// !!!!!! should be 0.00007292158553; */
/*   v_cross(v_rot_atmo, omega_rot_atmo, r_ecef2cg_ECEF_for_rot_atmo); */
/*   v_sub(v_rel, v_ecef2cg_ECEF_for_rot_atmo, v_rot_atmo); */
  
/*   // // Back to ECI */
/*   SpiceDouble       xform_new[6][6]; */
/*   double estate_new[6], jstate_new[6]; */
/*   sxform_c (  PARAMS->EARTH.earth_fixed_frame, "J2000",  et,    xform_new  );  */
/*   estate_new[0] = r_ecef2cg_ECEF_for_rot_atmo[0];estate_new[1] = r_ecef2cg_ECEF_for_rot_atmo[1];estate_new[2] = r_ecef2cg_ECEF_for_rot_atmo[2]; */
/*   estate_new[3] = v_rel[0];estate_new[4] = v_rel[1];estate_new[5] = v_rel[2]; */
/*   mxvg_c   (  xform_new,       estate_new,   6,  6, jstate_new );  */
/*   double v_rel_eci[3]; */
/*   v_rel_eci[0] = jstate_new[3]; v_rel_eci[1] = jstate_new[4]; v_rel_eci[2] = jstate_new[5]; */
/*   /\* v_print(v_rot_atmo, "v_rot_atmo"); *\/ */
/*   /\* v_print(v_rel, "v_rel"); *\/ */
  double v_rel_eci[3];
  double v_rot_atmo[3];
  double omega_rot_atmo_scal = 0.00007292158553;
  v_rel_eci[0] = v_i2cg_INRTL[0] + omega_rot_atmo_scal * r_i2cg_INRTL[1];
  v_rel_eci[1] = v_i2cg_INRTL[1] - omega_rot_atmo_scal * r_i2cg_INRTL[0];
  v_rel_eci[2] = v_i2cg_INRTL[2];
  // End of take into account the rotating atmosphere: v_rel = v_sat - v_rot_atmo

  m_trans(T_ntw_2_inrtl, T_inrtl_2_ntw);
  m_x_v(v_i2cg_NTW, T_inrtl_2_ntw, v_rel_eci); //  !!!!!!!!!!!!!!!! v_rel_eci here is new!Before it was: v_i2cg_INRTL
  // !!!!!!!!!!!!!!!! the code for the rotating atmo has not been checked yet
  //  v_print(v_i2cg_NTW, "v_i2cg_NTW");

  if (strcmp(INTEGRATOR->attitude.attitude_profile, "ensemble_angular_velocity") == 0){
    v_angle[0] = INTEGRATOR->attitude.pitch_angular_velocity_ensemble * ( et - et_sc_initial ) + INTEGRATOR->attitude.pitch_ini_ensemble;
    v_angle[1] = INTEGRATOR->attitude.roll_angular_velocity_ensemble * ( et - et_sc_initial ) + INTEGRATOR->attitude.roll_ini_ensemble;
    v_angle[2] = INTEGRATOR->attitude.yaw_angular_velocity_ensemble * ( et - et_sc_initial ) + INTEGRATOR->attitude.yaw_ini_ensemble;
    order_rotation[0] = 1; // !!!!!!!! we might want to change that in the future
    order_rotation[1] = 2;
    order_rotation[2] = 3;
    INTEGRATOR->attitude.pitch_current = v_angle[0];
    INTEGRATOR->attitude.roll_current = v_angle[1];
    INTEGRATOR->attitude.yaw_current = v_angle[2];
    INTEGRATOR->attitude.order_pitch_current = order_rotation[0];
    INTEGRATOR->attitude.order_roll_current = order_rotation[1];
    INTEGRATOR->attitude.order_yaw_current = order_rotation[2];


  }

  if (strcmp(INTEGRATOR->attitude.attitude_profile, "ensemble_initial_attitude") == 0) {
    v_angle[0] =  INTEGRATOR->attitude.pitch_for_attitude_ensemble  +  INTEGRATOR->attitude.pitch_angular_velocity_constant * ( et - et_sc_initial );
    v_angle[1] =  INTEGRATOR->attitude.roll_for_attitude_ensemble  +  INTEGRATOR->attitude.roll_angular_velocity_constant * ( et - et_sc_initial );
    v_angle[2] =  INTEGRATOR->attitude.yaw_for_attitude_ensemble  +  INTEGRATOR->attitude.yaw_angular_velocity_constant * ( et - et_sc_initial );

    order_rotation[0]  = 1; order_rotation[1]  = 2; order_rotation[2]  = 3;
	   
    INTEGRATOR->attitude.pitch_current = v_angle[0];
    INTEGRATOR->attitude.roll_current = v_angle[1];
    INTEGRATOR->attitude.yaw_current = v_angle[2];
	          INTEGRATOR->attitude.order_pitch_current = order_rotation[0];
    INTEGRATOR->attitude.order_roll_current = order_rotation[1];
    INTEGRATOR->attitude.order_yaw_current = order_rotation[2];


  }
  
  if ( (strcmp(INTEGRATOR->attitude.attitude_profile, "ensemble_angular_velocity") != 0) && (strcmp(INTEGRATOR->attitude.attitude_profile, "ensemble_initial_attitude") != 0) && (strcmp(INTEGRATOR->attitude.attitude_profile, "sun_pointed") != 0) ){ // otherwise (atittude is nadir, sun_pointed or manual (from an input file)) 
    //  index_in_attitude_interpolated = floor( ( et - et_initial_epoch ) / ( INTEGRATOR->dt / 2.0) ) ;


    //    et2utc_c(et, "D", 3, 35, timestamp);
    //  printf("et = %s ||", timestamp);
    /*  et2utc_c(et_initial_epoch, "D", 3, 35, timestamp); */
    /* printf("et_initial_epoch = %s \n", timestamp); */
	    if (INTEGRATOR->file_is_quaternion == 0){
    v_angle[0] = INTEGRATOR->attitude.pitch[index_in_attitude_interpolated];
    v_angle[1] = INTEGRATOR->attitude.roll[index_in_attitude_interpolated];
    v_angle[2] = INTEGRATOR->attitude.yaw[index_in_attitude_interpolated];
    //    printf("%d %d %d\n", INTEGRATOR->attitude.order_pitch[index_in_attitude_interpolated], index_in_attitude_interpolated, INTEGRATOR->sc_ensemble_nb);
    order_rotation[0] = INTEGRATOR->attitude.order_pitch[index_in_attitude_interpolated];
    order_rotation[1] = INTEGRATOR->attitude.order_roll[index_in_attitude_interpolated];
    order_rotation[2] = INTEGRATOR->attitude.order_yaw[index_in_attitude_interpolated];
    // printf("%d %d %d\n", order_rotation[0], order_rotation[1],order_rotation[2]);
    INTEGRATOR->attitude.pitch_current = v_angle[0];
    INTEGRATOR->attitude.roll_current = v_angle[1];
    INTEGRATOR->attitude.yaw_current = v_angle[2];
    INTEGRATOR->attitude.order_pitch_current = order_rotation[0];
    INTEGRATOR->attitude.order_roll_current = order_rotation[1];
    INTEGRATOR->attitude.order_yaw_current = order_rotation[2];

	    }
	    else{
	      q_copy( INTEGRATOR->attitude.quaternion_current, INTEGRATOR->attitude.quaternion[index_in_attitude_interpolated]);
	    }
    
    //       printf("%d | %e | %e | %e \n", index_in_attitude_interpolated, v_angle[0], v_angle[1],v_angle[2]);
  }

  compute_T_sc_to_lvlh( T_sc_to_lvlh, v_angle, order_rotation, INTEGRATOR->attitude.attitude_profile, &et,  r_i2cg_INRTL, v_i2cg_INRTL, INTEGRATOR->file_is_quaternion, INTEGRATOR->attitude.quaternion_current);



  //  m_print(T_sc_to_lvlh, "T_sc_to_lvlh");
  //  printf("%d %d %d %d %d\n", index_in_attitude_interpolated, OPTIONS->nb_time_steps*2, order_rotation[0], order_rotation[1], order_rotation[2]);

  /* LVLH to inertial */

  compute_T_inrtl_2_lvlh(T_inrtl_2_lvlh, r_i2cg_INRTL, v_i2cg_INRTL);
  m_trans(T_lvlh_2_inrtl, T_inrtl_2_lvlh);


  double v_i2cg_NTW_mag;
  v_mag(&v_i2cg_NTW_mag, v_i2cg_NTW);

  double cd;
	double cos_angle_v_sc_normal;
	double normal_in_ntw_normalized;
 
	double A_ref;
	double A_ref_tot = 0;
	double cd_tot_norm =  0;
	//	   	etprint(et, "");

  if (INTEGRATOR->initialize_geo_with_bstar != 1){

    // to not use the new cd calculation, uncomment line below
    if (OPTIONS->new_cd !=1){
       ballistic_coefficient = INTEGRATOR->surface[0].Cd * INTEGRATOR->surface[0].area / INTEGRATOR->mass;
       cd =  INTEGRATOR->surface[0].Cd;
    }
/* 	if (SC->INTEGRATOR.write_given_output == 1){ */
/* 	  	  fprintf(INTEGRATOR->file_given_output, "%s ", timestamp_isoc ); */
/* 	} */
    for (sss = 0; sss < INTEGRATOR->nb_surfaces; sss++){
      //      printf("surface %d\n", sss);
      if (sss == 0){
	/* Conversion of the normal vector from the SC reference system to the NTW reference system */

	/* SC to LVLH */
	  m_x_v(normal_in_lvlh, T_sc_to_lvlh, INTEGRATOR->surface[0].normal );
	  /* LVLH to inertial */
	  m_x_v(normal_in_inertial, T_lvlh_2_inrtl, normal_in_lvlh);



	/* inertial to NTW (note: we could simply make the dot product of the speed and the normal vector of the surface in the intertial frame, we don't have to do convert both vectors in the NTW frame! But this was done because it was easy to check that the consistency of the results in the NTW frame (it does not add that much time at the computation)) */

	m_x_v(normal_in_ntw, T_inrtl_2_ntw, normal_in_inertial);
	v_dot(&v_i2cg_NTW_DOT_normal_to_the_surface_NTW, v_i2cg_NTW, normal_in_ntw);



	// !!!!!!!!!!!!!!!!!!!!!! REMOVE LINES BELOW!!!!!!!!!

	/* v_mag(&v_i2cg_NTW_DOT_normal_to_the_surface_NTW, v_i2cg_NTW); */
	/* ballistic_coefficient = 2.2 * 0.01 / 1000./1000.; */
	// !!!!!!!!!!!!!!!!!!!!!! END OF REMOVE LINES BELOW!!!!!!!!!	

	v_mag(&normal_in_ntw_normalized, normal_in_ntw);// just in case it's not normalized 
	cos_angle_v_sc_normal = v_i2cg_NTW_DOT_normal_to_the_surface_NTW / ( v_i2cg_NTW_mag * normal_in_ntw_normalized );
	//      printf("%e\n", cos_angle_v_sc_normal);

	if (cos_angle_v_sc_normal > 5e-3){ // 1e-3 for numerical reasons (should be 0 theoritically)

	  
    // new cd
	  if (OPTIONS->new_cd == 1){
	calculate_cd(&cd,
		     OPTIONS->surface[0].acco_coeff,// !!!!!!!! shoulc be OPTIONS->surface[0].acco_coeff, 
		     v_i2cg_NTW_mag, //in km/s
		     SC->INTEGRATOR.Ta, // atmospheric temperature in K, from NRLSMSIS if the user didn't chose "density_file" or "gitm", in which case SC->INTEGRATOR.Ta = 800K
		     INTEGRATOR->surface[0].area, // in m^2
		     cos_angle_v_sc_normal,
		     16./1000 // in kg/mol!!!!! assuming mainly atom of O
		     );
	//	printf("cd[%d]: %f | %e %f\n", sss, cd,cd*INTEGRATOR->surface[sss].area*cos_angle_v_sc_normal, OPTIONS->surface[0].acco_coeff );
    ballistic_coefficient = cd * INTEGRATOR->surface[0].area / INTEGRATOR->mass;
	  }
    // end of new cd
      A_ref = INTEGRATOR->surface[0].area * cos_angle_v_sc_normal * 1000000; // convert km^2 to m^2

	  total_cross_area = total_cross_area + INTEGRATOR->surface[0].area* 1000000 * cos_angle_v_sc_normal;
/* 	if (SC->INTEGRATOR.write_given_output == 1){ */
/* 	  	  fprintf(INTEGRATOR->file_given_output, "%e %d %e (%e) %e | %e || ", cd, sss, cos_angle_v_sc_normal, v_i2cg_NTW_DOT_normal_to_the_surface_NTW , A_ref, cd*A_ref); */
/* 	} */

        A_ref_tot = A_ref_tot + A_ref;
	cd_tot_norm = cd_tot_norm + cd*A_ref;


	  a_i2cg_NTW[0] =  -0.5 * *density * v_i2cg_NTW_DOT_normal_to_the_surface_NTW * v_i2cg_NTW[0] * ballistic_coefficient; // Correction by CBV: added the factor INTEGRATOR->Cd //  first component of NTW is v normalized-> IN TRACK (CBV)
	  a_i2cg_NTW[1] =  -0.5 * *density * v_i2cg_NTW_DOT_normal_to_the_surface_NTW * v_i2cg_NTW[1] * ballistic_coefficient; // radial cross v (then normalized) -> CROSS-TRACK (CBV)
	  a_i2cg_NTW[2] =  -0.5 * *density * v_i2cg_NTW_DOT_normal_to_the_surface_NTW * v_i2cg_NTW[2] * ballistic_coefficient; // first component cross second component (CBV)

	}
	else{
	  a_i2cg_NTW[0] = 0.0;
	  a_i2cg_NTW[1] = 0.0;
	  a_i2cg_NTW[2] = 0.0;
	}
      } // end of if sss == 0
      else{

	/* Conversion of the normal vector from the SC reference system to the NTW reference system */

	/* SC to LVLH */
	m_x_v(normal_in_lvlh, T_sc_to_lvlh, INTEGRATOR->surface[sss].normal );
      
	/* LVLH to inertial */
	m_x_v(normal_in_inertial, T_lvlh_2_inrtl, normal_in_lvlh);


	/* inertial to NTW (note: we could simply make the dot product of the speed and the normal vector of the surface in the intertial frame, we don't have to do convert both vectors in the NTW frame! But this was done because it was easy to check that the consistency of the results in the NTW frame (it does not add that much time at the computation)) */
	m_x_v(normal_in_ntw, T_inrtl_2_ntw, normal_in_inertial);
	
	v_dot(&v_i2cg_NTW_DOT_normal_to_the_surface_NTW, v_i2cg_NTW, normal_in_ntw);
	v_mag(&normal_in_ntw_normalized, normal_in_ntw);// just in case it's not normalized 
	cos_angle_v_sc_normal = v_i2cg_NTW_DOT_normal_to_the_surface_NTW / ( v_i2cg_NTW_mag * normal_in_ntw_normalized );


	if (cos_angle_v_sc_normal > 5e-3){ // 1e-3 for numerical reasons (should be 0 theoritically)
	  if (INTEGRATOR->initialize_geo_with_bstar != 1){


    // new cd
	  if (OPTIONS->new_cd == 1){
	calculate_cd(&cd,
		     OPTIONS->surface[sss].acco_coeff,// !!!!!!!! shoul be OPTIONS->surface[sss].acco_coeff, 
		     v_i2cg_NTW_mag, //in km/s
		     SC->INTEGRATOR.Ta, // atmospheric temperature in K, from NRLSMSIS if the user didn't chose "density_file" or "gitm", in which case SC->INTEGRATOR.Ta = 800K
		     INTEGRATOR->surface[sss].area, // in m^2
		     cos_angle_v_sc_normal,
		     16./1000 // in kg/mol!!!!! assuming mainly atom of O
		     );
	//	printf("cd: %f | %e %f\n", sss, cd,cd*INTEGRATOR->surface[sss].area*cos_angle_v_sc_normal, OPTIONS->surface[sss].acco_coeff );
    ballistic_coefficient = cd * INTEGRATOR->surface[sss].area / INTEGRATOR->mass;
	  }
    // end of new cd
	        A_ref = INTEGRATOR->surface[sss].area * cos_angle_v_sc_normal * 1000000; // convert km^2 to km

/* 	if (SC->INTEGRATOR.write_given_output == 1){ */
/* 	  	  total_cross_area = total_cross_area + INTEGRATOR->surface[sss].area* 1000000 * cos_angle_v_sc_normal; */
/* 		  //		  fprintf(INTEGRATOR->file_given_output, "%e %d %e (%e) %e | %e || ", cd, sss, cos_angle_v_sc_normal,v_i2cg_NTW_DOT_normal_to_the_surface_NTW, A_ref,  cd*A_ref ); */
/* 	} */



	if (OPTIONS->new_cd !=1){
    	    ballistic_coefficient = INTEGRATOR->surface[sss].Cd * INTEGRATOR->surface[sss].area / INTEGRATOR->mass;
	    cd = INTEGRATOR->surface[sss].Cd;
	}

        A_ref_tot = A_ref_tot + A_ref;
	cd_tot_norm = cd_tot_norm + cd*A_ref;

	  }


	  a_i2cg_NTW[0] = a_i2cg_NTW[0]  -0.5 * *density * v_i2cg_NTW_DOT_normal_to_the_surface_NTW * v_i2cg_NTW[0] * ballistic_coefficient; // Correction by CBV: added the factor INTEGRATOR->Cd //  first component of NTW is v normalized-> IN TRACK (CBV)
	  a_i2cg_NTW[1] = a_i2cg_NTW[1] -0.5 * *density * v_i2cg_NTW_DOT_normal_to_the_surface_NTW * v_i2cg_NTW[1] * ballistic_coefficient; // radial cross v (then normalized) -> CROSS-TRACK (CBV)
	  a_i2cg_NTW[2] = a_i2cg_NTW[2] -0.5 * *density * v_i2cg_NTW_DOT_normal_to_the_surface_NTW * v_i2cg_NTW[2] * ballistic_coefficient; // first component cross second component (CBV)

	}
	
      }
   
    }
  }

  else if (INTEGRATOR->initialize_geo_with_bstar == 1){

    ballistic_coefficient = 6378. * 2.461 * 0.00001 / (2 * INTEGRATOR->bstar ) ;
    ballistic_coefficient = 1/ballistic_coefficient / 1000000; // "/1000000" to convert from m^2/kg to km^2/kg

    a_i2cg_NTW[0] =  -0.5 * *density *  v_i2cg_NTW[0] * v_i2cg_NTW[0] * ballistic_coefficient; // Correction by CBV: added the factor INTEGRATOR->Cd //  first component of NTW is v normalized-> IN TRACK (CBV)
    a_i2cg_NTW[1] =  -0.5 * *density * v_i2cg_NTW[1] * v_i2cg_NTW[1] * ballistic_coefficient; // radial cross v (then normalized) -> CROSS-TRACK (CBV)
    a_i2cg_NTW[2] =  -0.5 * *density * v_i2cg_NTW[2] * v_i2cg_NTW[2] * ballistic_coefficient; // first component cross second component (CBV)

  }


  // !!!!!!!!!!!!!!!!!! ERASE LINES BELOW
  /*  v_norm_print(v_i2cg_NTW, "v_i2cg_NTW"); */
  /*  v_norm_print(v_i2cg_INRTL, "v_i2cg_INRTL"); */
  /*   double v_mag_inrtl;  */
  /* v_mag(&v_mag_inrtl, v_rel_eci); */
  /* v_scale(adrag_i2cg_INRTL, v_rel_eci, -1/2* *density * 2.2 * 0.01 * M_PI / 1000. / 1000.* v_mag_inrtl); */
  /* v_norm_print(adrag_i2cg_INRTL, "adrag_i2cg_INRTL"); */
  //  MPI_Finalize();exit(0);
  // !!!!!!!!!!!!!!!!!! END OF ERASE LINES BELOW

  if ( strcmp(INTEGRATOR->format_density_driver, "density_file") != 0 ){ // the user does not choose to directly input the density from a file
    INTEGRATOR->density[index_in_driver_interpolated] = *density;
  }

  m_x_v(adrag_i2cg_INRTL, T_ntw_2_inrtl, a_i2cg_NTW);

  v_copy(SC->a_i2cg_INRTL_drag, adrag_i2cg_INRTL);
  if (iDebugLevel >= 2){
    printf("--- (compute_drag) Just got out of compute_drag ... (iProc %d | iii = %d, eee = %d)\n", iProc, INTEGRATOR->sc_main_nb, INTEGRATOR->sc_ensemble_nb   );
  }


/*   // # TOTAL NORMALIZED DRAG COEFFICIENT */
/*   double A_ref_tot = 0; */
/*   double A_ref; */
/*   double cd_tot_norm = 0; */
/*   for (sss = 0; sss < INTEGRATOR->nb_surfaces; sss++){ */
/*     if (cos_angle_v_sc_normal[sss] >= 0){ */
/*       A_ref = INTEGRATOR->surface[sss].area * cos_angle_v_sc_normal[sss] * 1000000; // convert km^2 to km */

/*         A_ref_tot = A_ref_tot + A_ref; */
/* 	cd_tot_norm = cd_tot_norm + cd[sss]*A_ref; */
/* 	//	printf("cd[%d] * A_ref: %e\n",sss, cd[sss]*A_ref); */
/*       } */
/*   } */

/* 	etprint(et, "time"); */
//	   printf("%e\n",cd_tot_norm);

  SC->density_here = *density;
	   SC->INTEGRATOR.sum_cd_a_cos = cd_tot_norm / 1000000;// used in the kalman filter. convert back to use km^2 on the area
  SC->INTEGRATOR.cd_tot_norm = cd_tot_norm / A_ref_tot;// # see sutton09a equation 9
  SC->INTEGRATOR.A_ref_tot =  A_ref_tot; // A_ref_tot in m^2 but in generate_ephemerides i convert it to km^2 for the output
  cd_tot_norm = cd_tot_norm / A_ref_tot;

	if (SC->INTEGRATOR.write_given_output == 1){
	  /* 	  fprintf(INTEGRATOR->file_given_output, "%e ", cd_tot_norm ); */
	  /* fprintf(INTEGRATOR->file_given_output, "%e \n", total_cross_area ); */
	  //	    printf( "%s %e\n", timestamp_isoc, SC->INTEGRATOR.cd_tot_norm );
	}

/* 	if (cd_tot_norm < 2.9){ */
/* 	  printf("XXXXXXXXXXXXXX\nXXXXXXXXXXXXX\n"); */
/* 	} */
  //  printf("Cd total normalized: %f %e\n",cd_tot_norm, A_ref_tot);
  //	printf("cd[%d]: %f | %e %f\n", sss, cd,cd*INTEGRATOR->surface[sss].area*cos_angle_v_sc_normal, OPTIONS->surface[0].acco_coeff );






  return 0;
}



/////////////////////////////////////////////////////////////////////////////////////////
//
//  Name:           shadow_light
//  Purpose:        Returns if the SC is in the umbra/penumbra of Earth
//  Assumptions:    None.
//  References      Vallado version 3 section 5.3.2
//
//  Change Log:
//      |   Developer   |       Date    |   SCR     |   Notes
//      | --------------|---------------|-----------|-------------------------------
//      | C. Bussy-Virat| 08/04/2015    |   ---     | Initial Implementation
//
/////////////////////////////////////////////////////////////////////////////////////////
int shadow_light( char      shadow[256],
		  double    r_i2cg_INRTL[3],
		  double    et,
		  PARAMS_T  *PARAMS) 
{

  /* Declarations */
  double x[6];
  double lt;
  double r_earth2sun_J2000[3];
  double mag_r_earth2sun_J2000;
  double r_earth2sun_J2000_normalized[3];
  double r_sun2earth_J2000_normalized[3];
  double r_i2cg_INRTL_normalized[3];
  double mag_r_i2cg_INRTL;
  double angle_minus_sun_to_sc;
  double cos_angle_minus_sun_to_sc;
  double sc_horiz, sc_vert; // satellite horizontal and vertical distances from the Sun-Earth line
  double pen_vert, umb_vert; // vertical lengths of the penumbra and the umbra
  double alpha_pen;
  double alpha_umb;
  
  /* Algorithm */
  
  /* Earth to Sun vector */
  spkez_c(10, et, "J2000", "NONE", 399, x, &lt); //   Return the state (position and velocity) of a target body relative to an observing body, optionally corrected for light time (planetary aberration) and stellar aberration. 
  r_earth2sun_J2000[0] = x[0];
  r_earth2sun_J2000[1] = x[1];
  r_earth2sun_J2000[2] = x[2];

  /* Umbra and penumbra angles */
  v_mag(&mag_r_earth2sun_J2000, r_earth2sun_J2000);
  alpha_umb = asin( ( PARAMS->SUN.radius - PARAMS->EARTH.radius ) / mag_r_earth2sun_J2000 ); // see errata of Vallado3
  alpha_pen = asin( ( PARAMS->SUN.radius + PARAMS->EARTH.radius ) / mag_r_earth2sun_J2000 ); // see errata of Vallado3

  /* Angle between the Sun-Earth direction and the Earth-satellite direction */
  v_norm(r_earth2sun_J2000_normalized, r_earth2sun_J2000);
  v_scale(r_sun2earth_J2000_normalized, r_earth2sun_J2000_normalized, -1.0);
  v_norm(r_i2cg_INRTL_normalized, r_i2cg_INRTL);
  v_dot(&cos_angle_minus_sun_to_sc, r_sun2earth_J2000_normalized, r_i2cg_INRTL_normalized);

  angle_minus_sun_to_sc = acos(cos_angle_minus_sun_to_sc);

  /* Calculate if the SC is in the umbra or penumbra */
  if (cos_angle_minus_sun_to_sc > 0){

    v_mag(&mag_r_i2cg_INRTL, r_i2cg_INRTL);
    sc_horiz = mag_r_i2cg_INRTL * cos( angle_minus_sun_to_sc );
    sc_vert =  mag_r_i2cg_INRTL * sin( angle_minus_sun_to_sc );
    pen_vert = PARAMS->EARTH.radius + tan( alpha_pen ) * sc_horiz;
  
    strcpy(shadow, "none");
    if ( sc_vert < pen_vert ){
      strcpy(shadow, "penumbra");
      umb_vert = PARAMS->EARTH.radius - tan( alpha_umb ) * sc_horiz;
      if ( sc_vert < umb_vert ){
	strcpy(shadow, "umbra");
      }
    }
    else{
      strcpy(shadow, "light");
    }
 
  }

  else{
    strcpy(shadow, "light");
  }

  return 0;
}



/////////////////////////////////////////////////////////////////////////////////////////
//
//  Name:           shadow_light
//  Purpose:        Returns if the SC is in the umbra/penumbra of Moon
//  Assumptions:    None.
//  References      Vallado version 3 section 5.3.2
//
//  Change Log:
//      |   Developer   |       Date    |   SCR     |   Notes
//      | --------------|---------------|-----------|-------------------------------
//      | C. Bussy-Virat| 08/04/2015    |   ---     | Initial Implementation
//
/////////////////////////////////////////////////////////////////////////////////////////
int shadow_light_moon( char      shadow[256],
		  double    r_i2cg_INRTL[3],
		  double    et,
		  PARAMS_T  *PARAMS) 
{

  /* Declarations */
  double x[6];
  double lt;
  double r_moon2sun_J2000[3];
  double mag_r_moon2sun_J2000;
  double r_moon2sun_J2000_normalized[3];
  double r_sun2moon_J2000_normalized[3];
  double mag_r_moon2sc_J2000;
  double angle_minus_sun_to_sc;
  double cos_angle_minus_sun_to_sc;
  double sc_horiz, sc_vert; // satellite horizontal and vertical distances from the Sun-Moon line
  double pen_vert, umb_vert; // vertical lengths of the penumbra and the umbra
  double alpha_pen;
  double alpha_umb;
  
  /* Algorithm */
  
  /* Moon to Sun vector */
  spkez_c(10, et, "J2000", "NONE", 301, x, &lt); //   Return the state (position and velocity) of a target body relative to an observing body, optionally corrected for light time (planetary aberration) and stellar aberration. 

  r_moon2sun_J2000[0] = x[0];
  r_moon2sun_J2000[1] = x[1];
  r_moon2sun_J2000[2] = x[2];

  /* Umbra and penumbra angles */
  v_mag(&mag_r_moon2sun_J2000, r_moon2sun_J2000);
  alpha_umb = asin( ( PARAMS->SUN.radius - PARAMS->MOON.radius ) / mag_r_moon2sun_J2000 ); // see errata of Vallado3
  alpha_pen = asin( ( PARAMS->SUN.radius + PARAMS->MOON.radius ) / mag_r_moon2sun_J2000 ); // see errata of Vallado3

  /* Angle between the Sun-Moon direction and the Moon-satellite direction */
  v_norm(r_moon2sun_J2000_normalized, r_moon2sun_J2000);
  v_scale(r_sun2moon_J2000_normalized, r_moon2sun_J2000_normalized, -1.0);

  double x_earth[6];
  double lt_earth;
spkez_c(10, et, "J2000", "NONE", 399, x_earth, &lt_earth); //   Return the state (position and velocity) of a target body relative to an observing body, optionally corrected for light time (planetary aberration) and stellar aberration. 

 double r_earth2sun_J2000[3];
  r_earth2sun_J2000[0] = x_earth[0];
  r_earth2sun_J2000[1] = x_earth[1];
  r_earth2sun_J2000[2] = x_earth[2];
  
  double r_moon2earth_J2000[3];
  v_sub(r_moon2earth_J2000, r_moon2sun_J2000, r_earth2sun_J2000);

  double r_moon2sc_J2000[3];
  v_add(r_moon2sc_J2000, r_moon2earth_J2000, r_i2cg_INRTL);
  
  double r_moon2sc_J2000_normalized[3];
  v_norm(r_moon2sc_J2000_normalized, r_moon2sc_J2000);
  v_dot(&cos_angle_minus_sun_to_sc, r_sun2moon_J2000_normalized, r_moon2sc_J2000_normalized);

  angle_minus_sun_to_sc = acos(cos_angle_minus_sun_to_sc);

  /* Calculate if the SC is in the umbra or penumbra */
  if (cos_angle_minus_sun_to_sc > 0){

    v_mag(&mag_r_moon2sc_J2000, r_moon2sc_J2000);
    sc_horiz = mag_r_moon2sc_J2000 * cos( angle_minus_sun_to_sc );
    sc_vert =  mag_r_moon2sc_J2000 * sin( angle_minus_sun_to_sc );
    pen_vert = PARAMS->MOON.radius + tan( alpha_pen ) * sc_horiz;
  
    strcpy(shadow, "none");
    if ( sc_vert < pen_vert ){
      strcpy(shadow, "penumbra");
      umb_vert = PARAMS->MOON.radius - tan( alpha_umb ) * sc_horiz;
      if ( sc_vert < umb_vert ){
	strcpy(shadow, "umbra");
      }
    }
    else{
      strcpy(shadow, "light");
    }
 
  }

  else{
    strcpy(shadow, "light");
  }

  return 0;
}



/////////////////////////////////////////////////////////////////////////////////////////
//
//  Name:           load_params
//  Purpose:        Loads params. Ellipsoid based on WGS84
//  Assumptions:    None.
//  References      Various
//
//  Change Log:
//      |   Developer   |       Date    |   SCR     |   Notes
//      | --------------|---------------|-----------|-------------------------------
//      | J. Getchius   | 05/20/2015    |   ---     | Initial Implementation
//      | C. Bussy-Virat| 10/14/2015    |   ---     | Change the inputs
//
/////////////////////////////////////////////////////////////////////////////////////////
//newstructure
//int load_params( PARAMS_T *PARAMS, char main_directory_location[256], int iDebugLevel, char earth_fixed_frame[100],   double use_ap_hist, int iProc) {
int load_params( PARAMS_T *PARAMS,  int iDebugLevel, char earth_fixed_frame[100],   double use_ap_hist, int iProc, char path_to_spice[256]) {
//newstructure

  int ii;

  PARAMS->EARTH.flattening    = 1/298.257223560;
  PARAMS->EARTH.radius        = 6378.137;
  PARAMS->EARTH.GRAVITY.mu    = 398600.4418;
  PARAMS->EARTH.GRAVITY.j2    = 1.081874e-3;
  strcpy(PARAMS->EARTH.earth_fixed_frame, earth_fixed_frame);
 
  PARAMS->MOON.flattening     = 0.0012;//https://nssdc.gsfc.nasa.gov/planetary/factsheet/moonfact.html
  PARAMS->MOON.radius         = 1737.0;//https://nssdc.gsfc.nasa.gov/planetary/factsheet/moonfact.html
  PARAMS->MOON.GRAVITY.mu     = 4.902801076000e+003; // (value from STK) 
  PARAMS->MOON.GRAVITY.j2     = 202.7e-6;//https://nssdc.gsfc.nasa.gov/planetary/factsheet/moonfact.html
  strcpy(PARAMS->MOON.earth_fixed_frame, "");

  PARAMS->SUN.flattening      = 0.0;
  PARAMS->SUN.radius          = 696300.0;
  PARAMS->SUN.GRAVITY.mu      = 1.327122000000E+011; // in km^3 / s^2 (value from STK)
  PARAMS->SUN.GRAVITY.j2      = 0.0;
  strcpy(PARAMS->SUN.earth_fixed_frame, "");
    
  if (iDebugLevel >= 2){
        if (iProc == 0) printf("--- (load_params) Loading gravity...\n");
  }
  //newstructure
  load_gravity(  &PARAMS->EARTH.GRAVITY,  path_to_spice );
  //  load_gravity(  &PARAMS->EARTH.GRAVITY, main_directory_location );
  //newstructure
  PARAMS->EARTH.GRAVITY.radius = 6378.137;
   

  // Flags for NRLMSIS-00e
  PARAMS->ATMOSPHERE.flags.switches[0]=0;
  for (ii=1;ii<24;ii++){
        PARAMS->ATMOSPHERE.flags.switches[ii]=1;
  }
    // this allow to use msis with historical ap rather than daily ap
  if (use_ap_hist == 1){
      PARAMS->ATMOSPHERE.flags.switches[9]=-1;
  }
  // !!!!!!!!!!!!!!!!!! TO ERASE AND UNCOMMENT BLOCK ABOVE
  /* FILE *fp_switch = NULL; */
  /* char *line = NULL; */
  /* size_t len = 0; */
  /* fp_switch = fopen("./input/switch_for_msis.txt","r"); */
  /* for (ii=1;ii<24;ii++){ */
  /*   getline(&line,&len,fp_switch); */
  /*   sscanf(line, "%d", &PARAMS->ATMOSPHERE.flags.switches[ii]); */
  /*      printf("[%d]: %d\n",ii, PARAMS->ATMOSPHERE.flags.switches[ii]); */
  /* } */
  /* fclose(fp_switch); */
  // !!!!!!!!!!!END OF TO ERASE AND UNCOMMENT BLOCK ABOVE


  return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////
//
//  Name:           test_print
//  Purpose:        Very useless function thatpr ints something to see if the code runs fine up to this line
//  Assumptions:    None
//  References      None
//
//  Change Log:
//      |   Developer   |       Date    |   SCR     |   Notes
//      | --------------|---------------|-----------|-------------------------------
//      | C. Bussy-Virat| 06/08/2015    |   ---     | Initial implementation
//
/////////////////////////////////////////////////////////////////////////////////////////
int test_print( char to_print[1]  )

{

  char to_print_more[10];
  int i;
  strcpy(to_print_more, "");
  //  for (i = 0; i < 10; i++ ){
      
    strcat(to_print_more, to_print);
    //  }
  printf("**********\n");
  for (i = 0; i < 3; i++){
    printf("%s\n", to_print_more);
  }
  printf("**********\n");
  return 0;
}



int test_print_iproc( int iProc, char to_print[1] )

{
  char to_print_more[20];
  int i;
  strcpy(to_print_more, "");
  for (i = 0; i < 10; i++ ){
    strcat(to_print_more, to_print);
  }    
  char iproc_str[5];
  sprintf(iproc_str, "%d", iProc);
  strcat(to_print_more, " ");
  strcat(to_print_more, iproc_str);
  printf("**********\n");
  for (i = 0; i < 3; i++){
    printf("%s\n", to_print_more);
  }
  printf("**********\n");
  return 0;
}



/////////////////////////////////////////////////////////////////////////////////////////
//
//  Name:           print_test
//  Purpose:        Prints something to see if the code runs fine up to this line
//  Assumptions:    None
//  References      None
//
//  Change Log:
//      |   Developer   |       Date    |   SCR     |   Notes
//      | --------------|---------------|-----------|-------------------------------
//      | C. Bussy-Virat| 12/09/2015    |   ---     | Initial implementation
//
/////////////////////////////////////////////////////////////////////////////////////////
int print_test(  )

{
  printf("\n****************** TEST ******************\n****************** TEST ******************\n\n");
  return 0;
}

int set_attitude( ATTITUDE_T attitude, int index_in_attitude_interpolated, OPTIONS_T *OPTIONS, int file_is_quaternion ){
  // this function sets the attitude in case no ensemble at all is run on the attitude


  /* Declarations */
  //  printf("%d\n", index_in_attitude_interpolated);
	  //	 printf("%d %f %f\n", index_in_attitude_interpolated, OPTIONS->pitch[index_in_attitude_interpolated], attitude.pitch[index_in_attitude_interpolated]);
  if (file_is_quaternion == 0){
  attitude.pitch[index_in_attitude_interpolated] = OPTIONS->pitch[index_in_attitude_interpolated];
  attitude.roll[index_in_attitude_interpolated] = OPTIONS->roll[index_in_attitude_interpolated];
  attitude.yaw[index_in_attitude_interpolated] = OPTIONS->yaw[index_in_attitude_interpolated];
  attitude.order_pitch[index_in_attitude_interpolated] = OPTIONS->order_pitch[index_in_attitude_interpolated];
  attitude.order_roll[index_in_attitude_interpolated] = OPTIONS->order_roll[index_in_attitude_interpolated];
  attitude.order_yaw[index_in_attitude_interpolated] = OPTIONS->order_yaw[index_in_attitude_interpolated];
  }
  else{
    attitude.quaternion[index_in_attitude_interpolated][0] = OPTIONS->quaternion[index_in_attitude_interpolated][0];
    attitude.quaternion[index_in_attitude_interpolated][1] = OPTIONS->quaternion[index_in_attitude_interpolated][1];
    attitude.quaternion[index_in_attitude_interpolated][2] = OPTIONS->quaternion[index_in_attitude_interpolated][2];
    attitude.quaternion[index_in_attitude_interpolated][3] = OPTIONS->quaternion[index_in_attitude_interpolated][3];
  }

  /* Algorithm */

  return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////
//
//  Name:           gitm_density
//  Purpose:        Calculate the density at the position of the satellite using the GITM thermospheric model 
//  Assumptions:    The epoch time has to be newer than 2000. Other assumptions are commented in the code
//  References      Various
//
//  Change Log:
//      |   Developer   |       Date    |   SCR     |   Notes
//      | --------------|---------------|-----------|-------------------------------
//      | C. Bussy-Virat| 01/24/2016    |   ---     | Initial implementation
//
/////////////////////////////////////////////////////////////////////////////////////////
int gitm_density( double *density, double et, double altitude, double latitude, double longitude, PARAMS_T *PARAMS) {

  // Declarations
  int pos_for_lon_gitm_right_before, iDataLength_gitm_right_before, iHeaderLength_gitm_right_before;
  int time_gitm_right_before_temp[7];
  int j,k;
  FILE *gitm_file_gitm_right_before;

  int i;
  int pos_for_lon_gitm_right_after, iDataLength_gitm_right_after, iHeaderLength_gitm_right_after;
  int time_gitm_right_after_temp[7];


  FILE *gitm_file_gitm_right_after;

  char name_gitm_file_right_before_epoch_of_sc[256], name_gitm_file_right_after_epoch_of_sc[256];
  char time_sc[256];
  double et_gitm_file;
  double delta_et, delta_et_right_before, delta_et_right_after;
  int found_2_closest_GITM_file_date;
  int need_to_read_GITM_before, need_to_read_GITM_after;
 
  et2utc_c(et, "ISOC" ,0 ,255 , time_sc);

  // The goal of this block is to find the 2 GITM files that have dates around the epoch of the sc
  if (PARAMS->ATMOSPHERE.is_first_step_of_run == 1){ //if it is the first step of the run then find the two cloest GITM file dates to the epoch start tie of the sc
    i = 0;
    found_2_closest_GITM_file_date = 0;
    while ( (i < PARAMS->ATMOSPHERE.nb_gitm_file) && (found_2_closest_GITM_file_date == 0) ){
      et_gitm_file = PARAMS->ATMOSPHERE.array_gitm_date[i][0];
      // Find the 2 GITM files that have dates around the epoch of the sc
      delta_et = et_gitm_file - et;
      if (delta_et <= 0){ // the date of the GITM file is older than the epoch of the sc
	found_2_closest_GITM_file_date = 0;
      }
      else {//if (delta_et > 0){
	if (i > 0){
	  PARAMS->ATMOSPHERE.gitm_index_lower = i-1;
	  PARAMS->ATMOSPHERE.gitm_index_higher = i;
	  found_2_closest_GITM_file_date = 1;
	}
	else{
	  printf("The oldest GITM file has to be older than epoch start time of the satellite. The program will stop.\n");
	  exit(0);
	}
      }
      i = i+1;
    }

    need_to_read_GITM_before = 1;
    need_to_read_GITM_after = 1;
  } // end of if first step of run
  else{ //if it is not the first step of run then just look if the epoch time of the sc got newer than the subsequent GITM file date
    if(et >= PARAMS->ATMOSPHERE.array_gitm_date[PARAMS->ATMOSPHERE.gitm_index_higher][0]){ // if the epoch got newer than the time of the GITM subsequent file then the new subsequent GITM file is the next file in the array PARAMS->ATMOSPHERE.array_gitm_file since this latter is arranged in ascending order of times. Then, read the new file
      PARAMS->ATMOSPHERE.gitm_index_lower = PARAMS->ATMOSPHERE.gitm_index_higher;
      PARAMS->ATMOSPHERE.gitm_index_higher = PARAMS->ATMOSPHERE.gitm_index_higher + 1; // the GITM files are arranged in the ascending order

      need_to_read_GITM_before = 0;
      need_to_read_GITM_after = 1;
    }
    else{ // otherwise no need to do anything: we already read the 2 GITM files
      need_to_read_GITM_before = 0;
      need_to_read_GITM_after = 0;
    }
  } // end of if it is not the first step of run 
  // end of the goal of this block is to find the 2 GITM files that have dates around the epoch of the sc


  strcpy(name_gitm_file_right_before_epoch_of_sc, PARAMS->ATMOSPHERE.array_gitm_file[PARAMS->ATMOSPHERE.gitm_index_lower]);
  strcpy(name_gitm_file_right_after_epoch_of_sc, PARAMS->ATMOSPHERE.array_gitm_file[PARAMS->ATMOSPHERE.gitm_index_higher]);
  delta_et_right_before = PARAMS->ATMOSPHERE.array_gitm_date[PARAMS->ATMOSPHERE.gitm_index_lower][0] - et; 
  delta_et_right_after = PARAMS->ATMOSPHERE.array_gitm_date[PARAMS->ATMOSPHERE.gitm_index_higher][0] - et;
  
  if (need_to_read_GITM_before == 1){ // read the GITM file that has a date right before the epoch date of the sc. This happens only at the first step of the run

    /*   /\******************************************************************\/ */
    /*   /\*************************** READ GITM RIGHT BEFORE ***************\/ */
    /*   /\******************************************************************\/ */

  
    gitm_file_gitm_right_before = fopen(name_gitm_file_right_before_epoch_of_sc,"rb");


    // TIME
    fseek(gitm_file_gitm_right_before,48+ 4+PARAMS->ATMOSPHERE.nVars_gitm*(4+4+40), SEEK_CUR	 );
    fread(&time_gitm_right_before_temp,sizeof(time_gitm_right_before_temp),1,gitm_file_gitm_right_before);
    /* printf("\n\n"); */
    /*  for (i = 0; i < 7; i++){ */
    /*     printf("time: %d\n", time_gitm_right_before_temp[i]); */
    /*    } */

    // SKIP VARIABLES BEFORE THE DENSITY
    iHeaderLength_gitm_right_before = 8L + 4+4 +	3*4 + 4+4 + 4 + 4+4 + PARAMS->ATMOSPHERE.nVars_gitm*40 + PARAMS->ATMOSPHERE.nVars_gitm*(4+4) +  7*4 + 4+4;
    iDataLength_gitm_right_before = PARAMS->ATMOSPHERE.nLons_gitm*PARAMS->ATMOSPHERE.nLats_gitm*PARAMS->ATMOSPHERE.nAlts_gitm*8 + 4+4;
    pos_for_lon_gitm_right_before = iHeaderLength_gitm_right_before + 3*iDataLength_gitm_right_before ;// density is the 4th variable in the GITM file
    fseek(gitm_file_gitm_right_before, pos_for_lon_gitm_right_before, SEEK_SET);


    // READ THE DENSITY AT THE GIVEN LON/LAT/ALT
    fseek(gitm_file_gitm_right_before, 4, SEEK_CUR );
    for (k = 0; k < PARAMS->ATMOSPHERE.nAlts_gitm; k++){
      for (j = 0; j < PARAMS->ATMOSPHERE.nLats_gitm; j++){
	for (i = 0; i < PARAMS->ATMOSPHERE.nLons_gitm; i++){
	  fread(&PARAMS->ATMOSPHERE.density_gitm_right_before[i][j][k], sizeof(PARAMS->ATMOSPHERE.density_gitm_right_before[i][j][k]), 1, gitm_file_gitm_right_before);
	}
      }
    }

    /******************************************************************/
    /******************** END OF READ GITM RIGHT BEFORE ***************/
    /******************************************************************/

    fclose(gitm_file_gitm_right_before);
  }  // end of read the GITM file that has a date right before the epoch date of the sc. This happens only at the first step of the run 



  if (need_to_read_GITM_after == 1){ // this happens at the first step of the run or when a new subsequent GITM file needs to be found (see previous comments above)

    if (PARAMS->ATMOSPHERE.is_first_step_of_run == 0){

      for (i = 0; i < 7; i++){
	time_gitm_right_before_temp[i] = time_gitm_right_after_temp[i];
      }

      for (k = 0; k < PARAMS->ATMOSPHERE.nAlts_gitm; k++){
	for (j = 0; j < PARAMS->ATMOSPHERE.nLats_gitm; j++){
	  for (i = 0; i < PARAMS->ATMOSPHERE.nLons_gitm; i++){
	    PARAMS->ATMOSPHERE.density_gitm_right_before[i][j][k] = PARAMS->ATMOSPHERE.density_gitm_right_after[i][j][k];
	  }
	}
      }

    }


    /******************************************************************/
    /**************** NOW READ THE NEW GITM RIGHT AFTER ***************/
    /******************************************************************/
    gitm_file_gitm_right_after = fopen(name_gitm_file_right_after_epoch_of_sc,"rb");

    // TIME
    fseek(gitm_file_gitm_right_after,48+ 4+PARAMS->ATMOSPHERE.nVars_gitm*(4+4+40), SEEK_CUR	 );
    fread(&time_gitm_right_after_temp,sizeof(time_gitm_right_after_temp),1,gitm_file_gitm_right_after);
    /* printf("\n\n"); */
    /*  for (i = 0; i < 7; i++){ */
    /*     printf("time: %d\n", time_gitm_right_after_temp[i]); */
    /*    } */

    // SKIP VARIABLES AFTER THE DENSITY
    iHeaderLength_gitm_right_after = 8L + 4+4 +	3*4 + 4+4 + 4 + 4+4 + PARAMS->ATMOSPHERE.nVars_gitm*40 + PARAMS->ATMOSPHERE.nVars_gitm*(4+4) +  7*4 + 4+4;
    iDataLength_gitm_right_after = PARAMS->ATMOSPHERE.nLons_gitm*PARAMS->ATMOSPHERE.nLats_gitm*PARAMS->ATMOSPHERE.nAlts_gitm*8 + 4+4;
    pos_for_lon_gitm_right_after = iHeaderLength_gitm_right_after + 3*iDataLength_gitm_right_after ;// density is the 4th variable in the GITM file
    fseek(gitm_file_gitm_right_after, pos_for_lon_gitm_right_after, SEEK_SET);


    // READ THE DENSITY AT THE GIVEN LON/LAT/ALT
    fseek(gitm_file_gitm_right_after, 4, SEEK_CUR );
    for (k = 0; k < PARAMS->ATMOSPHERE.nAlts_gitm; k++){
      for (j = 0; j < PARAMS->ATMOSPHERE.nLats_gitm; j++){
	for (i = 0; i < PARAMS->ATMOSPHERE.nLons_gitm; i++){
	  fread(&PARAMS->ATMOSPHERE.density_gitm_right_after[i][j][k], sizeof(PARAMS->ATMOSPHERE.density_gitm_right_after[i][j][k]), 1, gitm_file_gitm_right_after);

	}
      }
    }

    /******************************************************************/
    /******************** END OF READ GITM RIGHT AFTER ***************/
    /******************************************************************/
    fclose(gitm_file_gitm_right_after);
  }

  /* printf("\n%s | %s | %s\n", name_gitm_file_right_before_epoch_of_sc, time_sc, name_gitm_file_right_after_epoch_of_sc);   */
  /* printf("longitude = %f \n", PARAMS->ATMOSPHERE.longitude_gitm[11][40][21]); */
  /* printf("latitude = %f \n", PARAMS->ATMOSPHERE.latitude_gitm[11][40][21]); */
  /* printf("altitude = %f \n", PARAMS->ATMOSPHERE.altitude_gitm[11][40][21]); */
  /* printf("density = %e | %e\n", PARAMS->ATMOSPHERE.density_gitm_right_before[11][40][21], PARAMS->ATMOSPHERE.density_gitm_right_after[11][40][21]); */



  /*   /\******************************************************************\/ */
  /*   /\********** INTERPOLATION LON/LAT/ALT WITH GITM RIGHT BEFORE ******\/ */
  /*   /\******************************************************************\/ */

  /*      // find the 2 closest longitudes in GITM file to the sc longitude */
  /*      double delta_longitude_GITM_file_right_before, delta_longitude_right_lower_sc_lon_in_GITM_file_right_before = -10*M_PI, delta_longitude_right_higher_sc_lon_in_GITM_file_right_before = 10*M_PI; */
  /*      double longitude_right_lower_sc_lon_in_GITM_file_right_before = 0, longitude_right_higher_sc_lon_in_GITM_file_right_before = 0; */
  /*      int index_longitude_right_lower_sc_lon_in_GITM_file_right_before = -1, index_longitude_right_higher_sc_lon_in_GITM_file_right_before = -1; */
  /*      int index_latitude_right_lower_sc_lat_in_GITM_file_right_before = -1, index_latitude_right_higher_sc_lat_in_GITM_file_right_before = -1; */
  /*      int index_altitude_right_lower_sc_alt_in_GITM_file_right_before = -1, index_altitude_right_higher_sc_alt_in_GITM_file_right_before = -1; */
  /*      for (i = 0; i < PARAMS->ATMOSPHERE.nLons_gitm; i++){ */
  /*        delta_longitude_GITM_file_right_before = PARAMS->ATMOSPHERE.longitude_gitm[i][0][0] - longitude; */
  /*        if (delta_longitude_GITM_file_right_before < 0){ // GITM longitude is smaller than sc longitude */
  /* 	 if (delta_longitude_GITM_file_right_before > delta_longitude_right_lower_sc_lon_in_GITM_file_right_before){ */
  /* 	   delta_longitude_right_lower_sc_lon_in_GITM_file_right_before = delta_longitude_GITM_file_right_before; */
  /* 	   longitude_right_lower_sc_lon_in_GITM_file_right_before = PARAMS->ATMOSPHERE.longitude_gitm[i][0][0]; */
  /* 	   index_longitude_right_lower_sc_lon_in_GITM_file_right_before = i; */
  /* 	 } */
  /*        } */
  /*        if (delta_longitude_GITM_file_right_before > 0){ // GITM longitude is greater than sc longitude */
  /* 	 if (delta_longitude_GITM_file_right_before < delta_longitude_right_higher_sc_lon_in_GITM_file_right_before){ */
  /* 	   delta_longitude_right_higher_sc_lon_in_GITM_file_right_before = delta_longitude_GITM_file_right_before; */
  /* 	   longitude_right_higher_sc_lon_in_GITM_file_right_before = PARAMS->ATMOSPHERE.longitude_gitm[i][0][0]; */
  /* 	   index_longitude_right_higher_sc_lon_in_GITM_file_right_before = i; */
  /* 	 } */
  /*        } */
  /*      } */


  /* // find the 2 closest latitudes in GITM file to the sc latitude */
  /*      double delta_latitude_GITM_file_right_before, delta_latitude_right_lower_sc_lat_in_GITM_file_right_before = -10*M_PI, delta_latitude_right_higher_sc_lat_in_GITM_file_right_before = 10*M_PI; */
  /*      double latitude_right_lower_sc_lat_in_GITM_file_right_before = 0, latitude_right_higher_sc_lat_in_GITM_file_right_before = 0; */
  /*      for (i = 0; i < PARAMS->ATMOSPHERE.nLats_gitm; i++){ */
  /*        delta_latitude_GITM_file_right_before = PARAMS->ATMOSPHERE.latitude_gitm[0][i][0] - latitude; */
  /*        if (delta_latitude_GITM_file_right_before < 0){ // GITM latitude is smaller than sc latitude */
  /* 	 if (delta_latitude_GITM_file_right_before > delta_latitude_right_lower_sc_lat_in_GITM_file_right_before){ */
  /* 	   delta_latitude_right_lower_sc_lat_in_GITM_file_right_before = delta_latitude_GITM_file_right_before; */
  /* 	   latitude_right_lower_sc_lat_in_GITM_file_right_before = PARAMS->ATMOSPHERE.latitude_gitm[0][i][0]; */
  /* 	   index_latitude_right_lower_sc_lat_in_GITM_file_right_before = i; */
  /* 	 } */
  /*        } */
  /*        if (delta_latitude_GITM_file_right_before > 0){ // GITM latitude is greater than sc latitude */
  /* 	 if (delta_latitude_GITM_file_right_before < delta_latitude_right_higher_sc_lat_in_GITM_file_right_before){ */
  /* 	   delta_latitude_right_higher_sc_lat_in_GITM_file_right_before = delta_latitude_GITM_file_right_before; */
  /* 	   latitude_right_higher_sc_lat_in_GITM_file_right_before = PARAMS->ATMOSPHERE.latitude_gitm[0][i][0]; */
  /* 	   index_latitude_right_higher_sc_lat_in_GITM_file_right_before = i; */
  /* 	 } */
  /*        } */
  /*      } */
  /*      /\* print_test(); *\/ */
  /*      /\* printf("%f | %f | %f\n", latitude_right_lower_sc_lat_in_GITM_file_right_before, latitude, latitude_right_higher_sc_lat_in_GITM_file_right_before); *\/ */
       


  // find the 2 closest altitudes in GITM file to the sc altitude
  double delta_altitude_GITM;
  int index_altitude_lower;
  int have_found_index_altitude_lower;
  i = PARAMS->ATMOSPHERE.index_altitude_right_below_perigee;  // index in altitude_gitm of the altitude that is right below the perigee altitude (calculated at the initialization). Note: if the duration of the run is long (so that the sc looses a good amount of altitude, so like 6 months) then this index is set to 0 so that we go over all the altitudes and not only the ones that start below the perigee calcualted at the initialization.
  have_found_index_altitude_lower = 0;
  while ( (i < PARAMS->ATMOSPHERE.nAlts_gitm) && (have_found_index_altitude_lower == 0) ) {

    delta_altitude_GITM = PARAMS->ATMOSPHERE.altitude_gitm[0][0][i] - altitude;
    if (delta_altitude_GITM >= 0){ // GITM altitude is smaller than sc altitude	 
      index_altitude_lower = i-1;
      have_found_index_altitude_lower = 1;
    }
    i = i + 1;
  }
  if (have_found_index_altitude_lower == 0){
    printf("The altitude of the satellite is not in the range of altitudes covered by the GITM files. The programs will stop.\n");
    exit(0);
  }


  int index_longitude_lower;
  index_longitude_lower = rint( longitude / (PARAMS->ATMOSPHERE.longitude_gitm[1][0][0] - PARAMS->ATMOSPHERE.longitude_gitm[0][0][0]) ) + 1;
  int index_latitude_lower;
  index_latitude_lower = rint( ( latitude + M_PI/2) / (PARAMS->ATMOSPHERE.latitude_gitm[0][1][0] - PARAMS->ATMOSPHERE.latitude_gitm[0][0][0]) ) + 1;
  double dist_lon, dist_lat, dist_alt;
  dist_lon = ( longitude - PARAMS->ATMOSPHERE.longitude_gitm[index_longitude_lower][0][0] ) / ( PARAMS->ATMOSPHERE.longitude_gitm[index_longitude_lower+1][0][0] - PARAMS->ATMOSPHERE.longitude_gitm[index_longitude_lower][0][0] );
  dist_lat = ( latitude - PARAMS->ATMOSPHERE.latitude_gitm[0][index_latitude_lower][0] ) / ( PARAMS->ATMOSPHERE.latitude_gitm[0][index_latitude_lower+1][0] - PARAMS->ATMOSPHERE.latitude_gitm[0][index_latitude_lower][0] );
  dist_alt = ( altitude - PARAMS->ATMOSPHERE.altitude_gitm[0][0][index_altitude_lower] ) / ( PARAMS->ATMOSPHERE.altitude_gitm[0][0][index_altitude_lower+1] - PARAMS->ATMOSPHERE.altitude_gitm[0][0][index_altitude_lower] );

     
  double density_lat_lon_lower_plane_gitm_right_before;
  density_lat_lon_lower_plane_gitm_right_before = (1-dist_lon) * (1-dist_lat) * PARAMS->ATMOSPHERE.density_gitm_right_before[index_longitude_lower][index_latitude_lower][index_altitude_lower] +
    dist_lon * (1-dist_lat) * PARAMS->ATMOSPHERE.density_gitm_right_before[index_longitude_lower+1][index_latitude_lower][index_altitude_lower] +
    dist_lon * dist_lat * PARAMS->ATMOSPHERE.density_gitm_right_before[index_longitude_lower+1][index_latitude_lower+1][index_altitude_lower] +
    (1-dist_lon) * dist_lat * PARAMS->ATMOSPHERE.density_gitm_right_before[index_longitude_lower][index_latitude_lower+1][index_altitude_lower];

  double density_lat_lon_higher_plane_gitm_right_before;
  density_lat_lon_higher_plane_gitm_right_before = (1-dist_lon) * (1-dist_lat) * PARAMS->ATMOSPHERE.density_gitm_right_before[index_longitude_lower][index_latitude_lower][index_altitude_lower+1] +
    dist_lon * (1-dist_lat) * PARAMS->ATMOSPHERE.density_gitm_right_before[index_longitude_lower+1][index_latitude_lower][index_altitude_lower+1] +
    dist_lon * dist_lat * PARAMS->ATMOSPHERE.density_gitm_right_before[index_longitude_lower+1][index_latitude_lower+1][index_altitude_lower+1] +
    (1-dist_lon) * dist_lat * PARAMS->ATMOSPHERE.density_gitm_right_before[index_longitude_lower][index_latitude_lower+1][index_altitude_lower+1];

  double density_interpolated_right_before;
  double a_right_before, b_right_before;
  a_right_before = ( log(density_lat_lon_higher_plane_gitm_right_before) - log(density_lat_lon_lower_plane_gitm_right_before) ) / ( PARAMS->ATMOSPHERE.altitude_gitm[0][0][index_altitude_lower+1] - PARAMS->ATMOSPHERE.altitude_gitm[0][0][index_altitude_lower] );
  b_right_before = log(density_lat_lon_lower_plane_gitm_right_before) - a_right_before*PARAMS->ATMOSPHERE.altitude_gitm[0][0][index_altitude_lower];
  density_interpolated_right_before = exp(a_right_before * altitude + b_right_before);
  //     printf("\n%e | %e | %e\n", density_lat_lon_lower_plane_gitm_right_before, density_interpolated_right_before, density_lat_lon_higher_plane_gitm_right_before);


  double density_lat_lon_lower_plane_gitm_right_after;
  density_lat_lon_lower_plane_gitm_right_after = (1-dist_lon) * (1-dist_lat) * PARAMS->ATMOSPHERE.density_gitm_right_after[index_longitude_lower][index_latitude_lower][index_altitude_lower] +
    dist_lon * (1-dist_lat) * PARAMS->ATMOSPHERE.density_gitm_right_after[index_longitude_lower+1][index_latitude_lower][index_altitude_lower] +
    dist_lon * dist_lat * PARAMS->ATMOSPHERE.density_gitm_right_after[index_longitude_lower+1][index_latitude_lower+1][index_altitude_lower] +
    (1-dist_lon) * dist_lat * PARAMS->ATMOSPHERE.density_gitm_right_after[index_longitude_lower][index_latitude_lower+1][index_altitude_lower];

  double density_lat_lon_higher_plane_gitm_right_after;
  density_lat_lon_higher_plane_gitm_right_after = (1-dist_lon) * (1-dist_lat) * PARAMS->ATMOSPHERE.density_gitm_right_after[index_longitude_lower][index_latitude_lower][index_altitude_lower+1] +
    dist_lon * (1-dist_lat) * PARAMS->ATMOSPHERE.density_gitm_right_after[index_longitude_lower+1][index_latitude_lower][index_altitude_lower+1] +
    dist_lon * dist_lat * PARAMS->ATMOSPHERE.density_gitm_right_after[index_longitude_lower+1][index_latitude_lower+1][index_altitude_lower+1] +
    (1-dist_lon) * dist_lat * PARAMS->ATMOSPHERE.density_gitm_right_after[index_longitude_lower][index_latitude_lower+1][index_altitude_lower+1];

  /* printf("\n%e | %e\n", density_lat_lon_lower_plane_gitm_right_after,density_lat_lon_higher_plane_gitm_right_after); */
  /* printf("%e | %e\n", PARAMS->ATMOSPHERE.density_gitm_right_after[index_longitude_lower][index_latitude_lower][index_altitude_lower],PARAMS->ATMOSPHERE.density_gitm_right_after[index_longitude_lower][index_latitude_lower][index_altitude_lower+1] ); */
  /* printf("%e | %e\n", PARAMS->ATMOSPHERE.density_gitm_right_after[index_longitude_lower+1][index_latitude_lower][index_altitude_lower],PARAMS->ATMOSPHERE.density_gitm_right_after[index_longitude_lower+1][index_latitude_lower][index_altitude_lower+1] ); */
  /* printf("%e | %e\n", PARAMS->ATMOSPHERE.density_gitm_right_after[index_longitude_lower][index_latitude_lower+1][index_altitude_lower],PARAMS->ATMOSPHERE.density_gitm_right_after[index_longitude_lower][index_latitude_lower+1][index_altitude_lower+1] ); */
  /* printf("%e | %e\n", PARAMS->ATMOSPHERE.density_gitm_right_after[index_longitude_lower+1][index_latitude_lower+1][index_altitude_lower],PARAMS->ATMOSPHERE.density_gitm_right_after[index_longitude_lower+1][index_latitude_lower+1][index_altitude_lower+1] ); */


  double density_interpolated_right_after;
  double a_right_after, b_right_after;
  a_right_after = ( log(density_lat_lon_higher_plane_gitm_right_after) - log(density_lat_lon_lower_plane_gitm_right_after) ) / ( PARAMS->ATMOSPHERE.altitude_gitm[0][0][index_altitude_lower+1] - PARAMS->ATMOSPHERE.altitude_gitm[0][0][index_altitude_lower] );
  b_right_after = log(density_lat_lon_lower_plane_gitm_right_after) - a_right_after*PARAMS->ATMOSPHERE.altitude_gitm[0][0][index_altitude_lower];
  density_interpolated_right_after = exp(a_right_after * altitude + b_right_after);
  //     printf("%e | %e | %e\n", density_lat_lon_lower_plane_gitm_right_after, density_interpolated_right_after, density_lat_lon_higher_plane_gitm_right_after);


  /*   /\******************************************************************\/ */
  /*   /\***************** INTERPOLATION IN TIME **************************\/ */
  /*   /\******************************************************************\/ */
  double a_time;
  double b_time;
  a_time = ( density_interpolated_right_after - density_interpolated_right_before ) / (PARAMS->ATMOSPHERE.array_gitm_date[PARAMS->ATMOSPHERE.gitm_index_higher][0] - PARAMS->ATMOSPHERE.array_gitm_date[PARAMS->ATMOSPHERE.gitm_index_lower][0]) ;
  b_time = density_interpolated_right_before - a_time * PARAMS->ATMOSPHERE.array_gitm_date[PARAMS->ATMOSPHERE.gitm_index_lower][0];
  *density = a_time * et + b_time;

  /* printf("\n%s | %s | %s\n",name_gitm_file_right_before_epoch_of_sc, time_sc, name_gitm_file_right_after_epoch_of_sc ); */
  /*     printf("%e | %e | %e\n", density_interpolated_right_before, *density, density_interpolated_right_after); */


  PARAMS->ATMOSPHERE.is_first_step_of_run = 0;
  return 0;

  /*   // Free memory */
  /*   for (i = 0; i < PARAMS->ATMOSPHERE.nLons_gitm; i++){ */
  /*     for (j = 0; j < PARAMS->ATMOSPHERE.nLats_gitm; j++){ */
  /*       free(PARAMS->ATMOSPHERE.longitude_gitm[i][j]); */
  /*       } */
  /*       free(PARAMS->ATMOSPHERE.longitude_gitm[i]); */
  /*   } */
  /*   free(PARAMS->ATMOSPHERE.longitude_gitm); */
  /*   for (i = 0; i < PARAMS->ATMOSPHERE.nLons_gitm; i++){ */
  /*     for (j = 0; j < PARAMS->ATMOSPHERE.nLats_gitm; j++){ */
  /*       free(PARAMS->ATMOSPHERE.latitude_gitm[i][j]); */
  /*       } */
  /*   free(PARAMS->ATMOSPHERE.latitude_gitm[i]); */
  /*   } */
  /*   free(PARAMS->ATMOSPHERE.latitude_gitm ); */
  /*   for (i = 0; i < PARAMS->ATMOSPHERE.nLons_gitm; i++){ */
  /*     for (j = 0; j < PARAMS->ATMOSPHERE.nLats_gitm; j++){ */
  /*       free(PARAMS->ATMOSPHERE.altitude_gitm[i][j]); */
  /*       } */
  /*   free(PARAMS->ATMOSPHERE.altitude_gitm[i]); */
  /*   } */
  /*   free(PARAMS->ATMOSPHERE.altitude_gitm           ); */
  /*   for (i = 0; i < PARAMS->ATMOSPHERE.nLons_gitm; i++){ */
  /*     for (j = 0; j < PARAMS->ATMOSPHERE.nLats_gitm  ; j++){ */
  /*       free(PARAMS->ATMOSPHERE.density_gitm_right_before[i][j]); */
  /*       } */
  /*   free(PARAMS->ATMOSPHERE.density_gitm_right_before[i]); */
  /*   } */
  /*   free(PARAMS->ATMOSPHERE.density_gitm_right_before); */

  /*   for (i = 0; i < PARAMS->ATMOSPHERE.nLons_gitm; i++){ */
  /*     for (j = 0; j < PARAMS->ATMOSPHERE.nLats_gitm; j++){ */
  /*       free(PARAMS->ATMOSPHERE.longitude_gitm[i][j]); */
  /*       } */
  /*       free(PARAMS->ATMOSPHERE.longitude_gitm[i]); */
  /*   } */
  /*   free(PARAMS->ATMOSPHERE.longitude_gitm              ); */
  /*   for (i = 0; i < PARAMS->ATMOSPHERE.nLons_gitm; i++){ */
  /*     for (j = 0; j < PARAMS->ATMOSPHERE.nLats_gitm; j++){ */
  /*       free(PARAMS->ATMOSPHERE.latitude_gitm[i][j]); */
  /*       } */
  /*   free(PARAMS->ATMOSPHERE.latitude_gitm[i]); */
  /*   } */
  /*   free(PARAMS->ATMOSPHERE.latitude_gitm       ); */
  /*   for (i = 0; i < PARAMS->ATMOSPHERE.nLons_gitm; i++){ */
  /*     for (j = 0; j < PARAMS->ATMOSPHERE.nLats_gitm; j++){ */
  /*       free(PARAMS->ATMOSPHERE.altitude_gitm[i][j]); */
  /*       } */
  /*   free(PARAMS->ATMOSPHERE.altitude_gitm[i]); */
  /*   } */
  /*   free(PARAMS->ATMOSPHERE.altitude_gitm       ); */
  /*   for (i = 0; i < PARAMS->ATMOSPHERE.nLons_gitm ; i++){ */
  /*     for (j = 0; j < PARAMS->ATMOSPHERE.nLats_gitm ; j++){ */
  /*       free(PARAMS->ATMOSPHERE.density_gitm_right_after[i][j]); */
  /*       } */
  /*   free(PARAMS->ATMOSPHERE.density_gitm_right_after[i]); */
  /*   } */
  /*   free(PARAMS->ATMOSPHERE.density_gitm_right_after ); */










  /* if ((dir = opendir ("/raid3/Gitm/Runs/20150317/data")) != NULL) { */
  /*   while ((ent = readdir (dir)) != NULL) { */
  /*     // find the extension of each file. We only care about the file if it is a .bin file */
  /*     strcpy(extension_gitm_file, ""); */
  /* next = &(ent->d_name)[0]; */
  /*   find_extension = (int)(strchr(next, '.') - next); */
  /*   strncat(extension_gitm_file, next+find_extension,4); */

  /*   if (strcmp(extension_gitm_file, ".bin") == 0) { // We only care about the file if it is a .bin file */
      
  /*     // We only care about the file if its name starts with "3DALL_t" (see IMPORTANT assumption above) */

  /*     strcpy(start_filename, ""); */
  /*     strncat(start_filename, &(ent->d_name)[0], 7); */
  /*     if (strcmp(start_filename, "3DALL_t") == 0) { // We only care about the file if its name starts with "3 *\/DALL_t" (see IMPORTANT assumption above) */

  /* 	// Convert the date format of the GITM file name to the Julian Epoch format */
  /* 	strcpy(date_gitm_file, "20"); */
  /* 	strncat(date_gitm_file, &(ent->d_name)[7],2); */
  /* 	strcat(date_gitm_file, "-"); */
  /* 	strncat(date_gitm_file, &(ent->d_name)[9],2); */
  /* 	strcat(date_gitm_file, "-"); */
  /* 	strncat(date_gitm_file, &(ent->d_name)[11],2); */
  /* 	strcat(date_gitm_file, "T"); */
  /* 	strncat(date_gitm_file, &(ent->d_name)[14],2); */
  /* 	strcat(date_gitm_file, ":"); */
  /* 	strncat(date_gitm_file, &(ent->d_name)[16],2); */
  /* 	strcat(date_gitm_file, ":"); */
  /* 	strncat(date_gitm_file, &(ent->d_name)[18],2); */
  /* 	str2et_c(date_gitm_file, &et_gitm_file); */

  /* 	// Find the 2 GITM files that have dates around the epoch of the sc */
  /* 	delta_et = et_gitm_file - et; */
  /* 	if (delta_et < 0){ // the date of the GITM file is older than the epoch of the sc */
  /* 	  if (delta_et > delta_et_right_before) { // the date of the GITM file is newer than the previous record */
  /* 	    delta_et_right_before = delta_et; */
  /* 	    et_right_before = et_gitm_file; */
  /* 	    strcpy(name_gitm_file_right_before_epoch_of_sc, "/raid3/Gitm/Runs/20150317/data/"); */
  /* 	    strcat(name_gitm_file_right_before_epoch_of_sc, ent->d_name); */
  /* 	  } */
  /* 	} */
  /* 	if (delta_et > 0){ // the date of the GITM file is newer than the epoch of the sc */
  /* 	  if (delta_et < delta_et_right_after) { // the date of the GITM file is older than the previous record */
  /* 	    delta_et_right_after = delta_et; */
  /* 	    et_right_after = et_gitm_file; */
  /* 	    strcpy(name_gitm_file_right_after_epoch_of_sc, "/raid3/Gitm/Runs/20150317/data/"); */
  /* 	    strcat(name_gitm_file_right_after_epoch_of_sc, ent->d_name); */
  /* 	  } */
  /* 	} */
  /* 	if (delta_et == 0){ // the date of the GITM file is exactly the epoch of the sc */
  /* 	  do_not_need_to_interpolate = 1; */
  /* 	  strcpy(name_gitm_file_right_before_epoch_of_sc, "/raid3/Gitm/Runs/20150317/data/"); */
  /* 	  strcat(name_gitm_file_right_before_epoch_of_sc, ent->d_name); */
  /* 	} */

  /*     } // end of we only care about the file if its name starts with "3DALL_t" (see IMPORTANT assumption above)   */
  /*   } // end of we only care about the file if it is a .bin file */
   
  /*   } */
  /*   closedir (dir); */
  /* } else { */
  /*   /\* could not open directory *\/ */
  /*   printf("Could not open the GITM data directory. The program will stop.\n"); */
  /*   exit(0); */
  /* } */





  //printf("\n\n%s | %s | %s \n\n", name_gitm_file_right_before_epoch_of_sc, time_sc, name_gitm_file_right_after_epoch_of_sc);
  //  char time_gitm_right_before_sc[256], time_gitm_right_after_sc[256];
  /* et2utc_c(et_right_before, "ISOC" ,0 ,255 , time_gitm_right_before_sc);     */
  /* et2utc_c(et_right_after, "ISOC" ,0 ,255 , time_gitm_right_after_sc);     */
  /* printf("\n\n%s | %s | %s || %d\n\n", time_gitm_right_before_sc, time_sc, time_gitm_right_after_sc, do_not_need_to_interpolate  ); */

}




/* int eclipse_sun_moon_sc(){ */






/* /\*   //  spkez_c(10, et, "J2000", "NONE", 399, x, &lt); //   Return the state (position and velocity) of a target body relative to an observing body, optionally corrected for light time (planetary aberration) and stellar aberration. *\/ */
/* /\*   double x_earth_sun[6], x_moon_sun[6]; *\/ */
/* /\*   double lt_earth_sun, lt_moon_sun; *\/ */
/* /\*     double r_earth2sun_J2000[3], r_moon2sun_J2000[3]; *\/ */
/* /\*     double moon_radius = 1737. km; *\/ */
/* /\*   spkez_c(10, et, "J2000", "NONE", 399, x_earth_sun, &lt_earth_sun); //   Return the state (position and velocity) of a target body relative to an observing body, optionally corrected for light time (planetary aberration) and stellar aberration. *\/ */
/* /\*   spkez_c(10, et, "J2000", "NONE", 301, x_moon_sun, &lt_moon_sun); //   Return the state (position and velocity) of a target body relative to an observing body, optionally corrected for light time (planetary aberration) and stellar aberration. *\/ */
/* /\*   r_earth2sun_J2000[0] = x_earth_sun[0]; *\/ */
/* /\*   r_earth2sun_J2000[1] = x_earth_sun[1]; *\/ */
/* /\*   r_earth2sun_J2000[2] = x_earth_sun[2]; *\/ */
  
/* /\*   r_moon2sun_J2000[0] = x_moon_sun[0]; *\/ */
/* /\*   r_moon2sun_J2000[1] = x_moon_sun[1]; *\/ */
/* /\*   r_moon2sun_J2000[2] = x_moon_sun[2]; *\/ */

/* /\*   r_edge_moon3sun_J2000[0] = jj; *\/ */
  

/* /\*   // sc in shadow of Moon if: *\/ */
/* /\*   // - angle (sun to moon, sun to sc) < angle (sun to moon, sun to edge of moon); and *\/ */
/* /\*   // - distance sun to moon < distance sun to sc *\/ */


/*   // sc in eclipse by moon if: */
/*   // - sc in shadow of Moon; and */
/*   // - sc not in shadow of Earth */
  
/* /\*   // Returns if the satellite is or not in the shadow of Earth *\/ */
/* /\*   shadow_light( SC->INTEGRATOR.shadow, SC->r_i2cg_INRTL, SC->et, PARAMS); *\/ */

/*   // Returns if the satellite is or not in the shadow of Moon */
/*   shadow_light_moon( SC->INTEGRATOR.shadow, SC->r_i2cg_INRTL, SC->et, PARAMS); */
  
/*   return 0; */

/* } */





/* ///////////////////////////////////////////////////////////////////////////////////////// */
/* // */
/* //  Name:           read_gitm_bin */
/* //  Purpose:        Read a GITM bin file   */
/* //  Assumptions:    None. */
/* //  References      Various. Note: the full code is in PropSim_v3/idl_propagator/read_gitm_bin.c */
/* // */
/* //  Change Log: */
/* //      |   Developer   |       Date    |   SCR     |   Notes */
/* //      | --------------|---------------|-----------|------------------------------- */
/* //      | C. Bussy-Virat| 01/24/2016    |   ---     | Initial implementation */
/* // */
/* ///////////////////////////////////////////////////////////////////////////////////////// */
/* int read_gitm_bin( double small_array[5], double ***longitude_gitm, double ***latitude_gitm, double ***altitude_gitm, double ***density_gitm, char time_gitm[256], int nLons, int nLats, int nAlts, char name_gitm_file_to_read[256]) { */

/*   // Declarations */
/*      int pos_for_lon, iDataLength, iHeaderLength; */
/*    int time_gitm_temp[7]; */
/*      int j,k; */
/*   int i; */
/*   int nVars = 0; */
/*   FILE *gitm_file; */

/*   //  small_array = malloc(sizeof(double) * 3); */
/*   small_array[0] = 0; */
/*   small_array[1] = 1; */
/*   small_array[2] = 2; */

/*   gitm_file = fopen(name_gitm_file_to_read,"rb"); */

/* // NLONS, NLATS, NALTS */
/* fseek(gitm_file, 20, SEEK_SET	 ); */
/*   fread(&nLons,sizeof(nLons),1,gitm_file); */
/*   fread(&nLats,sizeof(nLats),1,gitm_file); */
/*   fread(&nAlts,sizeof(nAlts),1,gitm_file); */
/* fseek(gitm_file, 4, SEEK_CUR	 ); */
/*  /\*   printf("nLons = %d \n", nLons); *\/ */
/*  /\* printf("nLats = %d \n", nLats); *\/ */
/*  /\* printf("nAlts = %d \n", nAlts); *\/ */

/* // NVARS */
/* fseek(gitm_file, 4, SEEK_CUR	 ); */
/*   fread(&nVars,sizeof(nVars),1,gitm_file); */
/* fseek(gitm_file, 4, SEEK_CUR	 ); */
/* // printf("nVars = %d \n", nVars); */
  
/*    // TIME */
/*    fseek(gitm_file, 4+nVars*(4+4+40), SEEK_CUR	 );     */
/*   fread(&time_gitm_temp,sizeof(time_gitm_temp),1,gitm_file); */
/*    /\* for (i = 0; i < 7; i++){ *\/ */
/*    /\*    printf("time: %d\n", time_gitm_temp[i]); *\/ */
/*    /\*   } *\/ */

/*      // SKIP VARIABLES BEFORE THE DENSITY */
/*      iHeaderLength = 8L + 4+4 +	3*4 + 4+4 + 4 + 4+4 + nVars*40 + nVars*(4+4) +  7*4 + 4+4; */
/*      iDataLength = nLons*nLats*nAlts*8 + 4+4; */
/*      pos_for_lon = iHeaderLength  ;// density is the 4th variable in the GITM file */
/*      fseek(gitm_file, pos_for_lon, SEEK_SET); */



/*      longitude_gitm = malloc(  nLons * sizeof(double **) ); */
/*       for (i = 0; i < nLons; i++){ */
/* 	longitude_gitm[i] = malloc(nLats * sizeof(double *)); */
/* 	for (j = 0; j < nLats; j++){ */
/* 	  longitude_gitm[i][j] = malloc(nAlts * sizeof(double)); */
/* 	} */
/*       }  */
/*      latitude_gitm = malloc(  nLons * sizeof(double **) ); */
/*       for (i = 0; i < nLons; i++){ */
/* 	latitude_gitm[i] = malloc(nLats * sizeof(double *)); */
/* 	for (j = 0; j < nLats; j++){ */
/* 	  latitude_gitm[i][j] = malloc(nAlts * sizeof(double)); */
/* 	} */
/*       } */
/*       altitude_gitm = malloc(  nLons * sizeof(double **) ); */
/*       for (i = 0; i < nLons; i++){ */
/* 	altitude_gitm[i] = malloc(nLats * sizeof(double *)); */
/* 	for (j = 0; j < nLats; j++){ */
/* 	  altitude_gitm[i][j] = malloc(nAlts * sizeof(double)); */
/* 	} */
/*       } */


/*       density_gitm = malloc(  nLons * sizeof(double **) ); */
/*       for (i = 0; i < nLons; i++){ */
/* 	density_gitm[i] = malloc(nLats * sizeof(double *)); */
/* 	for (j = 0; j < nLats; j++){ */
/* 	  density_gitm[i][j] = malloc(nAlts * sizeof(double)); */
/* 	} */
/*       } */
/*       if (  longitude_gitm == NULL ){ */
/*   	printf("Could not allow memory for longitude_gitm. The program will stop.\n"); */
/*   	exit(0); */
/*       } */
/*       if (  latitude_gitm == NULL ){ */
/*   	printf("Could not allow memory for latitude_gitm. The program will stop.\n"); */
/*   	exit(0); */
/*       } */
/*       if (  altitude_gitm == NULL ){ */
/*   	printf("Could not allow memory for altitude_gitm. The program will stop.\n"); */
/*   	exit(0); */
/*       } */

/*        if (  density_gitm == NULL ){ */
/*   	printf("Could not allow memory for density_gitm. The program will stop.\n"); */
/*   	exit(0); */
/*       } */

/*   // READ THE LONGITUDE */
/*      fseek(gitm_file, 4, SEEK_CUR ); */
/*      for (k = 0; k < nAlts; k++){ */
/*        for (j = 0; j < nLats; j++){ */
/*   	 for (i = 0; i < nLons; i++){ */
/*      	   fread(&longitude_gitm[i][j][k], sizeof(longitude_gitm[i][j][k]), 1, gitm_file); */
/*      	 } */
/*        } */
/*      } */
/*      fseek(gitm_file, 4, SEEK_CUR ); */
/*      //    printf("longitude = %f\n", longitude_gitm[11][40][21]); */


/*   // READ THE LATITUDE */
/*      fseek(gitm_file, 4, SEEK_CUR ); */
/*      for (k = 0; k < nAlts; k++){ */
/*        for (j = 0; j < nLats; j++){ */
/*   	 for (i = 0; i < nLons; i++){ */
/*      	   fread(&latitude_gitm[i][j][k], sizeof(latitude_gitm[i][j][k]), 1, gitm_file); */
/*      	 } */
/*        } */
/*      } */
/*      fseek(gitm_file, 4, SEEK_CUR ); */
/*      //     printf("latitude = %f\n", latitude_gitm[11][40][21]); */

/*   // READ THE ALTITUDE */

/*      fseek(gitm_file, 4, SEEK_CUR ); */
/*      for (k = 0; k < nAlts; k++){ */
/*        for (j = 0; j < nLats; j++){ */
/*   	 for (i = 0; i < nLons; i++){ */
/*      	   fread(&altitude_gitm[i][j][k], sizeof(altitude_gitm[i][j][k]), 1, gitm_file); */
/*      	 } */
/*        } */
/*      } */
/*      fseek(gitm_file, 4, SEEK_CUR ); */
/*      //   printf("altitude = %f\n", altitude_gitm[11][40][21]); */

/*      // READ THE DENSITY AT THE GIVEN LON/LAT/ALT */

/*      fseek(gitm_file, 4, SEEK_CUR ); */
/*      for (k = 0; k < nAlts; k++){ */
/*        for (j = 0; j < nLats; j++){ */
/*   	 for (i = 0; i < nLons; i++){ */
/*      	   fread(&density_gitm[i][j][k], sizeof(density_gitm[i][j][k]), 1, gitm_file); */
/*      	 } */
/*        } */
/*      } */
/*      //               printf("density = %e\n", density_gitm[11][40][21]); */

/* /\* int read_gitm_bin( double *longitude_gitm, double *latitude_gitm, double *altitude_gitm, double *density_gitm, char time_gitm[256], char name_gitm_file_to_read[256]) { *\/ */
/*   return 0; */
/* } */


int calculate_cd(double *cd,
		 double acco_coeff, 
		 double v_sc_mag, //in km/s
		 double Ta, // atmospheric temperature in K, from NRLSMSIS
		 double surface_area, // in km^2
		 double gamma,
		 double Ma // in kg/mol!!!!!from NRLMSIS?
		 ){
  //Ta = 800;  // !!!!!!!!!!!!!!!!! REMOVE
  //v_sc_mag  = 7.3; // !!!!!!!!!!!!!!!!! REMOVE

//printf("%e\n",surface_area*gamma);
  double Tw = 300.; // temperature of the satellite surface // assumed at 300 K in Moe04, 273 in Sutton07 and Bruisma03
  v_sc_mag = v_sc_mag * 1000.; // km/s to m/s
  surface_area = surface_area * 1000000.; // km^2 to m^2
  //    printf("%e\n", v_sc_mag);

      double A_ref =  surface_area * gamma;
      double R = 8.31; // universal gas constant in J/mol.K
	double s = v_sc_mag / sqrt( 2 * R * Ta / Ma) ;
	double Q = 1 + 1 / ( 2 * s*s );
	double T = Ma * v_sc_mag * v_sc_mag / ( 3 * R );  // kinetic temperature
				      
	double Vr = v_sc_mag * sqrt( 2./3 * ( 1 + acco_coeff * ( Tw / T - 1 )  ) );
	
	double P = exp( -( gamma*gamma ) * ( s*s ) ) / s;
	double Z = 1 + erf( gamma * s );

	double    term1 = P / sqrt(M_PI);
	double term2 = gamma * Q * Z;
	double fac1 = gamma * Vr / (2 * v_sc_mag);
	  double fac2 = gamma * sqrt(M_PI) * Z + P;
	  double term3 = fac1 * fac2;
	  double term = term1 + term2 + term3;
	  cd[0] = surface_area / A_ref * term ;


	  //	  printf("%e %e %e\n",term1 , term2 , term3);
/*     // TOTAL NORMALIZED DRAG COEFFICIENT */
/*     A_ref_tot = 0 */
/*     for isurf in range(nb_surf): */
/*   if gamma[isurf] >= 0: */
/*   A_ref_tot = A_ref_tot + A_ref[isurf] */
/*     cd_tot_norm = np.sum(cd * A_ref) / A_ref_tot //     see sutton09a equation 9 */

//	  printf("%f\n", cd[0]);
    return 0;

}
