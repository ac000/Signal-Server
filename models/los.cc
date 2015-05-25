#include <stdio.h>
#include <math.h>

#include "../common.h"
#include "../main.hh"
#include "fspl.hh"
#include "models.hh"
#include "itwom3.0.hh"

void PlotLOSPath(struct site source, struct site destination, char mask_value,
		 FILE *fd)
{
	/* This function analyzes the path between the source and
	   destination locations.  It determines which points along
	   the path have line-of-sight visibility to the source.
	   Points along with path having line-of-sight visibility
	   to the source at an AGL altitude equal to that of the
	   destination location are stored by setting bit 1 in the
	   mask[][] array, which are displayed in green when PPM
	   maps are later generated by ss. */

	char block;
	int x, y;
	register double cos_xmtr_angle, cos_test_angle, test_alt;
	double distance, rx_alt, tx_alt;

	ReadPath(source, destination);

	for (y = 0; y < path.length; y++) {
		/* Test this point only if it hasn't been already
		   tested and found to be free of obstructions. */

		if ((GetMask(path.lat[y], path.lon[y]) & mask_value) == 0) {
			distance = 5280.0 * path.distance[y];
			tx_alt = earthradius + source.alt + path.elevation[0];
			rx_alt =
			    earthradius + destination.alt + path.elevation[y];

			/* Calculate the cosine of the elevation of the
			   transmitter as seen at the temp rx point. */

			cos_xmtr_angle =
			    ((rx_alt * rx_alt) + (distance * distance) -
			     (tx_alt * tx_alt)) / (2.0 * rx_alt * distance);

			for (x = y, block = 0; x >= 0 && block == 0; x--) {
				distance =
				    5280.0 * (path.distance[y] -
					      path.distance[x]);
				test_alt =
				    earthradius + (path.elevation[x] ==
						   0.0 ? path.
						   elevation[x] : path.
						   elevation[x] + clutter);

				cos_test_angle =
				    ((rx_alt * rx_alt) + (distance * distance) -
				     (test_alt * test_alt)) / (2.0 * rx_alt *
							       distance);

				/* Compare these two angles to determine if
				   an obstruction exists.  Since we're comparing
				   the cosines of these angles rather than
				   the angles themselves, the following "if"
				   statement is reversed from what it would
				   be if the actual angles were compared. */

				if (cos_xmtr_angle >= cos_test_angle)
					block = 1;
			}

			if (block == 0)
				OrMask(path.lat[y], path.lon[y], mask_value);
		}
	}
}

void PlotPropPath(struct site source, struct site destination,
		  unsigned char mask_value, FILE * fd, int propmodel,
		  int knifeedge, int pmenv)
{

	int x, y, ifs, ofs, errnum;
	char block = 0, strmode[100];
	double loss, azimuth, pattern = 0.0,
	    xmtr_alt, dest_alt, xmtr_alt2, dest_alt2,
	    cos_rcvr_angle, cos_test_angle = 0.0, test_alt,
	    elevation = 0.0, distance = 0.0, radius = 0.0, four_thirds_earth,
	    field_strength = 0.0, rxp, dBm, txelev, dkm, diffloss;
	struct site temp;

	radius = Distance(source, destination);

	ReadPath(source, destination);

	four_thirds_earth = FOUR_THIRDS * EARTHRADIUS;

	for (x = 1; x < path.length - 1; x++)
		elev[x + 2] =
		    (path.elevation[x] ==
		     0.0 ? path.elevation[x] * METERS_PER_FOOT : (clutter +
								  path.
								  elevation[x])
		     * METERS_PER_FOOT);

	/* Copy ending points without clutter */

	elev[2] = path.elevation[0] * METERS_PER_FOOT;
	txelev = elev[2] + (source.alt * METERS_PER_FOOT);

	elev[path.length + 1] =
	    path.elevation[path.length - 1] * METERS_PER_FOOT;

	/* Since the only energy the Longley-Rice model considers
	   reaching the destination is based on what is scattered
	   or deflected from the first obstruction along the path,
	   we first need to find the location and elevation angle
	   of that first obstruction (if it exists).  This is done
	   using a 4/3rds Earth radius to match the model used by
	   Longley-Rice.  This information is required for properly
	   integrating the antenna's elevation pattern into the
	   calculation for overall path loss. */

	for (y = 2; (y < (path.length - 1) && path.distance[y] <= max_range);
	     y++) {
		/* Process this point only if it
		   has not already been processed. */

		if ((GetMask(path.lat[y], path.lon[y]) & 248) !=
		    (mask_value << 3)) {
			distance = 5280.0 * path.distance[y];
			xmtr_alt =
			    four_thirds_earth + source.alt + path.elevation[0];
			dest_alt =
			    four_thirds_earth + destination.alt +
			    path.elevation[y];
			dest_alt2 = dest_alt * dest_alt;
			xmtr_alt2 = xmtr_alt * xmtr_alt;

			/* Calculate the cosine of the elevation of
			   the receiver as seen by the transmitter. */

			cos_rcvr_angle =
			    ((xmtr_alt2) + (distance * distance) -
			     (dest_alt2)) / (2.0 * xmtr_alt * distance);

			if (cos_rcvr_angle > 1.0)
				cos_rcvr_angle = 1.0;

			if (cos_rcvr_angle < -1.0)
				cos_rcvr_angle = -1.0;

			if (got_elevation_pattern || fd != NULL) {
				/* Determine the elevation angle to the first obstruction
				   along the path IF elevation pattern data is available
				   or an output (.ano) file has been designated. */

				for (x = 2, block = 0; (x < y && block == 0);
				     x++) {
					distance = 5280.0 * path.distance[x];

					test_alt =
					    four_thirds_earth +
					    (path.elevation[x] ==
					     0.0 ? path.elevation[x] : path.
					     elevation[x] + clutter);

					/* Calculate the cosine of the elevation
					   angle of the terrain (test point)
					   as seen by the transmitter. */

					cos_test_angle =
					    ((xmtr_alt2) +
					     (distance * distance) -
					     (test_alt * test_alt)) / (2.0 *
								       xmtr_alt
								       *
								       distance);

					if (cos_test_angle > 1.0)
						cos_test_angle = 1.0;

					if (cos_test_angle < -1.0)
						cos_test_angle = -1.0;

					/* Compare these two angles to determine if
					   an obstruction exists.  Since we're comparing
					   the cosines of these angles rather than
					   the angles themselves, the sense of the
					   following "if" statement is reversed from
					   what it would be if the angles themselves
					   were compared. */

					if (cos_rcvr_angle >= cos_test_angle)
						block = 1;
				}

				if (block)
					elevation =
					    ((acos(cos_test_angle)) / DEG2RAD) -
					    90.0;
				else
					elevation =
					    ((acos(cos_rcvr_angle)) / DEG2RAD) -
					    90.0;
			}

			/* Determine attenuation for each point along the
			   path using a prop model starting at y=2 (number_of_points = 1), the
			   shortest distance terrain can play a role in
			   path loss. */

			elev[0] = y - 1;	/* (number of points - 1) */

			/* Distance between elevation samples */

			elev[1] =
			    METERS_PER_MILE * (path.distance[y] -
					       path.distance[y - 1]);

			if (path.elevation[y] < 1) {
				path.elevation[y] = 1;
			}

			dkm = (elev[1] * elev[0]) / 1000;	// km

			switch (propmodel) {
			case 1:
				// Longley Rice ITM
				point_to_point_ITM(elev,
						   source.alt * METERS_PER_FOOT,
						   destination.alt *
						   METERS_PER_FOOT,
						   LR.eps_dielect,
						   LR.sgm_conductivity,
						   LR.eno_ns_surfref,
						   LR.frq_mhz, LR.radio_climate,
						   LR.pol, LR.conf, LR.rel,
						   loss, strmode, errnum);
				break;
			case 3:
				//HATA 1, 2 & 3
				loss =
				    HATApathLoss(LR.frq_mhz, txelev,
						 path.elevation[y] +
						 (destination.alt *
						  METERS_PER_FOOT), dkm, pmenv);
				break;
			case 4:
				// COST231-HATA
				loss =
				    ECC33pathLoss(LR.frq_mhz, txelev,
						  path.elevation[y] +
						  (destination.alt *
						   METERS_PER_FOOT), dkm,
						  pmenv);
				break;
			case 5:
				// SUI
				loss =
				    SUIpathLoss(LR.frq_mhz, txelev,
						path.elevation[y] +
						(destination.alt *
						 METERS_PER_FOOT), dkm, pmenv);
				break;
			case 6:
				loss =
				    COST231pathLoss(LR.frq_mhz, txelev,
						    path.elevation[y] +
						    (destination.alt *
						     METERS_PER_FOOT), dkm,
						    pmenv);
				break;
			case 7:
				// ITU-R P.525 Free space path loss
				loss = FSPLpathLoss(LR.frq_mhz, dkm);
				break;
			case 8:
				// ITWOM 3.0
				point_to_point(elev,
					       source.alt * METERS_PER_FOOT,
					       destination.alt *
					       METERS_PER_FOOT, LR.eps_dielect,
					       LR.sgm_conductivity,
					       LR.eno_ns_surfref, LR.frq_mhz,
					       LR.radio_climate, LR.pol,
					       LR.conf, LR.rel, loss, strmode,
					       errnum);
				break;
			case 9:
				// Ericsson
				loss =
				    EricssonpathLoss(LR.frq_mhz, txelev,
						     path.elevation[y] +
						     (destination.alt *
						      METERS_PER_FOOT), dkm,
						     pmenv);
				break;

			default:
				point_to_point_ITM(elev,
						   source.alt * METERS_PER_FOOT,
						   destination.alt *
						   METERS_PER_FOOT,
						   LR.eps_dielect,
						   LR.sgm_conductivity,
						   LR.eno_ns_surfref,
						   LR.frq_mhz, LR.radio_climate,
						   LR.pol, LR.conf, LR.rel,
						   loss, strmode, errnum);

			}

			if (knifeedge == 1) {
				diffloss =
				    ked(LR.frq_mhz, elev,
					destination.alt * METERS_PER_FOOT, dkm);
				loss += (diffloss);	// ;)
			}
			//Key stage. Link dB for p2p is returned as 'loss'.

			temp.lat = path.lat[y];
			temp.lon = path.lon[y];

			azimuth = (Azimuth(source, temp));

			if (fd != NULL)
				fprintf(fd, "%.7f, %.7f, %.3f, %.3f, ",
					path.lat[y], path.lon[y], azimuth,
					elevation);

			/* If ERP==0, write path loss to alphanumeric
			   output file.  Otherwise, write field strength
			   or received power level (below), as appropriate. */

			if (fd != NULL && LR.erp == 0.0)
				fprintf(fd, "%.2f", loss);

			/* Integrate the antenna's radiation
			   pattern into the overall path loss. */

			x = (int)rint(10.0 * (10.0 - elevation));

			if (x >= 0 && x <= 1000) {
				azimuth = rint(azimuth);

				pattern =
				    (double)LR.antenna_pattern[(int)azimuth][x];

				if (pattern != 0.0) {
					pattern = 20.0 * log10(pattern);
					loss -= pattern;
				}
			}

			if (LR.erp != 0.0) {
				if (dbm) {
					/* dBm is based on EIRP (ERP + 2.14) */

					rxp =
					    LR.erp /
					    (pow(10.0, (loss - 2.14) / 10.0));

					dBm = 10.0 * (log10(rxp * 1000.0));

					if (fd != NULL)
						fprintf(fd, "%.3f", dBm);

					/* Scale roughly between 0 and 255 */

					ifs = 200 + (int)rint(dBm);

					if (ifs < 0)
						ifs = 0;

					if (ifs > 255)
						ifs = 255;

					ofs =
					    GetSignal(path.lat[y], path.lon[y]);

					if (ofs > ifs)
						ifs = ofs;

					PutSignal(path.lat[y], path.lon[y],
						  (unsigned char)ifs);

				}

				else {
					field_strength =
					    (139.4 +
					     (20.0 * log10(LR.frq_mhz)) -
					     loss) +
					    (10.0 * log10(LR.erp / 1000.0));

					ifs = 100 + (int)rint(field_strength);

					if (ifs < 0)
						ifs = 0;

					if (ifs > 255)
						ifs = 255;

					ofs =
					    GetSignal(path.lat[y], path.lon[y]);

					if (ofs > ifs)
						ifs = ofs;

					PutSignal(path.lat[y], path.lon[y],
						  (unsigned char)ifs);

					if (fd != NULL)
						fprintf(fd, "%.3f",
							field_strength);
				}
			}

			else {
				if (loss > 255)
					ifs = 255;
				else
					ifs = (int)rint(loss);

				ofs = GetSignal(path.lat[y], path.lon[y]);

				if (ofs < ifs && ofs != 0)
					ifs = ofs;

				PutSignal(path.lat[y], path.lon[y],
					  (unsigned char)ifs);
			}

			if (fd != NULL) {
				if (block)
					fprintf(fd, " *");

				fprintf(fd, "\n");
			}

			/* Mark this point as having been analyzed */

			PutMask(path.lat[y], path.lon[y],
				(GetMask(path.lat[y], path.lon[y]) & 7) +
				(mask_value << 3));
		}
	}

}

void PlotLOSMap(struct site source, double altitude, char *plo_filename)
{
	/* This function performs a 360 degree sweep around the
	   transmitter site (source location), and plots the
	   line-of-sight coverage of the transmitter on the ss
	   generated topographic map based on a receiver located
	   at the specified altitude (in feet AGL).  Results
	   are stored in memory, and written out in the form
	   of a topographic map when the WritePPM() function
	   is later invoked. */

	int y, z;
	struct site edge;
	unsigned char x;
	double lat, lon, minwest, maxnorth, th;
	static unsigned char mask_value = 1;
	FILE *fd = NULL;

	if (plo_filename[0] != 0)
		fd = fopen(plo_filename, "wb");

	if (fd != NULL) {
		fprintf(fd,
			"%d, %d\t; max_west, min_west\n%d, %d\t; max_north, min_north\n",
			max_west, min_west, max_north, min_north);
	}

	th = ppd / loops;

	z = (int)(th * ReduceAngle(max_west - min_west));

	minwest = dpp + (double)min_west;
	maxnorth = (double)max_north - dpp;

	for (lon = minwest, x = 0, y = 0;
	     (LonDiff(lon, (double)max_west) <= 0.0);
	     y++, lon = minwest + (dpp * (double)y)) {
		if (lon >= 360.0)
			lon -= 360.0;

		edge.lat = max_north;
		edge.lon = lon;
		edge.alt = altitude;

		PlotLOSPath(source, edge, mask_value, fd);
	}

	z = (int)(th * (double)(max_north - min_north));

	for (lat = maxnorth, x = 0, y = 0; lat >= (double)min_north;
	     y++, lat = maxnorth - (dpp * (double)y)) {
		edge.lat = lat;
		edge.lon = min_west;
		edge.alt = altitude;

		PlotLOSPath(source, edge, mask_value, fd);

	}

	z = (int)(th * ReduceAngle(max_west - min_west));

	for (lon = minwest, x = 0, y = 0;
	     (LonDiff(lon, (double)max_west) <= 0.0);
	     y++, lon = minwest + (dpp * (double)y)) {
		if (lon >= 360.0)
			lon -= 360.0;

		edge.lat = min_north;
		edge.lon = lon;
		edge.alt = altitude;

		PlotLOSPath(source, edge, mask_value, fd);

	}

	z = (int)(th * (double)(max_north - min_north));

	for (lat = (double)min_north, x = 0, y = 0; lat < (double)max_north;
	     y++, lat = (double)min_north + (dpp * (double)y)) {
		edge.lat = lat;
		edge.lon = max_west;
		edge.alt = altitude;

		PlotLOSPath(source, edge, mask_value, fd);

	}

	switch (mask_value) {
	case 1:
		mask_value = 8;
		break;

	case 8:
		mask_value = 16;
		break;

	case 16:
		mask_value = 32;
	}
}

void PlotPropagation(struct site source, double altitude, char *plo_filename,
		     int propmodel, int knifeedge, int haf, int pmenv)
{
	int y, z, count;
	struct site edge;
	double lat, lon, minwest, maxnorth, th;
	unsigned char x;
	static unsigned char mask_value = 1;
	FILE *fd = NULL;

	minwest = dpp + (double)min_west;
	maxnorth = (double)max_north - dpp;

	count = 0;

	if (LR.erp == 0.0 && debug)
		fprintf(stdout, "path loss");
	else {
		if (debug) {
			if (dbm)
				fprintf(stdout, "signal power level");
			else
				fprintf(stdout, "field strength");
		}
	}
	if (debug) {
		fprintf(stdout,
			" contours of \"%s\"\nout to a radius of %.2f %s with Rx antenna(s) at %.2f %s AGL\n",
			source.name,
			metric ? max_range * KM_PER_MILE : max_range,
			metric ? "kilometers" : "miles",
			metric ? altitude * METERS_PER_FOOT : altitude,
			metric ? "meters" : "feet");
	}

	if (clutter > 0.0 && debug)
		fprintf(stdout, "\nand %.2f %s of ground clutter",
			metric ? clutter * METERS_PER_FOOT : clutter,
			metric ? "meters" : "feet");

	if (debug) {
		fprintf(stdout, "...\n\n 0%c to  25%c ", 37, 37);
		fflush(stdout);
	}

	if (plo_filename[0] != 0)
		fd = fopen(plo_filename, "wb");

	if (fd != NULL) {
		fprintf(fd,
			"%d, %d\t; max_west, min_west\n%d, %d\t; max_north, min_north\n",
			max_west, min_west, max_north, min_north);
	}

	th = ppd / loops;

	// Four sections start here

	//S1
	if (haf == 0 || haf == 1) {
		z = (int)(th * ReduceAngle(max_west - min_west));

		for (lon = minwest, x = 0, y = 0;
		     (LonDiff(lon, (double)max_west) <= 0.0);
		     y++, lon = minwest + (dpp * (double)y)) {
			if (lon >= 360.0)
				lon -= 360.0;

			edge.lat = max_north;
			edge.lon = lon;
			edge.alt = altitude;

			PlotPropPath(source, edge, mask_value, fd, propmodel,
				     knifeedge, pmenv);
			count++;

			if (count == z) {
				count = 0;

				if (x == 3)
					x = 0;
				else
					x++;
			}
		}

	}
	//S2
	if (haf == 0 || haf == 1) {
		count = 0;
		if (debug) {
			fprintf(stdout, "\n25%c to  50%c ", 37, 37);
			fflush(stdout);
		}

		z = (int)(th * (double)(max_north - min_north));

		for (lat = maxnorth, x = 0, y = 0; lat >= (double)min_north;
		     y++, lat = maxnorth - (dpp * (double)y)) {
			edge.lat = lat;
			edge.lon = min_west;
			edge.alt = altitude;

			PlotPropPath(source, edge, mask_value, fd, propmodel,
				     knifeedge, pmenv);
			count++;

			if (count == z) {
				count = 0;

				if (x == 3)
					x = 0;
				else
					x++;
			}
		}

	}
	//S3
	if (haf == 0 || haf == 2) {
		count = 0;
		if (debug) {
			fprintf(stdout, "\n50%c to  75%c ", 37, 37);
			fflush(stdout);
		}

		z = (int)(th * ReduceAngle(max_west - min_west));

		for (lon = minwest, x = 0, y = 0;
		     (LonDiff(lon, (double)max_west) <= 0.0);
		     y++, lon = minwest + (dpp * (double)y)) {
			if (lon >= 360.0)
				lon -= 360.0;

			edge.lat = min_north;
			edge.lon = lon;
			edge.alt = altitude;

			PlotPropPath(source, edge, mask_value, fd, propmodel,
				     knifeedge, pmenv);
			count++;
			if (count == z) {
				count = 0;

				if (x == 3)
					x = 0;
				else
					x++;
			}

		}

	}
	//S4
	if (haf == 0 || haf == 2) {
		count = 0;
		if (debug) {
			fprintf(stdout, "\n75%c to 100%c ", 37, 37);
			fflush(stdout);
		}
		z = (int)(th * (double)(max_north - min_north));

		for (lat = (double)min_north, x = 0, y = 0;
		     lat < (double)max_north;
		     y++, lat = (double)min_north + (dpp * (double)y)) {
			edge.lat = lat;
			edge.lon = max_west;
			edge.alt = altitude;

			PlotPropPath(source, edge, mask_value, fd, propmodel,
				     knifeedge, pmenv);
			count++;

			if (count == z) {

				count = 0;

				if (x == 3)
					x = 0;
				else
					x++;
			}
		}

	}			//S4

	if (fd != NULL)
		fclose(fd);

	if (mask_value < 30)
		mask_value++;
}

void PlotPath(struct site source, struct site destination, char mask_value)
{
	/* This function analyzes the path between the source and
	   destination locations.  It determines which points along
	   the path have line-of-sight visibility to the source.
	   Points along with path having line-of-sight visibility
	   to the source at an AGL altitude equal to that of the
	   destination location are stored by setting bit 1 in the
	   mask[][] array, which are displayed in green when PPM
	   maps are later generated by SPLAT!. */

	char block;
	int x, y;
	register double cos_xmtr_angle, cos_test_angle, test_alt;
	double distance, rx_alt, tx_alt;

	ReadPath(source, destination);

	for (y = 0; y < path.length; y++) {
		/* Test this point only if it hasn't been already
		   tested and found to be free of obstructions. */

		if ((GetMask(path.lat[y], path.lon[y]) & mask_value) == 0) {
			distance = 5280.0 * path.distance[y];
			tx_alt = earthradius + source.alt + path.elevation[0];
			rx_alt =
			    earthradius + destination.alt + path.elevation[y];

			/* Calculate the cosine of the elevation of the
			   transmitter as seen at the temp rx point. */

			cos_xmtr_angle =
			    ((rx_alt * rx_alt) + (distance * distance) -
			     (tx_alt * tx_alt)) / (2.0 * rx_alt * distance);

			for (x = y, block = 0; x >= 0 && block == 0; x--) {
				distance =
				    5280.0 * (path.distance[y] -
					      path.distance[x]);
				test_alt =
				    earthradius + (path.elevation[x] ==
						   0.0 ? path.
						   elevation[x] : path.
						   elevation[x] + clutter);

				cos_test_angle =
				    ((rx_alt * rx_alt) + (distance * distance) -
				     (test_alt * test_alt)) / (2.0 * rx_alt *
							       distance);

				/* Compare these two angles to determine if
				   an obstruction exists.  Since we're comparing
				   the cosines of these angles rather than
				   the angles themselves, the following "if"
				   statement is reversed from what it would
				   be if the actual angles were compared. */

				if (cos_xmtr_angle >= cos_test_angle)
					block = 1;
			}

			if (block == 0)
				OrMask(path.lat[y], path.lon[y], mask_value);
		}
	}
}
